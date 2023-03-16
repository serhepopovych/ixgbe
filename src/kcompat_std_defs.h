/* SPDX-License-Identifier: GPL-2.0-only */
/* Copyright (C) 1999 - 2023 Intel Corporation */

#ifndef _KCOMPAT_STD_DEFS_H_
#define _KCOMPAT_STD_DEFS_H_

/* This file contains the definitions for what kernel features need backports
 * for a given kernel. It targets only the standard stable kernel releases.
 * It must check only LINUX_VERSION_CODE and assume the kernel is a standard
 * release, and not a custom distribution.
 *
 * It must define HAVE_<FLAG> and NEED_<FLAG> for features. It must not
 * implement any backports, instead leaving the implementation to the
 * kcompat_impl.h header.
 *
 * If a feature can be easily implemented as a replacement macro or fully
 * backported, use a NEED_<FLAG> to indicate that the feature needs
 * a backport. (If NEED_<FLAG> is undefined, then no backport for that feature
 * is needed).
 *
 * If a feature cannot be easily implemented in kcompat directly, but
 * requires drivers to make specific changes such as stripping out an entire
 * feature or modifying a function pointer prototype, use a HAVE_<FLAG>.
 */

#ifndef LINUX_VERSION_CODE
#error "LINUX_VERSION_CODE is undefined"
#endif

#ifndef KERNEL_VERSION
#error "KERNEL_VERSION is undefined"
#endif

/*****************************************************************************/
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0))
#else /* >= 3,10,0 */
#define NEED_NETIF_NAPI_ADD_NO_WEIGHT
#define NEED_ETHTOOL_SPRINTF
#endif /* 3,10,0 */

/*****************************************************************************/
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,17,0))
#define NEED_DEVM_KASPRINTF
#else /* >= 3,17,0 */
#endif /* 3,17,0 */

/*****************************************************************************/
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,18,0))
#define NEED_DEV_PM_DOMAIN_ATTACH_DETACH
#else /* >= 3,18,0 */
#endif /* 3,18,0 */

/*****************************************************************************/
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,19,0))
#define NEED_DEV_PRINTK_ONCE
#else /* >= 3,19,0 */
#endif /* 3,19,0 */

/*****************************************************************************/
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4,3,0))
#define NEED_DEFINE_STATIC_KEY_FALSE
#define NEED_STATIC_BRANCH
#else /* >= 4,3,0 */
#define NEED_DECLARE_STATIC_KEY_FALSE
#endif /* 4,3,0 */

/*****************************************************************************/
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4,6,0))
#else /* >= 4,6,0 */
#define HAVE_DEVLINK_PORT_SPLIT
#endif /* 4,6,0 */

/*****************************************************************************/
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4,8,0))
#else /* >= 4,8,0 */
#define HAVE_TCF_EXTS_TO_LIST
#define HAVE_PCI_ALLOC_IRQ
#define HAVE_NDO_UDP_TUNNEL_CALLBACK
#endif /* 4,8,0 */

/*****************************************************************************/
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4,9,0))
#define NEED_JIFFIES_64_TIME_IS_MACROS
#else /* >= 4,9,0 */
#define HAVE_KTHREAD_DELAYED_API
#define HAVE_NDO_OFFLOAD_STATS
#undef NEED_DECLARE_STATIC_KEY_FALSE
#define HAVE_INCLUDE_BITFIELD
#endif /* 4,9,0 */

/*****************************************************************************/
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4,9,62))
#define NEED_IN_TASK
#else /* >= 4,9,62 */
#endif /* 4,9,62 */

/*****************************************************************************/
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4,12,0))
#else /* >= 4,12,0 */
#define HAVE_NAPI_BUSY_LOOP
#endif /* 4,12,0 */

/*****************************************************************************/
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4,13,0))
#else /* >= 4,13,0 */
#define HAVE_FLOW_DISSECTOR_KEY_IP
#endif /* 4,13,0 */

