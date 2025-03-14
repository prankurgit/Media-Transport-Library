/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2022 Intel Corporation
 */

/**
 * @file mtl_api.h
 *
 * This header define the public interfaces of Intel® Media Transport Library.
 *
 */

#include <pthread.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#ifndef _MTL_API_HEAD_H_
#define _MTL_API_HEAD_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include "mtl_build_config.h"

/**
 * Compiler specific pack specifier
 */
#ifdef __GNUC__
#define MTL_PACK(__Declaration__) __Declaration__ __attribute__((__packed__))
#endif

#ifdef _MSC_VER
#define MTL_PACK(__Declaration__) \
  __pragma(pack(push, 1)) __Declaration__ __pragma(pack(pop))
#endif

/**
 * Macro to compute a version number usable for comparisons
 */
#define MTL_VERSION_NUM(a, b, c) ((a) << 16 | (b) << 8 | (c))
/**
 * All version numbers in one to compare with ST_VERSION_NUM()
 */
#define MTL_VERSION \
  MTL_VERSION_NUM(MTL_VERSION_MAJOR, MTL_VERSION_MINOR, MTL_VERSION_LAST)

/**
 * Get the uint64_t value for a specified bit set(0 to 63).
 */
#define MTL_BIT64(nr) (UINT64_C(1) << (nr))

/**
 * Get the uint32_t value for a specified bit set(0 to 31).
 */
#define MTL_BIT32(nr) (UINT32_C(1) << (nr))

/**
 * Max length of a DPDK port name and session logical port
 */
#define MTL_PORT_MAX_LEN (64)
/**
 * Length of a IPV4 address
 */
#define MTL_IP_ADDR_LEN (4)
/**
 * Length of a mac address
 */
#define MTL_MAC_ADDR_LEN (6)
/**
 * Defined if current platform is little endian
 */
#define MTL_LITTLE_ENDIAN /* x86 use little endian */

/** Standard mtu size is 1500 */
#define MTL_MTU_MAX_BYTES (1500)

/** Standard UDP is 1460 bytes, mtu is 1500 */
#define MTL_UDP_MAX_BYTES (1460)

/**
 * Max bytes in one RTP packet, include payload and header
 * standard UDP is 1460 bytes, and UDP headers are 8 bytes
 * leave 100 for network extra space
 */
#define MTL_PKT_MAX_RTP_BYTES (MTL_UDP_MAX_BYTES - 8 - 100)

/**
 * Max allowed number of dma devs
 */
#define MTL_DMA_DEV_MAX (8)

/**
 * Max length of a pcap dump filename
 */
#define MTL_PCAP_FILE_MAX_LEN (32)

/** Helper to get array size from arrays */
#define MTL_ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))

/** Helper to get M unit */
#define MTL_STAT_M_UNIT (1000 * 1000)

/**
 * Handle to MTL transport device context
 */
typedef struct mtl_main_impl* mtl_handle;
/**
 * Handle to st user dma device
 */
typedef struct mtl_dma_lender_dev* mtl_udma_handle;

/**
 * IO virtual address type.
 */
typedef uint64_t mtl_iova_t;

/**
 * Handle to dma mem
 */
typedef struct mtl_dma_mem* mtl_dma_mem_handle;

/**
 * Bad IOVA address
 */
#define MTL_BAD_IOVA ((mtl_iova_t)-1)

/**
 * Macro to align a value, align should be a power-of-two value.
 */
#define MTL_ALIGN(val, align) (((val) + ((align)-1)) & ~((align)-1))

#ifdef __MTL_LIB_BUILD__
#define __mtl_deprecated_msg(msg)
#else
/** Macro to mark functions and fields to be removal */
#define __mtl_deprecated_msg(msg) __attribute__((__deprecated__(msg)))
#endif

/**
 * Port logical type
 */
enum mtl_port {
  MTL_PORT_P = 0, /**< primary port */
  MTL_PORT_R,     /**< redundant port */
  MTL_PORT_2,     /**< port index: 2 */
  MTL_PORT_3,     /**< port index: 3 */
  MTL_PORT_4,     /**< port index: 4 */
  MTL_PORT_5,     /**< port index: 5 */
  MTL_PORT_6,     /**< port index: 6 */
  MTL_PORT_7,     /**< port index: 7 */
  MTL_PORT_MAX,   /**< max value of this enum */
};

/**
 * Session port logical type
 */
enum mtl_session_port {
  MTL_SESSION_PORT_P = 0, /**< primary session(logical) port */
  MTL_SESSION_PORT_R,     /**< redundant session(logical) port */
  MTL_SESSION_PORT_MAX,   /**< max value of this enum */
};

/**
 * Log level type to MTL context
 */
enum mtl_log_level {
  MTL_LOG_LEVEL_DEBUG = 0, /**< debug log level */
  MTL_LOG_LEVEL_INFO,      /**< info log level */
  MTL_LOG_LEVEL_NOTICE,    /**< notice log level */
  MTL_LOG_LEVEL_WARNING,   /**< warning log level */
  MTL_LOG_LEVEL_ERROR,     /**< error log level */
  MTL_LOG_LEVEL_MAX,       /**< max value of this enum */
};

/**
 * Poll mode driver type
 */
