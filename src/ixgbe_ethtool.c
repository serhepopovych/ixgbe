// SPDX-License-Identifier: GPL-2.0
/* Copyright(c) 1999 - 2020 Intel Corporation. */

/* ethtool support for ixgbe */

#include <linux/types.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/netdevice.h>
#include <linux/ethtool.h>
#include <linux/vmalloc.h>
#include <linux/highmem.h>

#ifdef SIOCETHTOOL
#include <asm/uaccess.h>

#include "ixgbe.h"
#ifdef ETHTOOL_GMODULEINFO
#include "ixgbe_phy.h"
#endif
#ifdef HAVE_ETHTOOL_GET_TS_INFO
#include <linux/net_tstamp.h>
#endif

#ifndef ETH_GSTRING_LEN
#define ETH_GSTRING_LEN 32
#endif

#define IXGBE_ALL_RAR_ENTRIES 16

#ifdef ETHTOOL_OPS_COMPAT
#include "kcompat_ethtool.c"
#endif
#ifdef ETHTOOL_GSTATS
struct ixgbe_stats {
	char stat_string[ETH_GSTRING_LEN];
	int sizeof_stat;
	int stat_offset;
};

#define IXGBE_NETDEV_STAT(_net_stat) { \
	.stat_string = #_net_stat, \
	.sizeof_stat = sizeof_field(struct net_device_stats, _net_stat), \
	.stat_offset = offsetof(struct net_device_stats, _net_stat) \
}
static const struct ixgbe_stats ixgbe_gstrings_net_stats[] = {
	IXGBE_NETDEV_STAT(rx_packets),
	IXGBE_NETDEV_STAT(tx_packets),
	IXGBE_NETDEV_STAT(rx_bytes),
	IXGBE_NETDEV_STAT(tx_bytes),
	IXGBE_NETDEV_STAT(rx_errors),
	IXGBE_NETDEV_STAT(tx_errors),
	IXGBE_NETDEV_STAT(rx_dropped),
	IXGBE_NETDEV_STAT(tx_dropped),
	IXGBE_NETDEV_STAT(multicast),
	IXGBE_NETDEV_STAT(collisions),
	IXGBE_NETDEV_STAT(rx_over_errors),
	IXGBE_NETDEV_STAT(rx_crc_errors),
	IXGBE_NETDEV_STAT(rx_frame_errors),
	IXGBE_NETDEV_STAT(rx_fifo_errors),
	IXGBE_NETDEV_STAT(rx_missed_errors),
	IXGBE_NETDEV_STAT(tx_aborted_errors),
	IXGBE_NETDEV_STAT(tx_carrier_errors),
	IXGBE_NETDEV_STAT(tx_fifo_errors),
	IXGBE_NETDEV_STAT(tx_heartbeat_errors),
};

#define IXGBE_STAT(_name, _stat) { \
	.stat_string = _name, \
	.sizeof_stat = sizeof_field(struct ixgbe_adapter, _stat), \
	.stat_offset = offsetof(struct ixgbe_adapter, _stat) \
}
static struct ixgbe_stats ixgbe_gstrings_stats[] = {
	IXGBE_STAT("rx_pkts_nic", stats.gprc),
	IXGBE_STAT("tx_pkts_nic", stats.gptc),
	IXGBE_STAT("rx_bytes_nic", stats.gorc),
	IXGBE_STAT("tx_bytes_nic", stats.gotc),
	IXGBE_STAT("lsc_int", lsc_int),
	IXGBE_STAT("tx_busy", tx_busy),
	IXGBE_STAT("non_eop_descs", non_eop_descs),
	IXGBE_STAT("broadcast", stats.bprc),
	IXGBE_STAT("rx_no_buffer_count", stats.rnbc[0]) ,
	IXGBE_STAT("tx_timeout_count", tx_timeout_count),
	IXGBE_STAT("tx_restart_queue", restart_queue),
	IXGBE_STAT("rx_length_errors", stats.rlec),
	IXGBE_STAT("rx_long_length_errors", stats.roc),
	IXGBE_STAT("rx_short_length_errors", stats.ruc),
	IXGBE_STAT("tx_flow_control_xon", stats.lxontxc),
	IXGBE_STAT("rx_flow_control_xon", stats.lxonrxc),
	IXGBE_STAT("tx_flow_control_xoff", stats.lxofftxc),
	IXGBE_STAT("rx_flow_control_xoff", stats.lxoffrxc),
	IXGBE_STAT("rx_csum_offload_errors", hw_csum_rx_error),
	IXGBE_STAT("alloc_rx_page", alloc_rx_page),
	IXGBE_STAT("alloc_rx_page_failed", alloc_rx_page_failed),
	IXGBE_STAT("alloc_rx_buff_failed", alloc_rx_buff_failed),
	IXGBE_STAT("rx_no_dma_resources", hw_rx_no_dma_resources),
	IXGBE_STAT("hw_rsc_aggregated", rsc_total_count),
	IXGBE_STAT("hw_rsc_flushed", rsc_total_flush),
#ifdef HAVE_TX_MQ
	IXGBE_STAT("fdir_match", stats.fdirmatch),
	IXGBE_STAT("fdir_miss", stats.fdirmiss),
	IXGBE_STAT("fdir_overflow", fdir_overflow),
#endif /* HAVE_TX_MQ */
#if IS_ENABLED(CONFIG_FCOE)
	IXGBE_STAT("fcoe_bad_fccrc", stats.fccrc),
	IXGBE_STAT("fcoe_last_errors", stats.fclast),
	IXGBE_STAT("rx_fcoe_dropped", stats.fcoerpdc),
	IXGBE_STAT("rx_fcoe_packets", stats.fcoeprc),
	IXGBE_STAT("rx_fcoe_dwords", stats.fcoedwrc),
	IXGBE_STAT("fcoe_noddp", stats.fcoe_noddp),
	IXGBE_STAT("fcoe_noddp_ext_buff", stats.fcoe_noddp_ext_buff),
	IXGBE_STAT("tx_fcoe_packets", stats.fcoeptc),
	IXGBE_STAT("tx_fcoe_dwords", stats.fcoedwtc),
#endif /* CONFIG_FCOE */
	IXGBE_STAT("os2bmc_rx_by_bmc", stats.o2bgptc),
	IXGBE_STAT("os2bmc_tx_by_bmc", stats.b2ospc),
	IXGBE_STAT("os2bmc_tx_by_host", stats.o2bspc),
	IXGBE_STAT("os2bmc_rx_by_host", stats.b2ogprc),
#ifdef HAVE_PTP_1588_CLOCK
	IXGBE_STAT("tx_hwtstamp_timeouts", tx_hwtstamp_timeouts),
	IXGBE_STAT("tx_hwtstamp_skipped", tx_hwtstamp_skipped),
	IXGBE_STAT("rx_hwtstamp_cleared", rx_hwtstamp_cleared),
#endif /* HAVE_PTP_1588_CLOCK */
};

/* ixgbe allocates num_tx_queues and num_rx_queues symmetrically so
 * we set the num_rx_queues to evaluate to num_tx_queues. This is
 * used because we do not have a good way to get the max number of
 * rx queues with CONFIG_RPS disabled.
 */
#ifdef HAVE_TX_MQ
#ifdef HAVE_NETDEV_SELECT_QUEUE
#define IXGBE_NUM_RX_QUEUES netdev->num_tx_queues
#define IXGBE_NUM_TX_QUEUES netdev->num_tx_queues
#else
#define IXGBE_NUM_RX_QUEUES adapter->indices
#define IXGBE_NUM_TX_QUEUES adapter->indices
#endif /* HAVE_NETDEV_SELECT_QUEUE */
#else /* HAVE_TX_MQ */
#define IXGBE_NUM_TX_QUEUES 1
#define IXGBE_NUM_RX_QUEUES ( \
		((struct ixgbe_adapter *)netdev_priv(netdev))->num_rx_queues)
#endif /* HAVE_TX_MQ */

#define IXGBE_QUEUE_STATS_LEN ( \
		(IXGBE_NUM_TX_QUEUES + IXGBE_NUM_RX_QUEUES) * \
		(sizeof(struct ixgbe_queue_stats) / sizeof(u64)))
#define IXGBE_GLOBAL_STATS_LEN	ARRAY_SIZE(ixgbe_gstrings_stats)
#define IXGBE_NETDEV_STATS_LEN	ARRAY_SIZE(ixgbe_gstrings_net_stats)
#define IXGBE_PB_STATS_LEN ( \
		(sizeof(((struct ixgbe_adapter *)0)->stats.pxonrxc) + \
		 sizeof(((struct ixgbe_adapter *)0)->stats.pxontxc) + \
		 sizeof(((struct ixgbe_adapter *)0)->stats.pxoffrxc) + \
		 sizeof(((struct ixgbe_adapter *)0)->stats.pxofftxc)) \
		/ sizeof(u64))
#define IXGBE_VF_STATS_LEN \
	((((struct ixgbe_adapter *)netdev_priv(netdev))->num_vfs) * \
	  (sizeof(struct vf_stats) / sizeof(u64)))
#define IXGBE_STATS_LEN (IXGBE_GLOBAL_STATS_LEN + \
			 IXGBE_NETDEV_STATS_LEN + \
			 IXGBE_PB_STATS_LEN + \
			 IXGBE_QUEUE_STATS_LEN + \
			 IXGBE_VF_STATS_LEN)

#endif /* ETHTOOL_GSTATS */
#ifdef ETHTOOL_TEST
static const char ixgbe_gstrings_test[][ETH_GSTRING_LEN] = {
	"Register test  (offline)", "Eeprom test    (offline)",
	"Interrupt test (offline)", "Loopback test  (offline)",
	"Link test   (on/offline)"
};
#define IXGBE_TEST_LEN	(sizeof(ixgbe_gstrings_test) / ETH_GSTRING_LEN)
#endif /* ETHTOOL_TEST */

#ifdef HAVE_ETHTOOL_GET_SSET_COUNT
static const char ixgbe_priv_flags_strings[][ETH_GSTRING_LEN] = {
#define IXGBE_PRIV_FLAGS_FD_ATR		BIT(0)
	"flow-director-atr",
#ifdef HAVE_SWIOTLB_SKIP_CPU_SYNC
#define IXGBE_PRIV_FLAGS_LEGACY_RX	BIT(1)
	"legacy-rx",
#endif
};

#define IXGBE_PRIV_FLAGS_STR_LEN ARRAY_SIZE(ixgbe_priv_flags_strings)

#endif /* HAVE_ETHTOOL_GET_SSET_COUNT */
/* currently supported speeds for 10G */
#define ADVERTISED_MASK_10G (SUPPORTED_10000baseT_Full | SUPPORTED_10000baseKX4_Full | SUPPORTED_10000baseKR_Full)

#define ixgbe_isbackplane(type)  ((type == ixgbe_media_type_backplane)? true : false)

#ifdef ETHTOOL_GLINKSETTINGS
static void ixgbe_set_supported_10gtypes(struct ixgbe_hw *hw,
					 struct ethtool_link_ksettings *cmd)
{
	if (!ixgbe_isbackplane(hw->phy.media_type)) {
		ethtool_link_ksettings_add_link_mode
			(cmd, supported, 10000baseT_Full);
		return;
	}

	switch (hw->device_id) {
	case IXGBE_DEV_ID_82598:
	case IXGBE_DEV_ID_82599_KX4:
	case IXGBE_DEV_ID_82599_KX4_MEZZ:
	case IXGBE_DEV_ID_X550EM_X_KX4:
		ethtool_link_ksettings_add_link_mode
			(cmd, supported, 10000baseKX4_Full);
		break;
	case IXGBE_DEV_ID_82598_BX:
	case IXGBE_DEV_ID_82599_KR:
	case IXGBE_DEV_ID_X550EM_X_KR:
	case IXGBE_DEV_ID_X550EM_X_XFI:
		ethtool_link_ksettings_add_link_mode
			(cmd, supported, 10000baseKR_Full);
		break;
	default:
		ethtool_link_ksettings_add_link_mode
			(cmd, supported, 10000baseKX4_Full);
		ethtool_link_ksettings_add_link_mode
			(cmd, supported, 10000baseKR_Full);
		break;
	}
}

static void ixgbe_set_advertising_10gtypes(struct ixgbe_hw *hw,
					   struct ethtool_link_ksettings *cmd)
{
	if (!ixgbe_isbackplane(hw->phy.media_type)) {
		ethtool_link_ksettings_add_link_mode
			(cmd, advertising, 10000baseT_Full);
		return;
	}

	switch (hw->device_id) {
	case IXGBE_DEV_ID_82598:
	case IXGBE_DEV_ID_82599_KX4:
	case IXGBE_DEV_ID_82599_KX4_MEZZ:
	case IXGBE_DEV_ID_X550EM_X_KX4:
		ethtool_link_ksettings_add_link_mode
			(cmd, advertising, 10000baseKX4_Full);
		break;
	case IXGBE_DEV_ID_82598_BX:
	case IXGBE_DEV_ID_82599_KR:
	case IXGBE_DEV_ID_X550EM_X_KR:
	case IXGBE_DEV_ID_X550EM_X_XFI:
		ethtool_link_ksettings_add_link_mode
			(cmd, advertising, 10000baseKR_Full);
		break;
	default:
		ethtool_link_ksettings_add_link_mode
			(cmd, advertising, 10000baseKX4_Full);
		ethtool_link_ksettings_add_link_mode
			(cmd, advertising, 10000baseKR_Full);
		break;
	}
}

static int ixgbe_get_link_ksettings(struct net_device *netdev,
				    struct ethtool_link_ksettings *cmd)
{
	struct ixgbe_adapter *adapter = netdev_priv(netdev);
	struct ixgbe_hw *hw = &adapter->hw;
	ixgbe_link_speed supported_link;
	bool autoneg = false;

	ethtool_link_ksettings_zero_link_mode(cmd, supported);
	ethtool_link_ksettings_zero_link_mode(cmd, advertising);

	hw->mac.ops.get_link_capabilities(hw, &supported_link, &autoneg);
	/* set the supported link speeds */
	if (supported_link & IXGBE_LINK_SPEED_10GB_FULL) {
		ixgbe_set_supported_10gtypes(hw, cmd);
		ixgbe_set_advertising_10gtypes(hw, cmd);
	}
#ifdef HAVE_ETHTOOL_5G_BITS
	if (supported_link & IXGBE_LINK_SPEED_5GB_FULL)
		ethtool_link_ksettings_add_link_mode(cmd, supported,
						     5000baseT_Full);
#endif
#ifdef HAVE_ETHTOOL_NEW_2500MB_BITS
	if (supported_link & IXGBE_LINK_SPEED_2_5GB_FULL)
		ethtool_link_ksettings_add_link_mode(cmd, supported,
						     2500baseT_Full);
#endif
	if (supported_link & IXGBE_LINK_SPEED_1GB_FULL) {
		if (ixgbe_isbackplane(hw->phy.media_type)) {
			ethtool_link_ksettings_add_link_mode(cmd, supported,
							     1000baseKX_Full);
			ethtool_link_ksettings_add_link_mode(cmd, advertising,
							     1000baseKX_Full);
		} else {
			ethtool_link_ksettings_add_link_mode(cmd, supported,
							     1000baseT_Full);
			ethtool_link_ksettings_add_link_mode(cmd, advertising,
							     1000baseT_Full);
		}
	}
	if (supported_link & IXGBE_LINK_SPEED_100_FULL) {
		ethtool_link_ksettings_add_link_mode(cmd, supported,
						     100baseT_Full);
		ethtool_link_ksettings_add_link_mode(cmd, advertising,
						     100baseT_Full);
	}
	if (supported_link & IXGBE_LINK_SPEED_10_FULL) {
		ethtool_link_ksettings_add_link_mode(cmd, supported,
						     10baseT_Full);
		ethtool_link_ksettings_add_link_mode(cmd, advertising,
						     10baseT_Full);
	}

	/* set the advertised speeds */
	if (hw->phy.autoneg_advertised)	{
		ethtool_link_ksettings_zero_link_mode(cmd, advertising);
		if (hw->phy.autoneg_advertised & IXGBE_LINK_SPEED_10_FULL)
			ethtool_link_ksettings_add_link_mode(cmd, advertising,
							     10baseT_Full);
		if (hw->phy.autoneg_advertised & IXGBE_LINK_SPEED_100_FULL)
			ethtool_link_ksettings_add_link_mode(cmd, advertising,
							     100baseT_Full);
		if (hw->phy.autoneg_advertised & IXGBE_LINK_SPEED_10GB_FULL)
			ixgbe_set_advertising_10gtypes(hw, cmd);
		if (hw->phy.autoneg_advertised & IXGBE_LINK_SPEED_1GB_FULL) {
			if (ethtool_link_ksettings_test_link_mode
				(cmd, supported, 1000baseKX_Full))
				ethtool_link_ksettings_add_link_mode
					(cmd, advertising, 1000baseKX_Full);
			else
				ethtool_link_ksettings_add_link_mode
					(cmd, advertising, 1000baseT_Full);
		}
#ifdef HAVE_ETHTOOL_5G_BITS
		if (hw->phy.autoneg_advertised & IXGBE_LINK_SPEED_5GB_FULL) {
			ethtool_link_ksettings_add_link_mode(cmd, advertising,
							     5000baseT_Full);
		}
#endif
#ifdef HAVE_ETHTOOL_NEW_2500MB_BITS
		if (hw->phy.autoneg_advertised & IXGBE_LINK_SPEED_2_5GB_FULL) {
			ethtool_link_ksettings_add_link_mode(cmd, advertising,
							     2500baseT_Full);
		}
#endif
	} else {
		if (hw->phy.multispeed_fiber && !autoneg) {
			if (supported_link & IXGBE_LINK_SPEED_10GB_FULL)
				ethtool_link_ksettings_add_link_mode
					(cmd, advertising, 10000baseT_Full);
		}
	}

	if (autoneg) {
		ethtool_link_ksettings_add_link_mode(cmd, supported, Autoneg);
		ethtool_link_ksettings_add_link_mode(cmd, advertising, Autoneg);
		cmd->base.autoneg = AUTONEG_ENABLE;
	} else {
		cmd->base.autoneg = AUTONEG_DISABLE;
	}

	/* Determine the remaining settings based on the PHY type. */
	switch (adapter->hw.phy.type) {
	case ixgbe_phy_tn:
	case ixgbe_phy_aq:
	case ixgbe_phy_x550em_ext_t:
	case ixgbe_phy_fw:
	case ixgbe_phy_cu_unknown:
		ethtool_link_ksettings_add_link_mode(cmd, supported, TP);
		ethtool_link_ksettings_add_link_mode(cmd, advertising, TP);
		cmd->base.port = PORT_TP;
		break;
	case ixgbe_phy_qt:
		ethtool_link_ksettings_add_link_mode(cmd, supported, FIBRE);
		ethtool_link_ksettings_add_link_mode(cmd, advertising, FIBRE);
		cmd->base.port = PORT_FIBRE;
		break;
	case ixgbe_phy_nl:
	case ixgbe_phy_sfp_passive_tyco:
	case ixgbe_phy_sfp_passive_unknown:
	case ixgbe_phy_sfp_ftl:
	case ixgbe_phy_sfp_avago:
	case ixgbe_phy_sfp_intel:
	case ixgbe_phy_sfp_unknown:
	case ixgbe_phy_qsfp_passive_unknown:
	case ixgbe_phy_qsfp_active_unknown:
	case ixgbe_phy_qsfp_intel:
	case ixgbe_phy_qsfp_unknown:
		switch (adapter->hw.phy.sfp_type) {
			/* SFP+ devices, further checking needed */
		case ixgbe_sfp_type_da_cu:
		case ixgbe_sfp_type_da_cu_core0:
		case ixgbe_sfp_type_da_cu_core1:
			ethtool_link_ksettings_add_link_mode(cmd, supported,
							     FIBRE);
			ethtool_link_ksettings_add_link_mode(cmd, advertising,
							     FIBRE);
			cmd->base.port = PORT_DA;
			break;
		case ixgbe_sfp_type_sr:
		case ixgbe_sfp_type_lr:
		case ixgbe_sfp_type_srlr_core0:
		case ixgbe_sfp_type_srlr_core1:
		case ixgbe_sfp_type_1g_sx_core0:
		case ixgbe_sfp_type_1g_sx_core1:
		case ixgbe_sfp_type_1g_lx_core0:
		case ixgbe_sfp_type_1g_lx_core1:
			ethtool_link_ksettings_add_link_mode(cmd, supported,
							     FIBRE);
			ethtool_link_ksettings_add_link_mode(cmd, advertising,
							     FIBRE);
			cmd->base.port = PORT_FIBRE;
			break;
		case ixgbe_sfp_type_not_present:
			ethtool_link_ksettings_add_link_mode(cmd, supported,
							     FIBRE);
			ethtool_link_ksettings_add_link_mode(cmd, advertising,
							     FIBRE);
			cmd->base.port = PORT_NONE;
			break;
		case ixgbe_sfp_type_1g_cu_core0:
		case ixgbe_sfp_type_1g_cu_core1:
			ethtool_link_ksettings_add_link_mode(cmd, supported,
							     TP);
			ethtool_link_ksettings_add_link_mode(cmd, advertising,
							     TP);
			cmd->base.port = PORT_TP;
			break;
		case ixgbe_sfp_type_unknown:
		default:
			ethtool_link_ksettings_add_link_mode(cmd, supported,
							     FIBRE);
			ethtool_link_ksettings_add_link_mode(cmd, advertising,
							     FIBRE);
			cmd->base.port = PORT_OTHER;
			break;
		}
		break;
	case ixgbe_phy_xaui:
		ethtool_link_ksettings_add_link_mode(cmd, supported, FIBRE);
		ethtool_link_ksettings_add_link_mode(cmd, advertising, FIBRE);
		cmd->base.port = PORT_NONE;
		break;
	case ixgbe_phy_unknown:
	case ixgbe_phy_generic:
	case ixgbe_phy_sfp_unsupported:
	default:
		ethtool_link_ksettings_add_link_mode(cmd, supported, FIBRE);
		ethtool_link_ksettings_add_link_mode(cmd, advertising, FIBRE);
		cmd->base.port = PORT_OTHER;
		break;
	}

	/* Indicate pause support */
	ethtool_link_ksettings_add_link_mode(cmd, supported, Pause);

	switch (hw->fc.requested_mode) {
	case ixgbe_fc_full:
		ethtool_link_ksettings_add_link_mode(cmd, advertising, Pause);
		break;
	case ixgbe_fc_rx_pause:
		ethtool_link_ksettings_add_link_mode(cmd, advertising, Pause);
		ethtool_link_ksettings_add_link_mode(cmd, advertising,
						     Asym_Pause);
		break;
	case ixgbe_fc_tx_pause:
		ethtool_link_ksettings_add_link_mode(cmd, advertising,
						     Asym_Pause);
		break;
	default:
		ethtool_link_ksettings_del_link_mode(cmd, advertising, Pause);
		ethtool_link_ksettings_del_link_mode(cmd, advertising,
						     Asym_Pause);
	}

	if (netif_carrier_ok(netdev)) {
		switch (adapter->link_speed) {
		case IXGBE_LINK_SPEED_10GB_FULL:
			cmd->base.speed = SPEED_10000;
			break;
		case IXGBE_LINK_SPEED_5GB_FULL:
			cmd->base.speed = SPEED_5000;
			break;
#ifdef SUPPORTED_2500baseX_Full
		case IXGBE_LINK_SPEED_2_5GB_FULL:
			cmd->base.speed = SPEED_2500;
			break;
#endif /* SUPPORTED_2500baseX_Full */
		case IXGBE_LINK_SPEED_1GB_FULL:
			cmd->base.speed = SPEED_1000;
			break;
		case IXGBE_LINK_SPEED_100_FULL:
			cmd->base.speed = SPEED_100;
			break;
		case IXGBE_LINK_SPEED_10_FULL:
			cmd->base.speed = SPEED_10;
			break;
		default:
			break;
		}
		cmd->base.duplex = DUPLEX_FULL;
	} else {
		cmd->base.speed = SPEED_UNKNOWN;
		cmd->base.duplex = DUPLEX_UNKNOWN;
	}

	return 0;
}
#else /* ETHTOOL_GLINKSETTINGS */
static __u32 ixgbe_backplane_type(struct ixgbe_hw *hw)
{
	__u32 mode = 0x00;

	switch (hw->device_id) {
	case IXGBE_DEV_ID_82598:
	case IXGBE_DEV_ID_82599_KX4:
	case IXGBE_DEV_ID_82599_KX4_MEZZ:
	case IXGBE_DEV_ID_X550EM_X_KX4:
		mode = SUPPORTED_10000baseKX4_Full;
		break;
	case IXGBE_DEV_ID_82598_BX:
	case IXGBE_DEV_ID_82599_KR:
	case IXGBE_DEV_ID_X550EM_X_KR:
	case IXGBE_DEV_ID_X550EM_X_XFI:
		mode = SUPPORTED_10000baseKR_Full;
		break;
	default:
		mode = (SUPPORTED_10000baseKX4_Full |
			SUPPORTED_10000baseKR_Full);
		break;
	}
	return mode;
}

static int ixgbe_get_settings(struct net_device *netdev,
			      struct ethtool_cmd *ecmd)
{
	struct ixgbe_adapter *adapter = netdev_priv(netdev);
	struct ixgbe_hw *hw = &adapter->hw;
	ixgbe_link_speed supported_link;
	bool autoneg = false;

