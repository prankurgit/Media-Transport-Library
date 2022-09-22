/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2022 Intel Corporation
 */

#include "st_kni.h"

// #define DEBUG
#include "st_arp.h"
#include "st_cni.h"
#include "st_dev.h"
#include "st_log.h"
#include "st_ptp.h"
#include "st_sch.h"
#include "st_util.h"

static struct st_main_impl* g_kni_main_impl;

static inline void kni_set_global_impl(struct st_main_impl* impl) {
  g_kni_main_impl = impl;
}

static struct st_main_impl* kni_get_global_impl(void) {
  struct st_main_impl* impl = g_kni_main_impl;

  if (!impl) err("%s, global impl not init\n", __func__);

  return impl;
}

static int kni_init_conf(uint16_t port_id, struct rte_kni_conf* conf) {
  int ret;
  struct rte_eth_dev_info dev_info;

  ret = rte_eth_dev_info_get(port_id, &dev_info);
  if (ret < 0) {
    err("%s(%d), rte_eth_dev_info_get fail %d\n", __func__, port_id, ret);
    return ret;
  }

  ret = rte_eth_dev_get_mtu(port_id, &conf->mtu);
  if (ret < 0) {
    err("%s(%d), rte_eth_dev_get_mtu fail %d\n", __func__, port_id, ret);
    return ret;
  }

  ret = rte_eth_macaddr_get(port_id, (struct rte_ether_addr*)&conf->mac_addr);
  if (ret < 0) {
    err("%s(%d), rte_eth_macaddr_get fail %d\n", __func__, port_id, ret);
    return ret;
  }

  snprintf(conf->name, RTE_KNI_NAMESIZE, "vStKni%u_%s", port_id, dev_info.driver_name);
  conf->group_id = port_id;
  conf->mbuf_size = 2048;
  conf->min_mtu = dev_info.min_mtu;
  conf->max_mtu = dev_info.max_mtu;
  return 0;
}

static int kni_change_mtu(uint16_t port_id, unsigned int mtu) {
  info("%s(%d), mtu %d\n", __func__, port_id, mtu);
  return -EINVAL;
}
static int kni_config_promiscusity(uint16_t port_id, uint8_t to_on) {
  info("%s(%d), to_on %d\n", __func__, port_id, to_on);
  return -EINVAL;
}

static int kni_config_allmulticast(uint16_t port_id, uint8_t to_on) {
  info("%s(%d), to_on %d\n", __func__, port_id, to_on);
  return -EINVAL;
}

static int kni_config_network_if(uint16_t port_id, uint8_t if_up) {
  struct st_main_impl* impl = kni_get_global_impl();
  struct st_cni_impl* cni = st_get_cni(impl);
  enum st_port port = st_port_by_id(impl, port_id);

  rte_atomic32_set(&cni->if_up[port], if_up);
  info("%s(%d), if_up %d\n", __func__, port, if_up);
  return 0;
}

static int kni_config_mac_address(uint16_t port, uint8_t macAddr[]) {
  info("%s(%d), start\n", __func__, port);
  return 0;
}

static int kni_assign_ip(struct st_main_impl* impl, enum st_port port) {
  struct st_cni_impl* cni = st_get_cni(impl);
  int sock, ret;
  uint8_t* ip;
  struct ifreq ifr;
  char* if_name;

  sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock < 0) {
    err("%s(%d), socket fail\n", __func__, port);
    return sock;
  }

  // Assign IP to KNI
  ip = st_sip_addr(impl, port);
  if_name = cni->conf[port].name;
  strncpy(ifr.ifr_name, if_name, sizeof(ifr.ifr_name) - 1);
  ifr.ifr_ifru.ifru_addr.sa_family = AF_INET;
  ((struct sockaddr_in*)&ifr.ifr_ifru.ifru_addr)->sin_port = 0;
  memcpy(&((struct sockaddr_in*)&ifr.ifr_ifru.ifru_addr)->sin_addr.s_addr, ip,
         ST_IP_ADDR_LEN);
  ret = ioctl(sock, SIOCSIFADDR, &ifr);
  if (ret < 0) err("%s(%d), SIOCSIFADDR IP fail\n", __func__, port);
  info("%s(%d), IP:%d.%d.%d.%d set to KNI %s\n", __func__, port, ip[0], ip[1], ip[2],
       ip[3], if_name);

  close(sock);

  return 0;
}