enum mtl_pmd_type {
  /** DPDK user driver PMD */
  MTL_PMD_DPDK_USER = 0,
  /** address family(kernel) high performance packet processing */
  MTL_PMD_DPDK_AF_XDP,
  /** max value of this enum */
  MTL_PMD_TYPE_MAX,
};

/**
 * RSS mode
 */
enum mtl_rss_mode {
  /** not using rss */
  MTL_RSS_MODE_NONE = 0,
  /** hash with both l3 src and dst, not use now */
  MTL_RSS_MODE_L3,
  /** hash with l3 src and dst address, l4 src port and dst port, used with shared rss */
  MTL_RSS_MODE_L3_L4,
  /** max value of this enum */
  MTL_RSS_MODE_MAX,
};

/**
 * IOVA mode
 */
enum mtl_iova_mode {
  /** let DPDK to choose IOVA mode */
  MTL_IOVA_MODE_AUTO = 0,
  /** using IOVA VA mode */
  MTL_IOVA_MODE_VA,
  /** using IOVA PA mode */
  MTL_IOVA_MODE_PA,
  /** max value of this enum */
  MTL_IOVA_MODE_MAX,
};

/**
 * Interface network protocol
 */
enum mtl_net_proto {
  /** using static IP configuration */
  MTL_PROTO_STATIC = 0,
  /** using DHCP(auto) IP configuration */
  MTL_PROTO_DHCP,
  /** max value of this enum */
  MTL_PROTO_MAX,
};

/**
 * Transport type
 */
enum mtl_transport_type {
  /** st2110 protocol transport */
  MTL_TRANSPORT_ST2110 = 0,
  /** udp transport */
  MTL_TRANSPORT_UDP,
  /** max value of this enum */
  MTL_TRANSPORT_TYPE_MAX,
};

/**
 * SIMD level type
 */
enum mtl_simd_level {
  MTL_SIMD_LEVEL_NONE = 0,     /**< Scalar */
  MTL_SIMD_LEVEL_AVX2,         /**< AVX2 */
  MTL_SIMD_LEVEL_AVX512,       /**< AVX512 */
  MTL_SIMD_LEVEL_AVX512_VBMI2, /**< AVX512 VBMI2 */
  MTL_SIMD_LEVEL_MAX,          /**< max value of this enum */
};

/**
 * st21 tx pacing way
 */
enum st21_tx_pacing_way {
  /** auto detected pacing */
  ST21_TX_PACING_WAY_AUTO = 0,
  /** rate limit based pacing */
  ST21_TX_PACING_WAY_RL,
  /** tsc based pacing */
  ST21_TX_PACING_WAY_TSC,
  /** tsn based pacing */
  ST21_TX_PACING_WAY_TSN,
  /** ptp based pacing */
  ST21_TX_PACING_WAY_PTP,
  /** best effort sending */
  ST21_TX_PACING_WAY_BE,
  /** tsc based pacing with single bulk transmitter */
  ST21_TX_PACING_WAY_TSC_NARROW,
  /** Max value of this enum */
  ST21_TX_PACING_WAY_MAX,
};

/**
 * Flag bit in flags of struct mtl_init_params.
 * If set, lib will call numa_bind to bind app thread and memory to NIC socket also.
 */
#define MTL_FLAG_BIND_NUMA (MTL_BIT64(0))
/**
 * Flag bit in flags of struct mtl_init_params.
 * Enable built-in PTP implementation, only for PF now.
 * If not enable, it will use system time as the PTP source.
 */
#define MTL_FLAG_PTP_ENABLE (MTL_BIT64(1))
/**
 * Flag bit in flags of struct mtl_init_params.
 * Separated lcore for RX video(st2110-20/st2110-22) session.
 */
#define MTL_FLAG_RX_SEPARATE_VIDEO_LCORE (MTL_BIT64(2))
/**
 * Flag bit in flags of struct mtl_init_params.
 * Enable migrate mode for tx video session if current LCORE is too busy to handle the
 * tx video tasklet, the busy session may be migrated to a new LCORE.
 * If not enable, tx video will always use static mapping based on quota.
 */
#define MTL_FLAG_TX_VIDEO_MIGRATE (MTL_BIT64(3))
/**
 * Flag bit in flags of struct mtl_init_params.
 * Enable migrate mode for rx video session if current LCORE is too busy to handle the
 * rx video tasklet, the busy session may be migrated to a new LCORE.
 * If not enable, rx video will always use static mapping based on quota.
 */
#define MTL_FLAG_RX_VIDEO_MIGRATE (MTL_BIT64(4))
/**
 * Flag bit in flags of struct mtl_init_params.
 * Run the tasklet inside one thread instead of a pinned lcore.
 */
#define MTL_FLAG_TASKLET_THREAD (MTL_BIT64(5))
/**
 * Flag bit in flags of struct mtl_init_params.
 * Enable the tasklet sleep if routine report task done.
 */
#define MTL_FLAG_TASKLET_SLEEP (MTL_BIT64(6))
/**
 * Flag bit in flags of struct mtl_init_params.
 * Set the supported SIMD bitwidth of rx/tx burst to 512 bit(AVX512).
 */
#define MTL_FLAG_RXTX_SIMD_512 (MTL_BIT64(7))
/**
 * Flag bit in flags of struct mtl_init_params.
 * Use PI controller for built-in PTP implementation, only for PF now.
 */