	hw->mac.ops.get_link_capabilities(hw, &supported_link, &autoneg);

	/* set the supported link speeds */
	if (supported_link & IXGBE_LINK_SPEED_10GB_FULL)
		ecmd->supported |= (ixgbe_isbackplane(hw->phy.media_type)) ?
				    ixgbe_backplane_type(hw) :
				    SUPPORTED_10000baseT_Full;
	if (supported_link & IXGBE_LINK_SPEED_1GB_FULL)
		ecmd->supported |= (ixgbe_isbackplane(hw->phy.media_type)) ?
				    SUPPORTED_1000baseKX_Full :
				    SUPPORTED_1000baseT_Full;
	if (supported_link & IXGBE_LINK_SPEED_100_FULL)
		ecmd->supported |= SUPPORTED_100baseT_Full;
	if (supported_link & IXGBE_LINK_SPEED_10_FULL)
		ecmd->supported |= SUPPORTED_10baseT_Full;

	/* default advertised speed if phy.autoneg_advertised isn't set */
	ecmd->advertising = ecmd->supported;

	/* set the advertised speeds */
	if (hw->phy.autoneg_advertised) {
		ecmd->advertising = 0;
		if (hw->phy.autoneg_advertised & IXGBE_LINK_SPEED_10_FULL)
			ecmd->advertising |= ADVERTISED_10baseT_Full;
		if (hw->phy.autoneg_advertised & IXGBE_LINK_SPEED_100_FULL)
			ecmd->advertising |= ADVERTISED_100baseT_Full;
		if (hw->phy.autoneg_advertised & IXGBE_LINK_SPEED_10GB_FULL)
			ecmd->advertising |= (ecmd->supported & ADVERTISED_MASK_10G);
		if (hw->phy.autoneg_advertised & IXGBE_LINK_SPEED_1GB_FULL) {
			if (ecmd->supported & SUPPORTED_1000baseKX_Full)
				ecmd->advertising |= ADVERTISED_1000baseKX_Full;
			else
				ecmd->advertising |= ADVERTISED_1000baseT_Full;
		}
	} else {
		if (hw->phy.multispeed_fiber && !autoneg) {
			if (supported_link & IXGBE_LINK_SPEED_10GB_FULL)
				ecmd->advertising = ADVERTISED_10000baseT_Full;
		}
	}

	if (autoneg) {
		ecmd->supported |= SUPPORTED_Autoneg;
		ecmd->advertising |= ADVERTISED_Autoneg;
		ecmd->autoneg = AUTONEG_ENABLE;
	} else {
		ecmd->autoneg = AUTONEG_DISABLE;
	}

	ecmd->transceiver = XCVR_EXTERNAL;

	/* Determine the remaining settings based on the PHY type. */
	switch (adapter->hw.phy.type) {
	case ixgbe_phy_tn:
	case ixgbe_phy_aq:
	case ixgbe_phy_x550em_ext_t:
	case ixgbe_phy_fw:
	case ixgbe_phy_cu_unknown:
		ecmd->supported |= SUPPORTED_TP;
		ecmd->advertising |= ADVERTISED_TP;
		ecmd->port = PORT_TP;
		break;
	case ixgbe_phy_qt:
		ecmd->supported |= SUPPORTED_FIBRE;
		ecmd->advertising |= ADVERTISED_FIBRE;
		ecmd->port = PORT_FIBRE;
		break;
	case ixgbe_phy_nl:
	case ixgbe_phy_sfp_passive_tyco:
	case ixgbe_phy_sfp_passive_unknown:
	case ixgbe_phy_sfp_ftl:
	case ixgbe_phy_sfp_avago:
	case ixgbe_phy_sfp_intel:
	case ixgbe_phy_sfp_unknown:
	case ixgbe_phy_qsfp_passive_unknown:
	case ixgbe_phy_qsfp_active_unknown:
	case ixgbe_phy_qsfp_intel:
	case ixgbe_phy_qsfp_unknown:
		switch (adapter->hw.phy.sfp_type) {
			/* SFP+ devices, further checking needed */
		case ixgbe_sfp_type_da_cu:
		case ixgbe_sfp_type_da_cu_core0:
		case ixgbe_sfp_type_da_cu_core1:
			ecmd->supported |= SUPPORTED_FIBRE;
			ecmd->advertising |= ADVERTISED_FIBRE;
			ecmd->port = PORT_DA;
			break;
		case ixgbe_sfp_type_sr:
		case ixgbe_sfp_type_lr:
		case ixgbe_sfp_type_srlr_core0:
		case ixgbe_sfp_type_srlr_core1:
		case ixgbe_sfp_type_1g_sx_core0:
		case ixgbe_sfp_type_1g_sx_core1:
		case ixgbe_sfp_type_1g_lx_core0:
		case ixgbe_sfp_type_1g_lx_core1:
			ecmd->supported |= SUPPORTED_FIBRE;
			ecmd->advertising |= ADVERTISED_FIBRE;
			ecmd->port = PORT_FIBRE;
			break;
		case ixgbe_sfp_type_not_present:
			ecmd->supported |= SUPPORTED_FIBRE;
			ecmd->advertising |= ADVERTISED_FIBRE;
			ecmd->port = PORT_NONE;
			break;
		case ixgbe_sfp_type_1g_cu_core0:
		case ixgbe_sfp_type_1g_cu_core1:
			ecmd->supported |= SUPPORTED_TP;
			ecmd->advertising |= ADVERTISED_TP;
			ecmd->port = PORT_TP;
			break;
		case ixgbe_sfp_type_unknown:
		default:
			ecmd->supported |= SUPPORTED_FIBRE;
			ecmd->advertising |= ADVERTISED_FIBRE;
			ecmd->port = PORT_OTHER;
			break;
		}
		break;
	case ixgbe_phy_xaui:
		ecmd->supported |= SUPPORTED_FIBRE;
		ecmd->advertising |= ADVERTISED_FIBRE;
		ecmd->port = PORT_NONE;
		break;
	case ixgbe_phy_unknown:
	case ixgbe_phy_generic:
	case ixgbe_phy_sfp_unsupported:
	default:
		ecmd->supported |= SUPPORTED_FIBRE;
		ecmd->advertising |= ADVERTISED_FIBRE;
		ecmd->port = PORT_OTHER;
		break;
	}

	/* Indicate pause support */
	ecmd->supported |= SUPPORTED_Pause;

	switch (hw->fc.requested_mode) {
	case ixgbe_fc_full:
		ecmd->advertising |= ADVERTISED_Pause;
		break;
	case ixgbe_fc_rx_pause:
		ecmd->advertising |= ADVERTISED_Pause |
				     ADVERTISED_Asym_Pause;
		break;
	case ixgbe_fc_tx_pause:
		ecmd->advertising |= ADVERTISED_Asym_Pause;
		break;
	default:
		ecmd->advertising &= ~(ADVERTISED_Pause |
				       ADVERTISED_Asym_Pause);
	}

	if (netif_carrier_ok(netdev)) {
		switch (adapter->link_speed) {
		case IXGBE_LINK_SPEED_10GB_FULL:
			ethtool_cmd_speed_set(ecmd, SPEED_10000);
			break;
		case IXGBE_LINK_SPEED_5GB_FULL:
			ethtool_cmd_speed_set(ecmd, SPEED_5000);
			break;
#ifdef SUPPORTED_2500baseX_Full
		case IXGBE_LINK_SPEED_2_5GB_FULL:
			ethtool_cmd_speed_set(ecmd, SPEED_2500);
			break;
#endif /* SUPPORTED_2500baseX_Full */
		case IXGBE_LINK_SPEED_1GB_FULL:
			ethtool_cmd_speed_set(ecmd, SPEED_1000);
			break;
		case IXGBE_LINK_SPEED_100_FULL:
			ethtool_cmd_speed_set(ecmd, SPEED_100);
			break;
		case IXGBE_LINK_SPEED_10_FULL:
			ethtool_cmd_speed_set(ecmd, SPEED_10);
			break;
		default:
			break;
		}
		ecmd->duplex = DUPLEX_FULL;
	} else {
		ethtool_cmd_speed_set(ecmd, SPEED_UNKNOWN);
		ecmd->duplex = DUPLEX_UNKNOWN;
	}

	return 0;
}
#endif /* !ETHTOOL_GLINKSETTINGS */

#ifdef ETHTOOL_GLINKSETTINGS
static int ixgbe_set_link_ksettings(struct net_device *netdev,
				    const struct ethtool_link_ksettings *cmd)
{
	struct ixgbe_adapter *adapter = netdev_priv(netdev);
	struct ixgbe_hw *hw = &adapter->hw;
	u32 advertised, old;
	s32 err = 0;

	if (hw->phy.media_type == ixgbe_media_type_copper ||
	    hw->phy.multispeed_fiber) {
		/*
		 * this function does not support duplex forcing, but can
		 * limit the advertising of the adapter to the specified speed
		 */
		if (!bitmap_subset(cmd->link_modes.advertising,
				   cmd->link_modes.supported,
				   __ETHTOOL_LINK_MODE_MASK_NBITS))
			return -EINVAL;

		/* only allow one speed at a time if no autoneg */
		if (!cmd->base.autoneg && hw->phy.multispeed_fiber) {
			if (ethtool_link_ksettings_test_link_mode
				(cmd, advertising, 10000baseT_Full) &&
				ethtool_link_ksettings_test_link_mode
					(cmd, advertising, 1000baseT_Full))
				return -EINVAL;
		}

		old = hw->phy.autoneg_advertised;
		advertised = 0;
		if (ethtool_link_ksettings_test_link_mode(cmd, advertising,
							  10000baseT_Full))
			advertised |= IXGBE_LINK_SPEED_10GB_FULL;
#ifdef HAVE_ETHTOOL_5G_BITS
		if (ethtool_link_ksettings_test_link_mode(cmd, advertising,
							  5000baseT_Full))
			advertised |= IXGBE_LINK_SPEED_5GB_FULL;
#endif
#ifdef HAVE_ETHTOOL_NEW_2500MB_BITS
		if (ethtool_link_ksettings_test_link_mode(cmd, advertising,
							  2500baseT_Full))
			advertised |= IXGBE_LINK_SPEED_2_5GB_FULL;
#endif
		if (ethtool_link_ksettings_test_link_mode(cmd, advertising,
							  1000baseT_Full))
			advertised |= IXGBE_LINK_SPEED_1GB_FULL;
		if (ethtool_link_ksettings_test_link_mode(cmd, advertising,
							  100baseT_Full))
			advertised |= IXGBE_LINK_SPEED_100_FULL;
		if (ethtool_link_ksettings_test_link_mode(cmd, advertising,
							  10baseT_Full))
			advertised |= IXGBE_LINK_SPEED_10_FULL;

		if (old == advertised)
			return err;
		/* this sets the link speed and restarts auto-neg */
		while (test_and_set_bit(__IXGBE_IN_SFP_INIT, &adapter->state))
			usleep_range(1000, 2000);

		hw->mac.autotry_restart = true;
		err = hw->mac.ops.setup_link(hw, advertised, true);
		if (err) {
			e_info(probe, "setup link failed with code %d\n", err);
			hw->mac.ops.setup_link(hw, old, true);
		}
		clear_bit(__IXGBE_IN_SFP_INIT, &adapter->state);
	} else {
		/* in this case we currently only support 10Gb/FULL */
		u32 speed = cmd->base.speed;

		if (cmd->base.autoneg == AUTONEG_ENABLE ||
		    (!ethtool_link_ksettings_test_link_mode(cmd, advertising,
							    10000baseT_Full)) ||
		    (speed + cmd->base.duplex != SPEED_10000 + DUPLEX_FULL))
			return -EINVAL;
	}

	return err;
}
#else
static int ixgbe_set_settings(struct net_device *netdev,
			      struct ethtool_cmd *ecmd)
{
	struct ixgbe_adapter *adapter = netdev_priv(netdev);
	struct ixgbe_hw *hw = &adapter->hw;
	u32 advertised, old;
	s32 err = 0;

	if ((hw->phy.media_type == ixgbe_media_type_copper) ||
	    (hw->phy.multispeed_fiber)) {
		/*
		 * this function does not support duplex forcing, but can
		 * limit the advertising of the adapter to the specified speed
		 */
		if (ecmd->advertising & ~ecmd->supported)
			return -EINVAL;

		/* only allow one speed at a time if no autoneg */
		if (!ecmd->autoneg && hw->phy.multispeed_fiber) {
			if (ecmd->advertising ==
			    (ADVERTISED_10000baseT_Full |
			     ADVERTISED_1000baseT_Full))
				return -EINVAL;
		}

		old = hw->phy.autoneg_advertised;
		advertised = 0;
		if (ecmd->advertising & ADVERTISED_10000baseT_Full)
			advertised |= IXGBE_LINK_SPEED_10GB_FULL;

		if (ecmd->advertising & ADVERTISED_1000baseT_Full)
			advertised |= IXGBE_LINK_SPEED_1GB_FULL;

		if (ecmd->advertising & ADVERTISED_100baseT_Full)
			advertised |= IXGBE_LINK_SPEED_100_FULL;

		if (ecmd->advertising & ADVERTISED_10baseT_Full)
			advertised |= IXGBE_LINK_SPEED_10_FULL;

		if (old == advertised)
			return err;
		/* this sets the link speed and restarts auto-neg */
		while (test_and_set_bit(__IXGBE_IN_SFP_INIT, &adapter->state))
			usleep_range(1000, 2000);

		hw->mac.autotry_restart = true;
		err = hw->mac.ops.setup_link(hw, advertised, true);
		if (err) {
			e_info(probe, "setup link failed with code %d\n", err);
			hw->mac.ops.setup_link(hw, old, true);
		}
		clear_bit(__IXGBE_IN_SFP_INIT, &adapter->state);
	}
	else {
		/* in this case we currently only support 10Gb/FULL */
		u32 speed = ethtool_cmd_speed(ecmd);

		if ((ecmd->autoneg == AUTONEG_ENABLE) ||
		    (ecmd->advertising != ADVERTISED_10000baseT_Full) ||
		    (speed + ecmd->duplex != SPEED_10000 + DUPLEX_FULL))
			return -EINVAL;
	}

	return err;
}
#endif /* !ETHTOOL_GLINKSETTINGS */

static void ixgbe_get_pauseparam(struct net_device *netdev,
				 struct ethtool_pauseparam *pause)
{
	struct ixgbe_adapter *adapter = netdev_priv(netdev);
	struct ixgbe_hw *hw = &adapter->hw;

	if (ixgbe_device_supports_autoneg_fc(hw) &&
	    !hw->fc.disable_fc_autoneg)
		pause->autoneg = 1;
	else
		pause->autoneg = 0;

	if (hw->fc.current_mode == ixgbe_fc_rx_pause) {
		pause->rx_pause = 1;
	} else if (hw->fc.current_mode == ixgbe_fc_tx_pause) {
		pause->tx_pause = 1;
	} else if (hw->fc.current_mode == ixgbe_fc_full) {
		pause->rx_pause = 1;
		pause->tx_pause = 1;
	}
}

static int ixgbe_set_pauseparam(struct net_device *netdev,
				struct ethtool_pauseparam *pause)
{
	struct ixgbe_adapter *adapter = netdev_priv(netdev);
	struct ixgbe_hw *hw = &adapter->hw;
	struct ixgbe_fc_info fc = hw->fc;

	/* 82598 does no support link flow control with DCB enabled */
	if ((hw->mac.type == ixgbe_mac_82598EB) &&
	    (adapter->flags & IXGBE_FLAG_DCB_ENABLED))
		return -EINVAL;


	/* some devices do not support autoneg of flow control */
	if ((pause->autoneg == AUTONEG_ENABLE) &&
	    !ixgbe_device_supports_autoneg_fc(hw))
	    return -EINVAL;

	fc.disable_fc_autoneg = (pause->autoneg != AUTONEG_ENABLE);

	if ((pause->rx_pause && pause->tx_pause) || pause->autoneg)
		fc.requested_mode = ixgbe_fc_full;
	else if (pause->rx_pause)
		fc.requested_mode = ixgbe_fc_rx_pause;
	else if (pause->tx_pause)
		fc.requested_mode = ixgbe_fc_tx_pause;
	else
		fc.requested_mode = ixgbe_fc_none;

	/* if the thing changed then we'll update and use new autoneg */
	if (memcmp(&fc, &hw->fc, sizeof(struct ixgbe_fc_info))) {
		hw->fc = fc;
		if (netif_running(netdev))
			ixgbe_reinit_locked(adapter);
		else
			ixgbe_reset(adapter);
	}

	return 0;
}

static u32 ixgbe_get_msglevel(struct net_device *netdev)
{
	struct ixgbe_adapter *adapter = netdev_priv(netdev);
	return adapter->msg_enable;
}

static void ixgbe_set_msglevel(struct net_device *netdev, u32 data)
{
	struct ixgbe_adapter *adapter = netdev_priv(netdev);
	adapter->msg_enable = data;
}

static int ixgbe_get_regs_len(struct net_device __always_unused *netdev)
{
#define IXGBE_REGS_LEN  1145
	return IXGBE_REGS_LEN * sizeof(u32);
}

#define IXGBE_GET_STAT(_A_, _R_)	(_A_->stats._R_)