/*****************************************************************************/
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4,15,0))
#define NEED_TC_SETUP_QDISC_MQPRIO
#define NEED_NETDEV_XDP_STRUCT
#else /* >= 4,15,0 */
#define HAVE_TC_CB_AND_SETUP_QDISC_MQPRIO
#define HAVE_NDO_BPF
#endif /* 4,15,0 */

/*****************************************************************************/
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4,16,0))
#define NEED_TC_CLS_CAN_OFFLOAD_AND_CHAIN0
#else /* >= 4,16,0 */
#define HAVE_XDP_BUFF_RXQ
#define HAVE_XDP_RXQ_INFO_REG_3_PARAMS
#endif /* 4,16,0 */

/*****************************************************************************/
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4,17,0))
#define NEED_CONVERT_ART_NS_TO_TSC
#else /* >= 4,17,0 */
#endif /* 4,17,0 */

/*****************************************************************************/
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4,18,0))
#define NEED_MACVLAN_ACCEL_PRIV
#define NEED_MACVLAN_RELEASE_L2FW_OFFLOAD
#define NEED_MACVLAN_SUPPORTS_DEST_FILTER
#else /* >= 4,18,0 */
#define HAVE_DEVLINK_PORT_ATTRS_SET_PORT_FLAVOUR
#define HAVE_DEVLINK_PORT_SPLIT_EXTACK
#endif /* 4,18,0 */

/*****************************************************************************/
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4,19,0))
#define NEED_IDA_ALLOC_MIN_MAX_RANGE_FREE
#else /* >= 4,19,0 */
#undef HAVE_TCF_EXTS_TO_LIST
#define HAVE_TCF_EXTS_FOR_EACH_ACTION
#define HAVE_DEVLINK_REGIONS
#define HAVE_TC_ETF_QOPT_OFFLOAD
#define HAVE_DEVLINK_PARAMS
#define HAVE_FLOW_DISSECTOR_KEY_ENC_IP
#endif /* 4,19,0 */

/*****************************************************************************/
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4,20,0))
#define NEED_NETDEV_TX_SENT_QUEUE
#else /* >= 4.20.0 */
#define HAVE_VXLAN_TYPE
#define HAVE_LINKMODE
#endif /* 4.20.0 */

/*****************************************************************************/
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5,0,0))
#define NEED_INDIRECT_CALL_WRAPPER_MACROS
#else /* >= 5.0.0 */
#define HAVE_GRETAP_TYPE
#define HAVE_GENEVE_TYPE
#define HAVE_INDIRECT_CALL_WRAPPER_HEADER
#endif /* 5.0.0 */

/*****************************************************************************/
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5,1,0))
#define NEED_FLOW_MATCH
#else /* >= 5.1.0 */
#define HAVE_ETHTOOL_200G_BITS
#define HAVE_ETHTOOL_NEW_100G_BITS
#define HAVE_DEVLINK_PARAMS_PUBLISH
#define HAVE_DEVLINK_HEALTH
#endif /* 5.1.0 */

/*****************************************************************************/
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5,2,0))
#else /* >= 5.2.0 */
#define HAVE_DEVLINK_PORT_ATTRS_SET_SWITCH_ID
#define HAVE_FLOW_DISSECTOR_KEY_CVLAN
#endif /* 5.2.0 */

/*****************************************************************************/
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5,3,0))
#define NEED_DEVLINK_FLASH_UPDATE_STATUS_NOTIFY
#define NEED_BUS_FIND_DEVICE_CONST_DATA
#else /* >= 5.3.0 */
#if (LINUX_VERSION_CODE > KERNEL_VERSION(5,3,10))
#define HAVE_DEVLINK_RELOAD_ENABLE_DISABLE
#endif /* 5.3.10 */
#endif /* 5.3.0 */

/*****************************************************************************/
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5,4,0))
#define NEED_SKB_FRAG_OFF_ADD
#define NEED_SKB_FRAG_OFF
#if (LINUX_VERSION_CODE > KERNEL_VERSION(4,14,241) && \
     LINUX_VERSION_CODE < KERNEL_VERSION(4,15,0))