#define MTL_FLAG_PTP_PI (MTL_BIT64(9))
/**
 * Flag bit in flags of struct mtl_init_params.
 * Enable background lcore mode for MTL_TRANSPORT_UDP.
 */
#define MTL_FLAG_UDP_LCORE (MTL_BIT64(10))
/**
 * Flag bit in flags of struct mtl_init_params.
 * Enable random source port for MTL_TRANSPORT_ST2110 tx.
 */
#define MTL_FLAG_RANDOM_SRC_PORT (MTL_BIT64(11))
/**
 * Flag bit in flags of struct mtl_init_params.
 * Enable multiple source port for MTL_TRANSPORT_ST2110 20 tx.
 */
#define MTL_FLAG_MULTI_SRC_PORT (MTL_BIT64(12))
/**
 * Flag bit in flags of struct mtl_init_params.
 * Enable shared queue for tx.
 */
#define MTL_FLAG_SHARED_TX_QUEUE (MTL_BIT64(13))
/**
 * Flag bit in flags of struct mtl_init_params.
 * Enable shared queue for rx.
 */
#define MTL_FLAG_SHARED_RX_QUEUE (MTL_BIT64(14))

/**
 * Flag bit in flags of struct mtl_init_params, debug usage only.
 * dedicate thread for cni message
 */
#define MTL_FLAG_CNI_THREAD (MTL_BIT64(32))
/**
 * Flag bit in flags of struct mtl_init_params, debug usage only.
 * Enable video rx ebu check
 */
#define MTL_FLAG_RX_VIDEO_EBU (MTL_BIT64(33))
/**
 * Flag bit in flags of struct mtl_init_params, debug usage only.
 * Enable NIC promiscuous mode for RX
 */
#define MTL_FLAG_NIC_RX_PROMISCUOUS (MTL_BIT64(34))
/**
 * Flag bit in flags of struct mtl_init_params, debug usage only.
 * use unicast address for ptp PTP_DELAY_REQ message
 */
#define MTL_FLAG_PTP_UNICAST_ADDR (MTL_BIT64(35))
/**
 * Flag bit in flags of struct mtl_init_params, debug usage only.
 * Mono memory pool for all rx queue(sessions)
 */
#define MTL_FLAG_RX_MONO_POOL (MTL_BIT64(36))
/**
 * Flag bit in flags of struct mtl_init_params, debug usage only.
 * Do mtl_start in mtl_init, mtl_stop in mtl_uninit, and skip the mtl_start/mtl_stop
 */
#define MTL_FLAG_DEV_AUTO_START_STOP (MTL_BIT64(37))
/**
 * Flag bit in flags of struct mtl_init_params, debug usage only.
 * Enable tasklet time measurement, report status if tasklet run time longer than
 * tasklet_time_thresh_us in mtl_init_params.
 */
#define MTL_FLAG_TASKLET_TIME_MEASURE (MTL_BIT64(38))
/**
 * Flag bit in flags of struct mtl_init_params, debug usage only.
 * Disable the zero copy for af_xdp tx video session
 */
#define MTL_FLAG_AF_XDP_ZC_DISABLE (MTL_BIT64(39))
/**
 * Flag bit in flags of struct mtl_init_params, debug usage only.
 * Mono memory pool for all tx queue(session)
 */
#define MTL_FLAG_TX_MONO_POOL (MTL_BIT64(40))
/**
 * Flag bit in flags of struct mtl_init_params, debug usage only.
 * Disable system rx queues, pls use mcast or manual TX mac.
 */
#define MTL_FLAG_DISABLE_SYSTEM_RX_QUEUES (MTL_BIT64(41))
/**
 * Flag bit in flags of struct mtl_init_params, debug usage only.
 * Force to get ptp time from tsc source.
 */
#define MTL_FLAG_PTP_SOURCE_TSC (MTL_BIT64(42))
/**
 * Flag bit in flags of struct mtl_init_params, debug usage only.
 * Disable TX chain mbuf, use same mbuf for header and payload.
 * Will do memcpy from framebuffer to packet payload.
 */
#define MTL_FLAG_TX_NO_CHAIN (MTL_BIT64(43))
/**
 * Flag bit in flags of struct mtl_init_params, debug usage only.
 * Disable the pkt check for TX burst API.
 */
#define MTL_FLAG_TX_NO_BURST_CHK (MTL_BIT64(44))
/**
 * Flag bit in flags of struct mtl_init_params, debug usage only.
 * Use CNI based queue for RX.
 */
#define MTL_FLAG_RX_USE_CNI (MTL_BIT64(45))
/**
 * Flag bit in flags of struct mtl_init_params.
 * Enable built-in PHC2SYS implementation.
 */
#define MTL_FLAG_PHC2SYS_ENABLE (MTL_BIT64(46))

/**
 * The structure describing how to init af_xdp interface.
 * See https://doc.dpdk.org/guides/nics/af_xdp.html for detail.
 */
struct mtl_af_xdp_params {
  /** starting netdev queue id, must > 0, 0 is reserved for system usage */
  uint8_t start_queue;
  /** total netdev queue number, must > 0 */
  uint8_t queue_count;
};

/**
 * The structure describing how to init the mtl context.
 * Include the PCIE port and other required info.
 */