static void ixgbe_get_regs(struct net_device *netdev, struct ethtool_regs *regs,
			   void *p)
{
	struct ixgbe_adapter *adapter = netdev_priv(netdev);
	struct ixgbe_hw *hw = &adapter->hw;
	u32 *regs_buff = p;
	u8 i;

	memset(p, 0, IXGBE_REGS_LEN * sizeof(u32));

	regs->version = hw->mac.type << 24 | hw->revision_id << 16 |
			hw->device_id;

	/* General Registers */
	regs_buff[0] = IXGBE_R32_Q(hw, IXGBE_CTRL);
	regs_buff[1] = IXGBE_R32_Q(hw, IXGBE_STATUS);
	regs_buff[2] = IXGBE_R32_Q(hw, IXGBE_CTRL_EXT);
	regs_buff[3] = IXGBE_R32_Q(hw, IXGBE_ESDP);
	regs_buff[4] = IXGBE_R32_Q(hw, IXGBE_EODSDP);
	regs_buff[5] = IXGBE_R32_Q(hw, IXGBE_LEDCTL);
	regs_buff[6] = IXGBE_R32_Q(hw, IXGBE_FRTIMER);
	regs_buff[7] = IXGBE_R32_Q(hw, IXGBE_TCPTIMER);

	/* NVM Register */
	regs_buff[8] = IXGBE_R32_Q(hw, IXGBE_EEC);
	regs_buff[9] = IXGBE_R32_Q(hw, IXGBE_EERD);
	regs_buff[10] = IXGBE_R32_Q(hw, IXGBE_FLA);
	regs_buff[11] = IXGBE_R32_Q(hw, IXGBE_EEMNGCTL);
	regs_buff[12] = IXGBE_R32_Q(hw, IXGBE_EEMNGDATA);
	regs_buff[13] = IXGBE_R32_Q(hw, IXGBE_FLMNGCTL);
	regs_buff[14] = IXGBE_R32_Q(hw, IXGBE_FLMNGDATA);
	regs_buff[15] = IXGBE_R32_Q(hw, IXGBE_FLMNGCNT);
	regs_buff[16] = IXGBE_R32_Q(hw, IXGBE_FLOP);
	regs_buff[17] = IXGBE_R32_Q(hw, IXGBE_GRC);

	/* Interrupt */
	/* don't read EICR because it can clear interrupt causes, instead
	 * read EICS which is a shadow but doesn't clear EICR */
	regs_buff[18] = IXGBE_R32_Q(hw, IXGBE_EICS);
	regs_buff[19] = IXGBE_R32_Q(hw, IXGBE_EICS);
	regs_buff[20] = IXGBE_R32_Q(hw, IXGBE_EIMS);
	regs_buff[21] = IXGBE_R32_Q(hw, IXGBE_EIMC);
	regs_buff[22] = IXGBE_R32_Q(hw, IXGBE_EIAC);
	regs_buff[23] = IXGBE_R32_Q(hw, IXGBE_EIAM);
	regs_buff[24] = IXGBE_R32_Q(hw, IXGBE_EITR(0));
	regs_buff[25] = IXGBE_R32_Q(hw, IXGBE_IVAR(0));
	regs_buff[26] = IXGBE_R32_Q(hw, IXGBE_MSIXT);
	regs_buff[27] = IXGBE_R32_Q(hw, IXGBE_MSIXPBA);
	regs_buff[28] = IXGBE_R32_Q(hw, IXGBE_PBACL(0));
	regs_buff[29] = IXGBE_R32_Q(hw, IXGBE_GPIE);

	/* Flow Control */
	regs_buff[30] = IXGBE_R32_Q(hw, IXGBE_PFCTOP);
	regs_buff[31] = IXGBE_R32_Q(hw, IXGBE_FCTTV(0));
	regs_buff[32] = IXGBE_R32_Q(hw, IXGBE_FCTTV(1));
	regs_buff[33] = IXGBE_R32_Q(hw, IXGBE_FCTTV(2));
	regs_buff[34] = IXGBE_R32_Q(hw, IXGBE_FCTTV(3));
	for (i = 0; i < 8; i++) {
		switch (hw->mac.type) {
		case ixgbe_mac_82598EB:
			regs_buff[35 + i] = IXGBE_R32_Q(hw, IXGBE_FCRTL(i));
			regs_buff[43 + i] = IXGBE_R32_Q(hw, IXGBE_FCRTH(i));
			break;
		case ixgbe_mac_82599EB:
		case ixgbe_mac_X540:
		case ixgbe_mac_X550:
		case ixgbe_mac_X550EM_x:
			regs_buff[35 + i] = IXGBE_R32_Q(hw,
							IXGBE_FCRTL_82599(i));
			regs_buff[43 + i] = IXGBE_R32_Q(hw,
							IXGBE_FCRTH_82599(i));
			break;
		default:
			break;
		}
	}
	regs_buff[51] = IXGBE_R32_Q(hw, IXGBE_FCRTV);
	regs_buff[52] = IXGBE_R32_Q(hw, IXGBE_TFCS);

	/* Receive DMA */
	for (i = 0; i < 64; i++)
		regs_buff[53 + i] = IXGBE_R32_Q(hw, IXGBE_RDBAL(i));
	for (i = 0; i < 64; i++)
		regs_buff[117 + i] = IXGBE_R32_Q(hw, IXGBE_RDBAH(i));
	for (i = 0; i < 64; i++)
		regs_buff[181 + i] = IXGBE_R32_Q(hw, IXGBE_RDLEN(i));
	for (i = 0; i < 64; i++)
		regs_buff[245 + i] = IXGBE_R32_Q(hw, IXGBE_RDH(i));
	for (i = 0; i < 64; i++)
		regs_buff[309 + i] = IXGBE_R32_Q(hw, IXGBE_RDT(i));
	for (i = 0; i < 64; i++)
		regs_buff[373 + i] = IXGBE_R32_Q(hw, IXGBE_RXDCTL(i));
	for (i = 0; i < 16; i++)
		regs_buff[437 + i] = IXGBE_R32_Q(hw, IXGBE_SRRCTL(i));
	for (i = 0; i < 16; i++)
		regs_buff[453 + i] = IXGBE_R32_Q(hw, IXGBE_DCA_RXCTRL(i));
	regs_buff[469] = IXGBE_R32_Q(hw, IXGBE_RDRXCTL);
	for (i = 0; i < 8; i++)
		regs_buff[470 + i] = IXGBE_R32_Q(hw, IXGBE_RXPBSIZE(i));
	regs_buff[478] = IXGBE_R32_Q(hw, IXGBE_RXCTRL);
	regs_buff[479] = IXGBE_R32_Q(hw, IXGBE_DROPEN);

	/* Receive */
	regs_buff[480] = IXGBE_R32_Q(hw, IXGBE_RXCSUM);
	regs_buff[481] = IXGBE_R32_Q(hw, IXGBE_RFCTL);
	for (i = 0; i < 16; i++)
		regs_buff[482 + i] = IXGBE_R32_Q(hw, IXGBE_RAL(i));
	for (i = 0; i < 16; i++)
		regs_buff[498 + i] = IXGBE_R32_Q(hw, IXGBE_RAH(i));
	regs_buff[514] = IXGBE_R32_Q(hw, IXGBE_PSRTYPE(0));
	regs_buff[515] = IXGBE_R32_Q(hw, IXGBE_FCTRL);
	regs_buff[516] = IXGBE_R32_Q(hw, IXGBE_VLNCTRL);
	regs_buff[517] = IXGBE_R32_Q(hw, IXGBE_MCSTCTRL);
	regs_buff[518] = IXGBE_R32_Q(hw, IXGBE_MRQC);
	regs_buff[519] = IXGBE_R32_Q(hw, IXGBE_VMD_CTL);
	for (i = 0; i < 8; i++)
		regs_buff[520 + i] = IXGBE_R32_Q(hw, IXGBE_IMIR(i));
	for (i = 0; i < 8; i++)
		regs_buff[528 + i] = IXGBE_R32_Q(hw, IXGBE_IMIREXT(i));
	regs_buff[536] = IXGBE_R32_Q(hw, IXGBE_IMIRVP);

	/* Transmit */
	for (i = 0; i < 32; i++)
		regs_buff[537 + i] = IXGBE_R32_Q(hw, IXGBE_TDBAL(i));
	for (i = 0; i < 32; i++)
		regs_buff[569 + i] = IXGBE_R32_Q(hw, IXGBE_TDBAH(i));
	for (i = 0; i < 32; i++)
		regs_buff[601 + i] = IXGBE_R32_Q(hw, IXGBE_TDLEN(i));
	for (i = 0; i < 32; i++)
		regs_buff[633 + i] = IXGBE_R32_Q(hw, IXGBE_TDH(i));
	for (i = 0; i < 32; i++)
		regs_buff[665 + i] = IXGBE_R32_Q(hw, IXGBE_TDT(i));
	for (i = 0; i < 32; i++)
		regs_buff[697 + i] = IXGBE_R32_Q(hw, IXGBE_TXDCTL(i));
	for (i = 0; i < 32; i++)
		regs_buff[729 + i] = IXGBE_R32_Q(hw, IXGBE_TDWBAL(i));
	for (i = 0; i < 32; i++)
		regs_buff[761 + i] = IXGBE_R32_Q(hw, IXGBE_TDWBAH(i));
	regs_buff[793] = IXGBE_R32_Q(hw, IXGBE_DTXCTL);
	for (i = 0; i < 16; i++)
		regs_buff[794 + i] = IXGBE_R32_Q(hw, IXGBE_DCA_TXCTRL(i));
	regs_buff[810] = IXGBE_R32_Q(hw, IXGBE_TIPG);
	for (i = 0; i < 8; i++)
		regs_buff[811 + i] = IXGBE_R32_Q(hw, IXGBE_TXPBSIZE(i));
	regs_buff[819] = IXGBE_R32_Q(hw, IXGBE_MNGTXMAP);

	/* Wake Up */
	regs_buff[820] = IXGBE_R32_Q(hw, IXGBE_WUC);
	regs_buff[821] = IXGBE_R32_Q(hw, IXGBE_WUFC);
	regs_buff[822] = IXGBE_R32_Q(hw, IXGBE_WUS);
	regs_buff[823] = IXGBE_R32_Q(hw, IXGBE_IPAV);
	regs_buff[824] = IXGBE_R32_Q(hw, IXGBE_IP4AT);
	regs_buff[825] = IXGBE_R32_Q(hw, IXGBE_IP6AT);
	regs_buff[826] = IXGBE_R32_Q(hw, IXGBE_WUPL);
	regs_buff[827] = IXGBE_R32_Q(hw, IXGBE_WUPM);
	regs_buff[828] = IXGBE_R32_Q(hw, IXGBE_FHFT(0));

	/* DCB */
	regs_buff[829] = IXGBE_R32_Q(hw, IXGBE_RMCS);   /* same as FCCFG  */
	regs_buff[831] = IXGBE_R32_Q(hw, IXGBE_PDPMCS); /* same as RTTPCS */

	switch (hw->mac.type) {
	case ixgbe_mac_82598EB:
		regs_buff[830] = IXGBE_R32_Q(hw, IXGBE_DPMCS);
		regs_buff[832] = IXGBE_R32_Q(hw, IXGBE_RUPPBMR);
		for (i = 0; i < 8; i++)
			regs_buff[833 + i] =
				IXGBE_R32_Q(hw, IXGBE_RT2CR(i));
		for (i = 0; i < 8; i++)
			regs_buff[841 + i] =
				IXGBE_R32_Q(hw, IXGBE_RT2SR(i));
		for (i = 0; i < 8; i++)
			regs_buff[849 + i] =
				IXGBE_R32_Q(hw, IXGBE_TDTQ2TCCR(i));
		for (i = 0; i < 8; i++)
			regs_buff[857 + i] =
				IXGBE_R32_Q(hw, IXGBE_TDTQ2TCSR(i));
		break;
	case ixgbe_mac_82599EB:
	case ixgbe_mac_X540:
		regs_buff[830] = IXGBE_R32_Q(hw, IXGBE_RTTDCS);
		regs_buff[832] = IXGBE_R32_Q(hw, IXGBE_RTRPCS);
		for (i = 0; i < 8; i++)
			regs_buff[833 + i] =
				IXGBE_R32_Q(hw, IXGBE_RTRPT4C(i));
		for (i = 0; i < 8; i++)
			regs_buff[841 + i] =
				IXGBE_R32_Q(hw, IXGBE_RTRPT4S(i));
		for (i = 0; i < 8; i++)
			regs_buff[849 + i] =
				IXGBE_R32_Q(hw, IXGBE_RTTDT2C(i));
		for (i = 0; i < 8; i++)
			regs_buff[857 + i] =
				IXGBE_R32_Q(hw, IXGBE_RTTDT2S(i));
		break;
	default:
		break;
	}

	for (i = 0; i < 8; i++)
		regs_buff[865 + i] =
		IXGBE_R32_Q(hw, IXGBE_TDPT2TCCR(i)); /* same as RTTPT2C */
	for (i = 0; i < 8; i++)
		regs_buff[873 + i] =
		IXGBE_R32_Q(hw, IXGBE_TDPT2TCSR(i)); /* same as RTTPT2S */

	/* Statistics */
	regs_buff[881] = IXGBE_GET_STAT(adapter, crcerrs);
	regs_buff[882] = IXGBE_GET_STAT(adapter, illerrc);
	regs_buff[883] = IXGBE_GET_STAT(adapter, errbc);
	regs_buff[884] = IXGBE_GET_STAT(adapter, mspdc);
	for (i = 0; i < 8; i++)
		regs_buff[885 + i] = IXGBE_GET_STAT(adapter, mpc[i]);
	regs_buff[893] = IXGBE_GET_STAT(adapter, mlfc);
	regs_buff[894] = IXGBE_GET_STAT(adapter, mrfc);
	regs_buff[895] = IXGBE_GET_STAT(adapter, rlec);
	regs_buff[896] = IXGBE_GET_STAT(adapter, lxontxc);
	regs_buff[897] = IXGBE_GET_STAT(adapter, lxonrxc);
	regs_buff[898] = IXGBE_GET_STAT(adapter, lxofftxc);
	regs_buff[899] = IXGBE_GET_STAT(adapter, lxoffrxc);
	for (i = 0; i < 8; i++)
		regs_buff[900 + i] = IXGBE_GET_STAT(adapter, pxontxc[i]);
	for (i = 0; i < 8; i++)
		regs_buff[908 + i] = IXGBE_GET_STAT(adapter, pxonrxc[i]);
	for (i = 0; i < 8; i++)
		regs_buff[916 + i] = IXGBE_GET_STAT(adapter, pxofftxc[i]);
	for (i = 0; i < 8; i++)
		regs_buff[924 + i] = IXGBE_GET_STAT(adapter, pxoffrxc[i]);
	regs_buff[932] = IXGBE_GET_STAT(adapter, prc64);
	regs_buff[933] = IXGBE_GET_STAT(adapter, prc127);
	regs_buff[934] = IXGBE_GET_STAT(adapter, prc255);
	regs_buff[935] = IXGBE_GET_STAT(adapter, prc511);
	regs_buff[936] = IXGBE_GET_STAT(adapter, prc1023);
	regs_buff[937] = IXGBE_GET_STAT(adapter, prc1522);
	regs_buff[938] = IXGBE_GET_STAT(adapter, gprc);
	regs_buff[939] = IXGBE_GET_STAT(adapter, bprc);
	regs_buff[940] = IXGBE_GET_STAT(adapter, mprc);
	regs_buff[941] = IXGBE_GET_STAT(adapter, gptc);
	regs_buff[942] = IXGBE_GET_STAT(adapter, gorc);
	regs_buff[944] = IXGBE_GET_STAT(adapter, gotc);
	for (i = 0; i < 8; i++)
		regs_buff[946 + i] = IXGBE_GET_STAT(adapter, rnbc[i]);
	regs_buff[954] = IXGBE_GET_STAT(adapter, ruc);
	regs_buff[955] = IXGBE_GET_STAT(adapter, rfc);
	regs_buff[956] = IXGBE_GET_STAT(adapter, roc);
	regs_buff[957] = IXGBE_GET_STAT(adapter, rjc);
	regs_buff[958] = IXGBE_GET_STAT(adapter, mngprc);
	regs_buff[959] = IXGBE_GET_STAT(adapter, mngpdc);
	regs_buff[960] = IXGBE_GET_STAT(adapter, mngptc);
	regs_buff[961] = IXGBE_GET_STAT(adapter, tor);
	regs_buff[963] = IXGBE_GET_STAT(adapter, tpr);
	regs_buff[964] = IXGBE_GET_STAT(adapter, tpt);
	regs_buff[965] = IXGBE_GET_STAT(adapter, ptc64);
	regs_buff[966] = IXGBE_GET_STAT(adapter, ptc127);
	regs_buff[967] = IXGBE_GET_STAT(adapter, ptc255);
	regs_buff[968] = IXGBE_GET_STAT(adapter, ptc511);
	regs_buff[969] = IXGBE_GET_STAT(adapter, ptc1023);
	regs_buff[970] = IXGBE_GET_STAT(adapter, ptc1522);
	regs_buff[971] = IXGBE_GET_STAT(adapter, mptc);
	regs_buff[972] = IXGBE_GET_STAT(adapter, bptc);
	regs_buff[973] = IXGBE_GET_STAT(adapter, xec);
	for (i = 0; i < 16; i++)
		regs_buff[974 + i] = IXGBE_GET_STAT(adapter, qprc[i]);
	for (i = 0; i < 16; i++)
		regs_buff[990 + i] = IXGBE_GET_STAT(adapter, qptc[i]);
	for (i = 0; i < 16; i++)
		regs_buff[1006 + i] = IXGBE_GET_STAT(adapter, qbrc[i]);
	for (i = 0; i < 16; i++)
		regs_buff[1022 + i] = IXGBE_GET_STAT(adapter, qbtc[i]);

	/* MAC */
	regs_buff[1038] = IXGBE_R32_Q(hw, IXGBE_PCS1GCFIG);
	regs_buff[1039] = IXGBE_R32_Q(hw, IXGBE_PCS1GLCTL);
	regs_buff[1040] = IXGBE_R32_Q(hw, IXGBE_PCS1GLSTA);
	regs_buff[1041] = IXGBE_R32_Q(hw, IXGBE_PCS1GDBG0);
	regs_buff[1042] = IXGBE_R32_Q(hw, IXGBE_PCS1GDBG1);
	regs_buff[1043] = IXGBE_R32_Q(hw, IXGBE_PCS1GANA);
	regs_buff[1044] = IXGBE_R32_Q(hw, IXGBE_PCS1GANLP);
	regs_buff[1045] = IXGBE_R32_Q(hw, IXGBE_PCS1GANNP);
	regs_buff[1046] = IXGBE_R32_Q(hw, IXGBE_PCS1GANLPNP);
	regs_buff[1047] = IXGBE_R32_Q(hw, IXGBE_HLREG0);
	regs_buff[1048] = IXGBE_R32_Q(hw, IXGBE_HLREG1);
	regs_buff[1049] = IXGBE_R32_Q(hw, IXGBE_PAP);
	regs_buff[1050] = IXGBE_R32_Q(hw, IXGBE_MACA);
	regs_buff[1051] = IXGBE_R32_Q(hw, IXGBE_APAE);
	regs_buff[1052] = IXGBE_R32_Q(hw, IXGBE_ARD);
	regs_buff[1053] = IXGBE_R32_Q(hw, IXGBE_AIS);
	regs_buff[1054] = IXGBE_R32_Q(hw, IXGBE_MSCA);
	regs_buff[1055] = IXGBE_R32_Q(hw, IXGBE_MSRWD);
	regs_buff[1056] = IXGBE_R32_Q(hw, IXGBE_MLADD);
	regs_buff[1057] = IXGBE_R32_Q(hw, IXGBE_MHADD);
	regs_buff[1058] = IXGBE_R32_Q(hw, IXGBE_TREG);
	regs_buff[1059] = IXGBE_R32_Q(hw, IXGBE_PCSS1);
	regs_buff[1060] = IXGBE_R32_Q(hw, IXGBE_PCSS2);
	regs_buff[1061] = IXGBE_R32_Q(hw, IXGBE_XPCSS);
	regs_buff[1062] = IXGBE_R32_Q(hw, IXGBE_SERDESC);
	regs_buff[1063] = IXGBE_R32_Q(hw, IXGBE_MACS);
	regs_buff[1064] = IXGBE_R32_Q(hw, IXGBE_AUTOC);
	regs_buff[1065] = IXGBE_R32_Q(hw, IXGBE_LINKS);
	regs_buff[1066] = IXGBE_R32_Q(hw, IXGBE_AUTOC2);
	regs_buff[1067] = IXGBE_R32_Q(hw, IXGBE_AUTOC3);
	regs_buff[1068] = IXGBE_R32_Q(hw, IXGBE_ANLP1);
	regs_buff[1069] = IXGBE_R32_Q(hw, IXGBE_ANLP2);
	regs_buff[1070] = IXGBE_R32_Q(hw, IXGBE_ATLASCTL);

	/* Diagnostic */
	regs_buff[1071] = IXGBE_R32_Q(hw, IXGBE_RDSTATCTL);
	for (i = 0; i < 8; i++)
		regs_buff[1072 + i] = IXGBE_R32_Q(hw, IXGBE_RDSTAT(i));
	regs_buff[1080] = IXGBE_R32_Q(hw, IXGBE_RDHMPN);
	for (i = 0; i < 4; i++)
		regs_buff[1081 + i] = IXGBE_R32_Q(hw, IXGBE_RIC_DW(i));
	regs_buff[1085] = IXGBE_R32_Q(hw, IXGBE_RDPROBE);
	regs_buff[1095] = IXGBE_R32_Q(hw, IXGBE_TDHMPN);
	for (i = 0; i < 4; i++)
		regs_buff[1096 + i] = IXGBE_R32_Q(hw, IXGBE_TIC_DW(i));
	regs_buff[1100] = IXGBE_R32_Q(hw, IXGBE_TDPROBE);
	regs_buff[1101] = IXGBE_R32_Q(hw, IXGBE_TXBUFCTRL);
	regs_buff[1102] = IXGBE_R32_Q(hw, IXGBE_TXBUFDATA0);
	regs_buff[1103] = IXGBE_R32_Q(hw, IXGBE_TXBUFDATA1);
	regs_buff[1104] = IXGBE_R32_Q(hw, IXGBE_TXBUFDATA2);
	regs_buff[1105] = IXGBE_R32_Q(hw, IXGBE_TXBUFDATA3);
	regs_buff[1106] = IXGBE_R32_Q(hw, IXGBE_RXBUFCTRL);
	regs_buff[1107] = IXGBE_R32_Q(hw, IXGBE_RXBUFDATA0);
	regs_buff[1108] = IXGBE_R32_Q(hw, IXGBE_RXBUFDATA1);
	regs_buff[1109] = IXGBE_R32_Q(hw, IXGBE_RXBUFDATA2);
	regs_buff[1110] = IXGBE_R32_Q(hw, IXGBE_RXBUFDATA3);
	for (i = 0; i < 8; i++)
		regs_buff[1111 + i] = IXGBE_R32_Q(hw, IXGBE_PCIE_DIAG(i));
	regs_buff[1119] = IXGBE_R32_Q(hw, IXGBE_RFVAL);
	regs_buff[1120] = IXGBE_R32_Q(hw, IXGBE_MDFTC1);
	regs_buff[1121] = IXGBE_R32_Q(hw, IXGBE_MDFTC2);
	regs_buff[1122] = IXGBE_R32_Q(hw, IXGBE_MDFTFIFO1);
	regs_buff[1123] = IXGBE_R32_Q(hw, IXGBE_MDFTFIFO2);
	regs_buff[1124] = IXGBE_R32_Q(hw, IXGBE_MDFTS);
	regs_buff[1125] = IXGBE_R32_Q(hw, IXGBE_PCIEECCCTL);
	regs_buff[1126] = IXGBE_R32_Q(hw, IXGBE_PBTXECC);
	regs_buff[1127] = IXGBE_R32_Q(hw, IXGBE_PBRXECC);

	/* 82599 X540 specific registers */
	regs_buff[1128] = IXGBE_R32_Q(hw, IXGBE_MFLCN);

	/* 82599 X540 specific DCB registers */
	regs_buff[1129] = IXGBE_R32_Q(hw, IXGBE_RTRUP2TC);
	regs_buff[1130] = IXGBE_R32_Q(hw, IXGBE_RTTUP2TC);
	for (i = 0; i < 4; i++)
		regs_buff[1131 + i] = IXGBE_R32_Q(hw, IXGBE_TXLLQ(i));
	regs_buff[1135] = IXGBE_R32_Q(hw, IXGBE_RTTBCNRM);
				     /* same as RTTQCNRM */
	regs_buff[1136] = IXGBE_R32_Q(hw, IXGBE_RTTBCNRD);
				     /* same as RTTQCNRR */

	/* X540 specific DCB registers */
	regs_buff[1137] = IXGBE_R32_Q(hw, IXGBE_RTTQCNCR);
	regs_buff[1138] = IXGBE_R32_Q(hw, IXGBE_RTTQCNTG);

	/* Security config registers */
	regs_buff[1139] = IXGBE_R32_Q(hw, IXGBE_SECTXCTRL);
	regs_buff[1140] = IXGBE_R32_Q(hw, IXGBE_SECTXSTAT);
	regs_buff[1141] = IXGBE_R32_Q(hw, IXGBE_SECTXBUFFAF);
	regs_buff[1142] = IXGBE_R32_Q(hw, IXGBE_SECTXMINIFG);
	regs_buff[1143] = IXGBE_R32_Q(hw, IXGBE_SECRXCTRL);
	regs_buff[1144] = IXGBE_R32_Q(hw, IXGBE_SECRXSTAT);

}

static int ixgbe_get_eeprom_len(struct net_device *netdev)
{
	struct ixgbe_adapter *adapter = netdev_priv(netdev);
	return pci_resource_len(adapter->pdev, 0);
}

static u8 ixgbe_nvmupd_get_module(u32 val)
{
	return (u8)(val & IXGBE_NVMUPD_MOD_PNT_MASK);
}

static int ixgbe_nvmupd_validate_offset(struct ixgbe_adapter *adapter,
					u32 offset)
{
	struct net_device *netdev = adapter->netdev;

	switch (offset) {
	case IXGBE_STATUS:
	case IXGBE_ESDP:
	case IXGBE_MSCA:
	case IXGBE_MSRWD:
	case IXGBE_EEC:
	case IXGBE_FLA:
	case IXGBE_FLOP:
	case IXGBE_SWSM:
	case IXGBE_FWSM:
	case IXGBE_FACTPS:
	case IXGBE_GSSR:
	case IXGBE_HICR:
	case IXGBE_FWSTS:
		return 0;
	default:
		if ((offset >= IXGBE_MAVTV(0) && offset <= IXGBE_MAVTV(7)) ||
		    (offset >= IXGBE_RAL(0) && offset <= IXGBE_RAH(15)))
			return 0;
	}

	switch (adapter->hw.mac.type) {
	case ixgbe_mac_82599EB:
		switch (offset) {
		case IXGBE_AUTOC:
		case IXGBE_EERD:
		case IXGBE_BARCTRL:
			return 0;
		default:
			if (offset >= 0x00020000 &&
			    offset <= ixgbe_get_eeprom_len(netdev))
				return 0;
		}
		break;
	case ixgbe_mac_X540:
		switch (offset) {
		case IXGBE_EERD:
		case IXGBE_EEWR:
		case IXGBE_SRAMREL:
		case IXGBE_BARCTRL:
			return 0;
		default:
			if ((offset >= 0x00020000 &&
			     offset <= ixgbe_get_eeprom_len(netdev)))
				return 0;
		}
		break;
	case ixgbe_mac_X550:
		switch (offset) {
		case IXGBE_EEWR:
		case IXGBE_SRAMREL:
		case IXGBE_PHYCTL_82599:
		case IXGBE_FWRESETCNT:
			return 0;
		default:
			if (offset >= IXGBE_FLEX_MNG_PTR(0) &&
			    offset <= IXGBE_FLEX_MNG_PTR(447))
				return 0;
		}
		break;
	case ixgbe_mac_X550EM_x:
		switch (offset) {
		case IXGBE_PHYCTL_82599:
		case IXGBE_NW_MNG_IF_SEL:
		case IXGBE_FWRESETCNT:
		case IXGBE_I2CCTL_X550:
			return 0;
		default:
			if ((offset >= IXGBE_FLEX_MNG_PTR(0) &&
			     offset <= IXGBE_FLEX_MNG_PTR(447)) ||
			    (offset >= IXGBE_FUSES0_GROUP(0) &&
			     offset <= IXGBE_FUSES0_GROUP(7)))
				return 0;
		}
		break;
	case ixgbe_mac_X550EM_a:
		switch (offset) {
		case IXGBE_PHYCTL_82599:
		case IXGBE_NW_MNG_IF_SEL:
		case IXGBE_FWRESETCNT:
		case IXGBE_I2CCTL_X550:
		case IXGBE_FLA_X550EM_a:
		case IXGBE_SWSM_X550EM_a:
		case IXGBE_FWSM_X550EM_a:
		case IXGBE_SWFW_SYNC_X550EM_a:
		case IXGBE_FACTPS_X550EM_a:
		case IXGBE_EEC_X550EM_a:
			return 0;
		default:
			if (offset >= IXGBE_FLEX_MNG_PTR(0) &&
			    offset <= IXGBE_FLEX_MNG_PTR(447))
				return 0;
		}
	default:
		break;
	}

	return -ENOTTY;
}

static int ixgbe_nvmupd_command(struct ixgbe_hw *hw,
				struct ixgbe_nvm_access *nvm,
				u8 *bytes)
{
	u32 command;
	int ret_val = 0;
	u8 module;

	command = nvm->command;
	module = ixgbe_nvmupd_get_module(nvm->config);

	switch (command) {
	case IXGBE_NVMUPD_CMD_REG_READ:
		switch (module) {
		case IXGBE_NVMUPD_EXEC_FEATURES:
			if (nvm->data_size == hw->nvmupd_features.size)
				memcpy(bytes, &hw->nvmupd_features,
				       hw->nvmupd_features.size);
			else
				ret_val = -ENOMEM;
		break;
		default:
			if (ixgbe_nvmupd_validate_offset(hw->back, nvm->offset))
				return -ENOTTY;

			if (nvm->data_size == 1)
				*((u8 *)bytes) = IXGBE_R8_Q(hw, nvm->offset);
			else
				*((u32 *)bytes) = IXGBE_R32_Q(hw, nvm->offset);
		break;
		}
	break;
	case IXGBE_NVMUPD_CMD_REG_WRITE:
		if (ixgbe_nvmupd_validate_offset(hw->back, nvm->offset))
			return -ENOTTY;

		IXGBE_WRITE_REG(hw, nvm->offset, *((u32 *)bytes));
	break;
	}

	return ret_val;
}

static int ixgbe_get_eeprom(struct net_device *netdev,
			    struct ethtool_eeprom *eeprom, u8 *bytes)
{
	struct ixgbe_adapter *adapter = netdev_priv(netdev);
	struct ixgbe_hw *hw = &adapter->hw;
	u16 *eeprom_buff;
	int first_word, last_word, eeprom_len;
	struct ixgbe_nvm_access *nvm;
	u32 magic;
	int ret_val = 0;
	u16 i;

	if (eeprom->len == 0)
		return -EINVAL;

	magic = hw->vendor_id | (hw->device_id << 16);
	if (eeprom->magic && eeprom->magic != magic) {
		nvm = (struct ixgbe_nvm_access *)eeprom;
		ret_val = ixgbe_nvmupd_command(hw, nvm, bytes);
		return ret_val;
	}

	/* normal ethtool get_eeprom support */
	eeprom->magic = hw->vendor_id | (hw->device_id << 16);

	first_word = eeprom->offset >> 1;
	last_word = (eeprom->offset + eeprom->len - 1) >> 1;
	eeprom_len = last_word - first_word + 1;

	eeprom_buff = kmalloc(sizeof(u16) * eeprom_len, GFP_KERNEL);
	if (!eeprom_buff)
		return -ENOMEM;

	ret_val = hw->eeprom.ops.read_buffer(hw, first_word, eeprom_len,
					   eeprom_buff);

	/* Device's eeprom is always little-endian, word addressable */
	for (i = 0; i < eeprom_len; i++)
		le16_to_cpus(&eeprom_buff[i]);

	memcpy(bytes, (u8 *)eeprom_buff + (eeprom->offset & 1), eeprom->len);
	kfree(eeprom_buff);

	return ret_val;
}

static int ixgbe_set_eeprom(struct net_device *netdev,
			    struct ethtool_eeprom *eeprom, u8 *bytes)
{
	struct ixgbe_adapter *adapter = netdev_priv(netdev);
	struct ixgbe_hw *hw = &adapter->hw;
	int max_len, first_word, last_word, ret_val = 0;
	struct ixgbe_nvm_access *nvm;
	u32 magic;
	u16 *eeprom_buff, i;
	void *ptr;

	if (eeprom->len == 0)
		return -EINVAL;

	magic = hw->vendor_id | (hw->device_id << 16);
	if (eeprom->magic && eeprom->magic != magic) {
		nvm = (struct ixgbe_nvm_access *)eeprom;
		ret_val = ixgbe_nvmupd_command(hw, nvm, bytes);
		return ret_val;
	}

	/* normal ethtool set_eeprom support */