static void* kni_bkg_thread(void* arg) {
  struct st_main_impl* impl = arg;
  struct st_cni_impl* cni = st_get_cni(impl);
  int num_ports = st_num_ports(impl);
  int ret, i;
  uint16_t port_id;
  struct rte_kni* rkni;
  struct rte_eth_link link;
  uint16_t link_status[num_ports];

  for (i = 0; i < num_ports; i++) link_status[i] = 0;

  info("%s, start\n", __func__);

  while (rte_atomic32_read(&cni->stop_kni) == 0) {
    for (i = 0; i < num_ports; i++) {
      port_id = st_port_id(impl, i);
      rkni = cni->rkni[i];

      rte_eth_link_get_nowait(port_id, &link);
      if (link_status[i] != link.link_status) {
        info("%s(%d), new link_status %d\n", __func__, i, link.link_status);
        ret = rte_kni_update_link(rkni, link.link_status);
        if (ret < 0) {
          err("%s(%d), rte_kni_update_link %d fail %d\n", __func__, i, link.link_status,
              ret);
        } else {
          link_status[i] = link.link_status;
          if (link.link_status) {
            st_sleep_ms(1 * 1000); /* 1s */
            kni_assign_ip(impl, i);
          }
        }
      }
    }
    st_sleep_ms(1 * 1000); /* 1s */
  }

  info("%s, stop\n", __func__);
  return NULL;
}

static int kni_start_port(struct st_main_impl* impl, enum st_port port) {
  struct st_cni_impl* cni = st_get_cni(impl);
  uint16_t port_id = st_port_id(impl, port);
  struct rte_kni* rkni;
  struct rte_kni_ops ops;

  memset(&ops, 0, sizeof(ops));
  ops.port_id = port_id;
  ops.config_network_if = kni_config_network_if;
  ops.config_mac_address = kni_config_mac_address;
  ops.change_mtu = kni_change_mtu;
  ops.config_promiscusity = kni_config_promiscusity;
  ops.config_allmulticast = kni_config_allmulticast;

  rkni = rte_kni_alloc(st_get_tx_mempool(impl, port), &cni->conf[port], &ops);
  if (!rkni) {
    err("%s(%d), rte_kni_alloc fail\n", __func__, port);
    return -ENOMEM;
  }
  cni->rkni[port] = rkni;

  info("%s(%d), succ\n", __func__, port_id);
  return 0;
}

static int kni_queues_uinit(struct st_main_impl* impl) {
  int num_ports = st_num_ports(impl);
  struct st_cni_impl* cni = st_get_cni(impl);

  for (int i = 0; i < num_ports; i++) {
    if (cni->tx_q_active[i]) {
      st_dev_free_tx_queue(impl, i, cni->tx_q_id[i]);
      cni->tx_q_active[i] = false;
    }
  }

  return 0;
}

static int kni_queues_init(struct st_main_impl* impl, struct st_cni_impl* cni) {
  int num_ports = st_num_ports(impl);
  int ret;

  for (int i = 0; i < num_ports; i++) {
    ret = st_dev_request_tx_queue(impl, i, &cni->tx_q_id[i], 0);
    if (ret < 0) {
      err("%s(%d), kni_tx_q create fail\n", __func__, i);
      kni_queues_uinit(impl);
      return ret;
    }
    cni->tx_q_active[i] = true;
    info("%s(%d), tx q %d\n", __func__, i, cni->tx_q_id[i]);
  }

  return 0;
}