struct mtl_init_params {
  /* below are mandatory parameters */
  /** Pcie BDF(ex: 0000:af:00.0) or enp175s0f0(MTL_PMD_DPDK_AF_XDP) */
  char port[MTL_PORT_MAX][MTL_PORT_MAX_LEN];
  /** number of pcie ports, 1 to MTL_PORT_MAX_LEN, mandatory */
  uint8_t num_ports;
  /** bound IP of ports, for MTL_PMD_DPDK_USER
   * This is not used when DHCP enabled, otherwise set the valid value.
   */
  uint8_t sip_addr[MTL_PORT_MAX][MTL_IP_ADDR_LEN];
  /** log level */
  enum mtl_log_level log_level;

  /* below are optional parameters */
  /** transport type, st2110 or udp */
  enum mtl_transport_type transport;
  /**
   * net mask of ports, for MTL_PMD_DPDK_USER.
   * This is not used when DHCP enabled, otherwise set the valid value.
   * Lib will use 255.255.255.0 if this value is blank
   */
  uint8_t netmask[MTL_PORT_MAX][MTL_IP_ADDR_LEN];
  /**
   * default gateway of ports, for MTL_PMD_DPDK_USER.
   * This is not used when DHCP enabled, otherwise set the valid value.
   * User can use "route -n" to get gateway before bind the port to DPDK PMD.
   * For MTL_PMD_DPDK_AF_XDP, lib will try to fetch gateway by route command
   * if this value is not assigned.
   */
  uint8_t gateway[MTL_PORT_MAX][MTL_IP_ADDR_LEN];
  /**
   * deprecated for MTL_TRANSPORT_ST2110.
   * max tx sessions(st20, st22, st30, st40) requested the lib to support,
   * use mtl_get_fix_info to query the actual count.
   * dpdk context will allocate the hw resources(queues, memory) based on this number.
   */
  uint16_t tx_sessions_cnt_max __mtl_deprecated_msg("Use tx_queues_cnt instead");
  /**
   * deprecated for MTL_TRANSPORT_ST2110.
   * max rx sessions(st20, st22, st30, st40) requested the lib to support,
   * use mtl_get_fix_info to query the actual count.
   * dpdk context will allocate the hw resources(queues, memory) based on this number.
   */
  uint16_t rx_sessions_cnt_max __mtl_deprecated_msg("Use rx_queues_cnt instead");
  /**
   * max tx user queues requested the lib to support.
   * for MTL_TRANSPORT_ST2110, use st_tx_sessions_queue_cnt to query.
   */
  uint16_t tx_queues_cnt[MTL_PORT_MAX];
  /**
   * max rx user queues requested the lib to support.
   * for MTL_TRANSPORT_ST2110, use st_rx_sessions_queue_cnt to query.
   */
  uint16_t rx_queues_cnt[MTL_PORT_MAX];

  /** dpdk user pmd or af_xdp */
  enum mtl_pmd_type pmd[MTL_PORT_MAX];
  /**
   * af_xdp port info, mandatory for MTL_PMD_DPDK_AF_XDP.
   * MTL_PMD_DPDK_AF_XDP will use the IP of kernel itself.
   */
  struct mtl_af_xdp_params xdp_info[MTL_PORT_MAX];
  /**
   * logical cores list can be used, e.g. "28,29,30,31".
   * NULL means determined by system itself
   */
  char* lcores;
  /** dma(CBDMA or DSA) dev Pcie BDF path like 0000:80:04.0 */
  char dma_dev_port[MTL_DMA_DEV_MAX][MTL_PORT_MAX_LEN];
  /** number of dma dev ports in dma_dev_port, leave to zero if no dma dev */
  uint8_t num_dma_dev_port;
  /** flags, value in MTL_FLAG_* */
  uint64_t flags;
  /** private data to the callback function */
  void* priv;
  /**
   * Function to acquire current ptp time(in nanoseconds) from user.
   * if NULL, ST instance will get from built-in ptp source(NIC) or system time instead.
   */
  uint64_t (*ptp_get_time_fn)(void* priv);
  /** stats dump period in seconds, 0 means determined by lib */
  uint16_t dump_period_s;
  /** stats dump callback in every dump_period_s */
  void (*stat_dump_cb_fn)(void* priv);
  /** data quota for each lcore, 0 means determined by lib */
  uint32_t data_quota_mbs_per_sch;
  /** the number of tasklets for each lcore, 0 means determined by lib */
  uint32_t tasklets_nb_per_sch;
  /** the number of max tx audio session for each lcore, 0 means determined by lib */
  uint32_t tx_audio_sessions_max_per_sch;
  /** the number of max rx audio session for each lcore, 0 means determined by lib */
  uint32_t rx_audio_sessions_max_per_sch;
  /**
   * number of transmit descriptors for each NIC TX queue, 0 means determined by lib.
   * It will affect the memory usage and the performance.
   */
  uint16_t nb_tx_desc;
  /**
   * number of receive descriptors for each NIC RX queue, 0 means determined by lib.
   * It will affect the memory usage and the performance.
   */
  uint16_t nb_rx_desc;
  /**
   * Suggest max allowed udp size for each network pkt, leave to zero if you don't known.
   */
  uint16_t pkt_udp_suggest_max_size;
  /**
   * The number for hdr split queues of rx, should smaller than rx_sessions_cnt_max.
   * Experimental feature.
   */
  uint16_t nb_rx_hdr_split_queues;
  /**
   * Suggest data room size for rx mempool,
   * the final data room size may be aligned to larger value,
   * some NICs may need this to avoid mbuf split.
   */
  uint16_t rx_pool_data_size;
  /**
   * The st21 tx pacing way, leave to zero(auto) if you don't known the detail.
   */
  enum st21_tx_pacing_way pacing;
  /**
   * The ptp pi controller proportional gain.
   */
  double kp;
  /**
   * The ptp pi controller integral gain.
   */
  double ki;
  /**
   * Suggest using rss (L3 or L4) for rx packets direction.
   */
  enum mtl_rss_mode rss_mode;
  /**
   * Select default or force IOVA mode.
   */
  enum mtl_iova_mode iova_mode;
  /**
   * Interface network protocol
   * static or DHCP
   */
  enum mtl_net_proto net_proto[MTL_PORT_MAX];
};