#undef NEED_SKB_FRAG_OFF
#endif /* > 4.14.241 && < 4.15.0 */
#if (LINUX_VERSION_CODE > KERNEL_VERSION(4,19,200) && \
     LINUX_VERSION_CODE < KERNEL_VERSION(4,20,0))
#undef NEED_SKB_FRAG_OFF
#endif /* > 4.19.200 && < 4.20.0 */

#define NEED_FLOW_INDR_BLOCK_CB_REGISTER
#else /* >= 5.4.0 */
#define HAVE_FLOW_INDR_BLOCK_LOCK
#define HAVE_XSK_UNALIGNED_CHUNK_PLACEMENT
#endif /* 5.4.0 */

/*****************************************************************************/
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5,5,0))
#else /* >= 5.5.0 */
#define HAVE_DEVLINK_HEALTH_OPS_EXTACK
#endif /* 5.5.0 */

/*****************************************************************************/
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5,7,0))
#define NEED_DEVLINK_REGION_CREATE_OPS
#define NEED_CPU_LATENCY_QOS_RENAME
#else /* >= 5.7.0 */
#define HAVE_DEVLINK_HEALTH_DEFAULT_AUTO_RECOVER
#define HAVE_DEVLINK_REGION_OPS_SNAPSHOT
#define HAVE_PTP_FIND_PIN_UNLOCKED
#endif /* 5.7.0 */

/*****************************************************************************/
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5,8,0))
#define NEED_XSK_UMEM_GET_RX_FRAME_SIZE
#else /* >= 5.8.0 */
#undef HAVE_XSK_UNALIGNED_CHUNK_PLACEMENT
#endif /* 5.8.0 */

/*****************************************************************************/
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5,9,0))
#define NEED_DEVLINK_PORT_ATTRS_SET_STRUCT
#define HAVE_XDP_QUERY_PROG
#define NEED_INDIRECT_CALL_3_AND_4
#define NEED_MUL_U64_U64_DIV_U64
#else /* >= 5.9.0 */
#define HAVE_TASKLET_SETUP
#endif /* 5.9.0 */

/*****************************************************************************/
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5,10,0))
#define NEED_NET_PREFETCH
#define NEED_DEVLINK_FLASH_UPDATE_TIMEOUT_NOTIFY
#define NEED_XSK_BUFF_DMA_SYNC_FOR_CPU
#define NEED_XSK_BUFF_POOL_RENAME
#else /* >= 5.10.0 */
#define HAVE_DEVLINK_RELOAD_ACTION_AND_LIMIT
#define HAVE_DEVLINK_REGION_OPS_SNAPSHOT_OPS
#define HAVE_DEVLINK_FLASH_UPDATE_PARAMS
#define HAVE_UDP_TUNNEL_NIC_SHARED
#define HAVE_NETDEV_BPF_XSK_POOL
#endif /* 5.10.0 */

/*****************************************************************************/
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5,11,0))
#define HAVE_DEVLINK_FLASH_UPDATE_BEGIN_END_NOTIFY
#else /* >= 5.11.0 */
#define HAVE_DEVLINK_FLASH_UPDATE_PARAMS_FW
#define HAVE_XSK_BATCHED_DESCRIPTOR_INTERFACES
#define HAVE_PASID_SUPPORT
#undef HAVE_XDP_RXQ_INFO_REG_3_PARAMS
#define HAVE_XSK_TX_PEEK_RELEASE_DESC_BATCH_3_PARAMS
#endif /* 5.11.0 */

/*****************************************************************************/
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5,12,0))
#define NEED_EXPORT_INDIRECT_CALLABLE
#else /* >= 5.12.0 */
#undef HAVE_NDO_UDP_TUNNEL_CALLBACK
#define HAVE_DEVLINK_OPS_CREATE_DEL
#endif /* 5.12.0 */