int st_kni_handle(struct st_main_impl* impl, enum st_port port, struct rte_mbuf** rx_pkts,
                  uint16_t nb_pkts) {
  struct st_cni_impl* cni = st_get_cni(impl);
  struct rte_kni* rkni = cni->rkni[port];
  uint16_t port_id = st_port_id(impl, port);

  if (!cni->has_kni_kmod) return 0;

  rte_kni_handle_request(rkni);
  if (!rte_atomic32_read(&cni->if_up[port])) {
    // dbg("%s(%d), if not up\n", __func__, i);
    return -EBUSY;
  }

  /* burst to kni tx queue */
  rte_kni_tx_burst(rkni, rx_pkts, nb_pkts);

  /* rx from kni rx if */
  struct rte_mbuf* pkts_rx[ST_CNI_RX_BURST_SIZE];
  uint16_t rx = rte_kni_rx_burst(rkni, pkts_rx, ST_CNI_RX_BURST_SIZE);
  if (rx > 0) {
    cni->kni_rx_cnt[port] += rx;
    // dbg("%s(%d), rte_kni_rx_burst %d\n", __func__, port, rx);
    rte_eth_tx_burst(port_id, cni->tx_q_id[port], pkts_rx, rx);
    /* How to handle the pkts not bursted? */
  }

  return 0;
}

int st_kni_init(struct st_main_impl* impl) {
  int ret, i;
  int num_ports = st_num_ports(impl);
  struct st_cni_impl* cni = st_get_cni(impl);
  uint16_t port_id;

  ret = rte_kni_init(num_ports);
  if (ret < 0) {
    info("%s, rte_kni_init fail %d\n", __func__, ret);
    cni->has_kni_kmod = false;
    return 0;
  }

  cni->has_kni_kmod = true;
  rte_atomic32_set(&cni->stop_kni, 0);
  kni_set_global_impl(impl);

  ret = kni_queues_init(impl, cni);
  if (ret < 0) return ret;

  for (i = 0; i < num_ports; i++) {
    rte_atomic32_set(&cni->if_up[i], 0);
    port_id = st_port_id(impl, i);

    ret = kni_init_conf(port_id, &cni->conf[i]);
    if (ret < 0) {
      err("%s(%d), kni_init_conf fail %d\n", __func__, i, ret);
      st_kni_uinit(impl);
      return ret;
    }

    ret = kni_start_port(impl, i);
    if (ret < 0) {
      err("%s(%d), kni_start_port fail %d\n", __func__, i, ret);
      st_kni_uinit(impl);
      return ret;
    }
  }

  ret = pthread_create(&cni->kni_bkg_tid, NULL, kni_bkg_thread, impl);
  if (ret < 0) {
    st_kni_uinit(impl);
    err("%s, create kni_bkg thread fail\n", __func__);
    return ret;
  }

  return 0;
}

int st_kni_uinit(struct st_main_impl* impl) {
  struct st_cni_impl* cni = st_get_cni(impl);
  int num_ports = st_num_ports(impl), ret;
  struct rte_kni* rkni;

  if (!cni->has_kni_kmod) return 0;

  if (cni->kni_bkg_tid) {
    rte_atomic32_set(&cni->stop_kni, 1);
    pthread_join(cni->kni_bkg_tid, NULL);
    cni->kni_bkg_tid = 0;
  }

  for (int i = 0; i < num_ports; i++) {
    rkni = cni->rkni[i];
    if (rkni) {
      rte_kni_update_link(rkni, 0);
      ret = rte_kni_release(rkni);
      if (ret < 0) err("%s(%d), rte_kni_release fail %d\n", __func__, i, ret);
    }
  }

  kni_queues_uinit(impl);

  rte_kni_close();
  kni_set_global_impl(NULL);
  info("%s, succ\n", __func__);
  return 0;
}