/**
 * A structure used to retrieve fixed info for an MTL instance.
 */
struct mtl_fix_info {
  /** the flags in mtl_init_params */
  uint64_t init_flags;
  /** number of pcie ports */
  uint8_t num_ports;
  /** max dma dev count for current transport context */
  uint8_t dma_dev_cnt_max;
};

/**
 * A structure used to retrieve varied info for an MTL instance.
 */
struct mtl_var_info {
  /** active scheduler count */
  uint8_t sch_cnt;
  /** active lcore count */
  uint8_t lcore_cnt;
  /** active dma dev count for current transport context */
  uint8_t dma_dev_cnt;
  /** if transport device is started(mtl_start) */
  bool dev_started;
};

/**
 * A structure used to retrieve general statistics(I/O) for a MTL port.
 */
struct mtl_port_status {
  /** Total number of received packets. */
  uint64_t rx_packets;
  /** Total number of transmitted packets. */
  uint64_t tx_packets;
  /** Total number of received bytes. */
  uint64_t rx_bytes;
  /** Total number of transmitted bytes. */
  uint64_t tx_bytes;
  /** Total number of failed received packets. */
  uint64_t rx_err_packets;
  /** Total number of received packets dropped by the HW. (i.e. Rx queues are full) */
  uint64_t rx_hw_dropped_packets;
  /** Total number of Rx mbuf allocation failures. */
  uint64_t rx_nombuf_packets;
  /** Total number of failed transmitted packets. */
  uint64_t tx_err_packets;
};

/**
 * Retrieve the fixed information of an MTL instance.
 *
 * @param mt
 *   The handle to MTL instance.
 * @param info
 *   A pointer to info structure.
 * @return
 *   - 0 if successful.
 *   - <0: Error code if fail.
 */
int mtl_get_fix_info(mtl_handle mt, struct mtl_fix_info* info);

/**
 * Retrieve the varied information of an MTL instance.
 *
 * @param mt
 *   The handle to MTL instance.
 * @param info
 *   A pointer to info structure.
 * @return
 *   - 0 if successful.
 *   - <0: Error code if fail.
 */
int mtl_get_var_info(mtl_handle mt, struct mtl_var_info* info);

/**
 * Retrieve the general statistics(I/O) for a MTL port.
 *
 * @param mt
 *   The handle to MTL instance.
 * @param port
 *   The port index.
 * @param stats
 *   A pointer to stats structure.
 * @return
 *   - 0 if successful.
 *   - <0: Error code if fail.
 */
int mtl_get_port_stats(mtl_handle mt, enum mtl_port port, struct mtl_port_status* stats);

/**
 * Reset the general statistics(I/O) for a MTL port.
 *
 * @param mt
 *   The handle to MTL instance.
 * @param port
 *   The port index.
 * @return
 *   - 0 if successful.
 *   - <0: Error code if fail.
 */
int mtl_reset_port_stats(mtl_handle mt, enum mtl_port port);

/**
 * Inline function returning primary port pointer from mtl_init_params
 * @param p
 *   The pointer to the init parameters.
 * @return
 *     Primary port name pointer
 */
static inline char* mtl_p_port(struct mtl_init_params* p) { return p->port[MTL_PORT_P]; }

/**
 * Inline function returning redundant port pointer from mtl_init_params
 * @param p
 *   The pointer to the init parameters.
 * @return
 *     Redundant port name pointer
 */
static inline char* mtl_r_port(struct mtl_init_params* p) { return p->port[MTL_PORT_R]; }

/**
 * Inline helper function returning primary port source IP address pointer
 * from mtl_init_params
 * @param p
 *   The pointer to the init parameters.
 * @return
 *     Primary port IP address pointer
 */
static inline uint8_t* mtl_p_sip_addr(struct mtl_init_params* p) {
  return p->sip_addr[MTL_PORT_P];
}

/**
 * Inline helper function returning redundant port source IP address pointer
 * from mtl_init_params
 * @param p
 *   The pointer to the init parameters.
 * @return
 *     Redundant port IP address pointer
 */
static inline uint8_t* mtl_r_sip_addr(struct mtl_init_params* p) {
  return p->sip_addr[MTL_PORT_R];
}

/**
 * Function returning version string
 * @return
 *     ST version string
 */
const char* mtl_version(void);

/**
 * Initialize the MTL transport device context which based on DPDK.
 *
 * @param p
 *   The pointer to the init parameters.
 * @return
 *   - NULL on error.
 *   - Otherwise, the handle to the MTL transport device context.
 */