	if (eeprom->magic != (hw->vendor_id | (hw->device_id << 16)))
		return -EINVAL;

	max_len = hw->eeprom.word_size * 2;

	first_word = eeprom->offset >> 1;
	last_word = (eeprom->offset + eeprom->len - 1) >> 1;
	eeprom_buff = kmalloc(max_len, GFP_KERNEL);
	if (!eeprom_buff)
		return -ENOMEM;

	ptr = eeprom_buff;

	if (eeprom->offset & 1) {
		/*
		 * need read/modify/write of first changed EEPROM word
		 * only the second byte of the word is being modified
		 */
		ret_val = hw->eeprom.ops.read(hw, first_word, &eeprom_buff[0]);
		if (ret_val)
			goto err;

		ptr++;
	}
	if (((eeprom->offset + eeprom->len) & 1) && (ret_val == 0)) {
		/*
		 * need read/modify/write of last changed EEPROM word
		 * only the first byte of the word is being modified
		 */
		ret_val = hw->eeprom.ops.read(hw, last_word,
					  &eeprom_buff[last_word - first_word]);
		if (ret_val)
			goto err;
	}

	/* Device's eeprom is always little-endian, word addressable */
	for (i = 0; i < last_word - first_word + 1; i++)
		le16_to_cpus(&eeprom_buff[i]);

	memcpy(ptr, bytes, eeprom->len);

	for (i = 0; i < last_word - first_word + 1; i++)
		cpu_to_le16s(&eeprom_buff[i]);

	ret_val = hw->eeprom.ops.write_buffer(hw, first_word,
					    last_word - first_word + 1,
					    eeprom_buff);

	/* Update the checksum */
	if (ret_val == 0)
		hw->eeprom.ops.update_checksum(hw);

err:
	kfree(eeprom_buff);
	return ret_val;
}

static void ixgbe_get_drvinfo(struct net_device *netdev,
			      struct ethtool_drvinfo *drvinfo)
{
	struct ixgbe_adapter *adapter = netdev_priv(netdev);

	strlcpy(drvinfo->driver, ixgbe_driver_name,
		sizeof(drvinfo->driver));
	strlcpy(drvinfo->version, ixgbe_driver_version,
		sizeof(drvinfo->version));

	strlcpy(drvinfo->fw_version, adapter->eeprom_id,
		sizeof(drvinfo->fw_version));
	strlcpy(drvinfo->bus_info, pci_name(adapter->pdev),
		sizeof(drvinfo->bus_info));
#ifdef HAVE_ETHTOOL_GET_SSET_COUNT

	drvinfo->n_priv_flags = IXGBE_PRIV_FLAGS_STR_LEN;
#endif
}

static void ixgbe_get_ringparam(struct net_device *netdev,
				struct ethtool_ringparam *ring)
{
	struct ixgbe_adapter *adapter = netdev_priv(netdev);

	ring->rx_max_pending = IXGBE_MAX_RXD;
	ring->tx_max_pending = IXGBE_MAX_TXD;
	ring->rx_mini_max_pending = 0;
	ring->rx_jumbo_max_pending = 0;
	ring->rx_pending = adapter->rx_ring_count;
	ring->tx_pending = adapter->tx_ring_count;
	ring->rx_mini_pending = 0;
	ring->rx_jumbo_pending = 0;
}

static int ixgbe_set_ringparam(struct net_device *netdev,
			       struct ethtool_ringparam *ring)
{
	struct ixgbe_adapter *adapter = netdev_priv(netdev);
	struct ixgbe_ring *temp_ring;
	int i, j, err = 0;
	u32 new_rx_count, new_tx_count;

	if ((ring->rx_mini_pending) || (ring->rx_jumbo_pending))
		return -EINVAL;

	new_tx_count = clamp_t(u32, ring->tx_pending,
			       IXGBE_MIN_TXD, IXGBE_MAX_TXD);
	new_tx_count = ALIGN(new_tx_count, IXGBE_REQ_TX_DESCRIPTOR_MULTIPLE);

	new_rx_count = clamp_t(u32, ring->rx_pending,
			       IXGBE_MIN_RXD, IXGBE_MAX_RXD);
	new_rx_count = ALIGN(new_rx_count, IXGBE_REQ_RX_DESCRIPTOR_MULTIPLE);

	if ((new_tx_count == adapter->tx_ring_count) &&
	    (new_rx_count == adapter->rx_ring_count)) {
		/* nothing to do */
		return 0;
	}

	while (test_and_set_bit(__IXGBE_RESETTING, &adapter->state))
		usleep_range(1000, 2000);

	if (!netif_running(adapter->netdev)) {
		for (i = 0; i < adapter->num_tx_queues; i++)
			adapter->tx_ring[i]->count = new_tx_count;
		for (i = 0; i < adapter->num_xdp_queues; i++)
			adapter->xdp_ring[i]->count = new_tx_count;
		for (i = 0; i < adapter->num_rx_queues; i++)
			adapter->rx_ring[i]->count = new_rx_count;
		adapter->tx_ring_count = new_tx_count;
		adapter->xdp_ring_count = new_tx_count;
		adapter->rx_ring_count = new_rx_count;
		goto clear_reset;
	}

	/* allocate temporary buffer to store rings in */
	i = max_t(int, adapter->num_tx_queues + adapter->num_xdp_queues,
		  adapter->num_rx_queues);
	temp_ring = vmalloc(i * sizeof(struct ixgbe_ring));

	if (!temp_ring) {
		err = -ENOMEM;
		goto clear_reset;
	}

	ixgbe_down(adapter);

	/*
	 * Setup new Tx resources and free the old Tx resources in that order.
	 * We can then assign the new resources to the rings via a memcpy.
	 * The advantage to this approach is that we are guaranteed to still
	 * have resources even in the case of an allocation failure.
	 */
	if (new_tx_count != adapter->tx_ring_count) {
		for (i = 0; i < adapter->num_tx_queues; i++) {
			memcpy(&temp_ring[i], adapter->tx_ring[i],
			       sizeof(struct ixgbe_ring));

			temp_ring[i].count = new_tx_count;
			err = ixgbe_setup_tx_resources(&temp_ring[i]);
			if (err) {
				while (i) {
					i--;
					ixgbe_free_tx_resources(&temp_ring[i]);
				}
				goto err_setup;
			}
		}

		for (j = 0; j < adapter->num_xdp_queues; j++, i++) {
			memcpy(&temp_ring[i], adapter->xdp_ring[j],
			       sizeof(struct ixgbe_ring));

			temp_ring[i].count = new_tx_count;
			err = ixgbe_setup_tx_resources(&temp_ring[i]);
			if (err) {
				while (i) {
					i--;
					ixgbe_free_tx_resources(&temp_ring[i]);
				}
				goto err_setup;
			}
		}

		for (i = 0; i < adapter->num_tx_queues; i++) {
			ixgbe_free_tx_resources(adapter->tx_ring[i]);

			memcpy(adapter->tx_ring[i], &temp_ring[i],
			       sizeof(struct ixgbe_ring));
		}
		for (j = 0; j < adapter->num_xdp_queues; j++, i++) {
			ixgbe_free_tx_resources(adapter->xdp_ring[j]);

			memcpy(adapter->xdp_ring[j], &temp_ring[i],
			       sizeof(struct ixgbe_ring));
		}

		adapter->tx_ring_count = new_tx_count;
	}

	/* Repeat the process for the Rx rings if needed */
	if (new_rx_count != adapter->rx_ring_count) {
		for (i = 0; i < adapter->num_rx_queues; i++) {
			memcpy(&temp_ring[i], adapter->rx_ring[i],
			       sizeof(struct ixgbe_ring));
#ifdef HAVE_XDP_BUFF_RXQ

			/* Clear copied XDP RX-queue info */
			memset(&temp_ring[i].xdp_rxq, 0,
			       sizeof(temp_ring[i].xdp_rxq));
#endif

			temp_ring[i].count = new_rx_count;
			err = ixgbe_setup_rx_resources(adapter, &temp_ring[i]);
			if (err) {
				while (i) {
					i--;
					ixgbe_free_rx_resources(&temp_ring[i]);
				}
				goto err_setup;
			}
		}


		for (i = 0; i < adapter->num_rx_queues; i++) {
			ixgbe_free_rx_resources(adapter->rx_ring[i]);

			memcpy(adapter->rx_ring[i], &temp_ring[i],
			       sizeof(struct ixgbe_ring));
		}

		adapter->rx_ring_count = new_rx_count;
	}

err_setup:
	ixgbe_up(adapter);
	vfree(temp_ring);
clear_reset:
	clear_bit(__IXGBE_RESETTING, &adapter->state);
	return err;
}

#ifndef HAVE_ETHTOOL_GET_SSET_COUNT
static int ixgbe_get_stats_count(struct net_device *netdev)
{
	return IXGBE_STATS_LEN;
}

#else /* HAVE_ETHTOOL_GET_SSET_COUNT */
static int ixgbe_get_sset_count(struct net_device *netdev, int sset)
{
#ifdef HAVE_TX_MQ
#ifndef HAVE_NETDEV_SELECT_QUEUE
	struct ixgbe_adapter *adapter = netdev_priv(netdev);
#endif
#endif

	switch (sset) {
	case ETH_SS_TEST:
		return IXGBE_TEST_LEN;
	case ETH_SS_STATS:
		return IXGBE_STATS_LEN;
	case ETH_SS_PRIV_FLAGS:
		return IXGBE_PRIV_FLAGS_STR_LEN;
	default:
		return -EOPNOTSUPP;
	}
}

#endif /* HAVE_ETHTOOL_GET_SSET_COUNT */
static void ixgbe_get_ethtool_stats(struct net_device *netdev,
				    struct ethtool_stats __always_unused *stats, u64 *data)
{
	struct ixgbe_adapter *adapter = netdev_priv(netdev);
#ifdef HAVE_NETDEV_STATS_IN_NETDEV
	struct net_device_stats *net_stats = &netdev->stats;
#else
	struct net_device_stats *net_stats = &adapter->net_stats;
#endif
	u64 *queue_stat;
	int stat_count, k;
#ifdef HAVE_NDO_GET_STATS64
	unsigned int start;
#endif
	struct ixgbe_ring *ring;
	int i, j;
	char *p;

	ixgbe_update_stats(adapter);

	for (i = 0; i < IXGBE_NETDEV_STATS_LEN; i++) {
		p = (char *)net_stats + ixgbe_gstrings_net_stats[i].stat_offset;
		data[i] = (ixgbe_gstrings_net_stats[i].sizeof_stat ==
			sizeof(u64)) ? *(u64 *)p : *(u32 *)p;
	}
	for (j = 0; j < IXGBE_GLOBAL_STATS_LEN; j++, i++) {
		p = (char *)adapter + ixgbe_gstrings_stats[j].stat_offset;
		data[i] = (ixgbe_gstrings_stats[j].sizeof_stat ==
			   sizeof(u64)) ? *(u64 *)p : *(u32 *)p;
	}
	for (j = 0; j < IXGBE_NUM_TX_QUEUES; j++) {
		ring = adapter->tx_ring[j];
		if (!ring) {
			data[i++] = 0;
			data[i++] = 0;
#ifdef BP_EXTENDED_STATS
			data[i++] = 0;
			data[i++] = 0;
			data[i++] = 0;
#endif
			continue;
		}

#ifdef HAVE_NDO_GET_STATS64
		do {
			start = u64_stats_fetch_begin_irq(&ring->syncp);
#endif
			data[i]   = ring->stats.packets;
			data[i+1] = ring->stats.bytes;
#ifdef HAVE_NDO_GET_STATS64
		} while (u64_stats_fetch_retry_irq(&ring->syncp, start));
#endif
		i += 2;
#ifdef BP_EXTENDED_STATS
		data[i] = ring->stats.yields;
		data[i+1] = ring->stats.misses;
		data[i+2] = ring->stats.cleaned;
		i += 3;
#endif
	}
	for (j = 0; j < IXGBE_NUM_RX_QUEUES; j++) {
		ring = adapter->rx_ring[j];
		if (!ring) {
			data[i++] = 0;
			data[i++] = 0;
#ifdef BP_EXTENDED_STATS
			data[i++] = 0;
			data[i++] = 0;
			data[i++] = 0;
#endif
			continue;
		}

#ifdef HAVE_NDO_GET_STATS64
		do {
			start = u64_stats_fetch_begin_irq(&ring->syncp);
#endif
			data[i]   = ring->stats.packets;
			data[i+1] = ring->stats.bytes;
#ifdef HAVE_NDO_GET_STATS64
		} while (u64_stats_fetch_retry_irq(&ring->syncp, start));
#endif
		i += 2;
#ifdef BP_EXTENDED_STATS
		data[i] = ring->stats.yields;
		data[i+1] = ring->stats.misses;
		data[i+2] = ring->stats.cleaned;
		i += 3;
#endif
	}
	for (j = 0; j < IXGBE_MAX_PACKET_BUFFERS; j++) {
		data[i++] = adapter->stats.pxontxc[j];
		data[i++] = adapter->stats.pxofftxc[j];
	}
	for (j = 0; j < IXGBE_MAX_PACKET_BUFFERS; j++) {
		data[i++] = adapter->stats.pxonrxc[j];
		data[i++] = adapter->stats.pxoffrxc[j];
	}
	stat_count = sizeof(struct vf_stats) / sizeof(u64);
	for (j = 0; j < adapter->num_vfs; j++) {
		queue_stat = (u64 *)&adapter->vfinfo[j].vfstats;
		for (k = 0; k < stat_count; k++)
			data[i + k] = queue_stat[k];
		queue_stat = (u64 *)&adapter->vfinfo[j].saved_rst_vfstats;
		for (k = 0; k < stat_count; k++)
			data[i + k] += queue_stat[k];
		i += k;
	}
}

static void ixgbe_get_strings(struct net_device *netdev, u32 stringset,
			      u8 *data)
{
	struct ixgbe_adapter *adapter = netdev_priv(netdev);
	char *p = (char *)data;
	unsigned int i;

	switch (stringset) {
	case ETH_SS_TEST:
		memcpy(data, *ixgbe_gstrings_test,
		       IXGBE_TEST_LEN * ETH_GSTRING_LEN);
		break;
	case ETH_SS_STATS:
		for (i = 0; i < IXGBE_NETDEV_STATS_LEN; i++) {
			memcpy(p, ixgbe_gstrings_net_stats[i].stat_string,
			       ETH_GSTRING_LEN);
			p += ETH_GSTRING_LEN;
		}
		for (i = 0; i < IXGBE_GLOBAL_STATS_LEN; i++) {
			memcpy(p, ixgbe_gstrings_stats[i].stat_string,
			       ETH_GSTRING_LEN);
			p += ETH_GSTRING_LEN;
		}
		for (i = 0; i < IXGBE_NUM_TX_QUEUES; i++) {
			snprintf(p, ETH_GSTRING_LEN,
				 "tx_queue_%u_packets", i);
			p += ETH_GSTRING_LEN;
			snprintf(p, ETH_GSTRING_LEN,
				 "tx_queue_%u_bytes", i);
			p += ETH_GSTRING_LEN;
#ifdef BP_EXTENDED_STATS
			snprintf(p, ETH_GSTRING_LEN,
				 "tx_queue_%u_bp_napi_yield", i);
			p += ETH_GSTRING_LEN;
			snprintf(p, ETH_GSTRING_LEN,
				 "tx_queue_%u_bp_misses", i);
			p += ETH_GSTRING_LEN;
			snprintf(p, ETH_GSTRING_LEN,
				 "tx_queue_%u_bp_cleaned", i);
			p += ETH_GSTRING_LEN;
#endif /* BP_EXTENDED_STATS */
		}
		for (i = 0; i < IXGBE_NUM_RX_QUEUES; i++) {
			snprintf(p, ETH_GSTRING_LEN,
				 "rx_queue_%u_packets", i);
			p += ETH_GSTRING_LEN;
			snprintf(p, ETH_GSTRING_LEN,
				 "rx_queue_%u_bytes", i);
			p += ETH_GSTRING_LEN;
#ifdef BP_EXTENDED_STATS
			snprintf(p, ETH_GSTRING_LEN,
				 "rx_queue_%u_bp_poll_yield", i);
			p += ETH_GSTRING_LEN;
			snprintf(p, ETH_GSTRING_LEN,
				 "rx_queue_%u_bp_misses", i);
			p += ETH_GSTRING_LEN;
			snprintf(p, ETH_GSTRING_LEN,
				 "rx_queue_%u_bp_cleaned", i);
			p += ETH_GSTRING_LEN;
#endif /* BP_EXTENDED_STATS */
		}
		for (i = 0; i < IXGBE_MAX_PACKET_BUFFERS; i++) {
			snprintf(p, ETH_GSTRING_LEN, "tx_pb_%u_pxon", i);
			p += ETH_GSTRING_LEN;
			snprintf(p, ETH_GSTRING_LEN, "tx_pb_%u_pxoff", i);
			p += ETH_GSTRING_LEN;
		}
		for (i = 0; i < IXGBE_MAX_PACKET_BUFFERS; i++) {
			snprintf(p, ETH_GSTRING_LEN, "rx_pb_%u_pxon", i);
			p += ETH_GSTRING_LEN;
			snprintf(p, ETH_GSTRING_LEN, "rx_pb_%u_pxoff", i);
			p += ETH_GSTRING_LEN;
		}
		for (i = 0; i < adapter->num_vfs; i++) {
			snprintf(p, ETH_GSTRING_LEN, "VF %u Rx Packets", i);
			p += ETH_GSTRING_LEN;
			snprintf(p, ETH_GSTRING_LEN, "VF %u Rx Bytes", i);
			p += ETH_GSTRING_LEN;
			snprintf(p, ETH_GSTRING_LEN, "VF %u Tx Packets", i);
			p += ETH_GSTRING_LEN;
			snprintf(p, ETH_GSTRING_LEN, "VF %u Tx Bytes", i);
			p += ETH_GSTRING_LEN;
			snprintf(p, ETH_GSTRING_LEN, "VF %u MC Packets", i);
			p += ETH_GSTRING_LEN;
		}
		/* BUG_ON(p - data != IXGBE_STATS_LEN * ETH_GSTRING_LEN); */
		break;
#ifdef HAVE_ETHTOOL_GET_SSET_COUNT
	case ETH_SS_PRIV_FLAGS:
		memcpy(data, ixgbe_priv_flags_strings,
		       IXGBE_PRIV_FLAGS_STR_LEN * ETH_GSTRING_LEN);
		break;
#endif /* HAVE_ETHTOOL_GET_SSET_COUNT */
	}
}

static int ixgbe_link_test(struct ixgbe_adapter *adapter, u64 *data)
{
	struct ixgbe_hw *hw = &adapter->hw;
	bool link_up;
	u32 link_speed = 0;

	if (IXGBE_REMOVED(hw->hw_addr)) {
		*data = 1;
		return 1;
	}
	*data = 0;
       hw->mac.ops.check_link(hw, &link_speed, &link_up, true);
	if (link_up)
		return *data;
	else
		*data = 1;
	return *data;
}

/* ethtool register test data */
struct ixgbe_reg_test {
	u16 reg;
	u8  array_len;
	u8  test_type;
	u32 mask;
	u32 write;
};

/* In the hardware, registers are laid out either singly, in arrays
 * spaced 0x40 bytes apart, or in contiguous tables.  We assume
 * most tests take place on arrays or single registers (handled
 * as a single-element array) and special-case the tables.
 * Table tests are always pattern tests.
 *
 * We also make provision for some required setup steps by specifying
 * registers to be written without any read-back testing.
 */

#define PATTERN_TEST	1
#define SET_READ_TEST	2
#define WRITE_NO_TEST	3
#define TABLE32_TEST	4
#define TABLE64_TEST_LO	5
#define TABLE64_TEST_HI	6

/* default 82599 register test */
static struct ixgbe_reg_test reg_test_82599[] = {
	{ IXGBE_FCRTL_82599(0), 1, PATTERN_TEST, 0x8007FFF0, 0x8007FFF0 },
	{ IXGBE_FCRTH_82599(0), 1, PATTERN_TEST, 0x8007FFF0, 0x8007FFF0 },
	{ IXGBE_PFCTOP, 1, PATTERN_TEST, 0xFFFFFFFF, 0xFFFFFFFF },
	{ IXGBE_VLNCTRL, 1, PATTERN_TEST, 0x00000000, 0x00000000 },
	{ IXGBE_RDBAL(0), 4, PATTERN_TEST, 0xFFFFFF80, 0xFFFFFF80 },
	{ IXGBE_RDBAH(0), 4, PATTERN_TEST, 0xFFFFFFFF, 0xFFFFFFFF },
	{ IXGBE_RDLEN(0), 4, PATTERN_TEST, 0x000FFF80, 0x000FFFFF },
	{ IXGBE_RXDCTL(0), 4, WRITE_NO_TEST, 0, IXGBE_RXDCTL_ENABLE },
	{ IXGBE_RDT(0), 4, PATTERN_TEST, 0x0000FFFF, 0x0000FFFF },
	{ IXGBE_RXDCTL(0), 4, WRITE_NO_TEST, 0, 0 },
	{ IXGBE_FCRTH(0), 1, PATTERN_TEST, 0x8007FFF0, 0x8007FFF0 },
	{ IXGBE_FCTTV(0), 1, PATTERN_TEST, 0xFFFFFFFF, 0xFFFFFFFF },
	{ IXGBE_TDBAL(0), 4, PATTERN_TEST, 0xFFFFFF80, 0xFFFFFFFF },
	{ IXGBE_TDBAH(0), 4, PATTERN_TEST, 0xFFFFFFFF, 0xFFFFFFFF },
	{ IXGBE_TDLEN(0), 4, PATTERN_TEST, 0x000FFF80, 0x000FFF80 },
	{ IXGBE_RXCTRL, 1, SET_READ_TEST, 0x00000001, 0x00000001 },
	{ IXGBE_RAL(0), 16, TABLE64_TEST_LO, 0xFFFFFFFF, 0xFFFFFFFF },
	{ IXGBE_RAL(0), 16, TABLE64_TEST_HI, 0x8001FFFF, 0x800CFFFF },
	{ IXGBE_MTA(0), 128, TABLE32_TEST, 0xFFFFFFFF, 0xFFFFFFFF },
	{ .reg = 0 }
};

/* default 82598 register test */
static struct ixgbe_reg_test reg_test_82598[] = {
	{ IXGBE_FCRTL(0), 1, PATTERN_TEST, 0x8007FFF0, 0x8007FFF0 },
	{ IXGBE_FCRTH(0), 1, PATTERN_TEST, 0x8007FFF0, 0x8007FFF0 },
	{ IXGBE_PFCTOP, 1, PATTERN_TEST, 0xFFFFFFFF, 0xFFFFFFFF },
	{ IXGBE_VLNCTRL, 1, PATTERN_TEST, 0x00000000, 0x00000000 },
	{ IXGBE_RDBAL(0), 4, PATTERN_TEST, 0xFFFFFF80, 0xFFFFFFFF },
	{ IXGBE_RDBAH(0), 4, PATTERN_TEST, 0xFFFFFFFF, 0xFFFFFFFF },
	{ IXGBE_RDLEN(0), 4, PATTERN_TEST, 0x000FFF80, 0x000FFFFF },
	/* Enable all four RX queues before testing. */
	{ IXGBE_RXDCTL(0), 4, WRITE_NO_TEST, 0, IXGBE_RXDCTL_ENABLE },
	/* RDH is read-only for 82598, only test RDT. */
	{ IXGBE_RDT(0), 4, PATTERN_TEST, 0x0000FFFF, 0x0000FFFF },
	{ IXGBE_RXDCTL(0), 4, WRITE_NO_TEST, 0, 0 },
	{ IXGBE_FCRTH(0), 1, PATTERN_TEST, 0x8007FFF0, 0x8007FFF0 },
	{ IXGBE_FCTTV(0), 1, PATTERN_TEST, 0xFFFFFFFF, 0xFFFFFFFF },
	{ IXGBE_TIPG, 1, PATTERN_TEST, 0x000000FF, 0x000000FF },
	{ IXGBE_TDBAL(0), 4, PATTERN_TEST, 0xFFFFFF80, 0xFFFFFFFF },
	{ IXGBE_TDBAH(0), 4, PATTERN_TEST, 0xFFFFFFFF, 0xFFFFFFFF },
	{ IXGBE_TDLEN(0), 4, PATTERN_TEST, 0x000FFF80, 0x000FFFFF },
	{ IXGBE_RXCTRL, 1, SET_READ_TEST, 0x00000003, 0x00000003 },
	{ IXGBE_DTXCTL, 1, SET_READ_TEST, 0x00000005, 0x00000005 },
	{ IXGBE_RAL(0), 16, TABLE64_TEST_LO, 0xFFFFFFFF, 0xFFFFFFFF },
	{ IXGBE_RAL(0), 16, TABLE64_TEST_HI, 0x800CFFFF, 0x800CFFFF },
	{ IXGBE_MTA(0), 128, TABLE32_TEST, 0xFFFFFFFF, 0xFFFFFFFF },
	{ .reg = 0 }
};


