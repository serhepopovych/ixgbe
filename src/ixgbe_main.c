// SPDX-License-Identifier: GPL-2.0
/* Copyright(c) 1999 - 2020 Intel Corporation. */

/******************************************************************************
 Copyright (c)2006 - 2007 Myricom, Inc. for some LRO specific code
******************************************************************************/
#include <linux/types.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/netdevice.h>
#include <linux/vmalloc.h>
#include <linux/highmem.h>
#include <linux/string.h>
#include <linux/in.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/pkt_sched.h>
#include <linux/ipv6.h>
#ifdef NETIF_F_TSO
#include <net/checksum.h>
#ifdef NETIF_F_TSO6
#include <net/ipv6.h>
#include <net/ip6_checksum.h>
#endif /* NETIF_F_TSO6 */
#endif /* NETIF_F_TSO */
#ifdef SIOCETHTOOL
#include <linux/ethtool.h>
#endif

#include <linux/if_bridge.h>
#include "ixgbe.h"
#ifdef HAVE_XDP_SUPPORT
#include <linux/bpf.h>
#include <linux/bpf_trace.h>
#include <linux/atomic.h>
#endif
#ifdef HAVE_AF_XDP_ZC_SUPPORT
#include <net/xdp_sock.h>
#endif
#ifdef HAVE_UDP_ENC_RX_OFFLOAD
#include <net/vxlan.h>
#include <net/udp_tunnel.h>
#endif /* HAVE_UDP_ENC_RX_OFFLOAD */
#ifdef HAVE_VXLAN_RX_OFFLOAD
#include <net/vxlan.h>
#endif /* HAVE_VXLAN_RX_OFFLOAD */
#ifdef NETIF_F_HW_TC
#include <net/pkt_cls.h>
#include <net/tc_act/tc_gact.h>
#include <net/tc_act/tc_mirred.h>
#endif /* NETIF_F_HW_TC */

#include "ixgbe_dcb_82599.h"
#include "ixgbe_sriov.h"
#include "ixgbe_txrx_common.h"
#ifdef HAVE_TC_SETUP_CLSU32
#include "ixgbe_model.h"
#endif /* HAVE_TC_SETUP_CLSU32 */

#define DRV_HW_PERF

#define FPGA

#define DRIVERIOV

#define BYPASS_TAG

#define RELEASE_TAG

#define DRV_VERSION	"5.9.4" \
			DRIVERIOV DRV_HW_PERF FPGA \
			BYPASS_TAG RELEASE_TAG
#define DRV_SUMMARY	"Intel(R) 10GbE PCI Express Linux Network Driver"
const char ixgbe_driver_version[] = DRV_VERSION;
#ifdef HAVE_NON_CONST_PCI_DRIVER_NAME
char ixgbe_driver_name[] = "ixgbe";
#else
const char ixgbe_driver_name[] = "ixgbe";
#endif
static const char ixgbe_driver_string[] = DRV_SUMMARY;
static const char ixgbe_copyright[] = "Copyright(c) 1999 - 2020 Intel Corporation.";
static const char ixgbe_overheat_msg[] =
		"Network adapter has been stopped because it has over heated. "
		"Restart the computer. If the problem persists, "
		"power off the system and replace the adapter";

/* ixgbe_pci_tbl - PCI Device ID Table
 *
 * Wildcard entries (PCI_ANY_ID) should come last
 * Last entry must be all 0s
 *
 * { Vendor ID, Device ID, SubVendor ID, SubDevice ID,
 *   Class, Class Mask, private data (not used) }
 */
static const struct pci_device_id ixgbe_pci_tbl[] = {
	{PCI_VDEVICE(INTEL, IXGBE_DEV_ID_82598), 0},
	{PCI_VDEVICE(INTEL, IXGBE_DEV_ID_82598AF_DUAL_PORT), 0},
	{PCI_VDEVICE(INTEL, IXGBE_DEV_ID_82598AF_SINGLE_PORT), 0},
	{PCI_VDEVICE(INTEL, IXGBE_DEV_ID_82598AT), 0},
	{PCI_VDEVICE(INTEL, IXGBE_DEV_ID_82598AT2), 0},
	{PCI_VDEVICE(INTEL, IXGBE_DEV_ID_82598EB_CX4), 0},
	{PCI_VDEVICE(INTEL, IXGBE_DEV_ID_82598_CX4_DUAL_PORT), 0},
	{PCI_VDEVICE(INTEL, IXGBE_DEV_ID_82598_DA_DUAL_PORT), 0},
	{PCI_VDEVICE(INTEL, IXGBE_DEV_ID_82598_SR_DUAL_PORT_EM), 0},
	{PCI_VDEVICE(INTEL, IXGBE_DEV_ID_82598EB_XF_LR), 0},
	{PCI_VDEVICE(INTEL, IXGBE_DEV_ID_82598EB_SFP_LOM), 0},
	{PCI_VDEVICE(INTEL, IXGBE_DEV_ID_82598_BX), 0},
	{PCI_VDEVICE(INTEL, IXGBE_DEV_ID_82599_KX4), 0},
	{PCI_VDEVICE(INTEL, IXGBE_DEV_ID_82599_XAUI_LOM), 0},
	{PCI_VDEVICE(INTEL, IXGBE_DEV_ID_82599_KR), 0},
	{PCI_VDEVICE(INTEL, IXGBE_DEV_ID_82599_SFP), 0},
	{PCI_VDEVICE(INTEL, IXGBE_DEV_ID_82599_SFP_EM), 0},
	{PCI_VDEVICE(INTEL, IXGBE_DEV_ID_82599_KX4_MEZZ), 0},
	{PCI_VDEVICE(INTEL, IXGBE_DEV_ID_82599_CX4), 0},
	{PCI_VDEVICE(INTEL, IXGBE_DEV_ID_82599_BACKPLANE_FCOE), 0},
	{PCI_VDEVICE(INTEL, IXGBE_DEV_ID_82599_SFP_FCOE), 0},
	{PCI_VDEVICE(INTEL, IXGBE_DEV_ID_82599_T3_LOM), 0},
	{PCI_VDEVICE(INTEL, IXGBE_DEV_ID_82599_COMBO_BACKPLANE), 0},
	{PCI_VDEVICE(INTEL, IXGBE_DEV_ID_X540T), 0},
	{PCI_VDEVICE(INTEL, IXGBE_DEV_ID_82599_SFP_SF2), 0},
	{PCI_VDEVICE(INTEL, IXGBE_DEV_ID_82599EN_SFP), 0},
	{PCI_VDEVICE(INTEL, IXGBE_DEV_ID_82599_SFP_SF_QP), 0},
	{PCI_VDEVICE(INTEL, IXGBE_DEV_ID_82599_QSFP_SF_QP), 0},
	{PCI_VDEVICE(INTEL, IXGBE_DEV_ID_X540T1), 0},
	{PCI_VDEVICE(INTEL, IXGBE_DEV_ID_X550T), 0},
	{PCI_VDEVICE(INTEL, IXGBE_DEV_ID_X550T1), 0},
	{PCI_VDEVICE(INTEL, IXGBE_DEV_ID_X550EM_X_KX4), 0},
	{PCI_VDEVICE(INTEL, IXGBE_DEV_ID_X550EM_X_XFI), 0},
	{PCI_VDEVICE(INTEL, IXGBE_DEV_ID_X550EM_X_KR), 0},
	{PCI_VDEVICE(INTEL, IXGBE_DEV_ID_X550EM_X_SFP), 0},
	{PCI_VDEVICE(INTEL, IXGBE_DEV_ID_X550EM_X_10G_T), 0},
	{PCI_VDEVICE(INTEL, IXGBE_DEV_ID_X550EM_X_1G_T), 0},
	{PCI_VDEVICE(INTEL, IXGBE_DEV_ID_X550EM_A_KR), 0},
	{PCI_VDEVICE(INTEL, IXGBE_DEV_ID_X550EM_A_KR_L), 0},
	{PCI_VDEVICE(INTEL, IXGBE_DEV_ID_X550EM_A_SFP_N), 0},
	{PCI_VDEVICE(INTEL, IXGBE_DEV_ID_X550EM_A_SGMII), 0},
	{PCI_VDEVICE(INTEL, IXGBE_DEV_ID_X550EM_A_SGMII_L), 0},
	{PCI_VDEVICE(INTEL, IXGBE_DEV_ID_X550EM_A_10G_T), 0},
	{PCI_VDEVICE(INTEL, IXGBE_DEV_ID_X550EM_A_QSFP), 0},
	{PCI_VDEVICE(INTEL, IXGBE_DEV_ID_X550EM_A_QSFP_N), 0},
	{PCI_VDEVICE(INTEL, IXGBE_DEV_ID_X550EM_A_SFP), 0},
	{PCI_VDEVICE(INTEL, IXGBE_DEV_ID_X550EM_A_1G_T), 0},
	{PCI_VDEVICE(INTEL, IXGBE_DEV_ID_X550EM_A_1G_T_L), 0},
	/* required last entry */
	{ .device = 0 }
};
MODULE_DEVICE_TABLE(pci, ixgbe_pci_tbl);

#if IS_ENABLED(CONFIG_DCA)
static int ixgbe_notify_dca(struct notifier_block *, unsigned long event,
			    void *p);
static struct notifier_block dca_notifier = {
	.notifier_call	= ixgbe_notify_dca,
	.next		= NULL,
	.priority	= 0
};
#endif /* CONFIG_DCA */
static void ixgbe_watchdog_link_is_down(struct ixgbe_adapter *);

MODULE_AUTHOR("Intel Corporation, <linux.nics@intel.com>");
MODULE_DESCRIPTION(DRV_SUMMARY);
MODULE_LICENSE("GPL");
MODULE_VERSION(DRV_VERSION);

#define DEFAULT_DEBUG_LEVEL_SHIFT 3

static struct workqueue_struct *ixgbe_wq;

static bool ixgbe_is_sfp(struct ixgbe_hw *hw);
static bool ixgbe_check_cfg_remove(struct ixgbe_hw *hw, struct pci_dev *pdev);

static int ixgbe_read_pci_cfg_word_parent(struct ixgbe_hw *hw,
					  u32 reg, u16 *value)
{
	struct ixgbe_adapter *adapter = hw->back;
	struct pci_dev *parent_dev;
	struct pci_bus *parent_bus;
	int pos;

	parent_bus = adapter->pdev->bus->parent;
	if (!parent_bus)
		return IXGBE_ERR_FEATURE_NOT_SUPPORTED;

	parent_dev = parent_bus->self;
	if (!parent_dev)
		return IXGBE_ERR_FEATURE_NOT_SUPPORTED;

	pos = pci_find_capability(parent_dev, PCI_CAP_ID_EXP);
	if (!pos)
		return IXGBE_ERR_FEATURE_NOT_SUPPORTED;

	pci_read_config_word(parent_dev, pos + reg, value);
	if (*value == IXGBE_FAILED_READ_CFG_WORD &&
	    ixgbe_check_cfg_remove(hw, parent_dev))
		return IXGBE_ERR_FEATURE_NOT_SUPPORTED;
	return IXGBE_SUCCESS;
}

/**
 * ixgbe_get_parent_bus_info - Set PCI bus info beyond switch
 * @hw: pointer to hardware structure
 *
 * Sets the PCI bus info (speed, width, type) within the ixgbe_hw structure
 * when the device is behind a switch.
 **/
static s32 ixgbe_get_parent_bus_info(struct ixgbe_hw *hw)
{
	u16 link_status = 0;
	int err;

	hw->bus.type = ixgbe_bus_type_pci_express;

	/* Get the negotiated link width and speed from PCI config space of the
	 * parent, as this device is behind a switch
	 */
	err = ixgbe_read_pci_cfg_word_parent(hw, 18, &link_status);

	/* If the read fails, fallback to default */
	if (err)
		link_status = IXGBE_READ_PCIE_WORD(hw, IXGBE_PCI_LINK_STATUS);

	ixgbe_set_pci_config_data_generic(hw, link_status);

	return IXGBE_SUCCESS;
}

/**
 * ixgbe_check_from_parent - determine whether to use parent for PCIe info
 * @hw: hw specific details
 *
 * This function is used by probe to determine whether a device's PCIe info
 * (speed, width, etc) should be obtained from the parent bus or directly. This
 * is useful for specialized device configurations containing PCIe bridges.
 */
static inline bool ixgbe_pcie_from_parent(struct ixgbe_hw *hw)
{
	switch (hw->device_id) {
	case IXGBE_DEV_ID_82599_QSFP_SF_QP:
	case IXGBE_DEV_ID_82599_SFP_SF_QP:
		return true;
	default:
		return false;
	}
}

static void ixgbe_check_minimum_link(struct ixgbe_adapter *adapter)
{
	struct ixgbe_hw *hw = &adapter->hw;
	struct pci_dev *pdev;

	/* Some devices are not connected over PCIe and thus do not negotiate
	 * speed. These devices do not have valid bus info, and thus any report
	 * we generate may not be correct.
	 */
	if (hw->bus.type == ixgbe_bus_type_internal)
		return;

	/* determine whether to use the parent device */
	if (ixgbe_pcie_from_parent(&adapter->hw))
		pdev = adapter->pdev->bus->parent->self;
	else
		pdev = adapter->pdev;

	pcie_print_link_status(pdev);
}

/**
 * ixgbe_enumerate_functions - Get the number of ports this device has
 * @adapter: adapter structure
 *
 * This function enumerates the phsyical functions co-located on a single slot,
 * in order to determine how many ports a device has. This is most useful in
 * determining the required GT/s of PCIe bandwidth necessary for optimal
 * performance.
 **/
static inline int ixgbe_enumerate_functions(struct ixgbe_adapter *adapter)
{
	struct pci_dev *entry, *pdev = adapter->pdev;
	int physfns = 0;

	/* Some cards can not use the generic count PCIe functions method,
	 * because they are behind a parent switch, so we hardcode these to
	 * correct number of ports.
	 */
	if (ixgbe_pcie_from_parent(&adapter->hw)) {
		physfns = 4;
	} else {
		list_for_each_entry(entry, &pdev->bus->devices, bus_list) {
#ifdef CONFIG_PCI_IOV
			/* don't count virtual functions */
			if (entry->is_virtfn)
				continue;
#endif

			/* When the devices on the bus don't all match our device ID,
			 * we can't reliably determine the correct number of
			 * functions. This can occur if a function has been direct 
			 * attached to a virtual machine using VT-d, for example. In
			 * this case, simply return -1 to indicate this.
			 */
			if ((entry->vendor != pdev->vendor) ||
			    (entry->device != pdev->device))
				return -1;

			physfns++;
		}
	}

	return physfns;
}

static void ixgbe_service_event_schedule(struct ixgbe_adapter *adapter)
{
	if (!test_bit(__IXGBE_DOWN, &adapter->state) &&
	    !test_bit(__IXGBE_REMOVING, &adapter->state) &&
	    !test_and_set_bit(__IXGBE_SERVICE_SCHED, &adapter->state))
		queue_work(ixgbe_wq, &adapter->service_task);
}

static void ixgbe_service_event_complete(struct ixgbe_adapter *adapter)
{
	BUG_ON(!test_bit(__IXGBE_SERVICE_SCHED, &adapter->state));

	/* flush memory to make sure state is correct before next watchog */
	smp_mb__before_atomic();
	clear_bit(__IXGBE_SERVICE_SCHED, &adapter->state);
}

static void ixgbe_remove_adapter(struct ixgbe_hw *hw)
{
	struct ixgbe_adapter *adapter = hw->back;

	if (!hw->hw_addr)
		return;
	hw->hw_addr = NULL;
	e_dev_err("Adapter removed\n");
	if (test_bit(__IXGBE_SERVICE_INITED, &adapter->state))
		ixgbe_service_event_schedule(adapter);
}

static u32
ixgbe_check_remove(struct ixgbe_hw *hw, u32 reg)
{
	u8 __iomem *reg_addr;
	u32 value;
	int i;

	reg_addr = READ_ONCE(hw->hw_addr);
	if (IXGBE_REMOVED(reg_addr))
		return IXGBE_FAILED_READ_REG;

	/* Register read of 0xFFFFFFFF can indicate the adapter has been
	 * removed, so perform several status register reads to determine if
	 * the adapter has been removed.
	 */
	for (i = 0; i < IXGBE_FAILED_READ_RETRIES; ++i) {
		value = readl(reg_addr + IXGBE_STATUS);
		if (value != IXGBE_FAILED_READ_REG)
			break;
		mdelay(3);
	}

	if (value == IXGBE_FAILED_READ_REG)
		ixgbe_remove_adapter(hw);
	else
		value = readl(reg_addr + reg);
	return value;
}

static u32
ixgbe_validate_register_read(struct ixgbe_hw *_hw, u32 reg, bool quiet)
{
	int i;
	u32 value;
	u8 __iomem *reg_addr;
	struct ixgbe_adapter *adapter = _hw->back;

	reg_addr = READ_ONCE(_hw->hw_addr);
	if (IXGBE_REMOVED(reg_addr))
		return IXGBE_FAILED_READ_REG;
	for (i = 0; i < IXGBE_DEAD_READ_RETRIES; ++i) {
		value = readl(reg_addr + reg);
		if (value != IXGBE_DEAD_READ_REG)
			break;
	}
	if (quiet)
		return value;
	if (value == IXGBE_DEAD_READ_REG)
		e_err(drv, "%s: register %x read unchanged\n", __func__, reg);
	else
		e_warn(hw, "%s: register %x read recovered after %d retries\n",
			__func__, reg, i + 1);
	return value;
}

u32 ixgbe_read_reg(struct ixgbe_hw *hw, u32 reg, bool quiet)
{
	u32 value;
	u8 __iomem *reg_addr;

	reg_addr = READ_ONCE(hw->hw_addr);
	if (IXGBE_REMOVED(reg_addr))
		return IXGBE_FAILED_READ_REG;
	if (unlikely(hw->phy.nw_mng_if_sel &
		     IXGBE_NW_MNG_IF_SEL_SGMII_ENABLE)) {
		struct ixgbe_adapter *adapter;
		int i;

		for (i = 0; i < 200; ++i) {
			value = readl(reg_addr + IXGBE_MAC_SGMII_BUSY);
			if (likely(!value))
				goto writes_completed;
			if (value == IXGBE_FAILED_READ_REG) {
				ixgbe_remove_adapter(hw);
				return IXGBE_FAILED_READ_REG;
			}
			udelay(5);
		}

		adapter = hw->back;
		e_warn(hw, "register writes incomplete %08x\n", value);
	}

writes_completed:
	value = readl(reg_addr + reg);
	if (unlikely(value == IXGBE_FAILED_READ_REG))
		value = ixgbe_check_remove(hw, reg);
	if (unlikely(value == IXGBE_DEAD_READ_REG))
		value = ixgbe_validate_register_read(hw, reg, quiet);
	return value;
}

static void ixgbe_release_hw_control(struct ixgbe_adapter *adapter)
{
	u32 ctrl_ext;

	/* Let firmware take over control of h/w */
	ctrl_ext = IXGBE_READ_REG(&adapter->hw, IXGBE_CTRL_EXT);
	IXGBE_WRITE_REG(&adapter->hw, IXGBE_CTRL_EXT,
			ctrl_ext & ~IXGBE_CTRL_EXT_DRV_LOAD);
}

static void ixgbe_get_hw_control(struct ixgbe_adapter *adapter)
{
	u32 ctrl_ext;

	/* Let firmware know the driver has taken over */
	ctrl_ext = IXGBE_READ_REG(&adapter->hw, IXGBE_CTRL_EXT);
	IXGBE_WRITE_REG(&adapter->hw, IXGBE_CTRL_EXT,
			ctrl_ext | IXGBE_CTRL_EXT_DRV_LOAD);
}

/*
 * ixgbe_set_ivar - set the IVAR registers, mapping interrupt causes to vectors
 * @adapter: pointer to adapter struct
 * @direction: 0 for Rx, 1 for Tx, -1 for other causes
 * @queue: queue to map the corresponding interrupt to
 * @msix_vector: the vector to map to the corresponding queue
 *
 */
static void ixgbe_set_ivar(struct ixgbe_adapter *adapter, s8 direction,
			   u8 queue, u8 msix_vector)
{
	u32 ivar, index;
	struct ixgbe_hw *hw = &adapter->hw;
	switch (hw->mac.type) {
	case ixgbe_mac_82598EB:
		msix_vector |= IXGBE_IVAR_ALLOC_VAL;
		if (direction == -1)
			direction = 0;
		index = (((direction * 64) + queue) >> 2) & 0x1F;
		ivar = IXGBE_READ_REG(hw, IXGBE_IVAR(index));
		ivar &= ~(0xFF << (8 * (queue & 0x3)));
		ivar |= (msix_vector << (8 * (queue & 0x3)));
		IXGBE_WRITE_REG(hw, IXGBE_IVAR(index), ivar);
		break;
	case ixgbe_mac_82599EB:
	case ixgbe_mac_X540:
	case ixgbe_mac_X550:
	case ixgbe_mac_X550EM_x:
	case ixgbe_mac_X550EM_a:
		if (direction == -1) {
			/* other causes */
			msix_vector |= IXGBE_IVAR_ALLOC_VAL;
			index = ((queue & 1) * 8);
			ivar = IXGBE_READ_REG(&adapter->hw, IXGBE_IVAR_MISC);
			ivar &= ~(0xFF << index);
			ivar |= (msix_vector << index);
			IXGBE_WRITE_REG(&adapter->hw, IXGBE_IVAR_MISC, ivar);
			break;
		} else {
			/* tx or rx causes */
			msix_vector |= IXGBE_IVAR_ALLOC_VAL;
			index = ((16 * (queue & 1)) + (8 * direction));
			ivar = IXGBE_READ_REG(hw, IXGBE_IVAR(queue >> 1));
			ivar &= ~(0xFF << index);
			ivar |= (msix_vector << index);
			IXGBE_WRITE_REG(hw, IXGBE_IVAR(queue >> 1), ivar);
			break;
		}
	default:
		break;
	}
}

void ixgbe_irq_rearm_queues(struct ixgbe_adapter *adapter,
			    u64 qmask)
{
	u32 mask;

	switch (adapter->hw.mac.type) {
	case ixgbe_mac_82598EB:
		mask = (IXGBE_EIMS_RTX_QUEUE & qmask);
		IXGBE_WRITE_REG(&adapter->hw, IXGBE_EICS, mask);
		break;
	case ixgbe_mac_82599EB:
	case ixgbe_mac_X540:
	case ixgbe_mac_X550:
	case ixgbe_mac_X550EM_x:
	case ixgbe_mac_X550EM_a:
		mask = (qmask & 0xFFFFFFFF);
		IXGBE_WRITE_REG(&adapter->hw, IXGBE_EICS_EX(0), mask);
		mask = (qmask >> 32);
		IXGBE_WRITE_REG(&adapter->hw, IXGBE_EICS_EX(1), mask);
		break;
	default:
		break;
	}
}

static void ixgbe_update_xoff_rx_lfc(struct ixgbe_adapter *adapter)
{
	struct ixgbe_hw *hw = &adapter->hw;
	struct ixgbe_hw_stats *hwstats = &adapter->stats;
	int i;
	u32 data;

	if ((hw->fc.current_mode != ixgbe_fc_full) &&
	    (hw->fc.current_mode != ixgbe_fc_rx_pause))
		return;

	switch (hw->mac.type) {
	case ixgbe_mac_82598EB:
		data = IXGBE_READ_REG(hw, IXGBE_LXOFFRXC);
		break;
	default:
		data = IXGBE_READ_REG(hw, IXGBE_LXOFFRXCNT);
	}
	hwstats->lxoffrxc += data;

	/* refill credits (no tx hang) if we received xoff */
	if (!data)
		return;

	for (i = 0; i < adapter->num_tx_queues; i++)
		clear_bit(__IXGBE_HANG_CHECK_ARMED,
			  &adapter->tx_ring[i]->state);
	for (i = 0; i < adapter->num_xdp_queues; i++)
		clear_bit(__IXGBE_HANG_CHECK_ARMED,
			  &adapter->xdp_ring[i]->state);
}

static void ixgbe_update_xoff_received(struct ixgbe_adapter *adapter)
{
	struct ixgbe_hw *hw = &adapter->hw;
	struct ixgbe_hw_stats *hwstats = &adapter->stats;
	u32 xoff[8] = {0};
	u8 tc;
	int i;
	bool pfc_en = adapter->dcb_cfg.pfc_mode_enable;

#ifdef HAVE_DCBNL_IEEE
	if (adapter->ixgbe_ieee_pfc)
		pfc_en |= !!(adapter->ixgbe_ieee_pfc->pfc_en);

#endif
	if (!(adapter->flags & IXGBE_FLAG_DCB_ENABLED) || !pfc_en) {
		ixgbe_update_xoff_rx_lfc(adapter);
		return;
	}

	/* update stats for each tc, only valid with PFC enabled */
	for (i = 0; i < MAX_TX_PACKET_BUFFERS; i++) {
		u32 pxoffrxc;

		switch (hw->mac.type) {
		case ixgbe_mac_82598EB:
			pxoffrxc = IXGBE_READ_REG(hw, IXGBE_PXOFFRXC(i));
			break;
		default:
			pxoffrxc = IXGBE_READ_REG(hw, IXGBE_PXOFFRXCNT(i));
		}
		hwstats->pxoffrxc[i] += pxoffrxc;
		/* Get the TC for given UP */
		tc = netdev_get_prio_tc_map(adapter->netdev, i);
		xoff[tc] += pxoffrxc;
	}

	/* disarm tx queues that have received xoff frames */
	for (i = 0; i < adapter->num_tx_queues; i++) {
		struct ixgbe_ring *tx_ring = adapter->tx_ring[i];

		tc = tx_ring->dcb_tc;
		if ((tc <= 7) && (xoff[tc]))
			clear_bit(__IXGBE_HANG_CHECK_ARMED, &tx_ring->state);
	}
	for (i = 0; i < adapter->num_xdp_queues; i++) {
		struct ixgbe_ring *xdp_ring = adapter->xdp_ring[i];

		tc = xdp_ring->dcb_tc;
		if (tc <= 7 && xoff[tc])
			clear_bit(__IXGBE_HANG_CHECK_ARMED, &xdp_ring->state);
		}
}

static u64 ixgbe_get_tx_completed(struct ixgbe_ring *ring)
{
	return ring->stats.packets;
}

static u64 ixgbe_get_tx_pending(struct ixgbe_ring *ring)
{
	unsigned int head, tail;

	head = ring->next_to_clean;
	tail = ring->next_to_use;

	return ((head <= tail) ? tail : tail + ring->count) - head;
}

static bool ixgbe_check_tx_hang(struct ixgbe_ring *tx_ring)
{
	u32 tx_done = ixgbe_get_tx_completed(tx_ring);
	u32 tx_done_old = tx_ring->tx_stats.tx_done_old;
	u32 tx_pending = ixgbe_get_tx_pending(tx_ring);
	bool ret = false;

	clear_check_for_tx_hang(tx_ring);

	/*
	 * Check for a hung queue, but be thorough. This verifies
	 * that a transmit has been completed since the previous
	 * check AND there is at least one packet pending. The
	 * ARMED bit is set to indicate a potential hang. The
	 * bit is cleared if a pause frame is received to remove
	 * false hang detection due to PFC or 802.3x frames. By
	 * requiring this to fail twice we avoid races with
	 * PFC clearing the ARMED bit and conditions where we
	 * run the check_tx_hang logic with a transmit completion
	 * pending but without time to complete it yet.
	 */
	if ((tx_done_old == tx_done) && tx_pending) {
		/* make sure it is true for two checks in a row */
		ret = test_and_set_bit(__IXGBE_HANG_CHECK_ARMED,
				       &tx_ring->state);
	} else {
		/* update completed stats and continue */
		tx_ring->tx_stats.tx_done_old = tx_done;
		/* reset the countdown */
		clear_bit(__IXGBE_HANG_CHECK_ARMED, &tx_ring->state);
	}

	return ret;
}

/**
 * ixgbe_tx_timeout_reset - initiate reset due to Tx timeout
 * @adapter: driver private struct
 **/
static void ixgbe_tx_timeout_reset(struct ixgbe_adapter *adapter)
{

	/* Do the reset outside of interrupt context */
	if (!test_bit(__IXGBE_DOWN, &adapter->state)) {
		set_bit(__IXGBE_RESET_REQUESTED, &adapter->state);
		ixgbe_service_event_schedule(adapter);
	}
}

/**
 * ixgbe_tx_timeout - Respond to a Tx Hang
 * @netdev: network interface device structure
 * @txqueue: specific tx queue
 **/
#ifdef HAVE_TX_TIMEOUT_TXQUEUE
static void ixgbe_tx_timeout(struct net_device *netdev, unsigned int txqueue)
#else
static void ixgbe_tx_timeout(struct net_device *netdev)
#endif
{
	struct ixgbe_adapter *adapter = netdev_priv(netdev);
	bool real_tx_hang = false;
	int i;

#define TX_TIMEO_LIMIT 16000
	for (i = 0; i < adapter->num_tx_queues; i++) {
		struct ixgbe_ring *tx_ring = adapter->tx_ring[i];
		if (check_for_tx_hang(tx_ring) && ixgbe_check_tx_hang(tx_ring))
			real_tx_hang = true;
	}

	if (real_tx_hang) {
		ixgbe_tx_timeout_reset(adapter);
	} else {
		e_info(drv, "Fake Tx hang detected with timeout of %d "
			"seconds\n", netdev->watchdog_timeo/HZ);

		/* fake Tx hang - increase the kernel timeout */
		if (netdev->watchdog_timeo < TX_TIMEO_LIMIT)
			netdev->watchdog_timeo *= 2;
	}
}

/**
 * ixgbe_clean_tx_irq - Reclaim resources after transmit completes
 * @q_vector: structure containing interrupt and ring information
 * @tx_ring: tx ring to clean
 * @napi_budget: Used to determine if we are in netpoll
 **/
static bool ixgbe_clean_tx_irq(struct ixgbe_q_vector *q_vector,
			       struct ixgbe_ring *tx_ring, int napi_budget)
{
	struct ixgbe_adapter *adapter = q_vector->adapter;
	struct ixgbe_tx_buffer *tx_buffer;
	union ixgbe_adv_tx_desc *tx_desc;
	unsigned int total_bytes = 0, total_packets = 0;
	unsigned int budget = q_vector->tx.work_limit;
	unsigned int i = tx_ring->next_to_clean;

	if (test_bit(__IXGBE_DOWN, &adapter->state))
		return true;

	tx_buffer = &tx_ring->tx_buffer_info[i];
	tx_desc = IXGBE_TX_DESC(tx_ring, i);
	i -= tx_ring->count;

	do {
		union ixgbe_adv_tx_desc *eop_desc = tx_buffer->next_to_watch;

		/* if next_to_watch is not set then there is no work pending */
		if (!eop_desc)
			break;

		/* prevent any other reads prior to eop_desc */
		smp_rmb();

		/* if DD is not set pending work has not been completed */
		if (!(eop_desc->wb.status & cpu_to_le32(IXGBE_TXD_STAT_DD)))
			break;

		/* clear next_to_watch to prevent false hangs */
		tx_buffer->next_to_watch = NULL;

		/* update the statistics for this packet */
		total_bytes += tx_buffer->bytecount;
		total_packets += tx_buffer->gso_segs;

		/* free the skb */
#ifdef HAVE_XDP_SUPPORT
		if (ring_is_xdp(tx_ring))
#ifdef HAVE_XDP_FRAME_STRUCT
			xdp_return_frame(tx_buffer->xdpf);
#else
			page_frag_free(tx_buffer->data);
#endif
		else
			napi_consume_skb(tx_buffer->skb, napi_budget);
#else
		napi_consume_skb(tx_buffer->skb, napi_budget);
#endif

		/* unmap skb header data */
		dma_unmap_single(tx_ring->dev,
				 dma_unmap_addr(tx_buffer, dma),
				 dma_unmap_len(tx_buffer, len),
				 DMA_TO_DEVICE);

		/* clear tx_buffer data */
		dma_unmap_len_set(tx_buffer, len, 0);

		/* unmap remaining buffers */
		while (tx_desc != eop_desc) {
			tx_buffer++;
			tx_desc++;
			i++;
			if (unlikely(!i)) {
				i -= tx_ring->count;
				tx_buffer = tx_ring->tx_buffer_info;
				tx_desc = IXGBE_TX_DESC(tx_ring, 0);
			}

			/* unmap any remaining paged data */
			if (dma_unmap_len(tx_buffer, len)) {
				dma_unmap_page(tx_ring->dev,
					       dma_unmap_addr(tx_buffer, dma),
					       dma_unmap_len(tx_buffer, len),
					       DMA_TO_DEVICE);
				dma_unmap_len_set(tx_buffer, len, 0);
			}
		}

		/* move us one more past the eop_desc for start of next pkt */
		tx_buffer++;
		tx_desc++;
		i++;
		if (unlikely(!i)) {
			i -= tx_ring->count;
			tx_buffer = tx_ring->tx_buffer_info;
			tx_desc = IXGBE_TX_DESC(tx_ring, 0);
		}

		/* issue prefetch for next Tx descriptor */
		prefetch(tx_desc);

		/* update budget accounting */
		budget--;
	} while (likely(budget));

	i += tx_ring->count;
	tx_ring->next_to_clean = i;
	u64_stats_update_begin(&tx_ring->syncp);
	tx_ring->stats.bytes += total_bytes;
	tx_ring->stats.packets += total_packets;
	u64_stats_update_end(&tx_ring->syncp);
	q_vector->tx.total_bytes += total_bytes;
	q_vector->tx.total_packets += total_packets;

	if (check_for_tx_hang(tx_ring) && ixgbe_check_tx_hang(tx_ring)) {
		/* schedule immediate reset if we believe we hung */
		struct ixgbe_hw *hw = &adapter->hw;

		e_err(drv, "Detected Tx Unit Hang%s\n"
			"  Tx Queue             <%d>\n"
			"  TDH, TDT             <%x>, <%x>\n"
			"  next_to_use          <%x>\n"
			"  next_to_clean        <%x>\n"
			"tx_buffer_info[next_to_clean]\n"
			"  time_stamp           <%lx>\n"
			"  jiffies              <%lx>\n",
			ring_is_xdp(tx_ring) ? " (XDP)" : "",
			tx_ring->queue_index,
			IXGBE_READ_REG(hw, IXGBE_TDH(tx_ring->reg_idx)),
			IXGBE_READ_REG(hw, IXGBE_TDT(tx_ring->reg_idx)),
			tx_ring->next_to_use, i,
			tx_ring->tx_buffer_info[i].time_stamp, jiffies);

		if (!ring_is_xdp(tx_ring))
			netif_stop_subqueue(tx_ring->netdev,
					    tx_ring->queue_index);

		e_info(probe,
		       "tx hang %d detected on queue %d, resetting adapter\n",
		       adapter->tx_timeout_count + 1, tx_ring->queue_index);

		ixgbe_tx_timeout_reset(adapter);

		/* the adapter is about to reset, no point in enabling stuff */
		return true;
	}

	if (ring_is_xdp(tx_ring))
		return !!budget;
	netdev_tx_completed_queue(txring_txq(tx_ring),
				  total_packets, total_bytes);

#define TX_WAKE_THRESHOLD (DESC_NEEDED * 2)
	if (unlikely(total_packets && netif_carrier_ok(netdev_ring(tx_ring)) &&
		     (ixgbe_desc_unused(tx_ring) >= TX_WAKE_THRESHOLD))) {
		/* Make sure that anybody stopping the queue after this
		 * sees the new next_to_clean.
		 */
		smp_mb();
#ifdef HAVE_TX_MQ
		if (__netif_subqueue_stopped(netdev_ring(tx_ring),
					     ring_queue_index(tx_ring))
		    && !test_bit(__IXGBE_DOWN, &q_vector->adapter->state)) {
			netif_wake_subqueue(netdev_ring(tx_ring),
					    ring_queue_index(tx_ring));
			++tx_ring->tx_stats.restart_queue;
		}
#else
		if (netif_queue_stopped(netdev_ring(tx_ring)) &&
		    !test_bit(__IXGBE_DOWN, &q_vector->adapter->state)) {
			netif_wake_queue(netdev_ring(tx_ring));
			++tx_ring->tx_stats.restart_queue;
		}
#endif
	}

	return !!budget;
}

#if IS_ENABLED(CONFIG_DCA)
static void ixgbe_update_tx_dca(struct ixgbe_adapter *adapter,
				struct ixgbe_ring *tx_ring,
				int cpu)
{
	struct ixgbe_hw *hw = &adapter->hw;
	u32 txctrl = 0;
	u16 reg_offset;

	if (adapter->flags & IXGBE_FLAG_DCA_ENABLED)
		txctrl = dca3_get_tag(tx_ring->dev, cpu);

	switch (hw->mac.type) {
	case ixgbe_mac_82598EB:
		reg_offset = IXGBE_DCA_TXCTRL(tx_ring->reg_idx);
		break;
	case ixgbe_mac_82599EB:
	case ixgbe_mac_X540:
		reg_offset = IXGBE_DCA_TXCTRL_82599(tx_ring->reg_idx);
		txctrl <<= IXGBE_DCA_TXCTRL_CPUID_SHIFT_82599;
		break;
	default:
		/* for unknown hardware do not write register */
		return;
	}

	/*
	 * We can enable relaxed ordering for reads, but not writes when
	 * DCA is enabled.  This is due to a known issue in some chipsets
	 * which will cause the DCA tag to be cleared.
	 */
	txctrl |= IXGBE_DCA_TXCTRL_DESC_RRO_EN |
		  IXGBE_DCA_TXCTRL_DATA_RRO_EN |
		  IXGBE_DCA_TXCTRL_DESC_DCA_EN;

	IXGBE_WRITE_REG(hw, reg_offset, txctrl);
}

static void ixgbe_update_rx_dca(struct ixgbe_adapter *adapter,
				struct ixgbe_ring *rx_ring,
				int cpu)
{
	struct ixgbe_hw *hw = &adapter->hw;
	u32 rxctrl = 0;
	u8 reg_idx = rx_ring->reg_idx;

	if (adapter->flags & IXGBE_FLAG_DCA_ENABLED)
		rxctrl = dca3_get_tag(rx_ring->dev, cpu);

	switch (hw->mac.type) {
	case ixgbe_mac_82599EB:
	case ixgbe_mac_X540:
		rxctrl <<= IXGBE_DCA_RXCTRL_CPUID_SHIFT_82599;
		break;
	default:
		break;
	}

	/*
	 * We can enable relaxed ordering for reads, but not writes when
	 * DCA is enabled.  This is due to a known issue in some chipsets
	 * which will cause the DCA tag to be cleared.
	 */
	rxctrl |= IXGBE_DCA_RXCTRL_DESC_RRO_EN |
		  IXGBE_DCA_RXCTRL_DATA_DCA_EN |
		  IXGBE_DCA_RXCTRL_DESC_DCA_EN;

	IXGBE_WRITE_REG(hw, IXGBE_DCA_RXCTRL(reg_idx), rxctrl);
}

static void ixgbe_update_dca(struct ixgbe_q_vector *q_vector)
{
	struct ixgbe_adapter *adapter = q_vector->adapter;
	struct ixgbe_ring *ring;
	int cpu = get_cpu();

	if (q_vector->cpu == cpu)
		goto out_no_update;

	ixgbe_for_each_ring(ring, q_vector->tx)
		ixgbe_update_tx_dca(adapter, ring, cpu);

	ixgbe_for_each_ring(ring, q_vector->rx)
		ixgbe_update_rx_dca(adapter, ring, cpu);

	q_vector->cpu = cpu;
out_no_update:
	put_cpu();
}

static void ixgbe_setup_dca(struct ixgbe_adapter *adapter)
{
	int v_idx;

	/* always use CB2 mode, difference is masked in the CB driver */
	if (adapter->flags & IXGBE_FLAG_DCA_ENABLED)
		IXGBE_WRITE_REG(&adapter->hw, IXGBE_DCA_CTRL,
				IXGBE_DCA_CTRL_DCA_MODE_CB2);
	else
		IXGBE_WRITE_REG(&adapter->hw, IXGBE_DCA_CTRL,
				IXGBE_DCA_CTRL_DCA_DISABLE);

	for (v_idx = 0; v_idx < adapter->num_q_vectors; v_idx++) {
		adapter->q_vector[v_idx]->cpu = -1;
		ixgbe_update_dca(adapter->q_vector[v_idx]);
	}
}

static int __ixgbe_notify_dca(struct device *dev, void *data)
{
	struct ixgbe_adapter *adapter = dev_get_drvdata(dev);
	unsigned long event = *(unsigned long *)data;

	if (!(adapter->flags & IXGBE_FLAG_DCA_CAPABLE))
		return 0;

	switch (event) {
	case DCA_PROVIDER_ADD:
		/* if we're already enabled, don't do it again */
		if (adapter->flags & IXGBE_FLAG_DCA_ENABLED)
			break;
		if (dca_add_requester(dev) == IXGBE_SUCCESS) {
			adapter->flags |= IXGBE_FLAG_DCA_ENABLED;
			IXGBE_WRITE_REG(&adapter->hw, IXGBE_DCA_CTRL,
					IXGBE_DCA_CTRL_DCA_MODE_CB2);
			break;
		}
		/* fall through - DCA is disabled */
	case DCA_PROVIDER_REMOVE:
		if (adapter->flags & IXGBE_FLAG_DCA_ENABLED) {
			dca_remove_requester(dev);
			adapter->flags &= ~IXGBE_FLAG_DCA_ENABLED;
			IXGBE_WRITE_REG(&adapter->hw, IXGBE_DCA_CTRL,
					IXGBE_DCA_CTRL_DCA_DISABLE);
		}
		break;
	}

	return IXGBE_SUCCESS;
}
#endif /* CONFIG_DCA */

#ifdef NETIF_F_RXHASH
#define IXGBE_RSS_L4_TYPES_MASK \
	((1ul << IXGBE_RXDADV_RSSTYPE_IPV4_TCP) | \
	 (1ul << IXGBE_RXDADV_RSSTYPE_IPV4_UDP) | \
	 (1ul << IXGBE_RXDADV_RSSTYPE_IPV6_TCP) | \
	 (1ul << IXGBE_RXDADV_RSSTYPE_IPV6_UDP) | \
	 (1ul << IXGBE_RXDADV_RSSTYPE_IPV6_TCP_EX) | \
	 (1ul << IXGBE_RXDADV_RSSTYPE_IPV6_UDP_EX))

static inline void ixgbe_rx_hash(struct ixgbe_ring *ring,
				 union ixgbe_adv_rx_desc *rx_desc,
				 struct sk_buff *skb)
{
	u16 rss_type; 

	if (!(netdev_ring(ring)->features & NETIF_F_RXHASH))
		return;

	rss_type = le16_to_cpu(rx_desc->wb.lower.lo_dword.hs_rss.pkt_info) &
		   IXGBE_RXDADV_RSSTYPE_MASK;

	if (!rss_type)
		return;

	skb_set_hash(skb, le32_to_cpu(rx_desc->wb.lower.hi_dword.rss),
		     (IXGBE_RSS_L4_TYPES_MASK & (1ul << rss_type)) ?
		     PKT_HASH_TYPE_L4 : PKT_HASH_TYPE_L3);
}
#endif /* NETIF_F_RXHASH */

#if IS_ENABLED(CONFIG_FCOE)
/**
 * ixgbe_rx_is_fcoe - check the rx desc for incoming pkt type
 * @ring: structure containing ring specific data
 * @rx_desc: advanced rx descriptor
 *
 * Returns : true if it is FCoE pkt
 */
static inline bool ixgbe_rx_is_fcoe(struct ixgbe_ring *ring,
				    union ixgbe_adv_rx_desc *rx_desc)
{
	__le16 pkt_info = rx_desc->wb.lower.lo_dword.hs_rss.pkt_info;

	return test_bit(__IXGBE_RX_FCOE, &ring->state) &&
	       ((pkt_info & cpu_to_le16(IXGBE_RXDADV_PKTTYPE_ETQF_MASK)) ==
		(cpu_to_le16(IXGBE_ETQF_FILTER_FCOE <<
			     IXGBE_RXDADV_PKTTYPE_ETQF_SHIFT)));
}
#endif /* CONFIG_FCOE */

/**
 * ixgbe_rx_checksum - indicate in skb if hw indicated a good cksum
 * @ring: structure containing ring specific data
 * @rx_desc: current Rx descriptor being processed
 * @skb: skb currently being received and modified
 **/
static inline void ixgbe_rx_checksum(struct ixgbe_ring *ring,
				     union ixgbe_adv_rx_desc *rx_desc,
				     struct sk_buff *skb)
{
	__le16 pkt_info = rx_desc->wb.lower.lo_dword.hs_rss.pkt_info;
	bool encap_pkt = false;

	skb_checksum_none_assert(skb);

	/* Rx csum disabled */
	if (!(netdev_ring(ring)->features & NETIF_F_RXCSUM))
		return;

	/* check for VXLAN or Geneve packet type */
	if (pkt_info & cpu_to_le16(IXGBE_RXDADV_PKTTYPE_VXLAN)) {
		encap_pkt = true;
#if defined(HAVE_UDP_ENC_RX_OFFLOAD) || defined(HAVE_VXLAN_RX_OFFLOAD)
		skb->encapsulation = 1;
#endif /* HAVE_UDP_ENC_RX_OFFLOAD || HAVE_VXLAN_RX_OFFLOAD */
		skb->ip_summed = CHECKSUM_NONE;
	}

	/* if IP and error */
	if (ixgbe_test_staterr(rx_desc, IXGBE_RXD_STAT_IPCS) &&
	    ixgbe_test_staterr(rx_desc, IXGBE_RXDADV_ERR_IPE)) {
		ring->rx_stats.csum_err++;
		return;
	}

	if (!ixgbe_test_staterr(rx_desc, IXGBE_RXD_STAT_L4CS))
		return;

	if (ixgbe_test_staterr(rx_desc, IXGBE_RXDADV_ERR_TCPE)) {

		/*
		 * 82599 errata, UDP frames with a 0 checksum can be marked as
		 * checksum errors.
		 */
		if ((pkt_info & cpu_to_le16(IXGBE_RXDADV_PKTTYPE_UDP)) &&
		    test_bit(__IXGBE_RX_CSUM_UDP_ZERO_ERR, &ring->state))
			return;

		ring->rx_stats.csum_err++;
		return;
	}

	/* It must be a TCP or UDP packet with a valid checksum */
	skb->ip_summed = CHECKSUM_UNNECESSARY;
	if (encap_pkt) {
		if (!ixgbe_test_staterr(rx_desc, IXGBE_RXD_STAT_OUTERIPCS))
			return;
		
		if (ixgbe_test_staterr(rx_desc, IXGBE_RXDADV_ERR_OUTERIPER)) {
			skb->ip_summed = CHECKSUM_NONE;
			return;
		}
#ifdef HAVE_SKBUFF_CSUM_LEVEL
		/* If we checked the outer header let the stack know */
		skb->csum_level = 1;
#endif /* HAVE_SKBUFF_CSUM_LEVEL */
	}
}

static inline void ixgbe_release_rx_desc(struct ixgbe_ring *rx_ring, u32 val)
{
	rx_ring->next_to_use = val;
#ifndef CONFIG_IXGBE_DISABLE_PACKET_SPLIT

	/* update next to alloc since we have filled the ring */
	rx_ring->next_to_alloc = val;
#endif
	/*
	 * Force memory writes to complete before letting h/w
	 * know there are new descriptors to fetch.  (Only
	 * applicable for weak-ordered memory model archs,
	 * such as IA-64).
	 */
	wmb();
	writel(val, rx_ring->tail);
}

#ifdef CONFIG_IXGBE_DISABLE_PACKET_SPLIT
static bool ixgbe_alloc_mapped_skb(struct ixgbe_ring *rx_ring,
				   struct ixgbe_rx_buffer *bi)
{
	struct sk_buff *skb = bi->skb;
	dma_addr_t dma = bi->dma;

	if (unlikely(dma))
		return true;

	if (likely(!skb)) {
		skb = netdev_alloc_skb_ip_align(netdev_ring(rx_ring),
						rx_ring->rx_buf_len);
		if (unlikely(!skb)) {
			rx_ring->rx_stats.alloc_rx_buff_failed++;
			return false;
		}

		bi->skb = skb;

	}

	dma = dma_map_single(rx_ring->dev, skb->data,
			     rx_ring->rx_buf_len, DMA_FROM_DEVICE);

	/*
	 * if mapping failed free memory back to system since
	 * there isn't much point in holding memory we can't use
	 */
	if (dma_mapping_error(rx_ring->dev, dma)) {
		dev_kfree_skb_any(skb);
		bi->skb = NULL;

		rx_ring->rx_stats.alloc_rx_buff_failed++;
		return false;
	}

	bi->dma = dma;
	return true;
}

#else /* !CONFIG_IXGBE_DISABLE_PACKET_SPLIT */
static inline unsigned int ixgbe_rx_offset(struct ixgbe_ring *rx_ring)
{
	unsigned int res;

	res = ring_uses_build_skb(rx_ring) ? IXGBE_SKB_PAD : 0;

#ifdef HAVE_XDP_SUPPORT
	if (rx_ring->xdp_prog && res < XDP_PACKET_HEADROOM)
		res = XDP_PACKET_HEADROOM;
#endif

	return res;
}

static bool ixgbe_alloc_mapped_page(struct ixgbe_ring *rx_ring,
				    struct ixgbe_rx_buffer *bi)
{
	struct page *page = bi->page;
	dma_addr_t dma;
#if defined(HAVE_STRUCT_DMA_ATTRS) && defined(HAVE_SWIOTLB_SKIP_CPU_SYNC)
	DEFINE_DMA_ATTRS(attrs);

	dma_set_attr(DMA_ATTR_SKIP_CPU_SYNC, &attrs);
	dma_set_attr(DMA_ATTR_WEAK_ORDERING, &attrs);
#endif

	/* since we are recycling buffers we should seldom need to alloc */
	if (likely(page))
		return true;

	/* alloc new page for storage */
	page = dev_alloc_pages(ixgbe_rx_pg_order(rx_ring));
	if (unlikely(!page)) {
		rx_ring->rx_stats.alloc_rx_page_failed++;
		return false;
	}

	/* map page for use */
	dma = dma_map_page_attrs(rx_ring->dev, page, 0,
				 ixgbe_rx_pg_size(rx_ring),
				 DMA_FROM_DEVICE,
#if defined(HAVE_STRUCT_DMA_ATTRS) && defined(HAVE_SWIOTLB_SKIP_CPU_SYNC)
				 &attrs);
#else
				 IXGBE_RX_DMA_ATTR);
#endif

	/*
	 * if mapping failed free memory back to system since
	 * there isn't much point in holding memory we can't use
	 */
	if (dma_mapping_error(rx_ring->dev, dma)) {
		__free_pages(page, ixgbe_rx_pg_order(rx_ring));

		rx_ring->rx_stats.alloc_rx_page_failed++;
		return false;
	}

	bi->dma = dma;
	bi->page = page;
	bi->page_offset = ixgbe_rx_offset(rx_ring);
#ifdef HAVE_PAGE_COUNT_BULK_UPDATE
	page_ref_add(page, USHRT_MAX - 1);
	bi->pagecnt_bias = USHRT_MAX;
#else
	bi->pagecnt_bias = 1;
#endif
	rx_ring->rx_stats.alloc_rx_page++;

	return true;
}

#endif /* !CONFIG_IXGBE_DISABLE_PACKET_SPLIT */
/**
 * ixgbe_alloc_rx_buffers - Replace used receive buffers
 * @rx_ring: ring to place buffers on
 * @cleaned_count: number of buffers to replace
 **/
void ixgbe_alloc_rx_buffers(struct ixgbe_ring *rx_ring, u16 cleaned_count)
{
	union ixgbe_adv_rx_desc *rx_desc;
	struct ixgbe_rx_buffer *bi;
	u16 i = rx_ring->next_to_use;
#ifndef CONFIG_IXGBE_DISABLE_PACKET_SPLIT
	u16 bufsz;
#endif

	/* nothing to do */
	if (!cleaned_count)
		return;

	rx_desc = IXGBE_RX_DESC(rx_ring, i);
	bi = &rx_ring->rx_buffer_info[i];
	i -= rx_ring->count;
#ifndef CONFIG_IXGBE_DISABLE_PACKET_SPLIT

	bufsz = ixgbe_rx_bufsz(rx_ring);
#endif

	do {
#ifdef CONFIG_IXGBE_DISABLE_PACKET_SPLIT
		if (!ixgbe_alloc_mapped_skb(rx_ring, bi))
			break;
#else
		if (!ixgbe_alloc_mapped_page(rx_ring, bi))
			break;

		/* sync the buffer for use by the device */
		dma_sync_single_range_for_device(rx_ring->dev, bi->dma,
						 bi->page_offset, bufsz,
						 DMA_FROM_DEVICE);
#endif

		/*
		 * Refresh the desc even if buffer_addrs didn't change
		 * because each write-back erases this info.
		 */
#ifdef CONFIG_IXGBE_DISABLE_PACKET_SPLIT
		rx_desc->read.pkt_addr = cpu_to_le64(bi->dma);
#else
		rx_desc->read.pkt_addr = cpu_to_le64(bi->dma + bi->page_offset);
#endif

		rx_desc++;
		bi++;
		i++;
		if (unlikely(!i)) {
			rx_desc = IXGBE_RX_DESC(rx_ring, 0);
			bi = rx_ring->rx_buffer_info;
			i -= rx_ring->count;
		}

		/* clear the length for the next_to_use descriptor */
		rx_desc->wb.upper.length = 0;

		cleaned_count--;
	} while (cleaned_count);

	i += rx_ring->count;

	if (rx_ring->next_to_use != i)
		ixgbe_release_rx_desc(rx_ring, i);
}

#ifdef CONFIG_IXGBE_DISABLE_PACKET_SPLIT
/**
 * ixgbe_merge_active_tail - merge active tail into lro skb
 * @tail: pointer to active tail in frag_list
 *
 * This function merges the length and data of an active tail into the
 * skb containing the frag_list.  It resets the tail's pointer to the head,
 * but it leaves the heads pointer to tail intact.
 **/
static inline struct sk_buff *ixgbe_merge_active_tail(struct sk_buff *tail)
{
	struct sk_buff *head = IXGBE_CB(tail)->head;

	if (!head)
		return tail;

	head->len += tail->len;
	head->data_len += tail->len;
	head->truesize += tail->truesize;

	IXGBE_CB(tail)->head = NULL;

	return head;
}

/**
 * ixgbe_add_active_tail - adds an active tail into the skb frag_list
 * @head: pointer to the start of the skb
 * @tail: pointer to active tail to add to frag_list
 *
 * This function adds an active tail to the end of the frag list.  This tail
 * will still be receiving data so we cannot yet ad it's stats to the main
 * skb.  That is done via ixgbe_merge_active_tail.
 **/
static inline void ixgbe_add_active_tail(struct sk_buff *head,
					 struct sk_buff *tail)
{
	struct sk_buff *old_tail = IXGBE_CB(head)->tail;

	if (old_tail) {
		ixgbe_merge_active_tail(old_tail);
		old_tail->next = tail;
	} else {
		skb_shinfo(head)->frag_list = tail;
	}

	IXGBE_CB(tail)->head = head;
	IXGBE_CB(head)->tail = tail;
}

/**
 * ixgbe_close_active_frag_list - cleanup pointers on a frag_list skb
 * @head: pointer to head of an active frag list
 *
 * This function will clear the frag_tail_tracker pointer on an active
 * frag_list and returns true if the pointer was actually set
 **/
static inline bool ixgbe_close_active_frag_list(struct sk_buff *head)
{
	struct sk_buff *tail = IXGBE_CB(head)->tail;

	if (!tail)
		return false;

	ixgbe_merge_active_tail(tail);

	IXGBE_CB(head)->tail = NULL;

	return true;
}

#endif
#ifdef HAVE_VLAN_RX_REGISTER
/**
 * ixgbe_receive_skb - Send a completed packet up the stack
 * @q_vector: structure containing interrupt and ring information
 * @skb: packet to send up
 **/
static void ixgbe_receive_skb(struct ixgbe_q_vector *q_vector,
			      struct sk_buff *skb)
{
	u16 vlan_tag = IXGBE_CB(skb)->vid;

#if defined(NETIF_F_HW_VLAN_TX) || defined(NETIF_F_HW_VLAN_CTAG_TX)
	if (vlan_tag & VLAN_VID_MASK) {
		/* by placing vlgrp at start of structure we can alias it */
		struct vlan_group **vlgrp = netdev_priv(skb->dev);
		if (!*vlgrp)
			dev_kfree_skb_any(skb);
		else if (q_vector->netpoll_rx)
			vlan_hwaccel_rx(skb, *vlgrp, vlan_tag);
		else
			vlan_gro_receive(&q_vector->napi,
					 *vlgrp, vlan_tag, skb);
	} else {
#endif /* NETIF_F_HW_VLAN_TX || NETIF_F_HW_VLAN_CTAG_TX */
		if (q_vector->netpoll_rx)
			netif_rx(skb);
		else
			napi_gro_receive(&q_vector->napi, skb);
#if defined(NETIF_F_HW_VLAN_TX) || defined(NETIF_F_HW_VLAN_CTAG_TX)
	}
#endif /* NETIF_F_HW_VLAN_TX || NETIF_F_HW_VLAN_CTAG_TX */
}

#endif /* HAVE_VLAN_RX_REGISTER */
#ifdef NETIF_F_GSO
static void ixgbe_set_rsc_gso_size(struct ixgbe_ring __maybe_unused *ring,
				   struct sk_buff *skb)
{
	u16 hdr_len = eth_get_headlen(skb->dev, skb->data, skb_headlen(skb));

	/* set gso_size to avoid messing up TCP MSS */
	skb_shinfo(skb)->gso_size = DIV_ROUND_UP((skb->len - hdr_len),
						 IXGBE_CB(skb)->append_cnt);
	skb_shinfo(skb)->gso_type = SKB_GSO_TCPV4;
}

#endif /* NETIF_F_GSO */
static void ixgbe_update_rsc_stats(struct ixgbe_ring *rx_ring,
				   struct sk_buff *skb)
{
	/* if append_cnt is 0 then frame is not RSC */
	if (!IXGBE_CB(skb)->append_cnt)
		return;

	rx_ring->rx_stats.rsc_count += IXGBE_CB(skb)->append_cnt;
	rx_ring->rx_stats.rsc_flush++;

#ifdef NETIF_F_GSO
	ixgbe_set_rsc_gso_size(rx_ring, skb);

#endif
	/* gso_size is computed using append_cnt so always clear it last */
	IXGBE_CB(skb)->append_cnt = 0;
}

static void ixgbe_rx_vlan(struct ixgbe_ring *ring,
			  union ixgbe_adv_rx_desc *rx_desc,
			  struct sk_buff *skb)
{
#ifdef NETIF_F_HW_VLAN_CTAG_RX
	if ((netdev_ring(ring)->features & NETIF_F_HW_VLAN_CTAG_RX) &&
#else
	if ((netdev_ring(ring)->features & NETIF_F_HW_VLAN_RX) &&
#endif
	    ixgbe_test_staterr(rx_desc, IXGBE_RXD_STAT_VP))
#ifndef HAVE_VLAN_RX_REGISTER
		__vlan_hwaccel_put_tag(skb,
				       htons(ETH_P_8021Q),
				       le16_to_cpu(rx_desc->wb.upper.vlan));
#else
		IXGBE_CB(skb)->vid = le16_to_cpu(rx_desc->wb.upper.vlan);
	else
		IXGBE_CB(skb)->vid = 0;
#endif
}

/**
 * ixgbe_process_skb_fields - Populate skb header fields from Rx descriptor
 * @rx_ring: rx descriptor ring packet is being transacted on
 * @rx_desc: pointer to the EOP Rx descriptor
 * @skb: pointer to current skb being populated
 *
 * This function checks the ring, descriptor, and packet information in
 * order to populate the hash, checksum, VLAN, timestamp, protocol, and
 * other fields within the skb.
 **/
void ixgbe_process_skb_fields(struct ixgbe_ring *rx_ring,
			      union ixgbe_adv_rx_desc *rx_desc,
			      struct sk_buff *skb)
{
#ifdef HAVE_PTP_1588_CLOCK
	u32 flags = rx_ring->q_vector->adapter->flags;

#endif
	ixgbe_update_rsc_stats(rx_ring, skb);

#ifdef NETIF_F_RXHASH
	ixgbe_rx_hash(rx_ring, rx_desc, skb);

#endif /* NETIF_F_RXHASH */
	ixgbe_rx_checksum(rx_ring, rx_desc, skb);
#ifdef HAVE_PTP_1588_CLOCK
	if (unlikely(flags & IXGBE_FLAG_RX_HWTSTAMP_ENABLED))
		ixgbe_ptp_rx_hwtstamp(rx_ring, rx_desc, skb);

#endif
	ixgbe_rx_vlan(rx_ring, rx_desc, skb);

	skb_record_rx_queue(skb, ring_queue_index(rx_ring));
	skb->protocol = eth_type_trans(skb, netdev_ring(rx_ring));
}

void ixgbe_rx_skb(struct ixgbe_q_vector *q_vector,
		  struct ixgbe_ring *rx_ring,
		  union ixgbe_adv_rx_desc *rx_desc,
		  struct sk_buff *skb)
{
#ifdef HAVE_NDO_BUSY_POLL
	skb_mark_napi_id(skb, &q_vector->napi);

	if (ixgbe_qv_busy_polling(q_vector) || q_vector->netpoll_rx) {
		netif_receive_skb(skb);
		/* exit early if we busy polled */
		return;
	}
#endif

#ifdef HAVE_VLAN_RX_REGISTER
		ixgbe_receive_skb(q_vector, skb);
#else
		napi_gro_receive(&q_vector->napi, skb);
#endif
#ifndef NETIF_F_GRO

	netdev_ring(rx_ring)->last_rx = jiffies;
#endif
}

/**
 * ixgbe_is_non_eop - process handling of non-EOP buffers
 * @rx_ring: Rx ring being processed
 * @rx_desc: Rx descriptor for current buffer
 * @skb: Current socket buffer containing buffer in progress
 *
 * This function updates next to clean.  If the buffer is an EOP buffer
 * this function exits returning false, otherwise it will place the
 * sk_buff in the next buffer to be chained and return true indicating
 * that this is in fact a non-EOP buffer.
 **/
static bool ixgbe_is_non_eop(struct ixgbe_ring *rx_ring,
			     union ixgbe_adv_rx_desc *rx_desc,
			     struct sk_buff *skb)
{
#ifdef CONFIG_IXGBE_DISABLE_PACKET_SPLIT
	struct sk_buff *next_skb;
#endif
	u32 ntc = rx_ring->next_to_clean + 1;

	/* fetch, update, and store next to clean */
	ntc = (ntc < rx_ring->count) ? ntc : 0;
	rx_ring->next_to_clean = ntc;

	prefetch(IXGBE_RX_DESC(rx_ring, ntc));

	/* update RSC append count if present */
	if (ring_is_rsc_enabled(rx_ring)) {
		__le32 rsc_enabled = rx_desc->wb.lower.lo_dword.data &
				     cpu_to_le32(IXGBE_RXDADV_RSCCNT_MASK);

		if (unlikely(rsc_enabled)) {
			u32 rsc_cnt = le32_to_cpu(rsc_enabled);

			rsc_cnt >>= IXGBE_RXDADV_RSCCNT_SHIFT;
			IXGBE_CB(skb)->append_cnt += rsc_cnt - 1;

			/* update ntc based on RSC value */
			ntc = le32_to_cpu(rx_desc->wb.upper.status_error);
			ntc &= IXGBE_RXDADV_NEXTP_MASK;
			ntc >>= IXGBE_RXDADV_NEXTP_SHIFT;
		}
	}

	/* if we are the last buffer then there is nothing else to do */
	if (likely(ixgbe_test_staterr(rx_desc, IXGBE_RXD_STAT_EOP)))
		return false;

	/* place skb in next buffer to be received */
#ifdef CONFIG_IXGBE_DISABLE_PACKET_SPLIT
	next_skb = rx_ring->rx_buffer_info[ntc].skb;

	ixgbe_add_active_tail(skb, next_skb);
	IXGBE_CB(next_skb)->head = skb;
#else
	rx_ring->rx_buffer_info[ntc].skb = skb;
#endif
	rx_ring->rx_stats.non_eop_descs++;

	return true;
}

#ifndef CONFIG_IXGBE_DISABLE_PACKET_SPLIT
/**
 * ixgbe_pull_tail - ixgbe specific version of skb_pull_tail
 * @skb: pointer to current skb being adjusted
 *
 * This function is an ixgbe specific version of __pskb_pull_tail.  The
 * main difference between this version and the original function is that
 * this function can make several assumptions about the state of things
 * that allow for significant optimizations versus the standard function.
 * As a result we can do things like drop a frag and maintain an accurate
 * truesize for the skb.
 */
static void ixgbe_pull_tail(struct sk_buff *skb)
{
	skb_frag_t *frag = &skb_shinfo(skb)->frags[0];
	unsigned char *va;
	unsigned int pull_len;

	/*
	 * it is valid to use page_address instead of kmap since we are
	 * working with pages allocated out of the lomem pool per
	 * alloc_page(GFP_ATOMIC)
	 */
	va = skb_frag_address(frag);

	/*
	 * we need the header to contain the greater of either ETH_HLEN or
	 * 60 bytes if the skb->len is less than 60 for skb_pad.
	 */
	pull_len = eth_get_headlen(skb->dev, va, IXGBE_RX_HDR_SIZE);

	/* align pull length to size of long to optimize memcpy performance */
	skb_copy_to_linear_data(skb, va, ALIGN(pull_len, sizeof(long)));

	/* update all of the pointers */
	skb_frag_size_sub(frag, pull_len);
	skb_frag_off_add(frag, pull_len);
	skb->data_len -= pull_len;
	skb->tail += pull_len;
}

/**
 * ixgbe_dma_sync_frag - perform DMA sync for first frag of SKB
 * @rx_ring: rx descriptor ring packet is being transacted on
 * @skb: pointer to current skb being updated
 *
 * This function provides a basic DMA sync up for the first fragment of an
 * skb.  The reason for doing this is that the first fragment cannot be
 * unmapped until we have reached the end of packet descriptor for a buffer
 * chain.
 */
static void ixgbe_dma_sync_frag(struct ixgbe_ring *rx_ring,
				struct sk_buff *skb)
{
#if defined(HAVE_STRUCT_DMA_ATTRS) && defined(HAVE_SWIOTLB_SKIP_CPU_SYNC)
	DEFINE_DMA_ATTRS(attrs);

	dma_set_attr(DMA_ATTR_SKIP_CPU_SYNC, &attrs);
	dma_set_attr(DMA_ATTR_WEAK_ORDERING, &attrs);

#endif
	/* if the page was released unmap it, else just sync our portion */
	if (unlikely(IXGBE_CB(skb)->page_released)) {
		dma_unmap_page_attrs(rx_ring->dev, IXGBE_CB(skb)->dma,
				     ixgbe_rx_pg_size(rx_ring),
				     DMA_FROM_DEVICE,
#if defined(HAVE_STRUCT_DMA_ATTRS) && defined(HAVE_SWIOTLB_SKIP_CPU_SYNC)
				     &attrs);
#else
				     IXGBE_RX_DMA_ATTR);
#endif
	} else if (ring_uses_build_skb(rx_ring)) {
		unsigned long offset = (unsigned long)(skb->data) & ~PAGE_MASK;

		dma_sync_single_range_for_cpu(rx_ring->dev,
					      IXGBE_CB(skb)->dma,
					      offset,
					      skb_headlen(skb),
					      DMA_FROM_DEVICE);
	} else {
		skb_frag_t *frag = &skb_shinfo(skb)->frags[0];

		dma_sync_single_range_for_cpu(rx_ring->dev,
					      IXGBE_CB(skb)->dma,
					      skb_frag_off(frag),
					      skb_frag_size(frag),
					      DMA_FROM_DEVICE);
	}
}

/**
 * ixgbe_cleanup_headers - Correct corrupted or empty headers
 * @rx_ring: rx descriptor ring packet is being transacted on
 * @rx_desc: pointer to the EOP Rx descriptor
 * @skb: pointer to current skb being fixed
 *
 * Check if the skb is valid. In the XDP case it will be an error pointer.
 * Return true in this case to abort processing and advance to next
 * descriptor.
 *
 * Check for corrupted packet headers caused by senders on the local L2
 * embedded NIC switch not setting up their Tx Descriptors right.  These
 * should be very rare.
 *
 * Also address the case where we are pulling data in on pages only
 * and as such no data is present in the skb header.
 *
 * In addition if skb is not at least 60 bytes we need to pad it so that
 * it is large enough to qualify as a valid Ethernet frame.
 *
 * Returns true if an error was encountered and skb was freed.
 **/
bool ixgbe_cleanup_headers(struct ixgbe_ring __maybe_unused *rx_ring,
			   union ixgbe_adv_rx_desc *rx_desc,
			   struct sk_buff *skb)
{
	/* XDP packets use error pointer so abort at this point */
	if (IS_ERR(skb))
		return true;

	/* verify that the packet does not have any known errors */
	if (unlikely(ixgbe_test_staterr(rx_desc,
					IXGBE_RXDADV_ERR_FRAME_ERR_MASK))) {
		dev_kfree_skb_any(skb);
		return true;
	}

	/* place header in linear portion of buffer */
	if (!skb_headlen(skb))
		ixgbe_pull_tail(skb);

#if IS_ENABLED(CONFIG_FCOE)
	/* do not attempt to pad FCoE Frames as this will disrupt DDP */
	if (ixgbe_rx_is_fcoe(rx_ring, rx_desc))
		return false;
#endif

	/* if eth_skb_pad returns an error the skb was freed */
	if (eth_skb_pad(skb))
		return true;

	return false;
}

/**
 * ixgbe_reuse_rx_page - page flip buffer and store it back on the ring
 * @rx_ring: rx descriptor ring to store buffers on
 * @old_buff: donor buffer to have page reused
 *
 * Synchronizes page for reuse by the adapter
 **/
static void ixgbe_reuse_rx_page(struct ixgbe_ring *rx_ring,
				struct ixgbe_rx_buffer *old_buff)
{
	struct ixgbe_rx_buffer *new_buff;
	u16 nta = rx_ring->next_to_alloc;

	new_buff = &rx_ring->rx_buffer_info[nta];

	/* update, and store next to alloc */
	nta++;
	rx_ring->next_to_alloc = (nta < rx_ring->count) ? nta : 0;

	/* Transfer page from old buffer to new buffer.
	 * Move each member individually to avoid possible store
	 * forwarding stalls and unnecessary copy of skb.
	 */
	new_buff->dma		= old_buff->dma;
	new_buff->page		= old_buff->page;
	new_buff->page_offset	= old_buff->page_offset;
	new_buff->pagecnt_bias	= old_buff->pagecnt_bias;
}

static inline bool ixgbe_page_is_reserved(struct page *page)
{
	return (page_to_nid(page) != numa_mem_id()) || page_is_pfmemalloc(page);
}

static bool ixgbe_can_reuse_rx_page(struct ixgbe_rx_buffer *rx_buffer)
{
	unsigned int pagecnt_bias = rx_buffer->pagecnt_bias;
	struct page *page = rx_buffer->page;

	/* avoid re-using remote pages */
	if (unlikely(ixgbe_page_is_reserved(page)))
		return false;

#if (PAGE_SIZE < 8192)
	/* if we are only owner of page we can reuse it */
#ifdef HAVE_PAGE_COUNT_BULK_UPDATE
	if (unlikely((page_ref_count(page) - pagecnt_bias) > 1))
#else
	if (unlikely((page_count(page) - pagecnt_bias) > 1))
#endif
		return false;
#else
	/* The last offset is a bit aggressive in that we assume the
	 * worst case of FCoE being enabled and using a 3K buffer.
	 * However this should have minimal impact as the 1K extra is
	 * still less than one buffer in size.
	 */
#define IXGBE_LAST_OFFSET \
	(SKB_WITH_OVERHEAD(PAGE_SIZE) - IXGBE_RXBUFFER_3K)
	if (rx_buffer->page_offset > IXGBE_LAST_OFFSET)
		return false;
#endif

#ifdef HAVE_PAGE_COUNT_BULK_UPDATE
	/* If we have drained the page fragment pool we need to update
	 * the pagecnt_bias and page count so that we fully restock the
	 * number of references the driver holds.
	 */
	if (unlikely(pagecnt_bias == 1)) {
		page_ref_add(page, USHRT_MAX - 1);
		rx_buffer->pagecnt_bias = USHRT_MAX;
	}
#else
	/* Even if we own the page, we are not allowed to use atomic_set()
	 * This would break get_page_unless_zero() users.
	 */
	if (likely(!pagecnt_bias)) {
		page_ref_inc(page);
		rx_buffer->pagecnt_bias = 1;
	}
#endif

	return true;
}

/**
 * ixgbe_add_rx_frag - Add contents of Rx buffer to sk_buff
 * @rx_ring: rx descriptor ring to transact packets on
 * @rx_buffer: buffer containing page to add
 * @skb: sk_buff to place the data into
 * @size: size of data
 *
 * This function will add the data contained in rx_buffer->page to the skb.
 * This is done either through a direct copy if the data in the buffer is
 * less than the skb header size, otherwise it will just attach the page as
 * a frag to the skb.
 *
 * The function will then update the page offset if necessary and return
 * true if the buffer can be reused by the adapter.
 **/
static void ixgbe_add_rx_frag(struct ixgbe_ring *rx_ring,
			      struct ixgbe_rx_buffer *rx_buffer,
			      struct sk_buff *skb,
			      unsigned int size)
{
#if (PAGE_SIZE < 8192)
	unsigned int truesize = ixgbe_rx_pg_size(rx_ring) / 2;
#else
	unsigned int truesize = ring_uses_build_skb(rx_ring) ?
				SKB_DATA_ALIGN(IXGBE_SKB_PAD + size) :
				SKB_DATA_ALIGN(size);
#endif

	skb_add_rx_frag(skb, skb_shinfo(skb)->nr_frags, rx_buffer->page,
			rx_buffer->page_offset, size, truesize);

#if (PAGE_SIZE < 8192)
	rx_buffer->page_offset ^= truesize;
#else
	rx_buffer->page_offset += truesize;
#endif
}

static struct ixgbe_rx_buffer *
ixgbe_get_rx_buffer(struct ixgbe_ring *rx_ring,
		    union ixgbe_adv_rx_desc *rx_desc, struct sk_buff **skb,
		    const unsigned int size)
{
	struct ixgbe_rx_buffer *rx_buffer;

	rx_buffer = &rx_ring->rx_buffer_info[rx_ring->next_to_clean];
	prefetchw(rx_buffer->page);
	*skb = rx_buffer->skb;

	/* Delay unmapping of the first packet. It carries the header
	 * information, HW may still access the header after the writeback.
	 * Only unmap it when EOP is reached
	 */
	if (!ixgbe_test_staterr(rx_desc, IXGBE_RXD_STAT_EOP)) {
		if (!*skb)
			goto skip_sync;
	} else {
		if (*skb)
			ixgbe_dma_sync_frag(rx_ring, *skb);
	}

	/* we are reusing so sync this buffer for CPU use */
	dma_sync_single_range_for_cpu(rx_ring->dev,
				      rx_buffer->dma,
				      rx_buffer->page_offset,
				      size,
				      DMA_FROM_DEVICE);
skip_sync:
	rx_buffer->pagecnt_bias--;

	return rx_buffer;
}

static void ixgbe_put_rx_buffer(struct ixgbe_ring *rx_ring,
				struct ixgbe_rx_buffer *rx_buffer,
				struct sk_buff *skb)
{
#if defined(HAVE_STRUCT_DMA_ATTRS) && defined(HAVE_SWIOTLB_SKIP_CPU_SYNC)
	DEFINE_DMA_ATTRS(attrs);

	dma_set_attr(DMA_ATTR_SKIP_CPU_SYNC, &attrs);
	dma_set_attr(DMA_ATTR_WEAK_ORDERING, &attrs);

#endif
	if (ixgbe_can_reuse_rx_page(rx_buffer)) {
		/* hand second half of page back to the ring */
		ixgbe_reuse_rx_page(rx_ring, rx_buffer);
	} else {
		if (!IS_ERR(skb) && IXGBE_CB(skb)->dma == rx_buffer->dma) {
			/* the page has been released from the ring */
			IXGBE_CB(skb)->page_released = true;
		} else {
			/* we are not reusing the buffer so unmap it */
			dma_unmap_page_attrs(rx_ring->dev, rx_buffer->dma,
					     ixgbe_rx_pg_size(rx_ring),
					     DMA_FROM_DEVICE,
#if defined(HAVE_STRUCT_DMA_ATTRS) && defined(HAVE_SWIOTLB_SKIP_CPU_SYNC)
					     &attrs);
#else
					     IXGBE_RX_DMA_ATTR);
#endif
		}
		__page_frag_cache_drain(rx_buffer->page,
					rx_buffer->pagecnt_bias);
	}

	/* clear contents of rx_buffer */
	rx_buffer->page = NULL;
	rx_buffer->skb = NULL;
}

static struct sk_buff *ixgbe_construct_skb(struct ixgbe_ring *rx_ring,
					   struct ixgbe_rx_buffer *rx_buffer,
					   struct xdp_buff *xdp,
					   union ixgbe_adv_rx_desc *rx_desc)
{
	unsigned int size = xdp->data_end - xdp->data;
#if (PAGE_SIZE < 8192)
	unsigned int truesize = ixgbe_rx_pg_size(rx_ring) / 2;
#else
	unsigned int truesize = SKB_DATA_ALIGN(xdp->data_end -
					       xdp->data_hard_start);
#endif
	struct sk_buff *skb;

	/* prefetch first cache line of first page */
	prefetch(xdp->data);
#if L1_CACHE_BYTES < 128
	prefetch(xdp->data + L1_CACHE_BYTES);
#endif
	/* Note, we get here by enabling legacy-rx via:
	 *
	 *    ethtool --set-priv-flags <dev> legacy-rx on
	 *
	 * In this mode, we currently get 0 extra XDP headroom as
	 * opposed to having legacy-rx off, where we process XDP
	 * packets going to stack via ixgbe_build_skb(). The latter
	 * provides us currently with 192 bytes of headroom.
	 *
	 * For ixgbe_construct_skb() mode it means that the
	 * xdp->data_meta will always point to xdp->data, since
	 * the helper cannot expand the head. Should this ever
	 * change in future for legacy-rx mode on, then lets also
	 * add xdp->data_meta handling here.
	 */

	/* allocate a skb to store the frags */
	skb = napi_alloc_skb(&rx_ring->q_vector->napi, IXGBE_RX_HDR_SIZE);
	if (unlikely(!skb))
		return NULL;

	if (size > IXGBE_RX_HDR_SIZE) {
		if (!ixgbe_test_staterr(rx_desc, IXGBE_RXD_STAT_EOP))
			IXGBE_CB(skb)->dma = rx_buffer->dma;

		skb_add_rx_frag(skb, 0, rx_buffer->page,
				xdp->data - page_address(rx_buffer->page),
				size, truesize);
#if (PAGE_SIZE < 8192)
		rx_buffer->page_offset ^= truesize;
#else
		rx_buffer->page_offset += truesize;
#endif
	} else {
		memcpy(__skb_put(skb, size),
		       xdp->data, ALIGN(size, sizeof(long)));
		rx_buffer->pagecnt_bias++;
	}

	return skb;
}

#ifdef HAVE_SWIOTLB_SKIP_CPU_SYNC
static struct sk_buff *ixgbe_build_skb(struct ixgbe_ring *rx_ring,
				       struct ixgbe_rx_buffer *rx_buffer,
				       struct xdp_buff *xdp,
				       union ixgbe_adv_rx_desc *rx_desc)
{
#ifdef HAVE_XDP_BUFF_DATA_META
	unsigned int metasize = xdp->data - xdp->data_meta;
	void *va = xdp->data_meta;
#else
	void *va = xdp->data;
#endif
#if (PAGE_SIZE < 8192)
	unsigned int truesize = ixgbe_rx_pg_size(rx_ring) / 2;
#else
	unsigned int truesize = SKB_DATA_ALIGN(sizeof(struct skb_shared_info)) +
				SKB_DATA_ALIGN(xdp->data_end -
					       xdp->data_hard_start);
#endif
	struct sk_buff *skb;

	/* prefetch first cache line of first page */
	prefetch(va);
#if L1_CACHE_BYTES < 128
	prefetch(va + L1_CACHE_BYTES);
#endif

	/* build an skb around the page buffer */
	skb = build_skb(xdp->data_hard_start, truesize);
	if (unlikely(!skb))
		return NULL;

	/* update pointers within the skb to store the data */
	skb_reserve(skb, xdp->data - xdp->data_hard_start);
	__skb_put(skb, xdp->data_end - xdp->data);
#ifdef HAVE_XDP_BUFF_DATA_META
	if (metasize)
		skb_metadata_set(skb, metasize);
#endif

	/* record DMA address if this is the start of a chain of buffers */
	if (!ixgbe_test_staterr(rx_desc, IXGBE_RXD_STAT_EOP))
		IXGBE_CB(skb)->dma = rx_buffer->dma;

	/* update buffer offset */
#if (PAGE_SIZE < 8192)
	rx_buffer->page_offset ^= truesize;
#else
	rx_buffer->page_offset += truesize;
#endif

	return skb;
}

#endif  /* HAVE_SWIOTLB_SKIP_CPU_SYNC */

static struct sk_buff *
ixgbe_run_xdp(struct ixgbe_adapter __maybe_unused *adapter,
	      struct ixgbe_ring __maybe_unused *rx_ring,
	      struct xdp_buff __maybe_unused *xdp)
{
	int result = IXGBE_XDP_PASS;
#ifdef HAVE_XDP_SUPPORT
	struct bpf_prog *xdp_prog;
#ifdef HAVE_XDP_FRAME_STRUCT
	struct xdp_frame *xdpf;
#endif
	int err;
	u32 act;

	rcu_read_lock();
	xdp_prog = READ_ONCE(rx_ring->xdp_prog);

	if (!xdp_prog)
		goto xdp_out;

#ifdef HAVE_XDP_FRAME_STRUCT
	prefetchw(xdp->data_hard_start); /* xdp_frame write */
#endif

	act = bpf_prog_run_xdp(xdp_prog, xdp);
	switch (act) {
	case XDP_PASS:
		break;
	case XDP_TX:
#ifdef HAVE_XDP_FRAME_STRUCT
		xdpf = xdp_convert_buff_to_frame(xdp);
		if (unlikely(!xdpf)) {
			result = IXGBE_XDP_CONSUMED;
			break;
		}
		result = ixgbe_xmit_xdp_ring(adapter, xdpf);
#else
		result = ixgbe_xmit_xdp_ring(adapter, xdp);
#endif /* !HAVE_XDP_FRAME_STRUCT */
		break;
	case XDP_REDIRECT:
		err = xdp_do_redirect(adapter->netdev, xdp, xdp_prog);
		if (!err) {
			result = IXGBE_XDP_REDIR;
		} else {
			result = IXGBE_XDP_CONSUMED;
		}
		break;
	default:
		bpf_warn_invalid_xdp_action(act);
		/* fallthrough */
	case XDP_ABORTED:
		trace_xdp_exception(rx_ring->netdev, xdp_prog, act);
		/* fallthrough -- handle aborts by dropping packet */
	case XDP_DROP:
		result = IXGBE_XDP_CONSUMED;
		break;
	}
xdp_out:
	rcu_read_unlock();
#endif /* HAVE_XDP_SUPPORT */

	return ERR_PTR(-result);
}

static void ixgbe_rx_buffer_flip(struct ixgbe_ring *rx_ring,
				 struct ixgbe_rx_buffer *rx_buffer,
				 unsigned int size)
{
#if (PAGE_SIZE < 8192)
	unsigned int truesize = ixgbe_rx_pg_size(rx_ring) / 2;

	rx_buffer->page_offset ^= truesize;
#else
	unsigned int truesize = ring_uses_build_skb(rx_ring) ?
				SKB_DATA_ALIGN(IXGBE_SKB_PAD + size) :
				SKB_DATA_ALIGN(size);

	rx_buffer->page_offset += truesize;
#endif
}

/**
 * ixgbe_clean_rx_irq - Clean completed descriptors from Rx ring - bounce buf
 * @q_vector: structure containing interrupt and ring information
 * @rx_ring: rx descriptor ring to transact packets on
 * @budget: Total limit on number of packets to process
 *
 * This function provides a "bounce buffer" approach to Rx interrupt
 * processing.  The advantage to this is that on systems that have
 * expensive overhead for IOMMU access this provides a means of avoiding
 * it by maintaining the mapping of the page to the syste.
 *
 * Returns amount of work completed.
 **/
static int ixgbe_clean_rx_irq(struct ixgbe_q_vector *q_vector,
			       struct ixgbe_ring *rx_ring,
			       int budget)
{
	unsigned int total_rx_bytes = 0, total_rx_packets = 0;
	struct ixgbe_adapter *adapter = q_vector->adapter;
#if IS_ENABLED(CONFIG_FCOE)
	int ddp_bytes;
	unsigned int mss = 0;
#endif /* CONFIG_FCOE */
	u16 cleaned_count = ixgbe_desc_unused(rx_ring);
	unsigned int xdp_xmit = 0;
	struct xdp_buff xdp;

	xdp.data = NULL;
	xdp.data_end = NULL;
#ifdef HAVE_XDP_BUFF_RXQ
	xdp.rxq = &rx_ring->xdp_rxq;
#endif

	while (likely(total_rx_packets < budget)) {
		union ixgbe_adv_rx_desc *rx_desc;
		struct ixgbe_rx_buffer *rx_buffer;
		struct sk_buff *skb;
		unsigned int size;

		/* return some buffers to hardware, one at a time is too slow */
		if (cleaned_count >= IXGBE_RX_BUFFER_WRITE) {
			ixgbe_alloc_rx_buffers(rx_ring, cleaned_count);
			cleaned_count = 0;
		}

		rx_desc = IXGBE_RX_DESC(rx_ring, rx_ring->next_to_clean);
		size = le16_to_cpu(rx_desc->wb.upper.length);
		if (!size)
			break;

		/* This memory barrier is needed to keep us from reading
		 * any other fields out of the rx_desc until we know the
		 * descriptor has been written back
		 */
		dma_rmb();

		rx_buffer = ixgbe_get_rx_buffer(rx_ring, rx_desc, &skb, size);

		/* retrieve a buffer from the ring */
		if (!skb) {
			xdp.data = page_address(rx_buffer->page) +
				   rx_buffer->page_offset;
#ifdef HAVE_XDP_BUFF_DATA_META
			xdp.data_meta = xdp.data;
#endif
			xdp.data_hard_start = xdp.data -
					      ixgbe_rx_offset(rx_ring);
			xdp.data_end = xdp.data + size;

			skb = ixgbe_run_xdp(adapter, rx_ring, &xdp);
		}

		if (IS_ERR(skb)) {
			unsigned int xdp_res = -PTR_ERR(skb);

			if (xdp_res & (IXGBE_XDP_TX | IXGBE_XDP_REDIR)) {
				xdp_xmit |= xdp_res;
				ixgbe_rx_buffer_flip(rx_ring, rx_buffer, size);
			} else {
				rx_buffer->pagecnt_bias++;
			}
			total_rx_packets++;
			total_rx_bytes += size;
		} else if (skb) {
			ixgbe_add_rx_frag(rx_ring, rx_buffer, skb, size);
#ifdef HAVE_SWIOTLB_SKIP_CPU_SYNC
		} else if (ring_uses_build_skb(rx_ring)) {
			skb = ixgbe_build_skb(rx_ring, rx_buffer,
					      &xdp, rx_desc);
#endif
		} else {
			skb = ixgbe_construct_skb(rx_ring, rx_buffer,
						  &xdp, rx_desc);
		}

		/* exit if we failed to retrieve a buffer */
		if (!skb) {
			rx_ring->rx_stats.alloc_rx_buff_failed++;
			rx_buffer->pagecnt_bias++;
			break;
		}

		ixgbe_put_rx_buffer(rx_ring, rx_buffer, skb);
		cleaned_count++;

		/* place incomplete frames back on ring for completion */
		if (ixgbe_is_non_eop(rx_ring, rx_desc, skb))
			continue;

		/* verify the packet layout is correct */
		if (ixgbe_cleanup_headers(rx_ring, rx_desc, skb))
			continue;

		/* probably a little skewed due to removing CRC */
		total_rx_bytes += skb->len;

		/* populate checksum, timestamp, VLAN, and protocol */
		ixgbe_process_skb_fields(rx_ring, rx_desc, skb);

#if IS_ENABLED(CONFIG_FCOE)
		/* if ddp, not passing to ULD unless for FCP_RSP or error */
		if (ixgbe_rx_is_fcoe(rx_ring, rx_desc)) {
			ddp_bytes = ixgbe_fcoe_ddp(q_vector->adapter,
						   rx_desc, skb);
			/* include DDPed FCoE data */
			if (ddp_bytes > 0) {
				if (!mss) {
					mss = netdev_ring(rx_ring)->mtu -
						sizeof(struct fcoe_hdr) -
						sizeof(struct fc_frame_header) -
						sizeof(struct fcoe_crc_eof);
					if (mss > 512)
						mss &= ~511;
				}
				total_rx_bytes += ddp_bytes;
				total_rx_packets += DIV_ROUND_UP(ddp_bytes,
								 mss);
			}
			if (!ddp_bytes) {
				dev_kfree_skb_any(skb);
#ifndef NETIF_F_GRO
				netdev_ring(rx_ring)->last_rx = jiffies;
#endif
				continue;
			}
		}
#endif /* CONFIG_FCOE */

		ixgbe_rx_skb(q_vector, rx_ring, rx_desc, skb);

		/* update budget accounting */
		total_rx_packets++;
	}

	if (xdp_xmit & IXGBE_XDP_REDIR)
		xdp_do_flush_map();

	if (xdp_xmit & IXGBE_XDP_TX) {
		struct ixgbe_ring *ring = adapter->xdp_ring[smp_processor_id()];

		/* Force memory writes to complete before letting h/w
		 * know there are new descriptors to fetch.
		 */
		wmb();
		writel(ring->next_to_use, ring->tail);
	}

	u64_stats_update_begin(&rx_ring->syncp);
	rx_ring->stats.packets += total_rx_packets;
	rx_ring->stats.bytes += total_rx_bytes;
	u64_stats_update_end(&rx_ring->syncp);
	q_vector->rx.total_packets += total_rx_packets;
	q_vector->rx.total_bytes += total_rx_bytes;

	return total_rx_packets;
}

#else /* CONFIG_IXGBE_DISABLE_PACKET_SPLIT */
/**
 * ixgbe_clean_rx_irq - Clean completed descriptors from Rx ring - legacy
 * @q_vector: structure containing interrupt and ring information
 * @rx_ring: rx descriptor ring to transact packets on
 * @budget: Total limit on number of packets to process
 *
 * This function provides a legacy approach to Rx interrupt
 * handling.  This version will perform better on systems with a low cost
 * dma mapping API.
 *
 * Returns amount of work completed.
 **/
static int ixgbe_clean_rx_irq(struct ixgbe_q_vector *q_vector,
			       struct ixgbe_ring *rx_ring,
			       int budget)
{
	unsigned int total_rx_bytes = 0, total_rx_packets = 0;
#if IS_ENABLED(CONFIG_FCOE)
	int ddp_bytes;
	unsigned int mss = 0;
#endif /* CONFIG_FCOE */
	u16 len = 0;
	u16 cleaned_count = ixgbe_desc_unused(rx_ring);

	while (likely(total_rx_packets < budget)) {
		struct ixgbe_rx_buffer *rx_buffer;
		union ixgbe_adv_rx_desc *rx_desc;
		struct sk_buff *skb;
		u16 ntc;

		/* return some buffers to hardware, one at a time is too slow */
		if (cleaned_count >= IXGBE_RX_BUFFER_WRITE) {
			ixgbe_alloc_rx_buffers(rx_ring, cleaned_count);
			cleaned_count = 0;
		}

		ntc = rx_ring->next_to_clean;
		rx_desc = IXGBE_RX_DESC(rx_ring, ntc);
		rx_buffer = &rx_ring->rx_buffer_info[ntc];

		if (!rx_desc->wb.upper.length)
			break;

		/* This memory barrier is needed to keep us from reading
		 * any other fields out of the rx_desc until we know the
		 * descriptor has been written back
		 */
		dma_rmb();

		skb = rx_buffer->skb;

		prefetch(skb->data);

		len = le16_to_cpu(rx_desc->wb.upper.length);
		/* pull the header of the skb in */
		__skb_put(skb, len);

		/*
		 * Delay unmapping of the first packet. It carries the
		 * header information, HW may still access the header after
		 * the writeback.  Only unmap it when EOP is reached
		 */
		if (!IXGBE_CB(skb)->head) {
			IXGBE_CB(skb)->dma = rx_buffer->dma;
		} else {
			skb = ixgbe_merge_active_tail(skb);
			dma_unmap_single(rx_ring->dev,
					 rx_buffer->dma,
					 rx_ring->rx_buf_len,
					 DMA_FROM_DEVICE);
		}

		/* clear skb reference in buffer info structure */
		rx_buffer->skb = NULL;
		rx_buffer->dma = 0;

		cleaned_count++;

		if (ixgbe_is_non_eop(rx_ring, rx_desc, skb))
			continue;

		dma_unmap_single(rx_ring->dev,
				 IXGBE_CB(skb)->dma,
				 rx_ring->rx_buf_len,
				 DMA_FROM_DEVICE);

		IXGBE_CB(skb)->dma = 0;

		if (ixgbe_close_active_frag_list(skb) &&
		    !IXGBE_CB(skb)->append_cnt) {
			/* if we got here without RSC the packet is invalid */
			dev_kfree_skb_any(skb);
			continue;
		}

		/* ERR_MASK will only have valid bits if EOP set */
		if (unlikely(ixgbe_test_staterr(rx_desc,
					   IXGBE_RXDADV_ERR_FRAME_ERR_MASK))) {
			dev_kfree_skb_any(skb);
			continue;
		}

		/* probably a little skewed due to removing CRC */
		total_rx_bytes += skb->len;

		/* populate checksum, timestamp, VLAN, and protocol */
		ixgbe_process_skb_fields(rx_ring, rx_desc, skb);

#if IS_ENABLED(CONFIG_FCOE)
		/* if ddp, not passing to ULD unless for FCP_RSP or error */
		if (ixgbe_rx_is_fcoe(rx_ring, rx_desc)) {
			ddp_bytes = ixgbe_fcoe_ddp(q_vector->adapter,
						   rx_desc, skb);
			/* include DDPed FCoE data */
			if (ddp_bytes > 0) {
				if (!mss) {
					mss = netdev_ring(rx_ring)->mtu -
						sizeof(struct fcoe_hdr) -
						sizeof(struct fc_frame_header) -
						sizeof(struct fcoe_crc_eof);
					if (mss > 512)
						mss &= ~511;
				}
				total_rx_bytes += ddp_bytes;
				total_rx_packets += DIV_ROUND_UP(ddp_bytes,
								 mss);
			}
			if (!ddp_bytes) {
				dev_kfree_skb_any(skb);
#ifndef NETIF_F_GRO
				netdev_ring(rx_ring)->last_rx = jiffies;
#endif
				continue;
			}
		}

#endif /* CONFIG_FCOE */
		ixgbe_rx_skb(q_vector, rx_ring, rx_desc, skb);

		/* update budget accounting */
		total_rx_packets++;
	}

	rx_ring->stats.packets += total_rx_packets;
	rx_ring->stats.bytes += total_rx_bytes;
	q_vector->rx.total_packets += total_rx_packets;
	q_vector->rx.total_bytes += total_rx_bytes;

	if (cleaned_count)
		ixgbe_alloc_rx_buffers(rx_ring, cleaned_count);

	return total_rx_packets;
}

#endif /* CONFIG_IXGBE_DISABLE_PACKET_SPLIT */
#ifdef HAVE_NDO_BUSY_POLL
/* must be called with local_bh_disable()d */
static int ixgbe_busy_poll_recv(struct napi_struct *napi)
{
	struct ixgbe_q_vector *q_vector =
			container_of(napi, struct ixgbe_q_vector, napi);
	struct ixgbe_adapter *adapter = q_vector->adapter;
	struct ixgbe_ring  *ring;
	int found = 0;

	if (test_bit(__IXGBE_DOWN, &adapter->state))
		return LL_FLUSH_FAILED;

	if (!ixgbe_qv_lock_poll(q_vector))
		return LL_FLUSH_BUSY;

	ixgbe_for_each_ring(ring, q_vector->rx) {
		found = ixgbe_clean_rx_irq(q_vector, ring, 4);
#ifdef BP_EXTENDED_STATS
		if (found)
			ring->stats.cleaned += found;
		else
			ring->stats.misses++;
#endif
		if (found)
			break;
	}

	ixgbe_qv_unlock_poll(q_vector);

	return found;
}

#endif /* HAVE_NDO_BUSY_POLL */
/**
 * ixgbe_configure_msix - Configure MSI-X hardware
 * @adapter: board private structure
 *
 * ixgbe_configure_msix sets up the hardware to properly generate MSI-X
 * interrupts.
 **/
static void ixgbe_configure_msix(struct ixgbe_adapter *adapter)
{
	int v_idx;
	u32 mask;

	/* Populate MSIX to EITR Select */
	if (adapter->num_vfs >= 32) {
		u32 eitrsel = (1 << (adapter->num_vfs - 32)) - 1;
		IXGBE_WRITE_REG(&adapter->hw, IXGBE_EITRSEL, eitrsel);
	}

	/*
	 * Populate the IVAR table and set the ITR values to the
	 * corresponding register.
	 */
	for (v_idx = 0; v_idx < adapter->num_q_vectors; v_idx++) {
		struct ixgbe_q_vector *q_vector = adapter->q_vector[v_idx];
		struct ixgbe_ring *ring;

		ixgbe_for_each_ring(ring, q_vector->rx)
			ixgbe_set_ivar(adapter, 0, ring->reg_idx, v_idx);

		ixgbe_for_each_ring(ring, q_vector->tx)
			ixgbe_set_ivar(adapter, 1, ring->reg_idx, v_idx);

		ixgbe_write_eitr(q_vector);
	}

	switch (adapter->hw.mac.type) {
	case ixgbe_mac_82598EB:
		ixgbe_set_ivar(adapter, -1, IXGBE_IVAR_OTHER_CAUSES_INDEX,
			       v_idx);
		break;
	case ixgbe_mac_82599EB:
	case ixgbe_mac_X540:
	case ixgbe_mac_X550:
	case ixgbe_mac_X550EM_x:
	case ixgbe_mac_X550EM_a:
		ixgbe_set_ivar(adapter, -1, 1, v_idx);
		break;
	default:
		break;
	}
	IXGBE_WRITE_REG(&adapter->hw, IXGBE_EITR(v_idx), 1950);

	/* set up to autoclear timer, and the vectors */
	mask = IXGBE_EIMS_ENABLE_MASK;
	mask &= ~(IXGBE_EIMS_OTHER |
		  IXGBE_EIMS_MAILBOX |
		  IXGBE_EIMS_LSC);

	IXGBE_WRITE_REG(&adapter->hw, IXGBE_EIAC, mask);
}

static inline bool ixgbe_container_is_rx(struct ixgbe_q_vector *q_vector,
					 struct ixgbe_ring_container *rc)
{
	return &q_vector->rx == rc;
}

/**
 * ixgbe_update_itr - update the dynamic ITR value based on statistics
 * @q_vector: structure containing interrupt and ring information
 * @ring_container: structure containing ring performance data
 *
 *      Stores a new ITR value based on packets and byte
 *      counts during the last interrupt.  The advantage of per interrupt
 *      computation is faster updates and more accurate ITR for the current
 *      traffic pattern.  Constants in this function were computed
 *      based on theoretical maximum wire speed and thresholds were set based
 *      on testing data as well as attempting to minimize response time
 *      while increasing bulk throughput.
 **/
static void ixgbe_update_itr(struct ixgbe_q_vector *q_vector,
			     struct ixgbe_ring_container *ring_container)
{
	unsigned int itr = IXGBE_ITR_ADAPTIVE_MIN_USECS |
			   IXGBE_ITR_ADAPTIVE_LATENCY;
	unsigned int avg_wire_size, packets, bytes;
	unsigned long next_update = jiffies;

	/* If we don't have any rings just leave ourselves set for maximum
	 * possible latency so we take ourselves out of the equation.
	 */
	if (!ring_container->ring)
		return;

	/* If we didn't update within up to 1 - 2 jiffies we can assume
	 * that either packets are coming in so slow there hasn't been
	 * any work, or that there is so much work that NAPI is dealing
	 * with interrupt moderation and we don't need to do anything.
	 */
	if (time_after(next_update, ring_container->next_update))
		goto clear_counts;

	packets = ring_container->total_packets;
	bytes = ring_container->total_bytes;

	if (ixgbe_container_is_rx(q_vector, ring_container)) {
		/* If Rx and there are 1 to 23 packets and bytes are less than
		 * 12112 assume insufficient data to use bulk rate limiting
		 * approach. Instead we will focus on simply trying to target
		 * receiving 8 times as much data in the next interrupt.
		 */
		if (packets && packets < 24 && bytes < 12112) {
			itr = IXGBE_ITR_ADAPTIVE_LATENCY;
			avg_wire_size = (bytes + packets * 24) * 2;
			avg_wire_size = clamp_t(unsigned int,
						avg_wire_size, 2560, 12800);
			goto adjust_for_speed;
		}
	}

	/* Less than 48 packets we can assume that our current interrupt delay
	 * is only slightly too low. As such we should increase it by a small
	 * fixed amount.
	 */
	if (packets < 48) {
		itr = (q_vector->itr >> 2) + IXGBE_ITR_ADAPTIVE_MIN_INC;
		if (itr > IXGBE_ITR_ADAPTIVE_MAX_USECS)
			itr = IXGBE_ITR_ADAPTIVE_MAX_USECS;

		/* If sample size is 0 - 7 we should probably switch
		 * to latency mode instead of trying to control
		 * things as though we are in bulk.
		 *
		 * Otherwise if the number of packets is less than 48
		 * we should maintain whatever mode we are currently
		 * in. The range between 8 and 48 is the cross-over
		 * point between latency and bulk traffic.
		 */
		if (packets < 8)
			itr += IXGBE_ITR_ADAPTIVE_LATENCY;
		else
			itr += ring_container->itr & IXGBE_ITR_ADAPTIVE_LATENCY;
		goto clear_counts;
	}

	/* Between 48 and 96 is our "goldilocks" zone where we are working
	 * out "just right". Just report that our current ITR is good for us.
	 */
	if (packets < 96) {
		itr = q_vector->itr >> 2;
		goto clear_counts;
	}

	/* If packet count is 96 or greater we are likely looking at a slight
	 * overrun of the delay we want. Try halving our delay to see if that
	 * will cut the number of packets in half per interrupt.
	 */
	if (packets < 256) {
		itr = q_vector->itr >> 3;
		if (itr < IXGBE_ITR_ADAPTIVE_MIN_USECS)
			itr = IXGBE_ITR_ADAPTIVE_MIN_USECS;
		goto clear_counts;
	}

	/* The paths below assume we are dealing with a bulk ITR since number
	 * of packets is 256 or greater. We are just going to have to compute
	 * a value and try to bring the count under control, though for smaller
	 * packet sizes there isn't much we can do as NAPI polling will likely
	 * be kicking in sooner rather than later.
	 */
	itr = IXGBE_ITR_ADAPTIVE_BULK;

	/* If packet counts are 256 or greater we can assume we have a gross
	 * overestimation of what the rate should be. Instead of trying to fine
	 * tune it just use the formula below to try and dial in an exact value
	 * give the current packet size of the frame.
	 */
	avg_wire_size = bytes / packets;

	/* The following is a crude approximation of:
	 *  wmem_default / (size + overhead) = desired_pkts_per_int
	 *  rate / bits_per_byte / (size + ethernet overhead) = pkt_rate
	 *  (desired_pkt_rate / pkt_rate) * usecs_per_sec = ITR value
	 *
	 * Assuming wmem_default is 212992 and overhead is 640 bytes per
	 * packet, (256 skb, 64 headroom, 320 shared info), we can reduce the
	 * formula down to
	 *
	 *  (170 * (size + 24)) / (size + 640) = ITR
	 *
	 * We first do some math on the packet size and then finally bitshift
	 * by 8 after rounding up. We also have to account for PCIe link speed
	 * difference as ITR scales based on this.
	 */
	if (avg_wire_size <= 60) {
		/* Start at 50k ints/sec */
		avg_wire_size = 5120;
	} else if (avg_wire_size <= 316) {
		/* 50K ints/sec to 16K ints/sec */
		avg_wire_size *= 40;
		avg_wire_size += 2720;
	} else if (avg_wire_size <= 1084) {
		/* 16K ints/sec to 9.2K ints/sec */
		avg_wire_size *= 15;
		avg_wire_size += 11452;
	} else if (avg_wire_size <= 1980) {
		/* 9.2K ints/sec to 8K ints/sec */
		avg_wire_size *= 5;
		avg_wire_size += 22420;
	} else {
		/* plateau at a limit of 8K ints/sec */
		avg_wire_size = 32256;
	}

adjust_for_speed:
	/* Resultant value is 256 times larger than it needs to be. This
	 * gives us room to adjust the value as needed to either increase
	 * or decrease the value based on link speeds of 10G, 2.5G, 1G, etc.
	 *
	 * Use addition as we have already recorded the new latency flag
	 * for the ITR value.
	 */
	switch (q_vector->adapter->link_speed) {
	case IXGBE_LINK_SPEED_10GB_FULL:
	case IXGBE_LINK_SPEED_100_FULL:
	default:
		itr += DIV_ROUND_UP(avg_wire_size,
				    IXGBE_ITR_ADAPTIVE_MIN_INC * 256) *
		       IXGBE_ITR_ADAPTIVE_MIN_INC;
		break;
	case IXGBE_LINK_SPEED_2_5GB_FULL:
	case IXGBE_LINK_SPEED_1GB_FULL:
	case IXGBE_LINK_SPEED_10_FULL:
		itr += DIV_ROUND_UP(avg_wire_size,
				    IXGBE_ITR_ADAPTIVE_MIN_INC * 64) *
		       IXGBE_ITR_ADAPTIVE_MIN_INC;
		break;
	}

	/* In the case of a latency specific workload only allow us to
	 * reduce the ITR by at most 2us. By doing this we should dial
	 * in so that our number of interrupts is no more than 2x the number
	 * of packets for the least busy workload. So for example in the case
	 * of a TCP worload the ack packets being received would set the
	 * the interrupt rate as they are a latency specific workload.
	 */
	if ((itr & IXGBE_ITR_ADAPTIVE_LATENCY) && itr < ring_container->itr)
		itr = ring_container->itr - IXGBE_ITR_ADAPTIVE_MIN_INC;
clear_counts:
	/* write back value */
	ring_container->itr = itr;

	/* next update should occur within next jiffy */
	ring_container->next_update = next_update + 1;

	ring_container->total_bytes = 0;
	ring_container->total_packets = 0;
}

/**
 * ixgbe_write_eitr - write EITR register in hardware specific way
 * @q_vector: structure containing interrupt and ring information
 *
 * This function is made to be called by ethtool and by the driver
 * when it needs to update EITR registers at runtime.  Hardware
 * specific quirks/differences are taken care of here.
 */
void ixgbe_write_eitr(struct ixgbe_q_vector *q_vector)
{
	struct ixgbe_adapter *adapter = q_vector->adapter;
	struct ixgbe_hw *hw = &adapter->hw;
	int v_idx = q_vector->v_idx;
	u32 itr_reg = q_vector->itr & IXGBE_MAX_EITR;

	switch (adapter->hw.mac.type) {
	case ixgbe_mac_82598EB:
		/* must write high and low 16 bits to reset counter */
		itr_reg |= (itr_reg << 16);
		break;
	case ixgbe_mac_82599EB:
	case ixgbe_mac_X540:
	case ixgbe_mac_X550:
	case ixgbe_mac_X550EM_x:
	case ixgbe_mac_X550EM_a:
		/*
		 * set the WDIS bit to not clear the timer bits and cause an
		 * immediate assertion of the interrupt
		 */
		itr_reg |= IXGBE_EITR_CNT_WDIS;
		break;
	default:
		break;
	}
	IXGBE_WRITE_REG(hw, IXGBE_EITR(v_idx), itr_reg);
}

static void ixgbe_set_itr(struct ixgbe_q_vector *q_vector)
{
	u32 new_itr;

	ixgbe_update_itr(q_vector, &q_vector->tx);
	ixgbe_update_itr(q_vector, &q_vector->rx);

	/* use the smallest value of new ITR delay calculations */
	new_itr = min(q_vector->rx.itr, q_vector->tx.itr);

	/* Clear latency flag if set, shift into correct position */
	new_itr &= IXGBE_ITR_ADAPTIVE_MASK_USECS;
	new_itr <<= 2;

	if (new_itr != q_vector->itr) {
		/* save the algorithm value here */
		q_vector->itr = new_itr;

		ixgbe_write_eitr(q_vector);
	}
}

/**
 * ixgbe_check_overtemp_subtask - check for over temperature
 * @adapter: pointer to adapter
 **/
static void ixgbe_check_overtemp_subtask(struct ixgbe_adapter *adapter)
{
	struct ixgbe_hw *hw = &adapter->hw;
	u32 eicr = adapter->interrupt_event;
	s32 rc;

	if (test_bit(__IXGBE_DOWN, &adapter->state))
		return;

	if (!(adapter->flags2 & IXGBE_FLAG2_TEMP_SENSOR_EVENT))
		return;

	adapter->flags2 &= ~IXGBE_FLAG2_TEMP_SENSOR_EVENT;

	switch (hw->device_id) {
	case IXGBE_DEV_ID_82599_T3_LOM:
		/*
		 * Since the warning interrupt is for both ports
		 * we don't have to check if:
		 *  - This interrupt wasn't for our port.
		 *  - We may have missed the interrupt so always have to
		 *    check if we  got a LSC
		 */
		if (!(eicr & IXGBE_EICR_GPI_SDP0) &&
		    !(eicr & IXGBE_EICR_LSC))
			return;

		if (!(eicr & IXGBE_EICR_LSC) && hw->mac.ops.check_link) {
			u32 speed;
			bool link_up = false;

			hw->mac.ops.check_link(hw, &speed, &link_up, false);

			if (link_up)
				return;
		}

		/* Check if this is not due to overtemp */
		if (hw->phy.ops.check_overtemp(hw) != IXGBE_ERR_OVERTEMP)
			return;

		break;
	case IXGBE_DEV_ID_X550EM_A_1G_T:
	case IXGBE_DEV_ID_X550EM_A_1G_T_L:
		rc = hw->phy.ops.check_overtemp(hw);
		if (rc != IXGBE_ERR_OVERTEMP)
			return;
		break;
	default:
		if (adapter->hw.mac.type >= ixgbe_mac_X540)
			return;
		if (!(eicr & IXGBE_EICR_GPI_SDP0_BY_MAC(hw)))
			return;
		break;
	}
	e_crit(drv, "%s\n", ixgbe_overheat_msg);

	adapter->interrupt_event = 0;
}

static void ixgbe_check_fan_failure(struct ixgbe_adapter *adapter, u32 eicr)
{
	struct ixgbe_hw *hw = &adapter->hw;

	if ((adapter->flags & IXGBE_FLAG_FAN_FAIL_CAPABLE) &&
	    (eicr & IXGBE_EICR_GPI_SDP1)) {
		e_crit(probe, "Fan has stopped, replace the adapter\n");
		/* write to clear the interrupt */
		IXGBE_WRITE_REG(hw, IXGBE_EICR, IXGBE_EICR_GPI_SDP1);
	}
}

static void ixgbe_check_overtemp_event(struct ixgbe_adapter *adapter, u32 eicr)
{
	if (!(adapter->flags2 & IXGBE_FLAG2_TEMP_SENSOR_CAPABLE))
		return;

	switch (adapter->hw.mac.type) {
	case ixgbe_mac_82599EB:
		/*
		 * Need to check link state so complete overtemp check
		 * on service task
		 */
		if (((eicr & IXGBE_EICR_GPI_SDP0) || (eicr & IXGBE_EICR_LSC)) &&
		    (!test_bit(__IXGBE_DOWN, &adapter->state))) {
			adapter->interrupt_event = eicr;
			adapter->flags2 |= IXGBE_FLAG2_TEMP_SENSOR_EVENT;
			ixgbe_service_event_schedule(adapter);
			return;
		}
		return;
	case ixgbe_mac_X550EM_a:
		if (eicr & IXGBE_EICR_GPI_SDP0_X550EM_a) {
			adapter->interrupt_event = eicr;
			adapter->flags2 |= IXGBE_FLAG2_TEMP_SENSOR_EVENT;
			ixgbe_service_event_schedule(adapter);
			IXGBE_WRITE_REG(&adapter->hw, IXGBE_EIMC,
					IXGBE_EICR_GPI_SDP0_X550EM_a);
			IXGBE_WRITE_REG(&adapter->hw, IXGBE_EICR,
					IXGBE_EICR_GPI_SDP0_X550EM_a);
		}
		return;
	case ixgbe_mac_X550:
	case ixgbe_mac_X540:
		if (!(eicr & IXGBE_EICR_TS))
			return;
		break;
	default:
		return;
	}

	e_crit(drv, "%s\n", ixgbe_overheat_msg);
}

static void ixgbe_check_sfp_event(struct ixgbe_adapter *adapter, u32 eicr)
{
	struct ixgbe_hw *hw = &adapter->hw;
	u32 eicr_mask = IXGBE_EICR_GPI_SDP2_BY_MAC(hw);

	if (!ixgbe_is_sfp(hw))
		return;
	if (hw->mac.type >= ixgbe_mac_X540)
		eicr_mask = IXGBE_EICR_GPI_SDP0_X540;

	if (eicr & eicr_mask) {
		/* Clear the interrupt */
		IXGBE_WRITE_REG(hw, IXGBE_EICR, eicr_mask);
		if (!test_bit(__IXGBE_DOWN, &adapter->state)) {
			adapter->flags2 |= IXGBE_FLAG2_SFP_NEEDS_RESET;
			adapter->sfp_poll_time = 0;
			ixgbe_service_event_schedule(adapter);
		}
	}

	if (adapter->hw.mac.type == ixgbe_mac_82599EB &&
	    (eicr & IXGBE_EICR_GPI_SDP1_BY_MAC(hw))) {
		/* Clear the interrupt */
		IXGBE_WRITE_REG(hw, IXGBE_EICR, IXGBE_EICR_GPI_SDP1_BY_MAC(hw));
		if (!test_bit(__IXGBE_DOWN, &adapter->state)) {
			adapter->flags |= IXGBE_FLAG_NEED_LINK_CONFIG;
			ixgbe_service_event_schedule(adapter);
		}
	}
}

static void ixgbe_check_lsc(struct ixgbe_adapter *adapter)
{
	struct ixgbe_hw *hw = &adapter->hw;

	adapter->lsc_int++;
	adapter->flags |= IXGBE_FLAG_NEED_LINK_UPDATE;
	adapter->link_check_timeout = jiffies;
	if (!test_bit(__IXGBE_DOWN, &adapter->state)) {
		IXGBE_WRITE_REG(hw, IXGBE_EIMC, IXGBE_EIMC_LSC);
		IXGBE_WRITE_FLUSH(hw);
		ixgbe_service_event_schedule(adapter);
	}
}

static void ixgbe_irq_enable_queues(struct ixgbe_adapter *adapter, u64 qmask)
{
	u32 mask;
	struct ixgbe_hw *hw = &adapter->hw;

	switch (hw->mac.type) {
	case ixgbe_mac_82598EB:
		mask = (IXGBE_EIMS_RTX_QUEUE & qmask);
		IXGBE_WRITE_REG(hw, IXGBE_EIMS, mask);
		break;
	case ixgbe_mac_82599EB:
	case ixgbe_mac_X540:
	case ixgbe_mac_X550:
	case ixgbe_mac_X550EM_x:
	case ixgbe_mac_X550EM_a:
		mask = (qmask & 0xFFFFFFFF);
		if (mask)
			IXGBE_WRITE_REG(hw, IXGBE_EIMS_EX(0), mask);
		mask = (qmask >> 32);
		if (mask)
			IXGBE_WRITE_REG(hw, IXGBE_EIMS_EX(1), mask);
		break;
	default:
		break;
	}
	/* skip the flush */
}

/**
 * ixgbe_irq_enable - Enable default interrupt generation settings
 * @adapter: board private structure
 * @queues: enable irqs for queues
 * @flush: flush register write
 **/
static inline void ixgbe_irq_enable(struct ixgbe_adapter *adapter, bool queues,
				    bool flush)
{
	u32 mask = (IXGBE_EIMS_ENABLE_MASK & ~IXGBE_EIMS_RTX_QUEUE);

	/* don't reenable LSC while waiting for link */
	if (adapter->flags & IXGBE_FLAG_NEED_LINK_UPDATE)
		mask &= ~IXGBE_EIMS_LSC;

	if (adapter->flags2 & IXGBE_FLAG2_TEMP_SENSOR_CAPABLE)
		switch (adapter->hw.mac.type) {
		case ixgbe_mac_82599EB:
		case ixgbe_mac_X550EM_a:
			mask |= IXGBE_EIMS_GPI_SDP0_BY_MAC(&adapter->hw);
			break;
		case ixgbe_mac_X540:
		case ixgbe_mac_X550:
			mask |= IXGBE_EIMS_TS;
			break;
		default:
			break;
		}
	if (adapter->flags & IXGBE_FLAG_FAN_FAIL_CAPABLE)
		mask |= IXGBE_EIMS_GPI_SDP1;
	switch (adapter->hw.mac.type) {
	case ixgbe_mac_82599EB:
		mask |= IXGBE_EIMS_GPI_SDP1;
		mask |= IXGBE_EIMS_GPI_SDP2;
		/* fall through */
	case ixgbe_mac_X540:
	case ixgbe_mac_X550:
	case ixgbe_mac_X550EM_x:
	case ixgbe_mac_X550EM_a:
		if (adapter->hw.device_id == IXGBE_DEV_ID_X550EM_X_SFP ||
		    adapter->hw.device_id == IXGBE_DEV_ID_X550EM_A_SFP ||
		    adapter->hw.device_id == IXGBE_DEV_ID_X550EM_A_SFP_N)
			mask |= IXGBE_EIMS_GPI_SDP0_BY_MAC(&adapter->hw);
		if (adapter->hw.phy.type == ixgbe_phy_x550em_ext_t)
			mask |= IXGBE_EICR_GPI_SDP0_X540;
		mask |= IXGBE_EIMS_ECC;
		mask |= IXGBE_EIMS_MAILBOX;
#ifdef HAVE_PTP_1588_CLOCK
		mask |= IXGBE_EIMS_TIMESYNC;
#endif

		break;
	default:
		break;
	}

	if ((adapter->flags & IXGBE_FLAG_FDIR_HASH_CAPABLE) &&
	    !(adapter->flags2 & IXGBE_FLAG2_FDIR_REQUIRES_REINIT))
		mask |= IXGBE_EIMS_FLOW_DIR;

	IXGBE_WRITE_REG(&adapter->hw, IXGBE_EIMS, mask);
	if (queues)
		ixgbe_irq_enable_queues(adapter, ~0);
	if (flush)
		IXGBE_WRITE_FLUSH(&adapter->hw);
}

static irqreturn_t ixgbe_msix_other(int __always_unused irq, void *data)
{
	struct ixgbe_adapter *adapter = data;
	struct ixgbe_hw *hw = &adapter->hw;
	u32 eicr;

	/*
	 * Workaround for Silicon errata #26 on 82598.  Use clear-by-write
	 * instead of clear-by-read.  Reading with EICS will return the
	 * interrupt causes without clearing, which later be done
	 * with the write to EICR.
	 */
	eicr = IXGBE_READ_REG(hw, IXGBE_EICS);

	/* The lower 16bits of the EICR register are for the queue interrupts
	 * which should be masked here in order to not accidently clear them if
	 * the bits are high when ixgbe_msix_other is called. There is a race
	 * condition otherwise which results in possible performance loss
	 * especially if the ixgbe_msix_other interrupt is triggering
	 * consistently (as it would when PPS is turned on for the X540 device)
	 */
	eicr &= 0xFFFF0000;

	IXGBE_WRITE_REG(hw, IXGBE_EICR, eicr);

	if (eicr & IXGBE_EICR_LSC)
		ixgbe_check_lsc(adapter);

	if (eicr & IXGBE_EICR_MAILBOX)
		ixgbe_msg_task(adapter);

	switch (hw->mac.type) {
	case ixgbe_mac_82599EB:
	case ixgbe_mac_X540:
	case ixgbe_mac_X550:
	case ixgbe_mac_X550EM_x:
	case ixgbe_mac_X550EM_a:
		if (hw->phy.type == ixgbe_phy_x550em_ext_t &&
		    (eicr & IXGBE_EICR_GPI_SDP0_X540)) {
			adapter->flags2 |= IXGBE_FLAG2_PHY_INTERRUPT;
			ixgbe_service_event_schedule(adapter);
			IXGBE_WRITE_REG(hw, IXGBE_EICR,
					IXGBE_EICR_GPI_SDP0_X540);
		}
		if (eicr & IXGBE_EICR_ECC) {
			e_info(link, "Received unrecoverable ECC Err,"
			       "initiating reset.\n");
			set_bit(__IXGBE_RESET_REQUESTED, &adapter->state);
			ixgbe_service_event_schedule(adapter);
			IXGBE_WRITE_REG(hw, IXGBE_EICR, IXGBE_EICR_ECC);
		}
#ifdef HAVE_TX_MQ
		/* Handle Flow Director Full threshold interrupt */
		if (eicr & IXGBE_EICR_FLOW_DIR) {
			int reinit_count = 0;
			int i;
			for (i = 0; i < adapter->num_tx_queues; i++) {
				struct ixgbe_ring *ring = adapter->tx_ring[i];
				if (test_and_clear_bit(
						      __IXGBE_TX_FDIR_INIT_DONE,
						      &ring->state))
					reinit_count++;
			}
			if (reinit_count) {
				/* no more flow director interrupts until
				 * after init
				 */
				IXGBE_WRITE_REG(hw, IXGBE_EIMC,
						IXGBE_EIMC_FLOW_DIR);
				adapter->flags2 |=
					IXGBE_FLAG2_FDIR_REQUIRES_REINIT;
				ixgbe_service_event_schedule(adapter);
			}
		}
#endif
		ixgbe_check_sfp_event(adapter, eicr);
		ixgbe_check_overtemp_event(adapter, eicr);
		break;
	default:
		break;
	}

	ixgbe_check_fan_failure(adapter, eicr);

#ifdef HAVE_PTP_1588_CLOCK
	if (unlikely(eicr & IXGBE_EICR_TIMESYNC))
	    ixgbe_ptp_check_pps_event(adapter);
#endif

	/* re-enable the original interrupt state, no lsc, no queues */
	if (!test_bit(__IXGBE_DOWN, &adapter->state))
		ixgbe_irq_enable(adapter, false, false);

	return IRQ_HANDLED;
}

static irqreturn_t ixgbe_msix_clean_rings(int __always_unused irq, void *data)
{
	struct ixgbe_q_vector *q_vector = data;

	/* EIAM disabled interrupts (on this vector) for us */

	if (q_vector->rx.ring || q_vector->tx.ring)
		napi_schedule_irqoff(&q_vector->napi);

	return IRQ_HANDLED;
}

/**
 * ixgbe_poll - NAPI polling RX/TX cleanup routine
 * @napi: napi struct with our devices info in it
 * @budget: amount of work driver is allowed to do this pass, in packets
 *
 * This function will clean all queues associated with a q_vector.
 **/
int ixgbe_poll(struct napi_struct *napi, int budget)
{
	struct ixgbe_q_vector *q_vector =
			       container_of(napi, struct ixgbe_q_vector, napi);
	struct ixgbe_adapter *adapter = q_vector->adapter;
	struct ixgbe_ring *ring;
	int per_ring_budget, work_done = 0;
	bool clean_complete = true;

#if IS_ENABLED(CONFIG_DCA)
	if (adapter->flags & IXGBE_FLAG_DCA_ENABLED)
		ixgbe_update_dca(q_vector);
#endif /* CONFIG_DCA */

	ixgbe_for_each_ring(ring, q_vector->tx) {
#ifdef HAVE_AF_XDP_ZC_SUPPORT
		bool wd = ring->xsk_umem ?
			  ixgbe_clean_xdp_tx_irq(q_vector, ring, budget) :
			  ixgbe_clean_tx_irq(q_vector, ring, budget);
		if (!wd)
#else
		if (!ixgbe_clean_tx_irq(q_vector, ring, budget))
#endif
			clean_complete = false;
	}

#ifdef HAVE_NDO_BUSY_POLL
	if (test_bit(NAPI_STATE_NPSVC, &napi->state))
		return budget;

	/* Exit if we are called by netpoll or busy polling is active */
	if ((budget <= 0) || !ixgbe_qv_lock_napi(q_vector))
		return budget;
#else
	/* Exit if we are called by netpoll */
	if (budget <= 0)
		return budget;
#endif

	/* attempt to distribute budget to each queue fairly, but don't allow
	 * the budget to go below 1 because we'll exit polling */
	if (q_vector->rx.count > 1)
		per_ring_budget = max(budget/q_vector->rx.count, 1);
	else
		per_ring_budget = budget;

	ixgbe_for_each_ring(ring, q_vector->rx) {
#ifdef HAVE_AF_XDP_ZC_SUPPORT
		int cleaned = ring->xsk_umem ?
			      ixgbe_clean_rx_irq_zc(q_vector, ring,
						    per_ring_budget) :
			      ixgbe_clean_rx_irq(q_vector, ring,
						 per_ring_budget);
#else
		int cleaned = ixgbe_clean_rx_irq(q_vector, ring,
						 per_ring_budget);
#endif /* HAVE_AF_XDP_ZC_SUPPORT */
		work_done += cleaned;
		if (cleaned >= per_ring_budget)
			clean_complete = false;
	}

#ifdef HAVE_NDO_BUSY_POLL
	ixgbe_qv_unlock_napi(q_vector);

#endif
#ifndef HAVE_NETDEV_NAPI_LIST
	if (!netif_running(adapter->netdev))
		clean_complete = true;

#endif
	/* If all work not completed, return budget and keep polling */
	if (!clean_complete)
		return budget;
	/* all work done, exit the polling mode */
	if (likely(napi_complete_done(napi, work_done))) {
		if (adapter->rx_itr_setting == 1)
			ixgbe_set_itr(q_vector);
		if (!test_bit(__IXGBE_DOWN, &adapter->state))
			ixgbe_irq_enable_queues(adapter,
						((u64)1 << q_vector->v_idx));
	}
	return min(work_done, budget - 1);
}

/**
 * ixgbe_request_msix_irqs - Initialize MSI-X interrupts
 * @adapter: board private structure
 *
 * ixgbe_request_msix_irqs allocates MSI-X vectors and requests
 * interrupts from the kernel.
 **/
static int ixgbe_request_msix_irqs(struct ixgbe_adapter *adapter)
{
	struct net_device *netdev = adapter->netdev;
	unsigned int ri = 0, ti = 0;
	int vector, err;

	for (vector = 0; vector < adapter->num_q_vectors; vector++) {
		struct ixgbe_q_vector *q_vector = adapter->q_vector[vector];
		struct msix_entry *entry = &adapter->msix_entries[vector];

		if (q_vector->tx.ring && q_vector->rx.ring) {
			snprintf(q_vector->name, sizeof(q_vector->name),
				 "%s-TxRx-%u", netdev->name, ri++);
			ti++;
		} else if (q_vector->rx.ring) {
			snprintf(q_vector->name, sizeof(q_vector->name),
				 "%s-rx-%u", netdev->name, ri++);
		} else if (q_vector->tx.ring) {
			snprintf(q_vector->name, sizeof(q_vector->name),
				 "%s-tx-%u", netdev->name, ti++);
		} else {
			/* skip this unused q_vector */
			continue;
		}
		err = request_irq(entry->vector, &ixgbe_msix_clean_rings, 0,
				  q_vector->name, q_vector);
		if (err) {
			e_err(probe, "request_irq failed for MSIX interrupt '%s' "
			      "Error: %d\n", q_vector->name, err);
			goto free_queue_irqs;
		}
#ifdef HAVE_IRQ_AFFINITY_HINT
		/* If Flow Director is enabled, set interrupt affinity */
		if (adapter->flags & IXGBE_FLAG_FDIR_HASH_CAPABLE) {
			/* assign the mask for this irq */
			irq_set_affinity_hint(entry->vector,
					      &q_vector->affinity_mask);
		}
#endif /* HAVE_IRQ_AFFINITY_HINT */
	}

	err = request_irq(adapter->msix_entries[vector].vector,
			  ixgbe_msix_other, 0, netdev->name, adapter);
	if (err) {
		e_err(probe, "request_irq for msix_other failed: %d\n", err);
		goto free_queue_irqs;
	}

	return IXGBE_SUCCESS;

free_queue_irqs:
	while (vector) {
		vector--;
#ifdef HAVE_IRQ_AFFINITY_HINT
		irq_set_affinity_hint(adapter->msix_entries[vector].vector,
				      NULL);
#endif
		free_irq(adapter->msix_entries[vector].vector,
			 adapter->q_vector[vector]);
	}
	adapter->flags &= ~IXGBE_FLAG_MSIX_ENABLED;
	pci_disable_msix(adapter->pdev);
	kfree(adapter->msix_entries);
	adapter->msix_entries = NULL;
	return err;
}

/**
 * ixgbe_intr - legacy mode Interrupt Handler
 * @irq: interrupt number
 * @data: pointer to a network interface device structure
 **/
static irqreturn_t ixgbe_intr(int __always_unused irq, void *data)
{
	struct ixgbe_adapter *adapter = data;
	struct ixgbe_hw *hw = &adapter->hw;
	struct ixgbe_q_vector *q_vector = adapter->q_vector[0];
	u32 eicr;

	/*
	 * Workaround for silicon errata #26 on 82598.  Mask the interrupt
	 * before the read of EICR.
	 */
	IXGBE_WRITE_REG(hw, IXGBE_EIMC, IXGBE_IRQ_CLEAR_MASK);

	/* for NAPI, using EIAM to auto-mask tx/rx interrupt bits on read
	 * therefore no explicit interrupt disable is necessary */
	eicr = IXGBE_READ_REG(hw, IXGBE_EICR);
	if (!eicr) {
		/*
		 * shared interrupt alert!
		 * make sure interrupts are enabled because the read will
		 * have disabled interrupts due to EIAM
		 * finish the workaround of silicon errata on 82598.  Unmask
		 * the interrupt that we masked before the EICR read.
		 */
		if (!test_bit(__IXGBE_DOWN, &adapter->state))
			ixgbe_irq_enable(adapter, true, true);
		return IRQ_NONE;	/* Not our interrupt */
	}

	if (eicr & IXGBE_EICR_LSC)
		ixgbe_check_lsc(adapter);

	switch (hw->mac.type) {
	case ixgbe_mac_82599EB:
	case ixgbe_mac_X540:
	case ixgbe_mac_X550:
	case ixgbe_mac_X550EM_x:
	case ixgbe_mac_X550EM_a:

		if (eicr & IXGBE_EICR_ECC) {
			e_info(link, "Received unrecoverable ECC Err,"
			       "initiating reset.\n");
			set_bit(__IXGBE_RESET_REQUESTED, &adapter->state);
			ixgbe_service_event_schedule(adapter);
			IXGBE_WRITE_REG(hw, IXGBE_EICR, IXGBE_EICR_ECC);
		}
		ixgbe_check_sfp_event(adapter, eicr);
		ixgbe_check_overtemp_event(adapter, eicr);
		break;
	default:
		break;
	}

	ixgbe_check_fan_failure(adapter, eicr);
#ifdef HAVE_PTP_1588_CLOCK
	if (unlikely(eicr & IXGBE_EICR_TIMESYNC))
	    ixgbe_ptp_check_pps_event(adapter);
#endif

	/* would disable interrupts here but EIAM disabled it */
	napi_schedule_irqoff(&q_vector->napi);

	/*
	 * re-enable link(maybe) and non-queue interrupts, no flush.
	 * ixgbe_poll will re-enable the queue interrupts
	 */
	if (!test_bit(__IXGBE_DOWN, &adapter->state))
		ixgbe_irq_enable(adapter, false, false);

	return IRQ_HANDLED;
}

/**
 * ixgbe_request_irq - initialize interrupts
 * @adapter: board private structure
 *
 * Attempts to configure interrupts using the best available
 * capabilities of the hardware and kernel.
 **/
static int ixgbe_request_irq(struct ixgbe_adapter *adapter)
{
	struct net_device *netdev = adapter->netdev;
	int err;

	if (adapter->flags & IXGBE_FLAG_MSIX_ENABLED)
		err = ixgbe_request_msix_irqs(adapter);
	else if (adapter->flags & IXGBE_FLAG_MSI_ENABLED)
		err = request_irq(adapter->pdev->irq, &ixgbe_intr, 0,
				  netdev->name, adapter);
	else
		err = request_irq(adapter->pdev->irq, &ixgbe_intr, IRQF_SHARED,
				  netdev->name, adapter);

	if (err)
		e_err(probe, "request_irq failed, Error %d\n", err);

	return err;
}

static void ixgbe_free_irq(struct ixgbe_adapter *adapter)
{
	int vector;

	if (!(adapter->flags & IXGBE_FLAG_MSIX_ENABLED)) {
		free_irq(adapter->pdev->irq, adapter);
		return;
	}

	if (!adapter->msix_entries)
		return;

	for (vector = 0; vector < adapter->num_q_vectors; vector++) {
		struct ixgbe_q_vector *q_vector = adapter->q_vector[vector];
		struct msix_entry *entry = &adapter->msix_entries[vector];

		/* free only the irqs that were actually requested */
		if (!q_vector->rx.ring && !q_vector->tx.ring)
			continue;

#ifdef HAVE_IRQ_AFFINITY_HINT
		/* clear the affinity_mask in the IRQ descriptor */
		irq_set_affinity_hint(entry->vector, NULL);

#endif
		free_irq(entry->vector, q_vector);
	}

	free_irq(adapter->msix_entries[vector].vector, adapter);
}

/**
 * ixgbe_irq_disable - Mask off interrupt generation on the NIC
 * @adapter: board private structure
 **/
static inline void ixgbe_irq_disable(struct ixgbe_adapter *adapter)
{
	switch (adapter->hw.mac.type) {
	case ixgbe_mac_82598EB:
		IXGBE_WRITE_REG(&adapter->hw, IXGBE_EIMC, ~0);
		break;
	case ixgbe_mac_82599EB:
	case ixgbe_mac_X540:
	case ixgbe_mac_X550:
	case ixgbe_mac_X550EM_x:
	case ixgbe_mac_X550EM_a:
		IXGBE_WRITE_REG(&adapter->hw, IXGBE_EIMC, 0xFFFF0000);
		IXGBE_WRITE_REG(&adapter->hw, IXGBE_EIMC_EX(0), ~0);
		IXGBE_WRITE_REG(&adapter->hw, IXGBE_EIMC_EX(1), ~0);
		break;
	default:
		break;
	}
	IXGBE_WRITE_FLUSH(&adapter->hw);
	if (adapter->flags & IXGBE_FLAG_MSIX_ENABLED) {
		int vector;

		for (vector = 0; vector < adapter->num_q_vectors; vector++)
			synchronize_irq(adapter->msix_entries[vector].vector);

		synchronize_irq(adapter->msix_entries[vector++].vector);
	} else {
		synchronize_irq(adapter->pdev->irq);
	}
}

/**
 * ixgbe_configure_msi_and_legacy - Initialize PIN (INTA...) and MSI interrupts
 * @adapter: board private structure
 *
 **/
static void ixgbe_configure_msi_and_legacy(struct ixgbe_adapter *adapter)
{
	struct ixgbe_q_vector *q_vector = adapter->q_vector[0];

	ixgbe_write_eitr(q_vector);

	ixgbe_set_ivar(adapter, 0, 0, 0);
	ixgbe_set_ivar(adapter, 1, 0, 0);

	e_info(hw, "Legacy interrupt IVAR setup done\n");
}

/**
 * ixgbe_configure_tx_ring - Configure 8259x Tx ring after Reset
 * @adapter: board private structure
 * @ring: structure containing ring specific data
 *
 * Configure the Tx descriptor ring after a reset.
 **/
void ixgbe_configure_tx_ring(struct ixgbe_adapter *adapter,
			     struct ixgbe_ring *ring)
{
	struct ixgbe_hw *hw = &adapter->hw;
	u64 tdba = ring->dma;
	int wait_loop = 10;
	u32 txdctl = IXGBE_TXDCTL_ENABLE;
	u8 reg_idx = ring->reg_idx;

#ifdef HAVE_AF_XDP_ZC_SUPPORT
	ring->xsk_umem = NULL;
	if (ring_is_xdp(ring))
		ring->xsk_umem = ixgbe_xsk_umem(adapter, ring);
#endif
	/* disable queue to avoid issues while updating state */
	IXGBE_WRITE_REG(hw, IXGBE_TXDCTL(reg_idx), IXGBE_TXDCTL_SWFLSH);
	IXGBE_WRITE_FLUSH(hw);

	IXGBE_WRITE_REG(hw, IXGBE_TDBAL(reg_idx), tdba & DMA_BIT_MASK(32));
	IXGBE_WRITE_REG(hw, IXGBE_TDBAH(reg_idx), tdba >> 32);
	IXGBE_WRITE_REG(hw, IXGBE_TDLEN(reg_idx),
			ring->count * sizeof(union ixgbe_adv_tx_desc));

	/* disable head writeback */
	IXGBE_WRITE_REG(hw, IXGBE_TDWBAH(reg_idx), 0);
	IXGBE_WRITE_REG(hw, IXGBE_TDWBAL(reg_idx), 0);

	/* reset head and tail pointers */
	IXGBE_WRITE_REG(hw, IXGBE_TDH(reg_idx), 0);
	IXGBE_WRITE_REG(hw, IXGBE_TDT(reg_idx), 0);
	ring->tail = adapter->io_addr + IXGBE_TDT(reg_idx);

	/* reset ntu and ntc to place SW in sync with hardwdare */
	ring->next_to_clean = 0;
	ring->next_to_use = 0;

	/*
	 * set WTHRESH to encourage burst writeback, it should not be set
	 * higher than 1 when:
	 * - ITR is 0 as it could cause false TX hangs
	 * - ITR is set to > 100k int/sec and BQL is enabled
	 *
	 * In order to avoid issues WTHRESH + PTHRESH should always be equal
	 * to or less than the number of on chip descriptors, which is
	 * currently 40.
	 */
	if (!ring->q_vector || (ring->q_vector->itr < IXGBE_100K_ITR))
		txdctl |= (1 << 16);	/* WTHRESH = 1 */
	else
		txdctl |= (8 << 16);	/* WTHRESH = 8 */

	/*
	 * Setting PTHRESH to 32 both improves performance
	 * and avoids a TX hang with DFP enabled
	 */
	txdctl |= (1 << 8) |	/* HTHRESH = 1 */
		   32;		/* PTHRESH = 32 */

	/* reinitialize flowdirector state */
	if (adapter->flags & IXGBE_FLAG_FDIR_HASH_CAPABLE) {
		ring->atr_sample_rate = adapter->atr_sample_rate;
		ring->atr_count = 0;
		set_bit(__IXGBE_TX_FDIR_INIT_DONE, &ring->state);
	} else {
		ring->atr_sample_rate = 0;
	}

	/* initialize XPS */
	if (!test_and_set_bit(__IXGBE_TX_XPS_INIT_DONE, &ring->state)) {
		struct ixgbe_q_vector *q_vector = ring->q_vector;

		if (q_vector)
			netif_set_xps_queue(adapter->netdev,
					    &q_vector->affinity_mask,
					    ring->queue_index);
	}

	clear_bit(__IXGBE_HANG_CHECK_ARMED, &ring->state);

	/* reinitialize tx_buffer_info */
	memset(ring->tx_buffer_info, 0,
	       sizeof(struct ixgbe_tx_buffer) * ring->count);

	/* enable queue */
	IXGBE_WRITE_REG(hw, IXGBE_TXDCTL(reg_idx), txdctl);

	/* TXDCTL.EN will return 0 on 82598 if link is down, so skip it */
	if (hw->mac.type == ixgbe_mac_82598EB &&
	    !(IXGBE_READ_REG(hw, IXGBE_LINKS) & IXGBE_LINKS_UP))
		return;

	/* poll to verify queue is enabled */
	do {
		msleep(1);
		txdctl = IXGBE_READ_REG(hw, IXGBE_TXDCTL(reg_idx));
	} while (--wait_loop && !(txdctl & IXGBE_TXDCTL_ENABLE));
	if (!wait_loop)
		hw_dbg(hw, "Could not enable Tx Queue %d\n", reg_idx);
}

static void ixgbe_setup_mtqc(struct ixgbe_adapter *adapter)
{
	struct ixgbe_hw *hw = &adapter->hw;
	u32 rttdcs, mtqc;
	u8 tcs = netdev_get_num_tc(adapter->netdev);

	if (hw->mac.type == ixgbe_mac_82598EB)
		return;

	/* disable the arbiter while setting MTQC */
	rttdcs = IXGBE_READ_REG(hw, IXGBE_RTTDCS);
	rttdcs |= IXGBE_RTTDCS_ARBDIS;
	IXGBE_WRITE_REG(hw, IXGBE_RTTDCS, rttdcs);

	/* set transmit pool layout */
	if (adapter->flags & IXGBE_FLAG_VMDQ_ENABLED) {
		mtqc = IXGBE_MTQC_VT_ENA;
		if (tcs > 4)
			mtqc |= IXGBE_MTQC_RT_ENA | IXGBE_MTQC_8TC_8TQ;
		else if (tcs > 1)
			mtqc |= IXGBE_MTQC_RT_ENA | IXGBE_MTQC_4TC_4TQ;
		else if (adapter->ring_feature[RING_F_VMDQ].mask ==
			 IXGBE_82599_VMDQ_4Q_MASK)
			mtqc |= IXGBE_MTQC_32VF;
		else
			mtqc |= IXGBE_MTQC_64VF;
	} else {
		if (tcs > 4) {
			mtqc = IXGBE_MTQC_RT_ENA | IXGBE_MTQC_8TC_8TQ;
		} else if (tcs > 1) {
			mtqc = IXGBE_MTQC_RT_ENA | IXGBE_MTQC_4TC_4TQ;
		} else {
			u8 max_txq = adapter->num_tx_queues +
				adapter->num_xdp_queues;
			if (max_txq > 63)
				mtqc = IXGBE_MTQC_RT_ENA | IXGBE_MTQC_4TC_4TQ;
			else
				mtqc = IXGBE_MTQC_64Q_1PB;
		}
	}

	IXGBE_WRITE_REG(hw, IXGBE_MTQC, mtqc);

	/* Enable Security TX Buffer IFG for multiple pb */
	if (tcs) {
		u32 sectx = IXGBE_READ_REG(hw, IXGBE_SECTXMINIFG);
		sectx |= IXGBE_SECTX_DCB;
		IXGBE_WRITE_REG(hw, IXGBE_SECTXMINIFG, sectx);
	}

	/* re-enable the arbiter */
	rttdcs &= ~IXGBE_RTTDCS_ARBDIS;
	IXGBE_WRITE_REG(hw, IXGBE_RTTDCS, rttdcs);
}

/**
 * ixgbe_configure_tx - Configure 8259x Transmit Unit after Reset
 * @adapter: board private structure
 *
 * Configure the Tx unit of the MAC after a reset.
 **/
static void ixgbe_configure_tx(struct ixgbe_adapter *adapter)
{
	struct ixgbe_hw *hw = &adapter->hw;
	u32 dmatxctl;
	u32 i;

#ifdef CONFIG_NETDEVICES_MULTIQUEUE
	if (adapter->num_tx_queues > 1)
		adapter->netdev->features |= NETIF_F_MULTI_QUEUE;
	else
		adapter->netdev->features &= ~NETIF_F_MULTI_QUEUE;

#endif
	ixgbe_setup_mtqc(adapter);

	if (hw->mac.type != ixgbe_mac_82598EB) {
		/* DMATXCTL.EN must be before Tx queues are enabled */
		dmatxctl = IXGBE_READ_REG(hw, IXGBE_DMATXCTL);
		dmatxctl |= IXGBE_DMATXCTL_TE;
		IXGBE_WRITE_REG(hw, IXGBE_DMATXCTL, dmatxctl);
	}

	/* Setup the HW Tx Head and Tail descriptor pointers */
	for (i = 0; i < adapter->num_tx_queues; i++)
		ixgbe_configure_tx_ring(adapter, adapter->tx_ring[i]);
	for (i = 0; i < adapter->num_xdp_queues; i++)
		ixgbe_configure_tx_ring(adapter, adapter->xdp_ring[i]);
}

static void ixgbe_enable_rx_drop(struct ixgbe_adapter *adapter,
				 struct ixgbe_ring *ring)
{
	struct ixgbe_hw *hw = &adapter->hw;
	u8 reg_idx = ring->reg_idx;
	u32 srrctl = IXGBE_READ_REG(hw, IXGBE_SRRCTL(reg_idx));

	srrctl |= IXGBE_SRRCTL_DROP_EN;

	IXGBE_WRITE_REG(hw, IXGBE_SRRCTL(reg_idx), srrctl);
}

static void ixgbe_disable_rx_drop(struct ixgbe_adapter *adapter,
				  struct ixgbe_ring *ring)
{
	struct ixgbe_hw *hw = &adapter->hw;
	u8 reg_idx = ring->reg_idx;
	u32 srrctl = IXGBE_READ_REG(hw, IXGBE_SRRCTL(reg_idx));

	srrctl &= ~IXGBE_SRRCTL_DROP_EN;

	IXGBE_WRITE_REG(hw, IXGBE_SRRCTL(reg_idx), srrctl);
}

void ixgbe_set_rx_drop_en(struct ixgbe_adapter *adapter)
{
	int i;
	bool pfc_en = adapter->dcb_cfg.pfc_mode_enable;

#ifdef HAVE_DCBNL_IEEE
	if (adapter->ixgbe_ieee_pfc)
		pfc_en |= !!(adapter->ixgbe_ieee_pfc->pfc_en);

#endif
	/*
	 * We should set the drop enable bit if:
	 *  SR-IOV is enabled
	 *   or
	 *  Number of Rx queues > 1 and flow control is disabled
	 *
	 *  This allows us to avoid head of line blocking for security
	 *  and performance reasons.
	 */
	if (adapter->num_vfs || (adapter->num_rx_queues > 1 &&
	    !(adapter->hw.fc.current_mode & ixgbe_fc_tx_pause) && !pfc_en)) {
		for (i = 0; i < adapter->num_rx_queues; i++)
			ixgbe_enable_rx_drop(adapter, adapter->rx_ring[i]);
	} else {
		for (i = 0; i < adapter->num_rx_queues; i++)
			ixgbe_disable_rx_drop(adapter, adapter->rx_ring[i]);
	}
}

static void ixgbe_configure_srrctl(struct ixgbe_adapter *adapter,
				   struct ixgbe_ring *rx_ring)
{
	struct ixgbe_hw *hw = &adapter->hw;
	u32 srrctl;
	u8 reg_idx = rx_ring->reg_idx;

	if (hw->mac.type == ixgbe_mac_82598EB) {
		u16 mask = adapter->ring_feature[RING_F_RSS].mask;

		/* program one srrctl register per VMDq index */
		if (adapter->flags & IXGBE_FLAG_VMDQ_ENABLED)
			mask = adapter->ring_feature[RING_F_VMDQ].mask;

		/*
		 * if VMDq is not active we must program one srrctl register
		 * per RSS queue since we have enabled RDRXCTL.MVMEN
		 */
		reg_idx &= mask;

		/* divide by the first bit of the mask to get the indices */
		if (reg_idx)
			reg_idx /= ((~mask) + 1) & mask;
	}

	/* configure header buffer length, needed for RSC */
	srrctl = IXGBE_RX_HDR_SIZE << IXGBE_SRRCTL_BSIZEHDRSIZE_SHIFT;

	/* configure the packet buffer length */
#ifdef CONFIG_IXGBE_DISABLE_PACKET_SPLIT
	srrctl |= ALIGN(rx_ring->rx_buf_len, 1024) >>
		  IXGBE_SRRCTL_BSIZEPKT_SHIFT;
#else
#ifdef HAVE_AF_XDP_ZC_SUPPORT
	if (rx_ring->xsk_umem) {
		u32 xsk_buf_len = rx_ring->xsk_umem->chunk_size_nohr -
				  XDP_PACKET_HEADROOM;

		/* If the MAC support setting RXDCTL.RLPML, the
		 * SRRCTL[n].BSIZEPKT is set to PAGE_SIZE and
		 * RXDCTL.RLPML is set to the actual UMEM buffer
		 * size. If not, then we are stuck with a 1k buffer
		 * size resolution. In this case frames larger than
		 * the UMEM buffer size viewed in a 1k resolution will
		 * be dropped.
		 */
		if (hw->mac.type != ixgbe_mac_82599EB)
			srrctl |= PAGE_SIZE >> IXGBE_SRRCTL_BSIZEPKT_SHIFT;
		else
			srrctl |= xsk_buf_len >> IXGBE_SRRCTL_BSIZEPKT_SHIFT;
	} else if (test_bit(__IXGBE_RX_3K_BUFFER, &rx_ring->state)) {
#else
	if (test_bit(__IXGBE_RX_3K_BUFFER, &rx_ring->state)) {
#endif /* HAVE_AF_XDP_ZC_SUPPORT */
		srrctl |= IXGBE_RXBUFFER_3K >> IXGBE_SRRCTL_BSIZEPKT_SHIFT;
	} else {
		srrctl |= IXGBE_RXBUFFER_2K >> IXGBE_SRRCTL_BSIZEPKT_SHIFT;
	}
#endif /* CONFIG_IXGBE_DISABLE_PACKET_SPLIT */

	/* configure descriptor type */
	srrctl |= IXGBE_SRRCTL_DESCTYPE_ADV_ONEBUF;

	IXGBE_WRITE_REG(hw, IXGBE_SRRCTL(reg_idx), srrctl);
}

/**
 * ixgbe_rss_indir_tbl_entries - Return RSS indirection table entries
 * @adapter: device handle
 *
 *  - 82598/82599/X540:     128
 *  - X550(non-SRIOV mode): 512
 *  - X550(SRIOV mode):     64
 */
u32 ixgbe_rss_indir_tbl_entries(struct ixgbe_adapter *adapter)
{
	if (adapter->hw.mac.type < ixgbe_mac_X550)
		return 128;
	else if (adapter->flags & IXGBE_FLAG_SRIOV_ENABLED)
		return 64;
	else
		return 512;
}

/**
 * ixgbe_store_key - Write the RSS key to HW
 * @adapter: device handle
 *
 * Write the RSS key stored in adapter.rss_key to HW.
 */
void ixgbe_store_key(struct ixgbe_adapter *adapter)
{
	struct ixgbe_hw *hw = &adapter->hw;
	int i;

	for (i = 0; i < 10; i++)
		IXGBE_WRITE_REG(hw, IXGBE_RSSRK(i), adapter->rss_key[i]);
}

/**
 * ixgbe_init_rss_key - Initialize adapter RSS key
 * @adapter: device handle
 *
 * Allocates and initializes the RSS key if it is not allocated.
 **/
static inline int ixgbe_init_rss_key(struct ixgbe_adapter *adapter)
{
	u32 *rss_key;

	if (!adapter->rss_key) {
		rss_key = kzalloc(IXGBE_RSS_KEY_SIZE, GFP_KERNEL);
		if (unlikely(!rss_key))
			return -ENOMEM;

		netdev_rss_key_fill(rss_key, IXGBE_RSS_KEY_SIZE);
		adapter->rss_key = rss_key;
	}

	return 0;
}

/**
 * ixgbe_store_reta - Write the RETA table to HW
 * @adapter: device handle
 *
 * Write the RSS redirection table stored in adapter.rss_indir_tbl[] to HW.
 */
void ixgbe_store_reta(struct ixgbe_adapter *adapter)
{
	u32 i, reta_entries = ixgbe_rss_indir_tbl_entries(adapter);
	struct ixgbe_hw *hw = &adapter->hw;
	u32 reta = 0;
	u32 indices_multi;
	u8 *indir_tbl = adapter->rss_indir_tbl;

	/* Fill out the redirection table as follows:
	 *  - 82598:      8 bit wide entries containing pair of 4 bit RSS
	 *    indices.
	 *  - 82599/X540: 8 bit wide entries containing 4 bit RSS index
	 *  - X550:       8 bit wide entries containing 6 bit RSS index
	 */
	if (adapter->hw.mac.type == ixgbe_mac_82598EB)
		indices_multi = 0x11;
	else
		indices_multi = 0x1;

	/* Write redirection table to HW */
	for (i = 0; i < reta_entries; i++) {
		reta |= indices_multi * indir_tbl[i] << (i & 0x3) * 8;
		if ((i & 3) == 3) {
			if (i < 128)
				IXGBE_WRITE_REG(hw, IXGBE_RETA(i >> 2), reta);
			else
				IXGBE_WRITE_REG(hw, IXGBE_ERETA((i >> 2) - 32),
						reta);
			reta = 0;
		}
	}
}

/**
 * ixgbe_store_vfreta - Write the RETA table to HW (x550 devices in SRIOV mode)
 * @adapter: device handle
 *
 * Write the RSS redirection table stored in adapter.rss_indir_tbl[] to HW.
 */
static void ixgbe_store_vfreta(struct ixgbe_adapter *adapter)
{
	u32 i, reta_entries = ixgbe_rss_indir_tbl_entries(adapter);
	struct ixgbe_hw *hw = &adapter->hw;
	u32 vfreta = 0;
	unsigned int pf_pool = adapter->num_vfs;

	/* Write redirection table to HW */
	for (i = 0; i < reta_entries; i++) {
		vfreta |= (u32)adapter->rss_indir_tbl[i] << (i & 0x3) * 8;
		if ((i & 3) == 3) {
			IXGBE_WRITE_REG(hw, IXGBE_PFVFRETA(i >> 2, pf_pool),
					vfreta);
			vfreta = 0;
		}
	}
}

static void ixgbe_setup_reta(struct ixgbe_adapter *adapter)
{
	u32 i, j;
	u32 reta_entries = ixgbe_rss_indir_tbl_entries(adapter);
	u16 rss_i = adapter->ring_feature[RING_F_RSS].indices;

	/* Program table for at least 4 queues w/ SR-IOV so that VFs can
	 * make full use of any rings they may have.  We will use the
	 * PSRTYPE register to control how many rings we use within the PF.
	 */
	if ((adapter->flags & IXGBE_FLAG_SRIOV_ENABLED) && (rss_i < 4))
		rss_i = 4;

	/* Fill out hash function seeds */
	ixgbe_store_key(adapter);

	/* Fill out redirection table */
	memset(adapter->rss_indir_tbl, 0, sizeof(adapter->rss_indir_tbl));

	for (i = 0, j = 0; i < reta_entries; i++, j++) {
		if (j == rss_i)
			j = 0;

		adapter->rss_indir_tbl[i] = j;
	}

	ixgbe_store_reta(adapter);
}

static void ixgbe_setup_vfreta(struct ixgbe_adapter *adapter)
{
	struct ixgbe_hw *hw = &adapter->hw;
	u16 rss_i = adapter->ring_feature[RING_F_RSS].indices;
	unsigned int pf_pool = adapter->num_vfs;
	int i, j;

	/* Fill out hash function seeds */
	for (i = 0; i < 10; i++)
		IXGBE_WRITE_REG(hw, IXGBE_PFVFRSSRK(i, pf_pool),
				*(adapter->rss_key + i));

	/* Fill out the redirection table */
	for (i = 0, j = 0; i < 64; i++, j++) {
		if (j == rss_i)
			j = 0;

		adapter->rss_indir_tbl[i] = j;
	}

	ixgbe_store_vfreta(adapter);
}


static void ixgbe_setup_mrqc(struct ixgbe_adapter *adapter)
{
	struct ixgbe_hw *hw = &adapter->hw;
	u32 rxcsum;
	u32 mrqc = 0, rss_field = 0;
	u32 vfmrqc = 0;

	/* Disable indicating checksum in descriptor, enables RSS hash */
	rxcsum = IXGBE_READ_REG(hw, IXGBE_RXCSUM);
	rxcsum |= IXGBE_RXCSUM_PCSD;
	IXGBE_WRITE_REG(hw, IXGBE_RXCSUM, rxcsum);

	if (adapter->hw.mac.type == ixgbe_mac_82598EB) {
		if (adapter->ring_feature[RING_F_RSS].mask)
			mrqc = IXGBE_MRQC_RSSEN;
	} else {
		u8 tcs = netdev_get_num_tc(adapter->netdev);

		if (adapter->flags & IXGBE_FLAG_VMDQ_ENABLED) {
			if (tcs > 4)
				mrqc = IXGBE_MRQC_VMDQRT8TCEN;	/* 8 TCs */
			else if (tcs > 1)
				mrqc = IXGBE_MRQC_VMDQRT4TCEN;	/* 4 TCs */
			else if (adapter->ring_feature[RING_F_VMDQ].mask ==
				 IXGBE_82599_VMDQ_4Q_MASK)
				mrqc = IXGBE_MRQC_VMDQRSS32EN;
			else
				mrqc = IXGBE_MRQC_VMDQRSS64EN;
		} else {
			if (tcs > 4)
				mrqc = IXGBE_MRQC_RTRSS8TCEN;
			else if (tcs > 1)
				mrqc = IXGBE_MRQC_RTRSS4TCEN;
			else
				mrqc = IXGBE_MRQC_RSSEN;
		}

		/* Enable L3/L4 for Tx Switched packets */
		mrqc |= IXGBE_MRQC_L3L4TXSWEN;
	}

	/* Perform hash on these packet types */
	rss_field = IXGBE_MRQC_RSS_FIELD_IPV4 |
		     IXGBE_MRQC_RSS_FIELD_IPV4_TCP |
		     IXGBE_MRQC_RSS_FIELD_IPV6 |
		     IXGBE_MRQC_RSS_FIELD_IPV6_TCP;

	if (adapter->flags2 & IXGBE_FLAG2_RSS_FIELD_IPV4_UDP)
		rss_field |= IXGBE_MRQC_RSS_FIELD_IPV4_UDP;
	if (adapter->flags2 & IXGBE_FLAG2_RSS_FIELD_IPV6_UDP)
		rss_field |= IXGBE_MRQC_RSS_FIELD_IPV6_UDP;

	if ((hw->mac.type >= ixgbe_mac_X550) &&
	    (adapter->flags & IXGBE_FLAG_SRIOV_ENABLED)) {
		unsigned int pf_pool = adapter->num_vfs;

		/* Enable VF RSS mode */
		mrqc |= IXGBE_MRQC_MULTIPLE_RSS;
		IXGBE_WRITE_REG(hw, IXGBE_MRQC, mrqc);

		/* Setup RSS through the VF registers */
		ixgbe_setup_vfreta(adapter);
		vfmrqc = IXGBE_MRQC_RSSEN;
		vfmrqc |= rss_field;
		IXGBE_WRITE_REG(hw, IXGBE_PFVFMRQC(pf_pool), vfmrqc);
	} else {
		ixgbe_setup_reta(adapter);
		mrqc |= rss_field;
		IXGBE_WRITE_REG(hw, IXGBE_MRQC, mrqc);
	}
}

/**
 * ixgbe_clear_rscctl - disable RSC for the indicated ring
 * @adapter: address of board private structure
 * @ring: structure containing ring specific data
 **/
void ixgbe_clear_rscctl(struct ixgbe_adapter *adapter,
			struct ixgbe_ring *ring)
{
	struct ixgbe_hw *hw = &adapter->hw;
	u32 rscctrl;
	u8 reg_idx = ring->reg_idx;

	rscctrl = IXGBE_READ_REG(hw, IXGBE_RSCCTL(reg_idx));
	rscctrl &= ~IXGBE_RSCCTL_RSCEN;
	IXGBE_WRITE_REG(hw, IXGBE_RSCCTL(reg_idx), rscctrl);

	clear_ring_rsc_enabled(ring);
}

/**
 * ixgbe_configure_rscctl - enable RSC for the indicated ring
 * @adapter:    address of board private structure
 * @ring: structure containing ring specific data
 **/
void ixgbe_configure_rscctl(struct ixgbe_adapter *adapter,
			    struct ixgbe_ring *ring)
{
	struct ixgbe_hw *hw = &adapter->hw;
	u32 rscctrl;
	u8 reg_idx = ring->reg_idx;

	if (!ring_is_rsc_enabled(ring))
		return;

	rscctrl = IXGBE_READ_REG(hw, IXGBE_RSCCTL(reg_idx));
	rscctrl |= IXGBE_RSCCTL_RSCEN;
	/*
	 * we must limit the number of descriptors so that the
	 * total size of max desc * buf_len is not greater
	 * than 65536
	 */
#ifndef CONFIG_IXGBE_DISABLE_PACKET_SPLIT
#if (MAX_SKB_FRAGS >= 16)
	rscctrl |= IXGBE_RSCCTL_MAXDESC_16;
#elif (MAX_SKB_FRAGS >= 8)
	rscctrl |= IXGBE_RSCCTL_MAXDESC_8;
#elif (MAX_SKB_FRAGS >= 4)
	rscctrl |= IXGBE_RSCCTL_MAXDESC_4;
#else
	rscctrl |= IXGBE_RSCCTL_MAXDESC_1;
#endif
#else /* CONFIG_IXGBE_DISABLE_PACKET_SPLIT */
	if (ring->rx_buf_len <= IXGBE_RXBUFFER_4K)
		rscctrl |= IXGBE_RSCCTL_MAXDESC_16;
	else if (ring->rx_buf_len <= IXGBE_RXBUFFER_8K)
		rscctrl |= IXGBE_RSCCTL_MAXDESC_8;
	else
		rscctrl |= IXGBE_RSCCTL_MAXDESC_4;
#endif /* !CONFIG_IXGBE_DISABLE_PACKET_SPLIT */
	IXGBE_WRITE_REG(hw, IXGBE_RSCCTL(reg_idx), rscctrl);
}

static void ixgbe_rx_desc_queue_enable(struct ixgbe_adapter *adapter,
				       struct ixgbe_ring *ring)
{
	struct ixgbe_hw *hw = &adapter->hw;
	int wait_loop = IXGBE_MAX_RX_DESC_POLL;
	u32 rxdctl;
	u8 reg_idx = ring->reg_idx;

	if (IXGBE_REMOVED(hw->hw_addr))
		return;
	/* RXDCTL.EN will return 0 on 82598 if link is down, so skip it */
	if (hw->mac.type == ixgbe_mac_82598EB &&
	    !(IXGBE_READ_REG(hw, IXGBE_LINKS) & IXGBE_LINKS_UP))
		return;

	do {
		msleep(1);
		rxdctl = IXGBE_READ_REG(hw, IXGBE_RXDCTL(reg_idx));
	} while (--wait_loop && !(rxdctl & IXGBE_RXDCTL_ENABLE));

	if (!wait_loop) {
		e_err(drv, "RXDCTL.ENABLE on Rx queue %d "
		      "not set within the polling period\n", reg_idx);
	}
}

void ixgbe_configure_rx_ring(struct ixgbe_adapter *adapter,
			     struct ixgbe_ring *ring)
{
	struct ixgbe_hw *hw = &adapter->hw;
	union ixgbe_adv_rx_desc *rx_desc;
	u64 rdba = ring->dma;
	u32 rxdctl;
	u8 reg_idx = ring->reg_idx;

#ifdef HAVE_AF_XDP_ZC_SUPPORT
	xdp_rxq_info_unreg_mem_model(&ring->xdp_rxq);
	ring->xsk_umem = ixgbe_xsk_umem(adapter, ring);
	if (ring->xsk_umem) {
		ring->zca.free = ixgbe_zca_free;
		WARN_ON(xdp_rxq_info_reg_mem_model(&ring->xdp_rxq,
						   MEM_TYPE_ZERO_COPY,
						   &ring->zca));

	} else {
		WARN_ON(xdp_rxq_info_reg_mem_model(&ring->xdp_rxq,
						   MEM_TYPE_PAGE_SHARED, NULL));
	}

#endif
	/* disable queue to avoid use of these values while updating state */
	rxdctl = IXGBE_READ_REG(hw, IXGBE_RXDCTL(reg_idx));
	rxdctl &= ~IXGBE_RXDCTL_ENABLE;

	/* write value back with RXDCTL.ENABLE bit cleared */
	IXGBE_WRITE_REG(hw, IXGBE_RXDCTL(reg_idx), rxdctl);
	IXGBE_WRITE_FLUSH(hw);

	IXGBE_WRITE_REG(hw, IXGBE_RDBAL(reg_idx), rdba & DMA_BIT_MASK(32));
	IXGBE_WRITE_REG(hw, IXGBE_RDBAH(reg_idx), rdba >> 32);
	IXGBE_WRITE_REG(hw, IXGBE_RDLEN(reg_idx),
			ring->count * sizeof(union ixgbe_adv_rx_desc));
	/* Force flushing of IXGBE_RDLEN to prevent MDD */
	IXGBE_WRITE_FLUSH(hw);

	/* reset head and tail pointers */
	IXGBE_WRITE_REG(hw, IXGBE_RDH(reg_idx), 0);
	IXGBE_WRITE_REG(hw, IXGBE_RDT(reg_idx), 0);
	ring->tail = adapter->io_addr + IXGBE_RDT(reg_idx);

	/* reset ntu and ntc to place SW in sync with hardwdare */
	ring->next_to_clean = 0;
	ring->next_to_use = 0;
#ifndef CONFIG_IXGBE_DISABLE_PACKET_SPLIT
	ring->next_to_alloc = 0;
#endif

	ixgbe_configure_srrctl(adapter, ring);
        /* In ESX, RSCCTL configuration is done by on demand */
	ixgbe_configure_rscctl(adapter, ring);

	switch (hw->mac.type) {
	case ixgbe_mac_82598EB:
		/*
		 * enable cache line friendly hardware writes:
		 * PTHRESH=32 descriptors (half the internal cache),
		 * this also removes ugly rx_no_buffer_count increment
		 * HTHRESH=4 descriptors (to minimize latency on fetch)
		 * WTHRESH=8 burst writeback up to two cache lines
		 */
		rxdctl &= ~0x3FFFFF;
		rxdctl |=  0x080420;
		break;
	case ixgbe_mac_X540:
	case ixgbe_mac_X550:
	case ixgbe_mac_X550EM_x:
	case ixgbe_mac_X550EM_a:
#ifndef CONFIG_IXGBE_DISABLE_PACKET_SPLIT
#if (PAGE_SIZE < 8192)
		rxdctl &= ~(IXGBE_RXDCTL_RLPMLMASK |
			    IXGBE_RXDCTL_RLPML_EN);

		/* Limit the maximum frame size so we don't overrun the skb */
		if (ring_uses_build_skb(ring) &&
		    !test_bit(__IXGBE_RX_3K_BUFFER, &ring->state))
			rxdctl |= IXGBE_MAX_2K_FRAME_BUILD_SKB |
				  IXGBE_RXDCTL_RLPML_EN;
#endif
#else /* CONFIG_IXGBE_DISABLE_PACKET_SPLIT */
		/* If operating in IOV mode set RLPML */
		if (!(adapter->flags & IXGBE_FLAG_SRIOV_ENABLED))
			break;
		rxdctl |= ring->rx_buf_len | IXGBE_RXDCTL_RLPML_EN;
#endif /* CONFIG_IXGBE_DISABLE_PACKET_SPLIT */
		break;
	default:
		break;
	}

#ifdef HAVE_AF_XDP_ZC_SUPPORT
	if (ring->xsk_umem && hw->mac.type != ixgbe_mac_82599EB) {
		u32 xsk_buf_len = ring->xsk_umem->chunk_size_nohr -
				  XDP_PACKET_HEADROOM;

		rxdctl &= ~(IXGBE_RXDCTL_RLPMLMASK |
			    IXGBE_RXDCTL_RLPML_EN);
		rxdctl |= xsk_buf_len | IXGBE_RXDCTL_RLPML_EN;

		ring->rx_buf_len = xsk_buf_len;
	}

#endif
	/* initialize rx_buffer_info */
	memset(ring->rx_buffer_info, 0,
	       sizeof(struct ixgbe_rx_buffer) * ring->count);

	/* initialize Rx descriptor 0 */
	rx_desc = IXGBE_RX_DESC(ring, 0);
	rx_desc->wb.upper.length = 0;

	/* enable receive descriptor ring */
	rxdctl |= IXGBE_RXDCTL_ENABLE;
	IXGBE_WRITE_REG(hw, IXGBE_RXDCTL(reg_idx), rxdctl);

	ixgbe_rx_desc_queue_enable(adapter, ring);
#ifdef HAVE_AF_XDP_ZC_SUPPORT
	if (ring->xsk_umem)
		ixgbe_alloc_rx_buffers_zc(ring, ixgbe_desc_unused(ring));
	else
		ixgbe_alloc_rx_buffers(ring, ixgbe_desc_unused(ring));
#else
	ixgbe_alloc_rx_buffers(ring, ixgbe_desc_unused(ring));
#endif /* HAVE_AF_XDP_ZC_SUPPORT */
}

static void ixgbe_setup_psrtype(struct ixgbe_adapter *adapter)
{
	struct ixgbe_hw *hw = &adapter->hw;
	int rss_i = adapter->ring_feature[RING_F_RSS].indices;
	int p;

	/* PSRTYPE must be initialized in non 82598 adapters */
	u32 psrtype = IXGBE_PSRTYPE_TCPHDR |
		      IXGBE_PSRTYPE_UDPHDR |
		      IXGBE_PSRTYPE_IPV4HDR |
		      IXGBE_PSRTYPE_L2HDR |
		      IXGBE_PSRTYPE_IPV6HDR;

	if (hw->mac.type == ixgbe_mac_82598EB)
		return;

	if (rss_i > 3)
		psrtype |= 2u << 29;
	else if (rss_i > 1)
		psrtype |= 1u << 29;

	for (p = 0; p < adapter->num_rx_pools; p++)
		IXGBE_WRITE_REG(hw, IXGBE_PSRTYPE(VMDQ_P(p)), psrtype);
}

/**
 * ixgbe_configure_bridge_mode - common settings for configuring bridge mode
 * @adapter: the private structure
 *
 * This function's purpose is to remove code duplication and configure some
 * settings require to switch bridge modes.
 **/
static void ixgbe_configure_bridge_mode(struct ixgbe_adapter *adapter)
{
	struct ixgbe_hw * hw = &adapter->hw;
	unsigned int p;
	u32 vmdctl;

	if (adapter->flags & IXGBE_FLAG_SRIOV_VEPA_BRIDGE_MODE) {
		/* disable Tx loopback, rely on switch hairpin mode */
		IXGBE_WRITE_REG(hw, IXGBE_PFDTXGSWC, 0);

		/* must enable Rx switching replication to allow multicast
		 * packet reception on all VFs, and to enable source address
		 * pruning.
		 */
		vmdctl = IXGBE_READ_REG(hw, IXGBE_VMD_CTL);
		vmdctl |= IXGBE_VT_CTL_REPLEN;
		IXGBE_WRITE_REG(hw, IXGBE_VMD_CTL, vmdctl);

		/* enable Rx source address pruning. Note, this requires
		 * replication to be enabled or else it does nothing.
		 */
		for (p = 0; p < (adapter->num_vfs + adapter->num_rx_pools); p++) {
			if (hw->mac.ops.set_source_address_pruning)
				hw->mac.ops.set_source_address_pruning(hw,
								       true,
								       p);
		}
	} else {
		/* enable Tx loopback for internal VF/PF communication */
		IXGBE_WRITE_REG(hw, IXGBE_PFDTXGSWC, IXGBE_PFDTXGSWC_VT_LBEN);

		/* disable Rx switching replication unless we have SR-IOV
		 * virtual functions
		 */
		vmdctl = IXGBE_READ_REG(hw, IXGBE_VMD_CTL);
		if (!adapter->num_vfs)
			vmdctl &= ~IXGBE_VT_CTL_REPLEN;
		IXGBE_WRITE_REG(hw, IXGBE_VMD_CTL, vmdctl);

		/* disable Rx source address pruning, since we don't expect to
		 * be receiving external loopback of our transmitted frames.
		 */
		for (p = 0; p < (adapter->num_vfs + adapter->num_rx_pools); p++) {
			if (hw->mac.ops.set_source_address_pruning)
				hw->mac.ops.set_source_address_pruning(hw,
								       false,
								       p);
		}
	}

}

static void ixgbe_configure_virtualization(struct ixgbe_adapter *adapter)
{
	struct ixgbe_hw *hw = &adapter->hw;
	u32 reg_offset, vf_shift, vmolr;
	u32 gcr_ext, vmdctl;
	int i;

	if (!(adapter->flags & IXGBE_FLAG_VMDQ_ENABLED))
		return;

	switch (hw->mac.type) {
	case ixgbe_mac_82598EB:
		vmdctl = IXGBE_READ_REG(hw, IXGBE_VMD_CTL);
		vmdctl |= IXGBE_VMD_CTL_VMDQ_EN;
		IXGBE_WRITE_REG(hw, IXGBE_VMD_CTL, vmdctl);
		break;
	case ixgbe_mac_82599EB:
	case ixgbe_mac_X540:
	case ixgbe_mac_X550:
	case ixgbe_mac_X550EM_x:
	case ixgbe_mac_X550EM_a:
		vmdctl = IXGBE_READ_REG(hw, IXGBE_VT_CTL);
		vmdctl |= IXGBE_VT_CTL_VT_ENABLE;
		vmdctl &= ~IXGBE_VT_CTL_POOL_MASK;
		vmdctl |= VMDQ_P(0) << IXGBE_VT_CTL_POOL_SHIFT;
		if (adapter->num_vfs)
			vmdctl |= IXGBE_VT_CTL_REPLEN;
		IXGBE_WRITE_REG(hw, IXGBE_VT_CTL, vmdctl);

		for (i = 1; i < adapter->num_rx_pools; i++) {
			int pool = VMDQ_P(i);

			/* accept untagged packets until a vlan tag is
			 * specifically set for the VMDQ queue/pool
			 */
			vmolr = IXGBE_READ_REG(hw, IXGBE_VMOLR(pool));
			vmolr |= IXGBE_VMOLR_AUPE;
			IXGBE_WRITE_REG(hw, IXGBE_VMOLR(pool), vmolr);
		}

		vf_shift = VMDQ_P(0) % 32;
		reg_offset = (VMDQ_P(0) >= 32) ? 1 : 0;

		/* Enable only the PF pools for Tx/Rx */
		IXGBE_WRITE_REG(hw, IXGBE_VFRE(reg_offset), (~0) << vf_shift);
		IXGBE_WRITE_REG(hw, IXGBE_VFRE(reg_offset ^ 1), reg_offset - 1);
		IXGBE_WRITE_REG(hw, IXGBE_VFTE(reg_offset), (~0) << vf_shift);
		IXGBE_WRITE_REG(hw, IXGBE_VFTE(reg_offset ^ 1), reg_offset - 1);

		/* clear VLAN promisc flag so VFTA
		 * will be updated if necessary
		 */
		adapter->flags2 &= ~IXGBE_FLAG2_VLAN_PROMISC;
		break;
	default:
		break;
	}

	if (!(adapter->flags & IXGBE_FLAG_SRIOV_ENABLED))
		return;

	/*
	 * Set up VF register offsets for selected VT Mode,
	 * i.e. 32 or 64 VFs for SR-IOV
	 */
	switch (adapter->ring_feature[RING_F_VMDQ].mask) {
	case IXGBE_82599_VMDQ_8Q_MASK:
		gcr_ext = IXGBE_GCR_EXT_VT_MODE_16;
		break;
	case IXGBE_82599_VMDQ_4Q_MASK:
		gcr_ext = IXGBE_GCR_EXT_VT_MODE_32;
		break;
	default:
		gcr_ext = IXGBE_GCR_EXT_VT_MODE_64;
		break;
	}

	IXGBE_WRITE_REG(hw, IXGBE_GCR_EXT, gcr_ext);

	/* configure default bridge settings */
	ixgbe_configure_bridge_mode(adapter);
#if IS_ENABLED(CONFIG_PCI_IOV)
	for (i = 0; i < adapter->num_vfs; i++) {
		/* configure spoof checking */
		ixgbe_ndo_set_vf_spoofchk(adapter->netdev, i,
					  adapter->vfinfo[i].spoofchk_enabled);

#ifdef HAVE_NDO_SET_VF_RSS_QUERY_EN
		/* Enable/Disable RSS query feature  */
		ixgbe_ndo_set_vf_rss_query_en(adapter->netdev, i,
					adapter->vfinfo[i].rss_query_enabled);
#endif /* HAVE_NDO_SET_VF_RSS_QUERY_EN */
	}
#endif /* CONFIG_PCI_IOV */
}

static void ixgbe_set_rx_buffer_len(struct ixgbe_adapter *adapter)
{
	struct ixgbe_hw *hw = &adapter->hw;
	struct net_device *netdev = adapter->netdev;
	int max_frame = netdev->mtu + ETH_HLEN + ETH_FCS_LEN;
	struct ixgbe_ring *rx_ring;
	int i;
	u32 mhadd, hlreg0;
#ifdef CONFIG_IXGBE_DISABLE_PACKET_SPLIT
	int rx_buf_len;
#endif

	switch (hw->mac.type) {
	case ixgbe_mac_X550:
	case ixgbe_mac_X550EM_x:
	case ixgbe_mac_X550EM_a:
		max_frame += IXGBE_TS_HDR_LEN;
	default:
		break;
	}

#if IS_ENABLED(CONFIG_FCOE)
	/* adjust max frame to be able to do baby jumbo for FCoE */
	if ((adapter->flags & IXGBE_FLAG_FCOE_ENABLED) &&
	    (max_frame < IXGBE_FCOE_JUMBO_FRAME_SIZE))
		max_frame = IXGBE_FCOE_JUMBO_FRAME_SIZE;
#endif /* CONFIG_FCOE */

	/* adjust max frame to be at least the size of a standard frame */
	if (max_frame < (ETH_FRAME_LEN + ETH_FCS_LEN))
		max_frame = (ETH_FRAME_LEN + ETH_FCS_LEN);

	mhadd = IXGBE_READ_REG(hw, IXGBE_MHADD);
	if (max_frame != (mhadd >> IXGBE_MHADD_MFS_SHIFT)) {
		mhadd &= ~IXGBE_MHADD_MFS_MASK;
		mhadd |= max_frame << IXGBE_MHADD_MFS_SHIFT;

		IXGBE_WRITE_REG(hw, IXGBE_MHADD, mhadd);
	}

#ifdef CONFIG_IXGBE_DISABLE_PACKET_SPLIT
		/* MHADD will allow an extra 4 bytes past for vlan tagged frames */
		max_frame += VLAN_HLEN;

	if (!(adapter->flags2 & IXGBE_FLAG2_RSC_ENABLED) &&
	    (max_frame <= MAXIMUM_ETHERNET_VLAN_SIZE)) {
		rx_buf_len = MAXIMUM_ETHERNET_VLAN_SIZE;
	/*
	 * Make best use of allocation by using all but 1K of a
	 * power of 2 allocation that will be used for skb->head.
	 */
	} else if (max_frame <= IXGBE_RXBUFFER_3K) {
		rx_buf_len = IXGBE_RXBUFFER_3K;
	} else if (max_frame <= IXGBE_RXBUFFER_7K) {
		rx_buf_len = IXGBE_RXBUFFER_7K;
	} else if (max_frame <= IXGBE_RXBUFFER_15K) {
		rx_buf_len = IXGBE_RXBUFFER_15K;
	} else {
		rx_buf_len = IXGBE_MAX_RXBUFFER;
	}

#endif /* CONFIG_IXGBE_DISABLE_PACKET_SPLIT */
	hlreg0 = IXGBE_READ_REG(hw, IXGBE_HLREG0);
	/* set jumbo enable since MHADD.MFS is keeping size locked at
	 * max_frame
	 */
	hlreg0 |= IXGBE_HLREG0_JUMBOEN;
	IXGBE_WRITE_REG(hw, IXGBE_HLREG0, hlreg0);

	/*
	 * Setup the HW Rx Head and Tail Descriptor Pointers and
	 * the Base and Length of the Rx Descriptor Ring
	 */
	for (i = 0; i < adapter->num_rx_queues; i++) {
		rx_ring = adapter->rx_ring[i];

		clear_ring_rsc_enabled(rx_ring);
		if (adapter->flags2 & IXGBE_FLAG2_RSC_ENABLED)
			set_ring_rsc_enabled(rx_ring);

#ifndef CONFIG_IXGBE_DISABLE_PACKET_SPLIT
		clear_bit(__IXGBE_RX_3K_BUFFER, &rx_ring->state);
		clear_bit(__IXGBE_RX_BUILD_SKB_ENABLED, &rx_ring->state);
#if IS_ENABLED(CONFIG_FCOE)

		if (test_bit(__IXGBE_RX_FCOE, &rx_ring->state))
			set_bit(__IXGBE_RX_3K_BUFFER, &rx_ring->state);
#endif
#ifdef HAVE_SWIOTLB_SKIP_CPU_SYNC

		if (adapter->flags2 & IXGBE_FLAG2_RX_LEGACY)
			continue;

		set_bit(__IXGBE_RX_BUILD_SKB_ENABLED, &rx_ring->state);

#if (PAGE_SIZE < 8192)
		if (adapter->flags2 & IXGBE_FLAG2_RSC_ENABLED)
			set_bit(__IXGBE_RX_3K_BUFFER, &rx_ring->state);

		if (IXGBE_2K_TOO_SMALL_WITH_PADDING ||
		    (max_frame > (ETH_FRAME_LEN + ETH_FCS_LEN)))
			set_bit(__IXGBE_RX_3K_BUFFER, &rx_ring->state);
#endif
#else /* !HAVE_SWIOTLB_SKIP_CPU_SYNC */

		adapter->flags2 |= IXGBE_FLAG2_RX_LEGACY;
#endif /* !HAVE_SWIOTLB_SKIP_CPU_SYNC */
#else /* CONFIG_IXGBE_DISABLE_PACKET_SPLIT */

		rx_ring->rx_buf_len = rx_buf_len;
#if IS_ENABLED(CONFIG_FCOE)

		if (test_bit(__IXGBE_RX_FCOE, &rx_ring->state) &&
		    (rx_buf_len < IXGBE_FCOE_JUMBO_FRAME_SIZE))
			rx_ring->rx_buf_len = IXGBE_FCOE_JUMBO_FRAME_SIZE;
#endif /* CONFIG_FCOE */
#endif /* CONFIG_IXGBE_DISABLE_PACKET_SPLIT */
	}
}

static void ixgbe_setup_rdrxctl(struct ixgbe_adapter *adapter)
{
	struct ixgbe_hw *hw = &adapter->hw;
	u32 rdrxctl = IXGBE_READ_REG(hw, IXGBE_RDRXCTL);

	switch (hw->mac.type) {
	case ixgbe_mac_X550:
	case ixgbe_mac_X550EM_x:
	case ixgbe_mac_X550EM_a:
		if (adapter->num_vfs)
			rdrxctl |= IXGBE_RDRXCTL_PSP;
		/* fall through */
	case ixgbe_mac_82599EB:
	case ixgbe_mac_X540:
		/* Disable RSC for ACK packets */
		IXGBE_WRITE_REG(hw, IXGBE_RSCDBU,
		   (IXGBE_RSCDBU_RSCACKDIS | IXGBE_READ_REG(hw, IXGBE_RSCDBU)));
		rdrxctl &= ~IXGBE_RDRXCTL_RSCFRSTSIZE;
		/* hardware requires some bits to be set by default */
		rdrxctl |= (IXGBE_RDRXCTL_RSCACKC | IXGBE_RDRXCTL_FCOE_WRFIX);
		rdrxctl |= IXGBE_RDRXCTL_CRCSTRIP;
		break;
	case ixgbe_mac_82598EB:
		/*
		 * For VMDq support of different descriptor types or
		 * buffer sizes through the use of multiple SRRCTL
		 * registers, RDRXCTL.MVMEN must be set to 1
		 *
		 * also, the manual doesn't mention it clearly but DCA hints
		 * will only use queue 0's tags unless this bit is set.  Side
		 * effects of setting this bit are only that SRRCTL must be
		 * fully programmed [0..15]
		 */
		rdrxctl |= IXGBE_RDRXCTL_MVMEN;
		break;
	default:
		/* We should do nothing since we don't know this hardware */
		return;
	}

	IXGBE_WRITE_REG(hw, IXGBE_RDRXCTL, rdrxctl);
}

/**
 * ixgbe_configure_rx - Configure 8259x Receive Unit after Reset
 * @adapter: board private structure
 *
 * Configure the Rx unit of the MAC after a reset.
 **/
static void ixgbe_configure_rx(struct ixgbe_adapter *adapter)
{
	struct ixgbe_hw *hw = &adapter->hw;
	int i;
	u32 rxctrl, rfctl;

	/* disable receives while setting up the descriptors */
	ixgbe_disable_rx(hw);

	ixgbe_setup_psrtype(adapter);
	ixgbe_setup_rdrxctl(adapter);

	/* RSC Setup */
	rfctl = IXGBE_READ_REG(hw, IXGBE_RFCTL);
	rfctl &= ~IXGBE_RFCTL_RSC_DIS;
	if (!(adapter->flags2 & IXGBE_FLAG2_RSC_ENABLED))
		rfctl |= IXGBE_RFCTL_RSC_DIS;

	/* disable NFS filtering */
	rfctl |= (IXGBE_RFCTL_NFSW_DIS | IXGBE_RFCTL_NFSR_DIS);
	IXGBE_WRITE_REG(hw, IXGBE_RFCTL, rfctl);

	/* Program registers for the distribution of queues */
	ixgbe_setup_mrqc(adapter);

	/* set_rx_buffer_len must be called before ring initialization */
	ixgbe_set_rx_buffer_len(adapter);

	/*
	 * Setup the HW Rx Head and Tail Descriptor Pointers and
	 * the Base and Length of the Rx Descriptor Ring
	 */
	for (i = 0; i < adapter->num_rx_queues; i++)
		ixgbe_configure_rx_ring(adapter, adapter->rx_ring[i]);

	rxctrl = IXGBE_READ_REG(hw, IXGBE_RXCTRL);
	/* disable drop enable for 82598 parts */
	if (hw->mac.type == ixgbe_mac_82598EB)
		rxctrl |= IXGBE_RXCTRL_DMBYPS;

	/* enable all receives */
	rxctrl |= IXGBE_RXCTRL_RXEN;
	hw->mac.ops.enable_rx_dma(hw, rxctrl);
}

#if defined(NETIF_F_HW_VLAN_TX) || defined(NETIF_F_HW_VLAN_CTAG_TX)
#ifdef HAVE_INT_NDO_VLAN_RX_ADD_VID
#ifdef NETIF_F_HW_VLAN_CTAG_TX
static int ixgbe_vlan_rx_add_vid(struct net_device *netdev,
				 __always_unused __be16 proto, u16 vid)
#else /* !NETIF_F_HW_VLAN_CTAG_TX */
static int ixgbe_vlan_rx_add_vid(struct net_device *netdev, u16 vid)
#endif /* NETIF_F_HW_VLAN_CTAG_TX */
#else /* !HAVE_INT_NDO_VLAN_RX_ADD_VID */
static void ixgbe_vlan_rx_add_vid(struct net_device *netdev, u16 vid)
#endif /* HAVE_INT_NDO_VLAN_RX_ADD_VID */
{
	struct ixgbe_adapter *adapter = netdev_priv(netdev);
	struct ixgbe_hw *hw = &adapter->hw;
	int pool_ndx = VMDQ_P(0);

	/* add VID to filter table */
	if (hw->mac.ops.set_vfta) {
#ifndef HAVE_VLAN_RX_REGISTER
		if (vid < VLAN_N_VID)
			set_bit(vid, adapter->active_vlans);
#endif

		if (!vid || !(adapter->flags2 & IXGBE_FLAG2_VLAN_PROMISC))
			hw->mac.ops.set_vfta(hw, vid, pool_ndx, true, !!vid);

		if (adapter->flags & IXGBE_FLAG_VMDQ_ENABLED &&
		    adapter->hw.mac.type != ixgbe_mac_82598EB) {
			int i;

			/* enable vlan id for all pools */
			for (i = 1; i < adapter->num_rx_pools; i++)
				hw->mac.ops.set_vfta(hw, vid, VMDQ_P(i), true,
#ifdef HAVE_VLAN_RX_REGISTER
						     false);
#else
						     true);
#endif
		}
	}
#ifndef HAVE_NETDEV_VLAN_FEATURES

	/*
	 * Copy feature flags from netdev to the vlan netdev for this vid.
	 * This allows things like TSO to bubble down to our vlan device.
	 * Some vlans, such as VLAN 0 for DCB will not have a v_netdev so
	 * we will not have a netdev that needs updating.
	 */
	if (adapter->vlgrp) {
		struct vlan_group *vlgrp = adapter->vlgrp;
		struct net_device *v_netdev = vlan_group_get_device(vlgrp, vid);
		if (v_netdev) {
			v_netdev->features |= netdev->features;
			vlan_group_set_device(vlgrp, vid, v_netdev);
		}
	}
#endif /* HAVE_NETDEV_VLAN_FEATURES */
#ifdef HAVE_INT_NDO_VLAN_RX_ADD_VID
	return 0;
#endif
}

#if defined(HAVE_VLAN_RX_REGISTER) && defined(CONFIG_PCI_IOV)
int ixgbe_find_vlvf_entry(struct ixgbe_hw *hw, u32 vlan)
#else
static int ixgbe_find_vlvf_entry(struct ixgbe_hw *hw, u32 vlan)
#endif
{
	u32 vlvf;
	int idx;

	/* short cut the special case */
	if (vlan == 0)
		return 0;

	/* Search for the vlan id in the VLVF entries */
	for (idx = IXGBE_VLVF_ENTRIES; --idx;) {
		vlvf = IXGBE_READ_REG(hw, IXGBE_VLVF(idx));
		if ((vlvf & VLAN_VID_MASK) == vlan)
			break;
	}

	return idx;
}

void ixgbe_update_pf_promisc_vlvf(struct ixgbe_adapter *adapter, u32 vid)
{
	struct ixgbe_hw *hw = &adapter->hw;
	u32 bits, word;
	int idx;

	idx = ixgbe_find_vlvf_entry(hw, vid);
	if (!idx)
		return;

	/* See if any other pools are set for this VLAN filter
	 * entry other than the PF.
	 */
	word = idx * 2 + (VMDQ_P(0) / 32);
	bits = ~(1 << (VMDQ_P(0)) % 32);
	bits &= IXGBE_READ_REG(hw, IXGBE_VLVFB(word));

	/* Disable the filter so this falls into the default pool. */
	if (!bits && !IXGBE_READ_REG(hw, IXGBE_VLVFB(word ^ 1))) {
		if (!(adapter->flags2 & IXGBE_FLAG2_VLAN_PROMISC))
			IXGBE_WRITE_REG(hw, IXGBE_VLVFB(word), 0);
		IXGBE_WRITE_REG(hw, IXGBE_VLVF(idx), 0);
	}
}

#ifdef HAVE_INT_NDO_VLAN_RX_ADD_VID
#ifdef NETIF_F_HW_VLAN_CTAG_RX
static int ixgbe_vlan_rx_kill_vid(struct net_device *netdev,
				  __always_unused __be16 proto, u16 vid)
#else /* !NETIF_F_HW_VLAN_CTAG_RX */
static int ixgbe_vlan_rx_kill_vid(struct net_device *netdev, u16 vid)
#endif /* NETIF_F_HW_VLAN_CTAG_RX */
#else
static void ixgbe_vlan_rx_kill_vid(struct net_device *netdev, u16 vid)
#endif
{
	struct ixgbe_adapter *adapter = netdev_priv(netdev);
	struct ixgbe_hw *hw = &adapter->hw;
	int pool_ndx = VMDQ_P(0);

	/* User is not allowed to remove vlan ID 0 */
	if (!vid)
#ifdef HAVE_INT_NDO_VLAN_RX_ADD_VID
		return 0;
#else
		return;
#endif

#ifdef HAVE_VLAN_RX_REGISTER
	if (!test_bit(__IXGBE_DOWN, &adapter->state))
		ixgbe_irq_disable(adapter);

	vlan_group_set_device(adapter->vlgrp, vid, NULL);

	if (!test_bit(__IXGBE_DOWN, &adapter->state))
		ixgbe_irq_enable(adapter, true, true);

#endif /* HAVE_VLAN_RX_REGISTER */
	/* remove VID from filter table */
	if (hw->mac.ops.set_vfta) {
		if (vid && !(adapter->flags2 & IXGBE_FLAG2_VLAN_PROMISC))
			hw->mac.ops.set_vfta(hw, vid, pool_ndx, false, true);

		if (adapter->flags & IXGBE_FLAG_VMDQ_ENABLED &&
		    adapter->hw.mac.type != ixgbe_mac_82598EB) {
			int i;

			/* remove vlan id from all pools */
			for (i = 1; i < adapter->num_rx_pools; i++)
				hw->mac.ops.set_vfta(hw, vid, VMDQ_P(i), false,
						     true);
		}
	}
#ifndef HAVE_VLAN_RX_REGISTER

	clear_bit(vid, adapter->active_vlans);
#endif
#ifdef HAVE_INT_NDO_VLAN_RX_ADD_VID
	return 0;
#endif
}

#ifdef HAVE_8021P_SUPPORT
/**
 * ixgbe_vlan_strip_disable - helper to disable vlan tag stripping
 * @adapter: driver data
 */
void ixgbe_vlan_strip_disable(struct ixgbe_adapter *adapter)
{
	struct ixgbe_hw *hw = &adapter->hw;
	u32 vlnctrl;
	int i, j;

	/* leave vlan tag stripping enabled for DCB */
	if (adapter->flags & IXGBE_FLAG_DCB_ENABLED)
		return;

	switch (hw->mac.type) {
	case ixgbe_mac_82598EB:
		vlnctrl = IXGBE_READ_REG(hw, IXGBE_VLNCTRL);
		vlnctrl &= ~IXGBE_VLNCTRL_VME;
		IXGBE_WRITE_REG(hw, IXGBE_VLNCTRL, vlnctrl);
		break;
	case ixgbe_mac_82599EB:
	case ixgbe_mac_X540:
	case ixgbe_mac_X550:
	case ixgbe_mac_X550EM_x:
	case ixgbe_mac_X550EM_a:
		for (i = 0; i < adapter->num_rx_queues; i++) {
			struct ixgbe_ring *ring = adapter->rx_ring[i];

			j = ring->reg_idx;
			vlnctrl = IXGBE_READ_REG(hw, IXGBE_RXDCTL(j));
			vlnctrl &= ~IXGBE_RXDCTL_VME;
			IXGBE_WRITE_REG(hw, IXGBE_RXDCTL(j), vlnctrl);
		}
		break;
	default:
		break;
	}
}

#endif /* HAVE_8021P_SUPPORT */
/**
 * ixgbe_vlan_strip_enable - helper to enable vlan tag stripping
 * @adapter: driver data
 */
void ixgbe_vlan_strip_enable(struct ixgbe_adapter *adapter)
{
	struct ixgbe_hw *hw = &adapter->hw;
	u32 vlnctrl;
	int i, j;

	switch (hw->mac.type) {
	case ixgbe_mac_82598EB:
		vlnctrl = IXGBE_READ_REG(hw, IXGBE_VLNCTRL);
		vlnctrl |= IXGBE_VLNCTRL_VME;
		IXGBE_WRITE_REG(hw, IXGBE_VLNCTRL, vlnctrl);
		break;
	case ixgbe_mac_82599EB:
	case ixgbe_mac_X540:
	case ixgbe_mac_X550:
	case ixgbe_mac_X550EM_x:
	case ixgbe_mac_X550EM_a:
		for (i = 0; i < adapter->num_rx_queues; i++) {
			struct ixgbe_ring *ring = adapter->rx_ring[i];

			j = ring->reg_idx;
			vlnctrl = IXGBE_READ_REG(hw, IXGBE_RXDCTL(j));
			vlnctrl |= IXGBE_RXDCTL_VME;
			IXGBE_WRITE_REG(hw, IXGBE_RXDCTL(j), vlnctrl);
		}
		break;
	default:
		break;
	}
}

#ifndef HAVE_VLAN_RX_REGISTER
static void ixgbe_vlan_promisc_enable(struct ixgbe_adapter *adapter)
{
	struct ixgbe_hw *hw = &adapter->hw;
	u32 vlnctrl, i;

	vlnctrl = IXGBE_READ_REG(hw, IXGBE_VLNCTRL);

	if (adapter->flags & IXGBE_FLAG_VMDQ_ENABLED) {
	/* we need to keep the VLAN filter on in SRIOV */
		vlnctrl |= IXGBE_VLNCTRL_VFE;
		IXGBE_WRITE_REG(hw, IXGBE_VLNCTRL, vlnctrl);
	} else {
		vlnctrl &= ~IXGBE_VLNCTRL_VFE;
		IXGBE_WRITE_REG(hw, IXGBE_VLNCTRL, vlnctrl);
		return;
	}

	/* Nothing to do for 82598 */
	if (hw->mac.type == ixgbe_mac_82598EB)
		return;

	/* We are already in VLAN promisc, nothing to do */
	if (adapter->flags2 & IXGBE_FLAG2_VLAN_PROMISC)
		return;

	/* Set flag so we don't redo unnecessary work */
	adapter->flags2 |= IXGBE_FLAG2_VLAN_PROMISC;

	/* Add PF to all active pools */
	for (i = IXGBE_VLVF_ENTRIES; --i;) {
		u32 reg_offset = IXGBE_VLVFB(i * 2 + VMDQ_P(0) / 32);
		u32 vlvfb = IXGBE_READ_REG(hw, reg_offset);

		vlvfb |= 1 << (VMDQ_P(0) % 32);
		IXGBE_WRITE_REG(hw, reg_offset, vlvfb);
	}

	/* Set all bits in the VLAN filter table array */
	for (i = hw->mac.vft_size; i--;)
		IXGBE_WRITE_REG(hw, IXGBE_VFTA(i), ~0U);
}

#define VFTA_BLOCK_SIZE 8
static void ixgbe_scrub_vfta(struct ixgbe_adapter *adapter, u32 vfta_offset)
{
	struct ixgbe_hw *hw = &adapter->hw;
	u32 vfta[VFTA_BLOCK_SIZE] = { 0 };
	u32 vid_start = vfta_offset * 32;
	u32 vid_end = vid_start + (VFTA_BLOCK_SIZE * 32);
	u32 i, vid, word, bits;

	for (i = IXGBE_VLVF_ENTRIES; --i;) {
		u32 vlvf = IXGBE_READ_REG(hw, IXGBE_VLVF(i));

		/* pull VLAN ID from VLVF */
		vid = vlvf & VLAN_VID_MASK;

		/* only concern outselves with a certain range */
		if (vid < vid_start || vid >= vid_end)
			continue;

		if (vlvf) {
			/* record VLAN ID in VFTA */
			vfta[(vid - vid_start) / 32] |= 1 << (vid % 32);

			/* if PF is part of this then continue */
			if (test_bit(vid, adapter->active_vlans))
				continue;
		}

		/* remove PF from the pool */
		word = i * 2 + VMDQ_P(0) / 32;
		bits = ~(1 << (VMDQ_P(0) % 32));
		bits &= IXGBE_READ_REG(hw, IXGBE_VLVFB(word));
		IXGBE_WRITE_REG(hw, IXGBE_VLVFB(word), bits);
	}

	/* extract values from active_vlans and write back to VFTA */
	for (i = VFTA_BLOCK_SIZE; i--;) {
		vid = (vfta_offset + i) * 32;
		word = vid / BITS_PER_LONG;
		bits = vid % BITS_PER_LONG;

		vfta[i] |= adapter->active_vlans[word] >> bits;

		IXGBE_WRITE_REG(hw, IXGBE_VFTA(vfta_offset + i), vfta[i]);
	}
}

static void ixgbe_vlan_promisc_disable(struct ixgbe_adapter *adapter)
{
	struct ixgbe_hw *hw = &adapter->hw;
	u32 vlnctrl, i;

	/* configure vlan filtering */
	vlnctrl = IXGBE_READ_REG(hw, IXGBE_VLNCTRL);
	vlnctrl |= IXGBE_VLNCTRL_VFE;
	IXGBE_WRITE_REG(hw, IXGBE_VLNCTRL, vlnctrl);

	if (!(adapter->flags & IXGBE_FLAG_VMDQ_ENABLED) ||
	    hw->mac.type == ixgbe_mac_82598EB)
		return;

	/* We are not in VLAN promisc, nothing to do */
	if (!(adapter->flags2 & IXGBE_FLAG2_VLAN_PROMISC))
		return;

	/* Set flag so we don't redo unnecessary work */
	adapter->flags2 &= ~IXGBE_FLAG2_VLAN_PROMISC;

	for (i = 0; i < hw->mac.vft_size; i += VFTA_BLOCK_SIZE)
		ixgbe_scrub_vfta(adapter, i);
}
#endif /* HAVE_VLAN_RX_REGISTER */

#ifdef HAVE_VLAN_RX_REGISTER
static void ixgbe_vlan_mode(struct net_device *netdev, struct vlan_group *grp)
#else
void ixgbe_vlan_mode(struct net_device *netdev, u32 features)
#endif
{
#if defined(HAVE_VLAN_RX_REGISTER) || defined(HAVE_8021P_SUPPORT)
	struct ixgbe_adapter *adapter = netdev_priv(netdev);
#endif
#ifdef HAVE_8021P_SUPPORT
	bool enable;
#endif

#ifdef HAVE_VLAN_RX_REGISTER
	if (!test_bit(__IXGBE_DOWN, &adapter->state))
		ixgbe_irq_disable(adapter);

	adapter->vlgrp = grp;

	if (!test_bit(__IXGBE_DOWN, &adapter->state))
		ixgbe_irq_enable(adapter, true, true);
#endif
#ifdef HAVE_8021P_SUPPORT
#ifdef HAVE_VLAN_RX_REGISTER
	enable = (grp || (adapter->flags & IXGBE_FLAG_DCB_ENABLED));
#else
#ifdef NETIF_F_HW_VLAN_CTAG_RX
	enable = !!(features & NETIF_F_HW_VLAN_CTAG_RX);
#else
	enable = !!(features & NETIF_F_HW_VLAN_RX);
#endif /* NETIF_F_HW_VLAN_CTAG_RX */
#endif /* HAVE_VLAN_RX_REGISTER */
	if (enable)
		/* enable VLAN tag insert/strip */
		ixgbe_vlan_strip_enable(adapter);
	else
		/* disable VLAN tag insert/strip */
		ixgbe_vlan_strip_disable(adapter);

#endif /* HAVE_8021P_SUPPORT */
}

static void ixgbe_restore_vlan(struct ixgbe_adapter *adapter)
{
	u16 vid = 1;
#ifdef HAVE_VLAN_RX_REGISTER

	ixgbe_vlan_mode(adapter->netdev, adapter->vlgrp);

	/*
	 * add vlan ID 0 and enable vlan tag stripping so we
	 * always accept priority-tagged traffic
	 */
#ifdef NETIF_F_HW_VLAN_CTAG_RX
	ixgbe_vlan_rx_add_vid(adapter->netdev, htons(ETH_P_8021Q), 0);
#else
	ixgbe_vlan_rx_add_vid(adapter->netdev, 0);
#endif
#ifndef HAVE_8021P_SUPPORT
	ixgbe_vlan_strip_enable(adapter);
#endif

	if (adapter->vlgrp) {
		for (; vid < VLAN_N_VID; vid++) {
			if (!vlan_group_get_device(adapter->vlgrp, vid))
				continue;
#ifdef NETIF_F_HW_VLAN_CTAG_RX
			ixgbe_vlan_rx_add_vid(adapter->netdev,
					      htons(ETH_P_8021Q), vid);
#else
			ixgbe_vlan_rx_add_vid(adapter->netdev, vid);
#endif
		}
	}
#else /* !HAVE_VLAN_RX_REGISTER */

#ifdef NETIF_F_HW_VLAN_CTAG_RX
	ixgbe_vlan_rx_add_vid(adapter->netdev, htons(ETH_P_8021Q), 0);
#else
	ixgbe_vlan_rx_add_vid(adapter->netdev, 0);
#endif

	for_each_set_bit_from(vid, adapter->active_vlans, VLAN_N_VID)
#ifdef NETIF_F_HW_VLAN_CTAG_RX
		ixgbe_vlan_rx_add_vid(adapter->netdev, htons(ETH_P_8021Q), vid);
#else
		ixgbe_vlan_rx_add_vid(adapter->netdev, vid);
#endif
#endif /* HAVE_VLAN_RX_REGISTER */
}

#endif
static u8 *ixgbe_addr_list_itr(struct ixgbe_hw __maybe_unused *hw, u8 **mc_addr_ptr, u32 *vmdq)
{
#ifdef NETDEV_HW_ADDR_T_MULTICAST
	struct netdev_hw_addr *mc_ptr;
#else
	struct dev_mc_list *mc_ptr;
#endif
#ifdef CONFIG_PCI_IOV
	struct ixgbe_adapter *adapter = hw->back;
#endif /* CONFIG_PCI_IOV */
	u8 *addr = *mc_addr_ptr;

	/* VMDQ_P implicitely uses the adapter struct when CONFIG_PCI_IOV is
	 * defined, so we have to wrap the pointer above correctly to prevent
	 * a warning.
	 */
	*vmdq = VMDQ_P(0);

#ifdef NETDEV_HW_ADDR_T_MULTICAST
	mc_ptr = container_of(addr, struct netdev_hw_addr, addr[0]);
	if (mc_ptr->list.next) {
		struct netdev_hw_addr *ha;

		ha = list_entry(mc_ptr->list.next, struct netdev_hw_addr, list);
		*mc_addr_ptr = ha->addr;
	}
#else
	mc_ptr = container_of(addr, struct dev_mc_list, dmi_addr[0]);
	if (mc_ptr->next)
		*mc_addr_ptr = mc_ptr->next->dmi_addr;
#endif
	else
		*mc_addr_ptr = NULL;

	return addr;
}

/**
 * ixgbe_write_mc_addr_list - write multicast addresses to MTA
 * @netdev: network interface device structure
 *
 * Writes multicast address list to the MTA hash table.
 * Returns: -ENOMEM on failure
 *                0 on no addresses written
 *                X on writing X addresses to MTA
 **/
int ixgbe_write_mc_addr_list(struct net_device *netdev)
{
	struct ixgbe_adapter *adapter = netdev_priv(netdev);
	struct ixgbe_hw *hw = &adapter->hw;
#ifdef NETDEV_HW_ADDR_T_MULTICAST
	struct netdev_hw_addr *ha;
#endif
	u8  *addr_list = NULL;
	int addr_count = 0;

	if (!hw->mac.ops.update_mc_addr_list)
		return -ENOMEM;

	if (!netif_running(netdev))
		return 0;

#ifdef CONFIG_PCI_IOV
	ixgbe_restore_vf_multicasts(adapter);
#endif

	if (netdev_mc_empty(netdev)) {
		hw->mac.ops.update_mc_addr_list(hw, NULL, 0,
						ixgbe_addr_list_itr, false);
	} else {
#ifdef NETDEV_HW_ADDR_T_MULTICAST
		ha = list_first_entry(&netdev->mc.list,
				      struct netdev_hw_addr, list);
		addr_list = ha->addr;
#else
		addr_list = netdev->mc_list->dmi_addr;
#endif
		addr_count = netdev_mc_count(netdev);

		hw->mac.ops.update_mc_addr_list(hw, addr_list, addr_count,
						ixgbe_addr_list_itr, false);
	}

	return addr_count;
}

#ifdef CONFIG_PCI_IOV
void ixgbe_full_sync_mac_table(struct ixgbe_adapter *adapter)
{
	struct ixgbe_mac_addr *mac_table = &adapter->mac_table[0];
	struct ixgbe_hw *hw = &adapter->hw;
	int i;

	for (i = 0; i < hw->mac.num_rar_entries; i++, mac_table++) {
		mac_table->state &= ~IXGBE_MAC_STATE_MODIFIED;

		if (mac_table->state & IXGBE_MAC_STATE_IN_USE)
			hw->mac.ops.set_rar(hw, i,
					    mac_table->addr,
					    mac_table->pool,
					    IXGBE_RAH_AV);
		else
			hw->mac.ops.clear_rar(hw, i);
	}
}
#endif

static void ixgbe_sync_mac_table(struct ixgbe_adapter *adapter)
{
	struct ixgbe_mac_addr *mac_table = &adapter->mac_table[0];
	struct ixgbe_hw *hw = &adapter->hw;
	int i;

	for (i = 0; i < hw->mac.num_rar_entries; i++, mac_table++) {
		if (!(mac_table->state & IXGBE_MAC_STATE_MODIFIED))
			continue;

		mac_table->state &= ~IXGBE_MAC_STATE_MODIFIED;

		if (mac_table->state & IXGBE_MAC_STATE_IN_USE)
			hw->mac.ops.set_rar(hw, i,
					    mac_table->addr,
					    mac_table->pool,
					    IXGBE_RAH_AV);
		else
			hw->mac.ops.clear_rar(hw, i);
	}
}

int ixgbe_available_rars(struct ixgbe_adapter *adapter, u16 pool)
{
	struct ixgbe_mac_addr *mac_table = &adapter->mac_table[0];
	struct ixgbe_hw *hw = &adapter->hw;
	int i, count = 0;

	for (i = 0; i < hw->mac.num_rar_entries; i++, mac_table++) {
		/* do not count default RAR as available */
		if (mac_table->state & IXGBE_MAC_STATE_DEFAULT)
			continue;

		/* only count unused and addresses that belong to us */
		if (mac_table->state & IXGBE_MAC_STATE_IN_USE) {
			if (mac_table->pool != pool)
				continue;
		}

		count++;
	}

	return count;
}

/* this function destroys the first RAR entry */
static void ixgbe_mac_set_default_filter(struct ixgbe_adapter *adapter)
{
	struct ixgbe_mac_addr *mac_table = &adapter->mac_table[0];
	struct ixgbe_hw *hw = &adapter->hw;

	ether_addr_copy(mac_table->addr, hw->mac.addr);
	mac_table->pool = VMDQ_P(0);

	mac_table->state = IXGBE_MAC_STATE_DEFAULT | IXGBE_MAC_STATE_IN_USE;

	hw->mac.ops.set_rar(hw, 0, mac_table->addr, mac_table->pool,
			    IXGBE_RAH_AV);
}

int ixgbe_add_mac_filter(struct ixgbe_adapter *adapter,
			 const u8 *addr, u16 pool)
{
	struct ixgbe_mac_addr *mac_table = &adapter->mac_table[0];
	struct ixgbe_hw *hw = &adapter->hw;
	int i;

	if (is_zero_ether_addr(addr))
		return -EINVAL;

	for (i = 0; i < hw->mac.num_rar_entries; i++, mac_table++) {
		if (mac_table->state & IXGBE_MAC_STATE_IN_USE) {
			continue;
		}

		ether_addr_copy(mac_table->addr, addr);
		mac_table->pool = pool;

		mac_table->state |= IXGBE_MAC_STATE_MODIFIED |
				    IXGBE_MAC_STATE_IN_USE;

		ixgbe_sync_mac_table(adapter);

		return i;
	}

	return -ENOMEM;
}

static void ixgbe_flush_sw_mac_table(struct ixgbe_adapter *adapter)
{
	struct ixgbe_mac_addr *mac_table = &adapter->mac_table[0];
	struct ixgbe_hw *hw = &adapter->hw;
	int i;

	for (i = 0; i < hw->mac.num_rar_entries; i++, mac_table++) {
		mac_table->state |= IXGBE_MAC_STATE_MODIFIED;
		mac_table->state &= ~IXGBE_MAC_STATE_IN_USE;
	}

	ixgbe_sync_mac_table(adapter);
}

int ixgbe_del_mac_filter(struct ixgbe_adapter *adapter,
			 const u8 *addr, u16 pool)
{
	struct ixgbe_mac_addr *mac_table = &adapter->mac_table[0];
	struct ixgbe_hw *hw = &adapter->hw;
	int i;

	if (is_zero_ether_addr(addr))
		return -EINVAL;

	/* search table for addr, if found clear IN USE flag and sync */
	for (i = 0; i < hw->mac.num_rar_entries; i++, mac_table++) {
		/* we can only delete an entry if it is in use */
		if (!(mac_table->state & IXGBE_MAC_STATE_IN_USE))
			continue;
		/* we only care about entries that belong to the given pool */
		if (mac_table->pool != pool)
			continue;
		/* we only care about a specific MAC address */
		if (!ether_addr_equal(addr, mac_table->addr))
			continue;

		mac_table->state |= IXGBE_MAC_STATE_MODIFIED;
		mac_table->state &= ~IXGBE_MAC_STATE_IN_USE;

		ixgbe_sync_mac_table(adapter);

		return 0;
	}

	return -ENOMEM;
}

#ifdef HAVE_SET_RX_MODE
static int ixgbe_uc_sync(struct net_device *netdev, const unsigned char *addr)
{
	struct ixgbe_adapter *adapter = netdev_priv(netdev);
	int ret;

	ret = ixgbe_add_mac_filter(adapter, addr, VMDQ_P(0));

	return min_t(int, ret, 0);
}

static int ixgbe_uc_unsync(struct net_device *netdev, const unsigned char *addr)
{
	struct ixgbe_adapter *adapter = netdev_priv(netdev);

	ixgbe_del_mac_filter(adapter, addr, VMDQ_P(0));

	return 0;
}

#endif
/**
 * ixgbe_set_rx_mode - Unicast, Multicast and Promiscuous mode set
 * @netdev: network interface device structure
 *
 * The set_rx_method entry point is called whenever the unicast/multicast
 * address list or the network interface flags are updated.  This routine is
 * responsible for configuring the hardware for proper unicast, multicast and
 * promiscuous mode.
 **/
void ixgbe_set_rx_mode(struct net_device *netdev)
{
	struct ixgbe_adapter *adapter = netdev_priv(netdev);
	struct ixgbe_hw *hw = &adapter->hw;
	u32 fctrl, vmolr = IXGBE_VMOLR_BAM | IXGBE_VMOLR_AUPE;
#if defined(HAVE_VLAN_RX_REGISTER) || defined(ESX55)
	u32 vlnctrl;
#endif
#if defined(NETIF_F_HW_VLAN_CTAG_FILTER) || defined(NETIF_F_HW_VLAN_FILTER)
	netdev_features_t features = netdev->features;
#endif
	int count;

	/* Check for Promiscuous and All Multicast modes */
	fctrl = IXGBE_READ_REG(hw, IXGBE_FCTRL);
#if defined(HAVE_VLAN_RX_REGISTER) || defined(ESX55)
	vlnctrl = IXGBE_READ_REG(hw, IXGBE_VLNCTRL);
#endif

	/* set all bits that we expect to always be set */
	fctrl |= IXGBE_FCTRL_BAM;
	fctrl |= IXGBE_FCTRL_DPF; /* discard pause frames when FC enabled */
	fctrl |= IXGBE_FCTRL_PMCF;

	/* clear the bits we are changing the status of */
	fctrl &= ~(IXGBE_FCTRL_UPE | IXGBE_FCTRL_MPE);
#if defined(HAVE_VLAN_RX_REGISTER) || defined(ESX55)
	vlnctrl  &= ~(IXGBE_VLNCTRL_VFE | IXGBE_VLNCTRL_CFIEN);
#endif
	if (netdev->flags & IFF_PROMISC) {
		hw->addr_ctrl.user_set_promisc = true;
		fctrl |= (IXGBE_FCTRL_UPE | IXGBE_FCTRL_MPE);
		vmolr |= IXGBE_VMOLR_MPE;
#ifdef HAVE_VLAN_RX_REGISTER
		/* Only disable hardware filter vlans in promiscuous mode
		 * if SR-IOV and VMDQ are disabled - otherwise ensure
		 * that hardware VLAN filters remain enabled.
		 */
		if ((adapter->flags & (IXGBE_FLAG_VMDQ_ENABLED |
				       IXGBE_FLAG_SRIOV_ENABLED)))
			vlnctrl |= (IXGBE_VLNCTRL_VFE | IXGBE_VLNCTRL_CFIEN);
#endif
#ifdef NETIF_F_HW_VLAN_CTAG_FILTER
		features &= ~NETIF_F_HW_VLAN_CTAG_FILTER;
#endif
#ifdef NETIF_F_HW_VLAN_FILTER
		features &= ~NETIF_F_HW_VLAN_FILTER;
#endif
	} else {
		if (netdev->flags & IFF_ALLMULTI) {
			fctrl |= IXGBE_FCTRL_MPE;
			vmolr |= IXGBE_VMOLR_MPE;
		}
		hw->addr_ctrl.user_set_promisc = false;
#if defined(HAVE_VLAN_RX_REGISTER) || defined(ESX55)
		/* enable hardware vlan filtering */
		vlnctrl |= IXGBE_VLNCTRL_VFE;
#endif
	}

#ifdef HAVE_SET_RX_MODE
	/*
	 * Write addresses to available RAR registers, if there is not
	 * sufficient space to store all the addresses then enable
	 * unicast promiscuous mode
	 */
	if (__dev_uc_sync(netdev, ixgbe_uc_sync, ixgbe_uc_unsync)) {
		fctrl |= IXGBE_FCTRL_UPE;
		vmolr |= IXGBE_VMOLR_ROPE;
	}

#endif
	/*
	 * Write addresses to the MTA, if the attempt fails
	 * then we should just turn on promiscuous mode so
	 * that we can at least receive multicast traffic
	 */
	count = ixgbe_write_mc_addr_list(netdev);
	if (count < 0) {
		fctrl |= IXGBE_FCTRL_MPE;
		vmolr |= IXGBE_VMOLR_MPE;
	} else if (count) {
		vmolr |= IXGBE_VMOLR_ROMPE;
	}

	if (hw->mac.type != ixgbe_mac_82598EB) {
		vmolr |= IXGBE_READ_REG(hw, IXGBE_VMOLR(VMDQ_P(0))) &
			 ~(IXGBE_VMOLR_MPE | IXGBE_VMOLR_ROMPE |
			   IXGBE_VMOLR_ROPE);
		IXGBE_WRITE_REG(hw, IXGBE_VMOLR(VMDQ_P(0)), vmolr);
	}

	IXGBE_WRITE_REG(hw, IXGBE_FCTRL, fctrl);

#ifdef HAVE_8021P_SUPPORT
#ifdef NETIF_F_HW_VLAN_CTAG_RX
	if (features & NETIF_F_HW_VLAN_CTAG_RX)
#else
	if (features & NETIF_F_HW_VLAN_RX)
#endif
		ixgbe_vlan_strip_enable(adapter);
	else
		ixgbe_vlan_strip_disable(adapter);
#endif /* HAVE_8021P_SUPPORT */

#if defined(NETIF_F_HW_VLAN_CTAG_FILTER)
	if (features & NETIF_F_HW_VLAN_CTAG_FILTER)
		ixgbe_vlan_promisc_disable(adapter);
	else
		ixgbe_vlan_promisc_enable(adapter);
#elif defined(NETIF_F_HW_VLAN_FILTER) && !defined(HAVE_VLAN_RX_REGISTER)
	if (features & NETIF_F_HW_VLAN_FILTER)
		ixgbe_vlan_promisc_disable(adapter);
	else
		ixgbe_vlan_promisc_enable(adapter);
#elif defined(HAVE_VLAN_RX_REGISTER) || defined(ESX55)
	IXGBE_WRITE_REG(hw, IXGBE_VLNCTRL, vlnctrl);
#endif /* NETIF_F_HW_VLAN_CTAG_FILTER */
}

static void ixgbe_napi_enable_all(struct ixgbe_adapter *adapter)
{
	struct ixgbe_q_vector *q_vector;
	int q_idx;

	for (q_idx = 0; q_idx < adapter->num_q_vectors; q_idx++) {
		q_vector = adapter->q_vector[q_idx];
#ifdef HAVE_NDO_BUSY_POLL
		ixgbe_qv_init_lock(adapter->q_vector[q_idx]);
#endif
		napi_enable(&q_vector->napi);
	}
}

static void ixgbe_napi_disable_all(struct ixgbe_adapter *adapter)
{
	struct ixgbe_q_vector *q_vector;
	int q_idx;

	for (q_idx = 0; q_idx < adapter->num_q_vectors; q_idx++) {
		q_vector = adapter->q_vector[q_idx];
		napi_disable(&q_vector->napi);
#ifdef HAVE_NDO_BUSY_POLL
		while(!ixgbe_qv_disable(adapter->q_vector[q_idx])) {
			pr_info("QV %d locked\n", q_idx);
			usleep_range(1000, 20000);
		}
#endif
	}
}

#ifdef HAVE_DCBNL_IEEE
s32 ixgbe_dcb_hw_ets(struct ixgbe_hw *hw, struct ieee_ets *ets, int max_frame)
{
	__u16 refill[IEEE_8021QAZ_MAX_TCS], max[IEEE_8021QAZ_MAX_TCS];
	__u8 prio_type[IEEE_8021QAZ_MAX_TCS];
	int i;

	/* naively give each TC a bwg to map onto CEE hardware */
	__u8 bwg_id[IEEE_8021QAZ_MAX_TCS] = {0, 1, 2, 3, 4, 5, 6, 7};

	/* Map TSA onto CEE prio type */
	for (i = 0; i < IEEE_8021QAZ_MAX_TCS; i++) {
		switch (ets->tc_tsa[i]) {
		case IEEE_8021QAZ_TSA_STRICT:
			prio_type[i] = 2;
			break;
		case IEEE_8021QAZ_TSA_ETS:
			prio_type[i] = 0;
			break;
		default:
			/* Hardware only supports priority strict or
			 * ETS transmission selection algorithms if
			 * we receive some other value from dcbnl
			 * throw an error
			 */
			return -EINVAL;
		}
	}

	ixgbe_dcb_calculate_tc_credits(ets->tc_tx_bw, refill, max, max_frame);
	return ixgbe_dcb_hw_config(hw, refill, max,
				   bwg_id, prio_type, ets->prio_tc);
}
#endif /* HAVE_DCBNL_IEEE */

#if defined(HAVE_UDP_ENC_RX_OFFLOAD) || defined(HAVE_VXLAN_RX_OFFLOAD)
void ixgbe_clear_udp_tunnel_port(struct ixgbe_adapter *adapter, u32 mask)
{
	struct ixgbe_hw *hw = &adapter->hw;
	u32 vxlanctrl;

	if (!(adapter->flags & (IXGBE_FLAG_VXLAN_OFFLOAD_CAPABLE |
				IXGBE_FLAG_GENEVE_OFFLOAD_CAPABLE)))
		return;

	vxlanctrl = IXGBE_READ_REG(hw, IXGBE_VXLANCTRL) & ~mask;
	IXGBE_WRITE_REG(hw, IXGBE_VXLANCTRL, vxlanctrl);

	if (mask & IXGBE_VXLANCTRL_VXLAN_UDPPORT_MASK)
		adapter->vxlan_port = 0;
#ifdef HAVE_UDP_ENC_RX_OFFLOAD
	if (mask & IXGBE_VXLANCTRL_GENEVE_UDPPORT_MASK)
		adapter->geneve_port = 0;
#endif /* HAVE_UDP_ENC_RX_OFFLOAD */
}
#endif /* HAVE_UDP_ENC_RX_OFFLOAD || HAVE_VXLAN_RX_OFFLOAD */

#ifdef NETIF_F_GSO_PARTIAL
/* NETIF_F_GSO_IPXIP4/6 may not be defined in all distributions */
#ifndef NETIF_F_GSO_IPXIP4
#define NETIF_F_GSO_IPXIP4 0
#endif
#ifndef NETIF_F_GSO_IPXIP6
#define NETIF_F_GSO_IPXIP6 0
#endif
#define IXGBE_GSO_PARTIAL_FEATURES (NETIF_F_GSO_GRE | \
				    NETIF_F_GSO_GRE_CSUM | \
				    NETIF_F_GSO_IPXIP4 | \
				    NETIF_F_GSO_IPXIP6 | \
				    NETIF_F_GSO_UDP_TUNNEL | \
				    NETIF_F_GSO_UDP_TUNNEL_CSUM)
#endif /* NETIF_F_GSO_PARTIAL */

static inline unsigned long ixgbe_tso_features(void)
{
	unsigned long features = 0;

#ifdef NETIF_F_TSO
	features |= NETIF_F_TSO;
#endif /* NETIF_F_TSO */
#ifdef NETIF_F_TSO6
	features |= NETIF_F_TSO6;
#endif /* NETIF_F_TSO6 */
#ifdef NETIF_F_GSO_PARTIAL
	features |= NETIF_F_GSO_PARTIAL | IXGBE_GSO_PARTIAL_FEATURES;
#endif

	return features;
}

/*
 * ixgbe_configure_dcb - Configure DCB hardware support
 * @adapter: ixgbe adapter struct
 *
 * Called when the driver opens or needs to reconfigure DCB related bits.
 */
static void ixgbe_configure_dcb(struct ixgbe_adapter *adapter)
{
	struct ixgbe_hw *hw = &adapter->hw;
	struct net_device *netdev = adapter->netdev;

	int max_frame = netdev->mtu + ETH_HLEN + ETH_FCS_LEN;

	/* The following workaround for 82598EB was originaly hidden inside a
	 * kcompat definition of netif_set_gso_max_size. This workaround is
	 * necessary as the 82598EB hardware does not support TSO and DCB
	 * unless the stack TSO maximum segment size can be reduced. Older
	 * kernels do not support the requisite interface, and thus need TSO
	 * disabled if we want to support DCB.
	 */
	if (!(adapter->flags & IXGBE_FLAG_DCB_ENABLED)) {
		if (hw->mac.type == ixgbe_mac_82598EB) {
#ifdef NETDEV_CAN_SET_GSO_MAX_SIZE
			netif_set_gso_max_size(netdev, 65536);
#else
			/* We previously disabled TSO, so we should enable it
			 * now. */
			netdev->features |= ixgbe_tso_features();
#ifdef NETIF_F_GSO_PARTIAL
			netdev->gso_partial_features =
						     IXGBE_GSO_PARTIAL_FEATURES;
#endif
#endif /* NETDEV_CAN_SET_GSO_MAX_SIZE */
		}
		return;
	}

	if (hw->mac.type == ixgbe_mac_82598EB) {
#ifdef NETDEV_CAN_SET_GSO_MAX_SIZE
		netif_set_gso_max_size(netdev, 32768);
#else
		/* Simply disable TSO since we cannot change the maximum
		 * segment size. */
		netdev->features &= ~ixgbe_tso_features();
#ifdef NETIF_F_GSO_PARTIAL
		netdev->gso_partial_features = 0;
#endif
#endif /* NETDEV_CAN_SET_GSO_MAX_SIZE */
	}

#if IS_ENABLED(CONFIG_FCOE)
	if (netdev->features & NETIF_F_FCOE_MTU)
		max_frame = max_t(int, max_frame,
				  IXGBE_FCOE_JUMBO_FRAME_SIZE);
#endif /* CONFIG_FCOE */

#ifdef HAVE_DCBNL_IEEE
	if (adapter->dcbx_cap & DCB_CAP_DCBX_VER_IEEE) {
		if (adapter->ixgbe_ieee_ets)
			ixgbe_dcb_hw_ets(&adapter->hw,
					 adapter->ixgbe_ieee_ets,
					 max_frame);

		if (adapter->ixgbe_ieee_pfc && adapter->ixgbe_ieee_ets) {
			struct ieee_pfc *pfc = adapter->ixgbe_ieee_pfc;
			u8 *tc = adapter->ixgbe_ieee_ets->prio_tc;

			ixgbe_dcb_config_pfc(&adapter->hw, pfc->pfc_en, tc);
		}
	} else
#endif /* HAVE_DCBNL_IEEE */
	{
		ixgbe_dcb_calculate_tc_credits_cee(hw,
						   &adapter->dcb_cfg,
						   max_frame,
						   IXGBE_DCB_TX_CONFIG);
		ixgbe_dcb_calculate_tc_credits_cee(hw,
						   &adapter->dcb_cfg,
						   max_frame,
						   IXGBE_DCB_RX_CONFIG);
		ixgbe_dcb_hw_config_cee(hw, &adapter->dcb_cfg);
	}

	/* Enable RSS Hash per TC */
	if (hw->mac.type != ixgbe_mac_82598EB) {
		u32 msb = 0;
		u16 rss_i = adapter->ring_feature[RING_F_RSS].indices - 1;

		while (rss_i) {
			msb++;
			rss_i >>= 1;
		}

		/* write msb to all 8 TCs in one write */
		IXGBE_WRITE_REG(hw, IXGBE_RQTC, msb * 0x11111111);
	}
}

#ifndef IXGBE_NO_LLI
static void ixgbe_configure_lli_82599(struct ixgbe_adapter *adapter)
{
	u16 port;

	if (adapter->lli_etype) {
		IXGBE_WRITE_REG(&adapter->hw, IXGBE_L34T_IMIR(0),
				(IXGBE_IMIR_LLI_EN_82599 |
				 IXGBE_IMIR_SIZE_BP_82599 |
				 IXGBE_IMIR_CTRL_BP_82599));
		IXGBE_WRITE_REG(&adapter->hw, IXGBE_ETQS(0), IXGBE_ETQS_LLI);
		IXGBE_WRITE_REG(&adapter->hw, IXGBE_ETQF(0),
				(adapter->lli_etype | IXGBE_ETQF_FILTER_EN));
	}

	if (adapter->lli_port) {
		port = swab16(adapter->lli_port);
		IXGBE_WRITE_REG(&adapter->hw, IXGBE_L34T_IMIR(0),
				(IXGBE_IMIR_LLI_EN_82599 |
				 IXGBE_IMIR_SIZE_BP_82599 |
				 IXGBE_IMIR_CTRL_BP_82599));
		IXGBE_WRITE_REG(&adapter->hw, IXGBE_FTQF(0),
				(IXGBE_FTQF_POOL_MASK_EN |
				 (IXGBE_FTQF_PRIORITY_MASK <<
				  IXGBE_FTQF_PRIORITY_SHIFT) |
				 (IXGBE_FTQF_DEST_PORT_MASK <<
				  IXGBE_FTQF_5TUPLE_MASK_SHIFT)));
		IXGBE_WRITE_REG(&adapter->hw, IXGBE_SDPQF(0), (port << 16));
	}

	if (adapter->flags & IXGBE_FLAG_LLI_PUSH) {
		switch (adapter->hw.mac.type) {
		case ixgbe_mac_82599EB:
			IXGBE_WRITE_REG(&adapter->hw, IXGBE_L34T_IMIR(0),
					(IXGBE_IMIR_LLI_EN_82599 |
					 IXGBE_IMIR_SIZE_BP_82599 |
					 IXGBE_IMIR_CTRL_PSH_82599 |
					 IXGBE_IMIR_CTRL_SYN_82599 |
					 IXGBE_IMIR_CTRL_URG_82599 |
					 IXGBE_IMIR_CTRL_ACK_82599 |
					 IXGBE_IMIR_CTRL_RST_82599 |
					 IXGBE_IMIR_CTRL_FIN_82599));
			IXGBE_WRITE_REG(&adapter->hw, IXGBE_LLITHRESH,
					0xfc000000);
			break;
		case ixgbe_mac_X540:
			IXGBE_WRITE_REG(&adapter->hw, IXGBE_L34T_IMIR(0),
					(IXGBE_IMIR_LLI_EN_82599 |
					 IXGBE_IMIR_SIZE_BP_82599 |
					 IXGBE_IMIR_CTRL_PSH_82599));
			break;
		default:
			break;
		}
		IXGBE_WRITE_REG(&adapter->hw, IXGBE_FTQF(0),
				(IXGBE_FTQF_POOL_MASK_EN |
				 (IXGBE_FTQF_PRIORITY_MASK <<
				  IXGBE_FTQF_PRIORITY_SHIFT) |
				 (IXGBE_FTQF_5TUPLE_MASK_MASK <<
				  IXGBE_FTQF_5TUPLE_MASK_SHIFT)));

		IXGBE_WRITE_REG(&adapter->hw, IXGBE_SYNQF, 0x80000100);
	}

	if (adapter->lli_size) {
		IXGBE_WRITE_REG(&adapter->hw, IXGBE_L34T_IMIR(0),
				(IXGBE_IMIR_LLI_EN_82599 |
				 IXGBE_IMIR_CTRL_BP_82599));
		IXGBE_WRITE_REG(&adapter->hw, IXGBE_LLITHRESH,
				adapter->lli_size);
		IXGBE_WRITE_REG(&adapter->hw, IXGBE_FTQF(0),
				(IXGBE_FTQF_POOL_MASK_EN |
				 (IXGBE_FTQF_PRIORITY_MASK <<
				  IXGBE_FTQF_PRIORITY_SHIFT) |
				 (IXGBE_FTQF_5TUPLE_MASK_MASK <<
				  IXGBE_FTQF_5TUPLE_MASK_SHIFT)));
	}

	if (adapter->lli_vlan_pri) {
		IXGBE_WRITE_REG(&adapter->hw, IXGBE_IMIRVP,
				(IXGBE_IMIRVP_PRIORITY_EN |
				 adapter->lli_vlan_pri));
	}
}

static void ixgbe_configure_lli(struct ixgbe_adapter *adapter)
{
	u16 port;

	/* lli should only be enabled with MSI-X and MSI */
	if (!(adapter->flags & IXGBE_FLAG_MSI_ENABLED) &&
	    !(adapter->flags & IXGBE_FLAG_MSIX_ENABLED))
		return;
	/* LLI not supported on X550 and X550EM_x*/
	if ((adapter->hw.mac.type == ixgbe_mac_X550) ||
	    (adapter->hw.mac.type == ixgbe_mac_X550EM_x))
		return;
	/* LLI not supported on X550EM_a */
	if (adapter->hw.mac.type == ixgbe_mac_X550EM_a)
		return;
	if (adapter->hw.mac.type != ixgbe_mac_82598EB) {
		ixgbe_configure_lli_82599(adapter);
		return;
	}

	if (adapter->lli_port) {
		/* use filter 0 for port */
		port = swab16(adapter->lli_port);
		IXGBE_WRITE_REG(&adapter->hw, IXGBE_IMIR(0),
				(port | IXGBE_IMIR_PORT_IM_EN));
		IXGBE_WRITE_REG(&adapter->hw, IXGBE_IMIREXT(0),
				(IXGBE_IMIREXT_SIZE_BP |
				 IXGBE_IMIREXT_CTRL_BP));
	}

	if (adapter->flags & IXGBE_FLAG_LLI_PUSH) {
		/* use filter 1 for push flag */
		IXGBE_WRITE_REG(&adapter->hw, IXGBE_IMIR(1),
				(IXGBE_IMIR_PORT_BP | IXGBE_IMIR_PORT_IM_EN));
		IXGBE_WRITE_REG(&adapter->hw, IXGBE_IMIREXT(1),
				(IXGBE_IMIREXT_SIZE_BP |
				 IXGBE_IMIREXT_CTRL_PSH));
	}

	if (adapter->lli_size) {
		/* use filter 2 for size */
		IXGBE_WRITE_REG(&adapter->hw, IXGBE_IMIR(2),
				(IXGBE_IMIR_PORT_BP | IXGBE_IMIR_PORT_IM_EN));
		IXGBE_WRITE_REG(&adapter->hw, IXGBE_IMIREXT(2),
				(adapter->lli_size | IXGBE_IMIREXT_CTRL_BP));
	}
}

#endif /* IXGBE_NO_LLI */
/* Additional bittime to account for IXGBE framing */
#define IXGBE_ETH_FRAMING 20

/*
 * ixgbe_hpbthresh - calculate high water mark for flow control
 *
 * @adapter: board private structure to calculate for
 * @pb - packet buffer to calculate
 */
static int ixgbe_hpbthresh(struct ixgbe_adapter *adapter, int pb)
{
	struct ixgbe_hw *hw = &adapter->hw;
	struct net_device *dev = adapter->netdev;
	int link, tc, kb, marker;
	u32 dv_id, rx_pba;

	/* Calculate max LAN frame size */
	tc = link = dev->mtu + ETH_HLEN + ETH_FCS_LEN + IXGBE_ETH_FRAMING;

#if IS_ENABLED(CONFIG_FCOE)
	/* FCoE traffic class uses FCOE jumbo frames */
	if ((dev->features & NETIF_F_FCOE_MTU) &&
	    (tc < IXGBE_FCOE_JUMBO_FRAME_SIZE) &&
	    (pb == netdev_get_prio_tc_map(dev, adapter->fcoe.up)))
		tc = IXGBE_FCOE_JUMBO_FRAME_SIZE;
#endif /* CONFIG_FCOE */

	/* Calculate delay value for device */
	switch (hw->mac.type) {
	case ixgbe_mac_X540:
	case ixgbe_mac_X550:
	case ixgbe_mac_X550EM_x:
	case ixgbe_mac_X550EM_a:
		dv_id = IXGBE_DV_X540(link, tc);
		break;
	default:
		dv_id = IXGBE_DV(link, tc);
		break;
	}

	/* Loopback switch introduces additional latency */
	if (adapter->flags & IXGBE_FLAG_SRIOV_ENABLED)
		dv_id += IXGBE_B2BT(tc);

	/* Delay value is calculated in bit times convert to KB */
	kb = IXGBE_BT2KB(dv_id);
	rx_pba = IXGBE_READ_REG(hw, IXGBE_RXPBSIZE(pb)) >> 10;

	marker = rx_pba - kb;

	/* It is possible that the packet buffer is not large enough
	 * to provide required headroom. In this case throw an error
	 * to user and a do the best we can.
	 */
	if (marker < 0) {
		e_warn(drv, "Packet Buffer(%i) can not provide enough"
			    "headroom to suppport flow control."
			    "Decrease MTU or number of traffic classes\n", pb);
		marker = tc + 1;
	}

	return marker;
}

/*
 * ixgbe_lpbthresh - calculate low water mark for for flow control
 *
 * @adapter: board private structure to calculate for
 * @pb - packet buffer to calculate
 */
static int ixgbe_lpbthresh(struct ixgbe_adapter *adapter, int __maybe_unused pb)
{
	struct ixgbe_hw *hw = &adapter->hw;
	struct net_device *dev = adapter->netdev;
	int tc;
	u32 dv_id;

	/* Calculate max LAN frame size */
	tc = dev->mtu + ETH_HLEN + ETH_FCS_LEN;

#if IS_ENABLED(CONFIG_FCOE)
	/* FCoE traffic class uses FCOE jumbo frames */
	if ((dev->features & NETIF_F_FCOE_MTU) &&
	    (tc < IXGBE_FCOE_JUMBO_FRAME_SIZE) &&
	    (pb == netdev_get_prio_tc_map(dev, adapter->fcoe.up)))
		tc = IXGBE_FCOE_JUMBO_FRAME_SIZE;
#endif /* CONFIG_FCOE */

	/* Calculate delay value for device */
	switch (hw->mac.type) {
	case ixgbe_mac_X540:
	case ixgbe_mac_X550:
	case ixgbe_mac_X550EM_x:
	case ixgbe_mac_X550EM_a:
		dv_id = IXGBE_LOW_DV_X540(tc);
		break;
	default:
		dv_id = IXGBE_LOW_DV(tc);
		break;
	}

	/* Delay value is calculated in bit times convert to KB */
	return IXGBE_BT2KB(dv_id);
}

/*
 * ixgbe_pbthresh_setup - calculate and setup high low water marks
 */
static void ixgbe_pbthresh_setup(struct ixgbe_adapter *adapter)
{
	struct ixgbe_hw *hw = &adapter->hw;
	int num_tc = netdev_get_num_tc(adapter->netdev);
	int i;

	if (!num_tc)
		num_tc = 1;


	for (i = 0; i < num_tc; i++) {
		hw->fc.high_water[i] = ixgbe_hpbthresh(adapter, i);
		hw->fc.low_water[i] = ixgbe_lpbthresh(adapter, i);

		/* Low water marks must not be larger than high water marks */
		if (hw->fc.low_water[i] > hw->fc.high_water[i])
			hw->fc.low_water[i] = 0;
	}

	for (; i < IXGBE_DCB_MAX_TRAFFIC_CLASS; i++)
		hw->fc.high_water[i] = 0;
}

static void ixgbe_configure_pb(struct ixgbe_adapter *adapter)
{
	struct ixgbe_hw *hw = &adapter->hw;
	int hdrm;
	u8 tc = netdev_get_num_tc(adapter->netdev);

	if (adapter->flags & IXGBE_FLAG_FDIR_HASH_CAPABLE ||
	    adapter->flags & IXGBE_FLAG_FDIR_PERFECT_CAPABLE)
		hdrm = 32 << adapter->fdir_pballoc;
	else
		hdrm = 0;

	hw->mac.ops.setup_rxpba(hw, tc, hdrm, PBA_STRATEGY_EQUAL);
	ixgbe_pbthresh_setup(adapter);
}

static void ixgbe_fdir_filter_restore(struct ixgbe_adapter *adapter)
{
	struct ixgbe_hw *hw = &adapter->hw;
	struct hlist_node *node2;
	struct ixgbe_fdir_filter *filter;
	u8 queue = 0;

	spin_lock(&adapter->fdir_perfect_lock);

	if (!hlist_empty(&adapter->fdir_filter_list))
		ixgbe_fdir_set_input_mask_82599(hw, &adapter->fdir_mask,
						adapter->cloud_mode);

	hlist_for_each_entry_safe(filter, node2,
				  &adapter->fdir_filter_list, fdir_node) {
		if (filter->action == IXGBE_FDIR_DROP_QUEUE) {
			queue = IXGBE_FDIR_DROP_QUEUE;
		} else {
			u32 ring = ethtool_get_flow_spec_ring(filter->action);
			u8 vf = ethtool_get_flow_spec_ring_vf(filter->action);

			if (!vf && ring >= adapter->num_rx_queues) {
				e_err(drv,
				      "FDIR restore failed w/o vf, ring:%u\n",
				      ring);
				continue;
			} else if (vf &&
				   ((vf > adapter->num_vfs) ||
				   ring >= adapter->num_rx_queues_per_pool)) {
				e_err(drv,
				      "FDIR restore failed vf:%hhu, ring:%u\n",
				      vf, ring);
				continue;
			}

			/* Map the ring onto the absolute queue index */
			if (!vf)
				queue = adapter->rx_ring[ring]->reg_idx;
			else
				queue = ((vf - 1) *
					adapter->num_rx_queues_per_pool) + ring;
		}

		ixgbe_fdir_write_perfect_filter_82599(hw,
						      &filter->filter,
						      filter->sw_idx,
						      queue,
						      adapter->cloud_mode);
	}

	spin_unlock(&adapter->fdir_perfect_lock);
}

/**
 * ixgbe_clean_rx_ring - Free Rx Buffers per Queue
 * @rx_ring: ring to free buffers from
 **/
static void ixgbe_clean_rx_ring(struct ixgbe_ring *rx_ring)
{
	u16 i = rx_ring->next_to_clean;
	struct ixgbe_rx_buffer *rx_buffer = &rx_ring->rx_buffer_info[i];
#if defined(HAVE_STRUCT_DMA_ATTRS) && defined(HAVE_SWIOTLB_SKIP_CPU_SYNC)
	DEFINE_DMA_ATTRS(attrs);

	dma_set_attr(DMA_ATTR_SKIP_CPU_SYNC, &attrs);
	dma_set_attr(DMA_ATTR_WEAK_ORDERING, &attrs);
#endif

#ifdef HAVE_AF_XDP_ZC_SUPPORT
	if (rx_ring->xsk_umem) {
		ixgbe_xsk_clean_rx_ring(rx_ring);
		goto skip_free;
	}

#endif
	/* Free all the Rx ring sk_buffs */
#ifdef CONFIG_IXGBE_DISABLE_PACKET_SPLIT
	while (i != rx_ring->next_to_use) {
#else
	while (i != rx_ring->next_to_alloc) {
#endif
		if (rx_buffer->skb) {
			struct sk_buff *skb = rx_buffer->skb;
#ifndef CONFIG_IXGBE_DISABLE_PACKET_SPLIT
			if (IXGBE_CB(skb)->page_released)
				dma_unmap_page_attrs(rx_ring->dev,
						     IXGBE_CB(skb)->dma,
						     ixgbe_rx_pg_size(rx_ring),
						     DMA_FROM_DEVICE,
#if defined(HAVE_STRUCT_DMA_ATTRS) && defined(HAVE_SWIOTLB_SKIP_CPU_SYNC)
						     &attrs);
#else
						     IXGBE_RX_DMA_ATTR);
#endif
#else
			/* We need to clean up RSC frag lists */
			skb = ixgbe_merge_active_tail(skb);
			if (ixgbe_close_active_frag_list(skb))
				dma_unmap_single(rx_ring->dev,
						 IXGBE_CB(skb)->dma,
						 rx_ring->rx_buf_len,
						 DMA_FROM_DEVICE);
			IXGBE_CB(skb)->dma = 0;
#endif /* CONFIG_IXGBE_DISABLE_PACKET_SPLIT */
			dev_kfree_skb(skb);
			rx_buffer->skb = NULL;
		}

#ifndef CONFIG_IXGBE_DISABLE_PACKET_SPLIT
		/* Invalidate cache lines that may have been written to by
		 * device so that we avoid corrupting memory.
		 */
		dma_sync_single_range_for_cpu(rx_ring->dev,
					      rx_buffer->dma,
					      rx_buffer->page_offset,
					      ixgbe_rx_bufsz(rx_ring),
					      DMA_FROM_DEVICE);

		/* free resources associated with mapping */
		dma_unmap_page_attrs(rx_ring->dev, rx_buffer->dma,
				     ixgbe_rx_pg_size(rx_ring),
				     DMA_FROM_DEVICE,
#if defined(HAVE_STRUCT_DMA_ATTRS) && defined(HAVE_SWIOTLB_SKIP_CPU_SYNC)
				     &attrs);
#else
				     IXGBE_RX_DMA_ATTR);
#endif

		__page_frag_cache_drain(rx_buffer->page,
					rx_buffer->pagecnt_bias);
#else /* CONFIG_IXGBE_DISABLE_PACKET_SPLIT */
		if (rx_buffer->dma) {
			dma_unmap_single(rx_ring->dev,
					 rx_buffer->dma,
					 rx_ring->rx_buf_len,
					 DMA_FROM_DEVICE);
			rx_buffer->dma = 0;
		}
#endif /* CONFIG_IXGBE_DISABLE_PACKET_SPLIT */
		i++;
		rx_buffer++;
		if (i == rx_ring->count) {
			i = 0;
			rx_buffer = rx_ring->rx_buffer_info;
		}
	}

#ifdef HAVE_AF_XDP_ZC_SUPPORT
skip_free:
#endif
#ifndef CONFIG_IXGBE_DISABLE_PACKET_SPLIT
	rx_ring->next_to_alloc = 0;
	rx_ring->next_to_clean = 0;
	rx_ring->next_to_use = 0;
#endif
}

static void ixgbe_configure(struct ixgbe_adapter *adapter)
{
	struct ixgbe_hw *hw = &adapter->hw;

	ixgbe_configure_pb(adapter);
	ixgbe_configure_dcb(adapter);

	/*
	 * We must restore virtualization before VLANs or else
	 * the VLVF registers will not be populated
	 */
	ixgbe_configure_virtualization(adapter);

	ixgbe_set_rx_mode(adapter->netdev);
#if defined(NETIF_F_HW_VLAN_TX) || defined(NETIF_F_HW_VLAN_CTAG_TX)
	ixgbe_restore_vlan(adapter);
#endif

	if (adapter->hw.mac.type == ixgbe_mac_82599EB ||
	    adapter->hw.mac.type == ixgbe_mac_X540)
		hw->mac.ops.disable_sec_rx_path(hw);

	if (adapter->flags & IXGBE_FLAG_FDIR_HASH_CAPABLE) {
		ixgbe_init_fdir_signature_82599(&adapter->hw,
						adapter->fdir_pballoc);
	} else if (adapter->flags & IXGBE_FLAG_FDIR_PERFECT_CAPABLE) {
		ixgbe_init_fdir_perfect_82599(&adapter->hw,
					      adapter->fdir_pballoc, adapter->cloud_mode);
		ixgbe_fdir_filter_restore(adapter);
	}

	if (adapter->hw.mac.type == ixgbe_mac_82599EB ||
	    adapter->hw.mac.type == ixgbe_mac_X540)
		hw->mac.ops.enable_sec_rx_path(hw);

	/* Enable EEE only when supported and enabled */
	if (hw->mac.ops.setup_eee &&
	    (adapter->flags2 & IXGBE_FLAG2_EEE_CAPABLE)) {
		bool eee_enable = !!(adapter->flags2 & IXGBE_FLAG2_EEE_ENABLED);

		hw->mac.ops.setup_eee(hw, eee_enable);
	}

#if IS_ENABLED(CONFIG_DCA)
	/* configure DCA */
	if (adapter->flags & IXGBE_FLAG_DCA_CAPABLE)
		ixgbe_setup_dca(adapter);
#endif

#if IS_ENABLED(CONFIG_FCOE)
	/* configure FCoE L2 filters, redirection table, and Rx control */
	ixgbe_configure_fcoe(adapter);
#endif /* CONFIG_FCOE */

	ixgbe_configure_tx(adapter);
	ixgbe_configure_rx(adapter);
}

static bool ixgbe_is_sfp(struct ixgbe_hw *hw)
{
	switch (hw->mac.type) {
	case ixgbe_mac_82598EB:
		if (hw->phy.type == ixgbe_phy_nl)
			return true;
		return false;
	case ixgbe_mac_82599EB:
		switch (hw->mac.ops.get_media_type(hw)) {
		case ixgbe_media_type_fiber:
		case ixgbe_media_type_fiber_qsfp:
			return true;
		default:
			return false;
		}
	case ixgbe_mac_X550EM_x:
	case ixgbe_mac_X550EM_a:
		if (hw->mac.ops.get_media_type(hw) == ixgbe_media_type_fiber)
			return true;
		return false;
	default:
		return false;
	}
}

/**
 * ixgbe_sfp_link_config - set up SFP+ link
 * @adapter: pointer to private adapter struct
 **/
static void ixgbe_sfp_link_config(struct ixgbe_adapter *adapter)
{
	/*
	 * We are assuming the worst case scenerio here, and that
	 * is that an SFP was inserted/removed after the reset
	 * but before SFP detection was enabled.  As such the best
	 * solution is to just start searching as soon as we start
	 */
	if (adapter->hw.mac.type == ixgbe_mac_82598EB)
		adapter->flags2 |= IXGBE_FLAG2_SEARCH_FOR_SFP;

	adapter->flags2 |= IXGBE_FLAG2_SFP_NEEDS_RESET;
	adapter->sfp_poll_time = 0;
}

/**
 * ixgbe_non_sfp_link_config - set up non-SFP+ link
 * @hw: pointer to private hardware struct
 *
 * Returns 0 on success, negative on failure
 **/
static int ixgbe_non_sfp_link_config(struct ixgbe_hw *hw)
{
	u32 speed;
	bool autoneg, link_up = false;
	u32 ret = IXGBE_ERR_LINK_SETUP;

	if (hw->mac.ops.check_link)
		ret = hw->mac.ops.check_link(hw, &speed, &link_up, false);

	if (ret)
		goto link_cfg_out;

	speed = hw->phy.autoneg_advertised;
	if (!speed && hw->mac.ops.get_link_capabilities) {
		ret = hw->mac.ops.get_link_capabilities(hw, &speed,
							&autoneg);
		speed &= ~(IXGBE_LINK_SPEED_5GB_FULL |
			   IXGBE_LINK_SPEED_2_5GB_FULL);
	}

	if (ret)
		goto link_cfg_out;

	if (hw->mac.ops.setup_link)
		ret = hw->mac.ops.setup_link(hw, speed, link_up);
link_cfg_out:
	return ret;
}

/**
 * ixgbe_clear_vf_stats_counters - Clear out VF stats after reset
 * @adapter: board private structure
 *
 * On a reset we need to clear out the VF stats or accounting gets
 * messed up because they're not clear on read.
 **/
static void ixgbe_clear_vf_stats_counters(struct ixgbe_adapter *adapter)
{
	struct ixgbe_hw *hw = &adapter->hw;
	int i;

	for (i = 0; i < adapter->num_vfs; i++) {
		adapter->vfinfo[i].last_vfstats.gprc =
			IXGBE_READ_REG(hw, IXGBE_PVFGPRC(i));
		adapter->vfinfo[i].saved_rst_vfstats.gprc +=
			adapter->vfinfo[i].vfstats.gprc;
		adapter->vfinfo[i].vfstats.gprc = 0;
		adapter->vfinfo[i].last_vfstats.gptc =
			IXGBE_READ_REG(hw, IXGBE_PVFGPTC(i));
		adapter->vfinfo[i].saved_rst_vfstats.gptc +=
			adapter->vfinfo[i].vfstats.gptc;
		adapter->vfinfo[i].vfstats.gptc = 0;
		adapter->vfinfo[i].last_vfstats.gorc =
			IXGBE_READ_REG(hw, IXGBE_PVFGORC_LSB(i));
		adapter->vfinfo[i].saved_rst_vfstats.gorc +=
			adapter->vfinfo[i].vfstats.gorc;
		adapter->vfinfo[i].vfstats.gorc = 0;
		adapter->vfinfo[i].last_vfstats.gotc =
			IXGBE_READ_REG(hw, IXGBE_PVFGOTC_LSB(i));
		adapter->vfinfo[i].saved_rst_vfstats.gotc +=
			adapter->vfinfo[i].vfstats.gotc;
		adapter->vfinfo[i].vfstats.gotc = 0;
		adapter->vfinfo[i].last_vfstats.mprc =
			IXGBE_READ_REG(hw, IXGBE_PVFMPRC(i));
		adapter->vfinfo[i].saved_rst_vfstats.mprc +=
			adapter->vfinfo[i].vfstats.mprc;
		adapter->vfinfo[i].vfstats.mprc = 0;
	}
}

static void ixgbe_setup_gpie(struct ixgbe_adapter *adapter)
{
	struct ixgbe_hw *hw = &adapter->hw;
	u32 gpie = 0;

	if (adapter->flags & IXGBE_FLAG_MSIX_ENABLED) {
		gpie = IXGBE_GPIE_MSIX_MODE | IXGBE_GPIE_PBA_SUPPORT |
		       IXGBE_GPIE_OCD;
		gpie |= IXGBE_GPIE_EIAME;
		/*
		 * use EIAM to auto-mask when MSI-X interrupt is asserted
		 * this saves a register write for every interrupt
		 */
		switch (hw->mac.type) {
		case ixgbe_mac_82598EB:
			IXGBE_WRITE_REG(hw, IXGBE_EIAM, IXGBE_EICS_RTX_QUEUE);
			break;
		case ixgbe_mac_82599EB:
		case ixgbe_mac_X540:
		case ixgbe_mac_X550:
		case ixgbe_mac_X550EM_x:
		case ixgbe_mac_X550EM_a:
		default:
			IXGBE_WRITE_REG(hw, IXGBE_EIAM_EX(0), 0xFFFFFFFF);
			IXGBE_WRITE_REG(hw, IXGBE_EIAM_EX(1), 0xFFFFFFFF);
			break;
		}
	} else {
		/* legacy interrupts, use EIAM to auto-mask when reading EICR,
		 * specifically only auto mask tx and rx interrupts */
		IXGBE_WRITE_REG(hw, IXGBE_EIAM, IXGBE_EICS_RTX_QUEUE);
	}

	if (adapter->flags & IXGBE_FLAG_SRIOV_ENABLED) {
		gpie &= ~IXGBE_GPIE_VTMODE_MASK;

		switch (adapter->ring_feature[RING_F_VMDQ].mask) {
		case IXGBE_82599_VMDQ_8Q_MASK:
			gpie |= IXGBE_GPIE_VTMODE_16;
			break;
		case IXGBE_82599_VMDQ_4Q_MASK:
			gpie |= IXGBE_GPIE_VTMODE_32;
			break;
		default:
			gpie |= IXGBE_GPIE_VTMODE_64;
			break;
		}
	}

	/* Enable Thermal over heat sensor interrupt */
	if (adapter->flags2 & IXGBE_FLAG2_TEMP_SENSOR_CAPABLE)
		switch (adapter->hw.mac.type) {
		case ixgbe_mac_82599EB:
			gpie |= IXGBE_SDP0_GPIEN;
			break;
		default:
			break;
		}

	/* Enable fan failure interrupt */
	if (adapter->flags & IXGBE_FLAG_FAN_FAIL_CAPABLE)
		gpie |= IXGBE_SDP1_GPIEN;

	switch (hw->mac.type) {
	case ixgbe_mac_82599EB:
		gpie |= IXGBE_SDP1_GPIEN | IXGBE_SDP2_GPIEN;
		break;
	case ixgbe_mac_X550EM_x:
	case ixgbe_mac_X550EM_a:
		gpie |= IXGBE_SDP0_GPIEN_X540;
		break;
	default:
		break;
	}

	IXGBE_WRITE_REG(hw, IXGBE_GPIE, gpie);
}

static void ixgbe_up_complete(struct ixgbe_adapter *adapter)
{
	struct ixgbe_hw *hw = &adapter->hw;
	int err;
	u32 ctrl_ext;

	ixgbe_get_hw_control(adapter);
	ixgbe_setup_gpie(adapter);

	if (adapter->flags & IXGBE_FLAG_MSIX_ENABLED)
		ixgbe_configure_msix(adapter);
	else
		ixgbe_configure_msi_and_legacy(adapter);

	/* enable the optics for 82599 SFP+ fiber */
	if (hw->mac.ops.enable_tx_laser)
		hw->mac.ops.enable_tx_laser(hw);
	ixgbe_set_phy_power(hw, true);

	smp_mb__before_atomic();
	clear_bit(__IXGBE_DOWN, &adapter->state);
	ixgbe_napi_enable_all(adapter);
#ifndef IXGBE_NO_LLI
	ixgbe_configure_lli(adapter);
#endif

	if (ixgbe_is_sfp(hw)) {
		ixgbe_sfp_link_config(adapter);
	} else if (!hw->phy.reset_disable) {
		err = ixgbe_non_sfp_link_config(hw);
		if (err)
			e_err(probe, "link_config FAILED %d\n", err);
	}

	/* clear any pending interrupts, may auto mask */
	IXGBE_READ_REG(hw, IXGBE_EICR);
	ixgbe_irq_enable(adapter, true, true);

	/*
	 * If this adapter has a fan, check to see if we had a failure
	 * before we enabled the interrupt.
	 */
	if (adapter->flags & IXGBE_FLAG_FAN_FAIL_CAPABLE) {
		u32 esdp = IXGBE_READ_REG(hw, IXGBE_ESDP);
		if (esdp & IXGBE_ESDP_SDP1)
			e_crit(drv, "Fan has stopped, replace the adapter\n");
	}

	/* enable transmits */
	netif_tx_start_all_queues(adapter->netdev);

	/* bring the link up in the watchdog, this could race with our first
	 * link up interrupt but shouldn't be a problem */
	adapter->flags |= IXGBE_FLAG_NEED_LINK_UPDATE;
	adapter->link_check_timeout = jiffies;
	mod_timer(&adapter->service_timer, jiffies);

	ixgbe_clear_vf_stats_counters(adapter);
	/* Set PF Reset Done bit so PF/VF Mail Ops can work */
	ctrl_ext = IXGBE_READ_REG(hw, IXGBE_CTRL_EXT);
	ctrl_ext |= IXGBE_CTRL_EXT_PFRSTD;
	IXGBE_WRITE_REG(hw, IXGBE_CTRL_EXT, ctrl_ext);
}

void ixgbe_reinit_locked(struct ixgbe_adapter *adapter)
{
	WARN_ON(in_interrupt());
	/* put off any impending NetWatchDogTimeout */
#ifdef HAVE_NETIF_TRANS_UPDATE
	netif_trans_update(adapter->netdev);
#else
	adapter->netdev->trans_start = jiffies;
#endif

	while (test_and_set_bit(__IXGBE_RESETTING, &adapter->state))
		usleep_range(1000, 2000);
	if (adapter->hw.phy.type == ixgbe_phy_fw)
		ixgbe_watchdog_link_is_down(adapter);
	ixgbe_down(adapter);
	/*
	 * If SR-IOV enabled then wait a bit before bringing the adapter
	 * back up to give the VFs time to respond to the reset.  The
	 * two second wait is based upon the watchdog timer cycle in
	 * the VF driver.
	 */
	if (adapter->flags & IXGBE_FLAG_SRIOV_ENABLED)
		msleep(2000);
	ixgbe_up(adapter);
	clear_bit(__IXGBE_RESETTING, &adapter->state);
}

void ixgbe_up(struct ixgbe_adapter *adapter)
{
	/* hardware has been reset, we need to reload some things */
	ixgbe_configure(adapter);

	ixgbe_up_complete(adapter);
}

static unsigned long ixgbe_get_completion_timeout(struct ixgbe_adapter *adapter)
{
	u16 devctl2;

	pcie_capability_read_word(adapter->pdev, PCI_EXP_DEVCTL2, &devctl2);

	switch (devctl2 & IXGBE_PCIDEVCTRL2_TIMEO_MASK) {
	case IXGBE_PCIDEVCTRL2_17_34s:
	case IXGBE_PCIDEVCTRL2_4_8s:
		/* For now we cap the upper limit on delay to 2 seconds
		 * as we end up going up to 34 seconds of delay in worst
		 * case timeout value.
		 */
	case IXGBE_PCIDEVCTRL2_1_2s:
		return 2000000ul;	/* 2.0 s */
	case IXGBE_PCIDEVCTRL2_260_520ms:
		return 520000ul;	/* 520 ms */
	case IXGBE_PCIDEVCTRL2_65_130ms:
		return 130000ul;	/* 130 ms */
	case IXGBE_PCIDEVCTRL2_16_32ms:
		return 32000ul;		/* 32 ms */
	case IXGBE_PCIDEVCTRL2_1_2ms:
		return 2000ul;		/* 2 ms */
	case IXGBE_PCIDEVCTRL2_50_100us:
		return 100ul;		/* 100 us */
	case IXGBE_PCIDEVCTRL2_16_32ms_def:
		return 32000ul;		/* 32 ms */
	default:
		break;
	}

	/* We shouldn't need to hit this path, but just in case default as
	 * though completion timeout is not supported and support 32ms.
	 */
	return 32000ul;
}

void ixgbe_disable_rx_queue(struct ixgbe_adapter *adapter)
{
	unsigned long wait_delay, delay_interval;
	struct ixgbe_hw *hw = &adapter->hw;
	int i, wait_loop;
	u32 rxdctl;

	/* disable receives */
	hw->mac.ops.disable_rx(hw);

	if (IXGBE_REMOVED(hw->hw_addr))
		return;

	/* disable all enabled Rx queues */
	for (i = 0; i < adapter->num_rx_queues; i++) {
		struct ixgbe_ring *ring = adapter->rx_ring[i];
		u8 reg_idx = ring->reg_idx;

		rxdctl = IXGBE_READ_REG(hw, IXGBE_RXDCTL(reg_idx));
		rxdctl &= ~IXGBE_RXDCTL_ENABLE;
		rxdctl |= IXGBE_RXDCTL_SWFLSH;

		/* write value back with RXDCTL.ENABLE bit cleared */
		IXGBE_WRITE_REG(hw, IXGBE_RXDCTL(reg_idx), rxdctl);
	}

	/* RXDCTL.EN may not change on 82598 if link is down, so skip it */
	if (hw->mac.type == ixgbe_mac_82598EB &&
	    !(IXGBE_READ_REG(hw, IXGBE_LINKS) & IXGBE_LINKS_UP))
		return;

	/* Determine our minimum delay interval. We will increase this value
	 * with each subsequent test. This way if the device returns quickly
	 * we should spend as little time as possible waiting, however as
	 * the time increases we will wait for larger periods of time.
	 *
	 * The trick here is that we increase the interval using the
	 * following pattern: 1x 3x 5x 7x 9x 11x 13x 15x 17x 19x. The result
	 * of that wait is that it totals up to 100x whatever interval we
	 * choose. Since our minimum wait is 100us we can just divide the
	 * total timeout by 100 to get our minimum delay interval.
	 */
	delay_interval = ixgbe_get_completion_timeout(adapter) / 100;

	wait_loop = IXGBE_MAX_RX_DESC_POLL;
	wait_delay = delay_interval;

	while (wait_loop--) {
		usleep_range(wait_delay, wait_delay + 10);
		wait_delay += delay_interval * 2;
		rxdctl = 0;

		/* OR together the reading of all the active RXDCTL registers,
		 * and then test the result. We need the disable to complete
		 * before we start freeing the memory and invalidating the
		 * DMA mappings.
		 */
		for (i = 0; i < adapter->num_rx_queues; i++) {
			struct ixgbe_ring *ring = adapter->rx_ring[i];
			u8 reg_idx = ring->reg_idx;

			rxdctl |= IXGBE_READ_REG(hw, IXGBE_RXDCTL(reg_idx));
		}

		if (!(rxdctl & IXGBE_RXDCTL_ENABLE))
			return;
	}

	e_err(drv,
	      "RXDCTL.ENABLE for one or more queues not cleared within the polling period\n");
}

void ixgbe_disable_tx_queue(struct ixgbe_adapter *adapter)
{
	unsigned long wait_delay, delay_interval;
	struct ixgbe_hw *hw = &adapter->hw;
	int i, wait_loop;
	u32 txdctl;

	if (IXGBE_REMOVED(hw->hw_addr))
		return;

	/* disable all enabled Tx queues */
	for (i = 0; i < adapter->num_tx_queues; i++) {
		struct ixgbe_ring *ring = adapter->tx_ring[i];
		u8 reg_idx = ring->reg_idx;

		IXGBE_WRITE_REG(hw, IXGBE_TXDCTL(reg_idx), IXGBE_TXDCTL_SWFLSH);
	}

	/* disable all enabled XDP Tx queues */
	for (i = 0; i < adapter->num_xdp_queues; i++) {
		struct ixgbe_ring *ring = adapter->xdp_ring[i];
		u8 reg_idx = ring->reg_idx;

		IXGBE_WRITE_REG(hw, IXGBE_TXDCTL(reg_idx), IXGBE_TXDCTL_SWFLSH);
	}

	/* If the link is not up there shouldn't be much in the way of
	 * pending transactions. Those that are left will be flushed out
	 * when the reset logic goes through the flush sequence to clean out
	 * the pending Tx transactions.
	 */
	if (!(IXGBE_READ_REG(hw, IXGBE_LINKS) & IXGBE_LINKS_UP))
		goto dma_engine_disable;

	/* Determine our minimum delay interval. We will increase this value
	 * with each subsequent test. This way if the device returns quickly
	 * we should spend as little time as possible waiting, however as
	 * the time increases we will wait for larger periods of time.
	 *
	 * The trick here is that we increase the interval using the
	 * following pattern: 1x 3x 5x 7x 9x 11x 13x 15x 17x 19x. The result
	 * of that wait is that it totals up to 100x whatever interval we
	 * choose. Since our minimum wait is 100us we can just divide the
	 * total timeout by 100 to get our minimum delay interval.
	 */
	delay_interval = ixgbe_get_completion_timeout(adapter) / 100;

	wait_loop = IXGBE_MAX_RX_DESC_POLL;
	wait_delay = delay_interval;

	while (wait_loop--) {
		usleep_range(wait_delay, wait_delay + 10);
		wait_delay += delay_interval * 2;
		txdctl = 0;

		/* OR together the reading of all the active TXDCTL registers,
		 * and then test the result. We need the disable to complete
		 * before we start freeing the memory and invalidating the
		 * DMA mappings.
		 */
		for (i = 0; i < adapter->num_tx_queues; i++) {
			struct ixgbe_ring *ring = adapter->tx_ring[i];
			u8 reg_idx = ring->reg_idx;

			txdctl |= IXGBE_READ_REG(hw, IXGBE_TXDCTL(reg_idx));
		}
		for (i = 0; i < adapter->num_xdp_queues; i++) {
			struct ixgbe_ring *ring = adapter->xdp_ring[i];
			u8 reg_idx = ring->reg_idx;

			txdctl |= IXGBE_READ_REG(hw, IXGBE_TXDCTL(reg_idx));
		}

		if (!(txdctl & IXGBE_TXDCTL_ENABLE))
			goto dma_engine_disable;
	}

	e_err(drv,
	      "TXDCTL.ENABLE for one or more queues not cleared within the polling period\n");

dma_engine_disable:
	/* Disable the Tx DMA engine on 82599 and later MAC */
	switch (hw->mac.type) {
	case ixgbe_mac_82599EB:
	case ixgbe_mac_X540:
	case ixgbe_mac_X550:
	case ixgbe_mac_X550EM_x:
	case ixgbe_mac_X550EM_a:
		IXGBE_WRITE_REG(hw, IXGBE_DMATXCTL,
				(IXGBE_READ_REG(hw, IXGBE_DMATXCTL) &
				 ~IXGBE_DMATXCTL_TE));
		/* fall through */
	default:
		break;
	}
}

void ixgbe_reset(struct ixgbe_adapter *adapter)
{
	struct ixgbe_hw *hw = &adapter->hw;
#ifdef HAVE_SET_RX_MODE
	struct net_device *netdev = adapter->netdev;
#endif
	int err;

	if (IXGBE_REMOVED(hw->hw_addr))
		return;
	/* lock SFP init bit to prevent race conditions with the watchdog */
	while (test_and_set_bit(__IXGBE_IN_SFP_INIT, &adapter->state))
		usleep_range(1000, 2000);

	/* clear all SFP and link config related flags while holding SFP_INIT */
	adapter->flags2 &= ~(IXGBE_FLAG2_SEARCH_FOR_SFP |
			     IXGBE_FLAG2_SFP_NEEDS_RESET);
	adapter->flags &= ~IXGBE_FLAG_NEED_LINK_CONFIG;

	err = hw->mac.ops.init_hw(hw);
	switch (err) {
	case IXGBE_SUCCESS:
	case IXGBE_ERR_SFP_NOT_PRESENT:
	case IXGBE_ERR_SFP_NOT_SUPPORTED:
		break;
	case IXGBE_ERR_MASTER_REQUESTS_PENDING:
		e_dev_err("master disable timed out\n");
		break;
	case IXGBE_ERR_EEPROM_VERSION:
		/* We are running on a pre-production device, log a warning */
		e_dev_warn("This device is a pre-production adapter/LOM. "
			   "Please be aware there may be issues associated "
			   "with your hardware.  If you are experiencing "
			   "problems please contact your Intel or hardware "
			   "representative who provided you with this "
			   "hardware.\n");
		break;
	case IXGBE_ERR_OVERTEMP:
		e_crit(drv, "%s\n", ixgbe_overheat_msg);
		break;
	default:
		e_dev_err("Hardware Error: %d\n", err);
	}

	clear_bit(__IXGBE_IN_SFP_INIT, &adapter->state);

	/* flush entries out of MAC table */
	ixgbe_flush_sw_mac_table(adapter);
#ifdef HAVE_SET_RX_MODE
	__dev_uc_unsync(netdev, NULL);
#endif

	/* do not flush user set addresses */
	ixgbe_mac_set_default_filter(adapter);

	/* update SAN MAC vmdq pool selection */
	if (hw->mac.san_mac_rar_index)
		hw->mac.ops.set_vmdq_san_mac(hw, VMDQ_P(0));

	/* Clear saved DMA coalescing values except for watchdog_timer */
	hw->mac.dmac_config.fcoe_en = false;
	hw->mac.dmac_config.link_speed = 0;
	hw->mac.dmac_config.fcoe_tc = 0;
	hw->mac.dmac_config.num_tcs = 0;

#ifdef HAVE_PTP_1588_CLOCK
	if (test_bit(__IXGBE_PTP_RUNNING, &adapter->state))
		ixgbe_ptp_reset(adapter);
#endif

	if (!netif_running(adapter->netdev) && !adapter->wol)
		ixgbe_set_phy_power(hw, false);
	else
		ixgbe_set_phy_power(hw, true);
}

/**
 * ixgbe_clean_tx_ring - Free Tx Buffers
 * @tx_ring: ring to be cleaned
 **/
static void ixgbe_clean_tx_ring(struct ixgbe_ring *tx_ring)
{
	u16 i = tx_ring->next_to_clean;
	struct ixgbe_tx_buffer *tx_buffer = &tx_ring->tx_buffer_info[i];

#ifdef HAVE_AF_XDP_ZC_SUPPORT
	if (tx_ring->xsk_umem) {
		ixgbe_xsk_clean_tx_ring(tx_ring);
		goto out;
	}

#endif
	while (i != tx_ring->next_to_use) {
		union ixgbe_adv_tx_desc *eop_desc, *tx_desc;

		/* Free all the Tx ring sk_buffs */
#ifdef HAVE_XDP_SUPPORT
		if (ring_is_xdp(tx_ring))
#ifdef HAVE_XDP_FRAME_STRUCT
			xdp_return_frame(tx_buffer->xdpf);
#else
			page_frag_free(tx_buffer->data);
#endif
		else
			dev_kfree_skb_any(tx_buffer->skb);
#else
		dev_kfree_skb_any(tx_buffer->skb);
#endif

		/* unmap skb header data */
		dma_unmap_single(tx_ring->dev,
				 dma_unmap_addr(tx_buffer, dma),
				 dma_unmap_len(tx_buffer, len),
				 DMA_TO_DEVICE);

		/* check for eop_desc to determine the end of the packet */
		eop_desc = tx_buffer->next_to_watch;
		tx_desc = IXGBE_TX_DESC(tx_ring, i);

		/* unmap remaining buffers */
		while (tx_desc != eop_desc) {
			tx_buffer++;
			tx_desc++;
			i++;
			if (unlikely(i == tx_ring->count)) {
				i = 0;
				tx_buffer = tx_ring->tx_buffer_info;
				tx_desc = IXGBE_TX_DESC(tx_ring, 0);
			}

			/* unmap any remaining paged data */
			if (dma_unmap_len(tx_buffer, len))
				dma_unmap_page(tx_ring->dev,
					       dma_unmap_addr(tx_buffer, dma),
					       dma_unmap_len(tx_buffer, len),
					       DMA_TO_DEVICE);
		}

		/* move us one more past the eop_desc for start of next pkt */
		tx_buffer++;
		i++;
		if (unlikely(i == tx_ring->count)) {
			i = 0;
			tx_buffer = tx_ring->tx_buffer_info;
		}
	}

	/* reset BQL for queue */
	if (!ring_is_xdp(tx_ring))
		netdev_tx_reset_queue(txring_txq(tx_ring));

#ifdef HAVE_AF_XDP_ZC_SUPPORT
out:
#endif

	/* reset next_to_use and next_to_clean */
	tx_ring->next_to_use = 0;
	tx_ring->next_to_clean = 0;
}

/**
 * ixgbe_clean_all_rx_rings - Free Rx Buffers for all queues
 * @adapter: board private structure
 **/
static void ixgbe_clean_all_rx_rings(struct ixgbe_adapter *adapter)
{
	int i;

	for (i = 0; i < adapter->num_rx_queues; i++)
		ixgbe_clean_rx_ring(adapter->rx_ring[i]);
}

/**
 * ixgbe_clean_all_tx_rings - Free Tx Buffers for all queues
 * @adapter: board private structure
 **/
static void ixgbe_clean_all_tx_rings(struct ixgbe_adapter *adapter)
{
	int i;

	for (i = 0; i < adapter->num_tx_queues; i++)
		ixgbe_clean_tx_ring(adapter->tx_ring[i]);
	for (i = 0; i < adapter->num_xdp_queues; i++)
		ixgbe_clean_tx_ring(adapter->xdp_ring[i]);
}

static void ixgbe_fdir_filter_exit(struct ixgbe_adapter *adapter)
{
	struct hlist_node *node2;
	struct ixgbe_fdir_filter *filter;

	spin_lock(&adapter->fdir_perfect_lock);

	hlist_for_each_entry_safe(filter, node2,
				  &adapter->fdir_filter_list, fdir_node) {
		hlist_del(&filter->fdir_node);
		kfree(filter);
	}
	adapter->fdir_filter_count = 0;

	spin_unlock(&adapter->fdir_perfect_lock);
}

void ixgbe_down(struct ixgbe_adapter *adapter)
{
	struct net_device *netdev = adapter->netdev;
	struct ixgbe_hw *hw = &adapter->hw;
	int i;

	/* signal that we are down to the interrupt handler */
	if (test_and_set_bit(__IXGBE_DOWN, &adapter->state))
		return; /* do nothing if already down */

	/* Shut off incoming Tx traffic */
	netif_tx_stop_all_queues(netdev);

	/* call carrier off first to avoid false dev_watchdog timeouts */
	netif_carrier_off(netdev);
	netif_tx_disable(netdev);

	/* Disable Rx */
	ixgbe_disable_rx_queue(adapter);

	/* synchronize_rcu() needed for pending XDP buffers to drain */
	if (adapter->xdp_ring[0])
		synchronize_rcu();

	ixgbe_irq_disable(adapter);

	ixgbe_napi_disable_all(adapter);

	adapter->flags2 &= ~(IXGBE_FLAG2_FDIR_REQUIRES_REINIT);
	clear_bit(__IXGBE_RESET_REQUESTED, &adapter->state);
	adapter->flags &= ~IXGBE_FLAG_NEED_LINK_UPDATE;

	del_timer_sync(&adapter->service_timer);

	if (adapter->num_vfs) {
		/* Clear EITR Select mapping */
		IXGBE_WRITE_REG(&adapter->hw, IXGBE_EITRSEL, 0);

		/* Mark all the VFs as inactive */
		for (i = 0 ; i < adapter->num_vfs; i++)
			adapter->vfinfo[i].clear_to_send = 0;

		/* ping all the active vfs to let them know we are going down */
		ixgbe_ping_all_vfs(adapter);

		/* Disable all VFTE/VFRE TX/RX */
		ixgbe_disable_tx_rx(adapter);
	}

	/* disable transmits in the hardware now that interrupts are off */
	ixgbe_disable_tx_queue(adapter);

#ifdef HAVE_PCI_ERS
	if (!pci_channel_offline(adapter->pdev))
#endif
		ixgbe_reset(adapter);

	/* power down the optics for 82599 SFP+ fiber */
	if (hw->mac.ops.disable_tx_laser)
		hw->mac.ops.disable_tx_laser(hw);

	ixgbe_clean_all_tx_rings(adapter);
	ixgbe_clean_all_rx_rings(adapter);
}

/**
 * ixgbe_eee_capable - helper function to determine EEE support on X550
 * @adapter: board private structure to initialize
 **/
static inline void ixgbe_set_eee_capable(struct ixgbe_adapter *adapter)
{
	struct ixgbe_hw *hw = &adapter->hw;

	switch (hw->device_id) {
	case IXGBE_DEV_ID_X550EM_A_1G_T:
	case IXGBE_DEV_ID_X550EM_A_1G_T_L:
		if (!hw->phy.eee_speeds_supported)
			break;
		adapter->flags2 |= IXGBE_FLAG2_EEE_CAPABLE;
		if (!hw->phy.eee_speeds_advertised)
			break;
		adapter->flags2 |= IXGBE_FLAG2_EEE_ENABLED;
		break;
	default:
		adapter->flags2 &= ~IXGBE_FLAG2_EEE_CAPABLE;
		adapter->flags2 &= ~IXGBE_FLAG2_EEE_ENABLED;
		break;
	}
}

#if IS_ENABLED(CONFIG_DCB)
static void ixgbe_init_dcb(struct ixgbe_adapter *adapter)
{
	struct ixgbe_dcb_tc_config *tc;
	int j, bwg_pct;

	/* Configure DCB traffic classes */
	bwg_pct = 100 / adapter->dcb_cfg.num_tcs.pg_tcs;
	for (j = 0; j < adapter->dcb_cfg.num_tcs.pg_tcs; j++) {
		tc = &adapter->dcb_cfg.tc_config[j];
		tc->path[IXGBE_DCB_TX_CONFIG].bwg_id = 0;
		tc->path[IXGBE_DCB_TX_CONFIG].bwg_percent = bwg_pct;
		tc->path[IXGBE_DCB_RX_CONFIG].bwg_id = 0;
		tc->path[IXGBE_DCB_RX_CONFIG].bwg_percent = bwg_pct;
		tc->pfc = ixgbe_dcb_pfc_disabled;
	}

	/* reset back to TC 0 */
	tc = &adapter->dcb_cfg.tc_config[0];

	/* total of all TCs bandwidth needs to be 100 */
	bwg_pct += 100 % adapter->dcb_cfg.num_tcs.pg_tcs;
	tc->path[IXGBE_DCB_TX_CONFIG].bwg_percent = bwg_pct;
	tc->path[IXGBE_DCB_RX_CONFIG].bwg_percent = bwg_pct;

	/* Initialize default user to priority mapping, UPx->TC0 */
	tc->path[IXGBE_DCB_TX_CONFIG].up_to_tc_bitmap = 0xFF;
	tc->path[IXGBE_DCB_RX_CONFIG].up_to_tc_bitmap = 0xFF;

	adapter->dcb_cfg.bw_percentage[IXGBE_DCB_TX_CONFIG][0] = 100;
	adapter->dcb_cfg.bw_percentage[IXGBE_DCB_RX_CONFIG][0] = 100;
	adapter->dcb_cfg.rx_pba_cfg = ixgbe_dcb_pba_equal;
	adapter->dcb_cfg.pfc_mode_enable = false;
	adapter->dcb_cfg.round_robin_enable = false;
	adapter->dcb_set_bitmap = 0x00;
	if (adapter->flags & IXGBE_FLAG_DCB_CAPABLE)
		adapter->dcbx_cap = DCB_CAP_DCBX_HOST | DCB_CAP_DCBX_VER_CEE;
	memcpy(&adapter->temp_dcb_cfg, &adapter->dcb_cfg,
	       sizeof(adapter->temp_dcb_cfg));
}
#endif /*CONFIG_DCB*/

/**
 * ixgbe_sw_init - Initialize general software structures (struct ixgbe_adapter)
 * @adapter: board private structure to initialize
 *
 * ixgbe_sw_init initializes the Adapter private data structure.
 * Fields are initialized based on PCI device information and
 * OS network device settings (MTU size).
 **/
#ifdef HAVE_CONFIG_HOTPLUG
static int __devinit ixgbe_sw_init(struct ixgbe_adapter *adapter)
#else
static int ixgbe_sw_init(struct ixgbe_adapter *adapter)
#endif
{
	struct ixgbe_hw *hw = &adapter->hw;
	struct pci_dev *pdev = adapter->pdev;
	int err;
	unsigned int fdir;
	u32 fwsm;
#ifdef HAVE_TC_SETUP_CLSU32
	int i;
#endif

	/* PCI config space info */

	hw->vendor_id = pdev->vendor;
	hw->device_id = pdev->device;
	pci_read_config_byte(pdev, PCI_REVISION_ID, &hw->revision_id);
	if (hw->revision_id == IXGBE_FAILED_READ_CFG_BYTE &&
	    ixgbe_check_cfg_remove(hw, pdev)) {
		e_err(probe, "read of revision id failed\n");
		err = -ENODEV;
		goto out;
	}
	hw->subsystem_vendor_id = pdev->subsystem_vendor;
	hw->subsystem_device_id = pdev->subsystem_device;

	err = ixgbe_init_shared_code(hw);
	if (err) {
		e_err(probe, "init_shared_code failed: %d\n", err);
		goto out;
	}
#ifdef HAVE_TC_SETUP_CLSU32
	/* initialize static ixgbe jump table entries */
	adapter->jump_tables[0] = kzalloc(sizeof(*adapter->jump_tables[0]),
					  GFP_KERNEL);
	if (!adapter->jump_tables[0])
		return -ENOMEM;
	adapter->jump_tables[0]->mat = ixgbe_ipv4_fields;

	for (i = 1; i < IXGBE_MAX_LINK_HANDLE; i++)
		adapter->jump_tables[i] = NULL;
#endif /* HAVE_TC_SETUP_CLSU32 */
	adapter->mac_table = kzalloc(sizeof(struct ixgbe_mac_addr) *
				     hw->mac.num_rar_entries,
				     GFP_KERNEL);
	if (!adapter->mac_table) {
		err = IXGBE_ERR_OUT_OF_MEM;
		e_err(probe, "mac_table allocation failed: %d\n", err);
		goto out;
	}

	if (ixgbe_init_rss_key(adapter)) {
		err = IXGBE_ERR_OUT_OF_MEM;
		e_err(probe, "rss_key allocation failed: %d\n", err);
		goto out;
	}

	/* Set common capability flags and settings */
#if IS_ENABLED(CONFIG_DCA)
	adapter->flags |= IXGBE_FLAG_DCA_CAPABLE;
#endif
#if IS_ENABLED(CONFIG_DCB)
	adapter->flags |= IXGBE_FLAG_DCB_CAPABLE;
	adapter->flags &= ~IXGBE_FLAG_DCB_ENABLED;
#endif
#if IS_ENABLED(CONFIG_FCOE)
	adapter->flags |= IXGBE_FLAG_FCOE_CAPABLE;
	adapter->flags &= ~IXGBE_FLAG_FCOE_ENABLED;
#if IS_ENABLED(CONFIG_DCB)
	/* Default traffic class to use for FCoE */
	adapter->fcoe.up = IXGBE_FCOE_DEFUP;
	adapter->fcoe.up_set = IXGBE_FCOE_DEFUP;
#endif /* CONFIG_DCB */
#endif /* CONFIG_FCOE */
	adapter->flags2 |= IXGBE_FLAG2_RSC_CAPABLE;
	fdir = min_t(int, IXGBE_MAX_FDIR_INDICES, num_online_cpus());
	adapter->ring_feature[RING_F_FDIR].limit = fdir;
	adapter->max_q_vectors = IXGBE_MAX_MSIX_Q_VECTORS_82599;

	/* Set MAC specific capability flags and exceptions */
	switch (hw->mac.type) {
	case ixgbe_mac_82598EB:
		adapter->flags |= IXGBE_FLAGS_82598_INIT;
		adapter->flags2 &= ~IXGBE_FLAG2_RSC_CAPABLE;

		if (hw->device_id == IXGBE_DEV_ID_82598AT)
			adapter->flags |= IXGBE_FLAG_FAN_FAIL_CAPABLE;

		adapter->max_q_vectors = IXGBE_MAX_MSIX_Q_VECTORS_82598;
		adapter->ring_feature[RING_F_FDIR].limit = 0;
#if IS_ENABLED(CONFIG_FCOE)
		adapter->flags &= ~IXGBE_FLAG_FCOE_CAPABLE;
#if IS_ENABLED(CONFIG_DCB)
		adapter->fcoe.up = 0;
		adapter->fcoe.up_set = 0;
#endif /* IXGBE_DCB */
#endif /* CONFIG_FCOE */
		break;
	case ixgbe_mac_82599EB:
		adapter->flags |= IXGBE_FLAGS_82599_INIT;
		if (hw->device_id == IXGBE_DEV_ID_82599_T3_LOM)
			adapter->flags2 |= IXGBE_FLAG2_TEMP_SENSOR_CAPABLE;
#ifndef IXGBE_NO_SMART_SPEED
		hw->phy.smart_speed = ixgbe_smart_speed_on;
#else
		hw->phy.smart_speed = ixgbe_smart_speed_off;
#endif
		break;
	case ixgbe_mac_X540:
		adapter->flags |= IXGBE_FLAGS_X540_INIT;
		fwsm = IXGBE_READ_REG(hw, IXGBE_FWSM);
		if (fwsm & IXGBE_FWSM_TS_ENABLED)
			adapter->flags2 |= IXGBE_FLAG2_TEMP_SENSOR_CAPABLE;
		break;
	case ixgbe_mac_X550EM_a:
		adapter->flags |= IXGBE_FLAG_GENEVE_OFFLOAD_CAPABLE;
		switch (hw->device_id) {
		case IXGBE_DEV_ID_X550EM_A_1G_T:
		case IXGBE_DEV_ID_X550EM_A_1G_T_L:
			adapter->flags2 |= IXGBE_FLAG2_TEMP_SENSOR_CAPABLE;
			break;
		default:
			break;
		}
		/* fall through */
	case ixgbe_mac_X550EM_x:
#if IS_ENABLED(CONFIG_DCB)
		adapter->flags &= ~IXGBE_FLAG_DCB_CAPABLE;
#endif
#if IS_ENABLED(CONFIG_FCOE)
		adapter->flags &= ~IXGBE_FLAG_FCOE_CAPABLE;
#if IS_ENABLED(CONFIG_DCB)
		adapter->fcoe.up = 0;
		adapter->fcoe.up_set = 0;
#endif /* CONFIG_DCB */
#endif /* CONFIG_FCOE */
	/* fall through */
	case ixgbe_mac_X550:
		if (hw->mac.type == ixgbe_mac_X550)
			adapter->flags2 |= IXGBE_FLAG2_TEMP_SENSOR_CAPABLE;
		ixgbe_set_eee_capable(adapter);
		adapter->flags |= IXGBE_FLAGS_X550_INIT;
#if IS_ENABLED(CONFIG_DCA)
		adapter->flags &= ~IXGBE_FLAG_DCA_CAPABLE;
#endif /* CONFIG_DCA */
	default:
		break;
	}

#if IS_ENABLED(CONFIG_FCOE)
	/* FCoE support exists, always init the FCoE lock */
	spin_lock_init(&adapter->fcoe.lock);
#endif /* CONFIG_FCOE */

	/* n-tuple support exists, always init our spinlock */
	spin_lock_init(&adapter->fdir_perfect_lock);

#if IS_ENABLED(CONFIG_DCB)
	switch (hw->mac.type) {
	case ixgbe_mac_82598EB:
	case ixgbe_mac_82599EB:
		adapter->dcb_cfg.num_tcs.pg_tcs = 8;
		adapter->dcb_cfg.num_tcs.pfc_tcs = 8;
		break;
	case ixgbe_mac_X540:
	case ixgbe_mac_X550:
		adapter->dcb_cfg.num_tcs.pg_tcs = 4;
		adapter->dcb_cfg.num_tcs.pfc_tcs = 4;
		break;
	case ixgbe_mac_X550EM_x:
	case ixgbe_mac_X550EM_a:
	default:
		adapter->dcb_cfg.num_tcs.pg_tcs = 1;
		adapter->dcb_cfg.num_tcs.pfc_tcs = 1;
		break;
	}
	ixgbe_init_dcb(adapter);

#endif /* CONFIG_DCB */

	if (hw->mac.type == ixgbe_mac_82599EB ||
	    hw->mac.type == ixgbe_mac_X550 ||
	    hw->mac.type == ixgbe_mac_X550EM_x ||
	    hw->mac.type == ixgbe_mac_X550EM_a ||
	    hw->mac.type == ixgbe_mac_X540)
		ixgbe_init_mbx_params_pf(hw);

	/* default flow control settings */
	hw->fc.requested_mode = ixgbe_fc_full;
	hw->fc.current_mode = ixgbe_fc_full;	/* init for ethtool output */

	adapter->last_lfc_mode = hw->fc.current_mode;
	ixgbe_pbthresh_setup(adapter);
	hw->fc.pause_time = IXGBE_DEFAULT_FCPAUSE;
	hw->fc.send_xon = true;
	hw->fc.disable_fc_autoneg = false;

	/* set default ring sizes */
	adapter->tx_ring_count = IXGBE_DEFAULT_TXD;
	adapter->rx_ring_count = IXGBE_DEFAULT_RXD;

	/* set default work limits */
	adapter->tx_work_limit = IXGBE_DEFAULT_TX_WORK;

	set_bit(__IXGBE_DOWN, &adapter->state);
out:
	return err;
}

/**
 * ixgbe_setup_tx_resources - allocate Tx resources (Descriptors)
 * @tx_ring:    tx descriptor ring (for a specific queue) to setup
 *
 * Return 0 on success, negative on failure
 **/
int ixgbe_setup_tx_resources(struct ixgbe_ring *tx_ring)
{
	struct device *dev = tx_ring->dev;
	int orig_node = dev_to_node(dev);
	int node = -1;
	int size;

	size = sizeof(struct ixgbe_tx_buffer) * tx_ring->count;

	if (tx_ring->q_vector)
		node = tx_ring->q_vector->node;

	tx_ring->tx_buffer_info = vmalloc_node(size, node);
	if (!tx_ring->tx_buffer_info)
		tx_ring->tx_buffer_info = vmalloc(size);
	if (!tx_ring->tx_buffer_info)
		goto err;

	/* round up to nearest 4K */
	tx_ring->size = tx_ring->count * sizeof(union ixgbe_adv_tx_desc);
	tx_ring->size = ALIGN(tx_ring->size, 4096);

	set_dev_node(dev, node);
	tx_ring->desc = dma_alloc_coherent(dev,
					   tx_ring->size,
					   &tx_ring->dma,
					   GFP_KERNEL);
	set_dev_node(dev, orig_node);
	if (!tx_ring->desc)
		tx_ring->desc = dma_alloc_coherent(dev, tx_ring->size,
						   &tx_ring->dma, GFP_KERNEL);
	if (!tx_ring->desc)
		goto err;

	return 0;

err:
	vfree(tx_ring->tx_buffer_info);
	tx_ring->tx_buffer_info = NULL;
	dev_err(dev, "Unable to allocate memory for the Tx descriptor ring\n");
	return -ENOMEM;
}

/**
 * ixgbe_setup_all_tx_resources - allocate all queues Tx resources
 * @adapter: board private structure
 *
 * If this function returns with an error, then it's possible one or
 * more of the rings is populated (while the rest are not).  It is the
 * callers duty to clean those orphaned rings.
 *
 * Return 0 on success, negative on failure
 **/
static int ixgbe_setup_all_tx_resources(struct ixgbe_adapter *adapter)
{
	int i, j = 0, err = 0;

	for (i = 0; i < adapter->num_tx_queues; i++) {

		err = ixgbe_setup_tx_resources(adapter->tx_ring[i]);
		if (!err)
			continue;

		e_err(probe, "Allocation for Tx Queue %u failed\n", i);
		goto err_setup_tx;
	}
	for (j = 0; j < adapter->num_xdp_queues; j++) {
		err = ixgbe_setup_tx_resources(adapter->xdp_ring[j]);
		if (!err)
			continue;

		e_err(probe, "Allocation for Tx Queue %u failed\n", j);
		goto err_setup_tx;
	}

	return 0;
err_setup_tx:
	/* rewind the index freeing the rings as we go */
	while (j--)
		ixgbe_free_tx_resources(adapter->xdp_ring[j]);
	while (i--)
		ixgbe_free_tx_resources(adapter->tx_ring[i]);
	return err;
}

/**
 * ixgbe_setup_rx_resources - allocate Rx resources (Descriptors)
 * @adapter: board private structure
 * @rx_ring: rx descriptor ring (for a specific queue) to setup
 *
 * Returns 0 on success, negative on failure
 **/
int ixgbe_setup_rx_resources(struct ixgbe_adapter *adapter,
			     struct ixgbe_ring *rx_ring)
{
	struct device *dev = rx_ring->dev;
	int orig_node = dev_to_node(dev);
	int node = -1;
	int size;
#ifdef HAVE_XDP_BUFF_RXQ
#ifdef HAVE_XDP_FRAME_STRUCT
#ifndef HAVE_AF_XDP_ZC_SUPPORT
	int err;
#endif /* HAVE_AF_XDP_ZC_SUPPORT */
#endif /* HAVE_XDP_FRAME_STRUCT */
#endif /* HAVE_XDP_BUFF_RXQ */

	size = sizeof(struct ixgbe_rx_buffer) * rx_ring->count;

	if (rx_ring->q_vector)
		node = rx_ring->q_vector->node;

	rx_ring->rx_buffer_info = vmalloc_node(size, node);
	if (!rx_ring->rx_buffer_info)
		rx_ring->rx_buffer_info = vmalloc(size);
	if (!rx_ring->rx_buffer_info)
		goto err;

	/* Round up to nearest 4K */
	rx_ring->size = rx_ring->count * sizeof(union ixgbe_adv_rx_desc);
	rx_ring->size = ALIGN(rx_ring->size, 4096);

	set_dev_node(dev, node);
	rx_ring->desc = dma_alloc_coherent(dev,
					   rx_ring->size,
					   &rx_ring->dma,
					   GFP_KERNEL);
	set_dev_node(dev, orig_node);
	if (!rx_ring->desc)
		rx_ring->desc = dma_alloc_coherent(dev, rx_ring->size,
						   &rx_ring->dma, GFP_KERNEL);
	if (!rx_ring->desc)
		goto err;

#ifdef HAVE_XDP_BUFF_RXQ
	/* XDP RX-queue info */
	if (xdp_rxq_info_reg(&rx_ring->xdp_rxq, adapter->netdev,
			     rx_ring->queue_index) < 0)
		goto err;

#ifndef HAVE_AF_XDP_ZC_SUPPORT
#ifdef HAVE_XDP_FRAME_STRUCT
	err = xdp_rxq_info_reg_mem_model(&rx_ring->xdp_rxq,
					 MEM_TYPE_PAGE_SHARED, NULL);
	if (err) {
		xdp_rxq_info_unreg(&rx_ring->xdp_rxq);
		goto err;
	}
#endif /* HAVE_XDP_FRAME_STRUCT */
#endif /* HAVE_AF_XDP_ZC_SUPPORT */
#endif /* HAVE_XDP_BUFF_RXQ */

	rx_ring->xdp_prog = adapter->xdp_prog;

	return 0;
err:
	vfree(rx_ring->rx_buffer_info);
	rx_ring->rx_buffer_info = NULL;
	dev_err(dev, "Unable to allocate memory for the Rx descriptor ring\n");
	return -ENOMEM;
}

/**
 * ixgbe_setup_all_rx_resources - allocate all queues Rx resources
 * @adapter: board private structure
 *
 * If this function returns with an error, then it's possible one or
 * more of the rings is populated (while the rest are not).  It is the
 * callers duty to clean those orphaned rings.
 *
 * Return 0 on success, negative on failure
 **/
static int ixgbe_setup_all_rx_resources(struct ixgbe_adapter *adapter)
{
	int i, err = 0;

	for (i = 0; i < adapter->num_rx_queues; i++) {
		err = ixgbe_setup_rx_resources(adapter, adapter->rx_ring[i]);
		if (!err)
			continue;

		e_err(probe, "Allocation for Rx Queue %u failed\n", i);
		goto err_setup_rx;
	}

#if IS_ENABLED(CONFIG_FCOE)
	err = ixgbe_setup_fcoe_ddp_resources(adapter);
	if (!err)
#endif
		return 0;
err_setup_rx:
	/* rewind the index freeing the rings as we go */
	while (i--)
		ixgbe_free_rx_resources(adapter->rx_ring[i]);
	return err;
}

/**
 * ixgbe_free_tx_resources - Free Tx Resources per Queue
 * @tx_ring: Tx descriptor ring for a specific queue
 *
 * Free all transmit software resources
 **/
void ixgbe_free_tx_resources(struct ixgbe_ring *tx_ring)
{
	ixgbe_clean_tx_ring(tx_ring);

	vfree(tx_ring->tx_buffer_info);
	tx_ring->tx_buffer_info = NULL;

	/* if not set, then don't free */
	if (!tx_ring->desc)
		return;

	dma_free_coherent(tx_ring->dev, tx_ring->size,
			  tx_ring->desc, tx_ring->dma);
	tx_ring->desc = NULL;
}

/**
 * ixgbe_free_all_tx_resources - Free Tx Resources for All Queues
 * @adapter: board private structure
 *
 * Free all transmit software resources
 **/
static void ixgbe_free_all_tx_resources(struct ixgbe_adapter *adapter)
{
	int i;

	for (i = 0; i < adapter->num_tx_queues; i++)
		ixgbe_free_tx_resources(adapter->tx_ring[i]);
	for (i = 0; i < adapter->num_xdp_queues; i++)
		if (adapter->xdp_ring[i]->desc)
			ixgbe_free_tx_resources(adapter->xdp_ring[i]);
}

/**
 * ixgbe_free_rx_resources - Free Rx Resources
 * @rx_ring: ring to clean the resources from
 *
 * Free all receive software resources
 **/
void ixgbe_free_rx_resources(struct ixgbe_ring *rx_ring)
{
	ixgbe_clean_rx_ring(rx_ring);

	rx_ring->xdp_prog = NULL;
#ifdef HAVE_XDP_BUFF_RXQ
	xdp_rxq_info_unreg(&rx_ring->xdp_rxq);
#endif
	vfree(rx_ring->rx_buffer_info);
	rx_ring->rx_buffer_info = NULL;

	/* if not set, then don't free */
	if (!rx_ring->desc)
		return;

	dma_free_coherent(rx_ring->dev, rx_ring->size,
			  rx_ring->desc, rx_ring->dma);

	rx_ring->desc = NULL;
}

/**
 * ixgbe_free_all_rx_resources - Free Rx Resources for All Queues
 * @adapter: board private structure
 *
 * Free all receive software resources
 **/
static void ixgbe_free_all_rx_resources(struct ixgbe_adapter *adapter)
{
	int i;

#if IS_ENABLED(CONFIG_FCOE)
	ixgbe_free_fcoe_ddp_resources(adapter);
#endif

	for (i = 0; i < adapter->num_rx_queues; i++)
		ixgbe_free_rx_resources(adapter->rx_ring[i]);
}

/**
 * ixgbe_change_mtu - Change the Maximum Transfer Unit
 * @netdev: network interface device structure
 * @new_mtu: new value for maximum frame size
 *
 * Returns 0 on success, negative on failure
 **/
static int ixgbe_change_mtu(struct net_device *netdev, int new_mtu)
{
	struct ixgbe_adapter *adapter = netdev_priv(netdev);
#ifndef HAVE_NETDEVICE_MIN_MAX_MTU
	int max_frame = new_mtu + ETH_HLEN + ETH_FCS_LEN;
#endif

	if (adapter->xdp_prog) {
		int new_frame_size = new_mtu + ETH_HLEN + ETH_FCS_LEN +
				     VLAN_HLEN;
		int i;

		for (i = 0; i < adapter->num_rx_queues; i++) {
			struct ixgbe_ring *ring = adapter->rx_ring[i];

			if (new_frame_size > ixgbe_rx_bufsz(ring)) {
				e_warn(probe, "Requested MTU size is not supported with XDP\n");
				return -EINVAL;
			}
		}
	}

#ifndef HAVE_NETDEVICE_MIN_MAX_MTU
	/* MTU < 68 is an error and causes problems on some kernels */
	if ((new_mtu < 68) || (max_frame > IXGBE_MAX_JUMBO_FRAME_SIZE))
		return -EINVAL;

#endif
	/*
	 * For 82599EB we cannot allow legacy VFs to enable their receive
	 * paths when MTU greater than 1500 is configured.  So display a
	 * warning that legacy VFs will be disabled.
	 */
	if ((adapter->flags & IXGBE_FLAG_SRIOV_ENABLED) &&
	    (adapter->hw.mac.type == ixgbe_mac_82599EB) &&
#ifndef HAVE_NETDEVICE_MIN_MAX_MTU
	    (max_frame > (ETH_FRAME_LEN + ETH_FCS_LEN)))
#else
	    (new_mtu > ETH_DATA_LEN))
#endif
		e_warn(probe, "Setting MTU > 1500 will disable legacy VFs\n");

	e_info(probe, "changing MTU from %d to %d\n", netdev->mtu, new_mtu);

	/* must set new MTU before calling down or up */
	netdev->mtu = new_mtu;

	if (netif_running(netdev))
		ixgbe_reinit_locked(adapter);

	return 0;
}

/**
 * ixgbe_open - Called when a network interface is made active
 * @netdev: network interface device structure
 *
 * Returns 0 on success, negative value on failure
 *
 * The open entry point is called when a network interface is made
 * active by the system (IFF_UP).  At this point all resources needed
 * for transmit and receive operations are allocated, the interrupt
 * handler is registered with the OS, the watchdog timer is started,
 * and the stack is notified that the interface is ready.
 **/
int ixgbe_open(struct net_device *netdev)
{
	struct ixgbe_adapter *adapter = netdev_priv(netdev);
	int err;

	/* disallow open during test */
	if (test_bit(__IXGBE_TESTING, &adapter->state))
		return -EBUSY;

	netif_carrier_off(netdev);

	/* allocate transmit descriptors */
	err = ixgbe_setup_all_tx_resources(adapter);
	if (err)
		goto err_setup_tx;

	/* allocate receive descriptors */
	err = ixgbe_setup_all_rx_resources(adapter);
	if (err)
		goto err_setup_rx;

	ixgbe_configure(adapter);

	err = ixgbe_request_irq(adapter);
	if (err)
		goto err_req_irq;

	/* Notify the stack of the actual queue counts. */
	err = netif_set_real_num_tx_queues(netdev,
					   adapter->num_rx_pools > 1 ? 1 :
					   adapter->num_tx_queues);
	if (err)
		goto err_set_queues;

	err = netif_set_real_num_rx_queues(netdev,
					   adapter->num_rx_pools > 1 ? 1 :
					   adapter->num_rx_queues);
	if (err)
		goto err_set_queues;

#ifdef HAVE_PTP_1588_CLOCK
	ixgbe_ptp_init(adapter);
#endif /* HAVE_PTP_1588_CLOCK*/

	ixgbe_up_complete(adapter);

#if defined(HAVE_UDP_ENC_RX_OFFLOAD) || defined(HAVE_VXLAN_RX_OFFLOAD)
	ixgbe_clear_udp_tunnel_port(adapter, IXGBE_VXLANCTRL_ALL_UDPPORT_MASK);
#endif
#ifdef HAVE_UDP_ENC_RX_OFFLOAD
	udp_tunnel_get_rx_info(netdev);
#elif defined(HAVE_VXLAN_RX_OFFLOAD)
	vxlan_get_rx_port(netdev);
#endif /* HAVE_UDP_ENC_RX_OFFLOAD */
	return IXGBE_SUCCESS;

err_set_queues:
	ixgbe_free_irq(adapter);
err_req_irq:
	ixgbe_free_all_rx_resources(adapter);
	if (!adapter->wol)
		ixgbe_set_phy_power(&adapter->hw, false);
err_setup_rx:
	ixgbe_free_all_tx_resources(adapter);
err_setup_tx:
	ixgbe_reset(adapter);

	return err;
}

/**
 * ixgbe_close_suspend - actions necessary to both suspend and close flows
 * @adapter: the private adapter struct
 *
 * This function should contain the necessary work common to both suspending
 * and closing of the device.
 */
static void ixgbe_close_suspend(struct ixgbe_adapter *adapter)
{
#ifdef HAVE_PTP_1588_CLOCK
	ixgbe_ptp_suspend(adapter);
#endif

	if (adapter->hw.phy.ops.enter_lplu) {
		adapter->hw.phy.reset_disable = true;
		ixgbe_down(adapter);
		ixgbe_enter_lplu(&adapter->hw);
		adapter->hw.phy.reset_disable = false;
	} else {
		ixgbe_down(adapter);
	}
	ixgbe_free_irq(adapter);

	ixgbe_free_all_rx_resources(adapter);
	ixgbe_free_all_tx_resources(adapter);
}

/**
 * ixgbe_close - Disables a network interface
 * @netdev: network interface device structure
 *
 * Returns 0, this is not allowed to fail
 *
 * The close entry point is called when an interface is de-activated
 * by the OS.  The hardware is still under the drivers control, but
 * needs to be disabled.  A global MAC reset is issued to stop the
 * hardware, and all transmit and receive resources are freed.
 **/
int ixgbe_close(struct net_device *netdev)
{
	struct ixgbe_adapter *adapter = netdev_priv(netdev);

#ifdef HAVE_PTP_1588_CLOCK
	ixgbe_ptp_stop(adapter);
#endif

	if (netif_device_present(netdev))
		ixgbe_close_suspend(adapter);

	ixgbe_fdir_filter_exit(adapter);

	ixgbe_release_hw_control(adapter);

	return 0;
}

#ifdef CONFIG_PM
#ifndef USE_LEGACY_PM_SUPPORT
static int ixgbe_resume(struct device *dev)
#else
static int ixgbe_resume(struct pci_dev *pdev)
#endif /* USE_LEGACY_PM_SUPPORT */
{
	struct ixgbe_adapter *adapter;
	struct net_device *netdev;
	u32 err;
#ifndef USE_LEGACY_PM_SUPPORT
	struct pci_dev *pdev = to_pci_dev(dev);
#endif

	adapter = pci_get_drvdata(pdev);
	netdev = adapter->netdev;
	adapter->hw.hw_addr = adapter->io_addr;
	pci_set_power_state(pdev, PCI_D0);
	pci_restore_state(pdev);
	/*
	 * pci_restore_state clears dev->state_saved so call
	 * pci_save_state to restore it.
	 */
	pci_save_state(pdev);

	err = pci_enable_device_mem(pdev);
	if (err) {
		e_dev_err("Cannot enable PCI device from suspend\n");
		return err;
	}
	smp_mb__before_atomic();
	clear_bit(__IXGBE_DISABLED, &adapter->state);
	pci_set_master(pdev);

	pci_wake_from_d3(pdev, false);

	ixgbe_reset(adapter);

	IXGBE_WRITE_REG(&adapter->hw, IXGBE_WUS, ~0);

	rtnl_lock();

	err = ixgbe_init_interrupt_scheme(adapter);
	if (!err && netif_running(netdev))
		err = ixgbe_open(netdev);


	if (!err)
		netif_device_attach(netdev);

	rtnl_unlock();

	return err;
}

#ifndef USE_LEGACY_PM_SUPPORT
/**
 * ixgbe_freeze - quiesce the device (no IRQ's or DMA)
 * @dev: The port's netdev
 */
static int ixgbe_freeze(struct device *dev)
{
	struct ixgbe_adapter *adapter = pci_get_drvdata(to_pci_dev(dev));
	struct net_device *netdev = adapter->netdev;
	bool lplu_enabled = !!adapter->hw.phy.ops.enter_lplu;

	rtnl_lock();
	netif_device_detach(netdev);

	if (netif_running(netdev)) {
		if (lplu_enabled) {
			adapter->hw.phy.reset_disable = true;
			ixgbe_down(adapter);
			adapter->hw.phy.reset_disable = false;
		} else {
			ixgbe_down(adapter);
		}
		ixgbe_free_irq(adapter);
	}

	ixgbe_reset_interrupt_capability(adapter);
	rtnl_unlock();

	return 0;
}

/**
 * ixgbe_thaw - un-quiesce the device
 * @dev: The port's netdev
 */
static int ixgbe_thaw(struct device *dev)
{
	struct ixgbe_adapter *adapter = pci_get_drvdata(to_pci_dev(dev));
	struct net_device *netdev = adapter->netdev;
	bool lplu_enabled = !!adapter->hw.phy.ops.enter_lplu;

	ixgbe_set_interrupt_capability(adapter);

	if (netif_running(netdev)) {
		u32 err = ixgbe_request_irq(adapter);
		if (err)
			return err;

		if (lplu_enabled) {
			adapter->hw.phy.reset_disable = true;
			ixgbe_up(adapter);
			adapter->hw.phy.reset_disable = false;
		} else {
			ixgbe_up(adapter);
		}
	}

	netif_device_attach(netdev);

	return 0;
}
#endif /* USE_LEGACY_PM_SUPPORT */
#endif /* CONFIG_PM */

/*
 * __ixgbe_shutdown is not used when power management
 * is disabled on older kernels (<2.6.12). causes a compile
 * warning/error, because it is defined and not used.
 */
#if defined(CONFIG_PM) || !defined(USE_REBOOT_NOTIFIER)
static int __ixgbe_shutdown(struct pci_dev *pdev, bool *enable_wake)
{
	struct ixgbe_adapter *adapter = pci_get_drvdata(pdev);
	struct net_device *netdev = adapter->netdev;
	struct ixgbe_hw *hw = &adapter->hw;
	u32 ctrl;
	u32 wufc = adapter->wol;
#ifdef CONFIG_PM
	int retval = 0;
#endif

	rtnl_lock();
	netif_device_detach(netdev);

	if (netif_running(netdev))
		ixgbe_close_suspend(adapter);

	ixgbe_clear_interrupt_scheme(adapter);
	rtnl_unlock();

#ifdef CONFIG_PM
	retval = pci_save_state(pdev);
	if (retval)
		return retval;

#endif

	/* this won't stop link of managebility or WoL is enabled */
	if (hw->mac.type == ixgbe_mac_82599EB)
		ixgbe_stop_mac_link_on_d3_82599(hw);

	if (wufc) {
		u32 fctrl;

		ixgbe_set_rx_mode(netdev);

		/* enable the optics for 82599 SFP+ fiber as we can WoL */
		if (hw->mac.ops.enable_tx_laser)
			hw->mac.ops.enable_tx_laser(hw);

		/* enable the reception of multicast packets */
		fctrl = IXGBE_READ_REG(hw, IXGBE_FCTRL);
		fctrl |= IXGBE_FCTRL_MPE;
		IXGBE_WRITE_REG(hw, IXGBE_FCTRL, fctrl);

		ctrl = IXGBE_READ_REG(hw, IXGBE_CTRL);
		ctrl |= IXGBE_CTRL_GIO_DIS;
		IXGBE_WRITE_REG(hw, IXGBE_CTRL, ctrl);

		IXGBE_WRITE_REG(hw, IXGBE_WUFC, wufc);
	} else {
		IXGBE_WRITE_REG(hw, IXGBE_WUC, 0);
		IXGBE_WRITE_REG(hw, IXGBE_WUFC, 0);
	}

	switch (hw->mac.type) {
	case ixgbe_mac_82598EB:
		pci_wake_from_d3(pdev, false);
		break;
	case ixgbe_mac_82599EB:
	case ixgbe_mac_X540:
	case ixgbe_mac_X550:
	case ixgbe_mac_X550EM_x:
	case ixgbe_mac_X550EM_a:
		pci_wake_from_d3(pdev, !!wufc);
		break;
	default:
		break;
	}

	*enable_wake = !!wufc;
	if (!*enable_wake)
		ixgbe_set_phy_power(hw, false);

	ixgbe_release_hw_control(adapter);

	if (!test_and_set_bit(__IXGBE_DISABLED, &adapter->state))
		pci_disable_device(pdev);

	return 0;
}
#endif /* defined(CONFIG_PM) || !defined(USE_REBOOT_NOTIFIER) */

#ifdef CONFIG_PM
#ifndef USE_LEGACY_PM_SUPPORT
static int ixgbe_suspend(struct device *dev)
#else
static int ixgbe_suspend(struct pci_dev *pdev,
			 pm_message_t __always_unused state)
#endif /* USE_LEGACY_PM_SUPPORT */
{
	int retval;
	bool wake;
#ifndef USE_LEGACY_PM_SUPPORT
	struct pci_dev *pdev = to_pci_dev(dev);
#endif

	retval = __ixgbe_shutdown(pdev, &wake);
	if (retval)
		return retval;

	if (wake) {
		pci_prepare_to_sleep(pdev);
	} else {
		pci_wake_from_d3(pdev, false);
		pci_set_power_state(pdev, PCI_D3hot);
	}

	return 0;
}
#endif /* CONFIG_PM */

#ifndef USE_REBOOT_NOTIFIER
static void ixgbe_shutdown(struct pci_dev *pdev)
{
	bool wake;

	__ixgbe_shutdown(pdev, &wake);

	if (system_state == SYSTEM_POWER_OFF) {
		pci_wake_from_d3(pdev, wake);
		pci_set_power_state(pdev, PCI_D3hot);
	}
}

#endif
#ifdef HAVE_NDO_GET_STATS64
static void ixgbe_get_ring_stats64(struct rtnl_link_stats64 *stats,
				   struct ixgbe_ring *ring)
{
	u64 bytes, packets;
	unsigned int start;

	if (ring) {
		do {
			start = u64_stats_fetch_begin_irq(&ring->syncp);
			packets = ring->stats.packets;
			bytes   = ring->stats.bytes;
		} while (u64_stats_fetch_retry_irq(&ring->syncp, start));
		stats->tx_packets += packets;
		stats->tx_bytes   += bytes;
	}
}

/**
 * ixgbe_get_stats64 - Get System Network Statistics
 * @netdev: network interface device structure
 * @stats: storage space for 64bit statistics
 *
 * Returns 64bit statistics, for use in the ndo_get_stats64 callback. This
 * function replaces ixgbe_get_stats for kernels which support it.
 */
#ifdef HAVE_VOID_NDO_GET_STATS64
static void ixgbe_get_stats64(struct net_device *netdev,
			      struct rtnl_link_stats64 *stats)
#else
static struct rtnl_link_stats64 *
ixgbe_get_stats64(struct net_device *netdev, struct rtnl_link_stats64 *stats)
#endif
{
	struct ixgbe_adapter *adapter = netdev_priv(netdev);
	int i;

	rcu_read_lock();
	for (i = 0; i < adapter->num_rx_queues; i++) {
		struct ixgbe_ring *ring = READ_ONCE(adapter->rx_ring[i]);
		u64 bytes, packets;
		unsigned int start;

		if (ring) {
			do {
				start = u64_stats_fetch_begin_irq(&ring->syncp);
				packets = ring->stats.packets;
				bytes   = ring->stats.bytes;
			} while (u64_stats_fetch_retry_irq(&ring->syncp, start));
			stats->rx_packets += packets;
			stats->rx_bytes   += bytes;
		}
	}

	for (i = 0; i < adapter->num_tx_queues; i++) {
		struct ixgbe_ring *ring = READ_ONCE(adapter->tx_ring[i]);

		ixgbe_get_ring_stats64(stats, ring);
	}

	for (i = 0; i < adapter->num_xdp_queues; i++) {
		struct ixgbe_ring *ring = READ_ONCE(adapter->xdp_ring[i]);

		ixgbe_get_ring_stats64(stats, ring);
	}
	rcu_read_unlock();

	/* following stats updated by ixgbe_watchdog_task() */
	stats->multicast	= netdev->stats.multicast;
	stats->rx_errors	= netdev->stats.rx_errors;
	stats->rx_length_errors	= netdev->stats.rx_length_errors;
	stats->rx_crc_errors	= netdev->stats.rx_crc_errors;
	stats->rx_missed_errors	= netdev->stats.rx_missed_errors;
#ifndef HAVE_VOID_NDO_GET_STATS64

	return stats;
#endif
}
#else
/**
 * ixgbe_get_stats - Get System Network Statistics
 * @netdev: network interface device structure
 *
 * Returns the address of the device statistics structure.
 * The statistics are actually updated from the timer callback.
 **/
static struct net_device_stats *ixgbe_get_stats(struct net_device *netdev)
{
	struct ixgbe_adapter *adapter = netdev_priv(netdev);

	/* update the stats data */
	ixgbe_update_stats(adapter);

#ifdef HAVE_NETDEV_STATS_IN_NETDEV
	/* only return the current stats */
	return &netdev->stats;
#else
	/* only return the current stats */
	return &adapter->net_stats;
#endif /* HAVE_NETDEV_STATS_IN_NETDEV */
}
#endif
/**
 * ixgbe_update_stats - Update the board statistics counters.
 * @adapter: board private structure
 **/
void ixgbe_update_stats(struct ixgbe_adapter *adapter)
{
#ifdef HAVE_NETDEV_STATS_IN_NETDEV
	struct net_device_stats *net_stats = &adapter->netdev->stats;
#else
	struct net_device_stats *net_stats = &adapter->net_stats;
#endif /* HAVE_NETDEV_STATS_IN_NETDEV */
	struct ixgbe_hw *hw = &adapter->hw;
	struct ixgbe_hw_stats *hwstats = &adapter->stats;
	u64 total_mpc = 0;
	u32 i, missed_rx = 0, mpc, bprc, lxon, lxoff, xon_off_tot;
	u64 non_eop_descs = 0, restart_queue = 0, tx_busy = 0;
	u64 alloc_rx_page_failed = 0, alloc_rx_buff_failed = 0;
	u64 alloc_rx_page = 0;
	u64 bytes = 0, packets = 0, hw_csum_rx_error = 0;

	if (test_bit(__IXGBE_DOWN, &adapter->state) ||
	    test_bit(__IXGBE_RESETTING, &adapter->state))
		return;

	if (adapter->flags2 & IXGBE_FLAG2_RSC_ENABLED) {
		u64 rsc_count = 0;
		u64 rsc_flush = 0;
		for (i = 0; i < adapter->num_rx_queues; i++) {
			rsc_count += adapter->rx_ring[i]->rx_stats.rsc_count;
			rsc_flush += adapter->rx_ring[i]->rx_stats.rsc_flush;
		}
		adapter->rsc_total_count = rsc_count;
		adapter->rsc_total_flush = rsc_flush;
	}

	for (i = 0; i < adapter->num_rx_queues; i++) {
		struct ixgbe_ring *rx_ring = adapter->rx_ring[i];
		non_eop_descs += rx_ring->rx_stats.non_eop_descs;
		alloc_rx_page += rx_ring->rx_stats.alloc_rx_page;
		alloc_rx_page_failed += rx_ring->rx_stats.alloc_rx_page_failed;
		alloc_rx_buff_failed += rx_ring->rx_stats.alloc_rx_buff_failed;
		hw_csum_rx_error += rx_ring->rx_stats.csum_err;
		bytes += rx_ring->stats.bytes;
		packets += rx_ring->stats.packets;

	}
	adapter->non_eop_descs = non_eop_descs;
	adapter->alloc_rx_page = alloc_rx_page;
	adapter->alloc_rx_page_failed = alloc_rx_page_failed;
	adapter->alloc_rx_buff_failed = alloc_rx_buff_failed;
	adapter->hw_csum_rx_error = hw_csum_rx_error;
	net_stats->rx_bytes = bytes;
	net_stats->rx_packets = packets;

	bytes = 0;
	packets = 0;
	/* gather some stats to the adapter struct that are per queue */
	for (i = 0; i < adapter->num_tx_queues; i++) {
		struct ixgbe_ring *tx_ring = adapter->tx_ring[i];
		restart_queue += tx_ring->tx_stats.restart_queue;
		tx_busy += tx_ring->tx_stats.tx_busy;
		bytes += tx_ring->stats.bytes;
		packets += tx_ring->stats.packets;
	}
	for (i = 0; i < adapter->num_xdp_queues; i++) {
		struct ixgbe_ring *xdp_ring = adapter->xdp_ring[i];

		restart_queue += xdp_ring->tx_stats.restart_queue;
		tx_busy += xdp_ring->tx_stats.tx_busy;
		bytes += xdp_ring->stats.bytes;
		packets += xdp_ring->stats.packets;
	}
	adapter->restart_queue = restart_queue;
	adapter->tx_busy = tx_busy;
	net_stats->tx_bytes = bytes;
	net_stats->tx_packets = packets;

	hwstats->crcerrs += IXGBE_READ_REG(hw, IXGBE_CRCERRS);

	/* 8 register reads */
	for (i = 0; i < 8; i++) {
		/* for packet buffers not used, the register should read 0 */
		mpc = IXGBE_READ_REG(hw, IXGBE_MPC(i));
		missed_rx += mpc;
		hwstats->mpc[i] += mpc;
		total_mpc += hwstats->mpc[i];
		hwstats->pxontxc[i] += IXGBE_READ_REG(hw, IXGBE_PXONTXC(i));
		hwstats->pxofftxc[i] += IXGBE_READ_REG(hw, IXGBE_PXOFFTXC(i));
		switch (hw->mac.type) {
		case ixgbe_mac_82598EB:
			hwstats->rnbc[i] += IXGBE_READ_REG(hw, IXGBE_RNBC(i));
			hwstats->qbtc[i] += IXGBE_READ_REG(hw, IXGBE_QBTC(i));
			hwstats->qbrc[i] += IXGBE_READ_REG(hw, IXGBE_QBRC(i));
			hwstats->pxonrxc[i] +=
				IXGBE_READ_REG(hw, IXGBE_PXONRXC(i));
			break;
		case ixgbe_mac_82599EB:
		case ixgbe_mac_X540:
		case ixgbe_mac_X550:
		case ixgbe_mac_X550EM_x:
		case ixgbe_mac_X550EM_a:
			hwstats->pxonrxc[i] +=
				IXGBE_READ_REG(hw, IXGBE_PXONRXCNT(i));
			break;
		default:
			break;
		}
	}

	/*16 register reads */
	for (i = 0; i < 16; i++) {
		hwstats->qptc[i] += IXGBE_READ_REG(hw, IXGBE_QPTC(i));
		hwstats->qprc[i] += IXGBE_READ_REG(hw, IXGBE_QPRC(i));
		if ((hw->mac.type == ixgbe_mac_82599EB) ||
		    (hw->mac.type == ixgbe_mac_X550) ||
		    (hw->mac.type == ixgbe_mac_X550EM_x) ||
		    (hw->mac.type == ixgbe_mac_X550EM_a) ||
		    (hw->mac.type == ixgbe_mac_X540)) {
			hwstats->qbtc[i] += IXGBE_READ_REG(hw, IXGBE_QBTC_L(i));
			IXGBE_READ_REG(hw, IXGBE_QBTC_H(i)); /* to clear */
			hwstats->qbrc[i] += IXGBE_READ_REG(hw, IXGBE_QBRC_L(i));
			IXGBE_READ_REG(hw, IXGBE_QBRC_H(i)); /* to clear */
		}
	}

	hwstats->gprc += IXGBE_READ_REG(hw, IXGBE_GPRC);
	/* work around hardware counting issue */
	hwstats->gprc -= missed_rx;

	ixgbe_update_xoff_received(adapter);

	/* 82598 hardware only has a 32 bit counter in the high register */
	switch (hw->mac.type) {
	case ixgbe_mac_82598EB:
		hwstats->lxonrxc += IXGBE_READ_REG(hw, IXGBE_LXONRXC);
		hwstats->gorc += IXGBE_READ_REG(hw, IXGBE_GORCH);
		hwstats->gotc += IXGBE_READ_REG(hw, IXGBE_GOTCH);
		hwstats->tor += IXGBE_READ_REG(hw, IXGBE_TORH);
		break;
	case ixgbe_mac_X540:
	case ixgbe_mac_X550:
	case ixgbe_mac_X550EM_x:
	case ixgbe_mac_X550EM_a:
		/* OS2BMC stats are X540 only*/
		hwstats->o2bgptc += IXGBE_READ_REG(hw, IXGBE_O2BGPTC);
		hwstats->o2bspc += IXGBE_READ_REG(hw, IXGBE_O2BSPC);
		hwstats->b2ospc += IXGBE_READ_REG(hw, IXGBE_B2OSPC);
		hwstats->b2ogprc += IXGBE_READ_REG(hw, IXGBE_B2OGPRC);
		/* fall through */
	case ixgbe_mac_82599EB:
		for (i = 0; i < 16; i++)
			adapter->hw_rx_no_dma_resources +=
					     IXGBE_READ_REG(hw, IXGBE_QPRDC(i));
		hwstats->gorc += IXGBE_READ_REG(hw, IXGBE_GORCL);
		IXGBE_READ_REG(hw, IXGBE_GORCH); /* to clear */
		hwstats->gotc += IXGBE_READ_REG(hw, IXGBE_GOTCL);
		IXGBE_READ_REG(hw, IXGBE_GOTCH); /* to clear */
		hwstats->tor += IXGBE_READ_REG(hw, IXGBE_TORL);
		IXGBE_READ_REG(hw, IXGBE_TORH); /* to clear */
		hwstats->lxonrxc += IXGBE_READ_REG(hw, IXGBE_LXONRXCNT);
#ifdef HAVE_TX_MQ
		hwstats->fdirmatch += IXGBE_READ_REG(hw, IXGBE_FDIRMATCH);
		hwstats->fdirmiss += IXGBE_READ_REG(hw, IXGBE_FDIRMISS);
#endif /* HAVE_TX_MQ */
#if IS_ENABLED(CONFIG_FCOE)
		hwstats->fccrc += IXGBE_READ_REG(hw, IXGBE_FCCRC);
		hwstats->fclast += IXGBE_READ_REG(hw, IXGBE_FCLAST);
		hwstats->fcoerpdc += IXGBE_READ_REG(hw, IXGBE_FCOERPDC);
		hwstats->fcoeprc += IXGBE_READ_REG(hw, IXGBE_FCOEPRC);
		hwstats->fcoeptc += IXGBE_READ_REG(hw, IXGBE_FCOEPTC);
		hwstats->fcoedwrc += IXGBE_READ_REG(hw, IXGBE_FCOEDWRC);
		hwstats->fcoedwtc += IXGBE_READ_REG(hw, IXGBE_FCOEDWTC);
		/* Add up per cpu counters for total ddp alloc fail */
		if (adapter->fcoe.ddp_pool) {
			struct ixgbe_fcoe *fcoe = &adapter->fcoe;
			struct ixgbe_fcoe_ddp_pool *ddp_pool;
			unsigned int cpu;
			u64 noddp = 0, noddp_ext_buff = 0;
			for_each_possible_cpu(cpu) {
				ddp_pool = per_cpu_ptr(fcoe->ddp_pool, cpu);
				noddp += ddp_pool->noddp;
				noddp_ext_buff += ddp_pool->noddp_ext_buff;
			}
			hwstats->fcoe_noddp = noddp;
			hwstats->fcoe_noddp_ext_buff = noddp_ext_buff;
		}

#endif /* CONFIG_FCOE */
		break;
	default:
		break;
	}
	bprc = IXGBE_READ_REG(hw, IXGBE_BPRC);
	hwstats->bprc += bprc;
	hwstats->mprc += IXGBE_READ_REG(hw, IXGBE_MPRC);
	if (hw->mac.type == ixgbe_mac_82598EB)
		hwstats->mprc -= bprc;
	hwstats->roc += IXGBE_READ_REG(hw, IXGBE_ROC);
	hwstats->prc64 += IXGBE_READ_REG(hw, IXGBE_PRC64);
	hwstats->prc127 += IXGBE_READ_REG(hw, IXGBE_PRC127);
	hwstats->prc255 += IXGBE_READ_REG(hw, IXGBE_PRC255);
	hwstats->prc511 += IXGBE_READ_REG(hw, IXGBE_PRC511);
	hwstats->prc1023 += IXGBE_READ_REG(hw, IXGBE_PRC1023);
	hwstats->prc1522 += IXGBE_READ_REG(hw, IXGBE_PRC1522);
	hwstats->rlec += IXGBE_READ_REG(hw, IXGBE_RLEC);
	lxon = IXGBE_READ_REG(hw, IXGBE_LXONTXC);
	hwstats->lxontxc += lxon;
	lxoff = IXGBE_READ_REG(hw, IXGBE_LXOFFTXC);
	hwstats->lxofftxc += lxoff;
	hwstats->gptc += IXGBE_READ_REG(hw, IXGBE_GPTC);
	hwstats->mptc += IXGBE_READ_REG(hw, IXGBE_MPTC);
	/*
	 * 82598 errata - tx of flow control packets is included in tx counters
	 */
	xon_off_tot = lxon + lxoff;
	hwstats->gptc -= xon_off_tot;
	hwstats->mptc -= xon_off_tot;
	hwstats->gotc -= (xon_off_tot * (ETH_ZLEN + ETH_FCS_LEN));
	hwstats->ruc += IXGBE_READ_REG(hw, IXGBE_RUC);
	hwstats->rfc += IXGBE_READ_REG(hw, IXGBE_RFC);
	hwstats->rjc += IXGBE_READ_REG(hw, IXGBE_RJC);
	hwstats->tpr += IXGBE_READ_REG(hw, IXGBE_TPR);
	hwstats->ptc64 += IXGBE_READ_REG(hw, IXGBE_PTC64);
	hwstats->ptc64 -= xon_off_tot;
	hwstats->ptc127 += IXGBE_READ_REG(hw, IXGBE_PTC127);
	hwstats->ptc255 += IXGBE_READ_REG(hw, IXGBE_PTC255);
	hwstats->ptc511 += IXGBE_READ_REG(hw, IXGBE_PTC511);
	hwstats->ptc1023 += IXGBE_READ_REG(hw, IXGBE_PTC1023);
	hwstats->ptc1522 += IXGBE_READ_REG(hw, IXGBE_PTC1522);
	hwstats->bptc += IXGBE_READ_REG(hw, IXGBE_BPTC);
	/* Fill out the OS statistics structure */
	net_stats->multicast = hwstats->mprc;

	/* Rx Errors */
	net_stats->rx_errors = hwstats->crcerrs +
				       hwstats->rlec;
	net_stats->rx_dropped = 0;
	net_stats->rx_length_errors = hwstats->rlec;
	net_stats->rx_crc_errors = hwstats->crcerrs;
	net_stats->rx_missed_errors = total_mpc;

	/*
	 * VF Stats Collection - skip while resetting because these
	 * are not clear on read and otherwise you'll sometimes get
	 * crazy values.
	 */
	if (!test_bit(__IXGBE_RESETTING, &adapter->state)) {
		for (i = 0; i < adapter->num_vfs; i++) {
			UPDATE_VF_COUNTER_32bit(IXGBE_PVFGPRC(i),	      \
					adapter->vfinfo[i].last_vfstats.gprc, \
					adapter->vfinfo[i].vfstats.gprc);
			UPDATE_VF_COUNTER_32bit(IXGBE_PVFGPTC(i),	      \
					adapter->vfinfo[i].last_vfstats.gptc, \
					adapter->vfinfo[i].vfstats.gptc);
			UPDATE_VF_COUNTER_36bit(IXGBE_PVFGORC_LSB(i),	      \
					IXGBE_PVFGORC_MSB(i),		      \
					adapter->vfinfo[i].last_vfstats.gorc, \
					adapter->vfinfo[i].vfstats.gorc);
			UPDATE_VF_COUNTER_36bit(IXGBE_PVFGOTC_LSB(i),	      \
					IXGBE_PVFGOTC_MSB(i),		      \
					adapter->vfinfo[i].last_vfstats.gotc, \
					adapter->vfinfo[i].vfstats.gotc);
			UPDATE_VF_COUNTER_32bit(IXGBE_PVFMPRC(i),	      \
					adapter->vfinfo[i].last_vfstats.mprc, \
					adapter->vfinfo[i].vfstats.mprc);
		}
	}
}

#ifdef HAVE_TX_MQ
/**
 * ixgbe_fdir_reinit_subtask - worker thread to reinit FDIR filter table
 * @adapter: pointer to the device adapter structure
 **/
static void ixgbe_fdir_reinit_subtask(struct ixgbe_adapter *adapter)
{
	struct ixgbe_hw *hw = &adapter->hw;
	int i;

	if (!(adapter->flags2 & IXGBE_FLAG2_FDIR_REQUIRES_REINIT))
		return;

	adapter->flags2 &= ~IXGBE_FLAG2_FDIR_REQUIRES_REINIT;

	/* if interface is down do nothing */
	if (test_bit(__IXGBE_DOWN, &adapter->state))
		return;

	/* do nothing if we are not using signature filters */
	if (!(adapter->flags & IXGBE_FLAG_FDIR_HASH_CAPABLE))
		return;

	adapter->fdir_overflow++;

	if (ixgbe_reinit_fdir_tables_82599(hw) == IXGBE_SUCCESS) {
		for (i = 0; i < adapter->num_tx_queues; i++)
			set_bit(__IXGBE_TX_FDIR_INIT_DONE,
				&(adapter->tx_ring[i]->state));
		for (i = 0; i < adapter->num_xdp_queues; i++)
			set_bit(__IXGBE_TX_FDIR_INIT_DONE,
				&adapter->xdp_ring[i]->state);
		/* re-enable flow director interrupts */
		IXGBE_WRITE_REG(hw, IXGBE_EIMS, IXGBE_EIMS_FLOW_DIR);
	} else {
		e_err(probe, "failed to finish FDIR re-initialization, "
		      "ignored adding FDIR ATR filters\n");
	}
}

#endif /* HAVE_TX_MQ */
/**
 * ixgbe_check_hang_subtask - check for hung queues and dropped interrupts
 * @adapter: pointer to the device adapter structure
 *
 * This function serves two purposes.  First it strobes the interrupt lines
 * in order to make certain interrupts are occurring.  Secondly it sets the
 * bits needed to check for TX hangs.  As a result we should immediately
 * determine if a hang has occurred.
 */
static void ixgbe_check_hang_subtask(struct ixgbe_adapter *adapter)
{
	struct ixgbe_hw *hw = &adapter->hw;
	u64 eics = 0;
	int i;

	/* If we're down, removing or resetting, just bail */
	if (test_bit(__IXGBE_DOWN, &adapter->state) ||
	    test_bit(__IXGBE_REMOVING, &adapter->state) ||
	    test_bit(__IXGBE_RESETTING, &adapter->state))
		return;

	/* Force detection of hung controller */
	if (netif_carrier_ok(adapter->netdev)) {
		for (i = 0; i < adapter->num_tx_queues; i++)
			set_check_for_tx_hang(adapter->tx_ring[i]);
		for (i = 0; i < adapter->num_xdp_queues; i++)
			set_check_for_tx_hang(adapter->xdp_ring[i]);
	}

	if (!(adapter->flags & IXGBE_FLAG_MSIX_ENABLED)) {
		/*
		 * for legacy and MSI interrupts don't set any bits
		 * that are enabled for EIAM, because this operation
		 * would set *both* EIMS and EICS for any bit in EIAM
		 */
		IXGBE_WRITE_REG(hw, IXGBE_EICS,
			(IXGBE_EICS_TCP_TIMER | IXGBE_EICS_OTHER));
	} else {
		/* get one bit for every active tx/rx interrupt vector */
		for (i = 0; i < adapter->num_q_vectors; i++) {
			struct ixgbe_q_vector *qv = adapter->q_vector[i];
			if (qv->rx.ring || qv->tx.ring)
				eics |= ((u64)1 << i);
		}
	}

	/* Cause software interrupt to ensure rings are cleaned */
	ixgbe_irq_rearm_queues(adapter, eics);
}

/**
 * ixgbe_watchdog_update_link - update the link status
 * @adapter: pointer to the device adapter structure
 **/
static void ixgbe_watchdog_update_link(struct ixgbe_adapter *adapter)
{
	struct ixgbe_hw *hw = &adapter->hw;
	u32 link_speed = adapter->link_speed;
	bool link_up = adapter->link_up;
	bool pfc_en = adapter->dcb_cfg.pfc_mode_enable;

	if (!(adapter->flags & IXGBE_FLAG_NEED_LINK_UPDATE))
		return;

	if (hw->mac.ops.check_link) {
		hw->mac.ops.check_link(hw, &link_speed, &link_up, false);
	} else {
		/* always assume link is up, if no check link function */
		link_speed = IXGBE_LINK_SPEED_10GB_FULL;
		link_up = true;
	}

#ifdef HAVE_DCBNL_IEEE
	if (adapter->ixgbe_ieee_pfc)
		pfc_en |= !!(adapter->ixgbe_ieee_pfc->pfc_en);

#endif
	if (link_up && !((adapter->flags & IXGBE_FLAG_DCB_ENABLED) && pfc_en)) {
		hw->mac.ops.fc_enable(hw);
		ixgbe_set_rx_drop_en(adapter);
	}

	if (link_up ||
	    time_after(jiffies, (adapter->link_check_timeout +
				 IXGBE_TRY_LINK_TIMEOUT))) {
		adapter->flags &= ~IXGBE_FLAG_NEED_LINK_UPDATE;
		IXGBE_WRITE_REG(hw, IXGBE_EIMS, IXGBE_EIMC_LSC);
		IXGBE_WRITE_FLUSH(hw);
	}

	adapter->link_up = link_up;
	adapter->link_speed = link_speed;

	if (hw->mac.ops.dmac_config && hw->mac.dmac_config.watchdog_timer) {
		u8 num_tcs = netdev_get_num_tc(adapter->netdev);
#if IS_ENABLED(CONFIG_FCOE)
		u8 fcoe_tc = ixgbe_fcoe_get_tc(adapter);
		bool fcoe_en = !!(adapter->flags & IXGBE_FLAG_FCOE_ENABLED);
#endif /* CONFIG_FCOE */

		if (hw->mac.dmac_config.link_speed != link_speed ||
#if IS_ENABLED(CONFIG_FCOE)
		    hw->mac.dmac_config.fcoe_tc != fcoe_tc ||
		    hw->mac.dmac_config.fcoe_en != fcoe_en ||
#endif /* CONFIG_FCOE */
		    hw->mac.dmac_config.num_tcs != num_tcs) {
			hw->mac.dmac_config.link_speed = link_speed;
			hw->mac.dmac_config.num_tcs = num_tcs;
#if IS_ENABLED(CONFIG_FCOE)
			hw->mac.dmac_config.fcoe_en = fcoe_en;
			hw->mac.dmac_config.fcoe_tc = fcoe_tc;
#endif /* CONFIG_FCOE */
			hw->mac.ops.dmac_config(hw);
		}
	}
}

static void ixgbe_update_default_up(struct ixgbe_adapter *adapter)
{
	u8 up = 0;
#ifdef HAVE_DCBNL_IEEE
	struct net_device *netdev = adapter->netdev;
	struct dcb_app app = {
			      .selector = DCB_APP_IDTYPE_ETHTYPE,
			      .protocol = 0,
			     };
	up = dcb_getapp(netdev, &app);
#endif

#if IS_ENABLED(CONFIG_FCOE)
	adapter->default_up = (up > 1) ? (ffs(up) - 1) : 0;
#else
	adapter->default_up = up;
#endif
}

/**
 * ixgbe_watchdog_link_is_up - update netif_carrier status and
 *                             print link up message
 * @adapter: pointer to the device adapter structure
 **/
static void ixgbe_watchdog_link_is_up(struct ixgbe_adapter *adapter)
{
	struct net_device *netdev = adapter->netdev;
	struct ixgbe_hw *hw = &adapter->hw;
	u32 link_speed = adapter->link_speed;
	const char *speed_str;
	bool flow_rx, flow_tx;

	/* only continue if link was previously down */
	if (netif_carrier_ok(netdev))
		return;

	adapter->flags2 &= ~IXGBE_FLAG2_SEARCH_FOR_SFP;

	switch (hw->mac.type) {
	case ixgbe_mac_82598EB: {
		u32 frctl = IXGBE_READ_REG(hw, IXGBE_FCTRL);
		u32 rmcs = IXGBE_READ_REG(hw, IXGBE_RMCS);
		flow_rx = !!(frctl & IXGBE_FCTRL_RFCE);
		flow_tx = !!(rmcs & IXGBE_RMCS_TFCE_802_3X);
	}
		break;
	case ixgbe_mac_X550:
	case ixgbe_mac_X550EM_x:
	case ixgbe_mac_X550EM_a:
	case ixgbe_mac_82599EB:
	case ixgbe_mac_X540: {
		u32 mflcn = IXGBE_READ_REG(hw, IXGBE_MFLCN);
		u32 fccfg = IXGBE_READ_REG(hw, IXGBE_FCCFG);
		flow_rx = !!(mflcn & IXGBE_MFLCN_RFCE);
		flow_tx = !!(fccfg & IXGBE_FCCFG_TFCE_802_3X);
	}
		break;
	default:
		flow_tx = false;
		flow_rx = false;
		break;
	}

#ifdef HAVE_PTP_1588_CLOCK
	adapter->last_rx_ptp_check = jiffies;

	if (test_bit(__IXGBE_PTP_RUNNING, &adapter->state))
		ixgbe_ptp_start_cyclecounter(adapter);

#endif
	switch (link_speed) {
	case IXGBE_LINK_SPEED_10GB_FULL:
		speed_str = "10 Gbps";
		break;
	case IXGBE_LINK_SPEED_5GB_FULL:
		speed_str = "5 Gbps";
		break;
	case IXGBE_LINK_SPEED_2_5GB_FULL:
		speed_str = "2.5 Gbps";
		break;
	case IXGBE_LINK_SPEED_1GB_FULL:
		speed_str = "1 Gbps";
		break;
	case IXGBE_LINK_SPEED_100_FULL:
		speed_str = "100 Mbps";
		break;
	case IXGBE_LINK_SPEED_10_FULL:
		speed_str = "10 Mbps";
		break;
	default:
		speed_str = "unknown speed";
		break;
	}
	e_info(drv, "NIC Link is Up %s, Flow Control: %s\n", speed_str,
	       ((flow_rx && flow_tx) ? "RX/TX" :
	       (flow_rx ? "RX" :
	       (flow_tx ? "TX" : "None"))));

	netif_carrier_on(netdev);
#ifdef IFLA_VF_MAX
	ixgbe_check_vf_rate_limit(adapter);
#endif /* IFLA_VF_MAX */
	/* Turn on malicious driver detection */
	if ((adapter->num_vfs) && (hw->mac.ops.enable_mdd) &&
		(adapter->flags & IXGBE_FLAG_MDD_ENABLED))
		hw->mac.ops.enable_mdd(hw);

	netif_tx_wake_all_queues(netdev);
	/* update the default user priority for VFs */
	ixgbe_update_default_up(adapter);
}

/**
 * ixgbe_watchdog_link_is_down - update netif_carrier status and
 *                               print link down message
 * @adapter: pointer to the adapter structure
 **/
static void ixgbe_watchdog_link_is_down(struct ixgbe_adapter *adapter)
{
	struct net_device *netdev = adapter->netdev;
	struct ixgbe_hw *hw = &adapter->hw;

	adapter->link_up = false;
	adapter->link_speed = 0;

	/* only continue if link was up previously */
	if (!netif_carrier_ok(netdev))
		return;

	/* poll for SFP+ cable when link is down */
	if (ixgbe_is_sfp(hw) && hw->mac.type == ixgbe_mac_82598EB)
		adapter->flags2 |= IXGBE_FLAG2_SEARCH_FOR_SFP;

#ifdef HAVE_PTP_1588_CLOCK
	if (test_bit(__IXGBE_PTP_RUNNING, &adapter->state))
		ixgbe_ptp_start_cyclecounter(adapter);

#endif
	e_info(drv, "NIC Link is Down\n");
	netif_carrier_off(netdev);
	netif_tx_stop_all_queues(netdev);

	/* ping all the active vfs to let them know link has changed */
	ixgbe_ping_all_vfs(adapter);
}

static bool ixgbe_ring_tx_pending(struct ixgbe_adapter *adapter)
{
	int i;

	for (i = 0; i < adapter->num_tx_queues; i++) {
		struct ixgbe_ring *tx_ring = adapter->tx_ring[i];

		if (tx_ring->next_to_use != tx_ring->next_to_clean)
			return true;
	}

	for (i = 0; i < adapter->num_xdp_queues; i++) {
		struct ixgbe_ring *ring = adapter->xdp_ring[i];

		if (ring->next_to_use != ring->next_to_clean)
			return true;
	}

	return false;
}

static bool ixgbe_vf_tx_pending(struct ixgbe_adapter *adapter)
{
	struct ixgbe_hw *hw = &adapter->hw;
	struct ixgbe_ring_feature *vmdq = &adapter->ring_feature[RING_F_VMDQ];
	u32 q_per_pool = __ALIGN_MASK(1, ~vmdq->mask);

	int i, j;

	if (!adapter->num_vfs)
		return false;

	/* resetting the PF is only needed for MACs < X550 */
	if (hw->mac.type >= ixgbe_mac_X550)
		return false;
	for (i = 0; i < adapter->num_vfs; i++) {
		for (j = 0; j < q_per_pool; j++) {
			u32 h, t;

			h = IXGBE_READ_REG(hw, IXGBE_PVFTDHn(q_per_pool, i, j));
			t = IXGBE_READ_REG(hw, IXGBE_PVFTDTn(q_per_pool, i, j));

			if (h != t)
				return true;
		}
	}

	return false;
}

/**
 * ixgbe_watchdog_flush_tx - flush queues on link down
 * @adapter: pointer to the device adapter structure
 **/
static void ixgbe_watchdog_flush_tx(struct ixgbe_adapter *adapter)
{
	if (!netif_carrier_ok(adapter->netdev)) {
		if (ixgbe_ring_tx_pending(adapter) ||
		    ixgbe_vf_tx_pending(adapter)) {
			/* We've lost link, so the controller stops DMA,
			 * but we've got queued Tx work that's never going
			 * to get done, so reset controller to flush Tx.
			 * (Do the reset outside of interrupt context).
			 */
			e_warn(drv, "initiating reset due to lost link with pending Tx work\n");
			set_bit(__IXGBE_RESET_REQUESTED, &adapter->state);
		}
	}
}

#ifdef CONFIG_PCI_IOV
static inline void ixgbe_issue_vf_flr(struct ixgbe_adapter *adapter,
				      struct pci_dev *vfdev)
{
	int pos, i;
	u16 status;

	/* wait for pending transactions on the bus */
	for (i = 0; i < 4; i++) {
		if (i)
			msleep((1 << (i - 1)) * 100);

		pcie_capability_read_word(vfdev, PCI_EXP_DEVSTA, &status);
		if (!(status & PCI_EXP_DEVSTA_TRPND))
			goto clear;
	}

	e_dev_warn("Issuing VFLR with pending transactions\n");

clear:
	pos = pci_find_capability(vfdev, PCI_CAP_ID_EXP);
	if (!pos)
		return;

	e_dev_err("Issuing VFLR for VF %s\n", pci_name(vfdev));
	pci_write_config_word(vfdev, pos + PCI_EXP_DEVCTL,
			      PCI_EXP_DEVCTL_BCR_FLR);
	msleep(100);
}

static void ixgbe_check_for_bad_vf(struct ixgbe_adapter *adapter)
{
	struct ixgbe_hw *hw = &adapter->hw;
	struct pci_dev *pdev = adapter->pdev;
	unsigned int vf;
	u32 gpc;

	if (!(netif_carrier_ok(adapter->netdev)))
		return;

	gpc = IXGBE_READ_REG(hw, IXGBE_TXDGPC);
	if (gpc) /* If incrementing then no need for the check below */
		return;
	/*
	 * Check to see if a bad DMA write target from an errant or
	 * malicious VF has caused a PCIe error.  If so then we can
	 * issue a VFLR to the offending VF(s) and then resume without
	 * requesting a full slot reset.
	 */

	if (!pdev)
		return;

	/* check status reg for all VFs owned by this PF */
	for (vf = 0; vf < adapter->num_vfs; ++vf) {
		struct pci_dev *vfdev = adapter->vfinfo[vf].vfdev;
		u16 status_reg;

		if (!vfdev)
			continue;
		pci_read_config_word(vfdev, PCI_STATUS, &status_reg);
		if (status_reg != IXGBE_FAILED_READ_CFG_WORD &&
		    status_reg & PCI_STATUS_REC_MASTER_ABORT)
			ixgbe_issue_vf_flr(adapter, vfdev);
	}
}

static void ixgbe_spoof_check(struct ixgbe_adapter *adapter)
{
	u32 ssvpc;

	/* Do not perform spoof check for 82598 or if not in IOV mode */
	if (adapter->hw.mac.type == ixgbe_mac_82598EB ||
	    adapter->num_vfs == 0)
		return;

	ssvpc = IXGBE_READ_REG(&adapter->hw, IXGBE_SSVPC);

	/*
	 * ssvpc register is cleared on read, if zero then no
	 * spoofed packets in the last interval.
	 */
	if (!ssvpc)
		return;

	e_warn(drv, "%d Spoofed packets detected\n", ssvpc);
}

#endif /* CONFIG_PCI_IOV */

/**
 * ixgbe_watchdog_subtask - check and bring link up
 * @adapter: pointer to the device adapter structure
 **/
static void ixgbe_watchdog_subtask(struct ixgbe_adapter *adapter)
{
	/* if interface is down, removing or resetting, do nothing */
	if (test_bit(__IXGBE_DOWN, &adapter->state) ||
	    test_bit(__IXGBE_REMOVING, &adapter->state) ||
	    test_bit(__IXGBE_RESETTING, &adapter->state))
		return;

	ixgbe_watchdog_update_link(adapter);

	if (adapter->link_up)
		ixgbe_watchdog_link_is_up(adapter);
	else
		ixgbe_watchdog_link_is_down(adapter);
#ifdef CONFIG_PCI_IOV
	ixgbe_spoof_check(adapter);
	ixgbe_check_for_bad_vf(adapter);
#endif /* CONFIG_PCI_IOV */
	ixgbe_update_stats(adapter);

	ixgbe_watchdog_flush_tx(adapter);
}

/**
 * ixgbe_sfp_detection_subtask - poll for SFP+ cable
 * @adapter: the ixgbe adapter structure
 **/
static void ixgbe_sfp_detection_subtask(struct ixgbe_adapter *adapter)
{
	struct ixgbe_hw *hw = &adapter->hw;
	s32 err;

	/* not searching for SFP so there is nothing to do here */
	if (!(adapter->flags2 & IXGBE_FLAG2_SEARCH_FOR_SFP) &&
	    !(adapter->flags2 & IXGBE_FLAG2_SFP_NEEDS_RESET))
		return;

	if (adapter->sfp_poll_time &&
	    time_after(adapter->sfp_poll_time, jiffies))
		return;	/* If not yet time to poll for SFP */

	/* someone else is in init, wait until next service event */
	if (test_and_set_bit(__IXGBE_IN_SFP_INIT, &adapter->state))
		return;

	adapter->sfp_poll_time = jiffies + IXGBE_SFP_POLL_JIFFIES - 1;

	err = hw->phy.ops.identify_sfp(hw);
	if (err == IXGBE_ERR_SFP_NOT_SUPPORTED)
		goto sfp_out;

	if (err == IXGBE_ERR_SFP_NOT_PRESENT) {
		/* If no cable is present, then we need to reset
		 * the next time we find a good cable. */
		adapter->flags2 |= IXGBE_FLAG2_SFP_NEEDS_RESET;
	}

	/* exit on error */
	if (err)
		goto sfp_out;

	/* exit if reset not needed */
	if (!(adapter->flags2 & IXGBE_FLAG2_SFP_NEEDS_RESET))
		goto sfp_out;

	adapter->flags2 &= ~IXGBE_FLAG2_SFP_NEEDS_RESET;

	/*
	 * A module may be identified correctly, but the EEPROM may not have
	 * support for that module.  setup_sfp() will fail in that case, so
	 * we should not allow that module to load.
	 */
	if (hw->mac.type == ixgbe_mac_82598EB)
		err = hw->phy.ops.reset(hw);
	else
		err = hw->mac.ops.setup_sfp(hw);

	if (err == IXGBE_ERR_SFP_NOT_SUPPORTED)
		goto sfp_out;

	adapter->flags |= IXGBE_FLAG_NEED_LINK_CONFIG;
	e_info(probe, "detected SFP+: %d\n", hw->phy.sfp_type);

sfp_out:
	clear_bit(__IXGBE_IN_SFP_INIT, &adapter->state);

	if ((err == IXGBE_ERR_SFP_NOT_SUPPORTED) &&
	    adapter->netdev_registered) {
		e_dev_err("failed to initialize because an unsupported "
			  "SFP+ module type was detected.\n");
		e_dev_err("Reload the driver after installing a "
			  "supported module.\n");
		unregister_netdev(adapter->netdev);
		adapter->netdev_registered = false;
	}
}

/**
 * ixgbe_sfp_link_config_subtask - set up link SFP after module install
 * @adapter: the ixgbe adapter structure
 **/
static void ixgbe_sfp_link_config_subtask(struct ixgbe_adapter *adapter)
{
	struct ixgbe_hw *hw = &adapter->hw;
	u32 cap_speed;
	u32 speed = 0;
	bool autoneg = false;

	if (!(adapter->flags & IXGBE_FLAG_NEED_LINK_CONFIG))
		return;

	/* someone else is in init, wait until next service event */
	if (test_and_set_bit(__IXGBE_IN_SFP_INIT, &adapter->state))
		return;

	adapter->flags &= ~IXGBE_FLAG_NEED_LINK_CONFIG;

	hw->mac.ops.get_link_capabilities(hw, &cap_speed, &autoneg);

	/* Advertise highest capable link speed */
	if (cap_speed & IXGBE_LINK_SPEED_10GB_FULL) {
		if (!autoneg)
			speed = IXGBE_LINK_SPEED_10GB_FULL;
		else
			speed = cap_speed;
	} else if (cap_speed == IXGBE_LINK_SPEED_1GB_FULL) {
		speed = IXGBE_LINK_SPEED_1GB_FULL;
	}

	if (hw->mac.ops.setup_link)
		hw->mac.ops.setup_link(hw, speed, true);

	adapter->flags |= IXGBE_FLAG_NEED_LINK_UPDATE;
	adapter->link_check_timeout = jiffies;
	clear_bit(__IXGBE_IN_SFP_INIT, &adapter->state);
}

/**
 * ixgbe_service_timer - Timer Call-back
 * @t: pointer to timer_list
 **/
static void ixgbe_service_timer(struct timer_list *t)
{
	struct ixgbe_adapter *adapter = from_timer(adapter, t, service_timer);
	unsigned long next_event_offset;

	/* poll faster when waiting for link */
	if (adapter->flags & IXGBE_FLAG_NEED_LINK_UPDATE)
		next_event_offset = HZ / 10;
	else
		next_event_offset = HZ * 2;

	/* Reset the timer */
	mod_timer(&adapter->service_timer, next_event_offset + jiffies);

	ixgbe_service_event_schedule(adapter);
}

static void ixgbe_phy_interrupt_subtask(struct ixgbe_adapter *adapter)
{
	u32 status;

	if (!(adapter->flags2 & IXGBE_FLAG2_PHY_INTERRUPT))
		return;
	adapter->flags2 &= ~IXGBE_FLAG2_PHY_INTERRUPT;
	status = ixgbe_handle_lasi(&adapter->hw);
	if (status != IXGBE_ERR_OVERTEMP)
		return;
	e_crit(drv, "%s\n", ixgbe_overheat_msg);
}

static void ixgbe_reset_subtask(struct ixgbe_adapter *adapter)
{
	if (!test_and_clear_bit(__IXGBE_RESET_REQUESTED, &adapter->state))
		return;

	rtnl_lock();
	/* If we're already down or resetting, just bail */
	if (test_bit(__IXGBE_DOWN, &adapter->state) ||
	    test_bit(__IXGBE_REMOVING, &adapter->state) ||
	    test_bit(__IXGBE_RESETTING, &adapter->state)) {
		rtnl_unlock();
		return;
	}

	netdev_err(adapter->netdev, "Reset adapter\n");
	adapter->tx_timeout_count++;

	ixgbe_reinit_locked(adapter);
	rtnl_unlock();
}

/**
 * ixgbe_check_fw_error - Check firmware for errors
 * @adapter: the adapter private structure
 *
 * Check firmware errors in register FWSM
 **/
static bool ixgbe_check_fw_error(struct ixgbe_adapter *adapter)
{
	struct ixgbe_hw *hw = &adapter->hw;

	if (hw->mac.ops.fw_recovery_mode && hw->mac.ops.fw_recovery_mode(hw)) {
		e_dev_err("Firmware recovery mode detected. Limiting functionality. Refer to the Intel(R) Ethernet Adapters and Devices User Guide for details on firmware recovery mode.\n");
		return true;
	}

	return false;
}

/**
 * ixgbe_service_task - manages and runs subtasks
 * @work: pointer to work_struct containing our data
 **/
static void ixgbe_service_task(struct work_struct *work)
{
	struct ixgbe_adapter *adapter = container_of(work,
						     struct ixgbe_adapter,
						     service_task);
	if (IXGBE_REMOVED(adapter->hw.hw_addr)) {
		if (!test_bit(__IXGBE_DOWN, &adapter->state)) {
			rtnl_lock();
			ixgbe_down(adapter);
			rtnl_unlock();
		}
		ixgbe_service_event_complete(adapter);
		return;
	}

	if (ixgbe_check_fw_error(adapter)) {
		if (!test_bit(__IXGBE_DOWN, &adapter->state)) {
			unregister_netdev(adapter->netdev);
			adapter->netdev_registered = false;
		}
		ixgbe_service_event_complete(adapter);
		return;
	}
#if defined(HAVE_UDP_ENC_RX_OFFLOAD) || defined(HAVE_VXLAN_RX_OFFLOAD)
	if (adapter->flags2 & IXGBE_FLAG2_UDP_TUN_REREG_NEEDED) {
		rtnl_lock();
		adapter->flags2 &= ~IXGBE_FLAG2_UDP_TUN_REREG_NEEDED;
#ifdef HAVE_UDP_ENC_RX_OFFLOAD
		udp_tunnel_get_rx_info(adapter->netdev);
#else
		vxlan_get_rx_port(adapter->netdev);
#endif /* HAVE_UDP_ENC_RX_OFFLOAD */
		rtnl_unlock();
	}
#endif /* HAVE_UDP_ENC_RX_OFFLOAD || HAVE_VXLAN_RX_OFFLOAD */
	ixgbe_reset_subtask(adapter);
	ixgbe_phy_interrupt_subtask(adapter);
	ixgbe_sfp_detection_subtask(adapter);
	ixgbe_sfp_link_config_subtask(adapter);
	ixgbe_check_overtemp_subtask(adapter);
	ixgbe_watchdog_subtask(adapter);
#ifdef HAVE_TX_MQ
	ixgbe_fdir_reinit_subtask(adapter);
#endif
	ixgbe_check_hang_subtask(adapter);
#ifdef HAVE_PTP_1588_CLOCK
	if (test_bit(__IXGBE_PTP_RUNNING, &adapter->state)) {
		ixgbe_ptp_overflow_check(adapter);
		if (unlikely(adapter->flags & IXGBE_FLAG_RX_HWTSTAMP_IN_REGISTER))
			ixgbe_ptp_rx_hang(adapter);
		ixgbe_ptp_tx_hang(adapter);
	}
#endif /* HAVE_PTP_1588_CLOCK */

	ixgbe_service_event_complete(adapter);
}

static int ixgbe_tso(struct ixgbe_ring *tx_ring,
		     struct ixgbe_tx_buffer *first,
		     u8 *hdr_len)
{
#ifdef NETIF_F_TSO
	u32 vlan_macip_lens, type_tucmd, mss_l4len_idx;
	struct sk_buff *skb = first->skb;
	union {
		struct iphdr *v4;
		struct ipv6hdr *v6;
		unsigned char *hdr;
	} ip;
	union {
		struct tcphdr *tcp;
		unsigned char *hdr;
	} l4;
	u32 paylen, l4_offset;
	int err;

	if (skb->ip_summed != CHECKSUM_PARTIAL)
		return 0;

	if (!skb_is_gso(skb))
		return 0;

	err = skb_cow_head(skb, 0);
	if (err < 0)
		return err;

	ip.hdr = skb_network_header(skb);
	l4.hdr = skb_checksum_start(skb);

	/* ADV DTYP TUCMD MKRLOC/ISCSIHEDLEN */
	type_tucmd = IXGBE_ADVTXD_TUCMD_L4T_TCP;

	/* initialize outer IP header fields */
	if (ip.v4->version == 4) {
		unsigned char *csum_start = skb_checksum_start(skb);
		unsigned char *trans_start = ip.hdr + (ip.v4->ihl * 4);

		/* IP header will have to cancel out any data that
		 * is not a part of the outer IP header
		 */
		ip.v4->check = csum_fold(csum_partial(trans_start,
						      csum_start - trans_start,
						      0));
		type_tucmd |= IXGBE_ADVTXD_TUCMD_IPV4;

		ip.v4->tot_len = 0;
		first->tx_flags |= IXGBE_TX_FLAGS_TSO |
				   IXGBE_TX_FLAGS_CSUM |
				   IXGBE_TX_FLAGS_IPV4;
	} else {
		ip.v6->payload_len = 0;
		first->tx_flags |= IXGBE_TX_FLAGS_TSO |
				   IXGBE_TX_FLAGS_CSUM;
	}

	/* determine offset of inner transport header */
	l4_offset = l4.hdr - skb->data;

	/* compute length of segmentation header */
	*hdr_len = (l4.tcp->doff * 4) + l4_offset;

	/* remove payload length from inner checksum */
	paylen = skb->len - l4_offset;
	csum_replace_by_diff(&l4.tcp->check, htonl(paylen));

	/* update gso size and bytecount with header size */
	first->gso_segs = skb_shinfo(skb)->gso_segs;
	first->bytecount += (first->gso_segs - 1) * *hdr_len;

	/* mss_l4len_id: use 0 as index for TSO */
	mss_l4len_idx = (*hdr_len - l4_offset) << IXGBE_ADVTXD_L4LEN_SHIFT;
	mss_l4len_idx |= skb_shinfo(skb)->gso_size << IXGBE_ADVTXD_MSS_SHIFT;

	/* vlan_macip_lens: HEADLEN, MACLEN, VLAN tag */
	vlan_macip_lens = l4.hdr - ip.hdr;
	vlan_macip_lens |= (ip.hdr - skb->data) << IXGBE_ADVTXD_MACLEN_SHIFT;
	vlan_macip_lens |= first->tx_flags & IXGBE_TX_FLAGS_VLAN_MASK;

	ixgbe_tx_ctxtdesc(tx_ring, vlan_macip_lens, 0, type_tucmd,
			  mss_l4len_idx);

	return 1;
#else
	return 0;
#endif /* NETIF_F_TSO */
}

static inline bool ixgbe_ipv6_csum_is_sctp(struct sk_buff *skb)
{
	unsigned int offset = 0;

	ipv6_find_hdr(skb, &offset, IPPROTO_SCTP, NULL, NULL);

	return offset == skb_checksum_start_offset(skb);
}

static void ixgbe_tx_csum(struct ixgbe_ring *tx_ring,
			  struct ixgbe_tx_buffer *first)
{
	struct sk_buff *skb = first->skb;
	u32 vlan_macip_lens = 0;
	u32 type_tucmd = 0;

	if (skb->ip_summed != CHECKSUM_PARTIAL) {
csum_failed:
		if (!(first->tx_flags & (IXGBE_TX_FLAGS_HW_VLAN |
					 IXGBE_TX_FLAGS_CC)))
			return;
		goto no_csum;
	}

	switch (skb->csum_offset) {
	case offsetof(struct tcphdr, check):
		type_tucmd = IXGBE_ADVTXD_TUCMD_L4T_TCP;
		/* fall through */
	case offsetof(struct udphdr, check):
		break;
	case offsetof(struct sctphdr, checksum):
		/* validate that this is actually an SCTP request */
		if (((first->protocol == htons(ETH_P_IP)) &&
		     (ip_hdr(skb)->protocol == IPPROTO_SCTP)) ||
		    ((first->protocol == htons(ETH_P_IPV6)) &&
		     ixgbe_ipv6_csum_is_sctp(skb))) {
			type_tucmd = IXGBE_ADVTXD_TUCMD_L4T_SCTP;
			break;
		}
		/* fall through */
	default:
		skb_checksum_help(skb);
		goto csum_failed;
	}

	/* update TX checksum flag */
	first->tx_flags |= IXGBE_TX_FLAGS_CSUM;
	vlan_macip_lens = skb_checksum_start_offset(skb) -
			  skb_network_offset(skb);
no_csum:
	/* vlan_macip_lens: MACLEN, VLAN tag */
	vlan_macip_lens |= skb_network_offset(skb) << IXGBE_ADVTXD_MACLEN_SHIFT;
	vlan_macip_lens |= first->tx_flags & IXGBE_TX_FLAGS_VLAN_MASK;

	ixgbe_tx_ctxtdesc(tx_ring, vlan_macip_lens, 0, type_tucmd, 0);
}

#define IXGBE_SET_FLAG(_input, _flag, _result) \
	((_flag <= _result) ? \
	 ((u32)(_input & _flag) * (_result / _flag)) : \
	 ((u32)(_input & _flag) / (_flag / _result)))

static u32 ixgbe_tx_cmd_type(u32 tx_flags)
{
	/* set type for advanced descriptor with frame checksum insertion */
	u32 cmd_type = IXGBE_ADVTXD_DTYP_DATA |
		       IXGBE_ADVTXD_DCMD_DEXT |
		       IXGBE_ADVTXD_DCMD_IFCS;

	/* set HW vlan bit if vlan is present */
	cmd_type |= IXGBE_SET_FLAG(tx_flags, IXGBE_TX_FLAGS_HW_VLAN,
				   IXGBE_ADVTXD_DCMD_VLE);

	/* set segmentation enable bits for TSO/FSO */
	cmd_type |= IXGBE_SET_FLAG(tx_flags, IXGBE_TX_FLAGS_TSO,
				   IXGBE_ADVTXD_DCMD_TSE);

	/* set timestamp bit if present */
	cmd_type |= IXGBE_SET_FLAG(tx_flags, IXGBE_TX_FLAGS_TSTAMP,
				   IXGBE_ADVTXD_MAC_TSTAMP);

	return cmd_type;
}

static void ixgbe_tx_olinfo_status(union ixgbe_adv_tx_desc *tx_desc,
				   u32 tx_flags, unsigned int paylen)
{
	u32 olinfo_status = paylen << IXGBE_ADVTXD_PAYLEN_SHIFT;

	/* enable L4 checksum for TSO and TX checksum offload */
	olinfo_status |= IXGBE_SET_FLAG(tx_flags,
					IXGBE_TX_FLAGS_CSUM,
					IXGBE_ADVTXD_POPTS_TXSM);

	/* enble IPv4 checksum for TSO */
	olinfo_status |= IXGBE_SET_FLAG(tx_flags,
					IXGBE_TX_FLAGS_IPV4,
					IXGBE_ADVTXD_POPTS_IXSM);

	/*
	 * Check Context must be set if Tx switch is enabled, which it
	 * always is for case where virtual functions are running
	 */
	olinfo_status |= IXGBE_SET_FLAG(tx_flags,
					IXGBE_TX_FLAGS_CC,
					IXGBE_ADVTXD_CC);

	tx_desc->read.olinfo_status = cpu_to_le32(olinfo_status);
}

static int __ixgbe_maybe_stop_tx(struct ixgbe_ring *tx_ring, u16 size)
{
	netif_stop_subqueue(tx_ring->netdev, tx_ring->queue_index);

	/* Herbert's original patch had:
	 *  smp_mb__after_netif_stop_queue();
	 * but since that doesn't exist yet, just open code it.
	 */
	smp_mb();

	/* We need to check again in a case another CPU has just
	 * made room available.
	 */
	if (likely(ixgbe_desc_unused(tx_ring) < size))
		return -EBUSY;

	/* A reprieve! - use start_queue because it doesn't call schedule */
	netif_start_subqueue(tx_ring->netdev, tx_ring->queue_index);
	++tx_ring->tx_stats.restart_queue;
	return 0;
}

static inline int ixgbe_maybe_stop_tx(struct ixgbe_ring *tx_ring, u16 size)
{
	if (likely(ixgbe_desc_unused(tx_ring) >= size))
		return 0;

	return __ixgbe_maybe_stop_tx(tx_ring, size);
}

static int ixgbe_tx_map(struct ixgbe_ring *tx_ring,
			struct ixgbe_tx_buffer *first,
			const u8 hdr_len)
{
	struct sk_buff *skb = first->skb;
	struct ixgbe_tx_buffer *tx_buffer;
	union ixgbe_adv_tx_desc *tx_desc;
	skb_frag_t *frag;
	dma_addr_t dma;
	unsigned int data_len, size;
	u32 tx_flags = first->tx_flags;
	u32 cmd_type = ixgbe_tx_cmd_type(tx_flags);
	u16 i = tx_ring->next_to_use;

	tx_desc = IXGBE_TX_DESC(tx_ring, i);

	ixgbe_tx_olinfo_status(tx_desc, tx_flags, skb->len - hdr_len);

	size = skb_headlen(skb);
	data_len = skb->data_len;

#if IS_ENABLED(CONFIG_FCOE)
	if (tx_flags & IXGBE_TX_FLAGS_FCOE) {
		if (data_len < sizeof(struct fcoe_crc_eof)) {
			size -= sizeof(struct fcoe_crc_eof) - data_len;
			data_len = 0;
		} else {
			data_len -= sizeof(struct fcoe_crc_eof);
		}
	}
#endif /* CONFIG_FCOE */

	dma = dma_map_single(tx_ring->dev, skb->data, size, DMA_TO_DEVICE);

	tx_buffer = first;

	for (frag = &skb_shinfo(skb)->frags[0];; frag++) {
		if (dma_mapping_error(tx_ring->dev, dma))
			goto dma_error;

		/* record length, and DMA address */
		dma_unmap_len_set(tx_buffer, len, size);
		dma_unmap_addr_set(tx_buffer, dma, dma);

		tx_desc->read.buffer_addr = cpu_to_le64(dma);

		while (unlikely(size > IXGBE_MAX_DATA_PER_TXD)) {
			tx_desc->read.cmd_type_len =
				cpu_to_le32(cmd_type ^ IXGBE_MAX_DATA_PER_TXD);

			i++;
			tx_desc++;
			if (i == tx_ring->count) {
				tx_desc = IXGBE_TX_DESC(tx_ring, 0);
				i = 0;
			}
			tx_desc->read.olinfo_status = 0;

			dma += IXGBE_MAX_DATA_PER_TXD;
			size -= IXGBE_MAX_DATA_PER_TXD;

			tx_desc->read.buffer_addr = cpu_to_le64(dma);
		}

		if (likely(!data_len))
			break;

		tx_desc->read.cmd_type_len = cpu_to_le32(cmd_type ^ size);

		i++;
		tx_desc++;
		if (i == tx_ring->count) {
			tx_desc = IXGBE_TX_DESC(tx_ring, 0);
			i = 0;
		}
		tx_desc->read.olinfo_status = 0;

#if IS_ENABLED(CONFIG_FCOE)
		size = min_t(unsigned int, data_len, skb_frag_size(frag));
#else
		size = skb_frag_size(frag);
#endif
		data_len -= size;

		dma = skb_frag_dma_map(tx_ring->dev, frag, 0, size,
				       DMA_TO_DEVICE);

		tx_buffer = &tx_ring->tx_buffer_info[i];
	}

	/* write last descriptor with RS and EOP bits */
	cmd_type |= size | IXGBE_TXD_CMD;
	tx_desc->read.cmd_type_len = cpu_to_le32(cmd_type);

	netdev_tx_sent_queue(txring_txq(tx_ring), first->bytecount);

	/* set the timestamp */
	first->time_stamp = jiffies;

#ifndef HAVE_TRANS_START_IN_QUEUE
	netdev_ring(tx_ring)->trans_start = first->time_stamp;
#endif
	/*
	 * Force memory writes to complete before letting h/w know there
	 * are new descriptors to fetch.  (Only applicable for weak-ordered
	 * memory model archs, such as IA-64).
	 *
	 * We also need this memory barrier to make certain all of the
	 * status bits have been updated before next_to_watch is written.
	 */
	wmb();

	/* set next_to_watch value indicating a packet is present */
	first->next_to_watch = tx_desc;

	i++;
	if (i == tx_ring->count)
		i = 0;

	tx_ring->next_to_use = i;

	ixgbe_maybe_stop_tx(tx_ring, DESC_NEEDED);

	skb_tx_timestamp(skb);

	if (netif_xmit_stopped(txring_txq(tx_ring)) || !netdev_xmit_more()) {
		writel(i, tx_ring->tail);
#ifndef SPIN_UNLOCK_IMPLIES_MMIOWB

		/* The following mmiowb() is required on certain
		 * architechtures (IA64/Altix in particular) in order to
		 * synchronize the I/O calls with respect to a spin lock. This
		 * is because the wmb() on those architectures does not
		 * guarantee anything for posted I/O writes.
		 *
		 * Note that the associated spin_unlock() is not within the
		 * driver code, but in the networking core stack.
		 */
		mmiowb();
#endif /* SPIN_UNLOCK_IMPLIES_MMIOWB */
	}

	return 0;
dma_error:
	dev_err(tx_ring->dev, "TX DMA map failed\n");

	/* clear dma mappings for failed tx_buffer_info map */
	for (;;) {
		tx_buffer = &tx_ring->tx_buffer_info[i];
		if (dma_unmap_len(tx_buffer, len))
			dma_unmap_page(tx_ring->dev,
				       dma_unmap_addr(tx_buffer, dma),
				       dma_unmap_len(tx_buffer, len),
				       DMA_TO_DEVICE);
		dma_unmap_len_set(tx_buffer, len, 0);
		if (tx_buffer == first)
			break;
		if (i == 0)
			i += tx_ring->count;
		i--;
	}

	dev_kfree_skb_any(first->skb);
	first->skb = NULL;

	tx_ring->next_to_use = i;

	return -1;
}

static void ixgbe_atr(struct ixgbe_ring *ring,
		      struct ixgbe_tx_buffer *first)
{
	struct ixgbe_q_vector *q_vector = ring->q_vector;
	union ixgbe_atr_hash_dword input = { .dword = 0 };
	union ixgbe_atr_hash_dword common = { .dword = 0 };
	union {
		unsigned char *network;
		struct iphdr *ipv4;
		struct ipv6hdr *ipv6;
	} hdr;
	struct tcphdr *th;
	struct sk_buff *skb;
	unsigned int hlen;
	__be16 vlan_id;
	int l4_proto;

	/* if ring doesn't have a interrupt vector, cannot perform ATR */
	if (!q_vector)
		return;

	/* do nothing if sampling is disabled */
	if (!ring->atr_sample_rate)
		return;

	ring->atr_count++;

	/* currently only IPv4/IPv6 with TCP is supported */
	if (first->protocol != htons(ETH_P_IP) &&
	    first->protocol != htons(ETH_P_IPV6))
		return;

	/* snag network header to get L4 type and address */
	skb = first->skb;
	hdr.network = skb_network_header(skb);

#if defined(HAVE_UDP_ENC_RX_OFFLOAD) || defined(HAVE_VXLAN_RX_OFFLOAD)
	if (unlikely(hdr.network <= skb->data))
		return;

	if (skb->encapsulation &&
	    first->protocol == htons(ETH_P_IP) &&
	    hdr.ipv4->protocol == IPPROTO_UDP) {
		struct ixgbe_adapter *adapter = q_vector->adapter;

		if (unlikely(skb_tail_pointer(skb) < hdr.network +
			     VXLAN_HEADROOM))
			return;

		/* verify the port is recognized as VXLAN or GENEVE*/
		if (adapter->vxlan_port &&
		    udp_hdr(skb)->dest == adapter->vxlan_port)
			hdr.network = skb_inner_network_header(skb);

#ifdef HAVE_UDP_ENC_RX_OFFLOAD
		if (adapter->geneve_port &&
		    udp_hdr(skb)->dest == adapter->geneve_port)
			hdr.network = skb_inner_network_header(skb);
#endif
	}
#endif /* HAVE_UDP_ENC_RX_OFFLOAD || HAVE_VXLAN_RX_OFFLOAD */

	/* Make sure we have at least [minimum IPv4 header + TCP]
	 * or [IPv6 header] bytes
	 */
	if (unlikely(skb_tail_pointer(skb) < hdr.network + 40))
		return;

	/* Currently only IPv4/IPv6 with TCP is supported */
	switch (hdr.ipv4->version) {
	case IPVERSION:
		/* access ihl as u8 to avoid unaligned access on ia64 */
		hlen = (hdr.network[0] & 0x0F) << 2;
		l4_proto = hdr.ipv4->protocol;
		break;
	case 6:
		hlen = hdr.network - skb->data;
		l4_proto = ipv6_find_hdr(skb, &hlen, IPPROTO_TCP, NULL, NULL);
		hlen -= hdr.network - skb->data;
		break;
	default:
		return;
	}

	if (l4_proto != IPPROTO_TCP)
		return;

	if (unlikely(skb_tail_pointer(skb) < hdr.network +
		     hlen + sizeof(struct tcphdr)))
		return;

	th = (struct tcphdr *)(hdr.network + hlen);

	/* skip this packet since the socket is closing */
	if (th->fin)
		return;

	/* sample on all syn packets or once every atr sample count */
	if (!th->syn && (ring->atr_count < ring->atr_sample_rate))
		return;

	/* reset sample count */
	ring->atr_count = 0;

	vlan_id = htons(first->tx_flags >> IXGBE_TX_FLAGS_VLAN_SHIFT);

	/*
	 * src and dst are inverted, think how the receiver sees them
	 *
	 * The input is broken into two sections, a non-compressed section
	 * containing vm_pool, vlan_id, and flow_type.  The rest of the data
	 * is XORed together and stored in the compressed dword.
	 */
	input.formatted.vlan_id = vlan_id;

	/*
	 * since src port and flex bytes occupy the same word XOR them together
	 * and write the value to source port portion of compressed dword
	 */
	if (first->tx_flags & (IXGBE_TX_FLAGS_SW_VLAN | IXGBE_TX_FLAGS_HW_VLAN))
		common.port.src ^= th->dest ^ htons(ETH_P_8021Q);
	else
		common.port.src ^= th->dest ^ first->protocol;
	common.port.dst ^= th->source;

	switch (hdr.ipv4->version) {
	case IPVERSION:
		input.formatted.flow_type = IXGBE_ATR_FLOW_TYPE_TCPV4;
		common.ip ^= hdr.ipv4->saddr ^ hdr.ipv4->daddr;
		break;
	case 6:
		input.formatted.flow_type = IXGBE_ATR_FLOW_TYPE_TCPV6;
		common.ip ^= hdr.ipv6->saddr.s6_addr32[0] ^
			     hdr.ipv6->saddr.s6_addr32[1] ^
			     hdr.ipv6->saddr.s6_addr32[2] ^
			     hdr.ipv6->saddr.s6_addr32[3] ^
			     hdr.ipv6->daddr.s6_addr32[0] ^
			     hdr.ipv6->daddr.s6_addr32[1] ^
			     hdr.ipv6->daddr.s6_addr32[2] ^
			     hdr.ipv6->daddr.s6_addr32[3];
		break;
	default:
		break;
	}

#if defined(HAVE_UDP_ENC_RX_OFFLOAD) || defined(HAVE_VXLAN_RX_OFFLOAD)
	if (hdr.network != skb_network_header(skb))
		input.formatted.flow_type |= IXGBE_ATR_L4TYPE_TUNNEL_MASK;
#endif /* HAVE_UDP_ENC_RX_OFFLOAD || HAVE_VXLAN_RX_OFFLOAD */

	/* This assumes the Rx queue and Tx queue are bound to the same CPU */
	ixgbe_fdir_add_signature_filter_82599(&q_vector->adapter->hw,
					      input, common, ring->queue_index);
}
#ifdef HAVE_NETDEV_SELECT_QUEUE
#if IS_ENABLED(CONFIG_FCOE)

#if defined(HAVE_NDO_SELECT_QUEUE_FALLBACK_REMOVED)
static u16 ixgbe_select_queue(struct net_device *dev, struct sk_buff *skb,
			      struct net_device *sb_dev)
#elif defined(HAVE_NDO_SELECT_QUEUE_SB_DEV)
static u16 ixgbe_select_queue(struct net_device *dev, struct sk_buff *skb,
			      __always_unused struct net_device *sb_dev,
			      select_queue_fallback_t fallback)
#elif defined(HAVE_NDO_SELECT_QUEUE_ACCEL_FALLBACK)
static u16 ixgbe_select_queue(struct net_device *dev, struct sk_buff *skb,
			      __always_unused void *accel,
			      select_queue_fallback_t fallback)
#elif defined(HAVE_NDO_SELECT_QUEUE_ACCEL)
static u16 ixgbe_select_queue(struct net_device *dev, struct sk_buff *skb,
			      __always_unused void *accel)
#else
static u16 ixgbe_select_queue(struct net_device *dev, struct sk_buff *skb)
#endif /* HAVE_NDO_SELECT_QUEUE_ACCEL_FALLBACK */
{
	struct ixgbe_adapter *adapter = netdev_priv(dev);
	struct ixgbe_ring_feature *f;
	int txq;

	/*
	 * only execute the code below if protocol is FCoE
	 * or FIP and we have FCoE enabled on the adapter
	 */
	switch (vlan_get_protocol(skb)) {
	case __constant_htons(ETH_P_FCOE):
	case __constant_htons(ETH_P_FIP):
		adapter = netdev_priv(dev);

		if (adapter->flags & IXGBE_FLAG_FCOE_ENABLED)
			break;
		/* fall through */
	default:
#if defined(HAVE_NDO_SELECT_QUEUE_FALLBACK_REMOVED)
		return netdev_pick_tx(dev, skb, sb_dev);
#elif defined(HAVE_NDO_SELECT_QUEUE_SB_DEV)
		return fallback(dev, skb, sb_dev);
#elif defined(HAVE_NDO_SELECT_QUEUE_ACCEL_FALLBACK)
		return fallback(dev, skb);
#else
		return __netdev_pick_tx(dev, skb);
#endif
	}

	f = &adapter->ring_feature[RING_F_FCOE];

	txq = skb_rx_queue_recorded(skb) ? skb_get_rx_queue(skb) :
					   smp_processor_id();

	while (txq >= f->indices)
		txq -= f->indices;

	return txq + f->offset;
}
#endif /* CONFIG_FCOE */
#endif /* HAVE_NETDEV_SELECT_QUEUE */

#ifdef HAVE_XDP_SUPPORT
#ifdef HAVE_XDP_FRAME_STRUCT
int ixgbe_xmit_xdp_ring(struct ixgbe_adapter *adapter,
			struct xdp_frame *xdpf)
#else
int ixgbe_xmit_xdp_ring(struct ixgbe_adapter *adapter,
			struct xdp_buff *xdp)
#endif
{
	struct ixgbe_ring *ring = adapter->xdp_ring[smp_processor_id()];
	struct ixgbe_tx_buffer *tx_buffer;
	union ixgbe_adv_tx_desc *tx_desc;
	u32 len, cmd_type;
	dma_addr_t dma;
	u16 i;

#ifdef HAVE_XDP_FRAME_STRUCT
	len = xdpf->len;
#else
	len = xdp->data_end - xdp->data;
#endif

	if (unlikely(!ixgbe_desc_unused(ring)))
		return IXGBE_XDP_CONSUMED;

#ifdef HAVE_XDP_FRAME_STRUCT
	dma = dma_map_single(ring->dev, xdpf->data, len, DMA_TO_DEVICE);
#else
	dma = dma_map_single(ring->dev, xdp->data, len, DMA_TO_DEVICE);
#endif
	if (dma_mapping_error(ring->dev, dma))
		return IXGBE_XDP_CONSUMED;

	/* record the location of the first descriptor for this packet */
	tx_buffer = &ring->tx_buffer_info[ring->next_to_use];
	tx_buffer->bytecount = len;
	tx_buffer->gso_segs = 1;
	tx_buffer->protocol = 0;

	i = ring->next_to_use;
	tx_desc = IXGBE_TX_DESC(ring, i);

	dma_unmap_len_set(tx_buffer, len, len);
	dma_unmap_addr_set(tx_buffer, dma, dma);
#ifdef HAVE_XDP_FRAME_STRUCT
	tx_buffer->xdpf = xdpf;
#else
	tx_buffer->data = xdp->data;
#endif

	tx_desc->read.buffer_addr = cpu_to_le64(dma);

	/* put descriptor type bits */
	cmd_type = IXGBE_ADVTXD_DTYP_DATA |
		   IXGBE_ADVTXD_DCMD_DEXT |
		   IXGBE_ADVTXD_DCMD_IFCS;
	cmd_type |= len | IXGBE_TXD_CMD;
	tx_desc->read.cmd_type_len = cpu_to_le32(cmd_type);
	tx_desc->read.olinfo_status =
		cpu_to_le32(len << IXGBE_ADVTXD_PAYLEN_SHIFT);

	/* Avoid any potential race with xdp_xmit and cleanup */
	smp_wmb();

	/* set next_to_watch value indicating a packet is present */
	i++;
	if (i == ring->count)
		i = 0;

	tx_buffer->next_to_watch = tx_desc;
	ring->next_to_use = i;

	return IXGBE_XDP_TX;
}

#ifdef HAVE_AF_XDP_ZC_SUPPORT
static void ixgbe_disable_txr_hw(struct ixgbe_adapter *adapter,
				 struct ixgbe_ring *tx_ring)
{
	unsigned long wait_delay, delay_interval;
	struct ixgbe_hw *hw = &adapter->hw;
	u8 reg_idx = tx_ring->reg_idx;
	int wait_loop;
	u32 txdctl;

	IXGBE_WRITE_REG(hw, IXGBE_TXDCTL(reg_idx), IXGBE_TXDCTL_SWFLSH);

	/* delay mechanism from ixgbe_disable_tx */
	delay_interval = ixgbe_get_completion_timeout(adapter) / 100;

	wait_loop = IXGBE_MAX_RX_DESC_POLL;
	wait_delay = delay_interval;

	while (wait_loop--) {
		usleep_range(wait_delay, wait_delay + 10);
		wait_delay += delay_interval * 2;
		txdctl = IXGBE_READ_REG(hw, IXGBE_TXDCTL(reg_idx));

		if (!(txdctl & IXGBE_TXDCTL_ENABLE))
			return;
	}

	e_err(drv, "TXDCTL.ENABLE not cleared within the polling period\n");
}

static void ixgbe_disable_txr(struct ixgbe_adapter *adapter,
			      struct ixgbe_ring *tx_ring)
{
	set_bit(__IXGBE_TX_DISABLED, &tx_ring->state);
	ixgbe_disable_txr_hw(adapter, tx_ring);
}

static void ixgbe_disable_rxr_hw(struct ixgbe_adapter *adapter,
				 struct ixgbe_ring *rx_ring)
{
	unsigned long wait_delay, delay_interval;
	struct ixgbe_hw *hw = &adapter->hw;
	u8 reg_idx = rx_ring->reg_idx;
	int wait_loop;
	u32 rxdctl;

	rxdctl = IXGBE_READ_REG(hw, IXGBE_RXDCTL(reg_idx));
	rxdctl &= ~IXGBE_RXDCTL_ENABLE;
	rxdctl |= IXGBE_RXDCTL_SWFLSH;

	/* write value back with RXDCTL.ENABLE bit cleared */
	IXGBE_WRITE_REG(hw, IXGBE_RXDCTL(reg_idx), rxdctl);

	/* RXDCTL.EN may not change on 82598 if link is down, so skip it */
	if (hw->mac.type == ixgbe_mac_82598EB &&
	    !(IXGBE_READ_REG(hw, IXGBE_LINKS) & IXGBE_LINKS_UP))
		return;

	/* delay mechanism from ixgbe_disable_rx */
	delay_interval = ixgbe_get_completion_timeout(adapter) / 100;

	wait_loop = IXGBE_MAX_RX_DESC_POLL;
	wait_delay = delay_interval;

	while (wait_loop--) {
		usleep_range(wait_delay, wait_delay + 10);
		wait_delay += delay_interval * 2;
		rxdctl = IXGBE_READ_REG(hw, IXGBE_RXDCTL(reg_idx));

		if (!(rxdctl & IXGBE_RXDCTL_ENABLE))
			return;
	}

	e_err(drv, "RXDCTL.ENABLE not cleared within the polling period\n");
}

static void ixgbe_reset_txr_stats(struct ixgbe_ring *tx_ring)
{
	memset(&tx_ring->stats, 0, sizeof(tx_ring->stats));
	memset(&tx_ring->tx_stats, 0, sizeof(tx_ring->tx_stats));
}

static void ixgbe_reset_rxr_stats(struct ixgbe_ring *rx_ring)
{
	memset(&rx_ring->stats, 0, sizeof(rx_ring->stats));
	memset(&rx_ring->rx_stats, 0, sizeof(rx_ring->rx_stats));
}

/**
 * ixgbe_txrx_ring_disable - Disable Rx/Tx/XDP Tx rings
 * @adapter: adapter structure
 * @ring: ring index
 *
 * This function disables a certain Rx/Tx/XDP Tx ring. The function
 * assumes that the netdev is running.
 **/
void ixgbe_txrx_ring_disable(struct ixgbe_adapter *adapter, int ring)
{
	struct ixgbe_ring *rx_ring, *tx_ring, *xdp_ring;

	rx_ring = adapter->rx_ring[ring];
	tx_ring = adapter->tx_ring[ring];
	xdp_ring = adapter->xdp_ring[ring];

	ixgbe_disable_txr(adapter, tx_ring);
	if (xdp_ring)
		ixgbe_disable_txr(adapter, xdp_ring);
	ixgbe_disable_rxr_hw(adapter, rx_ring);

	if (xdp_ring)
		synchronize_rcu();

	/* Rx/Tx/XDP Tx share the same napi context. */
	napi_disable(&rx_ring->q_vector->napi);

	ixgbe_clean_tx_ring(tx_ring);
	if (xdp_ring)
		ixgbe_clean_tx_ring(xdp_ring);
	ixgbe_clean_rx_ring(rx_ring);

	ixgbe_reset_txr_stats(tx_ring);
	if (xdp_ring)
		ixgbe_reset_txr_stats(xdp_ring);
	ixgbe_reset_rxr_stats(rx_ring);
}

/**
 * ixgbe_txrx_ring_enable - Enable Rx/Tx/XDP Tx rings
 * @adapter: adapter structure
 * @ring: ring index
 *
 * This function enables a certain Rx/Tx/XDP Tx ring. The function
 * assumes that the netdev is running.
 **/
void ixgbe_txrx_ring_enable(struct ixgbe_adapter *adapter, int ring)
{
	struct ixgbe_ring *rx_ring, *tx_ring, *xdp_ring;

	rx_ring = adapter->rx_ring[ring];
	tx_ring = adapter->tx_ring[ring];
	xdp_ring = adapter->xdp_ring[ring];

	/* Rx/Tx/XDP Tx share the same napi context. */
	napi_enable(&rx_ring->q_vector->napi);

	ixgbe_configure_tx_ring(adapter, tx_ring);
	if (xdp_ring)
		ixgbe_configure_tx_ring(adapter, xdp_ring);
	ixgbe_configure_rx_ring(adapter, rx_ring);

	clear_bit(__IXGBE_TX_DISABLED, &tx_ring->state);
	if (xdp_ring)
		clear_bit(__IXGBE_TX_DISABLED, &xdp_ring->state);
}
#endif /* HAVE_AF_XDP_ZC_SUPPORT */
#endif /* HAVE_XDP_SUPPORT */

netdev_tx_t ixgbe_xmit_frame_ring(struct sk_buff *skb,
				  struct ixgbe_adapter __maybe_unused *adapter,
				  struct ixgbe_ring *tx_ring)
{
	struct ixgbe_tx_buffer *first;
	int tso;
	u32 tx_flags = 0;
	unsigned short f;
	u16 count = TXD_USE_COUNT(skb_headlen(skb));
	__be16 protocol = skb->protocol;
	u8 hdr_len = 0;

	/*
	 * need: 1 descriptor per page * PAGE_SIZE/IXGBE_MAX_DATA_PER_TXD,
	 *       + 1 desc for skb_headlen/IXGBE_MAX_DATA_PER_TXD,
	 *       + 2 desc gap to keep tail from touching head,
	 *       + 1 desc for context descriptor,
	 * otherwise try next time
	 */
	for (f = 0; f < skb_shinfo(skb)->nr_frags; f++)
		count += TXD_USE_COUNT(skb_frag_size(&skb_shinfo(skb)->
						     frags[f]));

	if (ixgbe_maybe_stop_tx(tx_ring, count + 3)) {
		tx_ring->tx_stats.tx_busy++;
		return NETDEV_TX_BUSY;
	}

	/* record the location of the first descriptor for this packet */
	first = &tx_ring->tx_buffer_info[tx_ring->next_to_use];
	first->skb = skb;
	first->bytecount = skb->len;
	first->gso_segs = 1;

	/* if we have a HW VLAN tag being added default to the HW one */
	if (skb_vlan_tag_present(skb)) {
		tx_flags |= skb_vlan_tag_get(skb) << IXGBE_TX_FLAGS_VLAN_SHIFT;
		tx_flags |= IXGBE_TX_FLAGS_HW_VLAN;
	/* else if it is a SW VLAN check the next protocol and store the tag */
	} else if (protocol == htons(ETH_P_8021Q)) {
		struct vlan_hdr *vhdr, _vhdr;
		vhdr = skb_header_pointer(skb, ETH_HLEN, sizeof(_vhdr), &_vhdr);
		if (!vhdr)
			goto out_drop;

		protocol = vhdr->h_vlan_encapsulated_proto;
		tx_flags |= ntohs(vhdr->h_vlan_TCI) <<
				  IXGBE_TX_FLAGS_VLAN_SHIFT;
		tx_flags |= IXGBE_TX_FLAGS_SW_VLAN;
	}

#ifdef HAVE_PTP_1588_CLOCK
#ifdef SKB_SHARED_TX_IS_UNION
	if (unlikely(skb_tx(skb)->hardware) &&
	    adapter->ptp_clock) {
		if (!test_and_set_bit_lock(__IXGBE_PTP_TX_IN_PROGRESS,
					   &adapter->state)) {
			skb_tx(skb)->in_progress = 1;
#else
	if (unlikely(skb_shinfo(skb)->tx_flags & SKBTX_HW_TSTAMP) &&
	    adapter->ptp_clock) {
		if (!test_and_set_bit_lock(__IXGBE_PTP_TX_IN_PROGRESS,
				   &adapter->state)) {
			skb_shinfo(skb)->tx_flags |= SKBTX_IN_PROGRESS;
#endif
			tx_flags |= IXGBE_TX_FLAGS_TSTAMP;

			/* schedule check for Tx timestamp */
			adapter->ptp_tx_skb = skb_get(skb);
			adapter->ptp_tx_start = jiffies;
			schedule_work(&adapter->ptp_tx_work);
		} else {
			adapter->tx_hwtstamp_skipped++;
		}
	}

#endif
#ifdef CONFIG_PCI_IOV
	/*
	 * Use the l2switch_enable flag - would be false if the DMA
	 * Tx switch had been disabled.
	 */
	if (adapter->flags & IXGBE_FLAG_SRIOV_L2SWITCH_ENABLE)
		tx_flags |= IXGBE_TX_FLAGS_CC;

#endif
#ifdef HAVE_TX_MQ
	if ((adapter->flags & IXGBE_FLAG_DCB_ENABLED) &&
	    ((tx_flags & (IXGBE_TX_FLAGS_HW_VLAN | IXGBE_TX_FLAGS_SW_VLAN)) ||
	     (skb->priority != TC_PRIO_CONTROL))) {
		tx_flags &= ~IXGBE_TX_FLAGS_VLAN_PRIO_MASK;
#if IS_ENABLED(CONFIG_FCOE)
		/* for FCoE with DCB, we force the priority to what
		 * was specified by the switch */
		if ((adapter->flags & IXGBE_FLAG_FCOE_ENABLED) &&
		    ((protocol == htons(ETH_P_FCOE)) ||
		     (protocol == htons(ETH_P_FIP))))
			tx_flags |= adapter->fcoe.up <<
				    IXGBE_TX_FLAGS_VLAN_PRIO_SHIFT;
		else
#endif /* CONFIG_FCOE */
			tx_flags |= skb->priority <<
				    IXGBE_TX_FLAGS_VLAN_PRIO_SHIFT;
		if (tx_flags & IXGBE_TX_FLAGS_SW_VLAN) {
			struct vlan_ethhdr *vhdr;
			if (skb_header_cloned(skb) &&
			    pskb_expand_head(skb, 0, 0, GFP_ATOMIC))
				goto out_drop;
			vhdr = (struct vlan_ethhdr *)skb->data;
			vhdr->h_vlan_TCI = htons(tx_flags >>
						 IXGBE_TX_FLAGS_VLAN_SHIFT);
		} else {
			tx_flags |= IXGBE_TX_FLAGS_HW_VLAN;
		}
	}

#endif /* HAVE_TX_MQ */
	/* record initial flags and protocol */
	first->tx_flags = tx_flags;
	first->protocol = protocol;

#if IS_ENABLED(CONFIG_FCOE)
	/* setup tx offload for FCoE */
	if ((protocol == htons(ETH_P_FCOE)) &&
	    (tx_ring->netdev->features & (NETIF_F_FSO | NETIF_F_FCOE_CRC))) {
		tso = ixgbe_fso(tx_ring, first, &hdr_len);
		if (tso < 0)
			goto out_drop;

		goto xmit_fcoe;
	} else if (protocol ==  htons(ETH_P_FIP)) {
		/* FCoE stack has a bug where it does not set the network
		 * header offset for FIP frames sent resulting into MACLEN
		 * being set to ZERO in the Tx context descriptor.
		 * This will cause MDD events when trying to Tx such frames.
		 */
		if (!skb_network_offset(skb)) {
			if (tx_flags & (IXGBE_TX_FLAGS_HW_VLAN |
			    IXGBE_TX_FLAGS_SW_VLAN))
				skb_set_network_header(skb,
						sizeof(struct ethhdr) +
						sizeof(struct vlan_hdr));
			else
				skb_set_network_header(skb,
						sizeof(struct ethhdr));
		}
	}
#endif /* CONFIG_FCOE */

	tso = ixgbe_tso(tx_ring, first, &hdr_len);
	if (tso < 0)
		goto out_drop;
	else if (!tso)
		ixgbe_tx_csum(tx_ring, first);

	/* add the ATR filter if ATR is on */
	if (test_bit(__IXGBE_TX_FDIR_INIT_DONE, &tx_ring->state))
		ixgbe_atr(tx_ring, first);

#if IS_ENABLED(CONFIG_FCOE)
xmit_fcoe:
#endif /* CONFIG_FCOE */
#ifdef HAVE_PTP_1588_CLOCK
	if (ixgbe_tx_map(tx_ring, first, hdr_len))
		goto cleanup_tx_tstamp;
#else
	ixgbe_tx_map(tx_ring, first, hdr_len);
#endif

	return NETDEV_TX_OK;

out_drop:
	dev_kfree_skb_any(first->skb);
	first->skb = NULL;
#ifdef HAVE_PTP_1588_CLOCK
cleanup_tx_tstamp:
	if (unlikely(tx_flags & IXGBE_TX_FLAGS_TSTAMP)) {
		dev_kfree_skb_any(adapter->ptp_tx_skb);
		adapter->ptp_tx_skb = NULL;
		cancel_work_sync(&adapter->ptp_tx_work);
		clear_bit_unlock(__IXGBE_PTP_TX_IN_PROGRESS, &adapter->state);
	}
#endif

	return NETDEV_TX_OK;
}

static netdev_tx_t __ixgbe_xmit_frame(struct sk_buff *skb,
				      struct net_device *netdev,
				      __always_unused struct ixgbe_ring *ring)
{
	struct ixgbe_adapter *adapter = netdev_priv(netdev);
	struct ixgbe_ring *tx_ring;
#ifdef HAVE_TX_MQ
	unsigned int r_idx = skb->queue_mapping;
#endif

	if (!netif_carrier_ok(netdev)) {
		dev_kfree_skb_any(skb);
		return NETDEV_TX_OK;
	}

	/*
	 * The minimum packet size for olinfo paylen is 17 so pad the skb
	 * in order to meet this minimum size requirement.
	 */
	if (skb_put_padto(skb, 17))
		return NETDEV_TX_OK;

#ifdef HAVE_TX_MQ
	if (r_idx >= adapter->num_tx_queues)
		r_idx = r_idx % adapter->num_tx_queues;
	tx_ring = adapter->tx_ring[r_idx];
#else
	tx_ring = adapter->tx_ring[0];
#endif

#ifdef HAVE_AF_XDP_ZC_SUPPORT
	if (unlikely(test_bit(__IXGBE_TX_DISABLED, &tx_ring->state)))
		return NETDEV_TX_BUSY;
#endif

	return ixgbe_xmit_frame_ring(skb, adapter, tx_ring);
}

static netdev_tx_t ixgbe_xmit_frame(struct sk_buff *skb,
				    struct net_device *netdev)
{
	return __ixgbe_xmit_frame(skb, netdev, NULL);
}

/**
 * ixgbe_set_mac - Change the Ethernet Address of the NIC
 * @netdev: network interface device structure
 * @p: pointer to an address structure
 *
 * Returns 0 on success, negative on failure
 **/
static int ixgbe_set_mac(struct net_device *netdev, void *p)
{
	struct ixgbe_adapter *adapter = netdev_priv(netdev);
	struct ixgbe_hw *hw = &adapter->hw;
	struct sockaddr *addr = p;

	if (!is_valid_ether_addr(addr->sa_data))
		return -EADDRNOTAVAIL;

	memcpy(netdev->dev_addr, addr->sa_data, netdev->addr_len);
	memcpy(hw->mac.addr, addr->sa_data, netdev->addr_len);

	ixgbe_mac_set_default_filter(adapter);

	return 0;
}

#if defined(HAVE_NETDEV_STORAGE_ADDRESS) && defined(NETDEV_HW_ADDR_T_SAN)
/**
 * ixgbe_add_sanmac_netdev - Add the SAN MAC address to the corresponding
 * dev->dev_addr_list
 * @dev: network interface device structure
 *
 * Returns non-zero on failure
 **/
static int ixgbe_add_sanmac_netdev(struct net_device *dev)
{
	int err = IXGBE_SUCCESS;
	struct ixgbe_adapter *adapter = netdev_priv(dev);
	struct ixgbe_hw *hw = &adapter->hw;

	if (is_valid_ether_addr(hw->mac.san_addr)) {
		rtnl_lock();
		err = dev_addr_add(dev, hw->mac.san_addr,
				   NETDEV_HW_ADDR_T_SAN);
		rtnl_unlock();

		/* update SAN MAC vmdq pool selection */
		hw->mac.ops.set_vmdq_san_mac(hw, VMDQ_P(0));
	}
	return err;
}

/**
 * ixgbe_del_sanmac_netdev - Removes the SAN MAC address to the corresponding
 * netdev->dev_addr_list
 * @dev: network interface device structure
 *
 * Returns non-zero on failure
 **/
static int ixgbe_del_sanmac_netdev(struct net_device *dev)
{
	int err = IXGBE_SUCCESS;
	struct ixgbe_adapter *adapter = netdev_priv(dev);
	struct ixgbe_mac_info *mac = &adapter->hw.mac;

	if (is_valid_ether_addr(mac->san_addr)) {
		rtnl_lock();
		err = dev_addr_del(dev, mac->san_addr, NETDEV_HW_ADDR_T_SAN);
		rtnl_unlock();
	}
	return err;
}

#endif /* (HAVE_NETDEV_STORAGE_ADDRESS) && defined(NETDEV_HW_ADDR_T_SAN) */

static int ixgbe_mdio_read(struct net_device *netdev, int prtad, int devad,
			   u16 addr)
{
	struct ixgbe_adapter *adapter = netdev_priv(netdev);
	struct ixgbe_hw *hw = &adapter->hw;
	u16 value;
	int rc;

	if (prtad != hw->phy.addr)
		return -EINVAL;

	if (!hw->phy.ops.read_reg)
		return -EOPNOTSUPP;

	rc = hw->phy.ops.read_reg(hw, addr, devad, &value);
	if (!rc)
		rc = value;

	return rc;
}

static int ixgbe_mdio_write(struct net_device *netdev, int prtad, int devad,
			    u16 addr, u16 value)
{
	struct ixgbe_adapter *adapter = netdev_priv(netdev);
	struct ixgbe_hw *hw = &adapter->hw;

	if (prtad != hw->phy.addr)
		return -EINVAL;

	if (!hw->phy.ops.write_reg)
		return -EOPNOTSUPP;

	return hw->phy.ops.write_reg(hw, addr, devad, value);
}

static int ixgbe_mii_ioctl(struct net_device *netdev, struct ifreq *ifr,
			   int cmd)
{
	struct mii_ioctl_data *mii = (struct mii_ioctl_data *) &ifr->ifr_data;
	int prtad, devad, ret;

	prtad = (mii->phy_id & MDIO_PHY_ID_PRTAD) >> 5;
	devad = (mii->phy_id & MDIO_PHY_ID_DEVAD);

	if (cmd == SIOCGMIIREG) {
		ret = ixgbe_mdio_read(netdev, prtad, devad, mii->reg_num);
		if (ret < 0)
			return ret;
		mii->val_out = ret;
		return 0;
	} else {
		return ixgbe_mdio_write(netdev, prtad, devad, mii->reg_num,
					mii->val_in);
	}
}

static int ixgbe_ioctl(struct net_device *netdev, struct ifreq *ifr, int cmd)
{
#ifdef HAVE_PTP_1588_CLOCK
	struct ixgbe_adapter *adapter = netdev_priv(netdev);

#endif
	switch (cmd) {
#ifdef HAVE_PTP_1588_CLOCK
#ifdef SIOCGHWTSTAMP
	case SIOCGHWTSTAMP:
		return ixgbe_ptp_get_ts_config(adapter, ifr);
#endif
	case SIOCSHWTSTAMP:
		return ixgbe_ptp_set_ts_config(adapter, ifr);
#endif
#ifdef ETHTOOL_OPS_COMPAT
	case SIOCETHTOOL:
		return ethtool_ioctl(ifr);
#endif
	case SIOCGMIIREG:
	case SIOCSMIIREG:
		return ixgbe_mii_ioctl(netdev, ifr, cmd);
	default:
		return -EOPNOTSUPP;
	}
}

#ifdef CONFIG_NET_POLL_CONTROLLER
/*
 * Polling 'interrupt' - used by things like netconsole to send skbs
 * without having to re-enable interrupts. It's not called while
 * the interrupt routine is executing.
 */
static void ixgbe_netpoll(struct net_device *netdev)
{
	struct ixgbe_adapter *adapter = netdev_priv(netdev);

	/* if interface is down do nothing */
	if (test_bit(__IXGBE_DOWN, &adapter->state))
		return;

	if (adapter->flags & IXGBE_FLAG_MSIX_ENABLED) {
		int i;
		for (i = 0; i < adapter->num_q_vectors; i++) {
			adapter->q_vector[i]->netpoll_rx = true;
			ixgbe_msix_clean_rings(0, adapter->q_vector[i]);
			adapter->q_vector[i]->netpoll_rx = false;
		}
	} else {
		ixgbe_intr(0, adapter);
	}
}
#endif /* CONFIG_NET_POLL_CONTROLLER */

/* ixgbe_validate_rtr - verify 802.1Qp to Rx packet buffer mapping is valid.
 * @adapter: pointer to ixgbe_adapter
 * @tc: number of traffic classes currently enabled
 *
 * Configure a valid 802.1Qp to Rx packet buffer mapping ie confirm
 * 802.1Q priority maps to a packet buffer that exists.
 */
static void ixgbe_validate_rtr(struct ixgbe_adapter *adapter, u8 tc)
{
	struct ixgbe_hw *hw = &adapter->hw;
	u32 reg, rsave;
	int i;

	/* 82598 have a static priority to TC mapping that can not
	 * be changed so no validation is needed.
	 */
	if (hw->mac.type == ixgbe_mac_82598EB)
		return;

	reg = IXGBE_READ_REG(hw, IXGBE_RTRUP2TC);
	rsave = reg;

	for (i = 0; i < IXGBE_DCB_MAX_TRAFFIC_CLASS; i++) {
		u8 up2tc = reg >> (i * IXGBE_RTRUP2TC_UP_SHIFT);

		/* If up2tc is out of bounds default to zero */
		if (up2tc > tc)
			reg &= ~(0x7 << IXGBE_RTRUP2TC_UP_SHIFT);
	}

	if (reg != rsave)
		IXGBE_WRITE_REG(hw, IXGBE_RTRUP2TC, reg);

	return;
}

/**
 * ixgbe_set_prio_tc_map - Configure netdev prio tc map
 * @adapter: Pointer to adapter struct
 *
 * Populate the netdev user priority to tc map
 */
static void ixgbe_set_prio_tc_map(struct ixgbe_adapter __maybe_unused *adapter)
{
#ifdef HAVE_DCBNL_IEEE
	struct net_device *dev = adapter->netdev;
	struct ixgbe_dcb_config *dcb_cfg = &adapter->dcb_cfg;
	struct ieee_ets *ets = adapter->ixgbe_ieee_ets;
	u8 prio;

	for (prio = 0; prio < IXGBE_DCB_MAX_USER_PRIORITY; prio++) {
		u8 tc = 0;

		if (adapter->dcbx_cap & DCB_CAP_DCBX_VER_CEE)
			tc = ixgbe_dcb_get_tc_from_up(dcb_cfg, 0, prio);
		else if (ets)
			tc = ets->prio_tc[prio];

		netdev_set_prio_tc_map(dev, prio, tc);
	}
#endif
}

#ifdef NETIF_F_HW_TC
#ifdef HAVE_TC_SETUP_CLSU32
static int ixgbe_delete_clsu32(struct ixgbe_adapter *adapter,
			       struct tc_cls_u32_offload *cls)
{
	u32 hdl = cls->knode.handle;
	u32 uhtid = TC_U32_USERHTID(cls->knode.handle);
	u32 loc = cls->knode.handle & 0xfffff;
	int err = 0, i, j;
	struct ixgbe_jump_table *jump = NULL;

	if (loc > IXGBE_MAX_HW_ENTRIES)
		return -EINVAL;

	if ((uhtid != 0x800) && (uhtid >= IXGBE_MAX_LINK_HANDLE))
		return -EINVAL;

	/* Clear this filter in the link data it is associated with */
	if (uhtid != 0x800) {
		jump = adapter->jump_tables[uhtid];
		if (!jump)
			return -EINVAL;
		if (!test_bit(loc - 1, jump->child_loc_map))
			return -EINVAL;
		clear_bit(loc - 1, jump->child_loc_map);
	}

	/* Check if the filter being deleted is a link */
	for (i = 1; i < IXGBE_MAX_LINK_HANDLE; i++) {
		jump = adapter->jump_tables[i];
		if (jump && jump->link_hdl == hdl) {
			/* Delete filters in the hardware in the child hash
			 * table associated with this link
			 */
			for (j = 0; j < IXGBE_MAX_HW_ENTRIES; j++) {
				if (!test_bit(j, jump->child_loc_map))
					continue;
				spin_lock(&adapter->fdir_perfect_lock);
				err = ixgbe_update_ethtool_fdir_entry(adapter,
								      NULL,
								      j + 1);
				spin_unlock(&adapter->fdir_perfect_lock);
				clear_bit(j, jump->child_loc_map);
			}
			/* Remove resources for this link */
			kfree(jump->input);
			kfree(jump->mask);
			kfree(jump);
			adapter->jump_tables[i] = NULL;
			return err;
		}
	}

	spin_lock(&adapter->fdir_perfect_lock);
	err = ixgbe_update_ethtool_fdir_entry(adapter, NULL, loc);
	spin_unlock(&adapter->fdir_perfect_lock);
	return err;
}

#ifdef HAVE_NDO_SETUP_TC_REMOVE_TC_TO_NETDEV
static int ixgbe_configure_clsu32_add_hnode(struct ixgbe_adapter *adapter,
					    struct tc_cls_u32_offload *cls)
{
#else
static int ixgbe_configure_clsu32_add_hnode(struct ixgbe_adapter *adapter,
					    __be16 protocol,
					    struct tc_cls_u32_offload *cls)
{
#endif
	u32 uhtid = TC_U32_USERHTID(cls->hnode.handle);

	if (uhtid >= IXGBE_MAX_LINK_HANDLE)
		return -EINVAL;

	/* This ixgbe devices do not support hash tables at the moment
	 * so abort when given hash tables.
	 */
	if (cls->hnode.divisor > 0)
		return -EINVAL;

	set_bit(uhtid - 1, &adapter->tables);
	return 0;
}

static int ixgbe_configure_clsu32_del_hnode(struct ixgbe_adapter *adapter,
					    struct tc_cls_u32_offload *cls)
{
	u32 uhtid = TC_U32_USERHTID(cls->hnode.handle);

	if (uhtid >= IXGBE_MAX_LINK_HANDLE)
		return -EINVAL;

	clear_bit(uhtid - 1, &adapter->tables);
	return 0;
}

#ifdef CONFIG_NET_CLS_ACT
#ifdef HAVE_TCF_MIRRED_REDIRECT
static int handle_redirect_action(struct ixgbe_adapter *adapter, int ifindex,
				  u8 *queue, u64 *action)
{
	unsigned int num_vfs = adapter->num_vfs, vf;
	struct net_device *upper;

	/* redirect to a SRIOV VF */
	for (vf = 0; vf < num_vfs; ++vf) {
		upper = pci_get_drvdata(adapter->vfinfo[vf].vfdev);
		if (upper->ifindex == ifindex) {
			if (adapter->num_rx_pools > 1)
				*queue = vf * 2;
			else
				*queue = vf * adapter->num_rx_queues_per_pool;

			*action = vf + 1;
			*action <<= ETHTOOL_RX_FLOW_SPEC_RING_VF_OFF;
			return 0;
		}
	}
	return -EINVAL;
}
#endif

static int parse_tc_actions(struct ixgbe_adapter *adapter,
			    struct tcf_exts *exts, u64 *action, u8 *queue)
{
	const struct tc_action *a;
#if defined(HAVE_TCF_EXTS_TO_LIST)
	LIST_HEAD(actions);
#elif defined(HAVE_TCF_EXTS_FOR_EACH_ACTION)
	int i;
#endif

#ifdef HAVE_TCF_EXTS_HAS_ACTION
	if (!tcf_exts_has_actions(exts))
#else
	if (tc_no_actions(exts))
#endif
		return -EINVAL;

#if defined(HAVE_TCF_EXTS_TO_LIST)
	tcf_exts_to_list(exts, &actions);
	list_for_each_entry(a, &actions, list) {
#elif defined(HAVE_TCF_EXTS_FOR_EACH_ACTION)
	tcf_exts_for_each_action(i, a, exts) {
#else
	tc_for_each_action(a, exts) {
#endif
		/* Drop action */
		if (is_tcf_gact_shot(a)) {
			*action = IXGBE_FDIR_DROP_QUEUE;
			*queue = IXGBE_FDIR_DROP_QUEUE;
			return 0;
		}

#ifdef HAVE_TCF_MIRRED_REDIRECT
		/* Redirect to a VF or a offloaded macvlan */
#ifdef HAVE_TCF_MIRRED_EGRESS_REDIRECT
		if (is_tcf_mirred_egress_redirect(a)) {
#else
		if (is_tcf_mirred_redirect(a)) {
#endif
#ifdef HAVE_TCF_MIRRED_DEV
			struct net_device *dev = tcf_mirred_dev(a);

			if (!dev)
				return -EINVAL;
			return handle_redirect_action(adapter, dev->ifindex,
						      queue, action);
#else
			int ifindex = tcf_mirred_ifindex(a);

			return handle_redirect_action(adapter, ifindex,
						      queue, action);
#endif
		}
#endif
		return -EINVAL;
	}

	return -EINVAL;
}
#else
static int parse_tc_actions(struct ixgbe_adapter *adapter,
			    struct tcf_exts *exts, u64 *action, u8 *queue)
{
	return -EINVAL;
}
#endif /* CONFIG_NET_CLS_ACT */

static int ixgbe_clsu32_build_input(struct ixgbe_fdir_filter *input,
				    union ixgbe_atr_input *mask,
				    struct tc_cls_u32_offload *cls,
				    struct ixgbe_mat_field *field_ptr,
				    struct ixgbe_nexthdr *nexthdr)
{
	int i, j, off;
	__be32 val, m;
	bool found_entry = false, found_jump_field = false;

	for (i = 0; i < cls->knode.sel->nkeys; i++) {
		off = cls->knode.sel->keys[i].off;
		val = cls->knode.sel->keys[i].val;
		m = cls->knode.sel->keys[i].mask;

		for (j = 0; field_ptr[j].val; j++) {
			if (field_ptr[j].off == off) {
				field_ptr[j].val(input, mask, val, m);
				input->filter.formatted.flow_type |=
					field_ptr[j].type;
				found_entry = true;
				break;
			}
		}
		if (nexthdr) {
			if (nexthdr->off == cls->knode.sel->keys[i].off &&
			    nexthdr->val == cls->knode.sel->keys[i].val &&
			    nexthdr->mask == cls->knode.sel->keys[i].mask)
				found_jump_field = true;
			else
				continue;
		}
	}

	if (nexthdr && !found_jump_field)
		return -EINVAL;

	if (!found_entry)
		return 0;

	mask->formatted.flow_type = IXGBE_ATR_L4TYPE_IPV6_MASK |
				    IXGBE_ATR_L4TYPE_MASK;

	if (input->filter.formatted.flow_type == IXGBE_ATR_FLOW_TYPE_IPV4)
		mask->formatted.flow_type &= IXGBE_ATR_L4TYPE_IPV6_MASK;

	return 0;
}

#ifdef HAVE_NDO_SETUP_TC_REMOVE_TC_TO_NETDEV
static int ixgbe_configure_clsu32(struct ixgbe_adapter *adapter,
				  struct tc_cls_u32_offload *cls)
{
	__be16 protocol = cls->common.protocol;
#else
static int ixgbe_configure_clsu32(struct ixgbe_adapter *adapter,
				  __be16 protocol,
				  struct tc_cls_u32_offload *cls)
{
#endif
	u32 loc = cls->knode.handle & 0xfffff;
	struct ixgbe_hw *hw = &adapter->hw;
	struct ixgbe_mat_field *field_ptr;
	struct ixgbe_fdir_filter *input = NULL;
	union ixgbe_atr_input *mask = NULL;
	struct ixgbe_jump_table *jump = NULL;
	int i, err = -EINVAL;
	u8 queue;
	u32 uhtid, link_uhtid;

	uhtid = TC_U32_USERHTID(cls->knode.handle);
	link_uhtid = TC_U32_USERHTID(cls->knode.link_handle);

	/* At the moment cls_u32 jumps to network layer and skips past
	 * L2 headers. The canonical method to match L2 frames is to use
	 * negative values. However this is error prone at best but really
	 * just broken because there is no way to "know" what sort of hdr
	 * is in front of the network layer. Fix cls_u32 to support L2
	 * headers when needed.
	 */
	if (protocol != htons(ETH_P_IP))
		return err;

	if (loc >= ((1024 << adapter->fdir_pballoc) - 2)) {
		e_err(drv, "Location out of range\n");
		return err;
	}

	/* cls u32 is a graph starting at root node 0x800. The driver tracks
	 * links and also the fields used to advance the parser across each
	 * link (e.g. nexthdr/eat parameters from 'tc'). This way we can map
	 * the u32 graph onto the hardware parse graph denoted in ixgbe_model.h
	 * To add support for new nodes update ixgbe_model.h parse structures
	 * this function _should_ be generic try not to hardcode values here.
	 */
	if (uhtid == 0x800) {
		field_ptr = (adapter->jump_tables[0])->mat;
	} else {
		if (uhtid >= IXGBE_MAX_LINK_HANDLE)
			return err;
		if (!adapter->jump_tables[uhtid])
			return err;
		field_ptr = (adapter->jump_tables[uhtid])->mat;
	}

	if (!field_ptr)
		return err;

	/* At this point we know the field_ptr is valid and need to either
	 * build cls_u32 link or attach filter. Because adding a link to
	 * a handle that does not exist is invalid and the same for adding
	 * rules to handles that don't exist.
	 */

	if (link_uhtid) {
		struct ixgbe_nexthdr *nexthdr = ixgbe_ipv4_jumps;

		if (link_uhtid >= IXGBE_MAX_LINK_HANDLE)
			return err;

		if (!test_bit(link_uhtid - 1, &adapter->tables))
			return err;

		/* Multiple filters as links to the same hash table are not
		 * supported. To add a new filter with the same next header
		 * but different match/jump conditions, create a new hash table
		 * and link to it.
		 */
		if (adapter->jump_tables[link_uhtid] &&
		    (adapter->jump_tables[link_uhtid])->link_hdl) {
			e_err(drv, "Link filter exists for link: %x\n",
			      link_uhtid);
			return err;
		}

		for (i = 0; nexthdr[i].jump; i++) {
			if (nexthdr[i].o != cls->knode.sel->offoff ||
			    nexthdr[i].s != cls->knode.sel->offshift ||
			    nexthdr[i].m != cls->knode.sel->offmask)
				return err;

			jump = kzalloc(sizeof(*jump), GFP_KERNEL);
			if (!jump)
				return -ENOMEM;
			input = kzalloc(sizeof(*input), GFP_KERNEL);
			if (!input) {
				err = -ENOMEM;
				goto free_jump;
			}
			mask = kzalloc(sizeof(*mask), GFP_KERNEL);
			if (!mask) {
				err = -ENOMEM;
				goto free_input;
			}
			jump->input = input;
			jump->mask = mask;
			jump->link_hdl = cls->knode.handle;

			err = ixgbe_clsu32_build_input(input, mask, cls,
						       field_ptr, &nexthdr[i]);
			if (!err) {
				jump->mat = nexthdr[i].jump;
				adapter->jump_tables[link_uhtid] = jump;
				break;
			}
		}

		return 0;
	}

	input = kzalloc(sizeof(*input), GFP_KERNEL);
	if (!input)
		return -ENOMEM;
	mask = kzalloc(sizeof(*mask), GFP_KERNEL);
	if (!mask) {
		err = -ENOMEM;
		goto free_input;
	}

	if ((uhtid != 0x800) && (adapter->jump_tables[uhtid])) {
		if ((adapter->jump_tables[uhtid])->input)
			memcpy(input, (adapter->jump_tables[uhtid])->input,
			       sizeof(*input));
		if ((adapter->jump_tables[uhtid])->mask)
			memcpy(mask, (adapter->jump_tables[uhtid])->mask,
			       sizeof(*mask));

		/* Lookup in all child hash tables if this location is already
		 * filled with a filter
		 */
		for (i = 1; i < IXGBE_MAX_LINK_HANDLE; i++) {
			struct ixgbe_jump_table *link = adapter->jump_tables[i];

			if (link && (test_bit(loc - 1, link->child_loc_map))) {
				e_err(drv, "Filter exists in location: %x\n",
				      loc);
				err = -EINVAL;
				goto err_out;
			}
		}
	}
	err = ixgbe_clsu32_build_input(input, mask, cls, field_ptr, NULL);
	if (err)
		goto err_out;

	err = parse_tc_actions(adapter, cls->knode.exts, &input->action,
			       &queue);
	if (err < 0)
		goto err_out;

	input->sw_idx = loc;

	spin_lock(&adapter->fdir_perfect_lock);

	if (hlist_empty(&adapter->fdir_filter_list)) {
		memcpy(&adapter->fdir_mask, mask, sizeof(*mask));
		err = ixgbe_fdir_set_input_mask_82599(hw, mask,
						      adapter->cloud_mode);
		if (err)
			goto err_out_w_lock;
	} else if (memcmp(&adapter->fdir_mask, mask, sizeof(*mask))) {
		err = -EINVAL;
		goto err_out_w_lock;
	}

	ixgbe_atr_compute_perfect_hash_82599(&input->filter, mask);
	err = ixgbe_fdir_write_perfect_filter_82599(hw, &input->filter,
						    input->sw_idx, queue,
						    adapter->cloud_mode);
	if (!err)
		ixgbe_update_ethtool_fdir_entry(adapter, input, input->sw_idx);
	spin_unlock(&adapter->fdir_perfect_lock);

	if ((uhtid != 0x800) && (adapter->jump_tables[uhtid]))
		set_bit(loc - 1, (adapter->jump_tables[uhtid])->child_loc_map);

	kfree(mask);
	return err;
err_out_w_lock:
	spin_unlock(&adapter->fdir_perfect_lock);
err_out:
	kfree(mask);
free_input:
	kfree(input);
free_jump:
	kfree(jump);
	return err;
}

#ifdef HAVE_NDO_SETUP_TC_REMOVE_TC_TO_NETDEV
#ifdef HAVE_TCF_BLOCK
static int ixgbe_setup_tc_cls_u32(struct ixgbe_adapter *adapter,
				  struct tc_cls_u32_offload *cls_u32)
#else
static int ixgbe_setup_tc_cls_u32(struct net_device *dev,
				  struct tc_cls_u32_offload *cls_u32)
#endif /* HAVE_TCF_BLOCK */
{
#ifdef HAVE_TCF_BLOCK
#ifndef HAVE_TCF_MIRRED_DEV
	if (cls_u32->common.chain_index)
		return -EOPNOTSUPP;
#endif
#else
	struct ixgbe_adapter *adapter = netdev_priv(dev);

	if (!is_classid_clsact_ingress(cls_u32->common.classid) ||
	    cls_u32->common.chain_index)
		return -EOPNOTSUPP;
#endif

	switch (cls_u32->command) {
	case TC_CLSU32_NEW_KNODE:
	case TC_CLSU32_REPLACE_KNODE:
		return ixgbe_configure_clsu32(adapter, cls_u32);
	case TC_CLSU32_DELETE_KNODE:
		return ixgbe_delete_clsu32(adapter, cls_u32);
	case TC_CLSU32_NEW_HNODE:
	case TC_CLSU32_REPLACE_HNODE:
		return ixgbe_configure_clsu32_add_hnode(adapter, cls_u32);
	case TC_CLSU32_DELETE_HNODE:
		return ixgbe_configure_clsu32_del_hnode(adapter, cls_u32);
	default:
		return -EOPNOTSUPP;

	}
}
#endif /* HAVE_NDO_SETUP_TC_REMOVE_TC_TO_NETDEV */
#endif /* HAVE_TC_SETUP_CLSU32 */

#ifdef HAVE_TCF_BLOCK
static int ixgbe_setup_tc_block_cb(enum tc_setup_type type, void *type_data,
				   void *cb_priv)
{
	struct ixgbe_adapter *adapter = cb_priv;

#ifdef HAVE_TCF_MIRRED_DEV
	if (!tc_cls_can_offload_and_chain0(adapter->netdev, type_data))
#else
	if (!tc_can_offload(adapter->netdev))
#endif
		return -EOPNOTSUPP;

	switch (type) {
#ifdef HAVE_TC_SETUP_CLSU32
	case TC_SETUP_CLSU32:
		return ixgbe_setup_tc_cls_u32(adapter, type_data);
#endif /* HAVE_TC_SETUP_CLSU32 */
	default:
		return -EOPNOTSUPP;
	}
}

static LIST_HEAD(ixgbe_block_cb_list);

#endif
#ifdef TC_MQPRIO_HW_OFFLOAD_MAX
static int ixgbe_setup_tc_mqprio(struct net_device *dev,
				 struct tc_mqprio_qopt *mqprio)
{
	mqprio->hw = TC_MQPRIO_HW_OFFLOAD_TCS;
	return ixgbe_setup_tc(dev, mqprio->num_tc);
}
#endif

#ifdef HAVE_TCF_BLOCK
#define IXGBE_SETUP_TC_DECLARE_ADAPTER
#endif /* HAVE_TCF_BLOCK */

#ifdef HAVE_TC_SETUP_CLSU32
#ifndef HAVE_NDO_SETUP_TC_REMOVE_TC_TO_NETDEV
#define IXGBE_SETUP_TC_DECLARE_ADAPTER
#endif /* !HAVE_NDO_SETUP_TC_REMOVE_TC_TO_NETDEV */
#endif /* HAVE_TC_SETUP_CLSU32 */

#if defined(HAVE_NDO_SETUP_TC_REMOVE_TC_TO_NETDEV)
static int
__ixgbe_setup_tc(struct net_device *dev, enum tc_setup_type type,
		 void *type_data)
#elif defined(HAVE_NDO_SETUP_TC_CHAIN_INDEX)
static int
__ixgbe_setup_tc(struct net_device *dev, __always_unused u32 handle,
		 u32 chain_index, __always_unused __be16 proto,
		 struct tc_to_netdev *tc)
#else
static int
__ixgbe_setup_tc(struct net_device *dev, __always_unused u32 handle,
		 __always_unused __be16 proto, struct tc_to_netdev *tc)
#endif
{
#ifdef IXGBE_SETUP_TC_DECLARE_ADAPTER
	struct ixgbe_adapter *adapter = netdev_priv(dev);
#endif
#ifndef HAVE_NDO_SETUP_TC_REMOVE_TC_TO_NETDEV
	unsigned int type = tc->type;
#ifdef HAVE_NDO_SETUP_TC_CHAIN_INDEX

	if (chain_index)
		return -EOPNOTSUPP;
#endif /* HAVE_NDO_SETUP_TC_CHAIN_INDEX */
#ifdef HAVE_TC_SETUP_CLSU32

	if (TC_H_MAJ(handle) == TC_H_MAJ(TC_H_INGRESS) &&
	    type == TC_SETUP_CLSU32) {

		switch (tc->cls_u32->command) {
		case TC_CLSU32_NEW_KNODE:
		case TC_CLSU32_REPLACE_KNODE:
			return ixgbe_configure_clsu32(adapter,
							  proto, tc->cls_u32);
		case TC_CLSU32_DELETE_KNODE:
			return ixgbe_delete_clsu32(adapter, tc->cls_u32);
		case TC_CLSU32_NEW_HNODE:
		case TC_CLSU32_REPLACE_HNODE:
			return ixgbe_configure_clsu32_add_hnode(adapter, proto,
								tc->cls_u32);
		case TC_CLSU32_DELETE_HNODE:
			return ixgbe_configure_clsu32_del_hnode(adapter,
								tc->cls_u32);
		default:
			return -EINVAL;
		}
	}
#endif /* HAVE_TC_SETUP_CLSU32 */
#endif /* !HAVE_NDO_SETUP_TC_REMOVE_TC_TO_NETDEV */

	switch (type) {
#if defined(HAVE_NDO_SETUP_TC_REMOVE_TC_TO_NETDEV)
#if defined(HAVE_TCF_BLOCK)
	case TC_SETUP_BLOCK:
		return flow_block_cb_setup_simple(type_data,
						  &ixgbe_block_cb_list,
						  ixgbe_setup_tc_block_cb,
						  adapter, adapter, true);
#else
#ifdef HAVE_TC_SETUP_CLSU32
	case TC_SETUP_CLSU32:
		return ixgbe_setup_tc_cls_u32(dev, type_data);
#endif /* HAVE_TC_SETUP_CLSU32 */
#endif /* HAVE_TCF_BLOCK */
#endif /* HAVE_NDO_SETUP_TC_REMOVE_TC_TO_NETDEV */
	case TC_SETUP_QDISC_MQPRIO:
#if defined(HAVE_NDO_SETUP_TC_REMOVE_TC_TO_NETDEV)
		return ixgbe_setup_tc_mqprio(dev, type_data);
#elif defined(TC_MQPRIO_HW_OFFLOAD_MAX)
		return ixgbe_setup_tc_mqprio(dev, tc->mqprio);
#else
		return ixgbe_setup_tc(dev, tc->tc);
#endif
	default:
		return -EOPNOTSUPP;
	}
}

#endif /* NETIF_F_HW_TC */
/**
 * ixgbe_setup_tc - routine to configure net_device for multiple traffic
 * classes.
 *
 * @dev: net device to configure
 * @tc: number of traffic classes to enable
 */
int ixgbe_setup_tc(struct net_device *dev, u8 tc)
{
	struct ixgbe_adapter *adapter = netdev_priv(dev);
	struct ixgbe_hw *hw = &adapter->hw;

	/* Hardware supports up to 8 traffic classes */
	if (tc > adapter->dcb_cfg.num_tcs.pg_tcs)
		return -EINVAL;

	if (tc && hw->mac.type == ixgbe_mac_82598EB &&
	    tc < IXGBE_DCB_MAX_TRAFFIC_CLASS)
		return -EINVAL;

	/* Hardware has to reinitialize queues and interrupts to
	 * match packet buffer alignment. Unfortunately, the
	 * hardware is not flexible enough to do this dynamically.
	 */
	if (netif_running(dev))
		ixgbe_close(dev);
	else
		ixgbe_reset(adapter);

	ixgbe_clear_interrupt_scheme(adapter);

	if (tc) {
#ifdef HAVE_XDP_SUPPORT
		if (adapter->xdp_prog) {
			e_warn(probe, "DCB is not supported  with XDP\n");

			ixgbe_init_interrupt_scheme(adapter);
			if (netif_running(dev))
				ixgbe_open(dev);
			return -EINVAL;
		}
#endif /* IXGBE_XDP_ENABLED */

		netdev_set_num_tc(dev, tc);
		ixgbe_set_prio_tc_map(adapter);

		adapter->flags |= IXGBE_FLAG_DCB_ENABLED;

		if (adapter->hw.mac.type == ixgbe_mac_82598EB) {
			adapter->last_lfc_mode = adapter->hw.fc.requested_mode;
			adapter->hw.fc.requested_mode = ixgbe_fc_none;
		}
	} else {
		netdev_reset_tc(dev);

		if (adapter->hw.mac.type == ixgbe_mac_82598EB)
			adapter->hw.fc.requested_mode = adapter->last_lfc_mode;

		adapter->flags &= ~IXGBE_FLAG_DCB_ENABLED;

		adapter->temp_dcb_cfg.pfc_mode_enable = false;
		adapter->dcb_cfg.pfc_mode_enable = false;
	}

	ixgbe_validate_rtr(adapter, tc);

	ixgbe_init_interrupt_scheme(adapter);
	if (netif_running(dev))
		ixgbe_open(dev);

	return 0;
}

#ifdef CONFIG_PCI_IOV
void ixgbe_sriov_reinit(struct ixgbe_adapter *adapter)
{
	struct net_device *netdev = adapter->netdev;

	rtnl_lock();
	ixgbe_setup_tc(netdev, netdev_get_num_tc(netdev));
	rtnl_unlock();
}
#endif

void ixgbe_do_reset(struct net_device *netdev)
{
	struct ixgbe_adapter *adapter = netdev_priv(netdev);

	if (netif_running(netdev))
		ixgbe_reinit_locked(adapter);
	else
		ixgbe_reset(adapter);
}

#ifdef HAVE_NDO_SET_FEATURES
#ifdef HAVE_RHEL6_NET_DEVICE_OPS_EXT
static u32 ixgbe_fix_features(struct net_device *netdev, u32 features)
#else
static netdev_features_t ixgbe_fix_features(struct net_device *netdev,
					    netdev_features_t features)
#endif
{
	struct ixgbe_adapter *adapter = netdev_priv(netdev);

#if IS_ENABLED(CONFIG_DCB)
	if (adapter->flags & IXGBE_FLAG_DCB_ENABLED)
#ifdef NETIF_F_HW_VLAN_CTAG_RX
		features |= NETIF_F_HW_VLAN_CTAG_RX;
#else
		features |= NETIF_F_HW_VLAN_RX;
#endif
#endif /* CONFIG_DCB */

	/* If Rx checksum is disabled, then RSC/LRO should also be disabled */
	if (!(features & NETIF_F_RXCSUM))
		features &= ~NETIF_F_LRO;

	/* Turn off LRO if not RSC capable */
	if (!(adapter->flags2 & IXGBE_FLAG2_RSC_CAPABLE))
		features &= ~NETIF_F_LRO;

	if (adapter->xdp_prog && (features & NETIF_F_LRO)) {
		e_dev_err("LRO is not supported with XDP\n");
		features &= ~NETIF_F_LRO;
	}

	return features;
}

#ifdef HAVE_RHEL6_NET_DEVICE_OPS_EXT
static int ixgbe_set_features(struct net_device *netdev, u32 features)
#else
static int ixgbe_set_features(struct net_device *netdev,
			      netdev_features_t features)
#endif
{
	struct ixgbe_adapter *adapter = netdev_priv(netdev);
	bool need_reset = false;
	netdev_features_t changed = netdev->features ^ features;

	/* Make sure RSC matches LRO, reset if change */
	if (!(features & NETIF_F_LRO)) {
		if (adapter->flags2 & IXGBE_FLAG2_RSC_ENABLED)
			need_reset = true;
		adapter->flags2 &= ~IXGBE_FLAG2_RSC_ENABLED;
	} else if ((adapter->flags2 & IXGBE_FLAG2_RSC_CAPABLE) &&
		   !(adapter->flags2 & IXGBE_FLAG2_RSC_ENABLED)) {
		if (adapter->rx_itr_setting == 1 ||
		    adapter->rx_itr_setting > IXGBE_MIN_RSC_ITR) {
			adapter->flags2 |= IXGBE_FLAG2_RSC_ENABLED;
			need_reset = true;
		} else if (changed & NETIF_F_LRO) {
			e_info(probe, "rx-usecs set too low, "
			       "disabling RSC\n");
		}
	}

	/*
	 * Check if Flow Director n-tuple support or hw_tc support was
	 * enabled or disabled.  If the state changed, we need to reset.
	 */
#ifdef NETIF_F_HW_TC
	if ((features & NETIF_F_NTUPLE) || (features & NETIF_F_HW_TC)) {
#else
	if (features & NETIF_F_NTUPLE) {
#endif
		/* turn off ATR, enable perfect filters and reset */
		if (!(adapter->flags & IXGBE_FLAG_FDIR_PERFECT_CAPABLE))
			need_reset = true;

		adapter->flags &= ~IXGBE_FLAG_FDIR_HASH_CAPABLE;
		adapter->flags |= IXGBE_FLAG_FDIR_PERFECT_CAPABLE;
	} else {
		/* turn off perfect filters, enable ATR and reset */
		if (adapter->flags & IXGBE_FLAG_FDIR_PERFECT_CAPABLE)
			need_reset = true;

		adapter->flags &= ~IXGBE_FLAG_FDIR_PERFECT_CAPABLE;

		/* We cannot enable ATR if VMDq is enabled */
		if (adapter->flags & IXGBE_FLAG_VMDQ_ENABLED ||
		    /* We cannot enable ATR if we have 2 or more tcs */
		    netdev_get_num_tc(netdev) > 1 ||
		    /* We cannot enable ATR if RSS is disabled */
		    adapter->ring_feature[RING_F_RSS].limit <= 1 ||
		    /* A sample rate of 0 indicates ATR disabled */
		    !adapter->atr_sample_rate)
			; /* do nothing not supported */
		else /* otherwise supported and set the flag */
			adapter->flags |= IXGBE_FLAG_FDIR_HASH_CAPABLE;
	}

	netdev->features = features;

#if defined(HAVE_UDP_ENC_RX_OFFLOAD) || defined(HAVE_VXLAN_RX_OFFLOAD)
	if (adapter->flags & IXGBE_FLAG_VXLAN_OFFLOAD_CAPABLE &&
	    features & NETIF_F_RXCSUM) {
		if (!need_reset)
			adapter->flags2 |= IXGBE_FLAG2_UDP_TUN_REREG_NEEDED;
	} else {
		u32 port_mask = IXGBE_VXLANCTRL_VXLAN_UDPPORT_MASK;

		ixgbe_clear_udp_tunnel_port(adapter, port_mask);
	}
#endif /* HAVE_UDP_ENC_RX_OFFLOAD || HAVE_VXLAN_RX_OFFLOAD */

#ifdef HAVE_UDP_ENC_RX_OFFLOAD
	if (adapter->flags & IXGBE_FLAG_GENEVE_OFFLOAD_CAPABLE &&
	    features & NETIF_F_RXCSUM) {
		if (!need_reset)
			adapter->flags2 |= IXGBE_FLAG2_UDP_TUN_REREG_NEEDED;
	} else {
		u32 port_mask = IXGBE_VXLANCTRL_GENEVE_UDPPORT_MASK;

		ixgbe_clear_udp_tunnel_port(adapter, port_mask);
	}
#endif /* HAVE_UDP_ENC_RX_OFFLOAD */

	if (need_reset)
		ixgbe_do_reset(netdev);
#ifdef NETIF_F_HW_VLAN_CTAG_FILTER
	else if (changed & (NETIF_F_HW_VLAN_CTAG_RX |
			    NETIF_F_HW_VLAN_CTAG_FILTER))
		ixgbe_set_rx_mode(netdev);
#endif
#ifdef NETIF_F_HW_VLAN_FILTER
	else if (changed & (NETIF_F_HW_VLAN_RX |
			    NETIF_F_HW_VLAN_FILTER))
		ixgbe_set_rx_mode(netdev);
#endif
	return 0;

}
#endif /* HAVE_NDO_SET_FEATURES */

#ifdef HAVE_UDP_ENC_RX_OFFLOAD
/**
 * ixgbe_add_udp_tunnel_port - Get notifications about adding UDP tunnel ports
 * @dev: The port's netdev
 * @ti: Tunnel endpoint information
 **/
static void ixgbe_add_udp_tunnel_port(struct net_device *dev,
				      struct udp_tunnel_info *ti)
{
	struct ixgbe_adapter *adapter = netdev_priv(dev);
	struct ixgbe_hw *hw = &adapter->hw;
	__be16 port = ti->port;
	u32 port_shift = 0;
	u32 reg;

	if (ti->sa_family != AF_INET)
		return;

	switch (ti->type) {
	case UDP_TUNNEL_TYPE_VXLAN:
		if (!(adapter->flags & IXGBE_FLAG_VXLAN_OFFLOAD_CAPABLE))
			return;

		if (adapter->vxlan_port == port)
			return;

		if (adapter->vxlan_port) {
			netdev_info(dev,
				    "VXLAN port %d set, not adding port %d\n",
				    ntohs(adapter->vxlan_port),
				    ntohs(port));
			return;
		}

		adapter->vxlan_port = port;
		break;
	case UDP_TUNNEL_TYPE_GENEVE:
		if (!(adapter->flags & IXGBE_FLAG_GENEVE_OFFLOAD_CAPABLE))
			return;

		if (adapter->geneve_port == port)
			return;

		if (adapter->geneve_port) {
			netdev_info(dev,
				    "GENEVE port %d set, not adding port %d\n",
				    ntohs(adapter->geneve_port),
				    ntohs(port));
			return;
		}

		port_shift = IXGBE_VXLANCTRL_GENEVE_UDPPORT_SHIFT;
		adapter->geneve_port = port;
		break;
	default:
		return;
	}

	reg = IXGBE_READ_REG(hw, IXGBE_VXLANCTRL) | ntohs(port) << port_shift;
	IXGBE_WRITE_REG(hw, IXGBE_VXLANCTRL, reg);
}

/**
 * ixgbe_del_udp_tunnel_port - Get notifications about removing UDP tunnel ports
 * @dev: The port's netdev
 * @ti: Tunnel endpoint information
 **/
static void ixgbe_del_udp_tunnel_port(struct net_device *dev,
				      struct udp_tunnel_info *ti)
{
	struct ixgbe_adapter *adapter = netdev_priv(dev);
	u32 port_mask;

	if (ti->type != UDP_TUNNEL_TYPE_VXLAN &&
	    ti->type != UDP_TUNNEL_TYPE_GENEVE)
		return;

	if (ti->sa_family != AF_INET)
		return;

	switch (ti->type) {
	case UDP_TUNNEL_TYPE_VXLAN:
		if (!(adapter->flags & IXGBE_FLAG_VXLAN_OFFLOAD_CAPABLE))
			return;

		if (adapter->vxlan_port != ti->port) {
			netdev_info(dev, "VXLAN port %d not found\n",
				    ntohs(ti->port));
			return;
		}

		port_mask = IXGBE_VXLANCTRL_VXLAN_UDPPORT_MASK;
		break;
	case UDP_TUNNEL_TYPE_GENEVE:
		if (!(adapter->flags & IXGBE_FLAG_GENEVE_OFFLOAD_CAPABLE))
			return;

		if (adapter->geneve_port != ti->port) {
			netdev_info(dev, "GENEVE port %d not found\n",
				    ntohs(ti->port));
			return;
		}

		port_mask = IXGBE_VXLANCTRL_GENEVE_UDPPORT_MASK;
		break;
	default:
		return;
	}

	ixgbe_clear_udp_tunnel_port(adapter, port_mask);
	adapter->flags2 |= IXGBE_FLAG2_UDP_TUN_REREG_NEEDED;
}
#elif defined(HAVE_VXLAN_RX_OFFLOAD)
/**
 * ixgbe_add_vxlan_port - Get notifications about VXLAN ports that come up
 * @dev: The port's netdev
 * @sa_family: Socket Family that VXLAN is notifiying us about
 * @port: New UDP port number that VXLAN started listening to
 */
static void ixgbe_add_vxlan_port(struct net_device *dev, sa_family_t sa_family,
				 __be16 port)
{
	struct ixgbe_adapter *adapter = netdev_priv(dev);
	struct ixgbe_hw *hw = &adapter->hw;

	if (sa_family != AF_INET)
		return;

	if (!(adapter->flags & IXGBE_FLAG_VXLAN_OFFLOAD_ENABLE))
		return;

	if (adapter->vxlan_port == port)
		return;

	if (adapter->vxlan_port) {
		netdev_info(dev,
			    "Hit Max num of VXLAN ports, not adding port %d\n",
			    ntohs(port));
		return;
	}

	adapter->vxlan_port = port;
	IXGBE_WRITE_REG(hw, IXGBE_VXLANCTRL, ntohs(port));
}

/**
 * ixgbe_del_vxlan_port - Get notifications about VXLAN ports that go away
 * @dev: The port's netdev
 * @sa_family: Socket Family that VXLAN is notifying us about
 * @port: UDP port number that VXLAN stopped listening to
 */
static void ixgbe_del_vxlan_port(struct net_device *dev, sa_family_t sa_family,
				 __be16 port)
{
	struct ixgbe_adapter *adapter = netdev_priv(dev);

	if (!(adapter->flags & IXGBE_FLAG_VXLAN_OFFLOAD_ENABLE))
		return;

	if (sa_family != AF_INET)
		return;

	if (adapter->vxlan_port != port) {
		netdev_info(dev, "Port %d was not found, not deleting\n",
			    ntohs(port));
		return;
	}

	ixgbe_clear_udp_tunnel_port(adapter, IXGBE_VXLANCTRL_ALL_UDPPORT_MASK);
	adapter->flags2 |= IXGBE_FLAG2_UDP_TUN_REREG_NEEDED;
}
#endif /* HAVE_VXLAN_RX_OFFLOAD */

#ifdef HAVE_NDO_GSO_CHECK
static bool
ixgbe_gso_check(struct sk_buff *skb, __always_unused struct net_device *dev)
{
	return vxlan_gso_check(skb);
}
#endif /* HAVE_NDO_GSO_CHECK */

#ifdef HAVE_FDB_OPS
#ifdef USE_CONST_DEV_UC_CHAR
static int ixgbe_ndo_fdb_add(struct ndmsg *ndm, struct nlattr *tb[],
			     struct net_device *dev,
			     const unsigned char *addr,
#ifdef HAVE_NDO_FDB_ADD_VID
			     u16 vid,
#endif
#ifdef HAVE_NDO_FDB_ADD_EXTACK
			     u16 flags,
			     struct netlink_ext_ack __always_unused *extack)
#else
			     u16 flags)
#endif
#else
static int ixgbe_ndo_fdb_add(struct ndmsg *ndm,
			     struct net_device *dev,
			     unsigned char *addr,
			     u16 flags)
#endif /* USE_CONST_DEV_UC_CHAR */
{
	/* guarantee we can provide a unique filter for the unicast address */
	if (is_unicast_ether_addr(addr) || is_link_local_ether_addr(addr)) {
		struct ixgbe_adapter *adapter = netdev_priv(dev);
		u16 pool = VMDQ_P(0);

		if (netdev_uc_count(dev) >= ixgbe_available_rars(adapter, pool))
			return -ENOMEM;
	}

#ifdef USE_CONST_DEV_UC_CHAR
#ifdef HAVE_NDO_FDB_ADD_VID
	return ndo_dflt_fdb_add(ndm, tb, dev, addr, vid, flags);
#else
	return ndo_dflt_fdb_add(ndm, tb, dev, addr, flags);
#endif /* HAVE_NDO_FDB_ADD_VID */
#else
	return ndo_dflt_fdb_add(ndm, dev, addr, flags);
#endif /* USE_CONST_DEV_UC_CHAR */
}

#ifdef HAVE_BRIDGE_ATTRIBS
#ifdef HAVE_NDO_BRIDGE_SETLINK_EXTACK
static int ixgbe_ndo_bridge_setlink(struct net_device *dev,
				    struct nlmsghdr *nlh,
				    __always_unused u16 flags,
				    struct netlink_ext_ack __always_unused *ext)
#elif defined(HAVE_NDO_BRIDGE_SET_DEL_LINK_FLAGS)
static int ixgbe_ndo_bridge_setlink(struct net_device *dev,
				    struct nlmsghdr *nlh,
				    __always_unused u16 flags)
#else
static int ixgbe_ndo_bridge_setlink(struct net_device *dev,
				    struct nlmsghdr *nlh)
#endif /* HAVE_NDO_BRIDGE_SETLINK_EXTACK */
{
	struct ixgbe_adapter *adapter = netdev_priv(dev);
	struct nlattr *attr, *br_spec;
	int rem;

	if (!(adapter->flags & IXGBE_FLAG_SRIOV_ENABLED))
		return -EOPNOTSUPP;

	br_spec = nlmsg_find_attr(nlh, sizeof(struct ifinfomsg), IFLA_AF_SPEC);

	nla_for_each_nested(attr, br_spec, rem) {
		__u16 mode;

		if (nla_type(attr) != IFLA_BRIDGE_MODE)
			continue;

		mode = nla_get_u16(attr);
		if (mode == BRIDGE_MODE_VEPA) {
			adapter->flags |= IXGBE_FLAG_SRIOV_VEPA_BRIDGE_MODE;
		} else if (mode == BRIDGE_MODE_VEB) {
			adapter->flags &= ~IXGBE_FLAG_SRIOV_VEPA_BRIDGE_MODE;
		} else {
			return -EINVAL;
		}

		adapter->bridge_mode = mode;

		/* re-configure settings related to bridge mode */
		ixgbe_configure_bridge_mode(adapter);

		e_info(drv, "enabling bridge mode: %s\n",
			mode == BRIDGE_MODE_VEPA ? "VEPA" : "VEB");
	}

	return 0;
}

#ifdef HAVE_NDO_BRIDGE_GETLINK_NLFLAGS
static int ixgbe_ndo_bridge_getlink(struct sk_buff *skb, u32 pid, u32 seq,
				    struct net_device *dev,
				    u32 __maybe_unused filter_mask,
				    int nlflags)
#elif defined(HAVE_BRIDGE_FILTER)
static int ixgbe_ndo_bridge_getlink(struct sk_buff *skb, u32 pid, u32 seq,
				    struct net_device *dev,
				    u32 __always_unused filter_mask)
#else
static int ixgbe_ndo_bridge_getlink(struct sk_buff *skb, u32 pid, u32 seq,
				    struct net_device *dev)
#endif /* HAVE_NDO_BRIDGE_GETLINK_NLFLAGS */
{
	struct ixgbe_adapter *adapter = netdev_priv(dev);
	u16 mode;

	if (!(adapter->flags & IXGBE_FLAG_SRIOV_ENABLED))
		return 0;

	mode = adapter->bridge_mode;
#ifdef HAVE_NDO_DFLT_BRIDGE_GETLINK_VLAN_SUPPORT
	return ndo_dflt_bridge_getlink(skb, pid, seq, dev, mode, 0, 0, nlflags,
				       filter_mask, NULL);
#elif defined(HAVE_NDO_BRIDGE_GETLINK_NLFLAGS)
	return ndo_dflt_bridge_getlink(skb, pid, seq, dev, mode, 0, 0, nlflags);
#elif defined(HAVE_NDO_FDB_ADD_VID) || \
      defined NDO_DFLT_BRIDGE_GETLINK_HAS_BRFLAGS
	return ndo_dflt_bridge_getlink(skb, pid, seq, dev, mode, 0, 0);
#else
	return ndo_dflt_bridge_getlink(skb, pid, seq, dev, mode);
#endif /* HAVE_NDO_BRIDGE_GETLINK_NLFLAGS */
}
#endif /* HAVE_BRIDGE_ATTRIBS */
#endif /* HAVE_FDB_OPS */

#ifdef HAVE_NDO_FEATURES_CHECK
#define IXGBE_MAX_TUNNEL_HDR_LEN 80
#ifdef NETIF_F_GSO_PARTIAL
#define IXGBE_MAX_MAC_HDR_LEN		127
#define IXGBE_MAX_NETWORK_HDR_LEN	511

static netdev_features_t
ixgbe_features_check(struct sk_buff *skb, struct net_device *dev,
		     netdev_features_t features)
{
	unsigned int network_hdr_len, mac_hdr_len;

	/* Make certain the headers can be described by a context descriptor */
	mac_hdr_len = skb_network_header(skb) - skb->data;
	if (unlikely(mac_hdr_len > IXGBE_MAX_MAC_HDR_LEN))
		return features & ~(NETIF_F_HW_CSUM |
				    NETIF_F_SCTP_CRC |
				    NETIF_F_HW_VLAN_CTAG_TX |
				    NETIF_F_TSO |
				    NETIF_F_TSO6);

	network_hdr_len = skb_checksum_start(skb) - skb_network_header(skb);
	if (unlikely(network_hdr_len >  IXGBE_MAX_NETWORK_HDR_LEN))
		return features & ~(NETIF_F_HW_CSUM |
				    NETIF_F_SCTP_CRC |
				    NETIF_F_TSO |
				    NETIF_F_TSO6);

	/* We can only support IPV4 TSO in tunnels if we can mangle the
	 * inner IP ID field, so strip TSO if MANGLEID is not supported.
	 */
	if (skb->encapsulation && !(features & NETIF_F_TSO_MANGLEID))
		features &= ~NETIF_F_TSO;

	return features;
}
#else
static netdev_features_t
ixgbe_features_check(struct sk_buff *skb, struct net_device *dev,
		     netdev_features_t features)
{
	if (!skb->encapsulation)
		return features;

	if (unlikely(skb_inner_mac_header(skb) - skb_transport_header(skb) >
		     IXGBE_MAX_TUNNEL_HDR_LEN))
		return features & ~NETIF_F_CSUM_MASK;

	return features;
}
#endif /* NETIF_F_GSO_PARTIAL */
#endif /* HAVE_NDO_FEATURES_CHECK */

#ifdef HAVE_XDP_SUPPORT
static int ixgbe_xdp_setup(struct net_device *dev, struct bpf_prog *prog)
{
	int i, frame_size = dev->mtu + ETH_HLEN + ETH_FCS_LEN + VLAN_HLEN;
	struct ixgbe_adapter *adapter = netdev_priv(dev);
	struct bpf_prog *old_prog;
	bool need_reset;

	if (adapter->flags & IXGBE_FLAG_SRIOV_ENABLED)
		return -EINVAL;

	if (adapter->flags & IXGBE_FLAG_DCB_ENABLED)
		return -EINVAL;

	/* verify ixgbe ring attributes are sufficient for XDP */
	for (i = 0; i < adapter->num_rx_queues; i++) {
		struct ixgbe_ring *ring = adapter->rx_ring[i];

		if (ring_is_rsc_enabled(ring))
			return -EINVAL;

		if (frame_size > ixgbe_rx_bufsz(ring))
			return -EINVAL;
	}

	if (nr_cpu_ids > MAX_XDP_QUEUES)
		return -ENOMEM;

	old_prog = xchg(&adapter->xdp_prog, prog);
	need_reset = (!!prog != !!old_prog);

	/* If transitioning XDP modes reconfigure rings */
	if (need_reset) {
		int err = ixgbe_setup_tc(dev, netdev_get_num_tc(dev));

		if (err) {
			rcu_assign_pointer(adapter->xdp_prog, old_prog);
			return -EINVAL;
		}
	} else {
		for (i = 0; i < adapter->num_rx_queues; i++)
			xchg(&adapter->rx_ring[i]->xdp_prog, adapter->xdp_prog);
	}

	if (old_prog)
		bpf_prog_put(old_prog);

#ifdef HAVE_AF_XDP_ZC_SUPPORT
	/* Kick start the NAPI context if there is an AF_XDP socket open
	 * on that queue id. This so that receiving will start.
	 */
	if (need_reset && prog)
		for (i = 0; i < adapter->num_rx_queues; i++)
			if (adapter->xdp_ring[i]->xsk_umem)
#ifdef HAVE_NDO_XSK_WAKEUP
				(void)ixgbe_xsk_wakeup(adapter->netdev, i,
						       XDP_WAKEUP_RX);
#else
				(void)ixgbe_xsk_async_xmit(adapter->netdev, i);
#endif

#endif
	return 0;
}

#ifdef HAVE_NDO_BPF
static int ixgbe_xdp(struct net_device *dev, struct netdev_bpf *xdp)
#else
static int ixgbe_xdp(struct net_device *dev, struct netdev_xdp *xdp)
#endif
{
#if defined(HAVE_XDP_QUERY_PROG) || defined(HAVE_AF_XDP_ZC_SUPPORT)
	struct ixgbe_adapter *adapter = netdev_priv(dev);
#endif

	switch (xdp->command) {
	case XDP_SETUP_PROG:
		return ixgbe_xdp_setup(dev, xdp->prog);
#ifdef HAVE_XDP_QUERY_PROG
	case XDP_QUERY_PROG:
#ifndef NO_NETDEV_BPF_PROG_ATTACHED
		xdp->prog_attached = !!(adapter->xdp_prog);
#endif /* !NO_NETDEV_BPF_PROG_ATTACHED */
		xdp->prog_id = adapter->xdp_prog ?
			       adapter->xdp_prog->aux->id : 0;
		return 0;
#endif
#ifdef HAVE_AF_XDP_ZC_SUPPORT
	case XDP_SETUP_XSK_UMEM:
		return ixgbe_xsk_umem_setup(adapter, xdp->xsk.umem,
					    xdp->xsk.queue_id);
#endif
	default:
		return -EINVAL;
	}
}

void ixgbe_xdp_ring_update_tail(struct ixgbe_ring *ring)
{
	/* Force memory writes to complete before letting h/w know there
	 * are new descriptors to fetch.
	 */
	wmb();
	writel(ring->next_to_use, ring->tail);
}

#ifdef HAVE_NDO_XDP_XMIT_BULK_AND_FLAGS
static int ixgbe_xdp_xmit(struct net_device *dev, int n,
			  struct xdp_frame **frames, u32 flags)
#else
static int ixgbe_xdp_xmit(struct net_device *dev, struct xdp_buff *xdp)
#endif
{
	struct ixgbe_adapter *adapter = netdev_priv(dev);
	struct ixgbe_ring *ring;
#ifdef HAVE_NDO_XDP_XMIT_BULK_AND_FLAGS
	int drops = 0;
	int i;
#else
	int err;
#endif

	if (unlikely(test_bit(__IXGBE_DOWN, &adapter->state)))
		return -ENETDOWN;

#ifdef HAVE_NDO_XDP_XMIT_BULK_AND_FLAGS
	if (unlikely(flags & ~XDP_XMIT_FLAGS_MASK))
		return -EINVAL;
#endif

	/* During program transitions its possible adapter->xdp_prog is assigned
	 * but ring has not been configured yet. In this case simply abort xmit.
	 */
	ring = adapter->xdp_prog ? adapter->xdp_ring[smp_processor_id()] : NULL;
	if (unlikely(!ring))
		return -ENXIO;

#ifdef HAVE_AF_XDP_ZC_SUPPORT
	if (unlikely(test_bit(__IXGBE_TX_DISABLED, &ring->state)))
		return -ENXIO;
#endif

#ifdef HAVE_NDO_XDP_XMIT_BULK_AND_FLAGS
	for (i = 0; i < n; i++) {
		struct xdp_frame *xdpf = frames[i];
		int err;

		err = ixgbe_xmit_xdp_ring(adapter, xdpf);
		if (err != IXGBE_XDP_TX) {
			xdp_return_frame_rx_napi(xdpf);
			drops++;
		}
	}

	if (unlikely(flags & XDP_XMIT_FLUSH))
		ixgbe_xdp_ring_update_tail(ring);

	return n - drops;
#else
	err = ixgbe_xmit_xdp_ring(adapter, xdp);
	if (err != IXGBE_XDP_TX)
		return -ENOSPC;

	return 0;
#endif
}

#ifndef NO_NDO_XDP_FLUSH
static void ixgbe_xdp_flush(struct net_device *dev)
{
	struct ixgbe_adapter *adapter = netdev_priv(dev);
	struct ixgbe_ring *ring;

	/* Its possible the device went down between xdp xmit and flush so
	 * we need to ensure device is still up.
	 */
	if (unlikely(test_bit(__IXGBE_DOWN, &adapter->state)))
		return;

	ring = adapter->xdp_prog ? adapter->xdp_ring[smp_processor_id()] : NULL;
	if (unlikely(!ring))
		return;

	ixgbe_xdp_ring_update_tail(ring);

	return;
}
#endif /* !NO_NDO_XDP_FLUSH */
#endif /* HAVE_XDP_SUPPORT */
#ifdef HAVE_NET_DEVICE_OPS
static const struct net_device_ops ixgbe_netdev_ops = {
	.ndo_open		= ixgbe_open,
	.ndo_stop		= ixgbe_close,
	.ndo_start_xmit		= ixgbe_xmit_frame,
#ifdef HAVE_NETDEV_SELECT_QUEUE
#if IS_ENABLED(CONFIG_FCOE)
	.ndo_select_queue	= ixgbe_select_queue,
#else
#ifndef HAVE_MQPRIO
	.ndo_select_queue	= __netdev_pick_tx,
#endif
#endif /* CONFIG_FCOE */
#endif /* HAVE_NETDEV_SELECT_QUEUE */
	.ndo_set_rx_mode	= ixgbe_set_rx_mode,
	.ndo_validate_addr	= eth_validate_addr,
	.ndo_set_mac_address	= ixgbe_set_mac,
#ifdef HAVE_RHEL7_EXTENDED_MIN_MAX_MTU
	.extended.ndo_change_mtu = ixgbe_change_mtu,
#else
	.ndo_change_mtu		= ixgbe_change_mtu,
#endif
	.ndo_tx_timeout		= ixgbe_tx_timeout,
#if defined(NETIF_F_HW_VLAN_TX) || defined(NETIF_F_HW_VLAN_CTAG_TX)
	.ndo_vlan_rx_add_vid	= ixgbe_vlan_rx_add_vid,
	.ndo_vlan_rx_kill_vid	= ixgbe_vlan_rx_kill_vid,
#endif
	.ndo_do_ioctl		= ixgbe_ioctl,
#ifdef HAVE_RHEL7_NET_DEVICE_OPS_EXT
/* RHEL7 requires this to be defined to enable extended ops.  RHEL7 uses the
 * function get_ndo_ext to retrieve offsets for extended fields from with the
 * net_device_ops struct and ndo_size is checked to determine whether or not
 * the offset is valid.
 */
	.ndo_size		= sizeof(const struct net_device_ops),
#endif
#ifdef IFLA_VF_MAX
	.ndo_set_vf_mac		= ixgbe_ndo_set_vf_mac,
#ifdef HAVE_RHEL7_NETDEV_OPS_EXT_NDO_SET_VF_VLAN
	.extended.ndo_set_vf_vlan = ixgbe_ndo_set_vf_vlan,
#else
	.ndo_set_vf_vlan	= ixgbe_ndo_set_vf_vlan,
#endif
#ifdef HAVE_NDO_SET_VF_MIN_MAX_TX_RATE
	.ndo_set_vf_rate	= ixgbe_ndo_set_vf_bw,
#else
	.ndo_set_vf_tx_rate	= ixgbe_ndo_set_vf_bw,
#endif /* HAVE_NDO_SET_VF_MIN_MAX_TX_RATE */
#if defined(HAVE_VF_SPOOFCHK_CONFIGURE) && IS_ENABLED(CONFIG_PCI_IOV)
	.ndo_set_vf_spoofchk	= ixgbe_ndo_set_vf_spoofchk,
#endif
#ifdef HAVE_NDO_SET_VF_RSS_QUERY_EN
	.ndo_set_vf_rss_query_en = ixgbe_ndo_set_vf_rss_query_en,
#endif
#ifdef HAVE_NDO_SET_VF_TRUST
#ifdef HAVE_RHEL7_NET_DEVICE_OPS_EXT
	.extended.ndo_set_vf_trust = ixgbe_ndo_set_vf_trust,
#else
	.ndo_set_vf_trust	= ixgbe_ndo_set_vf_trust,
#endif /* HAVE_RHEL7_NET_DEVICE_OPS_EXT */
#endif /* HAVE_NDO_SET_VF_TRUST */
	.ndo_get_vf_config	= ixgbe_ndo_get_vf_config,
#endif /* IFLA_VF_MAX */
#ifdef HAVE_NDO_GET_STATS64
	.ndo_get_stats64	= ixgbe_get_stats64,
#else
	.ndo_get_stats		= ixgbe_get_stats,
#endif /* HAVE_NDO_GET_STATS64 */
#ifdef HAVE_SETUP_TC
#ifdef HAVE_RHEL7_NETDEV_OPS_EXT_NDO_SETUP_TC
	.extended.ndo_setup_tc_rh = __ixgbe_setup_tc,
#else
#ifdef NETIF_F_HW_TC
	.ndo_setup_tc		= __ixgbe_setup_tc,
#else
	.ndo_setup_tc		= ixgbe_setup_tc,
#endif /* NETIF_F_HW_TC */
#endif /* HAVE_RHEL7_NETDEV_OPS_EXT_NDO_SETUP_TC */
#endif /* HAVE_SETUP_TC */
#ifdef CONFIG_NET_POLL_CONTROLLER
	.ndo_poll_controller	= ixgbe_netpoll,
#endif
#ifndef HAVE_RHEL6_NET_DEVICE_EXTENDED
#ifdef HAVE_NDO_BUSY_POLL
	.ndo_busy_poll		= ixgbe_busy_poll_recv,
#endif /* HAVE_NDO_BUSY_POLL */
#endif /* !HAVE_RHEL6_NET_DEVICE_EXTENDED */
#if IS_ENABLED(CONFIG_FCOE)
	.ndo_fcoe_ddp_setup = ixgbe_fcoe_ddp_get,
#ifdef HAVE_NETDEV_OPS_FCOE_DDP_TARGET
	.ndo_fcoe_ddp_target = ixgbe_fcoe_ddp_target,
#endif
	.ndo_fcoe_ddp_done = ixgbe_fcoe_ddp_put,
#ifdef HAVE_NETDEV_OPS_FCOE_ENABLE
	.ndo_fcoe_enable = ixgbe_fcoe_enable,
	.ndo_fcoe_disable = ixgbe_fcoe_disable,
#endif
#ifdef HAVE_NETDEV_OPS_FCOE_GETWWN
	.ndo_fcoe_get_wwn = ixgbe_fcoe_get_wwn,
#endif
#endif /* CONFIG_FCOE */
#ifdef HAVE_VLAN_RX_REGISTER
	.ndo_vlan_rx_register	= &ixgbe_vlan_mode,
#endif
#ifdef HAVE_FDB_OPS
	.ndo_fdb_add		= ixgbe_ndo_fdb_add,
#ifndef USE_DEFAULT_FDB_DEL_DUMP
	.ndo_fdb_del		= ndo_dflt_fdb_del,
	.ndo_fdb_dump		= ndo_dflt_fdb_dump,
#endif
#ifdef HAVE_BRIDGE_ATTRIBS
	.ndo_bridge_setlink	= ixgbe_ndo_bridge_setlink,
	.ndo_bridge_getlink	= ixgbe_ndo_bridge_getlink,
#endif /* HAVE_BRIDGE_ATTRIBS */
#endif
#ifdef HAVE_UDP_ENC_RX_OFFLOAD
#ifdef HAVE_RHEL7_NETDEV_OPS_EXT_NDO_UDP_TUNNEL
	.extended.ndo_udp_tunnel_add = ixgbe_add_udp_tunnel_port,
	.extended.ndo_udp_tunnel_del = ixgbe_del_udp_tunnel_port,
#else
	.ndo_udp_tunnel_add	= ixgbe_add_udp_tunnel_port,
	.ndo_udp_tunnel_del	= ixgbe_del_udp_tunnel_port,
#endif
#elif defined(HAVE_VXLAN_RX_OFFLOAD)
	.ndo_add_vxlan_port	= ixgbe_add_vxlan_port,
	.ndo_del_vxlan_port	= ixgbe_del_vxlan_port,
#endif /* HAVE_UDP_ENC_RX_OFFLOAD */
#ifdef HAVE_NDO_GSO_CHECK
	.ndo_gso_check		= ixgbe_gso_check,
#endif /* HAVE_NDO_GSO_CHECK */
#ifdef HAVE_NDO_FEATURES_CHECK
	.ndo_features_check	= ixgbe_features_check,
#endif /* HAVE_NDO_FEATURES_CHECK */
#ifdef HAVE_XDP_SUPPORT
#ifdef HAVE_NDO_BPF
	.ndo_bpf                = ixgbe_xdp,
#else
	.ndo_xdp                = ixgbe_xdp,
#endif
	.ndo_xdp_xmit           = ixgbe_xdp_xmit,
#ifdef HAVE_AF_XDP_ZC_SUPPORT
#ifdef HAVE_NDO_XSK_WAKEUP
	.ndo_xsk_wakeup         = ixgbe_xsk_wakeup,
#else
	.ndo_xsk_async_xmit     = ixgbe_xsk_async_xmit,
#endif
#endif
#ifndef NO_NDO_XDP_FLUSH
	.ndo_xdp_flush          = ixgbe_xdp_flush,
#endif /* !NO_NDO_XDP_FLUSH */
#endif
#ifdef HAVE_RHEL6_NET_DEVICE_OPS_EXT
};

/* RHEL6 keeps these operations in a separate structure */
static const struct net_device_ops_ext ixgbe_netdev_ops_ext = {
	.size = sizeof(struct net_device_ops_ext),
#endif /* HAVE_RHEL6_NET_DEVICE_OPS_EXT */
#ifdef HAVE_NDO_SET_FEATURES
	.ndo_set_features = ixgbe_set_features,
	.ndo_fix_features = ixgbe_fix_features,
#endif /* HAVE_NDO_SET_FEATURES */
};
#endif /* HAVE_NET_DEVICE_OPS */

void ixgbe_assign_netdev_ops(struct net_device *dev)
{
#ifdef HAVE_NET_DEVICE_OPS
	dev->netdev_ops = &ixgbe_netdev_ops;
#ifdef HAVE_RHEL6_NET_DEVICE_OPS_EXT
	set_netdev_ops_ext(dev, &ixgbe_netdev_ops_ext);
#endif /* HAVE_RHEL6_NET_DEVICE_OPS_EXT */
#else /* HAVE_NET_DEVICE_OPS */
	dev->open = &ixgbe_open;
	dev->stop = &ixgbe_close;
	dev->hard_start_xmit = &ixgbe_xmit_frame;
	dev->get_stats = &ixgbe_get_stats;
#ifdef HAVE_SET_RX_MODE
	dev->set_rx_mode = &ixgbe_set_rx_mode;
#endif
	dev->set_multicast_list = &ixgbe_set_rx_mode;
	dev->set_mac_address = &ixgbe_set_mac;
	dev->change_mtu = &ixgbe_change_mtu;
	dev->do_ioctl = &ixgbe_ioctl;
#ifdef HAVE_TX_TIMEOUT
	dev->tx_timeout = &ixgbe_tx_timeout;
#endif
#if defined(NETIF_F_HW_VLAN_TX) || defined(NETIF_F_HW_VLAN_CTAG_TX)
	dev->vlan_rx_register = &ixgbe_vlan_mode;
	dev->vlan_rx_add_vid = &ixgbe_vlan_rx_add_vid;
	dev->vlan_rx_kill_vid = &ixgbe_vlan_rx_kill_vid;
#endif
#ifdef CONFIG_NET_POLL_CONTROLLER
	dev->poll_controller = &ixgbe_netpoll;
#endif
#ifdef HAVE_NETDEV_SELECT_QUEUE
#if IS_ENABLED(CONFIG_FCOE)
	dev->select_queue = &ixgbe_select_queue;
#else
	dev->select_queue = &__netdev_pick_tx;
#endif
#endif /* HAVE_NETDEV_SELECT_QUEUE */
#endif /* HAVE_NET_DEVICE_OPS */

#ifdef HAVE_RHEL6_NET_DEVICE_EXTENDED
#ifdef HAVE_NDO_BUSY_POLL
	netdev_extended(dev)->ndo_busy_poll		= ixgbe_busy_poll_recv;
#endif /* HAVE_NDO_BUSY_POLL */
#endif /* HAVE_RHEL6_NET_DEVICE_EXTENDED */

	ixgbe_set_ethtool_ops(dev);
	dev->watchdog_timeo = 5 * HZ;
}

/**
 * ixgbe_wol_supported - Check whether device supports WoL
 * @adapter: the adapter private structure
 * @device_id: the device ID
 * @subdevice_id: the subsystem device ID
 *
 * This function is used by probe and ethtool to determine
 * which devices have WoL support
 *
 **/
bool ixgbe_wol_supported(struct ixgbe_adapter *adapter, u16 device_id,
			 u16 subdevice_id)
{
	struct ixgbe_hw *hw = &adapter->hw;
	u16 wol_cap = adapter->eeprom_cap & IXGBE_DEVICE_CAPS_WOL_MASK;

	/* WOL not supported on 82598 */
	if (hw->mac.type == ixgbe_mac_82598EB)
		return false;

	/* check eeprom to see if WOL is enabled for X540 and newer */
	if (hw->mac.type >= ixgbe_mac_X540) {
		if ((wol_cap == IXGBE_DEVICE_CAPS_WOL_PORT0_1) ||
		    ((wol_cap == IXGBE_DEVICE_CAPS_WOL_PORT0) &&
		     (hw->bus.func == 0)))
			return true;
	}

	/* WOL is determined based on device IDs for 82599 MACs */
	switch (device_id) {
	case IXGBE_DEV_ID_82599_SFP:
		/* Only these subdevices could supports WOL */
		switch (subdevice_id) {
		case IXGBE_SUBDEV_ID_82599_560FLR:
		case IXGBE_SUBDEV_ID_82599_LOM_SNAP6:
		case IXGBE_SUBDEV_ID_82599_SFP_WOL0:
		case IXGBE_SUBDEV_ID_82599_SFP_2OCP:
			/* only support first port */
			if (hw->bus.func != 0)
				break;
			/* fall through */
		case IXGBE_SUBDEV_ID_82599_SP_560FLR:
		case IXGBE_SUBDEV_ID_82599_SFP:
		case IXGBE_SUBDEV_ID_82599_RNDC:
		case IXGBE_SUBDEV_ID_82599_ECNA_DP:
		case IXGBE_SUBDEV_ID_82599_SFP_1OCP:
		case IXGBE_SUBDEV_ID_82599_SFP_LOM_OEM1:
		case IXGBE_SUBDEV_ID_82599_SFP_LOM_OEM2:
			return true;
		}
		break;
	case IXGBE_DEV_ID_82599EN_SFP:
		/* Only these subdevices support WoL */
		switch (subdevice_id) {
		case IXGBE_SUBDEV_ID_82599EN_SFP_OCP1:
			return true;
		}
		break;
	case IXGBE_DEV_ID_82599_COMBO_BACKPLANE:
		/* All except this subdevice support WOL */
		if (subdevice_id != IXGBE_SUBDEV_ID_82599_KX4_KR_MEZZ)
			return true;
		break;
	case IXGBE_DEV_ID_82599_KX4:
		return  true;
	default:
		break;
	}

	return false;
}

/**
 * ixgbe_set_fw_version - Set FW version
 * @adapter: the adapter private structure
 *
 * This function is used by probe and ethtool to determine the FW version to
 * format to display. The FW version is taken from the EEPROM/NVM.
 *
 **/
static void ixgbe_set_fw_version(struct ixgbe_adapter *adapter)
{
	struct ixgbe_hw *hw = &adapter->hw;
	u16 eeprom_verh = 0, eeprom_verl = 0;
	u16 offset = 0;
	u32 etrack_id;

	/* Check for OEM Product Version block format */
	hw->eeprom.ops.read(hw, 0x1b, &offset);

	/* Make sure offset to OEM Product Version block is valid */
	if (!(offset == 0x0) && !(offset == 0xffff)) {
		u16 mod_len = 0, cap = 0, prod_ver = 0, rel_num = 0;
		u16 build, major, patch;

		/* Read product version block */
		hw->eeprom.ops.read(hw, offset, &mod_len);
		hw->eeprom.ops.read(hw, offset + 0x1, &cap);
		hw->eeprom.ops.read(hw, offset + 0x2, &prod_ver);
		hw->eeprom.ops.read(hw, offset + 0x3, &rel_num);

		/* Only display OEM product version if valid block */
		if (mod_len == 0x3 && (cap & 0xf) == 0x0) {
			major = prod_ver >> 8;
			build = prod_ver & 0xff;
			patch = rel_num;

			snprintf(adapter->eeprom_id, sizeof(adapter->eeprom_id),
				 "%x.%x.%x", major, build, patch);
			return;
		}
	}

	/*
	 * Save off EEPROM version number and Option Rom version which
	 * together make a unique identify for the eeprom
	 */
	hw->eeprom.ops.read(hw, 0x2e, &eeprom_verh);
	hw->eeprom.ops.read(hw, 0x2d, &eeprom_verl);
	etrack_id = (eeprom_verh << 16) | eeprom_verl;

	/* Check for SCSI block version format */
	hw->eeprom.ops.read(hw, 0x17, &offset);

	/* Make sure offset to SCSI block is valid */
	if (!(offset == 0x0) && !(offset == 0xffff)) {
		u16 eeprom_cfg_blkh = 0, eeprom_cfg_blkl = 0;
		u16 build, major, patch;

		hw->eeprom.ops.read(hw, offset + 0x84, &eeprom_cfg_blkh);
		hw->eeprom.ops.read(hw, offset + 0x83, &eeprom_cfg_blkl);

		/* Only display Option Rom if exist */
		if (eeprom_cfg_blkl && eeprom_cfg_blkh) {
			major = eeprom_cfg_blkl >> 8;
			build = (eeprom_cfg_blkl << 8) | (eeprom_cfg_blkh >> 8);
			patch = eeprom_cfg_blkh & 0x00ff;

			snprintf(adapter->eeprom_id, sizeof(adapter->eeprom_id),
				 "0x%08x, %d.%d.%d", etrack_id, major, build,
				 patch);
			return;
		}
	}

	/* Set ETrack ID format */
	snprintf(adapter->eeprom_id, sizeof(adapter->eeprom_id),
		 "0x%08x", etrack_id);
}

/**
 * ixgbe_probe - Device Initialization Routine
 * @pdev: PCI device information struct
 * @ent: entry in ixgbe_pci_tbl
 *
 * Returns 0 on success, negative on failure
 *
 * ixgbe_probe initializes an adapter identified by a pci_dev structure.
 * The OS initialization, configuring of the adapter private structure,
 * and a hardware reset occur.
 **/
#ifdef HAVE_CONFIG_HOTPLUG
static int __devinit ixgbe_probe(struct pci_dev *pdev,
				 const struct pci_device_id __always_unused *ent)
#else
static int ixgbe_probe(struct pci_dev *pdev,
		       const struct pci_device_id __always_unused *ent)
#endif
{
	struct net_device *netdev;
	struct ixgbe_adapter *adapter = NULL;
	struct ixgbe_hw *hw = NULL;
	static int cards_found;
	int err, pci_using_dac;
	char *info_string;
	u8 part_str[IXGBE_PBANUM_LENGTH];
	enum ixgbe_mac_type mac_type = ixgbe_mac_unknown;
#ifdef HAVE_TX_MQ
	unsigned int indices = MAX_TX_QUEUES;
#endif /* HAVE_TX_MQ */
	bool disable_dev = false;
#if IS_ENABLED(CONFIG_FCOE)
	u16 device_caps;
#endif
#ifndef NETIF_F_GSO_PARTIAL
#ifdef HAVE_NDO_SET_FEATURES
#ifndef HAVE_RHEL6_NET_DEVICE_OPS_EXT
	netdev_features_t hw_features;
#else
	u32 hw_features;
#endif
#endif
#endif /* NETIF_F_GSO_PARTIAL */
	int i;
	err = pci_enable_device_mem(pdev);
	if (err)
		return err;

	if (!dma_set_mask(pci_dev_to_dev(pdev), DMA_BIT_MASK(64)) &&
	    !dma_set_coherent_mask(pci_dev_to_dev(pdev), DMA_BIT_MASK(64))) {
		pci_using_dac = 1;
	} else {
		err = dma_set_mask(pci_dev_to_dev(pdev), DMA_BIT_MASK(32));
		if (err) {
			err = dma_set_coherent_mask(pci_dev_to_dev(pdev),
						    DMA_BIT_MASK(32));
			if (err) {
				dev_err(pci_dev_to_dev(pdev), "No usable DMA "
					"configuration, aborting\n");
				goto err_dma;
			}
		}
		pci_using_dac = 0;
	}

	err = pci_request_mem_regions(pdev, ixgbe_driver_name);
	if (err) {
		dev_err(pci_dev_to_dev(pdev),
			"pci_request_selected_regions failed 0x%x\n", err);
		goto err_pci_reg;
	}

	/*
	 * The mac_type is needed before we have the adapter is  set up
	 * so rather than maintain two devID -> MAC tables we dummy up
	 * an ixgbe_hw stuct and use ixgbe_set_mac_type.
	 */
	hw = vmalloc(sizeof(struct ixgbe_hw));
	if (!hw) {
		pr_info("Unable to allocate memory for early mac "
			"check\n");
	} else {
		hw->vendor_id = pdev->vendor;
		hw->device_id = pdev->device;
		ixgbe_set_mac_type(hw);
		mac_type = hw->mac.type;
		vfree(hw);
	}

	/*
	 * Workaround of Silicon errata on 82598. Disable LOs in the PCI switch
	 * port to which the 82598 is connected to prevent duplicate
	 * completions caused by LOs.  We need the mac type so that we only
	 * do this on 82598 devices, ixgbe_set_mac_type does this for us if
	 * we set it's device ID.
	 */
	if (mac_type == ixgbe_mac_82598EB)
		pci_disable_link_state(pdev, PCIE_LINK_STATE_L0S);

	pci_enable_pcie_error_reporting(pdev);

	pci_set_master(pdev);

#ifdef HAVE_TX_MQ
	if (mac_type == ixgbe_mac_82598EB) {
#if IS_ENABLED(CONFIG_DCB)
		indices = IXGBE_MAX_DCB_INDICES * 4;
#else /* CONFIG_DCB */
		indices = IXGBE_MAX_RSS_INDICES;
#endif /* !CONFIG_DCB */
	}

	netdev = alloc_etherdev_mq(sizeof(struct ixgbe_adapter), indices);
#else /* HAVE_TX_MQ */
	netdev = alloc_etherdev(sizeof(struct ixgbe_adapter));
#endif /* HAVE_TX_MQ */
	if (!netdev) {
		err = -ENOMEM;
		goto err_alloc_etherdev;
	}

	SET_MODULE_OWNER(netdev);
	SET_NETDEV_DEV(netdev, pci_dev_to_dev(pdev));

	adapter = netdev_priv(netdev);
#ifdef HAVE_TX_MQ
#ifndef HAVE_NETDEV_SELECT_QUEUE
	adapter->indices = indices;
#endif
#endif
	adapter->netdev = netdev;
	adapter->pdev = pdev;
	hw = &adapter->hw;
	hw->back = adapter;
	adapter->msg_enable = (1 << DEFAULT_DEBUG_LEVEL_SHIFT) - 1;

	hw->hw_addr = ioremap(pci_resource_start(pdev, 0),
			      pci_resource_len(pdev, 0));
	adapter->io_addr = hw->hw_addr;
	if (!hw->hw_addr) {
		err = -EIO;
		goto err_ioremap;
	}

	ixgbe_assign_netdev_ops(netdev);

	strscpy(netdev->name, pci_name(pdev), sizeof(netdev->name));

	adapter->bd_number = cards_found;

	ixgbe_get_hw_control(adapter);
	/* setup the private structure */
	err = ixgbe_sw_init(adapter);
	if (err)
		goto err_sw_init;

	/* Make sure the SWFW semaphore is in a valid state */
	if (hw->mac.ops.init_swfw_sync)
		hw->mac.ops.init_swfw_sync(hw);

	/* Make it possible the adapter to be woken up via WOL */
	switch (adapter->hw.mac.type) {
	case ixgbe_mac_82599EB:
	case ixgbe_mac_X540:
	case ixgbe_mac_X550:
	case ixgbe_mac_X550EM_x:
	case ixgbe_mac_X550EM_a:
		IXGBE_WRITE_REG(&adapter->hw, IXGBE_WUS, ~0);
		break;
	default:
		break;
	}

	/*
	 * If we have a fan, this is as early we know, warn if we
	 * have had a failure.
	 */
	if (adapter->flags & IXGBE_FLAG_FAN_FAIL_CAPABLE) {
		u32 esdp = IXGBE_READ_REG(hw, IXGBE_ESDP);
		if (esdp & IXGBE_ESDP_SDP1)
			e_crit(probe, "Fan has stopped, replace the adapter\n");
	}

	/*
	 * check_options must be called before setup_link to set up
	 * hw->fc completely
	 */
	ixgbe_check_options(adapter);

	/* reset_hw fills in the perm_addr as well */
	hw->phy.reset_if_overtemp = true;
	err = hw->mac.ops.reset_hw(hw);
	hw->phy.reset_if_overtemp = false;
	if (err == IXGBE_ERR_SFP_NOT_PRESENT) {
		err = IXGBE_SUCCESS;
	} else if (err == IXGBE_ERR_SFP_NOT_SUPPORTED) {
		e_dev_err("failed to load because an unsupported SFP+ or QSFP "
			  "module type was detected.\n");
		e_dev_err("Reload the driver after installing a supported "
			  "module.\n");
		goto err_sw_init;
	} else if (err) {
		e_dev_err("HW Init failed: %d\n", err);
		goto err_sw_init;
	}

#ifdef CONFIG_PCI_IOV
#if defined(HAVE_SRIOV_CONFIGURE) || defined(HAVE_RHEL6_SRIOV_CONFIGURE)
	if (adapter->max_vfs > 0) {
		e_dev_warn("Enabling SR-IOV VFs using the max_vfs module parameter is deprecated.\n");
		e_dev_warn("Please use the pci sysfs interface instead. Ex:\n");
		e_dev_warn("echo '%d' > /sys/bus/pci/devices/%04x:%02x:%02x.%1x/sriov_numvfs\n",
			   adapter->max_vfs,
			   pci_domain_nr(pdev->bus),
			   pdev->bus->number,
			   PCI_SLOT(pdev->devfn),
			   PCI_FUNC(pdev->devfn)
			   );
	}

#endif
	if (adapter->flags & IXGBE_FLAG_SRIOV_CAPABLE) {
		pci_sriov_set_totalvfs(pdev, IXGBE_MAX_VFS_DRV_LIMIT);
		ixgbe_enable_sriov(adapter);
	}

#endif /* CONFIG_PCI_IOV */

#ifdef NETIF_F_GSO_PARTIAL
	/* keep |= here to avoid conflict with features set in param.c */
	netdev->features |= NETIF_F_SG |
			    NETIF_F_TSO |
			    NETIF_F_TSO6 |
			    NETIF_F_RXHASH |
			    NETIF_F_RXCSUM |
			    NETIF_F_HW_CSUM;

	netdev->gso_partial_features = IXGBE_GSO_PARTIAL_FEATURES;
	netdev->features |= NETIF_F_GSO_PARTIAL |
			    IXGBE_GSO_PARTIAL_FEATURES;

	if (hw->mac.type >= ixgbe_mac_82599EB)
		netdev->features |= NETIF_F_SCTP_CRC;

	/* copy netdev features into list of user selectable features */
	netdev->hw_features |= netdev->features |
			       NETIF_F_HW_VLAN_CTAG_FILTER |
			       NETIF_F_HW_VLAN_CTAG_RX |
			       NETIF_F_HW_VLAN_CTAG_TX |
			       NETIF_F_RXALL;

	if (hw->mac.type >= ixgbe_mac_82599EB)
		netdev->hw_features |= NETIF_F_NTUPLE |
				       NETIF_F_HW_TC;

	if (pci_using_dac)
		netdev->features |= NETIF_F_HIGHDMA;

	netdev->vlan_features |= netdev->features | NETIF_F_TSO_MANGLEID;
	netdev->hw_enc_features |= netdev->vlan_features;
	netdev->mpls_features |= NETIF_F_HW_CSUM;

	/* set this bit last since it cannot be part of vlan_features */
	netdev->features |= NETIF_F_HW_VLAN_CTAG_FILTER |
			    NETIF_F_HW_VLAN_CTAG_RX |
			    NETIF_F_HW_VLAN_CTAG_TX;

	netdev->priv_flags |= IFF_UNICAST_FLT;
	netdev->priv_flags |= IFF_SUPP_NOFCS;

	/* give us the option of enabling RSC/LRO later */
	if (adapter->flags2 & IXGBE_FLAG2_RSC_CAPABLE)
		netdev->hw_features |= NETIF_F_LRO;

#else	/* NETIF_F_GSO_PARTIAL */
	/* keep |= here to avoid conflict with features set in param.c */
	netdev->features |= NETIF_F_SG |
			    NETIF_F_IP_CSUM;

#ifdef NETIF_F_IPV6_CSUM
	netdev->features |= NETIF_F_IPV6_CSUM;
#endif

#ifdef NETIF_F_HW_VLAN_CTAG_TX
	netdev->features |= NETIF_F_HW_VLAN_CTAG_TX |
			    NETIF_F_HW_VLAN_CTAG_FILTER |
			    NETIF_F_HW_VLAN_CTAG_RX;
#endif

#ifdef NETIF_F_HW_VLAN_TX
	netdev->features |= NETIF_F_HW_VLAN_TX |
			    NETIF_F_HW_VLAN_FILTER |
			    NETIF_F_HW_VLAN_RX;
#endif
	netdev->features |= ixgbe_tso_features();
#ifdef NETIF_F_RXHASH
	netdev->features |= NETIF_F_RXHASH;
#endif /* NETIF_F_RXHASH */
	netdev->features |= NETIF_F_RXCSUM;

#ifdef HAVE_NDO_SET_FEATURES
	/* copy netdev features into list of user selectable features */
#ifndef HAVE_RHEL6_NET_DEVICE_OPS_EXT
	hw_features = netdev->hw_features;
#else
	hw_features = get_netdev_hw_features(netdev);
#endif
	hw_features |= netdev->features;

	/* give us the option of enabling RSC/LRO later */
	if (adapter->flags2 & IXGBE_FLAG2_RSC_CAPABLE)
		hw_features |= NETIF_F_LRO;

#else
#ifdef NETIF_F_GRO

	/* this is only needed on kernels prior to 2.6.39 */
	netdev->features |= NETIF_F_GRO;
#endif /* NETIF_F_GRO */
#endif /* HAVE_NDO_SET_FEATURES */

	switch (adapter->hw.mac.type) {
	case ixgbe_mac_82599EB:
	case ixgbe_mac_X540:
	case ixgbe_mac_X550:
	case ixgbe_mac_X550EM_x:
	case ixgbe_mac_X550EM_a:
		netdev->features |= NETIF_F_SCTP_CSUM;
#ifdef HAVE_NDO_SET_FEATURES
		hw_features |= NETIF_F_SCTP_CSUM |
#ifdef NETIF_F_HW_TC
			       NETIF_F_HW_TC |
#endif
			       NETIF_F_NTUPLE;
#endif
		break;
	default:
		break;
	}
#ifdef HAVE_NDO_SET_FEATURES
#ifdef HAVE_RHEL6_NET_DEVICE_OPS_EXT
	set_netdev_hw_features(netdev, hw_features);
#else
	netdev->hw_features = hw_features;
#endif
#endif

#ifdef HAVE_NETDEV_VLAN_FEATURES
	netdev->vlan_features |= NETIF_F_SG |
				 NETIF_F_IP_CSUM |
				 NETIF_F_IPV6_CSUM |
				 NETIF_F_TSO |
				 NETIF_F_TSO6;

#endif /* HAVE_NETDEV_VLAN_FEATURES */
#ifdef HAVE_ENCAP_CSUM_OFFLOAD
	netdev->hw_enc_features |= NETIF_F_SG;
#endif /* HAVE_ENCAP_CSUM_OFFLOAD */
#ifdef HAVE_VXLAN_RX_OFFLOAD
	if (adapter->flags & IXGBE_FLAG_VXLAN_OFFLOAD_CAPABLE) {
		netdev->hw_enc_features |= NETIF_F_IP_CSUM |
					   NETIF_F_IPV6_CSUM;
	}
#endif /* NETIF_F_GSO_PARTIAL */

#endif /* HAVE_VXLAN_RX_OFFLOAD */
	if (netdev->features & NETIF_F_LRO) {
		if ((adapter->flags2 & IXGBE_FLAG2_RSC_CAPABLE) &&
		    ((adapter->rx_itr_setting == 1) ||
		     (adapter->rx_itr_setting > IXGBE_MIN_RSC_ITR))) {
			adapter->flags2 |= IXGBE_FLAG2_RSC_ENABLED;
		} else if (adapter->flags2 & IXGBE_FLAG2_RSC_CAPABLE) {
			e_dev_info("InterruptThrottleRate set too high, "
				   "disabling RSC\n");
		}
	}
#ifdef IFF_UNICAST_FLT
	netdev->priv_flags |= IFF_UNICAST_FLT;
#endif
#ifdef IFF_SUPP_NOFCS
	netdev->priv_flags |= IFF_SUPP_NOFCS;
#endif

#ifdef HAVE_NETDEVICE_MIN_MAX_MTU
	/* MTU range: 68 - 9710 */
#ifdef HAVE_RHEL7_EXTENDED_MIN_MAX_MTU
	netdev->extended->min_mtu = ETH_MIN_MTU;
	netdev->extended->max_mtu = IXGBE_MAX_JUMBO_FRAME_SIZE -
				    (ETH_HLEN + ETH_FCS_LEN);
#else
	netdev->min_mtu = ETH_MIN_MTU;
	netdev->max_mtu = IXGBE_MAX_JUMBO_FRAME_SIZE - (ETH_HLEN + ETH_FCS_LEN);
#endif

#endif
#if IS_ENABLED(CONFIG_DCB)
	if (adapter->flags & IXGBE_FLAG_DCB_CAPABLE)
		netdev->dcbnl_ops = &ixgbe_dcbnl_ops;

#endif /* CONFIG_DCB */
#if IS_ENABLED(CONFIG_FCOE)
#ifdef NETIF_F_FSO
	if (adapter->flags & IXGBE_FLAG_FCOE_CAPABLE) {
		unsigned int fcoe_l;

		hw->mac.ops.get_device_caps(hw, &device_caps);
		if (device_caps & IXGBE_DEVICE_CAPS_FCOE_OFFLOADS) {
			adapter->flags &= ~IXGBE_FLAG_FCOE_ENABLED;
			adapter->flags &= ~IXGBE_FLAG_FCOE_CAPABLE;
			e_dev_info("FCoE offload feature is not available. "
				   "Disabling FCoE offload feature\n");
		} else {
			netdev->features |= NETIF_F_FSO |
					    NETIF_F_FCOE_CRC;
#ifndef HAVE_NETDEV_OPS_FCOE_ENABLE
			ixgbe_fcoe_ddp_enable(adapter);
			adapter->flags |= IXGBE_FLAG_FCOE_ENABLED;
			netdev->features |= NETIF_F_FCOE_MTU;
#endif /* HAVE_NETDEV_OPS_FCOE_ENABLE */
		}

		fcoe_l = min_t(int, IXGBE_FCRETA_SIZE, num_online_cpus());
		adapter->ring_feature[RING_F_FCOE].limit = fcoe_l;

#ifdef HAVE_NETDEV_VLAN_FEATURES
		netdev->vlan_features |= NETIF_F_FSO |
					 NETIF_F_FCOE_CRC |
					 NETIF_F_FCOE_MTU;
#endif /* HAVE_NETDEV_VLAN_FEATURES */
	}
#endif /* NETIF_F_FSO */
#endif /* CONFIG_FCOE */
	if (pci_using_dac) {
		netdev->features |= NETIF_F_HIGHDMA;
#ifdef HAVE_NETDEV_VLAN_FEATURES
		netdev->vlan_features |= NETIF_F_HIGHDMA;
#endif /* HAVE_NETDEV_VLAN_FEATURES */
	}

	if (ixgbe_check_fw_error(adapter)) {
		err = -EIO;
		goto err_sw_init;
	}

	if (hw->eeprom.ops.validate_checksum  &&
	    (hw->eeprom.ops.validate_checksum(hw, NULL) < 0)) {
		e_dev_err("The EEPROM Checksum Is Not Valid\n");
		err = -EIO;
		goto err_sw_init;
	}

	memcpy(netdev->dev_addr, hw->mac.perm_addr, netdev->addr_len);
#ifdef ETHTOOL_GPERMADDR
	memcpy(netdev->perm_addr, hw->mac.perm_addr, netdev->addr_len);
#endif

	if (!is_valid_ether_addr(netdev->dev_addr)) {
		e_dev_err("invalid MAC address\n");
		err = -EIO;
		goto err_sw_init;
	}

	/* Set hw->mac.addr to permanent MAC address */
	ether_addr_copy(hw->mac.addr, hw->mac.perm_addr);
	ixgbe_mac_set_default_filter(adapter);

	timer_setup(&adapter->service_timer, ixgbe_service_timer, 0);

	if (IXGBE_REMOVED(hw->hw_addr)) {
		err = -EIO;
		goto err_sw_init;
	}
	INIT_WORK(&adapter->service_task, ixgbe_service_task);
	set_bit(__IXGBE_SERVICE_INITED, &adapter->state);
	clear_bit(__IXGBE_SERVICE_SCHED, &adapter->state);

	err = ixgbe_init_interrupt_scheme(adapter);
	if (err)
		goto err_sw_init;

	for (i = 0; i < adapter->num_xdp_queues; i++)
		u64_stats_init(&adapter->xdp_ring[i]->syncp);
	/* WOL not supported for all devices */
	adapter->wol = 0;
	hw->eeprom.ops.read(hw, 0x2c, &adapter->eeprom_cap);
	if (ixgbe_wol_supported(adapter, pdev->device, pdev->subsystem_device))
		adapter->wol = IXGBE_WUFC_MAG;

	hw->wol_enabled = !!(adapter->wol);

	device_set_wakeup_enable(pci_dev_to_dev(adapter->pdev), adapter->wol);

	ixgbe_set_fw_version(adapter);

	/* reset the hardware with the new settings */
	err = hw->mac.ops.start_hw(hw);
	if (err == IXGBE_ERR_EEPROM_VERSION) {
		/* We are running on a pre-production device, log a warning */
		e_dev_warn("This device is a pre-production adapter/LOM. "
			   "Please be aware there may be issues associated "
			   "with your hardware.  If you are experiencing "
			   "problems please contact your Intel or hardware "
			   "representative who provided you with this "
			   "hardware.\n");
	} else if (err == IXGBE_ERR_OVERTEMP) {
		e_crit(drv, "%s\n", ixgbe_overheat_msg);
		goto err_register;
	} else if (err) {
		e_dev_err("HW init failed\n");
		goto err_register;
	}

	/* pick up the PCI bus settings for reporting later */
	if (ixgbe_pcie_from_parent(hw))
		ixgbe_get_parent_bus_info(hw);
	else
		if (hw->mac.ops.get_bus_info)
			hw->mac.ops.get_bus_info(hw);

	strscpy(netdev->name, "eth%d", sizeof(netdev->name));
	pci_set_drvdata(pdev, adapter);
	err = register_netdev(netdev);
	if (err)
		goto err_register;
	adapter->netdev_registered = true;

#ifdef HAVE_PCI_ERS
	/*
	 * call save state here in standalone driver because it relies on
	 * adapter struct to exist, and needs to call netdev_priv
	 */
	pci_save_state(pdev);

#endif

	/* power down the optics for 82599 SFP+ fiber */
	if (hw->mac.ops.disable_tx_laser)
		hw->mac.ops.disable_tx_laser(hw);

	/* carrier off reporting is important to ethtool even BEFORE open */
	netif_carrier_off(netdev);
	/* keep stopping all the transmit queues for older kernels */
	netif_tx_stop_all_queues(netdev);

#if IS_ENABLED(CONFIG_DCA)
	if (adapter->flags & IXGBE_FLAG_DCA_CAPABLE) {
		err = dca_add_requester(pci_dev_to_dev(pdev));
		switch (err) {
		case IXGBE_SUCCESS:
			adapter->flags |= IXGBE_FLAG_DCA_ENABLED;
			ixgbe_setup_dca(adapter);
			break;
		/* -19 is returned from the kernel when no provider is found */
		case -19:
			e_info(rx_err, "No DCA provider found. Please "
			       "start ioatdma for DCA functionality.\n");
			break;
		default:
			e_info(probe, "DCA registration failed: %d\n", err);
			break;
		}
	}
#endif

	/* print all messages at the end so that we use our eth%d name */
	ixgbe_check_minimum_link(adapter);

	/* First try to read PBA as a string */
	err = ixgbe_read_pba_string(hw, part_str, IXGBE_PBANUM_LENGTH);
	if (err)
		strscpy(part_str, "Unknown", IXGBE_PBANUM_LENGTH);
	if (ixgbe_is_sfp(hw) && hw->phy.sfp_type != ixgbe_sfp_type_not_present)
		e_info(probe, "MAC: %d, PHY: %d, SFP+: %d, PBA No: %s\n",
		       hw->mac.type, hw->phy.type, hw->phy.sfp_type, part_str);
	else
		e_info(probe, "MAC: %d, PHY: %d, PBA No: %s\n",
		      hw->mac.type, hw->phy.type, part_str);

	e_dev_info("%02x:%02x:%02x:%02x:%02x:%02x\n",
		   netdev->dev_addr[0], netdev->dev_addr[1],
		   netdev->dev_addr[2], netdev->dev_addr[3],
		   netdev->dev_addr[4], netdev->dev_addr[5]);

#define INFO_STRING_LEN 255
	info_string = kzalloc(INFO_STRING_LEN, GFP_KERNEL);
	if (!info_string) {
		e_err(probe, "allocation for info string failed\n");
		goto no_info_string;
	}
	snprintf(info_string, INFO_STRING_LEN,
		 "Enabled Features: RxQ: %d TxQ: %d",
		 adapter->num_rx_queues, adapter->num_tx_queues);
#if IS_ENABLED(CONFIG_FCOE)
	if (adapter->flags & IXGBE_FLAG_FCOE_ENABLED)
		strlcat(info_string, " FCoE", INFO_STRING_LEN);
#endif
	if (adapter->flags & IXGBE_FLAG_FDIR_HASH_CAPABLE)
		strlcat(info_string, " FdirHash", INFO_STRING_LEN);
	if (adapter->flags & IXGBE_FLAG_DCB_ENABLED)
		strlcat(info_string, " DCB", INFO_STRING_LEN);
	if (adapter->flags & IXGBE_FLAG_DCA_ENABLED)
		strlcat(info_string, " DCA", INFO_STRING_LEN);
	if (adapter->flags2 & IXGBE_FLAG2_RSC_ENABLED)
		strlcat(info_string, " RSC", INFO_STRING_LEN);
	if (adapter->flags & IXGBE_FLAG_VXLAN_OFFLOAD_ENABLE)
		strlcat(info_string, " vxlan_rx", INFO_STRING_LEN);

	/* end features printing */
	e_info(probe, "%s\n", info_string);
	kfree(info_string);
no_info_string:
#ifdef CONFIG_PCI_IOV
	if (adapter->flags & IXGBE_FLAG_SRIOV_ENABLED) {
		int i;
		for (i = 0; i < adapter->num_vfs; i++)
			ixgbe_vf_configuration(pdev, (i | 0x10000000));
	}
#endif

	/* Initialize the LED link active for LED blink support */
	if (hw->mac.ops.init_led_link_act)
		hw->mac.ops.init_led_link_act(hw);

	/* firmware requires blank numerical version */
	if (hw->mac.ops.set_fw_drv_ver)
		hw->mac.ops.set_fw_drv_ver(hw, 0xFF, 0xFF, 0xFF, 0xFF,
					   sizeof(ixgbe_driver_version) - 1,
					   ixgbe_driver_version);

#if defined(HAVE_NETDEV_STORAGE_ADDRESS) && defined(NETDEV_HW_ADDR_T_SAN)
	/* add san mac addr to netdev */
	ixgbe_add_sanmac_netdev(netdev);

#endif /* (HAVE_NETDEV_STORAGE_ADDRESS) && (NETDEV_HW_ADDR_T_SAN) */
	e_info(probe, "Intel(R) 10 Gigabit Network Connection\n");
	cards_found++;

#ifdef IXGBE_SYSFS
	if (ixgbe_sysfs_init(adapter))
		e_err(probe, "failed to allocate sysfs resources\n");
#else
#ifdef IXGBE_PROCFS
	if (ixgbe_procfs_init(adapter))
		e_err(probe, "failed to allocate procfs resources\n");
#endif /* IXGBE_PROCFS */
#endif /* IXGBE_SYSFS */
#ifdef HAVE_IXGBE_DEBUG_FS

	ixgbe_dbg_adapter_init(adapter);
#endif /* HAVE_IXGBE_DEBUG_FS */

	/* setup link for SFP devices with MNG FW, else wait for IXGBE_UP */
	if (ixgbe_mng_enabled(hw) && ixgbe_is_sfp(hw) && hw->mac.ops.setup_link)
		hw->mac.ops.setup_link(hw,
			IXGBE_LINK_SPEED_10GB_FULL | IXGBE_LINK_SPEED_1GB_FULL,
			true);

	if (hw->mac.ops.setup_eee &&
	    (adapter->flags2 & IXGBE_FLAG2_EEE_CAPABLE)) {
		bool eee_enable = !!(adapter->flags2 & IXGBE_FLAG2_EEE_ENABLED);

		hw->mac.ops.setup_eee(hw, eee_enable);
	}

	return 0;

err_register:
	ixgbe_clear_interrupt_scheme(adapter);
err_sw_init:
	ixgbe_release_hw_control(adapter);
#ifdef CONFIG_PCI_IOV
	ixgbe_disable_sriov(adapter);
#endif /* CONFIG_PCI_IOV */
	adapter->flags2 &= ~IXGBE_FLAG2_SEARCH_FOR_SFP;
#ifdef HAVE_TC_SETUP_CLSU32
	kfree(adapter->jump_tables[0]);
#endif
	kfree(adapter->mac_table);
	kfree(adapter->rss_key);
	iounmap(adapter->io_addr);
err_ioremap:
	disable_dev = !test_and_set_bit(__IXGBE_DISABLED, &adapter->state);
	free_netdev(netdev);
err_alloc_etherdev:
	pci_release_mem_regions(pdev);
err_pci_reg:
err_dma:
	if (!adapter || disable_dev)
		pci_disable_device(pdev);
	return err;
}

/**
 * ixgbe_remove - Device Removal Routine
 * @pdev: PCI device information struct
 *
 * ixgbe_remove is called by the PCI subsystem to alert the driver
 * that it should release a PCI device.  The could be caused by a
 * Hot-Plug event, or because the driver is going to be removed from
 * memory.
 **/
#ifdef HAVE_CONFIG_HOTPLUG
static void __devexit ixgbe_remove(struct pci_dev *pdev)
#else
static void ixgbe_remove(struct pci_dev *pdev)
#endif
{
	struct ixgbe_adapter *adapter = pci_get_drvdata(pdev);
	struct net_device *netdev;
	bool disable_dev;
#ifdef HAVE_TC_SETUP_CLSU32
	int i;
#endif

	/* if !adapter then we already cleaned up in probe */
	if (!adapter)
		return;

	netdev = adapter->netdev;
#ifdef HAVE_IXGBE_DEBUG_FS
	ixgbe_dbg_adapter_exit(adapter);

#endif /*HAVE_IXGBE_DEBUG_FS */
	set_bit(__IXGBE_REMOVING, &adapter->state);
	cancel_work_sync(&adapter->service_task);

#if IS_ENABLED(CONFIG_DCA)
	if (adapter->flags & IXGBE_FLAG_DCA_ENABLED) {
		adapter->flags &= ~IXGBE_FLAG_DCA_ENABLED;
		dca_remove_requester(pci_dev_to_dev(pdev));
		IXGBE_WRITE_REG(&adapter->hw, IXGBE_DCA_CTRL,
				IXGBE_DCA_CTRL_DCA_DISABLE);
	}
#endif /* CONFIG_DCA */

#ifdef IXGBE_SYSFS
	ixgbe_sysfs_exit(adapter);
#else
#ifdef IXGBE_PROCFS
	ixgbe_procfs_exit(adapter);
#endif /* IXGBE_PROCFS */
#endif /* IXGBE-SYSFS */

#if defined(HAVE_NETDEV_STORAGE_ADDRESS) && defined(NETDEV_HW_ADDR_T_SAN)
	/* remove the added san mac */
	ixgbe_del_sanmac_netdev(netdev);

#endif /* (HAVE_NETDEV_STORAGE_ADDRESS) && (NETDEV_HW_ADDR_T_SAN) */

#ifdef CONFIG_PCI_IOV
	ixgbe_disable_sriov(adapter);
#endif /* CONFIG_PCI_IOV */
	if (adapter->netdev_registered) {
		unregister_netdev(netdev);
		adapter->netdev_registered = false;
	}

#if IS_ENABLED(CONFIG_FCOE)
#ifndef HAVE_NETDEV_OPS_FCOE_ENABLE
	ixgbe_fcoe_ddp_disable(adapter);
#endif
#endif /* CONFIG_FCOE */
	ixgbe_clear_interrupt_scheme(adapter);
	ixgbe_release_hw_control(adapter);

#ifdef HAVE_DCBNL_IEEE
	kfree(adapter->ixgbe_ieee_pfc);
	kfree(adapter->ixgbe_ieee_ets);

#endif
	iounmap(adapter->io_addr);
	pci_release_mem_regions(pdev);
#ifdef HAVE_TC_SETUP_CLSU32
	for (i = 0; i < IXGBE_MAX_LINK_HANDLE; i++) {
		if (adapter->jump_tables[i]) {
			kfree(adapter->jump_tables[i]->input);
			kfree(adapter->jump_tables[i]->mask);
		}
		kfree(adapter->jump_tables[i]);
	}
#endif /* HAVE_TC_SETUP_CLSU32 */
	kfree(adapter->mac_table);
	kfree(adapter->rss_key);
	disable_dev = !test_and_set_bit(__IXGBE_DISABLED, &adapter->state);
	free_netdev(netdev);

	pci_disable_pcie_error_reporting(pdev);

	if (disable_dev)
		pci_disable_device(pdev);

}

static bool ixgbe_check_cfg_remove(struct ixgbe_hw *hw, struct pci_dev *pdev)
{
	u16 value;

	pci_read_config_word(pdev, PCI_VENDOR_ID, &value);
	if (value == IXGBE_FAILED_READ_CFG_WORD) {
		ixgbe_remove_adapter(hw);
		return true;
	}
	return false;
}

u16 ixgbe_read_pci_cfg_word(struct ixgbe_hw *hw, u32 reg)
{
	struct ixgbe_adapter *adapter = hw->back;
	u16 value;

	if (IXGBE_REMOVED(hw->hw_addr))
		return IXGBE_FAILED_READ_CFG_WORD;
	pci_read_config_word(adapter->pdev, reg, &value);
	if (value == IXGBE_FAILED_READ_CFG_WORD &&
	    ixgbe_check_cfg_remove(hw, adapter->pdev))
		return IXGBE_FAILED_READ_CFG_WORD;
	return value;
}

#ifdef HAVE_PCI_ERS
#ifdef CONFIG_PCI_IOV
static u32 ixgbe_read_pci_cfg_dword(struct ixgbe_hw *hw, u32 reg)
{
	struct ixgbe_adapter *adapter = hw->back;
	u32 value;

	if (IXGBE_REMOVED(hw->hw_addr))
		return IXGBE_FAILED_READ_CFG_DWORD;
	pci_read_config_dword(adapter->pdev, reg, &value);
	if (value == IXGBE_FAILED_READ_CFG_DWORD &&
	    ixgbe_check_cfg_remove(hw, adapter->pdev))
		return IXGBE_FAILED_READ_CFG_DWORD;
	return value;
}
#endif /* CONFIG_PCI_IOV */
#endif /* HAVE_PCI_ERS */

void ixgbe_write_pci_cfg_word(struct ixgbe_hw *hw, u32 reg, u16 value)
{
	struct ixgbe_adapter *adapter = hw->back;

	if (IXGBE_REMOVED(hw->hw_addr))
		return;
	pci_write_config_word(adapter->pdev, reg, value);
}

void ewarn(struct ixgbe_hw *hw, const char *st)
{
	struct ixgbe_adapter *adapter = hw->back;

	netif_warn(adapter, drv, adapter->netdev,  "%s", st);
}

#ifdef HAVE_PCI_ERS
/**
 * ixgbe_io_error_detected - called when PCI error is detected
 * @pdev: Pointer to PCI device
 * @state: The current pci connection state
 *
 * This function is called after a PCI bus error affecting
 * this device has been detected.
 */
static pci_ers_result_t ixgbe_io_error_detected(struct pci_dev *pdev,
						pci_channel_state_t state)
{
	struct ixgbe_adapter *adapter = pci_get_drvdata(pdev);
	struct net_device *netdev = adapter->netdev;

#ifdef CONFIG_PCI_IOV
	struct ixgbe_hw *hw = &adapter->hw;
	struct pci_dev *bdev, *vfdev;
	u32 dw0, dw1, dw2, dw3;
	int vf, pos;
	u16 req_id, pf_func;

	if (adapter->hw.mac.type == ixgbe_mac_82598EB ||
	    adapter->num_vfs == 0)
		goto skip_bad_vf_detection;

	bdev = pdev->bus->self;
	while (bdev && (pci_pcie_type(bdev) != PCI_EXP_TYPE_ROOT_PORT))
		bdev = bdev->bus->self;

	if (!bdev)
		goto skip_bad_vf_detection;

	pos = pci_find_ext_capability(bdev, PCI_EXT_CAP_ID_ERR);
	if (!pos)
		goto skip_bad_vf_detection;

	dw0 = ixgbe_read_pci_cfg_dword(hw, pos + PCI_ERR_HEADER_LOG);
	dw1 = ixgbe_read_pci_cfg_dword(hw, pos + PCI_ERR_HEADER_LOG + 4);
	dw2 = ixgbe_read_pci_cfg_dword(hw, pos + PCI_ERR_HEADER_LOG + 8);
	dw3 = ixgbe_read_pci_cfg_dword(hw, pos + PCI_ERR_HEADER_LOG + 12);
	if (IXGBE_REMOVED(hw->hw_addr))
		goto skip_bad_vf_detection;

	req_id = dw1 >> 16;
	/* On the 82599 if bit 7 of the requestor ID is set then it's a VF */
	if (!(req_id & 0x0080))
		goto skip_bad_vf_detection;

	pf_func = req_id & 0x01;
	if ((pf_func & 1) == (pdev->devfn & 1)) {
		unsigned int device_id;

		vf = (req_id & 0x7F) >> 1;
		e_dev_err("VF %d has caused a PCIe error\n", vf);
		e_dev_err("TLP: dw0: %8.8x\tdw1: %8.8x\tdw2: "
				"%8.8x\tdw3: %8.8x\n",
		dw0, dw1, dw2, dw3);
		switch (adapter->hw.mac.type) {
		case ixgbe_mac_82599EB:
			device_id = IXGBE_DEV_ID_82599_VF;
			break;
		case ixgbe_mac_X540:
			device_id = IXGBE_DEV_ID_X540_VF;
			break;
		case ixgbe_mac_X550:
			device_id = IXGBE_DEV_ID_X550_VF;
			break;
		case ixgbe_mac_X550EM_x:
			device_id = IXGBE_DEV_ID_X550EM_X_VF;
			break;
		case ixgbe_mac_X550EM_a:
			device_id = IXGBE_DEV_ID_X550EM_A_VF;
			break;
		default:
			device_id = 0;
			break;
		}

		/* Find the pci device of the offending VF */
		vfdev = pci_get_device(PCI_VENDOR_ID_INTEL, device_id, NULL);
		while (vfdev) {
			if (vfdev->devfn == (req_id & 0xFF))
				break;
			vfdev = pci_get_device(PCI_VENDOR_ID_INTEL,
					       device_id, vfdev);
		}
		/*
		 * There's a slim chance the VF could have been hot plugged,
		 * so if it is no longer present we don't need to issue the
		 * VFLR.  Just clean up the AER in that case.
		 */
		if (vfdev) {
			ixgbe_issue_vf_flr(adapter, vfdev);
			/* Free device reference count */
			pci_dev_put(vfdev);
		}

		pci_aer_clear_nonfatal_status(pdev);
	}

	/*
	 * Even though the error may have occurred on the other port
	 * we still need to increment the vf error reference count for
	 * both ports because the I/O resume function will be called
	 * for both of them.
	 */
	adapter->vferr_refcount++;

	return PCI_ERS_RESULT_RECOVERED;

skip_bad_vf_detection:
#endif /* CONFIG_PCI_IOV */
	if (!test_bit(__IXGBE_SERVICE_INITED, &adapter->state))
		return PCI_ERS_RESULT_DISCONNECT;

	if (!netif_device_present(netdev))
		return PCI_ERS_RESULT_DISCONNECT;

	rtnl_lock();
	netif_device_detach(netdev);

	if (netif_running(netdev))
		ixgbe_close_suspend(adapter);

	if (state == pci_channel_io_perm_failure) {
		rtnl_unlock();
		return PCI_ERS_RESULT_DISCONNECT;
	}

	if (!test_and_set_bit(__IXGBE_DISABLED, &adapter->state))
		pci_disable_device(pdev);
	rtnl_unlock();

	/* Request a slot reset. */
	return PCI_ERS_RESULT_NEED_RESET;
}

/**
 * ixgbe_io_slot_reset - called after the pci bus has been reset.
 * @pdev: Pointer to PCI device
 *
 * Restart the card from scratch, as if from a cold-boot.
 */
static pci_ers_result_t ixgbe_io_slot_reset(struct pci_dev *pdev)
{
	struct ixgbe_adapter *adapter = pci_get_drvdata(pdev);
	pci_ers_result_t result;

	if (pci_enable_device_mem(pdev)) {
		e_err(probe, "Cannot re-enable PCI device after reset.\n");
		result = PCI_ERS_RESULT_DISCONNECT;
	} else {
		smp_mb__before_atomic();
		clear_bit(__IXGBE_DISABLED, &adapter->state);
		adapter->hw.hw_addr = adapter->io_addr;
		pci_set_master(pdev);
		pci_restore_state(pdev);
		/*
		 * After second error pci->state_saved is false, this
		 * resets it so EEH doesn't break.
		 */
		pci_save_state(pdev);

		pci_wake_from_d3(pdev, false);

		ixgbe_reset(adapter);
		IXGBE_WRITE_REG(&adapter->hw, IXGBE_WUS, ~0);
		result = PCI_ERS_RESULT_RECOVERED;
	}

	pci_aer_clear_nonfatal_status(pdev);

	return result;
}

/**
 * ixgbe_io_resume - called when traffic can start flowing again.
 * @pdev: Pointer to PCI device
 *
 * This callback is called when the error recovery driver tells us that
 * its OK to resume normal operation.
 */
static void ixgbe_io_resume(struct pci_dev *pdev)
{
	struct ixgbe_adapter *adapter = pci_get_drvdata(pdev);
	struct net_device *netdev = adapter->netdev;

#ifdef CONFIG_PCI_IOV
	if (adapter->vferr_refcount) {
		e_info(drv, "Resuming after VF err\n");
		adapter->vferr_refcount--;
		return;
	}

#endif
	rtnl_lock();
	if (netif_running(netdev))
		ixgbe_open(netdev);

	netif_device_attach(netdev);
	rtnl_unlock();
}

#ifdef HAVE_CONST_STRUCT_PCI_ERROR_HANDLERS
static const struct pci_error_handlers ixgbe_err_handler = {
#else
static struct pci_error_handlers ixgbe_err_handler = {
#endif
	.error_detected = ixgbe_io_error_detected,
	.slot_reset = ixgbe_io_slot_reset,
	.resume = ixgbe_io_resume,
};
#endif /* HAVE_PCI_ERS */

struct net_device *ixgbe_hw_to_netdev(const struct ixgbe_hw *hw)
{
	return ((struct ixgbe_adapter *)hw->back)->netdev;
}
struct ixgbe_msg *ixgbe_hw_to_msg(const struct ixgbe_hw *hw)
{
	struct ixgbe_adapter *adapter =
		container_of(hw, struct ixgbe_adapter, hw);
	return (struct ixgbe_msg *)&adapter->msg_enable;
}

#ifdef HAVE_RHEL6_SRIOV_CONFIGURE
static struct pci_driver_rh ixgbe_driver_rh = {
	.sriov_configure = ixgbe_pci_sriov_configure,
};
#endif

#ifdef CONFIG_PM
#ifndef USE_LEGACY_PM_SUPPORT
static const struct dev_pm_ops ixgbe_pm_ops = {
	.suspend        = ixgbe_suspend,
	.resume         = ixgbe_resume,
	.freeze         = ixgbe_freeze,
	.thaw           = ixgbe_thaw,
	.poweroff       = ixgbe_suspend,
	.restore        = ixgbe_resume,
};
#endif /* USE_LEGACY_PM_SUPPORT */
#endif

static struct pci_driver ixgbe_driver = {
	.name     = ixgbe_driver_name,
	.id_table = ixgbe_pci_tbl,
	.probe    = ixgbe_probe,
#ifdef HAVE_CONFIG_HOTPLUG
	.remove   = __devexit_p(ixgbe_remove),
#else
	.remove   = ixgbe_remove,
#endif
#ifdef CONFIG_PM
#ifndef USE_LEGACY_PM_SUPPORT
	.driver = {
		.pm = &ixgbe_pm_ops,
	},
#else
	.suspend  = ixgbe_suspend,
	.resume   = ixgbe_resume,
#endif /* USE_LEGACY_PM_SUPPORT */
#endif
#ifndef USE_REBOOT_NOTIFIER
	.shutdown = ixgbe_shutdown,
#endif
#if defined(HAVE_SRIOV_CONFIGURE)
	.sriov_configure = ixgbe_pci_sriov_configure,
#elif defined(HAVE_RHEL6_SRIOV_CONFIGURE)
	.rh_reserved = &ixgbe_driver_rh,
#endif /* HAVE_SRIOV_CONFIGURE */
#ifdef HAVE_PCI_ERS
	.err_handler = &ixgbe_err_handler
#endif
};

bool ixgbe_is_ixgbe(struct pci_dev *pcidev)
{
	if (pci_dev_driver(pcidev) != &ixgbe_driver)
		return false;
	else
		return true;
}

/**
 * ixgbe_init_module - Driver Registration Routine
 *
 * ixgbe_init_module is the first routine called when the driver is
 * loaded. All it does is register with the PCI subsystem.
 **/
static int __init ixgbe_init_module(void)
{
	int ret;
	pr_info("%s - version %s\n", ixgbe_driver_string, ixgbe_driver_version);
	pr_info("%s\n", ixgbe_copyright);

	ixgbe_wq = create_singlethread_workqueue(ixgbe_driver_name);
	if (!ixgbe_wq) {
		pr_err("%s: Failed to create workqueue\n", ixgbe_driver_name);
		return -ENOMEM;
	}

#ifdef IXGBE_PROCFS
	if (ixgbe_procfs_topdir_init())
		pr_info("Procfs failed to initialize topdir\n");
#endif

#ifdef HAVE_IXGBE_DEBUG_FS
	ixgbe_dbg_init();
#endif /* HAVE_IXGBE_DEBUG_FS */

	ret = pci_register_driver(&ixgbe_driver);
	if (ret) {
		destroy_workqueue(ixgbe_wq);
#ifdef HAVE_IXGBE_DEBUG_FS
		ixgbe_dbg_exit();
#endif /* HAVE_IXGBE_DEBUG_FS */
#ifdef IXGBE_PROCFS
		ixgbe_procfs_topdir_exit();
#endif
		return ret;
}
#if IS_ENABLED(CONFIG_DCA)

	dca_register_notify(&dca_notifier);
#endif

	return ret;
}

module_init(ixgbe_init_module);

/**
 * ixgbe_exit_module - Driver Exit Cleanup Routine
 *
 * ixgbe_exit_module is called just before the driver is removed
 * from memory.
 **/
static void __exit ixgbe_exit_module(void)
{
#if IS_ENABLED(CONFIG_DCA)
	dca_unregister_notify(&dca_notifier);
#endif
	pci_unregister_driver(&ixgbe_driver);
#ifdef IXGBE_PROCFS
	ixgbe_procfs_topdir_exit();
#endif
	destroy_workqueue(ixgbe_wq);
#ifdef HAVE_IXGBE_DEBUG_FS
	ixgbe_dbg_exit();
#endif /* HAVE_IXGBE_DEBUG_FS */
}

#if IS_ENABLED(CONFIG_DCA)
static int ixgbe_notify_dca(struct notifier_block __always_unused *nb, unsigned long event,
			    void __always_unused *p)
{
	int ret_val;

	ret_val = driver_for_each_device(&ixgbe_driver.driver, NULL, &event,
					 __ixgbe_notify_dca);

	return ret_val ? NOTIFY_BAD : NOTIFY_DONE;
}
#endif
module_exit(ixgbe_exit_module);

/* ixgbe_main.c */