mtl_handle mtl_init(struct mtl_init_params* p);

/**
 * Un-initialize the MTL transport device context.
 *
 * @param mt
 *   The handle to the MTL transport device context.
 * @return
 *   - 0: Success, device un-initialized.
 *   - <0: Error code of the device un-initialize.
 */
int mtl_uninit(mtl_handle mt);

/**
 * Start the MTL transport device context.
 *
 * @param mt
 *   The handle to the MTL transport device context.
 * @return
 *   - 0: Success, device started.
 *   - <0: Error code of the device start.
 */
int mtl_start(mtl_handle mt);

/**
 * Stop the MTL transport device context.
 *
 * @param mt
 *   The handle to the MTL transport device context.
 * @return
 *   - 0: Success, device stopped.
 *   - <0: Error code of the device stop.
 */
int mtl_stop(mtl_handle mt);

/**
 * Abort the MTL transport device context.
 * Usually called in the exception case, e.g CTRL-C.
 *
 * @param mt
 *   The handle to the MTL transport device context.
 * @return
 *   - 0: Success, device aborted.
 *   - <0: Error code of the device abort.
 */
int mtl_abort(mtl_handle mt);

/**
 * Enable or disable sleep mode for sch.
 *
 * @param mt
 *   The handle to the MTL transport device context.
 * @param sch_idx
 *   The sch index, get from st20_tx_get_sch_idx or st20_rx_get_sch_idx.
 * @param enable
 *   enable or not.
 * @return
 *   - 0: Success.
 *   - <0: Error code.
 */
int mtl_sch_enable_sleep(mtl_handle mt, int sch_idx, bool enable);

/**
 * Set the sleep us for the sch if MTL_FLAG_TASKLET_SLEEP is enabled.
 * Debug usage only.
 *
 * @param mt
 *   The handle to the MTL transport device context.
 * @param us
 *   The max sleep us.
 * @return
 *   - 0: Success.
 *   - <0: Error code.
 */
int mtl_sch_set_sleep_us(mtl_handle mt, uint64_t us);

/**
 * Request one DPDK lcore from the MTL transport device context.
 *
 * @param mt
 *   The handle to the MTL transport device context.
 * @param lcore
 *   A pointer to the returned lcore number.
 * @return
 *   - 0 if successful.
 *   - <0: Error code if fail.
 */
int mtl_get_lcore(mtl_handle mt, unsigned int* lcore);

/**
 * Bind one thread to lcore.
 *
 * @param mt
 *   The handle to the MTL transport device context.
 * @param thread
 *   the thread which request the bind action.
 * @param lcore
 *   the DPDK lcore which requested by mtl_get_lcore.
 * @return
 *   - 0 if successful.
 *   - <0: Error code if fail.
 */
int mtl_bind_to_lcore(mtl_handle mt, pthread_t thread, unsigned int lcore);

/**
 * Put back the DPDK lcore which requested from the MTL transport device context.
 *
 * @param mt
 *   The handle to the MTL transport device context.
 * @param lcore
 *   the DPDK lcore which requested by mtl_get_lcore.
 * @return
 *   - 0 if successful.
 *   - <0: Error code if fail.
 */
int mtl_put_lcore(mtl_handle mt, unsigned int lcore);

/**
 * Performance optimized memcpy, e.g. AVX-512.
 *
 * @param dest
 *   Pointer to the destination of the data.
 * @param src
 *   Pointer to the source data.
 * @param n
 *   Number of bytes to copy..
 * @return
 *   - Pointer to the destination data.
 */
void* mtl_memcpy(void* dest, const void* src, size_t n);

/**
 * Read current time from ptp source.
 *
 * @param mt
 *   The handle to the MTL transport device context.
 * @return
 *   - The time in nanoseconds in current ptp system
 */
uint64_t mtl_ptp_read_time(mtl_handle mt);

/**
 * Allocate memory from the huge-page area of memory. The memory is not cleared.
 * In NUMA systems, the memory allocated from the same NUMA socket of the port.
 * Note the memory is mmap to IOVA already, use mtl_hp_virt2iova to get the iova.
 *
 * @param mt
 *   The handle to the MTL transport device context.
 * @param size
 *   Size (in bytes) to be allocated.
 * @param port
 *   Port for the memory to be allocated.
 * @return
 *   - NULL on error. Not enough memory, or invalid arguments
 *   - Otherwise, the pointer to the allocated memory.
 */
void* mtl_hp_malloc(mtl_handle mt, size_t size, enum mtl_port port);

/**
 * Allocate zero'ed memory from the huge-page area of memory.
 * Equivalent to mtl_hp_malloc() except that the memory zone is cleared with zero.
 * In NUMA systems, the memory allocated from the same NUMA socket of the port.
 * Note the memory is mmap to IOVA already, use mtl_hp_virt2iova to get the iova.
 *
 * @param mt
 *   The handle to the MTL transport device context.
 * @param size
 *   Size (in bytes) to be allocated.
 * @param port
 *   Port for the memory to be allocated.
 * @return
 *   - NULL on error. Not enough memory, or invalid arguments
 *   - Otherwise, the virtual address pointer to the allocated memory.
 */
void* mtl_hp_zmalloc(mtl_handle mt, size_t size, enum mtl_port port);