static bool reg_pattern_test(struct ixgbe_adapter *adapter, u64 *data, int reg,
			     u32 mask, u32 write)
{
	u32 pat, val, before;
	static const u32 test_pattern[] = {
		0x5A5A5A5A, 0xA5A5A5A5, 0x00000000, 0xFFFFFFFF
	};

	if (IXGBE_REMOVED(adapter->hw.hw_addr)) {
		*data = 1;
		return true;
	}
	for (pat = 0; pat < ARRAY_SIZE(test_pattern); pat++) {
		before = IXGBE_READ_REG(&adapter->hw, reg);
		IXGBE_WRITE_REG(&adapter->hw, reg, test_pattern[pat] & write);
		val = IXGBE_READ_REG(&adapter->hw, reg);
		if (val != (test_pattern[pat] & write & mask)) {
			e_err(drv,
			      "pattern test reg %04X failed: got 0x%08X expected 0x%08X\n",
			      reg, val, test_pattern[pat] & write & mask);
			*data = reg;
			IXGBE_WRITE_REG(&adapter->hw, reg, before);
			return true;
		}
		IXGBE_WRITE_REG(&adapter->hw, reg, before);
	}
	return false;
}

static bool reg_set_and_check(struct ixgbe_adapter *adapter, u64 *data, int reg,
			      u32 mask, u32 write)
{
	u32 val, before;

	if (IXGBE_REMOVED(adapter->hw.hw_addr)) {
		*data = 1;
		return true;
	}
	before = IXGBE_READ_REG(&adapter->hw, reg);
	IXGBE_WRITE_REG(&adapter->hw, reg, write & mask);
	val = IXGBE_READ_REG(&adapter->hw, reg);
	if ((write & mask) != (val & mask)) {
		e_err(drv,
		      "set/check reg %04X test failed: got 0x%08X expected 0x%08X\n",
		      reg, (val & mask), (write & mask));
		*data = reg;
		IXGBE_WRITE_REG(&adapter->hw, reg, before);
		return true;
	}
	IXGBE_WRITE_REG(&adapter->hw, reg, before);
	return false;
}

static bool ixgbe_reg_test(struct ixgbe_adapter *adapter, u64 *data)
{
	struct ixgbe_reg_test *test;
	struct ixgbe_hw *hw = &adapter->hw;
	u32 value, before, after;
	u32 i, toggle;

	if (IXGBE_REMOVED(hw->hw_addr)) {
		e_err(drv, "Adapter removed - register test blocked\n");
		*data = 1;
		return true;
	}
	switch (hw->mac.type) {
	case ixgbe_mac_82598EB:
		toggle = 0x7FFFF3FF;
		test = reg_test_82598;
		break;
	case ixgbe_mac_82599EB:
	case ixgbe_mac_X540:
	case ixgbe_mac_X550:
	case ixgbe_mac_X550EM_x:
	case ixgbe_mac_X550EM_a:
		toggle = 0x7FFFF30F;
		test = reg_test_82599;
		break;
	default:
		*data = 1;
		return true;
	}

	/*
	 * Because the status register is such a special case,
	 * we handle it separately from the rest of the register
	 * tests.  Some bits are read-only, some toggle, and some
	 * are writeable on newer MACs.
	 */
	before = IXGBE_READ_REG(hw, IXGBE_STATUS);
	value = IXGBE_READ_REG(hw, IXGBE_STATUS) & toggle;
	IXGBE_WRITE_REG(hw, IXGBE_STATUS, toggle);
	after = IXGBE_READ_REG(hw, IXGBE_STATUS) & toggle;
	if (value != after) {
		e_err(drv,
		      "failed STATUS register test got: 0x%08X expected: 0x%08X\n",
		      after, value);
		*data = 1;
		return true;
	}
	/* restore previous status */
	IXGBE_WRITE_REG(hw, IXGBE_STATUS, before);

	/*
	 * Perform the remainder of the register test, looping through
	 * the test table until we either fail or reach the null entry.
	 */
	while (test->reg) {
		for (i = 0; i < test->array_len; i++) {
			bool b = false;

			switch (test->test_type) {
			case PATTERN_TEST:
				b = reg_pattern_test(adapter, data,
						      test->reg + (i * 0x40),
						      test->mask,
						      test->write);
				break;
			case SET_READ_TEST:
				b = reg_set_and_check(adapter, data,
						       test->reg + (i * 0x40),
						       test->mask,
						       test->write);
				break;
			case WRITE_NO_TEST:
				IXGBE_WRITE_REG(hw, test->reg + (i * 0x40),
						test->write);
				break;
			case TABLE32_TEST:
				b = reg_pattern_test(adapter, data,
						      test->reg + (i * 4),
						      test->mask,
						      test->write);
				break;
			case TABLE64_TEST_LO:
				b = reg_pattern_test(adapter, data,
						      test->reg + (i * 8),
						      test->mask,
						      test->write);
				break;
			case TABLE64_TEST_HI:
				b = reg_pattern_test(adapter, data,
						      (test->reg + 4) + (i * 8),
						      test->mask,
						      test->write);
				break;
			}
			if (b)
				return true;
		}
		test++;
	}

	*data = 0;
	return false;
}

static bool ixgbe_eeprom_test(struct ixgbe_adapter *adapter, u64 *data)
{
	struct ixgbe_hw *hw = &adapter->hw;

	if (hw->eeprom.ops.validate_checksum(hw, NULL)) {
		*data = 1;
		return true;
	} else {
		*data = 0;
		return false;
	}
}

static irqreturn_t ixgbe_test_intr(int __always_unused irq, void *data)
{
	struct net_device *netdev = (struct net_device *) data;
	struct ixgbe_adapter *adapter = netdev_priv(netdev);

	adapter->test_icr |= IXGBE_READ_REG(&adapter->hw, IXGBE_EICR);

	return IRQ_HANDLED;
}

static int ixgbe_intr_test(struct ixgbe_adapter *adapter, u64 *data)
{
	struct net_device *netdev = adapter->netdev;
	u32 mask, i = 0, shared_int = true;
	u32 irq = adapter->pdev->irq;

	if (IXGBE_REMOVED(adapter->hw.hw_addr)) {
		*data = 1;
		return -1;
	}
	*data = 0;

	/* Hook up test interrupt handler just for this test */
	if (adapter->msix_entries) {
		/* NOTE: we don't test MSI-X interrupts here, yet */
		return 0;
	} else if (adapter->flags & IXGBE_FLAG_MSI_ENABLED) {
		shared_int = false;
		if (request_irq(irq, &ixgbe_test_intr, 0, netdev->name,
				netdev)) {
			*data = 1;
			return -1;
		}
	} else if (!request_irq(irq, &ixgbe_test_intr, IRQF_PROBE_SHARED,
				netdev->name, netdev)) {
		shared_int = false;
	} else if (request_irq(irq, &ixgbe_test_intr, IRQF_SHARED,
			       netdev->name, netdev)) {
		*data = 1;
		return -1;
	}
	e_info(hw, "testing %s interrupt\n",
	       (shared_int ? "shared" : "unshared"));

	/* Disable all the interrupts */
	IXGBE_WRITE_REG(&adapter->hw, IXGBE_EIMC, 0xFFFFFFFF);
	IXGBE_WRITE_FLUSH(&adapter->hw);
	usleep_range(10000, 20000);

	/* Test each interrupt */
	for (; i < 10; i++) {
		/* Interrupt to test */
		mask = 1 << i;

		if (!shared_int) {
			/*
			 * Disable the interrupts to be reported in
			 * the cause register and then force the same
			 * interrupt and see if one gets posted.  If
			 * an interrupt was posted to the bus, the
			 * test failed.
			 */
			adapter->test_icr = 0;
			IXGBE_WRITE_REG(&adapter->hw, IXGBE_EIMC,
					~mask & 0x00007FFF);
			IXGBE_WRITE_REG(&adapter->hw, IXGBE_EICS,
					~mask & 0x00007FFF);
			IXGBE_WRITE_FLUSH(&adapter->hw);
			usleep_range(10000, 20000);

			if (adapter->test_icr & mask) {
				*data = 3;
				break;
			}
		}

		/*
		 * Enable the interrupt to be reported in the cause
		 * register and then force the same interrupt and see
		 * if one gets posted.  If an interrupt was not posted
		 * to the bus, the test failed.
		 */
		adapter->test_icr = 0;
		IXGBE_WRITE_REG(&adapter->hw, IXGBE_EIMS, mask);
		IXGBE_WRITE_REG(&adapter->hw, IXGBE_EICS, mask);
		IXGBE_WRITE_FLUSH(&adapter->hw);
		usleep_range(10000, 20000);

		if (!(adapter->test_icr & mask)) {
			*data = 4;
			break;
		}

		if (!shared_int) {
			/*
			 * Disable the other interrupts to be reported in
			 * the cause register and then force the other
			 * interrupts and see if any get posted.  If
			 * an interrupt was posted to the bus, the
			 * test failed.
			 */
			adapter->test_icr = 0;
			IXGBE_WRITE_REG(&adapter->hw, IXGBE_EIMC,
					~mask & 0x00007FFF);
			IXGBE_WRITE_REG(&adapter->hw, IXGBE_EICS,
					~mask & 0x00007FFF);
			IXGBE_WRITE_FLUSH(&adapter->hw);
			usleep_range(10000, 20000);

			if (adapter->test_icr) {
				*data = 5;
				break;
			}
		}
	}

	/* Disable all the interrupts */
	IXGBE_WRITE_REG(&adapter->hw, IXGBE_EIMC, 0xFFFFFFFF);
	IXGBE_WRITE_FLUSH(&adapter->hw);
	usleep_range(10000, 20000);

	/* Unhook test interrupt handler */
	free_irq(irq, netdev);

	return *data;
}

static void ixgbe_free_desc_rings(struct ixgbe_adapter *adapter)
{
	/* Shut down the DMA engines now so they can be reinitialized later,
	 * since the test rings and normally used rings should overlap on
	 * queue 0 we can just use the standard disable Rx/Tx calls and they
	 * will take care of disabling the test rings for us.
	 */

	/* first Rx */
	ixgbe_disable_rx_queue(adapter);

	/* now Tx */
	ixgbe_disable_tx_queue(adapter);

	ixgbe_reset(adapter);

	ixgbe_free_tx_resources(&adapter->test_tx_ring);
	ixgbe_free_rx_resources(&adapter->test_rx_ring);
}

static int ixgbe_setup_desc_rings(struct ixgbe_adapter *adapter)
{
	struct ixgbe_ring *tx_ring = &adapter->test_tx_ring;
	struct ixgbe_ring *rx_ring = &adapter->test_rx_ring;
	u32 rctl, reg_data;
	int ret_val;
	int err;

	/* Setup Tx descriptor ring and Tx buffers */
	tx_ring->count = IXGBE_DEFAULT_TXD;
	tx_ring->queue_index = 0;
	tx_ring->dev = pci_dev_to_dev(adapter->pdev);
	tx_ring->netdev = adapter->netdev;
	tx_ring->reg_idx = adapter->tx_ring[0]->reg_idx;

	err = ixgbe_setup_tx_resources(tx_ring);
	if (err)
		return 1;

	switch (adapter->hw.mac.type) {
	case ixgbe_mac_82599EB:
	case ixgbe_mac_X540:
	case ixgbe_mac_X550:
	case ixgbe_mac_X550EM_x:
	case ixgbe_mac_X550EM_a:
		reg_data = IXGBE_READ_REG(&adapter->hw, IXGBE_DMATXCTL);
		reg_data |= IXGBE_DMATXCTL_TE;
		IXGBE_WRITE_REG(&adapter->hw, IXGBE_DMATXCTL, reg_data);
		break;
	default:
		break;
	}

	ixgbe_configure_tx_ring(adapter, tx_ring);

	/* Setup Rx Descriptor ring and Rx buffers */
	rx_ring->count = IXGBE_DEFAULT_RXD;
	rx_ring->queue_index = 0;
	rx_ring->dev = pci_dev_to_dev(adapter->pdev);
	rx_ring->netdev = adapter->netdev;
	rx_ring->reg_idx = adapter->rx_ring[0]->reg_idx;
#ifdef CONFIG_IXGBE_DISABLE_PACKET_SPLIT
	rx_ring->rx_buf_len = IXGBE_RXBUFFER_2K;
#endif

	err = ixgbe_setup_rx_resources(adapter, rx_ring);
	if (err) {
		ret_val = 4;
		goto err_nomem;
	}

	ixgbe_disable_rx(&adapter->hw);

	ixgbe_configure_rx_ring(adapter, rx_ring);

	rctl = IXGBE_READ_REG(&adapter->hw, IXGBE_RXCTRL); 
	rctl |= IXGBE_RXCTRL_DMBYPS;
	IXGBE_WRITE_REG(&adapter->hw, IXGBE_RXCTRL, rctl);
	ixgbe_enable_rx(&adapter->hw);

	return 0;

err_nomem:
	ixgbe_free_desc_rings(adapter);
	return ret_val;
}

static int ixgbe_setup_loopback_test(struct ixgbe_adapter *adapter)
{
	struct ixgbe_hw *hw = &adapter->hw;
	u32 reg_data;


	/* Setup MAC loopback */
	reg_data = IXGBE_READ_REG(hw, IXGBE_HLREG0);
	reg_data |= IXGBE_HLREG0_LPBK;
	IXGBE_WRITE_REG(hw, IXGBE_HLREG0, reg_data);

	reg_data = IXGBE_READ_REG(hw, IXGBE_FCTRL);
	reg_data |= IXGBE_FCTRL_BAM | IXGBE_FCTRL_SBP | IXGBE_FCTRL_MPE;
	IXGBE_WRITE_REG(hw, IXGBE_FCTRL, reg_data);

	/* X540 needs to set the MACC.FLU bit to force link up */
	switch (adapter->hw.mac.type) {
	case ixgbe_mac_X550:
	case ixgbe_mac_X550EM_x:
	case ixgbe_mac_X550EM_a:
	case ixgbe_mac_X540:
		reg_data = IXGBE_READ_REG(hw, IXGBE_MACC);
		reg_data |= IXGBE_MACC_FLU;
		IXGBE_WRITE_REG(hw, IXGBE_MACC, reg_data);
		break;
	default:
		if (hw->mac.orig_autoc) {
			reg_data = hw->mac.orig_autoc | IXGBE_AUTOC_FLU;
			IXGBE_WRITE_REG(hw, IXGBE_AUTOC, reg_data);
		} else {
			return 10;
		}
	}
	IXGBE_WRITE_FLUSH(hw);
	usleep_range(10000, 20000);

	/* Disable Atlas Tx lanes; re-enabled in reset path */
	if (hw->mac.type == ixgbe_mac_82598EB) {
		u8 atlas;

		hw->mac.ops.read_analog_reg8(hw, IXGBE_ATLAS_PDN_LPBK, &atlas);
		atlas |= IXGBE_ATLAS_PDN_TX_REG_EN;
		hw->mac.ops.write_analog_reg8(hw, IXGBE_ATLAS_PDN_LPBK, atlas);

		hw->mac.ops.read_analog_reg8(hw, IXGBE_ATLAS_PDN_10G, &atlas);
		atlas |= IXGBE_ATLAS_PDN_TX_10G_QL_ALL;
		hw->mac.ops.write_analog_reg8(hw, IXGBE_ATLAS_PDN_10G, atlas);

		hw->mac.ops.read_analog_reg8(hw, IXGBE_ATLAS_PDN_1G, &atlas);
		atlas |= IXGBE_ATLAS_PDN_TX_1G_QL_ALL;
		hw->mac.ops.write_analog_reg8(hw, IXGBE_ATLAS_PDN_1G, atlas);

		hw->mac.ops.read_analog_reg8(hw, IXGBE_ATLAS_PDN_AN, &atlas);
		atlas |= IXGBE_ATLAS_PDN_TX_AN_QL_ALL;
		hw->mac.ops.write_analog_reg8(hw, IXGBE_ATLAS_PDN_AN, atlas);
	}

	return 0;
}

static void ixgbe_loopback_cleanup(struct ixgbe_adapter *adapter)
{
	u32 reg_data;

	reg_data = IXGBE_READ_REG(&adapter->hw, IXGBE_HLREG0);
	reg_data &= ~IXGBE_HLREG0_LPBK;
	IXGBE_WRITE_REG(&adapter->hw, IXGBE_HLREG0, reg_data);
}

static void ixgbe_create_lbtest_frame(struct sk_buff *skb,
				      unsigned int frame_size)
{
	memset(skb->data, 0xFF, frame_size);
	frame_size >>= 1;
	memset(&skb->data[frame_size], 0xAA, frame_size / 2 - 1);
	memset(&skb->data[frame_size + 10], 0xBE, 1);
	memset(&skb->data[frame_size + 12], 0xAF, 1);
}

static bool ixgbe_check_lbtest_frame(struct ixgbe_rx_buffer *rx_buffer,
				     unsigned int frame_size)
{
	unsigned char *data;
	bool match = true;

	frame_size >>= 1;

#ifdef CONFIG_IXGBE_DISABLE_PACKET_SPLIT
	data = rx_buffer->skb->data;
#else
	data = kmap(rx_buffer->page) + rx_buffer->page_offset;
#endif

	if (data[3] != 0xFF ||
	    data[frame_size + 10] != 0xBE ||
	    data[frame_size + 12] != 0xAF)
		match = false;

#ifndef CONFIG_IXGBE_DISABLE_PACKET_SPLIT
	kunmap(rx_buffer->page);

#endif
	return match;
}

static u16 ixgbe_clean_test_rings(struct ixgbe_ring *rx_ring,
				  struct ixgbe_ring *tx_ring,
				  unsigned int size)
{
	union ixgbe_adv_rx_desc *rx_desc;
#ifdef CONFIG_IXGBE_DISABLE_PACKET_SPLIT
	const int bufsz = rx_ring->rx_buf_len;
#else
	const int bufsz = ixgbe_rx_bufsz(rx_ring);
#endif
	u16 rx_ntc, tx_ntc, count = 0;

	/* initialize next to clean and descriptor values */
	rx_ntc = rx_ring->next_to_clean;
	tx_ntc = tx_ring->next_to_clean;
	rx_desc = IXGBE_RX_DESC(rx_ring, rx_ntc);

	while (tx_ntc != tx_ring->next_to_use) {
		union ixgbe_adv_tx_desc *tx_desc;
		struct ixgbe_tx_buffer *tx_buffer;

		tx_desc = IXGBE_TX_DESC(tx_ring, tx_ntc);

		/* if DD is not set transmit has not completed */
		if (!(tx_desc->wb.status & cpu_to_le32(IXGBE_TXD_STAT_DD)))
			return count;

		/* unmap buffer on Tx side */
		tx_buffer = &tx_ring->tx_buffer_info[tx_ntc];

		/* Free all the Tx ring sk_buffs */
		dev_kfree_skb_any(tx_buffer->skb);

		/* unmap skb header data */
		dma_unmap_single(tx_ring->dev,
				 dma_unmap_addr(tx_buffer, dma),
				 dma_unmap_len(tx_buffer, len),
				 DMA_TO_DEVICE);
		dma_unmap_len_set(tx_buffer, len, 0);

		/* increment Tx next to clean counter */
		tx_ntc++;
		if (tx_ntc == tx_ring->count)
			tx_ntc = 0;
	}

	while (rx_desc->wb.upper.length) {
		struct ixgbe_rx_buffer *rx_buffer;

		/* check Rx buffer */
		rx_buffer = &rx_ring->rx_buffer_info[rx_ntc];

		/* sync Rx buffer for CPU read */
		dma_sync_single_for_cpu(rx_ring->dev,
					rx_buffer->dma,
					bufsz,
					DMA_FROM_DEVICE);

		/* verify contents of skb */
		if (ixgbe_check_lbtest_frame(rx_buffer, size))
			count++;
		else
			break;

		/* sync Rx buffer for device write */
		dma_sync_single_for_device(rx_ring->dev,
					   rx_buffer->dma,
					   bufsz,
					   DMA_FROM_DEVICE);

		/* increment Rx next to clean counter */
		rx_ntc++;
		if (rx_ntc == rx_ring->count)
			rx_ntc = 0;

		/* fetch next descriptor */
		rx_desc = IXGBE_RX_DESC(rx_ring, rx_ntc);
	}

	/* re-map buffers to ring, store next to clean values */
	ixgbe_alloc_rx_buffers(rx_ring, count);
	rx_ring->next_to_clean = rx_ntc;
	tx_ring->next_to_clean = tx_ntc;

	return count;
}

static int ixgbe_run_loopback_test(struct ixgbe_adapter *adapter)
{
	struct ixgbe_ring *tx_ring = &adapter->test_tx_ring;
	struct ixgbe_ring *rx_ring = &adapter->test_rx_ring;
	int i, j, lc, ret_val = 0;
	unsigned int size = 1024;
	netdev_tx_t tx_ret_val;
	struct sk_buff *skb;
	u32 flags_orig = adapter->flags;

	/* DCB can modify the frames on Tx */
	adapter->flags &= ~IXGBE_FLAG_DCB_ENABLED;

	/* allocate test skb */
	skb = alloc_skb(size, GFP_KERNEL);
	if (!skb)
		return 11;

	/* place data into test skb */
	ixgbe_create_lbtest_frame(skb, size);
	skb_put(skb, size);

	/*
	 * Calculate the loop count based on the largest descriptor ring
	 * The idea is to wrap the largest ring a number of times using 64
	 * send/receive pairs during each loop
	 */

	if (rx_ring->count <= tx_ring->count)
		lc = ((tx_ring->count / 64) * 2) + 1;
	else
		lc = ((rx_ring->count / 64) * 2) + 1;

	for (j = 0; j <= lc; j++) {
		unsigned int good_cnt;

		/* reset count of good packets */
		good_cnt = 0;

		/* place 64 packets on the transmit queue*/
		for (i = 0; i < 64; i++) {
			skb_get(skb);
			tx_ret_val = ixgbe_xmit_frame_ring(skb,
							   adapter,
							   tx_ring);
			if (tx_ret_val == NETDEV_TX_OK)
				good_cnt++;
		}

		if (good_cnt != 64) {
			ret_val = 12;
			break;
		}

		/* allow 200 milliseconds for packets to go from Tx to Rx */
		msleep(200);

		good_cnt = ixgbe_clean_test_rings(rx_ring, tx_ring, size);
		if (good_cnt != 64) {
			ret_val = 13;
			break;
		}
	}

	/* free the original skb */
	kfree_skb(skb);
	adapter->flags = flags_orig;

	return ret_val;
}

static int ixgbe_loopback_test(struct ixgbe_adapter *adapter, u64 *data)
{
	*data = ixgbe_setup_desc_rings(adapter);
	if (*data)
		goto out;
	*data = ixgbe_setup_loopback_test(adapter);
	if (*data)
		goto err_loopback;
	*data = ixgbe_run_loopback_test(adapter);
	ixgbe_loopback_cleanup(adapter);

err_loopback:
	ixgbe_free_desc_rings(adapter);
out:
	return *data;
}

#ifndef HAVE_ETHTOOL_GET_SSET_COUNT
static int ixgbe_diag_test_count(struct net_device __always_unused *netdev)
{
	return IXGBE_TEST_LEN;
}

#endif /* HAVE_ETHTOOL_GET_SSET_COUNT */
static void ixgbe_diag_test(struct net_device *netdev,
			    struct ethtool_test *eth_test, u64 *data)
{
	struct ixgbe_adapter *adapter = netdev_priv(netdev);
	bool if_running = netif_running(netdev);
	struct ixgbe_hw *hw = &adapter->hw;

	if (IXGBE_REMOVED(hw->hw_addr)) {
		e_err(hw, "Adapter removed - test blocked\n");
		data[0] = 1;
		data[1] = 1;
		data[2] = 1;
		data[3] = 1;
		data[4] = 1;
		eth_test->flags |= ETH_TEST_FL_FAILED;
		return;
	}
	set_bit(__IXGBE_TESTING, &adapter->state);
	if (eth_test->flags == ETH_TEST_FL_OFFLINE) {
		if (adapter->flags & IXGBE_FLAG_SRIOV_ENABLED) {
			int i;
			for (i = 0; i < adapter->num_vfs; i++) {
				if (adapter->vfinfo[i].clear_to_send) {
					e_warn(drv, "Please take active VFS "
					       "offline and restart the "
					       "adapter before running NIC "
					       "diagnostics\n");
					data[0] = 1;
					data[1] = 1;
					data[2] = 1;
					data[3] = 1;
					data[4] = 1;
					eth_test->flags |= ETH_TEST_FL_FAILED;
					clear_bit(__IXGBE_TESTING,
						  &adapter->state);
					goto skip_ol_tests;
				}
			}
		}

		/* Offline tests */
		e_info(hw, "offline testing starting\n");

		/* Link test performed before hardware reset so autoneg doesn't
		 * interfere with test result */
		if (ixgbe_link_test(adapter, &data[4]))
			eth_test->flags |= ETH_TEST_FL_FAILED;

		if (if_running)
			/* indicate we're in test mode */
			ixgbe_close(netdev);
		else
			ixgbe_reset(adapter);

		e_info(hw, "register testing starting\n");
		if (ixgbe_reg_test(adapter, &data[0]))
			eth_test->flags |= ETH_TEST_FL_FAILED;

		ixgbe_reset(adapter);
		e_info(hw, "eeprom testing starting\n");
		if (ixgbe_eeprom_test(adapter, &data[1]))
			eth_test->flags |= ETH_TEST_FL_FAILED;

		ixgbe_reset(adapter);
		e_info(hw, "interrupt testing starting\n");
		if (ixgbe_intr_test(adapter, &data[2]))
			eth_test->flags |= ETH_TEST_FL_FAILED;

		/* If SRIOV or VMDq is enabled then skip MAC
		 * loopback diagnostic. */
		if (adapter->flags & (IXGBE_FLAG_SRIOV_ENABLED |
				      IXGBE_FLAG_VMDQ_ENABLED)) {
			e_info(hw, "skip MAC loopback diagnostic in VT mode\n");
			data[3] = 0;
			goto skip_loopback;
		}

		ixgbe_reset(adapter);
		e_info(hw, "loopback testing starting\n");
		if (ixgbe_loopback_test(adapter, &data[3]))
			eth_test->flags |= ETH_TEST_FL_FAILED;

skip_loopback:
		ixgbe_reset(adapter);

		/* clear testing bit and return adapter to previous state */
		clear_bit(__IXGBE_TESTING, &adapter->state);
		if (if_running)
			ixgbe_open(netdev);
		else if (hw->mac.ops.disable_tx_laser)
			hw->mac.ops.disable_tx_laser(hw);
	} else {
		e_info(hw, "online testing starting\n");

		/* Online tests */
		if (ixgbe_link_test(adapter, &data[4]))
			eth_test->flags |= ETH_TEST_FL_FAILED;

		/* Offline tests aren't run; pass by default */
		data[0] = 0;
		data[1] = 0;
		data[2] = 0;
		data[3] = 0;

		clear_bit(__IXGBE_TESTING, &adapter->state);
	}

skip_ol_tests:
	msleep_interruptible(4 * 1000);
}