/*****************************************************************************/
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5,13,0))
/* HAVE_KOBJ_IN_MDEV_PARENT_OPS_CREATE
 *
 * create api changed as part of the commit c2ef2f50ad0c( vfio/mdev: Remove
 * kobj from mdev_parent_ops->create())
 *
 * if flag is defined use the old API else new API
 */
#define HAVE_KOBJ_IN_MDEV_PARENT_OPS_CREATE
#define HAVE_DEV_IN_MDEV_API
#else /* >= 5.13.0 */
#define HAVE_XPS_MAP_TYPE
#undef NEED_ETHTOOL_SPRINTF
#endif /* 5.13.0 */

/*****************************************************************************/
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5,14,0))
#else /* >= 5.14.0 */
#define HAVE_TTY_WRITE_ROOM_UINT
#endif /* 5.14.0 */

/*****************************************************************************/
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5,15,0))
#define NEED_DEVLINK_ALLOC_SETS_DEV
#define HAVE_DEVLINK_REGISTER_SETS_DEV
#define NEED_ETH_HW_ADDR_SET
#else /* >= 5.15.0 */
#define HAVE_ETHTOOL_COALESCE_EXTACK
#define HAVE_NDO_ETH_IOCTL
#define HAVE_DEVICE_IN_MDEV_PARENT_OPS
#define HAVE_LMV1_SUPPORT
#define NEED_PCI_IOV_VF_ID
#define HAVE_DEVLINK_SET_STATE_3_PARAM
#endif /* 5.15.0 */

/*****************************************************************************/
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5,16,0))
#else /* >= 5.16.0 */
#undef HAVE_PASID_SUPPORT
#define HAVE_DEVLINK_SET_FEATURES
#define HAVE_DEVLINK_NOTIFY_REGISTER
#undef HAVE_DEVLINK_RELOAD_ENABLE_DISABLE
#undef HAVE_DEVLINK_PARAMS_PUBLISH
#define HAVE_XSK_BATCHED_RX_ALLOC
#endif /* 5.16.0 */

/*****************************************************************************/
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5,17,0))
#define NEED_NO_NETDEV_PROG_XDP_WARN_ACTION
#else /* >=5.17.0*/
#define HAVE_XDP_DO_FLUSH
#define HAVE_ETHTOOL_EXTENDED_RINGPARAMS
#endif /* 5.17.0 */

/*****************************************************************************/
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5,18,0))
#else /* >=5.18.0*/
#undef HAVE_LMV1_SUPPORT
#undef NEED_PCI_IOV_VF_ID
#define HAVE_GTP_SUPPORT
#undef HAVE_XSK_TX_PEEK_RELEASE_DESC_BATCH_3_PARAMS
#define HAVE_DEVLINK_PORT_SPLIT_PORT_STRUCT
#define HAVE_DEVL_PORT_REGISTER
#endif /* 5.18.0 */

/*****************************************************************************/
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5,19,0))
#else /* >=5.19.0 */
#define HAVE_NDO_FDB_DEL_EXTACK
#define HAVE_NETIF_SET_TSO_MAX
#endif /* 5.19.0 */

/*****************************************************************************/
#if (LINUX_VERSION_CODE < KERNEL_VERSION(6,0,0))
#else /* >=6.0.0 */
#define HAVE_FLOW_DISSECTOR_KEY_PPPOE
#endif /* 6.0.0 */

/*****************************************************************************/
#if (LINUX_VERSION_CODE < KERNEL_VERSION(6,1,0))
#else /* >=6.1.0 */
#define HAVE_FLOW_DISSECTOR_KEY_L2TPV3
#undef NEED_NETIF_NAPI_ADD_NO_WEIGHT
#define HAVE_TTY_TERMIOS_CONST_STRUCT
#endif /* 6.1.0 */

#if (LINUX_VERSION_CODE < KERNEL_VERSION(6,2,0))
#else /* >=6.2.0 */
#define HAVE_SET_NETDEV_DEVLINK_PORT
#undef HAVE_NDO_GET_DEVLINK_PORT
#endif /* 6.2.0 */

#endif /* _KCOMPAT_STD_DEFS_H_ */