/**
 * Frees the memory pointed by the pointer.
 *
 * This pointer must have been returned by a previous call to
 * mtl_hp_malloc(), mtl_hp_zmalloc().
 * The behaviour is undefined if the pointer does not match this requirement.
 *
 * @param mt
 *   The handle to the MTL transport device context.
 * @param ptr
 *   The virtual address pointer to memory to be freed.
 */
void mtl_hp_free(mtl_handle mt, void* ptr);

/**
 * Return the IO address of a virtual address from mtl_hp_malloc/mtl_hp_zmalloc
 *
 * @param mt
 *   The handle to the MTL transport device context.
 * @param vaddr
 *   Virtual address obtained from previous mtl_hp_malloc/mtl_hp_zmalloc call
 * @return
 *   MTL_BAD_IOVA on error
 *   otherwise return an address suitable for IO
 */
mtl_iova_t mtl_hp_virt2iova(mtl_handle mt, const void* vaddr);

/**
 * Return the detected page size on the system.
 *
 * @param mt
 *   The handle to the MTL transport device context.
 * @return
 *   page size
 */
size_t mtl_page_size(mtl_handle mt);

/**
 * Perform DMA mapping with virtual address that can be used for IO.
 * The virtual address and size must align to page size(mtl_page_size).
 *
 * @param mt
 *   The handle to the MTL transport device context.
 * @param vaddr
 *   Virtual address of memory to be mapped and must align to page size.
 * @param size
 *   Length of memory segment being mapped.
 *
 * @return
 *   MTL_BAD_IOVA on error
 *   otherwise return an address suitable for IO
 */
mtl_iova_t mtl_dma_map(mtl_handle mt, const void* vaddr, size_t size);

/**
 * Perform DMA unmapping on the mtl_dma_map
 *
 * @param mt
 *   The handle to the MTL transport device context.
 * @param vaddr
 *   Virtual address of memory to be unmapped and must align to page size.
 * @param iova
 *   iova address
 * @param size
 *   Length of memory segment being unmapped.
 *
 * @return
 *   - 0 if successful.
 *   - <0: Error code if fail.
 */
int mtl_dma_unmap(mtl_handle mt, const void* vaddr, mtl_iova_t iova, size_t size);

/**
 * Allocate memory block more than required and map valid data to IOVA.
 *
 * The memory malloc layout:
 *
 * |___________|/////////////// valid ////////////////|____|___|
 *
 * |___________|<--------------- size --------------->|____|___|
 *
 * |___________|<---------------- iova_size -------------->|___|
 *
 * |<--------------- alloc_size (pgsz multiple)--------------->|
 *
 * *alloc_addr *addr(page aligned)
 *
 * @param mt
 *   The handle to the MTL transport device context.
 * @param size
 *   Size of valid data.
 * @return
 *   - NULL on error.
 *   - Otherwise, the handle to the dma mem.
 */
mtl_dma_mem_handle mtl_dma_mem_alloc(mtl_handle mt, size_t size);

/**
 * Free the dma mem memory block.
 * This will use memset to clear the st dma mem struct.
 *
 * @param mt
 *   The handle to the MTL transport device context.
 * @param handle
 *   The handle to the st dma mem.
 */
void mtl_dma_mem_free(mtl_handle mt, mtl_dma_mem_handle handle);

/**
 * Get the begin address of dma mapped memory.
 *
 * @param handle
 *   The handle to the st dma mem.
 * @return
 *   - Begin address of dma mapped memory.
 */
void* mtl_dma_mem_addr(mtl_dma_mem_handle handle);

/**
 * Get the begin IOVA of dma mapped memory.
 *
 * @param handle
 *   The handle to the st dma mem.
 * @return
 *   - Begin IOVA of dma mapped memory.
 */
mtl_iova_t mtl_dma_mem_iova(mtl_dma_mem_handle handle);

/**
 * Allocate a user DMA dev from the dma_dev_port(mtl_init_params) list.
 * In NUMA systems, the dma dev allocated from the same NUMA socket of the port.
 *
 * @param mt
 *   The handle to the MTL transport device context.
 * @param nb_desc
 *   Number of descriptor for the user DMA device
 * @param port
 *   Port for the user DMA device to be allocated.
 * @return
 *   - NULL on error.
 *   - Otherwise, the handle to the st user dma dev.
 */
mtl_udma_handle mtl_udma_create(mtl_handle mt, uint16_t nb_desc, enum mtl_port port);

/**
 * Free the st user dma dev.
 *
 * @param handle
 *   The handle to the st user dma dev.
 * @return
 *   - 0: Success.
 *   - <0: Error code of the free.
 */
int mtl_udma_free(mtl_udma_handle handle);

/**
 * Enqueue a copy operation onto the user dma dev.
 *
 * @param handle
 *   The handle to the st user dma dev.
 * @param dst
 *   The mtl_iova_t address of the destination buffer.
 *   Must be the memory address by mtl_hp_virt2iova.
 * @param src
 *   The mtl_iova_t address of the source buffer.
 *   Must be the memory address by mtl_hp_virt2iova.
 * @param length
 *   The length of the data to be copied.
 *
 * @return
 *   - 0..UINT16_MAX: index of enqueued job.
 *   - -ENOSPC: if no space left to enqueue.
 *   - other values < 0 on failure.
 */
int mtl_udma_copy(mtl_udma_handle handle, mtl_iova_t dst, mtl_iova_t src,
                  uint32_t length);