static int ixgbe_wol_exclusion(struct ixgbe_adapter *adapter,
			       struct ethtool_wolinfo *wol)
{
	struct ixgbe_hw *hw = &adapter->hw;
	int retval = 0;

	/* WOL not supported for all devices */
	if (!ixgbe_wol_supported(adapter, hw->device_id,
				 hw->subsystem_device_id)) {
		retval = 1;
		wol->supported = 0;
	}

	return retval;
}

static void ixgbe_get_wol(struct net_device *netdev,
			  struct ethtool_wolinfo *wol)
{
	struct ixgbe_adapter *adapter = netdev_priv(netdev);

	wol->supported = WAKE_UCAST | WAKE_MCAST |
			 WAKE_BCAST | WAKE_MAGIC;
	wol->wolopts = 0;

	if (ixgbe_wol_exclusion(adapter, wol) ||
	    !device_can_wakeup(pci_dev_to_dev(adapter->pdev)))
		return;

	if (adapter->wol & IXGBE_WUFC_EX)
		wol->wolopts |= WAKE_UCAST;
	if (adapter->wol & IXGBE_WUFC_MC)
		wol->wolopts |= WAKE_MCAST;
	if (adapter->wol & IXGBE_WUFC_BC)
		wol->wolopts |= WAKE_BCAST;
	if (adapter->wol & IXGBE_WUFC_MAG)
		wol->wolopts |= WAKE_MAGIC;
}

static int ixgbe_set_wol(struct net_device *netdev, struct ethtool_wolinfo *wol)
{
	struct ixgbe_adapter *adapter = netdev_priv(netdev);
	struct ixgbe_hw *hw = &adapter->hw;

	if (wol->wolopts & (WAKE_PHY | WAKE_ARP | WAKE_MAGICSECURE |
			    WAKE_FILTER))
		return -EOPNOTSUPP;

	if (ixgbe_wol_exclusion(adapter, wol))
		return wol->wolopts ? -EOPNOTSUPP : 0;

	adapter->wol = 0;

	if (wol->wolopts & WAKE_UCAST)
		adapter->wol |= IXGBE_WUFC_EX;
	if (wol->wolopts & WAKE_MCAST)
		adapter->wol |= IXGBE_WUFC_MC;
	if (wol->wolopts & WAKE_BCAST)
		adapter->wol |= IXGBE_WUFC_BC;
	if (wol->wolopts & WAKE_MAGIC)
		adapter->wol |= IXGBE_WUFC_MAG;

	hw->wol_enabled = !!(adapter->wol);

	device_set_wakeup_enable(pci_dev_to_dev(adapter->pdev), adapter->wol);

	return 0;
}

static int ixgbe_nway_reset(struct net_device *netdev)
{
	struct ixgbe_adapter *adapter = netdev_priv(netdev);

	if (netif_running(netdev))
		ixgbe_reinit_locked(adapter);

	return 0;
}

#ifdef HAVE_ETHTOOL_SET_PHYS_ID
static int ixgbe_set_phys_id(struct net_device *netdev,
			     enum ethtool_phys_id_state state)
{
	struct ixgbe_adapter *adapter = netdev_priv(netdev);
	struct ixgbe_hw *hw = &adapter->hw;

	if (!hw->mac.ops.led_on || !hw->mac.ops.led_off)
		return -EOPNOTSUPP;

	switch (state) {
	case ETHTOOL_ID_ACTIVE:
		adapter->led_reg = IXGBE_READ_REG(hw, IXGBE_LEDCTL);
		return 2;

	case ETHTOOL_ID_ON:
		if (hw->mac.ops.led_on(hw, hw->mac.led_link_act))
			return -EINVAL;
		break;

	case ETHTOOL_ID_OFF:
		if (hw->mac.ops.led_off(hw, hw->mac.led_link_act))
			return -EINVAL;
		break;

	case ETHTOOL_ID_INACTIVE:
		/* Restore LED settings */
		IXGBE_WRITE_REG(&adapter->hw, IXGBE_LEDCTL, adapter->led_reg);
		break;
	}

	return 0;
}
#else
static int ixgbe_phys_id(struct net_device *netdev, u32 data)
{
	struct ixgbe_adapter *adapter = netdev_priv(netdev);
	struct ixgbe_hw *hw = &adapter->hw;
	u32 led_reg = IXGBE_READ_REG(hw, IXGBE_LEDCTL);
	u32 i;

	if (!hw->mac.ops.led_on || !hw->mac.ops.led_off)
		return -EOPNOTSUPP;

	if (!data || data > 300)
		data = 300;

	for (i = 0; i < (data * 1000); i += 400) {
		if (hw->mac.ops.led_on(hw, hw->mac.led_link_act))
			return -EINVAL;
		msleep_interruptible(200);
		if (hw->mac.ops.led_off(hw, hw->mac.led_link_act))
			return -EINVAL;
		msleep_interruptible(200);
	}

	/* Restore LED settings */
	IXGBE_WRITE_REG(hw, IXGBE_LEDCTL, led_reg);

	return IXGBE_SUCCESS;
}
#endif /* HAVE_ETHTOOL_SET_PHYS_ID */

static int ixgbe_get_coalesce(struct net_device *netdev,
			      struct ethtool_coalesce *ec)
{
	struct ixgbe_adapter *adapter = netdev_priv(netdev);

	ec->tx_max_coalesced_frames_irq = adapter->tx_work_limit;
	/* only valid if in constant ITR mode */
	if (adapter->rx_itr_setting <= 1)
		ec->rx_coalesce_usecs = adapter->rx_itr_setting;
	else
		ec->rx_coalesce_usecs = adapter->rx_itr_setting >> 2;

	/* if in mixed tx/rx queues per vector mode, report only rx settings */
	if (adapter->q_vector[0]->tx.count && adapter->q_vector[0]->rx.count)
		return 0;

	/* only valid if in constant ITR mode */
	if (adapter->tx_itr_setting <= 1)
		ec->tx_coalesce_usecs = adapter->tx_itr_setting;
	else
		ec->tx_coalesce_usecs = adapter->tx_itr_setting >> 2;

	return 0;
}

/*
 * this function must be called before setting the new value of
 * rx_itr_setting
 */
static bool ixgbe_update_rsc(struct ixgbe_adapter *adapter)
{
	struct net_device *netdev = adapter->netdev;

	/* nothing to do if LRO or RSC are not enabled */
	if (!(adapter->flags2 & IXGBE_FLAG2_RSC_CAPABLE) ||
	    !(netdev->features & NETIF_F_LRO))
		return false;

	/* check the feature flag value and enable RSC if necessary */
	if (adapter->rx_itr_setting == 1 ||
	    adapter->rx_itr_setting > IXGBE_MIN_RSC_ITR) {
		if (!(adapter->flags2 & IXGBE_FLAG2_RSC_ENABLED)) {
			adapter->flags2 |= IXGBE_FLAG2_RSC_ENABLED;
			e_info(probe, "rx-usecs value high enough "
				      "to re-enable RSC\n");
			return true;
		}
	/* if interrupt rate is too high then disable RSC */
	} else if (adapter->flags2 & IXGBE_FLAG2_RSC_ENABLED) {
		adapter->flags2 &= ~IXGBE_FLAG2_RSC_ENABLED;
		e_info(probe, "rx-usecs set too low, disabling RSC\n");
		return true;
	}
	return false;
}

static int ixgbe_set_coalesce(struct net_device *netdev,
			      struct ethtool_coalesce *ec)
{
	struct ixgbe_adapter *adapter = netdev_priv(netdev);
	int i;
	u16 tx_itr_param, rx_itr_param;
	u16  tx_itr_prev;
	bool need_reset = false;

	if (adapter->q_vector[0]->tx.count && adapter->q_vector[0]->rx.count) {
		/* reject Tx specific changes in case of mixed RxTx vectors */
		if (ec->tx_coalesce_usecs)
			return -EINVAL;
		tx_itr_prev = adapter->rx_itr_setting;
	} else {
		tx_itr_prev = adapter->tx_itr_setting;
	}

	if (ec->tx_max_coalesced_frames_irq)
		adapter->tx_work_limit = ec->tx_max_coalesced_frames_irq;

	if ((ec->rx_coalesce_usecs > (IXGBE_MAX_EITR >> 2)) ||
	    (ec->tx_coalesce_usecs > (IXGBE_MAX_EITR >> 2)))
		return -EINVAL;

	if (ec->rx_coalesce_usecs > 1)
		adapter->rx_itr_setting = ec->rx_coalesce_usecs << 2;
	else
		adapter->rx_itr_setting = ec->rx_coalesce_usecs;

	if (adapter->rx_itr_setting == 1)
		rx_itr_param = IXGBE_20K_ITR;
	else
		rx_itr_param = adapter->rx_itr_setting;

	if (ec->tx_coalesce_usecs > 1)
		adapter->tx_itr_setting = ec->tx_coalesce_usecs << 2;
	else
		adapter->tx_itr_setting = ec->tx_coalesce_usecs;

	if (adapter->tx_itr_setting == 1)
		tx_itr_param = IXGBE_12K_ITR;
	else
		tx_itr_param = adapter->tx_itr_setting;

	/* mixed Rx/Tx */
	if (adapter->q_vector[0]->tx.count && adapter->q_vector[0]->rx.count)
		adapter->tx_itr_setting = adapter->rx_itr_setting;

	/* detect ITR changes that require update of TXDCTL.WTHRESH */
	if ((adapter->tx_itr_setting != 1) &&
	    (adapter->tx_itr_setting < IXGBE_100K_ITR)) {
		if ((tx_itr_prev == 1) ||
		    (tx_itr_prev >= IXGBE_100K_ITR))
			need_reset = true;
	} else {
		if ((tx_itr_prev != 1) &&
		    (tx_itr_prev < IXGBE_100K_ITR))
			need_reset = true;
	}

	/* check the old value and enable RSC if necessary */
	need_reset |= ixgbe_update_rsc(adapter);

	if (adapter->hw.mac.dmac_config.watchdog_timer &&
	    (!adapter->rx_itr_setting && !adapter->tx_itr_setting)) {
		e_info(probe,
		       "Disabling DMA coalescing because interrupt throttling is disabled\n");
		adapter->hw.mac.dmac_config.watchdog_timer = 0;
		ixgbe_dmac_config(&adapter->hw);
	}

	for (i = 0; i < adapter->num_q_vectors; i++) {
		struct ixgbe_q_vector *q_vector = adapter->q_vector[i];

		q_vector->tx.work_limit = adapter->tx_work_limit;
		if (q_vector->tx.count && !q_vector->rx.count)
			/* tx only */
			q_vector->itr = tx_itr_param;
		else
			/* rx only or mixed */
			q_vector->itr = rx_itr_param;
		ixgbe_write_eitr(q_vector);
	}

	/*
	 * do reset here at the end to make sure EITR==0 case is handled
	 * correctly w.r.t stopping tx, and changing TXDCTL.WTHRESH settings
	 * also locks in RSC enable/disable which requires reset
	 */
	if (need_reset)
		ixgbe_do_reset(netdev);

	return 0;
}

#ifndef HAVE_NDO_SET_FEATURES
static u32 ixgbe_get_rx_csum(struct net_device *netdev)
{
	return !!(netdev->features & NETIF_F_RXCSUM);
}

static int ixgbe_set_rx_csum(struct net_device *netdev, u32 data)
{
	struct ixgbe_adapter *adapter = netdev_priv(netdev);
	bool need_reset = false;

	if (data)
		netdev->features |= NETIF_F_RXCSUM;
	else
		netdev->features &= ~NETIF_F_RXCSUM;

	/* LRO and RSC both depend on RX checksum to function */
	if (!data && (netdev->features & NETIF_F_LRO)) {
		netdev->features &= ~NETIF_F_LRO;

		if (adapter->flags2 & IXGBE_FLAG2_RSC_ENABLED) {
			adapter->flags2 &= ~IXGBE_FLAG2_RSC_ENABLED;
			need_reset = true;
		}
	}

#ifdef HAVE_VXLAN_RX_OFFLOAD
	if (adapter->flags & IXGBE_FLAG_VXLAN_OFFLOAD_CAPABLE && data) {
		netdev->hw_enc_features |= NETIF_F_RXCSUM |
					   NETIF_F_IP_CSUM |
					   NETIF_F_IPV6_CSUM;
		if (!need_reset)
			adapter->flags2 |= IXGBE_FLAG2_VXLAN_REREG_NEEDED;
	} else {
		netdev->hw_enc_features &= ~(NETIF_F_RXCSUM |
					     NETIF_F_IP_CSUM |
					     NETIF_F_IPV6_CSUM);
		ixgbe_clear_udp_tunnel_port(adapter,
					    IXGBE_VXLANCTRL_ALL_UDPPORT_MASK);
	}
#endif /* HAVE_VXLAN_RX_OFFLOAD */

	if (need_reset)
		ixgbe_do_reset(netdev);

	return 0;
}

static int ixgbe_set_tx_csum(struct net_device *netdev, u32 data)
{
	struct ixgbe_adapter *adapter = netdev_priv(netdev);
#ifdef NETIF_F_IPV6_CSUM
	u32 feature_list = NETIF_F_IP_CSUM | NETIF_F_IPV6_CSUM;
#else
	u32 feature_list = NETIF_F_IP_CSUM;
#endif

	switch (adapter->hw.mac.type) {
	case ixgbe_mac_82599EB:
	case ixgbe_mac_X540:
	case ixgbe_mac_X550:
	case ixgbe_mac_X550EM_x:
	case ixgbe_mac_X550EM_a:
#ifdef HAVE_ENCAP_TSO_OFFLOAD
		if (data)
			netdev->hw_enc_features |= NETIF_F_GSO_UDP_TUNNEL;
		else
			netdev->hw_enc_features &= ~NETIF_F_GSO_UDP_TUNNEL;
		feature_list |= NETIF_F_GSO_UDP_TUNNEL;
#endif /* HAVE_ENCAP_TSO_OFFLOAD */
		feature_list |= NETIF_F_SCTP_CSUM;
		break;
	default:
		break;
	}

	if (data)
		netdev->features |= feature_list;
	else
		netdev->features &= ~feature_list;

	return 0;
}

#ifdef NETIF_F_TSO
static int ixgbe_set_tso(struct net_device *netdev, u32 data)
{
#ifdef NETIF_F_TSO6
	u32 feature_list = NETIF_F_TSO | NETIF_F_TSO6;
#else
	u32 feature_list = NETIF_F_TSO;
#endif

	if (data)
		netdev->features |= feature_list;
	else
		netdev->features &= ~feature_list;

#ifndef HAVE_NETDEV_VLAN_FEATURES
	if (!data) {
		struct ixgbe_adapter *adapter = netdev_priv(netdev);
		struct net_device *v_netdev;
		int i;

		/* disable TSO on all VLANs if they're present */
		if (!adapter->vlgrp)
			goto tso_out;

		for (i = 0; i < VLAN_GROUP_ARRAY_LEN; i++) {
			v_netdev = vlan_group_get_device(adapter->vlgrp, i);
			if (!v_netdev)
				continue;

			v_netdev->features &= ~feature_list;
			vlan_group_set_device(adapter->vlgrp, i, v_netdev);
		}
	}

tso_out:

#endif /* HAVE_NETDEV_VLAN_FEATURES */
	return 0;
}

#endif /* NETIF_F_TSO */
#ifdef ETHTOOL_GFLAGS
static int ixgbe_set_flags(struct net_device *netdev, u32 data)
{
	struct ixgbe_adapter *adapter = netdev_priv(netdev);
	u32 supported_flags = ETH_FLAG_RXVLAN | ETH_FLAG_TXVLAN;
	u32 changed = netdev->features ^ data;
	bool need_reset = false;
	int rc;

#ifndef HAVE_VLAN_RX_REGISTER
	if ((adapter->flags & IXGBE_FLAG_DCB_ENABLED) &&
	    !(data & ETH_FLAG_RXVLAN))
		return -EINVAL;

#endif
	if (adapter->flags2 & IXGBE_FLAG2_RSC_CAPABLE)
		supported_flags |= ETH_FLAG_LRO;

#ifdef ETHTOOL_GRXRINGS
	switch (adapter->hw.mac.type) {
	case ixgbe_mac_X550:
	case ixgbe_mac_X550EM_x:
	case ixgbe_mac_X550EM_a:
	case ixgbe_mac_X540:
	case ixgbe_mac_82599EB:
		supported_flags |= ETH_FLAG_NTUPLE;
	default:
		break;
	}

#endif
#ifdef NETIF_F_RXHASH
	supported_flags |= ETH_FLAG_RXHASH;

#endif
	rc = ethtool_op_set_flags(netdev, data, supported_flags);
	if (rc)
		return rc;

#ifndef HAVE_VLAN_RX_REGISTER
	if (changed & ETH_FLAG_RXVLAN)
		ixgbe_vlan_mode(netdev, netdev->features);
#endif

#ifdef HAVE_VXLAN_RX_OFFLOAD
	if (adapter->flags & IXGBE_FLAG_VXLAN_OFFLOAD_CAPABLE &&
	    netdev->features & NETIF_F_RXCSUM) {
		vxlan_get_rx_port(netdev);
	else
		ixgbe_clear_udp_tunnel_port(adapter,
					    IXGBE_VXLANCTRL_ALL_UDPPORT_MASK);
	}
#endif /* HAVE_VXLAN_RX_OFFLOAD */

	/* if state changes we need to update adapter->flags and reset */
	if (!(netdev->features & NETIF_F_LRO)) {
		if (adapter->flags2 & IXGBE_FLAG2_RSC_ENABLED)
			need_reset = true;
		adapter->flags2 &= ~IXGBE_FLAG2_RSC_ENABLED;
	} else if ((adapter->flags2 & IXGBE_FLAG2_RSC_CAPABLE) &&
		   !(adapter->flags2 & IXGBE_FLAG2_RSC_ENABLED)) {
		if (adapter->rx_itr_setting == 1 ||
		    adapter->rx_itr_setting > IXGBE_MIN_RSC_ITR) {
			adapter->flags2 |= IXGBE_FLAG2_RSC_ENABLED;
			need_reset = true;
		} else if (changed & ETH_FLAG_LRO) {
			e_info(probe, "rx-usecs set too low, "
			       "disabling RSC\n");
		}
	}

#ifdef ETHTOOL_GRXRINGS
	/*
	 * Check if Flow Director n-tuple support was enabled or disabled.  If
	 * the state changed, we need to reset.
	 */
	switch (netdev->features & NETIF_F_NTUPLE) {
	case NETIF_F_NTUPLE:
		/* turn off ATR, enable perfect filters and reset */
		if (!(adapter->flags & IXGBE_FLAG_FDIR_PERFECT_CAPABLE))
			need_reset = true;

		adapter->flags &= ~IXGBE_FLAG_FDIR_HASH_CAPABLE;
		adapter->flags |= IXGBE_FLAG_FDIR_PERFECT_CAPABLE;
		break;
	default:
		/* turn off perfect filters, enable ATR and reset */
		if (adapter->flags & IXGBE_FLAG_FDIR_PERFECT_CAPABLE)
			need_reset = true;

		adapter->flags &= ~IXGBE_FLAG_FDIR_PERFECT_CAPABLE;

		/* We cannot enable ATR if VMDq is enabled */
		if (adapter->flags & IXGBE_FLAG_VMDQ_ENABLED)
			break;

		/* We cannot enable ATR if we have 2 or more traffic classes */
		if (netdev_get_num_tc(netdev) > 1)
			break;

		/* We cannot enable ATR if RSS is disabled */
		if (adapter->ring_feature[RING_F_RSS].limit <= 1)
			break;

		/* A sample rate of 0 indicates ATR disabled */
		if (!adapter->atr_sample_rate)
			break;

		adapter->flags |= IXGBE_FLAG_FDIR_HASH_CAPABLE;
		break;
	}

#endif /* ETHTOOL_GRXRINGS */
	if (need_reset)
		ixgbe_do_reset(netdev);

	return 0;
}

#endif /* ETHTOOL_GFLAGS */
#endif /* HAVE_NDO_SET_FEATURES */
#ifdef ETHTOOL_GRXRINGS
static int ixgbe_get_ethtool_fdir_entry(struct ixgbe_adapter *adapter,
					struct ethtool_rxnfc *cmd)
{
	union ixgbe_atr_input *mask = &adapter->fdir_mask;
	struct ethtool_rx_flow_spec *fsp =
		(struct ethtool_rx_flow_spec *)&cmd->fs;
	struct hlist_node *node2;
	struct ixgbe_fdir_filter *rule = NULL;

	/* report total rule count */
	cmd->data = (1024 << adapter->fdir_pballoc) - 2;

	hlist_for_each_entry_safe(rule, node2,
				  &adapter->fdir_filter_list, fdir_node) {
		if (fsp->location <= rule->sw_idx)
			break;
	}

	if (!rule || fsp->location != rule->sw_idx)
		return -EINVAL;

	/* fill out the flow spec entry */

	/* set flow type field */
	switch (rule->filter.formatted.flow_type) {
	case IXGBE_ATR_FLOW_TYPE_TCPV4:
		fsp->flow_type = TCP_V4_FLOW;
		break;
	case IXGBE_ATR_FLOW_TYPE_UDPV4:
		fsp->flow_type = UDP_V4_FLOW;
		break;
	case IXGBE_ATR_FLOW_TYPE_TCPV6:
		fsp->flow_type = TCP_V6_FLOW;
		break;
	case IXGBE_ATR_FLOW_TYPE_UDPV6:
		fsp->flow_type = UDP_V6_FLOW;
		break;
	case IXGBE_ATR_FLOW_TYPE_SCTPV4:
		fsp->flow_type = SCTP_V4_FLOW;
		break;
	case IXGBE_ATR_FLOW_TYPE_IPV4:
		fsp->flow_type = IP_USER_FLOW;
		fsp->h_u.usr_ip4_spec.ip_ver = ETH_RX_NFC_IP4;
		fsp->h_u.usr_ip4_spec.proto = 0;
		fsp->m_u.usr_ip4_spec.proto = 0;
		break;
	default:
		return -EINVAL;
	}

#ifdef HAVE_ETHTOOL_FLOW_UNION_IP6_SPEC
	if (rule->filter.formatted.flow_type & IXGBE_ATR_L4TYPE_IPV6_MASK) {
		fsp->h_u.tcp_ip6_spec.psrc = rule->filter.formatted.src_port;
		fsp->m_u.tcp_ip6_spec.psrc = mask->formatted.src_port;
		fsp->h_u.tcp_ip6_spec.pdst = rule->filter.formatted.dst_port;
		fsp->m_u.tcp_ip6_spec.pdst = mask->formatted.dst_port;
	} else {
#endif
		fsp->h_u.tcp_ip4_spec.psrc = rule->filter.formatted.src_port;
		fsp->m_u.tcp_ip4_spec.psrc = mask->formatted.src_port;
		fsp->h_u.tcp_ip4_spec.pdst = rule->filter.formatted.dst_port;
		fsp->m_u.tcp_ip4_spec.pdst = mask->formatted.dst_port;
		fsp->h_u.tcp_ip4_spec.ip4src = rule->filter.formatted.src_ip[0];
		fsp->m_u.tcp_ip4_spec.ip4src = mask->formatted.src_ip[0];
		fsp->h_u.tcp_ip4_spec.ip4dst = rule->filter.formatted.dst_ip[0];
		fsp->m_u.tcp_ip4_spec.ip4dst = mask->formatted.dst_ip[0];
#ifdef HAVE_ETHTOOL_FLOW_UNION_IP6_SPEC
	}
#endif

	fsp->h_ext.vlan_tci = rule->filter.formatted.vlan_id;
	fsp->m_ext.vlan_tci = mask->formatted.vlan_id;
	fsp->h_ext.vlan_etype = rule->filter.formatted.flex_bytes;
	fsp->m_ext.vlan_etype = mask->formatted.flex_bytes;
	fsp->h_ext.data[1] = htonl(rule->filter.formatted.vm_pool);
	fsp->m_ext.data[1] = htonl(mask->formatted.vm_pool);
	fsp->flow_type |= FLOW_EXT;

	/* record action */
	if (rule->action == IXGBE_FDIR_DROP_QUEUE)
		fsp->ring_cookie = RX_CLS_FLOW_DISC;
	else
		fsp->ring_cookie = rule->action;

	return 0;
}

static int ixgbe_get_ethtool_fdir_all(struct ixgbe_adapter *adapter,
				      struct ethtool_rxnfc *cmd,
				      u32 *rule_locs)
{
	struct hlist_node *node2;
	struct ixgbe_fdir_filter *rule;
	int cnt = 0;

	/* report total rule count */
	cmd->data = (1024 << adapter->fdir_pballoc) - 2;

	hlist_for_each_entry_safe(rule, node2,
				  &adapter->fdir_filter_list, fdir_node) {
		if (cnt == cmd->rule_cnt)
			return -EMSGSIZE;
		rule_locs[cnt] = rule->sw_idx;
		cnt++;
	}

	cmd->rule_cnt = cnt;

	return 0;
}

static int ixgbe_get_rss_hash_opts(struct ixgbe_adapter *adapter,
				   struct ethtool_rxnfc *cmd)
{
	cmd->data = 0;

	/* Report default options for RSS on ixgbe */
	switch (cmd->flow_type) {
	case TCP_V4_FLOW:
		cmd->data |= RXH_L4_B_0_1 | RXH_L4_B_2_3;
		/* fall through */
	case UDP_V4_FLOW:
		if (adapter->flags2 & IXGBE_FLAG2_RSS_FIELD_IPV4_UDP)
			cmd->data |= RXH_L4_B_0_1 | RXH_L4_B_2_3;
		/* fall through */
	case SCTP_V4_FLOW:
	case AH_ESP_V4_FLOW:
	case AH_V4_FLOW:
	case ESP_V4_FLOW:
	case IPV4_FLOW:
		cmd->data |= RXH_IP_SRC | RXH_IP_DST;
		break;
	case TCP_V6_FLOW:
		cmd->data |= RXH_L4_B_0_1 | RXH_L4_B_2_3;
		/* fall through */
	case UDP_V6_FLOW:
		if (adapter->flags2 & IXGBE_FLAG2_RSS_FIELD_IPV6_UDP)
			cmd->data |= RXH_L4_B_0_1 | RXH_L4_B_2_3;
		/* fall through */
	case SCTP_V6_FLOW:
	case AH_ESP_V6_FLOW:
	case AH_V6_FLOW:
	case ESP_V6_FLOW:
	case IPV6_FLOW:
		cmd->data |= RXH_IP_SRC | RXH_IP_DST;
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static int ixgbe_get_rxnfc(struct net_device *dev, struct ethtool_rxnfc *cmd,
#ifdef HAVE_ETHTOOL_GET_RXNFC_VOID_RULE_LOCS
			   void *rule_locs)
#else
			   u32 *rule_locs)
#endif
{
	struct ixgbe_adapter *adapter = netdev_priv(dev);
	int ret = -EOPNOTSUPP;

	switch (cmd->cmd) {
	case ETHTOOL_GRXRINGS:
		cmd->data = adapter->num_rx_queues;
		ret = 0;
		break;
	case ETHTOOL_GRXCLSRLCNT:
		cmd->rule_cnt = adapter->fdir_filter_count;
		ret = 0;
		break;
	case ETHTOOL_GRXCLSRULE:
		ret = ixgbe_get_ethtool_fdir_entry(adapter, cmd);
		break;
	case ETHTOOL_GRXCLSRLALL:
		ret = ixgbe_get_ethtool_fdir_all(adapter, cmd,
						 (u32 *)rule_locs);
		break;
	case ETHTOOL_GRXFH:
		ret = ixgbe_get_rss_hash_opts(adapter, cmd);
		break;
	default:
		break;
	}

	return ret;
}

int ixgbe_update_ethtool_fdir_entry(struct ixgbe_adapter *adapter,
				    struct ixgbe_fdir_filter *input,
				    u16 sw_idx)
{
	struct ixgbe_hw *hw = &adapter->hw;
	struct hlist_node *node2;
	struct ixgbe_fdir_filter *rule, *parent;
	bool deleted = false;
	s32 err;

	parent = NULL;
	rule = NULL;

	hlist_for_each_entry_safe(rule, node2,
				  &adapter->fdir_filter_list, fdir_node) {
		/* hash found, or no matching entry */
		if (rule->sw_idx >= sw_idx)
			break;
		parent = rule;
	}

	/* if there is an old rule occupying our place remove it */
	if (rule && (rule->sw_idx == sw_idx)) {
		/* hardware filters are only configured when interface is up,
		 * and we should not issue filter commands while the interface
		 * is down
		 */
		if (netif_running(adapter->netdev) &&
		    (!input || (rule->filter.formatted.bkt_hash !=
				input->filter.formatted.bkt_hash))) {
			err = ixgbe_fdir_erase_perfect_filter_82599(hw,
								&rule->filter,
								sw_idx);
			if (err)
				return -EINVAL;
		}

		hlist_del(&rule->fdir_node);
		kfree(rule);
		adapter->fdir_filter_count--;
		deleted = true;
	}

	/* If we weren't given an input, then this was a request to delete a
	 * filter. We should return -EINVAL if the filter wasn't found, but
	 * return 0 if the rule was successfully deleted.
	 */
	if (!input)
		return deleted ? 0 : -EINVAL;

	/* initialize node and set software index */
	INIT_HLIST_NODE(&input->fdir_node);

	/* add filter to the list */
	if (parent)
		hlist_add_behind(&input->fdir_node, &parent->fdir_node);
	else
		hlist_add_head(&input->fdir_node,
			       &adapter->fdir_filter_list);

	/* update counts */
	adapter->fdir_filter_count++;

	return 0;
}

static int ixgbe_flowspec_to_flow_type(struct ethtool_rx_flow_spec *fsp,
				       u8 *flow_type)
{
	switch (fsp->flow_type & ~FLOW_EXT) {
	case TCP_V4_FLOW:
		*flow_type = IXGBE_ATR_FLOW_TYPE_TCPV4;
		break;
	case UDP_V4_FLOW:
		*flow_type = IXGBE_ATR_FLOW_TYPE_UDPV4;
		break;
	case TCP_V6_FLOW:
		*flow_type = IXGBE_ATR_FLOW_TYPE_TCPV6;
		break;
	case UDP_V6_FLOW:
		*flow_type = IXGBE_ATR_FLOW_TYPE_UDPV6;
		break;
	case SCTP_V4_FLOW:
		*flow_type = IXGBE_ATR_FLOW_TYPE_SCTPV4;
		break;
	case IP_USER_FLOW:
		switch (fsp->h_u.usr_ip4_spec.proto) {
		case IPPROTO_TCP:
			*flow_type = IXGBE_ATR_FLOW_TYPE_TCPV4;
			break;
		case IPPROTO_UDP:
			*flow_type = IXGBE_ATR_FLOW_TYPE_UDPV4;
			break;
		case IPPROTO_SCTP:
			*flow_type = IXGBE_ATR_FLOW_TYPE_SCTPV4;
			break;
		case 0:
			if (!fsp->m_u.usr_ip4_spec.proto) {
				*flow_type = IXGBE_ATR_FLOW_TYPE_IPV4;
				break;
			}
			/* fall through */
		default:
			return 0;
		}
		break;
	default:
		return 0;
	}

	return 1;
}

static bool ixgbe_match_ethtool_fdir_entry(struct ixgbe_adapter *adapter,
					   struct ixgbe_fdir_filter *input)
{
	struct hlist_node *node2;
	struct ixgbe_fdir_filter *rule = NULL;

	hlist_for_each_entry_safe(rule, node2,
				  &adapter->fdir_filter_list, fdir_node) {
		if (rule->filter.formatted.bkt_hash ==
		    input->filter.formatted.bkt_hash &&
		    rule->action == input->action) {
			e_info(drv, "FDIR entry already exist\n");
			return true;
		}
	}
	return false;
}

static int ixgbe_add_ethtool_fdir_entry(struct ixgbe_adapter *adapter,
					struct ethtool_rxnfc *cmd)
{
	struct ethtool_rx_flow_spec *fsp =
		(struct ethtool_rx_flow_spec *)&cmd->fs;
	struct ixgbe_hw *hw = &adapter->hw;
	struct ixgbe_fdir_filter *input;
	union ixgbe_atr_input mask;
	u8 queue;
	int err;

	if (!(adapter->flags & IXGBE_FLAG_FDIR_PERFECT_CAPABLE))
		return -EOPNOTSUPP;

	/* ring_cookie is a masked into a set of queues and ixgbe pools or
	 * we use drop index
	 */
	if (fsp->ring_cookie == RX_CLS_FLOW_DISC) {
		queue = IXGBE_FDIR_DROP_QUEUE;
	} else {
		u32 ring = ethtool_get_flow_spec_ring(fsp->ring_cookie);
		u8 vf = ethtool_get_flow_spec_ring_vf(fsp->ring_cookie);

		if (!vf && ring >= adapter->num_rx_queues)
			return -EINVAL;
		else if (vf &&
			 ((vf > adapter->num_vfs) ||
			   ring >= adapter->num_rx_queues_per_pool))
			return -EINVAL;

		/* Map the ring onto the absolute queue index */
		if (!vf)
			queue = adapter->rx_ring[ring]->reg_idx;
		else
			queue = ((vf - 1) *
				adapter->num_rx_queues_per_pool) + ring;
	}

	/* Don't allow indexes to exist outside of available space */
	if (fsp->location >= ((1024 << adapter->fdir_pballoc) - 2)) {
		e_err(drv, "Location out of range\n");
		return -EINVAL;
	}

	input = kzalloc(sizeof(*input), GFP_ATOMIC);
	if (!input)
		return -ENOMEM;

	memset(&mask, 0, sizeof(union ixgbe_atr_input));

	/* set SW index */
	input->sw_idx = fsp->location;

	/* record flow type */
	if (!ixgbe_flowspec_to_flow_type(fsp,
					 &input->filter.formatted.flow_type)) {
		e_err(drv, "Unrecognized flow type\n");
		goto err_out;
	}

	mask.formatted.flow_type = IXGBE_ATR_L4TYPE_IPV6_MASK |
				   IXGBE_ATR_L4TYPE_MASK;

	if (input->filter.formatted.flow_type == IXGBE_ATR_FLOW_TYPE_IPV4)
		mask.formatted.flow_type &= IXGBE_ATR_L4TYPE_IPV6_MASK;

#ifdef HAVE_ETHTOOL_FLOW_UNION_IP6_SPEC
	/* not support full IPV6 address filtering */
	if (input->filter.formatted.flow_type & IXGBE_ATR_L4TYPE_IPV6_MASK) {
		if (fsp->m_u.tcp_ip6_spec.ip6src[0] ||
		    fsp->m_u.tcp_ip6_spec.ip6src[1] ||
		    fsp->m_u.tcp_ip6_spec.ip6src[2] ||
		    fsp->m_u.tcp_ip6_spec.ip6src[3] ||
		    fsp->m_u.tcp_ip6_spec.ip6dst[0] ||
		    fsp->m_u.tcp_ip6_spec.ip6dst[1] ||
		    fsp->m_u.tcp_ip6_spec.ip6dst[2] ||
		    fsp->m_u.tcp_ip6_spec.ip6dst[3]) {
			e_err(drv, "Error not support IPv6 address fitlers\n");
			goto err_out;
		}
		input->filter.formatted.src_port = fsp->h_u.tcp_ip6_spec.psrc;
		mask.formatted.src_port = fsp->m_u.tcp_ip6_spec.psrc;
		input->filter.formatted.dst_port = fsp->h_u.tcp_ip6_spec.pdst;
		mask.formatted.dst_port = fsp->m_u.tcp_ip6_spec.pdst;
	} else {
#endif
		/* Copy input into formatted structures */
		input->filter.formatted.src_ip[0] =
			fsp->h_u.tcp_ip4_spec.ip4src;
		mask.formatted.src_ip[0] =
			fsp->m_u.tcp_ip4_spec.ip4src;
		input->filter.formatted.dst_ip[0] =
			fsp->h_u.tcp_ip4_spec.ip4dst;
		mask.formatted.dst_ip[0] =
			fsp->m_u.tcp_ip4_spec.ip4dst;
		input->filter.formatted.src_port =
			fsp->h_u.tcp_ip4_spec.psrc;
		mask.formatted.src_port =
			fsp->m_u.tcp_ip4_spec.psrc;
		input->filter.formatted.dst_port =
			fsp->h_u.tcp_ip4_spec.pdst;
		mask.formatted.dst_port =
			fsp->m_u.tcp_ip4_spec.pdst;
#ifdef HAVE_ETHTOOL_FLOW_UNION_IP6_SPEC
	}
#endif

	if (fsp->flow_type & FLOW_EXT) {
		input->filter.formatted.vm_pool =
				(unsigned char)ntohl(fsp->h_ext.data[1]);
		mask.formatted.vm_pool =
				(unsigned char)ntohl(fsp->m_ext.data[1]);
		input->filter.formatted.vlan_id = fsp->h_ext.vlan_tci;
		mask.formatted.vlan_id = fsp->m_ext.vlan_tci;
		input->filter.formatted.flex_bytes =
						fsp->h_ext.vlan_etype;
		mask.formatted.flex_bytes = fsp->m_ext.vlan_etype;
	}

	/* determine if we need to drop or route the packet */
	if (fsp->ring_cookie == RX_CLS_FLOW_DISC)
		input->action = IXGBE_FDIR_DROP_QUEUE;
	else
		input->action = fsp->ring_cookie;

	spin_lock(&adapter->fdir_perfect_lock);

	if (hlist_empty(&adapter->fdir_filter_list)) {
		/* save mask and program input mask into HW */
		memcpy(&adapter->fdir_mask, &mask, sizeof(mask));
		err = ixgbe_fdir_set_input_mask_82599(hw, &mask, adapter->cloud_mode);
		if (err) {
			e_err(drv, "Error writing mask\n");
			goto err_out_w_lock;
		}
	} else if (memcmp(&adapter->fdir_mask, &mask, sizeof(mask))) {
		e_err(drv, "Hardware only supports one mask per port. To change the mask you must first delete all the rules.\n");
		goto err_out_w_lock;
	}

	/* apply mask and compute/store hash */
	ixgbe_atr_compute_perfect_hash_82599(&input->filter, &mask);

	/* check if new entry does not exist on filter list */
	if (ixgbe_match_ethtool_fdir_entry(adapter, input))
		goto err_out_w_lock;

	/* only program filters to hardware if the net device is running, as
	 * we store the filters in the Rx buffer which is not allocated when
	 * the device is down
	 */
	if (netif_running(adapter->netdev)) {
		err = ixgbe_fdir_write_perfect_filter_82599(hw,
					&input->filter, input->sw_idx, queue,
					adapter->cloud_mode);
		if (err)
			goto err_out_w_lock;
	}

	ixgbe_update_ethtool_fdir_entry(adapter, input, input->sw_idx);

	spin_unlock(&adapter->fdir_perfect_lock);

	return 0;
err_out_w_lock:
	spin_unlock(&adapter->fdir_perfect_lock);
err_out:
	kfree(input);
	return -EINVAL;
}

static int ixgbe_del_ethtool_fdir_entry(struct ixgbe_adapter *adapter,
					struct ethtool_rxnfc *cmd)
{
	struct ethtool_rx_flow_spec *fsp =
		(struct ethtool_rx_flow_spec *)&cmd->fs;
	int err;

	spin_lock(&adapter->fdir_perfect_lock);
	err = ixgbe_update_ethtool_fdir_entry(adapter, NULL, fsp->location);
	spin_unlock(&adapter->fdir_perfect_lock);

	return err;
}

#ifdef ETHTOOL_SRXNTUPLE
/*
 * We need to keep this around for kernels 2.6.33 - 2.6.39 in order to avoid
 * a null pointer dereference as it was assumend if the NETIF_F_NTUPLE flag
 * was defined that this function was present.
 */
static int ixgbe_set_rx_ntuple(struct net_device __always_unused *dev,
			       struct ethtool_rx_ntuple __always_unused *cmd)
{
	return -EOPNOTSUPP;
}

#endif
#define UDP_RSS_FLAGS (IXGBE_FLAG2_RSS_FIELD_IPV4_UDP | \
		       IXGBE_FLAG2_RSS_FIELD_IPV6_UDP)
static int ixgbe_set_rss_hash_opt(struct ixgbe_adapter *adapter,
				  struct ethtool_rxnfc *nfc)
{
	u32 flags2 = adapter->flags2;

	/*
	 * RSS does not support anything other than hashing
	 * to queues on src and dst IPs and ports
	 */
	if (nfc->data & ~(RXH_IP_SRC | RXH_IP_DST |
			  RXH_L4_B_0_1 | RXH_L4_B_2_3))
		return -EINVAL;

	switch (nfc->flow_type) {
	case TCP_V4_FLOW:
	case TCP_V6_FLOW:
		if (!(nfc->data & RXH_IP_SRC) ||
		    !(nfc->data & RXH_IP_DST) ||
		    !(nfc->data & RXH_L4_B_0_1) ||
		    !(nfc->data & RXH_L4_B_2_3))
			return -EINVAL;
		break;
	case UDP_V4_FLOW:
		if (!(nfc->data & RXH_IP_SRC) ||
		    !(nfc->data & RXH_IP_DST))
			return -EINVAL;
		switch (nfc->data & (RXH_L4_B_0_1 | RXH_L4_B_2_3)) {
		case 0:
			flags2 &= ~IXGBE_FLAG2_RSS_FIELD_IPV4_UDP;
			break;
		case (RXH_L4_B_0_1 | RXH_L4_B_2_3):
			flags2 |= IXGBE_FLAG2_RSS_FIELD_IPV4_UDP;
			break;
		default:
			return -EINVAL;
		}
		break;
	case UDP_V6_FLOW:
		if (!(nfc->data & RXH_IP_SRC) ||
		    !(nfc->data & RXH_IP_DST))
			return -EINVAL;
		switch (nfc->data & (RXH_L4_B_0_1 | RXH_L4_B_2_3)) {
		case 0:
			flags2 &= ~IXGBE_FLAG2_RSS_FIELD_IPV6_UDP;
			break;
		case (RXH_L4_B_0_1 | RXH_L4_B_2_3):
			flags2 |= IXGBE_FLAG2_RSS_FIELD_IPV6_UDP;
			break;
		default:
			return -EINVAL;
		}
		break;
	case AH_ESP_V4_FLOW:
	case AH_V4_FLOW:
	case ESP_V4_FLOW:
	case SCTP_V4_FLOW:
	case AH_ESP_V6_FLOW:
	case AH_V6_FLOW:
	case ESP_V6_FLOW:
	case SCTP_V6_FLOW:
		if (!(nfc->data & RXH_IP_SRC) ||
		    !(nfc->data & RXH_IP_DST) ||
		    (nfc->data & RXH_L4_B_0_1) ||
		    (nfc->data & RXH_L4_B_2_3))
			return -EINVAL;
		break;
	default:
		return -EINVAL;
	}

	/* if we changed something we need to update flags */
	if (flags2 != adapter->flags2) {
		struct ixgbe_hw *hw = &adapter->hw;
		u32 mrqc;
		unsigned int pf_pool = adapter->num_vfs;

		if ((hw->mac.type >= ixgbe_mac_X550) &&
		    (adapter->flags & IXGBE_FLAG_SRIOV_ENABLED))
			mrqc = IXGBE_READ_REG(hw, IXGBE_PFVFMRQC(pf_pool));
		else
			mrqc = IXGBE_READ_REG(hw, IXGBE_MRQC);

		if ((flags2 & UDP_RSS_FLAGS) &&
		    !(adapter->flags2 & UDP_RSS_FLAGS))
			e_warn(drv, "enabling UDP RSS: fragmented packets"
			       " may arrive out of order to the stack above\n");

		adapter->flags2 = flags2;

		/* Perform hash on these packet types */
		mrqc |= IXGBE_MRQC_RSS_FIELD_IPV4
		      | IXGBE_MRQC_RSS_FIELD_IPV4_TCP
		      | IXGBE_MRQC_RSS_FIELD_IPV6
		      | IXGBE_MRQC_RSS_FIELD_IPV6_TCP;

		mrqc &= ~(IXGBE_MRQC_RSS_FIELD_IPV4_UDP |
			  IXGBE_MRQC_RSS_FIELD_IPV6_UDP);

		if (flags2 & IXGBE_FLAG2_RSS_FIELD_IPV4_UDP)
			mrqc |= IXGBE_MRQC_RSS_FIELD_IPV4_UDP;

		if (flags2 & IXGBE_FLAG2_RSS_FIELD_IPV6_UDP)
			mrqc |= IXGBE_MRQC_RSS_FIELD_IPV6_UDP;

		if ((hw->mac.type >= ixgbe_mac_X550) &&
		    (adapter->flags & IXGBE_FLAG_SRIOV_ENABLED))
			IXGBE_WRITE_REG(hw, IXGBE_PFVFMRQC(pf_pool), mrqc);
		else
			IXGBE_WRITE_REG(hw, IXGBE_MRQC, mrqc);
	}

	return 0;
}

static int ixgbe_set_rxnfc(struct net_device *dev, struct ethtool_rxnfc *cmd)
{
	struct ixgbe_adapter *adapter = netdev_priv(dev);
	int ret = -EOPNOTSUPP;

	switch (cmd->cmd) {
	case ETHTOOL_SRXCLSRLINS:
		ret = ixgbe_add_ethtool_fdir_entry(adapter, cmd);
		break;
	case ETHTOOL_SRXCLSRLDEL:
		ret = ixgbe_del_ethtool_fdir_entry(adapter, cmd);
		break;
	case ETHTOOL_SRXFH:
		ret = ixgbe_set_rss_hash_opt(adapter, cmd);
		break;
	default:
		break;
	}

	return ret;
}

#if defined(ETHTOOL_GRSSH) && defined(ETHTOOL_SRSSH)
static int ixgbe_rss_indir_tbl_max(struct ixgbe_adapter *adapter)
{
	if (adapter->hw.mac.type < ixgbe_mac_X550)
		return 16;
	else
		return 64;
}

static u32 ixgbe_get_rxfh_key_size(struct net_device *netdev)
{
	return IXGBE_RSS_KEY_SIZE;
}

static u32 ixgbe_rss_indir_size(struct net_device *netdev)
{
	struct ixgbe_adapter *adapter = netdev_priv(netdev);

	return ixgbe_rss_indir_tbl_entries(adapter);
}

static void ixgbe_get_reta(struct ixgbe_adapter *adapter, u32 *indir)
{
	int i, reta_size = ixgbe_rss_indir_tbl_entries(adapter);
	u16 rss_m = adapter->ring_feature[RING_F_RSS].mask;

	if (adapter->flags & IXGBE_FLAG_SRIOV_ENABLED)
		rss_m = adapter->ring_feature[RING_F_RSS].indices - 1;

	for (i = 0; i < reta_size; i++)
		indir[i] = adapter->rss_indir_tbl[i] & rss_m;
}

#ifdef HAVE_RXFH_HASHFUNC
static int ixgbe_get_rxfh(struct net_device *netdev, u32 *indir, u8 *key,
			  u8 *hfunc)
#else
static int ixgbe_get_rxfh(struct net_device *netdev, u32 *indir, u8 *key)
#endif
{
	struct ixgbe_adapter *adapter = netdev_priv(netdev);

#ifdef HAVE_RXFH_HASHFUNC
	if (hfunc)
		*hfunc = ETH_RSS_HASH_TOP;
#endif

	if (indir)
		ixgbe_get_reta(adapter, indir);

	if (key)
		memcpy(key, adapter->rss_key, ixgbe_get_rxfh_key_size(netdev));

	return 0;
}

#ifdef HAVE_RXFH_HASHFUNC
static int ixgbe_set_rxfh(struct net_device *netdev, const u32 *indir,
			  const u8 *key, const u8 hfunc)
#else
#ifdef HAVE_RXFH_NONCONST
static int ixgbe_set_rxfh(struct net_device *netdev, u32 *indir, u8 *key)
#else
static int ixgbe_set_rxfh(struct net_device *netdev, const u32 *indir,
			  const u8 *key)
#endif /* HAVE_RXFH_NONCONST */
#endif /* HAVE_RXFH_HASHFUNC */
{
	struct ixgbe_adapter *adapter = netdev_priv(netdev);
	int i;
	u32 reta_entries = ixgbe_rss_indir_tbl_entries(adapter);

#ifdef HAVE_RXFH_HASHFUNC
	if (hfunc)
		return -EINVAL;
#endif

	/* Fill out the redirection table */
	if (indir) {
		int max_queues = min_t(int, adapter->num_rx_queues,
				       ixgbe_rss_indir_tbl_max(adapter));

		/*Allow at least 2 queues w/ SR-IOV.*/
		if ((adapter->flags & IXGBE_FLAG_SRIOV_ENABLED) &&
		    (max_queues < 2))
			max_queues = 2;

		/* Verify user input. */
		for (i = 0; i < reta_entries; i++)
			if (indir[i] >= max_queues)
				return -EINVAL;

		for (i = 0; i < reta_entries; i++)
			adapter->rss_indir_tbl[i] = indir[i];

		ixgbe_store_reta(adapter);
	}

	/* Fill out the rss hash key */
	if (key) {
		memcpy(adapter->rss_key, key, ixgbe_get_rxfh_key_size(netdev));
		ixgbe_store_key(adapter);
	}

	return 0;
}
#endif /* ETHTOOL_GRSSH && ETHTOOL_SRSSH */