/**
 * Enqueue a fill operation onto the virtual DMA channel.
 *
 * @param handle
 *   The handle to the st user dma dev.
 * @param dst
 *   The mtl_iova_t address of the destination buffer.
 *   Must be the memory address by mtl_hp_virt2iova.
 * @param pattern
 *   The pattern(u64) to populate the destination buffer with.
 * @param length
 *   The length of the data to be copied.
 *
 * @return
 *   - 0..UINT16_MAX: index of enqueued job.
 *   - -ENOSPC: if no space left to enqueue.
 *   - other values < 0 on failure.
 */
int mtl_udma_fill(mtl_udma_handle handle, mtl_iova_t dst, uint64_t pattern,
                  uint32_t length);

/**
 * Enqueue a fill operation onto the virtual DMA channel.
 *
 * @param handle
 *   The handle to the st user dma dev.
 * @param dst
 *   The mtl_iova_t address of the destination buffer.
 *   Must be the memory address by mtl_hp_virt2iova.
 * @param pattern
 *   The pattern(u8) to populate the destination buffer with.
 * @param length
 *   The length of the data to be copied.
 *
 * @return
 *   - 0..UINT16_MAX: index of enqueued job.
 *   - -ENOSPC: if no space left to enqueue.
 *   - other values < 0 on failure.
 */
static inline int mtl_udma_fill_u8(mtl_udma_handle handle, mtl_iova_t dst,
                                   uint8_t pattern, uint32_t length) {
  uint64_t pattern_u64;
  /* pattern to u64 */
  memset(&pattern_u64, pattern, sizeof(pattern_u64));
  return mtl_udma_fill(handle, dst, pattern_u64, length);
}

/**
 * Trigger hardware to begin performing enqueued operations.
 *
 * @param handle
 *   The handle to the st user dma dev.
 * @return
 *   - 0: Success.
 *   - <0: Error code of the submit.
 */
int mtl_udma_submit(mtl_udma_handle handle);

/**
 * Return the number of operations that have been successfully completed.
 *
 * @param handle
 *   The handle to the st user dma dev.
 * @param nb_cpls
 *   The maximum number of completed operations that can be processed.
 *
 * @return
 *   The number of operations that successfully completed. This return value
 *   must be less than or equal to the value of nb_cpls.
 */
uint16_t mtl_udma_completed(mtl_udma_handle handle, const uint16_t nb_cpls);

/**
 * Get the rss mode.
 *
 * @param mt
 *   The handle to the MTL transport device context.
 * @return
 *   - enum mtl_rss_mode.
 */
enum mtl_rss_mode mtl_rss_mode_get(mtl_handle mt);

/**
 * Get the iova mode.
 *
 * @param mt
 *   The handle to the MTL transport device context.
 * @return
 *   - enum mtl_iova_mode.
 */
enum mtl_iova_mode mtl_iova_mode_get(mtl_handle mt);

/**
 * Get the ip info(address, netmask, gateway) for one mtl port.
 *
 * @param handle
 *   The handle to the st user dma dev.
 * @param ip
 *   The buffer for IP address.
 * @param netmask
 *   The buffer for netmask address.
 * @param gateway
 *   The buffer for gateway address.
 * @return
 *   - 0: Success.
 *   - <0: Error code.
 */
int mtl_port_ip_info(mtl_handle mt, enum mtl_port port, uint8_t ip[MTL_IP_ADDR_LEN],
                     uint8_t netmask[MTL_IP_ADDR_LEN], uint8_t gateway[MTL_IP_ADDR_LEN]);

/**
 * Get SIMD level current cpu supported.
 *
 * @return
 *   - The simd level
 */
enum mtl_simd_level mtl_get_simd_level(void);

/**
 * Get name of CPU simd level
 *
 * @param level
 *     The simd level
 * @return
 *     simd level name
 */
const char* mtl_get_simd_level_name(enum mtl_simd_level level);

/**
 * Helper function to get pmd type by port name.
 *
 * @param port
 *   port name.
 * @return
 *   pmd type.
 */
enum mtl_pmd_type mtl_pmd_by_port_name(const char* port);

/**
 * Helper function to get ip for interface.
 *
 * @param if_name
 *   if name.
 * @param ip
 *   point to IP address.
 * @return
 *   - 0: Success.
 *   - <0: Error code.
 */
int mtl_get_if_ip(char* if_name, uint8_t ip[MTL_IP_ADDR_LEN],
                  uint8_t netmask[MTL_IP_ADDR_LEN]);

/**
 * Change the stream that will be used by the logging system.
 *
 * The FILE* f argument represents the stream to be used for logging.
 * If f is NULL, the default output is used. This can be done at any time.
 *
 * @param f
 *   Pointer to the FILE stream.
 * @return
 *   - 0: Success.
 *   - <0: Error code.
 */
int mtl_openlog_stream(FILE* f);

/**
 * Helper function which align a size with pages
 *
 * @param sz
 *   Input fb size.
 * @param pg_sz
 *   Page size want to aligned.
 * @return
 *     The aligned size.
 */
static inline size_t mtl_size_page_align(size_t sz, size_t pg_sz) {
  if (sz % pg_sz) sz += pg_sz - (sz % pg_sz);
  return sz;
}

#if defined(__cplusplus)
}
#endif

#endif