#ifdef HAVE_ETHTOOL_GET_TS_INFO
static int ixgbe_get_ts_info(struct net_device *dev,
			     struct ethtool_ts_info *info)
{
	struct ixgbe_adapter *adapter = netdev_priv(dev);

	/* we always support timestamping disabled */
	info->rx_filters = BIT(HWTSTAMP_FILTER_NONE);

	switch (adapter->hw.mac.type) {
#ifdef HAVE_PTP_1588_CLOCK
	case ixgbe_mac_X550:
	case ixgbe_mac_X550EM_x:
	case ixgbe_mac_X550EM_a:
		info->rx_filters |= BIT(HWTSTAMP_FILTER_ALL);
		break;
	case ixgbe_mac_X540:
	case ixgbe_mac_82599EB:
		info->rx_filters |=
			BIT(HWTSTAMP_FILTER_PTP_V1_L4_SYNC) |
			BIT(HWTSTAMP_FILTER_PTP_V1_L4_DELAY_REQ) |
			BIT(HWTSTAMP_FILTER_PTP_V2_EVENT);
		break;
#endif /* HAVE_PTP_1588_CLOCK */
	default:
		return ethtool_op_get_ts_info(dev, info);
	}
#ifdef HAVE_PTP_1588_CLOCK

	info->so_timestamping =
			SOF_TIMESTAMPING_TX_SOFTWARE |
			SOF_TIMESTAMPING_RX_SOFTWARE |
			SOF_TIMESTAMPING_SOFTWARE |
			SOF_TIMESTAMPING_TX_HARDWARE |
			SOF_TIMESTAMPING_RX_HARDWARE |
			SOF_TIMESTAMPING_RAW_HARDWARE;

	if (adapter->ptp_clock)
		info->phc_index = ptp_clock_index(adapter->ptp_clock);
	else
		info->phc_index = -1;

	info->tx_types =
		BIT(HWTSTAMP_TX_OFF) |
		BIT(HWTSTAMP_TX_ON);

	return 0;
#endif /* HAVE_PTP_1588_CLOCK */
}
#endif /* HAVE_ETHTOOL_GET_TS_INFO */

#endif /* ETHTOOL_GRXRINGS */
#ifdef ETHTOOL_SCHANNELS
static unsigned int ixgbe_max_channels(struct ixgbe_adapter *adapter)
{
	unsigned int max_combined;
	u8 tcs = netdev_get_num_tc(adapter->netdev);

	if (!(adapter->flags & IXGBE_FLAG_MSIX_ENABLED)) {
		/* We only support one q_vector without MSI-X */
		max_combined = 1;
	} else if (adapter->flags & IXGBE_FLAG_SRIOV_ENABLED) {
		/* Limit value based on the queue mask */
		max_combined = adapter->ring_feature[RING_F_RSS].mask + 1;
	} else if (tcs > 1) {
		/* For DCB report channels per traffic class */
		if (adapter->hw.mac.type == ixgbe_mac_82598EB) {
			/* 8 TC w/ 4 queues per TC */
			max_combined = 4;
		} else if (tcs > 4) {
			/* 8 TC w/ 8 queues per TC */
			max_combined = 8;
		} else {
			/* 4 TC w/ 16 queues per TC */
			max_combined = 16;
		}
	} else if (adapter->atr_sample_rate) {
		/* support up to 64 queues with ATR */
		max_combined = IXGBE_MAX_FDIR_INDICES;
	} else {
		/* support up to max allowed queues with RSS */
		max_combined = ixgbe_max_rss_indices(adapter);
	}

	return max_combined;
}

static void ixgbe_get_channels(struct net_device *dev,
			       struct ethtool_channels *ch)
{
	struct ixgbe_adapter *adapter = netdev_priv(dev);

	/* report maximum channels */
	ch->max_combined = ixgbe_max_channels(adapter);

	/* report info for other vector */
	if (adapter->flags & IXGBE_FLAG_MSIX_ENABLED) {
		ch->max_other = NON_Q_VECTORS;
		ch->other_count = NON_Q_VECTORS;
	}

	/* record RSS queues */
	ch->combined_count = adapter->ring_feature[RING_F_RSS].indices;

	/* nothing else to report if RSS is disabled */
	if (ch->combined_count == 1)
		return;

	/* we do not support ATR queueing if SR-IOV is enabled */
	if (adapter->flags & IXGBE_FLAG_SRIOV_ENABLED)
		return;

	/* same thing goes for being DCB enabled */
	if (netdev_get_num_tc(dev) > 1)
		return;

	/* if ATR is disabled we can exit */
	if (!adapter->atr_sample_rate)
		return;

	/* report flow director queues as maximum channels */
	ch->combined_count = adapter->ring_feature[RING_F_FDIR].indices;
}

static int ixgbe_set_channels(struct net_device *dev,
			      struct ethtool_channels *ch)
{
	struct ixgbe_adapter *adapter = netdev_priv(dev);
	unsigned int count = ch->combined_count;
	u8 max_rss_indices = ixgbe_max_rss_indices(adapter);

	/* verify they are not requesting separate vectors */
	if (!count || ch->rx_count || ch->tx_count)
		return -EINVAL;

	/* verify other_count has not changed */
	if (ch->other_count != NON_Q_VECTORS)
		return -EINVAL;

	/* verify the number of channels does not exceed hardware limits */
	if (count > ixgbe_max_channels(adapter))
		return -EINVAL;

	/* update feature limits from largest to smallest supported values */
	adapter->ring_feature[RING_F_FDIR].limit = count;

	/* cap RSS limit */
	if (count > max_rss_indices)
		count = max_rss_indices;
	adapter->ring_feature[RING_F_RSS].limit = count;

#if IS_ENABLED(CONFIG_FCOE)
	/* cap FCoE limit at 8 */
	if (count > IXGBE_FCRETA_SIZE)
		count = IXGBE_FCRETA_SIZE;
	adapter->ring_feature[RING_F_FCOE].limit = count;
#endif /* CONFIG_FCOE */

	/* use setup TC to update any traffic class queue mapping */
	return ixgbe_setup_tc(dev, netdev_get_num_tc(dev));
}
#endif /* ETHTOOL_SCHANNELS */

#ifdef ETHTOOL_GMODULEINFO
static int ixgbe_get_module_info(struct net_device *dev,
				       struct ethtool_modinfo *modinfo)
{
	struct ixgbe_adapter *adapter = netdev_priv(dev);
	struct ixgbe_hw *hw = &adapter->hw;
	u32 status;
	u8 sff8472_rev, addr_mode;
	bool page_swap = false;

	/* Check whether we support SFF-8472 or not */
	status = hw->phy.ops.read_i2c_eeprom(hw,
					     IXGBE_SFF_SFF_8472_COMP,
					     &sff8472_rev);
	if (status != 0)
		return -EIO;

	/* addressing mode is not supported */
	status = hw->phy.ops.read_i2c_eeprom(hw,
					     IXGBE_SFF_SFF_8472_SWAP,
					     &addr_mode);
	if (status != 0)
		return -EIO;

	if (addr_mode & IXGBE_SFF_ADDRESSING_MODE) {
		e_err(drv, "Address change required to access page 0xA2, but not supported. Please report the module type to the driver maintainers.\n");
		page_swap = true;
	}

	if (sff8472_rev == IXGBE_SFF_SFF_8472_UNSUP || page_swap) {
		/* We have a SFP, but it does not support SFF-8472 */
		modinfo->type = ETH_MODULE_SFF_8079;
		modinfo->eeprom_len = ETH_MODULE_SFF_8079_LEN;
	} else {
		/* We have a SFP which supports a revision of SFF-8472. */
		modinfo->type = ETH_MODULE_SFF_8472;
		modinfo->eeprom_len = ETH_MODULE_SFF_8472_LEN;
	}

	return 0;
}

static int ixgbe_get_module_eeprom(struct net_device *dev,
					 struct ethtool_eeprom *ee,
					 u8 *data)
{
	struct ixgbe_adapter *adapter = netdev_priv(dev);
	struct ixgbe_hw *hw = &adapter->hw;
	u32 status = IXGBE_ERR_PHY_ADDR_INVALID;
	u8 databyte = 0xFF;
	int i = 0;

	if (ee->len == 0)
		return -EINVAL;

	for (i = ee->offset; i < ee->offset + ee->len; i++) {
		/* I2C reads can take long time */
		if (test_bit(__IXGBE_IN_SFP_INIT, &adapter->state))
			return -EBUSY;

		if (i < ETH_MODULE_SFF_8079_LEN)
			status = hw->phy.ops.read_i2c_eeprom(hw, i, &databyte);
		else
			status = hw->phy.ops.read_i2c_sff8472(hw, i, &databyte);

		if (status != 0)
			return -EIO;

		data[i - ee->offset] = databyte;
	}

	return 0;
}
#endif /* ETHTOOL_GMODULEINFO */

#ifdef ETHTOOL_GEEE

static const struct {
	ixgbe_link_speed mac_speed;
	u32 supported;
} ixgbe_ls_map[] = {
	{ IXGBE_LINK_SPEED_10_FULL, SUPPORTED_10baseT_Full },
	{ IXGBE_LINK_SPEED_100_FULL, SUPPORTED_100baseT_Full },
	{ IXGBE_LINK_SPEED_1GB_FULL, SUPPORTED_1000baseT_Full },
	{ IXGBE_LINK_SPEED_2_5GB_FULL, SUPPORTED_2500baseX_Full },
	{ IXGBE_LINK_SPEED_10GB_FULL, SUPPORTED_10000baseT_Full },
};

static const struct {
	u32 lp_advertised;
	u32 mac_speed;
} ixgbe_lp_map[] = {
	{ FW_PHY_ACT_UD_2_100M_TX_EEE, SUPPORTED_100baseT_Full },
	{ FW_PHY_ACT_UD_2_1G_T_EEE, SUPPORTED_1000baseT_Full },
	{ FW_PHY_ACT_UD_2_10G_T_EEE, SUPPORTED_10000baseT_Full },
	{ FW_PHY_ACT_UD_2_1G_KX_EEE, SUPPORTED_1000baseKX_Full },
	{ FW_PHY_ACT_UD_2_10G_KX4_EEE, SUPPORTED_10000baseKX4_Full },
	{ FW_PHY_ACT_UD_2_10G_KR_EEE, SUPPORTED_10000baseKR_Full},
};

static int
ixgbe_get_eee_fw(struct ixgbe_adapter *adapter, struct ethtool_eee *edata)
{
	u32 info[FW_PHY_ACT_DATA_COUNT] = { 0 };
	struct ixgbe_hw *hw = &adapter->hw;
	s32 rc;
	u16 i;

	rc = ixgbe_fw_phy_activity(hw, FW_PHY_ACT_UD_2, &info);
	if (rc)
		return rc;

	edata->lp_advertised = 0;
	for (i = 0; i < ARRAY_SIZE(ixgbe_lp_map); ++i) {
		if (info[0] & ixgbe_lp_map[i].lp_advertised)
			edata->lp_advertised |= ixgbe_lp_map[i].mac_speed;
	}

	edata->supported = 0;
	for (i = 0; i < ARRAY_SIZE(ixgbe_ls_map); ++i) {
		if (hw->phy.eee_speeds_supported & ixgbe_ls_map[i].mac_speed)
			edata->supported |= ixgbe_ls_map[i].supported;
	}

	edata->advertised = 0;
	for (i = 0; i < ARRAY_SIZE(ixgbe_ls_map); ++i) {
		if (hw->phy.eee_speeds_advertised & ixgbe_ls_map[i].mac_speed)
			edata->advertised |= ixgbe_ls_map[i].supported;
	}

	edata->eee_enabled = !!edata->advertised;
	edata->tx_lpi_enabled = edata->eee_enabled;
	if (edata->advertised & edata->lp_advertised)
		edata->eee_active = true;

	return 0;
}

static int ixgbe_get_eee(struct net_device *netdev, struct ethtool_eee *edata)
{
	struct ixgbe_adapter *adapter = netdev_priv(netdev);
	struct ixgbe_hw *hw = &adapter->hw;

	if (!hw->mac.ops.setup_eee)
		return -EOPNOTSUPP;

	if (!(adapter->flags2 & IXGBE_FLAG2_EEE_CAPABLE))
		return -EOPNOTSUPP;

	if (hw->phy.eee_speeds_supported && hw->phy.type == ixgbe_phy_fw)
		return ixgbe_get_eee_fw(adapter, edata);

	return -EOPNOTSUPP;
}
#endif /* ETHTOOL_GEEE */

#ifdef ETHTOOL_SEEE
static int ixgbe_set_eee(struct net_device *netdev, struct ethtool_eee *edata)
{
	struct ixgbe_adapter *adapter = netdev_priv(netdev);
	struct ixgbe_hw *hw = &adapter->hw;
	struct ethtool_eee eee_data;
	s32 ret_val;

	if (!(hw->mac.ops.setup_eee &&
	    (adapter->flags2 & IXGBE_FLAG2_EEE_CAPABLE)))
		return -EOPNOTSUPP;

	memset(&eee_data, 0, sizeof(struct ethtool_eee));

	ret_val = ixgbe_get_eee(netdev, &eee_data);
	if (ret_val)
		return ret_val;

	if (eee_data.tx_lpi_enabled != edata->tx_lpi_enabled) {
		e_dev_err("Setting EEE tx-lpi is not supported\n");
		return -EINVAL;
	}

	if (eee_data.tx_lpi_timer != edata->tx_lpi_timer) {
		e_dev_err("Setting EEE Tx LPI timer is not supported\n");
		return -EINVAL;
	}

	if (eee_data.advertised != edata->advertised) {
		e_dev_err("Setting EEE advertised speeds is not supported\n");
		return -EINVAL;
	}

	if (eee_data.eee_enabled != edata->eee_enabled) {

		if (edata->eee_enabled) {
			adapter->flags2 |= IXGBE_FLAG2_EEE_ENABLED;
			hw->phy.eee_speeds_advertised =
						   hw->phy.eee_speeds_supported;
		} else {
			adapter->flags2 &= ~IXGBE_FLAG2_EEE_ENABLED;
			hw->phy.eee_speeds_advertised = 0;
		}

		/* reset link */
		if (netif_running(netdev))
			ixgbe_reinit_locked(adapter);
		else
			ixgbe_reset(adapter);
	}

	return 0;
}
#endif /* ETHTOOL_SEEE */

#ifdef HAVE_ETHTOOL_GET_SSET_COUNT
/**
 * ixgbe_get_priv_flags - report device private flags
 * @netdev: network interface device structure
 *
 * The get string set count and the string set should be matched for each
 * flag returned.  Add new strings for each flag to the ixgbe_priv_flags_strings
 * array.
 *
 * Returns a u32 bitmap of flags.
 **/
static u32 ixgbe_get_priv_flags(struct net_device *netdev)
{
	struct ixgbe_adapter *adapter = netdev_priv(netdev);
	u32 priv_flags = 0;

	if (adapter->flags & IXGBE_FLAG_FDIR_HASH_CAPABLE)
		priv_flags |= IXGBE_PRIV_FLAGS_FD_ATR;
#ifdef HAVE_SWIOTLB_SKIP_CPU_SYNC

	if (adapter->flags2 & IXGBE_FLAG2_RX_LEGACY)
		priv_flags |= IXGBE_PRIV_FLAGS_LEGACY_RX;
#endif

	return priv_flags;
}

/**
 * ixgbe_set_priv_flags - set private flags
 * @netdev: network interface device structure
 * @priv_flags: bit flags to be set
 **/
static int ixgbe_set_priv_flags(struct net_device *netdev, u32 priv_flags)
{
	struct ixgbe_adapter *adapter = netdev_priv(netdev);
#ifdef HAVE_SWIOTLB_SKIP_CPU_SYNC
	unsigned int flags2 = adapter->flags2;
#endif
	unsigned int flags = adapter->flags;

	/* allow the user to control the state of the Flow
	 * Director ATR (Application Targeted Routing) feature
	 * of the driver
	 */
	flags &= ~IXGBE_FLAG_FDIR_HASH_CAPABLE;
	if (priv_flags & IXGBE_PRIV_FLAGS_FD_ATR) {
		/* We cannot enable ATR if VMDq is enabled */
		if (flags & IXGBE_FLAG_VMDQ_ENABLED)
			return -EINVAL;
		/* We cannot enable ATR if we have 2 or more traffic classes */
		if (netdev_get_num_tc(netdev) > 1)
			return -EINVAL;
		/* We cannot enable ATR if RSS is disabled */
		if (adapter->ring_feature[RING_F_RSS].limit <= 1)
			return -EINVAL;
		/* A sample rate of 0 indicates ATR disabled */
		if (!adapter->atr_sample_rate)
			return -EINVAL;
		flags |= IXGBE_FLAG_FDIR_HASH_CAPABLE;
	}
#ifdef HAVE_SWIOTLB_SKIP_CPU_SYNC

	flags2 &= ~IXGBE_FLAG2_RX_LEGACY;
	if (priv_flags & IXGBE_PRIV_FLAGS_LEGACY_RX)
		flags2 |= IXGBE_FLAG2_RX_LEGACY;
#endif

	if (flags != adapter->flags) {
		adapter->flags = flags;

		/* ATR state change requires a reset */
		ixgbe_do_reset(netdev);
#ifndef HAVE_SWIOTLB_SKIP_CPU_SYNC
	}
#else
	} else if (flags2 != adapter->flags2) {
		adapter->flags2 = flags2;

		/* reset interface to repopulate queues */
		if (netif_running(netdev))
			ixgbe_reinit_locked(adapter);
	}
#endif

	return 0;
}

#endif /* HAVE_ETHTOOL_GET_SSET_COUNT */
static struct ethtool_ops ixgbe_ethtool_ops = {
#ifdef ETHTOOL_GLINKSETTINGS
	.get_link_ksettings	= ixgbe_get_link_ksettings,
	.set_link_ksettings	= ixgbe_set_link_ksettings,
#else
	.get_settings		= ixgbe_get_settings,
	.set_settings		= ixgbe_set_settings,
#endif
	.get_drvinfo		= ixgbe_get_drvinfo,
	.get_regs_len		= ixgbe_get_regs_len,
	.get_regs		= ixgbe_get_regs,
	.get_wol		= ixgbe_get_wol,
	.set_wol		= ixgbe_set_wol,
	.nway_reset		= ixgbe_nway_reset,
	.get_link		= ethtool_op_get_link,
	.get_eeprom_len		= ixgbe_get_eeprom_len,
	.get_eeprom		= ixgbe_get_eeprom,
	.set_eeprom		= ixgbe_set_eeprom,
	.get_ringparam		= ixgbe_get_ringparam,
	.set_ringparam		= ixgbe_set_ringparam,
	.get_pauseparam		= ixgbe_get_pauseparam,
	.set_pauseparam		= ixgbe_set_pauseparam,
	.get_msglevel		= ixgbe_get_msglevel,
	.set_msglevel		= ixgbe_set_msglevel,
#ifndef HAVE_ETHTOOL_GET_SSET_COUNT
	.self_test_count	= ixgbe_diag_test_count,
#endif /* HAVE_ETHTOOL_GET_SSET_COUNT */
	.self_test		= ixgbe_diag_test,
	.get_strings		= ixgbe_get_strings,
#ifndef HAVE_RHEL6_ETHTOOL_OPS_EXT_STRUCT
#ifdef HAVE_ETHTOOL_SET_PHYS_ID
	.set_phys_id		= ixgbe_set_phys_id,
#else
	.phys_id		= ixgbe_phys_id,
#endif /* HAVE_ETHTOOL_SET_PHYS_ID */
#endif /* HAVE_RHEL6_ETHTOOL_OPS_EXT_STRUCT */
#ifndef HAVE_ETHTOOL_GET_SSET_COUNT
	.get_stats_count	= ixgbe_get_stats_count,
#else /* HAVE_ETHTOOL_GET_SSET_COUNT */
	.get_sset_count		= ixgbe_get_sset_count,
	.get_priv_flags		= ixgbe_get_priv_flags,
	.set_priv_flags		= ixgbe_set_priv_flags,
#endif /* HAVE_ETHTOOL_GET_SSET_COUNT */
	.get_ethtool_stats      = ixgbe_get_ethtool_stats,
#ifdef HAVE_ETHTOOL_GET_PERM_ADDR
	.get_perm_addr		= ethtool_op_get_perm_addr,
#endif
	.get_coalesce		= ixgbe_get_coalesce,
	.set_coalesce		= ixgbe_set_coalesce,
#ifdef ETHTOOL_COALESCE_USECS
	.supported_coalesce_params = ETHTOOL_COALESCE_USECS,
#endif
#ifndef HAVE_NDO_SET_FEATURES
	.get_rx_csum		= ixgbe_get_rx_csum,
	.set_rx_csum		= ixgbe_set_rx_csum,
	.get_tx_csum		= ethtool_op_get_tx_csum,
	.set_tx_csum		= ixgbe_set_tx_csum,
	.get_sg			= ethtool_op_get_sg,
	.set_sg			= ethtool_op_set_sg,
#ifdef NETIF_F_TSO
	.get_tso		= ethtool_op_get_tso,
	.set_tso		= ixgbe_set_tso,
#endif
#ifdef ETHTOOL_GFLAGS
	.get_flags		= ethtool_op_get_flags,
	.set_flags		= ixgbe_set_flags,
#endif
#endif /* HAVE_NDO_SET_FEATURES */
#ifdef ETHTOOL_GRXRINGS
	.get_rxnfc		= ixgbe_get_rxnfc,
	.set_rxnfc		= ixgbe_set_rxnfc,
#ifdef ETHTOOL_SRXNTUPLE
	.set_rx_ntuple		= ixgbe_set_rx_ntuple,
#endif
#endif /* ETHTOOL_GRXRINGS */
#ifndef HAVE_RHEL6_ETHTOOL_OPS_EXT_STRUCT
#ifdef ETHTOOL_GEEE
	.get_eee		= ixgbe_get_eee,
#endif /* ETHTOOL_GEEE */
#ifdef ETHTOOL_SEEE
	.set_eee		= ixgbe_set_eee,
#endif /* ETHTOOL_SEEE */
#ifdef ETHTOOL_SCHANNELS
	.get_channels		= ixgbe_get_channels,
	.set_channels		= ixgbe_set_channels,
#endif
#ifdef ETHTOOL_GMODULEINFO
	.get_module_info	= ixgbe_get_module_info,
	.get_module_eeprom	= ixgbe_get_module_eeprom,
#endif
#ifdef HAVE_ETHTOOL_GET_TS_INFO
	.get_ts_info		= ixgbe_get_ts_info,
#endif
#if defined(ETHTOOL_GRSSH) && defined(ETHTOOL_SRSSH)
	.get_rxfh_indir_size	= ixgbe_rss_indir_size,
	.get_rxfh_key_size	= ixgbe_get_rxfh_key_size,
	.get_rxfh		= ixgbe_get_rxfh,
	.set_rxfh		= ixgbe_set_rxfh,
#endif /* ETHTOOL_GRSSH && ETHTOOL_SRSSH */
#endif /* HAVE_RHEL6_ETHTOOL_OPS_EXT_STRUCT */
};

#ifdef HAVE_RHEL6_ETHTOOL_OPS_EXT_STRUCT
static const struct ethtool_ops_ext ixgbe_ethtool_ops_ext = {
	.size			= sizeof(struct ethtool_ops_ext),
	.get_ts_info		= ixgbe_get_ts_info,
	.set_phys_id		= ixgbe_set_phys_id,
	.get_channels		= ixgbe_get_channels,
	.set_channels		= ixgbe_set_channels,
#ifdef ETHTOOL_GMODULEINFO
	.get_module_info	= ixgbe_get_module_info,
	.get_module_eeprom	= ixgbe_get_module_eeprom,
#endif
#if defined(ETHTOOL_GRSSH) && defined(ETHTOOL_SRSSH)
	.get_rxfh_indir_size	= ixgbe_rss_indir_size,
	.get_rxfh_key_size	= ixgbe_get_rxfh_key_size,
	.get_rxfh		= ixgbe_get_rxfh,
	.set_rxfh		= ixgbe_set_rxfh,
#endif /* ETHTOOL_GRSSH && ETHTOOL_SRSSH */
#ifdef ETHTOOL_GEEE
	.get_eee		= ixgbe_get_eee,
#endif /* ETHTOOL_GEEE */
#ifdef ETHTOOL_SEEE
	.set_eee		= ixgbe_set_eee,
#endif /* ETHTOOL_SEEE */
};

#endif /* HAVE_RHEL6_ETHTOOL_OPS_EXT_STRUCT */
void ixgbe_set_ethtool_ops(struct net_device *netdev)
{
#ifndef ETHTOOL_OPS_COMPAT
	netdev->ethtool_ops = &ixgbe_ethtool_ops;
#else
	SET_ETHTOOL_OPS(netdev, &ixgbe_ethtool_ops);
#endif

#ifdef HAVE_RHEL6_ETHTOOL_OPS_EXT_STRUCT
	set_ethtool_ops_ext(netdev, &ixgbe_ethtool_ops_ext);
#endif /* HAVE_RHEL6_ETHTOOL_OPS_EXT_STRUCT */
}
#endif /* SIOCETHTOOL */

