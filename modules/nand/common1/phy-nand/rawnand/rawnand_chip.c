/* SPDX-License-Identifier: GPL-2.0 */
/**
 * rawnand_chip.c
 *
 * Copyright (C) 2019 Allwinner.
 *
 * 2019.9.11 cuizhikui<cuizhikui@allwinnertech.com>
 * from
 *			eNand
 *		nand flash driver scan module
 *	 Copyright(C),2008-2009, SoftWinners Microelectronic Co.,Ltd.
 *			All Rigths Reserved
 * File name: nand_chip_function.c/nand_chip_common.c/nand_chip_interface.c
 *	nand_super_chip_function.c/nand_super_chip_common.c/
 *	nand_super_chip_interface.c
 * Author   :
 * Version  :
 * Date     : 2013-11-20
 * Description :
 * Others   : None at present
 * */


#include "../../nfd/nand_osal_for_linux.h"
#include "../nand_errno.h"
#include "controller/ndfc_base.h"
#include "rawnand.h"
#include "rawnand_base.h"
#include "rawnand_cfg.h"
#include "rawnand_debug.h"
#include "rawnand_ids.h"
#include "rawnand_ops.h"
#include "rawnand_readretry.h"
#include "controller/ndfc_base.h"
#include "controller/ndfc_ops.h"
#include "rawnand_boot.h"
#include <linux/sunxi-boot.h>

struct _nand_storage_info *g_nsi;
struct _nand_storage_info g_nsi_data;
rawnand_storage_info_t *g_nand_storage_info;
rawnand_storage_info_t g_nand_storage_info_data;
struct nand_chip_info nci_data[MAX_CHIP_PER_CHANNEL * 2];

struct _nand_super_storage_info *g_nssi;
struct _nand_super_storage_info g_nssi_data = {0};
struct nand_super_chip_info nsci_data[MAX_CHIP_PER_CHANNEL * 2] = {0};

 /*
  **Name         : nci_add_to_nsi
  **Description  :
  **Parameter    :
  **Return       : 0:ok  -1:fail
  **Note         :
  */
int nci_add_to_nsi(struct _nand_storage_info *nsi, struct nand_chip_info *node)
{
	struct nand_chip_info *nci;

	node->chip_no = 0;
	if (nsi->nci == NULL) {
		nsi->nci = node;
		return NAND_OP_TRUE;
	}

	nci = nsi->nci;
	node->chip_no = 1;
	while (nci->nsi_next != NULL) {
		nci = nci->nsi_next;
		node->chip_no++;
	}
	nci->nsi_next = node;

	return NAND_OP_TRUE;
}
/**
 * nci_delete_from_nsi:
 */
void nci_delete_from_nsi(struct _nand_storage_info *nsi)
{
	struct nand_chip_info *tail = NULL;
	struct nand_chip_info *next = NULL;

		RAWNAND_DBG("%s %d nsi->nci:%p\n", __func__, __LINE__, nsi->nci);
	for (tail = nsi->nci; tail != NULL; tail = next) {
		next = tail->nsi_next;
		nand_free(tail);
	}
	nsi->nci = NULL;
}
void delete_nsi(void)
{
	/*nci delete in nctri*/
	/*nci_delete_from_nsi(g_nsi);*/
	g_nsi = NULL;
}
 /*
  **Name         :
  **Description  :
  **Parameter    :
  **Return       : 0:ok  -1:fail
  **Note         :
  */
int nsci_add_to_nssi(struct _nand_super_storage_info *nssi, struct nand_super_chip_info *node)
{
	struct nand_super_chip_info *nsci;

	if (nssi == NULL) {
		RAWNAND_ERR("%s nssi is null\n", __func__);
		return ERR_NO_12;
	}
	node->chip_no = 0;
	if (nssi->nsci == NULL) {
		nssi->nsci = node;
		return NAND_OP_TRUE;
	}

	node->chip_no = 1;
	nsci = nssi->nsci;
	while (nsci->nssi_next != NULL) {
		nsci = nsci->nssi_next;
		node->chip_no++;
	}
	nsci->nssi_next = node;
	return NAND_OP_TRUE;
}

int nsci_delete_from_nssi(struct nand_super_chip_info *node)
{
	struct nand_super_chip_info *tail = NULL;
	struct nand_super_chip_info *next = NULL;
	struct nand_super_chip_info *prev = NULL;

	if (g_nssi == NULL) {
		RAWNAND_ERR("%s g_nssi is null\n", __func__);
		return ERR_NO_12;
	}
	node->chip_no = 0;
	if (node == g_nssi->nsci) {
		g_nssi->nsci = g_nssi->nsci->nssi_next;
		return NAND_OP_TRUE;
	}
	for (tail = g_nssi->nsci; tail != NULL; tail = next) {
		next = tail->nssi_next;
		if (node == tail) {
			prev->nssi_next = tail->nssi_next;
			tail->chip_no = 0;
		}
		prev = tail;
	}
	return NAND_OP_TRUE;
}
int delete_nssi(void)
{
	struct nand_super_chip_info *tail = NULL;
	struct nand_super_chip_info *next = NULL;
	if (g_nssi == NULL) {
		RAWNAND_ERR("%s g_nssi is null\n", __func__);
		return ERR_NO_12;
	}
	for (tail = g_nssi->nsci; tail != NULL; tail = next) {
		next = tail->nssi_next;
		tail->chip_no = 0;
		g_nssi->nsci = next;
	}
	g_nssi = NULL;
	return NAND_OP_TRUE;
}

 /*
  **Name         :
  **Description  :
  **Parameter    :
  **Return       : 0:ok  -1:fail
  **Note         :
  */
int nci_add_to_nctri(struct nand_controller_info *nctri, struct nand_chip_info *node)
{
	struct nand_chip_info *nci;

	node->nctri_chip_no = 0;
	if (nctri->nci == NULL) {
		nctri->nci = node;
		return NAND_OP_TRUE;
	}

	nci = nctri->nci;
	node->nctri_chip_no = 1;
	while (nci->nctri_next != NULL) {
		nci = nci->nctri_next;
		node->nctri_chip_no++;
	}
	nci->nctri_next = node;

	return NAND_OP_TRUE;
}
/**
 * free nci in nctri list
 * */
void nci_delete_from_nctri(struct nand_controller_info *nctri)
{
	struct nand_chip_info *tail = NULL;
	struct nand_chip_info *next = NULL;

	for (tail = nctri->nci; tail != NULL; tail = next) {
		next = tail->nctri_next;
		nand_free(tail);
	}
	nctri->nci = NULL;
}
/*
 *Name         :
 *Description  :
 *Parameter    :
 *Return       : 0:ok  -1:fail
 *Note         :
 */
struct nand_chip_info *nci_get_from_nctri(struct nand_controller_info *nctri, unsigned int num)
{
	int i;
	struct nand_chip_info *nci;

	for (i = 0, nci = nctri->nci; i < num; i++)
		nci = nci->nctri_next;

	return nci;
}

/*
 *Name         :
 *Description  :
 *Parameter    :
 *Return       : 0:ok  -1:fail
 *Note         :
 */
struct nand_chip_info *nci_get_from_nsi(struct _nand_storage_info *nsi, unsigned int num)
{
	int i;
	struct nand_chip_info *nci;

	for (i = 0, nci = nsi->nci; i < num; i++)
		nci = nci->nsi_next;

	return nci;
}

/*
 *Name         :
 *Description  :
 *Parameter    :
 *Return       : 0:ok  -1:fail
 *Note         :
 */
struct nand_super_chip_info *nsci_get_from_nssi(struct _nand_super_storage_info *nssi, unsigned int num)
{
	int i;
	struct nand_super_chip_info *nsci;

	for (i = 0, nsci = nssi->nsci; i < num; i++)
		nsci = nsci->nssi_next;

	return nsci;
}

/*
 *Name         :
 *Description  :
 *Parameter    :
 *Return       : 0:ok  -1:fail
 *Note         :
 */
int init_nci_from_id(struct nand_chip_info *nci, struct sunxi_nand_flash_device *npi)
{

	memcpy(nci->id, npi->id, 8);

	nci->npi = npi;
	nci->opt_phy_op_par = &phy_op_para[npi->cmd_set_no];
	nci->nfc_init_ddr_info = &def_ddr_info[npi->ddr_info_no];

	nci->blk_cnt_per_chip = npi->die_cnt_per_chip * npi->blk_cnt_per_die;
	nci->sector_cnt_per_page = npi->sect_cnt_per_page;
	nci->page_cnt_per_blk = npi->page_cnt_per_blk;
	nci->page_offset_for_next_blk = npi->page_cnt_per_blk;
	nci->ecc_mode = npi->ecc_mode;
	nci->max_erase_times = npi->max_blk_erase_times;
	//	nci->driver_no = npi->driver_no;
	nci->randomizer = (npi->operation_opt & NAND_RANDOM) ? 1 : 0;
	nci->lsb_page_type = ((npi->operation_opt) & NAND_LSB_PAGE_TYPE) >> 12;
	nci->multi_plane_block_offset = npi->multi_plane_block_offset;
	nci->bad_block_flag_position = npi->bad_block_flag_position;
	nci->random_cmd2_send_flag = npi->random_cmd2_send_flag;
	nci->random_addr_num = npi->random_addr_num;
	nci->nand_real_page_size = npi->nand_real_page_size;
	/* nand interface */
	nci->interface_type = npi->ddr_type; //0x0: sdr; 0x2: nvddr; 0x3: tgddr; 0x12: nvddr2; 0x13: tgddr2;
	if (g_phy_cfg->phy_interface_cfg == 1) {
		if ((nci->interface_type == 0x3) || (nci->interface_type == 0x13)) {
			nci->interface_type = 0x03;
		} else if ((nci->interface_type == 0x2) || (nci->interface_type == 0x12)) {
			nci->interface_type = 0x02;
		} else {
			nci->interface_type = 0x00;
		}
	}

	/*
	 *if (npi->operation_opt & NAND_PAIRED_PAGE_SYNC)
	 *        NAND_Check_3DNand();
	 */

	if (npi->ddr_opt & NAND_VCCQ_1P8V) {
		nand_vccq_1p8v_enable();
	}
	nci->frequency = npi->access_freq;
#ifdef FPGA_PLATFORM
	nci->frequency = 6;
#endif

	nci->timing_mode = 0x0;
	nci->itf_cfg.onfi_cfg.support_change_onfi_timing_mode =
		(npi->ddr_opt & NAND_ONFI_TIMING_MODE) ? 1 : 0;
	nci->itf_cfg.onfi_cfg.support_ddr2_specific_cfg =
		(npi->ddr_opt & NAND_ONFI_DDR2_CFG) ? 1 : 0;
	nci->itf_cfg.onfi_cfg.support_io_driver_strength =
		(npi->ddr_opt & NAND_ONFI_IO_DRIVER_STRENGTH) ? 1 : 0;
	nci->itf_cfg.onfi_cfg.support_rb_pull_down_strength =
		(npi->ddr_opt & NAND_ONFI_RB_STRENGTH) ? 1 : 0;

	nci->support_toggle_only = (npi->operation_opt & NAND_TOGGLE_ONLY) ? 1 : 0;

	nci->itf_cfg.toggle_cfg.support_specific_setting =
		(npi->ddr_opt & NAND_TOGGLE_SPECIFIC_CFG) ? 1 : 0;
	nci->itf_cfg.toggle_cfg.support_io_driver_strength_setting =
		(npi->ddr_opt & NAND_TOGGLE_IO_DRIVER_STRENGTH) ? 1 : 0;
	nci->itf_cfg.toggle_cfg.support_vendor_specific_setting =
		(npi->operation_opt & NAND_TOGGLE_VENDOR_SPECIFIC_CFG) ? 1 : 0;

	nci->ecc_sector = 1;
	nci->sdata_bytes_per_page = (nci->sector_cnt_per_page >> nci->ecc_sector) * 4;
	if (nci->sdata_bytes_per_page > 16)
		nci->sdata_bytes_per_page = 16;

	nci->nand_physic_erase_block = rawnand_ops.erase_single_block;
	nci->nand_physic_read_page = rawnand_ops.read_single_page;
	nci->nand_physic_write_page = rawnand_ops.write_single_page;
	nci->nand_physic_bad_block_check = rawnand_ops.single_bad_block_check;
	nci->nand_physic_bad_block_mark = rawnand_ops.single_bad_block_mark;
	nci->nand_read_boot0_page = rawnand_boot0_ops.read_boot0_page;
	nci->nand_write_boot0_page = rawnand_boot0_ops.write_boot0_page;
	nci->nand_read_boot0_one = rawnand_boot0_ops.read_boot0_one;
	if (rawnand_boot0_ops.write_boot0_one == NULL) {
		selected_write_boot0_one(nci->npi->selected_write_boot0_no);
	}
	nci->nand_write_boot0_one = rawnand_boot0_ops.write_boot0_one;

	nci->is_lsb_page = chose_lsb_func(((nci->npi->operation_opt >> LSB_PAGE_POS) & 0xff));

	/*according to id table, update the interface setting bind*/
	rawnand_update_timings_ift_ops(nci->npi->mfr_id);

	return NAND_OP_TRUE;
}


/*
 *Name         :
 *Description  :
 *Parameter    :
 *Return       : 0:ok  -1:fail
 *Note         :
 */
int init_nsci_from_nctri(struct _nand_super_storage_info *nssi, struct nand_super_chip_info *nsci, struct nand_controller_info *nctri, unsigned int channel_num, unsigned int chip_no, unsigned int nsci_num_in_nctri)
{
	int rb1, rb0;
	struct nand_chip_info *nci;

	if (chip_no >= nsci_num_in_nctri) {
		chip_no -= nsci_num_in_nctri;
		nctri = nctri->next;
	}

	if (channel_num == 1) {
		nsci->channel_num = 1;
		nsci->dual_channel = 0;
		nssi->support_dual_channel = 0;

		nci = nci_get_from_nctri(nctri, chip_no);
		nsci->nci_first = nci;
		nci->nsci = nsci;
	} else {
		/*must be channel_num == 2*/
		nsci->channel_num = 2;

		nci = nci_get_from_nctri(nctri, chip_no);
		nsci->nci_first = nci;
		nci->nsci = nsci;

		if (nssi->support_dual_channel != 0) {
			nsci->dual_channel = 1;
			nci = nci_get_from_nctri(nctri, chip_no);
			nsci->d_channel_nci_1 = nci;
			nci->nsci = nsci;

			nci = nci_get_from_nctri(nctri->next, chip_no);
			nsci->d_channel_nci_2 = nci;
			nci->nsci = nsci;
		}
	}

	if (nssi->support_v_interleave != 0) {
		nsci->vertical_interleave = 1;

		if (nctri->chip_cnt == 2) {
			nci = nci_get_from_nctri(nctri, chip_no);
			nsci->v_intl_nci_1 = nci;
			nci->nsci = nsci;
			nci = nci_get_from_nctri(nctri, chip_no + 1);
			nsci->v_intl_nci_2 = nci;
			nci->nsci = nsci;
		} else {
			/*must be  nctri->chip_cnt == 4*/
			rb0 = nctri->rb[0];
			rb1 = nctri->rb[1];
			//            rb2 = nctri->rb[2];
			//            rb3 = nctri->rb[3];
			if (rb0 == rb1) {
				/*rb0 == rb1*/
				nci = nci_get_from_nctri(nctri, chip_no);
				nsci->v_intl_nci_1 = nci;
				nci->nsci = nsci;
				nci = nci_get_from_nctri(nctri, chip_no + 2);
				nsci->v_intl_nci_2 = nci;
				nci->nsci = nsci;
			} else {
				 /*must be rb0 == rb2*/
				nci = nci_get_from_nctri(nctri, (chip_no << 1));
				nsci->v_intl_nci_1 = nci;
				nci->nsci = nsci;
				nci = nci_get_from_nctri(nctri, (chip_no << 1) + 1);
				nsci->v_intl_nci_2 = nci;
				nci->nsci = nsci;
			}
		}
	}

	nsci->driver_no = nsci->nci_first->driver_no;
	nsci->blk_cnt_per_super_chip = nctri->nci->blk_cnt_per_chip;
	nsci->sector_cnt_per_super_page = nctri->nci->sector_cnt_per_page;
	nsci->page_cnt_per_super_blk = nctri->nci->page_cnt_per_blk;
	nsci->page_offset_for_next_super_blk = nctri->nci->page_offset_for_next_blk;

	if (nsci->dual_channel == 1) {
		nsci->sector_cnt_per_super_page <<= 1;
	}
	if ((nssi->support_two_plane != 0) && (nsci->nci_first->npi->operation_opt & NAND_MULTI_PROGRAM) && (nsci->nci_first->multi_plane_block_offset == 1)) {
		nsci->two_plane = 1;
	} else {
		nsci->two_plane = 0;
	}

	if (nsci->sector_cnt_per_super_page > MAX_SECTORS_PER_PAGE_FOR_TWO_PLANE) {
		nsci->two_plane = 0;
	}

	if (nctri->nci->sector_cnt_per_page == 4) {
		nsci->two_plane = 1;
	}

	if (nsci->two_plane == 1) {
		nsci->blk_cnt_per_super_chip >>= 1;
		nsci->sector_cnt_per_super_page <<= 1;
	} else {
		nssi->support_two_plane = 0;
	}

	if (nssi->support_two_plane == 0) {
		nssi->plane_cnt = 1;
	} else {
		nssi->plane_cnt = 2;
	}

	if (nsci->vertical_interleave == 1) {
		nsci->page_cnt_per_super_blk <<= 1;
		nsci->page_offset_for_next_super_blk <<= 1;
	}

	nsci->spare_bytes = 16;
	nsci->nand_physic_erase_super_block = rawnand_ops.erase_super_block;
	nsci->nand_physic_read_super_page = rawnand_ops.read_super_page;
	nsci->nand_physic_write_super_page = rawnand_ops.write_super_page;
	nsci->nand_physic_super_bad_block_check = rawnand_ops.super_bad_block_check;
	nsci->nand_physic_super_bad_block_mark = rawnand_ops.super_bad_block_mark;

	return NAND_OP_TRUE;
}

/*
 *Name         :
 *Description  :
 *Parameter    :
 *Return       : 0:ok  -1:fail
 *Note         :
 */
void nand_enable_chip(struct nand_chip_info *nci)
{
	struct nand_controller_info *nctri = nci->nctri;
	unsigned int chip_no = nci->nctri_chip_no;

	ndfc_select_chip(nctri, nctri->ce[chip_no]);
	ndfc_select_rb(nctri, nctri->rb[chip_no]);
}

/*
 *Name         :
 *Description  :
 *Parameter    :
 *Return       : 0:ok  -1:fail
 *Note         :
 */
void nand_disable_chip(struct nand_chip_info *nci)
{
	// select invalid CE signal, disable all chips
	ndfc_select_chip(nci->nctri, 0xf);
	// select invalid RB signal, no any more RB busy to ready interrupt
	ndfc_select_rb(nci->nctri, 0x3);
	return;
}

/*
 *Name         :
 *Description  :
 *Parameter    :
 *Return       : 0:ok  -1:fail
 *Note         :
 */
int nand_reset_chip(struct nand_chip_info *nci)
{
	int ret = 0;
	struct _nctri_cmd_seq *cmd_seq = &nci->nctri->nctri_cmd_seq;

	nand_enable_chip(nci);

	ndfc_clean_cmd_seq(cmd_seq);
	cmd_seq->cmd_type = CMD_TYPE_NORMAL;
	cmd_seq->nctri_cmd[0].cmd_valid = 1;
	cmd_seq->nctri_cmd[0].cmd = CMD_RESET; //??? don't support onfi ddr interface
	cmd_seq->nctri_cmd[0].cmd_send = 1;
	cmd_seq->nctri_cmd[0].cmd_wait_rb = 1;
	ret = ndfc_execute_cmd(nci->nctri, cmd_seq);
	if (ret) {
		RAWNAND_ERR("nand_reset_chip, reset failed!\n");
	}

	nand_disable_chip(nci);

	return ret;
}

/*
 *Name         :
 *Description  :
 *Parameter    :
 *Return       : 0:ok  -1:fail
 *Note         :
 */
int nand_sync_reset_chip(struct nand_chip_info *nci)
{
	int ret = 0;
	struct _nctri_cmd_seq *cmd_seq = &nci->nctri->nctri_cmd_seq;

	nand_enable_chip(nci);

	ndfc_clean_cmd_seq(cmd_seq);
	cmd_seq->cmd_type = CMD_TYPE_NORMAL;
	cmd_seq->nctri_cmd[0].cmd_valid = 1;
	cmd_seq->nctri_cmd[0].cmd = CMD_SYNC_RESET; //support onfi ddr interface
	cmd_seq->nctri_cmd[0].cmd_send = 1;
	cmd_seq->nctri_cmd[0].cmd_wait_rb = 1;
	ret = ndfc_execute_cmd(nci->nctri, cmd_seq);
	if (ret) {
		RAWNAND_ERR("nand_reset_chip, reset failed!\n");
	}

	nand_disable_chip(nci);

	return ret;
}

/*
 *Name         :
 *Description  :
 *Parameter    :
 *Return       : 0:ok  -1:fail
 *Note         :
 */
int nand_first_reset_chip(struct nand_chip_info *nci, unsigned int chip_no)
{
	int ret = 0;
	struct _nctri_cmd_seq *cmd_seq = &nci->nctri->nctri_cmd_seq;

	// enable chip, don't select rb signal
	ndfc_select_chip(nci->nctri, chip_no);

	ndfc_clean_cmd_seq(cmd_seq);
	cmd_seq->cmd_type = CMD_TYPE_NORMAL;
	cmd_seq->nctri_cmd[0].cmd_valid = 1;
	cmd_seq->nctri_cmd[0].cmd = CMD_RESET; //??? don't support onfi ddr interface
	cmd_seq->nctri_cmd[0].cmd_send = 1;
	cmd_seq->nctri_cmd[0].cmd_wait_rb = 0;
	ret = ndfc_execute_cmd(nci->nctri, cmd_seq);
	if (ret) {
		RAWNAND_ERR("nand_first_reset_chip, reset failed!\n");
		goto ERROR;
	}

	// wait all rb ready, because we don't know which rb sigal connect to current chip
	ret = ndfc_wait_all_rb_ready(nci->nctri);
	if (ret) {
		RAWNAND_ERR("nand_first_reset_chip, ndfc_wait_all_rb_ready timeout\n");
		goto ERROR;
	}

	nand_disable_chip(nci);
	return NAND_OP_TRUE;

ERROR:
	nand_disable_chip(nci);
	return NAND_OP_FALSE;
}

/*
 *Name         :
 *Description  :
 *Parameter    :
 *Return       : 0:ok  -1:fail
 *Note         :
 */
int nand_read_chip_status(struct nand_chip_info *nci, u8 *pstatus)
{
	int ret = 0;
	struct _nctri_cmd_seq *cmd_seq = &nci->nctri->nctri_cmd_seq;

	nand_enable_chip(nci);
	ndfc_repeat_mode_enable(nci->nctri);

	ndfc_clean_cmd_seq(cmd_seq);

	cmd_seq->cmd_type = CMD_TYPE_NORMAL;
	cmd_seq->nctri_cmd[0].cmd_valid = 1;
	cmd_seq->nctri_cmd[0].cmd = CMD_READ_STA;
	cmd_seq->nctri_cmd[0].cmd_send = 1;

	cmd_seq->nctri_cmd[0].cmd_trans_data_nand_bus = 1;
	cmd_seq->nctri_cmd[0].cmd_swap_data = 1;
	cmd_seq->nctri_cmd[0].cmd_swap_data_dma = 0;
	cmd_seq->nctri_cmd[0].cmd_direction = 0; //read
	cmd_seq->nctri_cmd[0].cmd_mdata_len = 1;
	cmd_seq->nctri_cmd[0].cmd_mdata_addr = pstatus;

	ret = ndfc_execute_cmd(nci->nctri, cmd_seq);
	if (ret) {
		RAWNAND_ERR("nand_reset_chip, read chip status failed!\n");
	}

	ndfc_repeat_mode_disable(nci->nctri);
	nand_disable_chip(nci);
	return ret;
}

/*
 *Name         :
 *Description  :
 *Parameter    :
 *Return       : 0:ok  -1:fail
 *Note         :
 */
int nand_read_chip_status_ready(struct nand_chip_info *nci)
{
	int ret;
	int timeout = 0xffff;
	u8 status;

	while (1) {
		ret = nand_read_chip_status(nci, &status);
		if (ret) {
			RAWNAND_ERR("read chip status failed!\n");
			ret = -NAND_OP_FALSE;
			break;
		}

		if (status & NAND_STATUS_READY)
			break;

		if (timeout-- < 0) {
			RAWNAND_ERR("wait nand status ready timeout,chip=%x, status=%x\n", nci->chip_no, status);
			ret = ERR_TIMEOUT;
			break;
		}
	}

	if (status & NAND_OPERATE_FAIL) {
		RAWNAND_ERR("read chip status failed %x %x!\n", nci->chip_no, status);
		ret = -NAND_OP_FALSE;
		//ret = 0;
	}

	return ret;
}

/*
 *Name         :
 *Description  :
 *Parameter    :
 *Return       : 0:ok  -1:fail
 *Note         :
 */
int is_chip_rb_ready(struct nand_chip_info *nci)
{
	struct nand_controller_info *nctri = nci->nctri;
	unsigned int chip_no = nci->chip_no;

	return ndfc_get_rb_sta(nctri, nctri->rb[chip_no]);
}

/*
 *Name         :
 *Description  :
 *Parameter    :
 *Return       : 0:ok  -1:fail
 *Note         :
 */
int nand_read_id(struct nand_chip_info *nci, unsigned char *id)
{
	int ret = 0;
	struct _nctri_cmd_seq *cmd_seq = &nci->nctri->nctri_cmd_seq;

	nand_enable_chip(nci);
	ndfc_repeat_mode_enable(nci->nctri);
	ndfc_disable_randomize(nci->nctri);

	ndfc_clean_cmd_seq(cmd_seq);

	cmd_seq->cmd_type = CMD_TYPE_NORMAL;
	cmd_seq->nctri_cmd[0].cmd_valid = 1;
	cmd_seq->nctri_cmd[0].cmd = CMD_READ_ID;
	cmd_seq->nctri_cmd[0].cmd_send = 1;

	cmd_seq->nctri_cmd[0].cmd_acnt = 1;
	cmd_seq->nctri_cmd[0].cmd_addr[0] = 0x0;

	cmd_seq->nctri_cmd[0].cmd_trans_data_nand_bus = 1;
	cmd_seq->nctri_cmd[0].cmd_swap_data = 1;
	cmd_seq->nctri_cmd[0].cmd_swap_data_dma = 0;
	cmd_seq->nctri_cmd[0].cmd_direction = 0; //read
	cmd_seq->nctri_cmd[0].cmd_mdata_len = 8;
	cmd_seq->nctri_cmd[0].cmd_mdata_addr = id;

	ret = ndfc_execute_cmd(nci->nctri, cmd_seq);
	if (ret) {
		RAWNAND_ERR("nand_reset_chip, read chip id failed!\n");
	}

	ndfc_repeat_mode_disable(nci->nctri);
	nand_disable_chip(nci);

	return ret;
}

int rawnand_read_parameter_page(struct nand_chip_info *nci, unsigned char *p)
{
	int ret = 0;
	struct _nctri_cmd_seq *cmd_seq = &nci->nctri->nctri_cmd_seq;

	nand_enable_chip(nci);
	ndfc_repeat_mode_enable(nci->nctri);
	ndfc_disable_randomize(nci->nctri);

	ndfc_clean_cmd_seq(cmd_seq);

	cmd_seq->cmd_type = CMD_TYPE_NORMAL;
	cmd_seq->nctri_cmd[0].cmd_valid = 1;
	cmd_seq->nctri_cmd[0].cmd = CMD_READ_PARAMETER;
	cmd_seq->nctri_cmd[0].cmd_send = 1;
	cmd_seq->nctri_cmd[0].cmd_wait_rb = 1;

	cmd_seq->nctri_cmd[0].cmd_acnt = 1;
	cmd_seq->nctri_cmd[0].cmd_addr[0] = MICRON_ONFI_PARAMETER_ADDR;

	cmd_seq->nctri_cmd[0].cmd_trans_data_nand_bus = 1;
	cmd_seq->nctri_cmd[0].cmd_swap_data = 1;
	cmd_seq->nctri_cmd[0].cmd_swap_data_dma = 0;
	cmd_seq->nctri_cmd[0].cmd_direction = 0; //read
	cmd_seq->nctri_cmd[0].cmd_mdata_len = REVISION_FEATURES_BLOCK_INFO_PARAMETER_LEN;
	cmd_seq->nctri_cmd[0].cmd_mdata_addr = p;

	ret = ndfc_execute_cmd(nci->nctri, cmd_seq);
	if (ret) {
		RAWNAND_ERR("nand_reset_chip, read chip parameter page failed!\n");
	}

	ndfc_repeat_mode_disable(nci->nctri);
	nand_disable_chip(nci);

	return ret;
}

/*
 *Name         :
 *Description  :
 *Parameter    :
 *Return       : 0:ok  -1:fail
 *Note         :
 */
int nand_first_read_id(struct nand_chip_info *nci, unsigned int chip_no, unsigned char *id)
{
	int ret = 0;
	struct _nctri_cmd_seq *cmd_seq = &nci->nctri->nctri_cmd_seq;

	// enable chip, don't select rb signal
	ndfc_select_chip(nci->nctri, chip_no);

	ndfc_clean_cmd_seq(cmd_seq);

	cmd_seq->cmd_type = CMD_TYPE_NORMAL;
	cmd_seq->nctri_cmd[0].cmd_valid = 1;
	cmd_seq->nctri_cmd[0].cmd = CMD_READ_ID;
	cmd_seq->nctri_cmd[0].cmd_send = 1;

	cmd_seq->nctri_cmd[0].cmd_acnt = 1;
	cmd_seq->nctri_cmd[0].cmd_addr[0] = 0x0;

	cmd_seq->nctri_cmd[0].cmd_trans_data_nand_bus = 1;
	cmd_seq->nctri_cmd[0].cmd_swap_data = 1;
	cmd_seq->nctri_cmd[0].cmd_swap_data_dma = 0;
	cmd_seq->nctri_cmd[0].cmd_direction = 0; //read
	cmd_seq->nctri_cmd[0].cmd_mdata_len = 8;
	cmd_seq->nctri_cmd[0].cmd_mdata_addr = (u8 *)id;

	ret = ndfc_execute_cmd(nci->nctri, cmd_seq);
	if (ret) {
		RAWNAND_ERR("nand_reset_chip, read chip id failed!\n");
	}

	nand_disable_chip(nci);
	if (ret)
		return NAND_OP_FALSE;
	else
		return NAND_OP_TRUE;
}

/*
 *Name         :
 *Description  :
 *Parameter    :
 *Return       : 0:ok  -1:fail
 *Note         :
 */
int nand_get_feature(struct nand_chip_info *nci, u8 *addr, u8 *feature)
{
	int ret = 0;
	struct _nctri_cmd_seq *cmd_seq = &nci->nctri->nctri_cmd_seq;

	nand_enable_chip(nci);
	ndfc_repeat_mode_enable(nci->nctri);
	ndfc_disable_randomize(nci->nctri);

	ndfc_clean_cmd_seq(cmd_seq);

	cmd_seq->cmd_type = CMD_TYPE_NORMAL;
	cmd_seq->nctri_cmd[0].cmd = CMD_GET_FEATURE;
	cmd_seq->nctri_cmd[0].cmd_valid = 1;
	cmd_seq->nctri_cmd[0].cmd_send = 1;

	cmd_seq->nctri_cmd[0].cmd_acnt = 1;
	cmd_seq->nctri_cmd[0].cmd_addr[0] = addr[0];
	cmd_seq->nctri_cmd[0].cmd_wait_rb = 1;

	cmd_seq->nctri_cmd[0].cmd_trans_data_nand_bus = 1;
	cmd_seq->nctri_cmd[0].cmd_swap_data = 1;
	cmd_seq->nctri_cmd[0].cmd_swap_data_dma = 0;
	cmd_seq->nctri_cmd[0].cmd_direction = 0; //read
	cmd_seq->nctri_cmd[0].cmd_mdata_len = 4;
	cmd_seq->nctri_cmd[0].cmd_mdata_addr = feature;

	ret = ndfc_execute_cmd(nci->nctri, cmd_seq);
	if (ret) {
		RAWNAND_ERR("nand_reset_chip, read chip id failed!\n");
	}

	ndfc_repeat_mode_disable(nci->nctri);
	nand_disable_chip(nci);
	return ret;
}

/*
 *Name         :
 *Description  :
 *Parameter    :
 *Return       : 0:ok  -1:fail
 *Note         :
 */
int nand_set_feature(struct nand_chip_info *nci, u8 *addr, u8 *feature)
{
	int ret = 0;
	struct _nctri_cmd_seq *cmd_seq = &nci->nctri->nctri_cmd_seq;

	nand_enable_chip(nci);
	ndfc_repeat_mode_enable(nci->nctri);
	ndfc_disable_randomize(nci->nctri);

	ndfc_clean_cmd_seq(cmd_seq);

	cmd_seq->cmd_type = CMD_TYPE_NORMAL;
	cmd_seq->nctri_cmd[0].cmd = CMD_SET_FEATURE;
	cmd_seq->nctri_cmd[0].cmd_valid = 1;
	cmd_seq->nctri_cmd[0].cmd_send = 1;

	cmd_seq->nctri_cmd[0].cmd_acnt = 1;
	cmd_seq->nctri_cmd[0].cmd_addr[0] = addr[0];
	cmd_seq->nctri_cmd[0].cmd_wait_rb = 1;

	cmd_seq->nctri_cmd[0].cmd_trans_data_nand_bus = 1;
	cmd_seq->nctri_cmd[0].cmd_swap_data = 1;
	cmd_seq->nctri_cmd[0].cmd_swap_data_dma = 0;
	cmd_seq->nctri_cmd[0].cmd_direction = 1; //write
	cmd_seq->nctri_cmd[0].cmd_mdata_len = 4;
	cmd_seq->nctri_cmd[0].cmd_mdata_addr = feature;

	ret = ndfc_execute_cmd(nci->nctri, cmd_seq);
	if (ret) {
		RAWNAND_ERR("nand_reset_chip, read chip id failed!\n");
	}

	ndfc_repeat_mode_disable(nci->nctri);
	nand_disable_chip(nci);
	return ret;
}

/*
 *Name         :
 *Description  :
 *Parameter    :
 *Return       : 0:ok  -1:fail
 *Note         :
 */
unsigned int get_row_addr(unsigned int page_offset_for_next_blk, unsigned int block, unsigned int page)
{
	unsigned int maddr;

	if (page_offset_for_next_blk == 32)
		maddr = (block << 5) + page;
	else if (page_offset_for_next_blk == 64)
		maddr = (block << 6) + page;
	else if (page_offset_for_next_blk == 128)
		maddr = (block << 7) + page;
	else if (page_offset_for_next_blk == 256)
		maddr = (block << 8) + page;
	else if (page_offset_for_next_blk == 512)
		maddr = (block << 9) + page;
	else if (page_offset_for_next_blk == 1024)
		maddr = (block << 10) + page;
	else if ((page_offset_for_next_blk > 256)
			 && (page_offset_for_next_blk < 512)) {
		maddr = (block << 9) + page;
		//RAWNAND_ERR("pb %d!\n", page_offset_for_next_blk);
	} else {
		maddr = 0xffffffff;
		RAWNAND_ERR("error page per block %d!\n", page_offset_for_next_blk);
	}
	return maddr;
}

/*
 *Name         :
 *Description  :
 *Parameter    :
 *Return       : 0:ok  -1:fail
 *Note         :
 */
int fill_cmd_addr(u32 col_addr, u32 col_cycle, u32 row_addr, u32 row_cycle, u8 *abuf)
{
	s32 i;

	if (col_cycle) {
		for (i = 0; i < col_cycle; i++)
			abuf[i] = (col_addr >> (i * 8)) & 0xff;
	}

	if (row_cycle) {
		for (i = 0 + col_cycle; i < col_cycle + row_cycle; i++)
			abuf[i] = (row_addr >> ((i - col_cycle) * 8)) & 0xff;
	}

	return 0;
}

/*
 *Name         :
 *Description  :
 *Parameter    :
 *Return       : 0:ok  -1:fail
 *Note         :
 */
void set_default_batch_read_cmd_seq(struct _nctri_cmd_seq *cmd_seq)
{
	cmd_seq->cmd_type = CMD_TYPE_BATCH;
	cmd_seq->ecc_layout = ECC_LAYOUT_INTERLEAVE;

	cmd_seq->nctri_cmd[0].cmd = CMD_READ_PAGE_CMD1;
	cmd_seq->nctri_cmd[0].cmd_wait_rb = 1;

	cmd_seq->nctri_cmd[1].cmd = CMD_READ_PAGE_CMD2;
	cmd_seq->nctri_cmd[2].cmd = CMD_CHANGE_READ_ADDR_CMD1;
	cmd_seq->nctri_cmd[3].cmd = CMD_CHANGE_READ_ADDR_CMD2;

	cmd_seq->nctri_cmd[0].cmd_valid = 1;
	cmd_seq->nctri_cmd[1].cmd_valid = 1;
	cmd_seq->nctri_cmd[2].cmd_valid = 1;
	cmd_seq->nctri_cmd[3].cmd_valid = 1;
}

/*
 *Name         :
 *Description  :
 *Parameter    :
 *Return       : 0:ok  -1:fail
 *Note         :
 */
void set_default_batch_write_cmd_seq(struct _nctri_cmd_seq *cmd_seq, u32 write_cmd1, u32 write_cmd2)
{
	cmd_seq->cmd_type = CMD_TYPE_BATCH;
	cmd_seq->ecc_layout = ECC_LAYOUT_INTERLEAVE;

	cmd_seq->nctri_cmd[0].cmd = write_cmd1;
	cmd_seq->nctri_cmd[0].cmd_wait_rb = 1;

	cmd_seq->nctri_cmd[1].cmd = write_cmd2;
	cmd_seq->nctri_cmd[2].cmd = CMD_CHANGE_WRITE_ADDR_CMD;

	cmd_seq->nctri_cmd[0].cmd_valid = 1;
	cmd_seq->nctri_cmd[1].cmd_valid = 1;
	cmd_seq->nctri_cmd[2].cmd_valid = 1;
}

/*
 *Name         :
 *Description  :
 *Parameter    :
 *Return       : 0:ok  -1:fail
 *Note         :
 */
s32 _cal_nand_onfi_timing_mode(u32 mode, u32 dclk)
{
	s32 tmode = -1;

	if (mode == 0x0) {
		/*SDR/Async mode*/
		if (dclk < 15)
			tmode = 0;
		else if (dclk < 24)
			tmode = 1;
		else if (dclk < 30)
			tmode = 2; //30.5->30
		else if (dclk < 36)
			tmode = 3; //36.5->36
		else if (dclk < 45)
			tmode = 4;
		else if (dclk <= 50)
			tmode = 5;
		else
			RAWNAND_ERR("wrong dclk(%d) in mode(%d)\n", dclk, mode);
	} else if (mode == 0x2) {
		/*nv-ddr*/
		if (dclk < 26)
			tmode = 0; //26.5->26
		else if (dclk < 41)
			tmode = 1; //41.5->41
		else if (dclk < 58)
			tmode = 2; //58.5->58
		else if (dclk < 75)
			tmode = 3;
		else if (dclk < 91)
			tmode = 4; //91.5->91
		else if (dclk <= 100)
			tmode = 5;
		else
			RAWNAND_ERR("wrong dclk(%d) in mode(%d)\n", dclk, mode);
	} else if (mode == 0x12) {
		 /*nv-ddr2*/
		if (dclk < 36)
			tmode = 0; //36.5->36
		else if (dclk < 53)
			tmode = 1;
		else if (dclk < 74)
			tmode = 2; //74.5->74
		else if (dclk < 91)
			tmode = 3; //91.5->91
		else if (dclk < 116)
			tmode = 4; //116.5->116
		else if (dclk < 149)
			tmode = 5; //149.5->149
		else if (dclk < 183)
			tmode = 6;
		else if (dclk < 200)
			tmode = 7;
		else
			RAWNAND_ERR("wrong dclk(%d) in mode(%d)\n", dclk, mode);
	} else {
		tmode = 0;
	}
	return tmode;
}

/*
 *Name         :
 *Description  :
 *Parameter    :
 *Return       : 0:ok  -1:fail
 *Note         :
 */
s32 _change_nand_onfi_timing_mode(struct nand_chip_info *nci, u32 if_type, u32 timing_mode)
{
	u8 addr;
	u8 p[4];

	if (!SUPPORT_CHANGE_ONFI_TIMING_MODE) {
		RAWNAND_ERR("don't support change onfi timing mode. if_type: %d\n", if_type);
		return ERR_NO_71;
	}

	if ((if_type != SDR) && (if_type != ONFI_DDR) && (if_type != ONFI_DDR2)) {
		RAWNAND_ERR("wrong onfi interface type: %d\n", if_type);
		return ERR_NO_70;
	}

	if ((if_type == SDR) && (timing_mode > 5)) {
		RAWNAND_ERR("wrong onfi timing mode(%d) in interface type(%d)\n", if_type, timing_mode);
		return ERR_NO_69;
	}
	if ((if_type == ONFI_DDR) && (timing_mode > 5)) {
		RAWNAND_ERR("wrong onfi timing mode(%d) in interface type(%d)\n", if_type, timing_mode);
		return ERR_NO_68;
	}
	if ((if_type == ONFI_DDR2) && (timing_mode > 7)) {
		RAWNAND_ERR("wrong onfi timing mode(%d) in interface type(%d)\n", if_type, timing_mode);
		return ERR_NO_67;
	}

	addr = 0x01; //feature address 01h, Timing Mode
	p[0] = 0;
	if (if_type == ONFI_DDR)
		p[0] = (0x1U << 4) | (timing_mode & 0xf);
	else if (if_type == ONFI_DDR2)
		p[0] = (0x2U << 4) | (timing_mode & 0xf);
	else
		p[0] = (timing_mode & 0xf);

	p[1] = 0x0;
	p[2] = 0x0;
	p[3] = 0x0;
	nand_set_feature(nci, &addr, p);
	//aw_delay(0x100); //max tITC is 1us

	return 0;
}

/*
 *Name         :
 *Description  :
 *Parameter    :
 *Return       : 0:ok  -1:fail
 *Note         :
 */
s32 _setup_ddr_nand_force_to_sdr_para(struct nand_chip_info *nci)
{
	u8 addr;
	u8 p[4];
	/*u8 pr[4]; //read back value*/

		/*sandisk/toshiba nand flash*/
		addr = 0x80; //Sandisk: This address (80h) is a vendor-specific setting used to turn on or turn off Toggle Mode

#if 0
	   nand_get_feature(nci, &addr, pr);
		ndfc_set_legacy_interface(nci->nctri);
		nand_get_feature(nci, &addr, pr);
		//RAWNAND_ERR("get feature(addr 0x80) 0x%x 0x%x 0x%x 0x%x!\n",pr[0],pr[1],pr[2],pr[3]);
		if (pr[0] == 0x00) {
		ndfc_set_toggle_interface(nci->nctri);
		} else {
			ndfc_set_legacy_interface(nci->nctri);
		}
#endif

		p[0] = 0x1; //disable toggle mode
		p[1] = 0x0;
		p[2] = 0x0;
		p[3] = 0x0;
		nand_set_feature(nci, &addr, p);
		ndfc_set_legacy_interface(nci->nctri);
#if 0
		nand_get_feature(nci, &addr, pr);
		//RAWNAND_ERR("get feature(addr 0x80) 0x%x 0x%x 0x%x 0x%x!\n",pr[0],pr[1],pr[2],pr[3]);
#endif

	return 0;
}

/*
 *Name         :
 *Description  :
 *Parameter    :
 *Return       : 0:ok  -1:fail
 *Note         :
 */
s32 _check_scan_data(u32 first_check, u32 chip, u32 *scan_good_blk_no, u8 *main_buf)
{
	s32 ret;
	u32 b, start_blk = 4, blk_cnt = 5;
	u8 oob_buf[64];
	u32 buf_size = 32768;
	u32 buf_flag = 0;

	if (main_buf == NULL) {
		main_buf = nand_get_temp_buf(buf_size);
		if (!main_buf) {
			RAWNAND_ERR("check scan data, main_buf %p is null!\n", main_buf);
			return ERR_NO_63;
		}
		buf_flag = 1;
	}

	if (first_check) {
		scan_good_blk_no[chip] = 0xffff;

		for (b = start_blk; b < start_blk + blk_cnt; b++) {
			//ret = nand_read_scan_data(0, b, 0, g_nsi->nci->sector_cnt_per_page, main_buf, oob_buf);
			ret = nand_read_scan_data(0, b, 0, g_nsi->nci->sector_cnt_per_page, NULL, oob_buf);
			if ((oob_buf[0] == 0xff) && (oob_buf[1] == 0xff) && (oob_buf[2] == 0xff) && (oob_buf[3] == 0xff)) {
				scan_good_blk_no[chip] = b;
				ret = 1;
				break;
			}

			if (ret >= 0) {
				ret = 0;
				scan_good_blk_no[chip] = b;
				break;
			}
		}
		RAWNAND_DBG("check scan data, first_check %u %d!\n", scan_good_blk_no[chip], ret);
	} else {
		//ret = nand_read_scan_data(0, scan_good_blk_no[chip], 0, g_nsi->nci->sector_cnt_per_page, main_buf, oob_buf);
		ret = nand_read_scan_data(0, scan_good_blk_no[chip], 0, g_nsi->nci->sector_cnt_per_page, NULL, oob_buf);
		if (ret >= 0) {
			ret = 0;
		}

		if ((oob_buf[0] == 0xff) && (oob_buf[1] == 0xff) && (oob_buf[2] == 0xff) && (oob_buf[3] == 0xff)) {
			ret = 1;
		}
	}

	if (buf_flag != 0) {
		nand_free_temp_buf(main_buf);
	}

	return ret;
}

/*
 *Name         :
 *Description  :
 *Parameter    :
 *Return       : 0:ok  -1:fail
 *Note         :
 */
s32 _change_all_nand_parameter(struct nand_controller_info *nctri, u32 ddr_type, u32 pre_ddr_type, u32 dclk)
{
	struct nand_chip_info *nci = nctri->nci;
	//__s32 ret;
	//__u32 bank;
	//__u8  chip, rb;
	__u32 ddr_change_mode = 0;
	__u32 tmode;
	s32 ret;
	//NFC_CMD_LIST reset_cmd;

	/*check parameter*/
	if (((ddr_type == ONFI_DDR) || (ddr_type == ONFI_DDR2)) && (pre_ddr_type == SDR)) //Async => ONFI DDR/DDR2
		ddr_change_mode = 1;
	else if ((ddr_type == SDR) && ((pre_ddr_type == ONFI_DDR) || (pre_ddr_type == ONFI_DDR2))) //ONFI DDR/DDR2 => Async
		ddr_change_mode = 2;
	else if (((ddr_type == TOG_DDR) || (ddr_type == TOG_DDR2)) && (pre_ddr_type == SDR)) //Async => Toggle DDR/DDR2
		ddr_change_mode = 3;
	else if ((ddr_type == SDR) && ((pre_ddr_type == TOG_DDR) || (pre_ddr_type == TOG_DDR2))) //Toggle DDR/DDR2 => Async
		ddr_change_mode = 4;
	else if (((ddr_type == TOG_DDR) && (pre_ddr_type == TOG_DDR2)) || ((ddr_type == TOG_DDR2) && (pre_ddr_type == TOG_DDR))) //Toggle DDR2 <=> Toggle DDR
		ddr_change_mode = 5;
	else if (ddr_type == pre_ddr_type)
		ddr_change_mode = 6;
	else {
		RAWNAND_ERR("_change_nand_parameter: wrong input para, "
			    "ddr_type %d, pre_ddr_type %d\n",
			    ddr_type, pre_ddr_type);
		return ERR_NO_62;
	}

	tmode = _cal_nand_onfi_timing_mode(ddr_type, dclk);
	nci->timing_mode = tmode;

	/*change nand flash parameter*/
	while (nci) {
		nand_enable_chip(nci);

		//RAWNAND_DBG("%s: ch: %d  chip: %d  rb: %d\n", __func__, NandIndex, chip, rb);

		if (ddr_change_mode == 1) {
			/* Async => ONFI DDR/DDR2 */
			RAWNAND_DBG("mode 1 : Async => ONFI DDR/DDR2\n");
			ret = rawnand_async_to_onfi_ddr_or_ddr2_set(nci, ddr_type);
			if (ret != NAND_OP_TRUE) {
				RAWNAND_ERR("rawnand err: %s rawnand async set to onfi %s fail!\n", __func__,
					ddr_type == ONFI_DDR ? "ddr" : "ddr2");
				return NAND_OP_FALSE;
			}
		} else if (ddr_change_mode == 2) {
			/* ONFI DDR/DDR2 => Async */

			ret = rawnand_onfi_ddr_or_ddr2_to_async_set(nci, ddr_type, pre_ddr_type);
			if (ret != NAND_OP_TRUE) {
				RAWNAND_ERR("rawnand err: %s rawnand onfi %s set to async fail!\n", __func__,
					pre_ddr_type == ONFI_DDR ? "ddr" : "ddr2");
				return NAND_OP_FALSE;
			}

		} else if (ddr_change_mode == 3) {
			/* Async => Toggle DDR/DDR2 */

			ret = rawnand_async_to_toggle_ddr_or_ddr2_set(nci, ddr_type);
			if (ret != NAND_OP_TRUE) {
				RAWNAND_ERR("rawnand err: %s rawnand async set to toggle %s fail\n", __func__,
					ddr_type == TOG_DDR ? "ddr" : "ddr2");
				return NAND_OP_FALSE;
			}
		} else if (ddr_change_mode == 4) {
			/* Toggle DDR/DDR2 => Async */
			RAWNAND_DBG("mode 4 : Toggle DDR/DDR2 => Async\n");

			ret = rawnand_toggle_ddr_or_ddr2_to_async_set(nci, ddr_type, pre_ddr_type);
			if (ret != NAND_OP_TRUE) {
				RAWNAND_ERR("rawnand err: %s rawnand toggle %s set to async fail\n", __func__,
					ddr_type == TOG_DDR ? "ddr" : "ddr2");
				return NAND_OP_FALSE;
			}
		} else if (ddr_change_mode == 5) {
			/* Toggle DDR2 <=> Toggle DDR */
			RAWNAND_DBG("mode 5 : Toggle DDR2 <=> Toggle DDR\n");

			ret = rawnand_toggle_ddr2_to_toggle_ddr_set(nci);
			if (ret != NAND_OP_TRUE) {
				RAWNAND_ERR("rawnand err: %s rawnand toggle ddr2 set to toggle ddr fail\n", __func__);
				return NAND_OP_FALSE;
			}
		} else if (ddr_change_mode == 6) {
			RAWNAND_DBG("mode 6: unchanged, rawnand set to %s\n",
					ddr_type == SDR ? "sdr" : (ddr_type == ONFI_DDR ? "onfi ddr" :
					(ddr_type == ONFI_DDR2 ? "onfi ddr2" : (ddr_type == TOG_DDR ?
						"toggle ddr" : (ddr_type == TOG_DDR2 ? "toggle ddr2" : "null")))));
			ret = rawnand_itf_unchanged_set(nci, ddr_type, pre_ddr_type);
			if (ret != NAND_OP_TRUE) {
				RAWNAND_ERR("rawnand err: %s rawnand set to %s fail\n", __func__,
					ddr_type == SDR ? "sdr" : (ddr_type == ONFI_DDR ? "onfi ddr" :
					(ddr_type == ONFI_DDR2 ? "onfi ddr2" : (ddr_type == TOG_DDR ?
						"toggle ddr" : (ddr_type == TOG_DDR2 ? "toggle ddr2" : "null")))));
				return NAND_OP_FALSE;
			}

		} else {
			;
		}
		nci = nci->nctri_next;
	}

	return 0;
}

/*
 *Name         :
 *Description  :
 *Parameter    :
 *Return       : 0:ok  -1:fail
 *Note         :
 */
s32 _get_right_timing_para(struct nand_controller_info *nctri, u32 ddr_type, u32 *good_sdr_edo, u32 *good_ddr_edo, u32 *good_ddr_delay)
{
	//	u32 chip = 0;
	struct nand_chip_info *nci = nctri->nci; //chip 0
	u32 *scan_blk_no = nctri->ddr_scan_blk_no;
	u32 edo, delay, edo_cnt, delay_cnt;
	u32 edo_delay[2][3], tmp_edo_delay[3];
	u32 good_cnt, tmp_good_cnt, store_index;
	s32 err_flag;
	u32 good_flag;
	u32 index, i, j;
	u8 tmpChipID[16];
	u32 param[2];
	s32 ret;
	u32 sclk0_bak, sclk1_bak;
	u32 sdr_edo, ddr_edo, ddr_delay, tmp;
	u8 *main_buf;

	for (i = 0; i < 2; i++) {
		for (j = 0; j < 3; j++)
			edo_delay[i][j] = 0xffffffff;
	}
	for (j = 0; j < 3; j++)
		tmp_edo_delay[j] = 0xffffffff;

	good_flag = 0;
	index = 0;
	store_index = 0;
	good_cnt = 0;
	tmp_good_cnt = 0;
	edo_cnt = 0;
	delay_cnt = 0;
	param[0] = 0xffffffff;
	param[1] = 0xffffffff;

	if (ddr_type == SDR) {
		edo_cnt = 3;
		delay_cnt = 1;
		if (SUPPORT_SCAN_EDO_FOR_SDR_NAND == 0) {
			RAWNAND_DBG("_get right timing para, set edo to 1 for sdr nand.\n");
			*good_sdr_edo = 1;
			return 0;
		}
	} else {
		delay_cnt = 64;
		if (NDFC_VERSION_V2 == nctri->type)
			edo_cnt = 32;
		else if (NDFC_VERSION_V1 == nctri->type)
			edo_cnt = 16; //32
		else {
			RAWNAND_ERR("wrong ndfc version, %d\n", nctri->type);
			return ERR_NO_61;
		}
	}

	main_buf = nand_get_temp_buf(32768);
	if (!main_buf) {
		RAWNAND_ERR("main_buf %p is null!\n", main_buf);
		return ERR_NO_60;
	}

	for (edo = 0; edo < edo_cnt; edo++) {
		good_flag = 0;
		for (delay = 0; delay < delay_cnt; delay += 2) {
			if (ddr_type == SDR) {
				sdr_edo = edo;
				ddr_edo = edo;
				tmp = sdr_edo << 8;
				ddr_delay = delay;
			} else {
				sdr_edo = edo;
				ddr_edo = edo;
				ddr_delay = delay;
				tmp = (ddr_edo << 8) | ddr_delay;
			}
			ndfc_change_nand_interface(nctri, ddr_type, sdr_edo, ddr_edo, ddr_delay);
			nctri->ddr_timing_ctl[0] = tmp;

			//read id
			ret = nand_read_id(nci, tmpChipID); //chip 0
			if (ret) {
				RAWNAND_ERR("read id failed! ---> continue\n");
				continue;
			}

			if ((nctri->nci->id[0] == tmpChipID[0]) && (nctri->nci->id[1] == tmpChipID[1]) && (nctri->nci->id[2] == tmpChipID[2]) && (nctri->nci->id[3] == tmpChipID[3])) {
				err_flag = _check_scan_data(0, 0, scan_blk_no, main_buf);
				if (err_flag == 0) {
					RAWNAND_DBG("ddr edo:0x%x, delay:0x%x is good\n", edo, ddr_delay);

					good_flag = 1;
					if (ddr_type == SDR) {
						/*sdr mode*/
						if (edo == 0) {
							nand_get_clk(&aw_ndfc, nctri->channel_id, &sclk0_bak, &sclk1_bak);
							RAWNAND_DBG("sclk0 %d MHz, edo %d\n", sclk0_bak, edo);
							if (sclk0_bak < 12) //less 12MHz
								break;
							else
								good_flag = 0;
						}
						break;
					}

					if ((index != 0) && (index != 1))
						RAWNAND_ERR("wrong index!\n");
					if (store_index == 0) {
						if (edo_delay[index][0] == 0xffffffff) {
							/* first found */
							edo_delay[index][0] = edo;   //k;
							edo_delay[index][1] = delay; //m;
							edo_delay[index][2] = delay; //m;
						} else {
							edo_delay[index][2] = delay; //m;
						}
						good_cnt++;
					} else if (store_index == 1) {
						if (tmp_edo_delay[0] == 0xffffffff) {
							/*first found*/
							tmp_edo_delay[0] = edo;   //k;
							tmp_edo_delay[1] = delay; //m;
							tmp_edo_delay[2] = delay; //m;
						} else {
							tmp_edo_delay[2] = delay; //m;
						}
						tmp_good_cnt++;
					}
				} else {
					/* id is ok, but data is wrong */
					RAWNAND_DBG("ddr edo:0x%x, delay:0x%x is wrong:%d\n", edo, ddr_delay,
							err_flag);
					if (good_cnt == 0) {
						store_index = 0;
					} else if (tmp_good_cnt == 0) {
						//store good {edo, delay} to tmp_edo_delay[]
						store_index = 1;
					} else if (tmp_good_cnt > good_cnt) {
						//move tmp_edo_delay[] to edo_delay[][]
						edo_delay[index][0] = tmp_edo_delay[0];
						edo_delay[index][1] = tmp_edo_delay[1];
						edo_delay[index][2] = tmp_edo_delay[2];
						good_cnt = tmp_good_cnt;

						//clear tmp_edo_delay[] for next valid group
						store_index = 1;
						tmp_good_cnt = 0;
						for (j = 0; j < 3; j++)
							tmp_edo_delay[j] = 0xffffffff;
					} else {
						store_index = 1;
						tmp_good_cnt = 0;
						for (j = 0; j < 3; j++)
							tmp_edo_delay[j] = 0xffffffff;
					}
				}
			} else {
				/* read id wrong */
				//RAWNAND_ERR("timing_para read id wrong!\n");
				if (good_cnt == 0) {
					store_index = 0;
				} else if (tmp_good_cnt == 0) {
					//store good {edo, delay} to tmp_edo_delay[]
					store_index = 1;
				} else if (tmp_good_cnt > good_cnt) {
					//move tmp_edo_delay[] to edo_delay[][]
					edo_delay[index][0] = tmp_edo_delay[0];
					edo_delay[index][1] = tmp_edo_delay[1];
					edo_delay[index][2] = tmp_edo_delay[2];
					good_cnt = tmp_good_cnt;

					//clear tmp_edo_delay[] for next valid group
					store_index = 1;
					tmp_good_cnt = 0;
					for (j = 0; j < 3; j++)
						tmp_edo_delay[j] = 0xffffffff;
				} else {
					store_index = 1;
					tmp_good_cnt = 0;
					for (j = 0; j < 3; j++)
						tmp_edo_delay[j] = 0xffffffff;
				}
			}
		}

		if (good_flag) {
			if (ddr_type == SDR) //sdr mode
				break;

			if (index == 0) {
				if (good_cnt >= GOOD_DDR_EDO_DELAY_CHAIN_TH) //8 groups of {edo, delay} is enough
					break;
				index = 1;
				store_index = 0;
				good_cnt = 0;
				tmp_good_cnt = 0;
				for (j = 0; j < 3; j++)
					tmp_edo_delay[j] = 0xffffffff;
			} else {
				break;
			}
		}
	}

	if (ddr_type == SDR) {
		if (good_flag) {
			*good_sdr_edo = edo;
		}
		goto RET;
	}

	if ((edo_delay[0][0] == 0xffffffff) && (edo_delay[1][0] == 0xffffffff)) {
		good_flag = 0;
		RAWNAND_ERR("can't find a good edo, delay chain. index %d:  %d %d %d\n", index, edo_delay[index][0], edo_delay[index][1], edo_delay[index][2]);
	} else if ((edo_delay[0][0] != 0xffffffff) && (edo_delay[1][0] != 0xffffffff)) {
		i = edo_delay[0][2] - edo_delay[0][1];
		j = edo_delay[1][2] - edo_delay[1][1];
		if (j > i) {
			param[0] = edo_delay[1][0];
			param[1] = (edo_delay[1][1] + edo_delay[1][2]) / 2 + 1;
			if (j >= GOOD_DDR_EDO_DELAY_CHAIN_TH)
				good_flag = 1;
			else
				good_flag = 0;
		} else {
			param[0] = edo_delay[0][0];
			param[1] = (edo_delay[0][1] + edo_delay[0][2]) / 2 + 1;
			if (j >= GOOD_DDR_EDO_DELAY_CHAIN_TH)
				good_flag = 1;
			else
				good_flag = 0;
		}
		RAWNAND_DBG("(0x%x, 0x%x - 0x%x), (0x%x, 0x%x - 0x%x)\n", edo_delay[0][0], edo_delay[0][1], edo_delay[0][2], edo_delay[1][0], edo_delay[1][1], edo_delay[1][2]);
		if (good_flag)
			RAWNAND_DBG("%d good edo: 0x%x, good delay chain: 0x%x\n", __LINE__,  param[0], param[1]);
		else
			RAWNAND_ERR("can't find a good edo, delay chain !\n");
	} else if ((edo_delay[0][0] != 0xffffffff) && (edo_delay[1][0] == 0xffffffff)) {
		i = edo_delay[0][2] - edo_delay[0][1];
		param[0] = edo_delay[0][0];
		param[1] = (edo_delay[0][1] + edo_delay[0][2]) / 2 + 1;
		RAWNAND_DBG("(0x%x, 0x%x - 0x%x) \n", edo_delay[0][0], edo_delay[0][1], edo_delay[0][2]);
		if (i >= GOOD_DDR_EDO_DELAY_CHAIN_TH)
			good_flag = 1;
		else
			good_flag = 0;

		if (good_flag)
			RAWNAND_DBG("%d good edo: 0x%x, good delay chain: 0x%x\n", __LINE__, param[0], param[1]);
		else
			RAWNAND_ERR("can't find a good edo, delay chain!!\n");
	} else {
		good_flag = 0;
		RAWNAND_ERR("scan error!!!!!!!\n");
	}

	*good_ddr_edo = param[0];
	*good_ddr_delay = param[1];

RET:
	nand_free_temp_buf(main_buf);
	if (good_flag == 0) {
		return ERR_NO_59;
	}

	return 0;
}

/*
 *Name         :
 *Description  :
 *Parameter    :
 *Return       : 0:ok  -1:fail
 *Note         :
 */
int set_cmd_with_nand_bus(struct nand_chip_info *nci, u8 *cmd, u32 wait_rb, u8 *addr, u8 *dat, u32 dat_len, u32 counter)
{
	int ret, i;
	u8 *p_dat = dat;

	struct _nctri_cmd_seq *cmd_seq = &nci->nctri->nctri_cmd_seq;

	ndfc_repeat_mode_enable(nci->nctri);

	if (nci->randomizer) {
		ndfc_disable_randomize(nci->nctri);
	}

	ndfc_clean_cmd_seq(cmd_seq);

	cmd_seq->cmd_type = CMD_TYPE_NORMAL;

	//RAWNAND_DBG("set cmd with nand bus:");

	for (i = 0; i < counter; i++) {
		cmd_seq->nctri_cmd[i].cmd_valid = 1;
		if (cmd != NULL) {
			cmd_seq->nctri_cmd[i].cmd = cmd[i];
			cmd_seq->nctri_cmd[i].cmd_send = 1;
			if (wait_rb != 0) {
				cmd_seq->nctri_cmd[i].cmd_wait_rb = 1;
			}
		}

		if ((addr == NULL) && (dat == NULL)) {
			//RAWNAND_DBG("cmd:0x%x;",cmd[i]);
		}

		if (addr != NULL) {
			cmd_seq->nctri_cmd[i].cmd_acnt = 1;
			cmd_seq->nctri_cmd[i].cmd_addr[0] = addr[i];

			//RAWNAND_DBG("addr:0x%x;",addr[i]);
		}

		if (dat != NULL) {
			cmd_seq->nctri_cmd[i].cmd_trans_data_nand_bus = 1;
			cmd_seq->nctri_cmd[i].cmd_swap_data = 1;
			cmd_seq->nctri_cmd[i].cmd_direction = 1;
			cmd_seq->nctri_cmd[i].cmd_mdata_len = dat_len;
			cmd_seq->nctri_cmd[i].cmd_mdata_addr = p_dat;

			//            RAWNAND_DBG("data:");
			//            for(j=0;j<dat_len;j++)
			//            {
			//                RAWNAND_DBG("0x%x,",p_dat[j]);
			//            }

			p_dat += dat_len;
		}
	}

	//RAWNAND_DBG("\n");

	ret = ndfc_execute_cmd(nci->nctri, cmd_seq);

	ndfc_repeat_mode_disable(nci->nctri);

	if (ret != 0) {
		RAWNAND_ERR("set cmd with nand bus fail, cmd:0x%x;dat_len:0x%x;counter:0x%x;!\n", cmd[0], dat_len, counter);
		return ret;
	}
	return 0;
}

/*
 *Name         :
 *Description  :
 *Parameter    :
 *Return       : 0:ok  -1:fail
 *Note         :
 */
int get_data_with_nand_bus_one_cmd(struct nand_chip_info *nci, u8 *cmd, u8 *addr, u8 *dat, u32 dat_len)
{
	int ret, j;
	u8 *p_dat = dat;

	struct _nctri_cmd_seq *cmd_seq = &nci->nctri->nctri_cmd_seq;

	if (dat_len > 1024) {
		RAWNAND_ERR("dat_len is too long %d!\n", dat_len);
		return ERR_NO_133;
	}

	ndfc_repeat_mode_enable(nci->nctri);

	if (nci->randomizer) {
		ndfc_disable_randomize(nci->nctri);
	}

	ndfc_clean_cmd_seq(cmd_seq);

	cmd_seq->cmd_type = CMD_TYPE_NORMAL;

	//RAWNAND_DBG("set cmd with nand bus:");

	cmd_seq->nctri_cmd[0].cmd_valid = 1;
	if (cmd != NULL) {
		cmd_seq->nctri_cmd[0].cmd = *cmd;
		cmd_seq->nctri_cmd[0].cmd_send = 1;
		cmd_seq->nctri_cmd[0].cmd_wait_rb = 1;
	}

	if ((addr == NULL) && (dat == NULL)) {
		RAWNAND_DBG("cmd:0x%x;", *cmd);
	}

	if (addr != NULL) {
		cmd_seq->nctri_cmd[0].cmd_acnt = 1;
		cmd_seq->nctri_cmd[0].cmd_addr[0] = *addr;

		//RAWNAND_DBG("addr:0x%x;",addr[i]);
	}

	if (dat != NULL) {
		cmd_seq->nctri_cmd[0].cmd_trans_data_nand_bus = 1;
		cmd_seq->nctri_cmd[0].cmd_swap_data = 1;
		cmd_seq->nctri_cmd[0].cmd_direction = 0;
		cmd_seq->nctri_cmd[0].cmd_mdata_len = dat_len;
		cmd_seq->nctri_cmd[0].cmd_mdata_addr = p_dat;

		//RAWNAND_DBG("data:");
		for (j = 0; j < dat_len; j++) {
			//RAWNAND_DBG("0x%x,",p_dat[j]);
		}
	}

	//RAWNAND_DBG("\n");

	ret = ndfc_execute_cmd(nci->nctri, cmd_seq);

	ndfc_repeat_mode_disable(nci->nctri);

	if (ret != 0) {
		RAWNAND_ERR("get_data_with nand_bus_one_cmd fail, cmd:0x%x;dat_len:0x%x!\n", cmd[0], dat_len);
		return ret;
	}
	return 0;
}

/*
 *Name         :
 *Description  :
 *Parameter    :
 *Return       : 0:ok  -1:fail
 *Note         :
 */
int set_one_cmd(struct nand_chip_info *nci, u8 cmd, u32 wait_rb)
{
	int ret;
	u8 cmd_8[1];

	cmd_8[0] = cmd;
	ret = set_cmd_with_nand_bus(nci, cmd_8, wait_rb, NULL, NULL, 0, 1);

	return ret;
}

/*
 *Name         :
 *Description  :
 *Parameter    :
 *Return       : 0:ok  -1:fail
 *Note         :
 */
int set_one_addr(struct nand_chip_info *nci, u8 addr)
{
	int ret;
	u8 addr_8[1];

	addr_8[0] = addr;
	ret = set_cmd_with_nand_bus(nci, NULL, 0, addr_8, NULL, 0, 1);

	return ret;
}

/*
 *Name         :
 *Description  :
 *Parameter    :
 *Return       : 0:ok  -1:fail
 *Note         :
 */
int switch_ddrtype_from_ddr_to_sdr(struct nand_controller_info *nctri)
{
	int cfg, ddr_type, ddr_type_bak;

	ddr_type = SDR;

	if ((nctri->nci->support_toggle_only == 0) && (nctri->nci->interface_type > 0)) {
		//		RAWNAND_DBG("switch_ddrtype_from_ddr_to_sdr start !\n");
		ddr_type_bak = nctri->nci->interface_type;
		nctri->nci->interface_type = SDR;

		nctri->nreg_bak.reg_timing_ctl = 0x100;

		cfg = nctri->nreg_bak.reg_ctl;
		cfg &= ~(0x3U << 18);
		cfg |= (ddr_type & 0x3) << 18;
		if (nctri->type == NDFC_VERSION_V2) {
			cfg &= ~(0x1 << 28);
			cfg |= ((ddr_type >> 4) & 0x1) << 28;
		}
		nctri->nreg_bak.reg_ctl = cfg;

		_change_all_nand_parameter(nctri, ddr_type, ddr_type_bak, 40);

		nctri->nci->interface_type = ddr_type_bak;
	}

	return 0;
}

/*
 *Name         :
 *Description  :
 *Parameter    :
 *Return       : 0:ok  -1:fail
 *Note         :
 */
int get_dummy_byte(int physic_page_size, int ecc_mode, int ecc_block_cnt, int user_data_size)
{
	int ecc_code_size_per_1K, valid_size, dummy_byte;
	u8 ecc_tab[16] = {16, 24, 28, 32, 40, 44, 48, 52, 56, 60, 64, 68, 72, 76, 80};
	int ecc_bit;

	ecc_bit = ecc_tab[ecc_mode];
	ecc_code_size_per_1K = 14 * ecc_bit / 8;
	valid_size = (1024 + ecc_code_size_per_1K) * ecc_block_cnt + user_data_size;
	dummy_byte = physic_page_size - valid_size;

	return dummy_byte;
}

/*
 *Name         :
 *Description  :
 *Parameter    :
 *Return       : 0:ok  -1:fail
 *Note         :
 */
int get_random_cmd2(struct _nand_physic_op_par *npo)
{
	return (npo->page % 3);
}

/*
 *Name         :
 *Description  :
 *Parameter    :
 *Return       : 0:ok  -1:fail
 *Note         :
 */
int get_data_block_cnt(unsigned int sect_bitmap)
{
	int i, count = 0;

	for (i = 0; i < 32; i++) {
		if ((sect_bitmap >> i) & 0x1)
			count++;
	}

	return count;
}

/*
 *Name         :
 *Description  :
 *Parameter    :
 *Return       : 0:ok  -1:fail
 *Note         :
 */
int get_data_block_cnt_for_boot0_ecccode(struct nand_chip_info *nci, u8 ecc_mode)
{
	int size_increased, cnt, i;
	u8 ecc_tab[16] = {16, 24, 28, 32, 40, 44, 48, 52, 56, 60, 64, 68, 72, 76, 80};
	int ecc_bit_boot0, ecc_bit_normal;

	ecc_bit_boot0 = ecc_tab[ecc_mode];
	ecc_bit_normal = ecc_tab[nci->ecc_mode];
	size_increased = 14 * (ecc_bit_boot0 - ecc_bit_normal) / 8;

	cnt = 1;
	for (i = (nci->sector_cnt_per_page / 2 - 1); i > 0; i--) {
		if ((size_increased * i) < (1024 * cnt))
			break;
		cnt++;
	}

	return cnt;
}

/*
 *Name         :
 *Description  :
 *Parameter    :
 *Return       : 0:ok  -1:fail
 *Note         :
 */
int nand_reset_super_chip(struct nand_super_chip_info *nsci, unsigned int super_chip_no)
{
	return 0;
}

/*
 *Name         :
 *Description  :
 *Parameter    :
 *Return       : 0:ok  -1:fail
 *Note         :
 */
int nand_read_super_chip_status(struct nand_super_chip_info *nsci, unsigned int super_chip_no)
{
	return 0;
}

/*
 *Name         :
 *Description  :
 *Parameter    :
 *Return       : 0:ok  -1:fail
 *Note         :
 */
int is_super_chip_rb_ready(struct nand_super_chip_info *nsci, unsigned int super_chip_no)
{
	return 0;
}

/*
 *Name         :
 *Description  :
 *Parameter    :
 *Return       : 0:ok  -1:fail
 *Note         :
 */
int nand_wait_all_rb_ready(void)
{
	int ret = 0;
	struct nand_controller_info *nctri = g_nctri;

	while (nctri != NULL) {
		ret |= ndfc_wait_all_rb_ready(nctri);
		nctri = nctri->next;
	}
	return ret;
}

char *rawnand_get_chip_name(struct nand_chip_info *chip)
{
	struct sunxi_nand_flash_device *info = chip->npi;
	return info->name;
}
void rawnand_get_chip_id(struct nand_chip_info *chip, unsigned char *id,
		int cnt)
{
	struct sunxi_nand_flash_device *info = chip->npi;
	memcpy(id, info->id, cnt);
}
unsigned int rawnand_get_chip_die_cnt(struct nand_chip_info *chip)
{
	struct sunxi_nand_flash_device *info = chip->npi;
	return info->die_cnt_per_chip;
}
int rawnand_get_chip_page_size(struct nand_chip_info *chip,
		enum size_type type)
{
	struct sunxi_nand_flash_device *info = chip->npi;
	if (likely(type == SECTOR)) {
		return info->sect_cnt_per_page;
	} else if (unlikely(type == BYTE)) {
		return info->sect_cnt_per_page * SECTOR_SIZE;
	} else {
		pr_err("no this type:%d single page size in BYTE(0)@byte "
				"SECTOR(1)@sector", type);
		return ERR_NO_22;
	}
}
int rawnand_get_chip_block_size(struct nand_chip_info *chip,
		enum size_type type)
{
	struct sunxi_nand_flash_device *info = chip->npi;
	if (likely(type == PAGE)) {
		return info->page_cnt_per_blk;
	} else if (unlikely(type == SECTOR)) {
		return info->page_cnt_per_blk * info->sect_cnt_per_page;
	} else if (unlikely(type == BYTE)) {
		return info->page_cnt_per_blk * info->sect_cnt_per_page
						* SECTOR_SIZE;
	} else {
		pr_err("no this type:%d single block size in BYTE(0)@byte "
				"SECTOR(1)@sector PAGE(2)@page", type);
		return ERR_NO_22;
	}
}
int rawnand_get_chip_die_size(struct nand_chip_info *chip,
		enum size_type type)
{
	struct sunxi_nand_flash_device *info = chip->npi;
	if (likely(type == BLOCK))
		return info->blk_cnt_per_die;
	else if (unlikely(type == BYTE))
		return info->blk_cnt_per_die * info->page_cnt_per_blk
			* info->sect_cnt_per_page * SECTOR_SIZE;
	else if (unlikely(type == SECTOR))
		return info->blk_cnt_per_die * info->page_cnt_per_blk
			* info->sect_cnt_per_page;
	else if (unlikely(type == PAGE))
		return info->blk_cnt_per_die * info->page_cnt_per_blk;
	else {
		pr_err("no this type:%d die size in BYTE(0)@byte "
			"SECTOR(1)@sector PAGE(2)@page BLOCK(3)@block", type);
		return ERR_NO_22;
	}
}
unsigned long long rawnand_get_chip_opt(struct nand_chip_info *chip)
{
	struct sunxi_nand_flash_device *info = chip->npi;
	return info->operation_opt;
}
unsigned int rawnand_get_chip_ecc_mode(struct nand_chip_info *chip)
{
	struct sunxi_nand_flash_device *info = chip->npi;
	return info->ecc_mode;
}
unsigned int rawnand_get_chip_freq(struct nand_chip_info *chip)
{
	struct sunxi_nand_flash_device *info = chip->npi;
	return info->access_freq;
}

/**
 * nand_chip_init: rawnand chip init
 * @nci: rawnand chip info
 * @c  : number of the chip in the channel
 */
static int rawnand_chip_init(struct nand_chip_info *nci, int c)
{
	struct sunxi_nand_flash_device *nand_id_tbl = NULL;
	unsigned char id[8] = {0};

	if (nci == NULL) {
		RAWNAND_ERR("rawnand err: %s nci is null\n", __func__);
		return ERR_NO_12;
	}


	if (nand_first_reset_chip(nci, c) != NAND_OP_TRUE) {
		RAWNAND_ERR("rawnand err: %s first reset chip fail\n", __func__);
		return ERR_NO_13;
	}

	if (nand_first_read_id(nci, c, id)) {
		RAWNAND_ERR("rawnand err:%s first read id fail\n", __func__);
		return ERR_NO_14;
	}

	nand_id_tbl = sunxi_search_id(nci, id);
	if (nand_id_tbl == NULL) {
		RAWNAND_ERR("rawnand not support chip %d: "
				"%02x %02x %02x %02x %02x %02x %02x %02x\n",
				c, id[0], id[1], id[2], id[3], id[4], id[5],
				id[6], id[7]);
		return ERR_NO_15;
	} else {
		RAWNAND_INFO("rawnand support chip %d: "
				"%02x %02x %02x %02x %02x %02x %02x %02x\n",
				c, id[0], id[1], id[2], id[3], id[4], id[5],
				id[6], id[7]);
	}

	if (init_nci_from_id(nci, nand_id_tbl) != NAND_OP_TRUE) {
		RAWNAND_ERR("rawnand err: %s init nci fail\n", __func__);
		return ERR_NO_16;
	}


	return NAND_OP_TRUE;
}

/**
 * nand_chips_init: rawnand chips init
 *
 * @chip: chip info
 * @info: nand info
 *
 *
 */
int rawnand_chips_init(struct nand_chip_info *nci)
{
	struct nand_chip_info *inci = NULL;
	struct nand_chip_info *inci_temp = NULL;
	int c = 0;
	int ret = 0;
	struct _nand_storage_info *nsi = g_nsi;

	struct nand_controller_info *nctri = in_container_of(nci,
			struct nand_controller_info, nci);


	if (nsi == NULL) {
		RAWNAND_ERR("%s no memory for nsi\n", __func__);
		return ERR_NO_12;
	}

	for (c = 0; c < MAX_CHIP_PER_CHANNEL; c++) {
		inci = nand_malloc(sizeof(struct nand_chip_info));
		if (inci == NULL) {
			RAWNAND_ERR("rawnand err: cannot get mem for nci");
			return ERR_NO_12; /*-ENOMEM*/
		}

		memset(inci, 0, sizeof(struct nand_chip_info));

		inci->nctri = nctri;
		inci->chip_no = nsi->chip_cnt;
		/*init chip*/
		ret = rawnand_chip_init(inci, c);
		if (ret == ERR_NO_15) {
			nand_free(inci);
			if (nsi->chip_cnt == 0) {
				RAWNAND_ERR("no rawnand  found!!!\n");
				return ret = NAND_OP_FALSE;
			} else
				return ret = NAND_OP_TRUE;
		} else if (ret != NAND_OP_TRUE) {
			break;
		}

		nsi->chip_cnt++;
		nci_add_to_nsi(nsi, inci);

		nctri->chip_cnt++;
		nctri->chip_connect_info |= (0x1U << c);
		nci_add_to_nctri(nctri, inci);

		/*the channel's chip should use the same id item*/
		if (c > 0) {
			if (inci_temp->npi->id_number != inci->npi->id_number) {
				RAWNAND_ERR("%s chip@%d and chip@%d no use the "
					"same id item\n", __func__, c - 1, c);

				goto err0;
			}
		}
		inci_temp = inci;

	}

	if (nctri->chip_cnt == 0) {
		RAWNAND_ERR("no rawnand found!\n");
		ret = NAND_OP_FALSE;
	} else
		ret = NAND_OP_TRUE;

	return ret;

err0:
	nci_delete_from_nsi(nsi);
	nci_delete_from_nctri(nctri);

	return NAND_OP_FALSE;
}

/**
 * rawnand_chip_special_init: init different flash special request eg. readretry
 * @type: readretry type
 *	NAND_READRETRY_NO: flash that no readretry
 *	NAND_READRETRY_HYNIX_16NM: hynix 16nm flash that has readretry
 *	NAND_READRETRY_HYNIX_20NM: hynix 20nm flash that has readretry
 *	NAND_READRETRY_HYNIX_26NM: hynix 26nm flash that has readretry
 *	NAND_READRETRY_MICRON: micron flash that has readretry
 *	NAND_READRETRY_SAMSUNG: samsung flash that has readretry
 *	NAND_READRETRY_TOSHIBA: toshiba flash that has readretry
 *	NADN_READRETRY_SANDISK: sandisk flash that has readretry
 *	NAND_READRETRY_SANDISK_A19: sandisk A19 flash that has readretry
 */

void rawnand_chip_special_init(enum nand_readretry_type type)
{
	switch (type) {
	case NAND_READRETRY_NO:
		generic_special_init();
		break;
	case NAND_READRETRY_HYNIX_16NM:
		hynix16nm_special_init();
		break;
	case NAND_READRETRY_HYNIX_20NM:
		hynix20nm_special_init();
		break;
	case NAND_READRETRY_HYNIX_26NM:
		hynix26nm_special_init();
		break;
	case NAND_READRETRY_MICRON:
		micron_special_init();
		break;
	case NAND_READRETRY_SAMSUNG:
		samsung_special_init();
		break;
	case NAND_READRETRY_TOSHIBA:
		toshiba_special_init();
		break;
	case NAND_READRETRY_SANDISK:
		sandisk_special_init();
		break;
	case NAND_READRETRY_SANDISK_A19:
		sandisk_A19_special_init();
		break;
	default:
		generic_special_init();
		break;
	}
}

/**
 * rawnand_chip_special_exit: exit different flash special request eg. readretry
 * @type: readretry type
 *	NAND_READRETRY_NO: flash that no readretry
 *	NAND_READRETRY_HYNIX_16NM: hynix 16nm flash that has readretry
 *	NAND_READRETRY_HYNIX_20NM: hynix 20nm flash that has readretry
 *	NAND_READRETRY_HYNIX_26NM: hynix 26nm flash that has readretry
 *	NAND_READRETRY_MICRON: micron flash that has readretry
 *	NAND_READRETRY_SAMSUNG: samsung flash that has readretry
 *	NAND_READRETRY_TOSHIBA: toshiba flash that has readretry
 *	NADN_READRETRY_SANDISK: sandisk flash that has readretry
 *	NAND_READRETRY_SANDISK_A19: sandisk A19 flash that has readretry
 */
void rawnand_chip_special_exit(enum nand_readretry_type type)
{
	switch (type) {
	case NAND_READRETRY_NO:
		generic_special_exit();
		break;
	case NAND_READRETRY_HYNIX_16NM:
		hynix16nm_special_exit();
		break;
	case NAND_READRETRY_HYNIX_20NM:
		hynix20nm_special_exit();
		break;
	case NAND_READRETRY_HYNIX_26NM:
		hynix26nm_special_exit();
		break;
	case NAND_READRETRY_MICRON:
		micron_special_exit();
		break;
	case NAND_READRETRY_SAMSUNG:
		samsung_special_exit();
		break;
	case NAND_READRETRY_TOSHIBA:
		toshiba_special_exit();
		break;
	case NAND_READRETRY_SANDISK:
		sandisk_special_exit();
		break;
	case NAND_READRETRY_SANDISK_A19:
		sandisk_A19_special_exit();
		break;
	default:
		generic_special_exit();
		break;
	}
}

/**
 * rawnand_sp_chips_init: build nand super storage info
 * @nsci: nand super chip info
 *
 * 规则：

 * 1.一个通道内需要贴同一种flash
 * 2.两个通道应该贴同样数目和类型的flash
 *
 *	单通道
 *		1.支持 two-plane
 *		2.支持 vertical_interleave
 *		3.如果超级页超过32k，不支持two-plane
 *		4.vertical_interleave 通道内rb不相同的chip配对
 *
 *	 双通道
 *	1.支持 two-plane
 *	2.支持dual_channel
 *	3.支持vertical_interleave
 *	4.如果超级页超过32k，不支持two-plane
 *	5.dual_channel 通道间chip0和chip0配对
 *	6.vertical_interleave 通道内rb不相同的chip配对
 */
int rawnand_sp_chips_init(struct nand_super_chip_info *nsci)
{

	int rb1, rb2, rb0, ret;
	int i, channel_num, nsci_num, nsci_num_in_nctri;
	struct nand_controller_info *nctri_temp;
	struct nand_super_chip_info *insci;
	struct nand_controller_info *nctri = g_nctri;
	struct _nand_super_storage_info *nssi = in_container_of(nsci,
			struct _nand_super_storage_info, nsci);

	if (nctri == NULL) {
		RAWNAND_ERR("rawnand err: %s nctri is null\n", __func__);
	}

	ret = get_nand_structure(nssi);
	if (ret != 0) {
		RAWNAND_ERR("%s get nand struction fail\n", __func__);
		return ret;
	}

	nsci_num = 0;

	for (channel_num = 0, nctri_temp = nctri; nctri_temp; nctri_temp = nctri_temp->next) {
		if (nctri_temp->chip_cnt != 0)
			channel_num++;
	}

	if (nctri->chip_cnt == 1) {
		nssi->support_v_interleave = 0;
	} else if (nctri->chip_cnt == 2) {
		rb0 = nctri->rb[0];
		rb1 = nctri->rb[1];
		if (rb0 == rb1) {
			nssi->support_v_interleave = 0;
		}
	} else {
		/*must be nctri->chip_cnt == 4*/
		rb0 = nctri->rb[0];
		rb1 = nctri->rb[1];
		rb2 = nctri->rb[2];
		//        rb3 = nctri->rb[3];
		if ((rb0 == rb1) && (rb0 == rb2)) {
			nssi->support_v_interleave = 0;
		}
	}

	if (channel_num == 1) {
		if ((nctri->chip_cnt == 1) || (nctri->chip_cnt == 2) || (nctri->chip_cnt == 4)) {
			if (nssi->support_v_interleave != 0) {
				nsci_num = nctri->chip_cnt / 4 + 1;
			} else {
				nsci_num = nctri->chip_cnt;
			}
		} else {
			RAWNAND_ERR("not support chip_cnt1 %u\n", nctri->chip_cnt);
			return NAND_OP_FALSE;
		}
		nsci_num_in_nctri = nsci_num;
	} else if (channel_num == 2) {
		if ((nctri->chip_cnt == 1) || (nctri->chip_cnt == 2) || (nctri->chip_cnt == 4)) {
			if (nssi->support_dual_channel != 0) {
				nsci_num = nctri->chip_cnt;
			} else {
				nsci_num = nctri->chip_cnt << 1;
			}

			nsci_num_in_nctri = nctri->chip_cnt;

			if (nssi->support_v_interleave != 0) {
				nsci_num >>= 1;
				nsci_num_in_nctri >>= 1;
			}
		} else {
			RAWNAND_ERR("not support chip_cnt2 %u\n", nctri->chip_cnt);
			return NAND_OP_FALSE;
		}
	} else {
		RAWNAND_ERR("not support channel_num %d\n", channel_num);
		return NAND_OP_FALSE;
	}

	for (i = 0; i < nsci_num; i++) {
		insci = &nsci_data[i];
		if (insci == NULL) {
			RAWNAND_ERR("no memory for nssi\n");
		}

		memset(insci, 0, sizeof(struct nand_super_chip_info));

		init_nsci_from_nctri(nssi, insci, nctri, channel_num, i, nsci_num_in_nctri);

		nsci_add_to_nssi(nssi, insci);

		insci = insci->nssi_next;
	}

	nssi->super_chip_cnt = nsci_num;

	if (nsci_num == 0) {
		RAWNAND_ERR("not support chip_cnt %d %d\n", channel_num, nctri->chip_cnt);
		return NAND_OP_FALSE;
	} else {
		RAWNAND_DBG("%s %d\n", __func__, __LINE__);
		return NAND_OP_TRUE;
	}
}

/**
 * nand_chips_cleanup: rawnand exit
 *
 * @chip: chip info
 *
 *
 */
void rawnand_chips_cleanup(struct nand_chip_info *chip)
{
	return;
}

/**
 * nand_chips_super_standby: rawnand super standby
 *
 * @chip: chip info
 *
 *
 */
int rawnand_chips_super_standby(struct nand_chip_info *chip)
{
	return 0;
}

/**
 * nand_chips_super_resume: rawnand super resume
 *
 * @chip: chip info
 *
 *
 */
int rawnand_chips_super_resume(struct nand_chip_info *chip)
{
	return 0;
}

/**
 * nand_chips_normal_standby: rawnand normal standby
 *
 * @chip: chip info
 *
 *
 */
int rawnand_chips_normal_standby(struct nand_chip_info *chip)
{
	return 0;
}

/**
 * nand_chips_normal_resume: rawnand normal resume
 *
 * @chip: chip info
 *
 *
 */
int rawnand_chips_normal_resume(struct nand_chip_info *chip)
{
	return 0;
}

/*
 *struct physic_special_ops special_ops = {
 *    .nand_physic_special_init = NULL,
 *    .nand_physic_special_exit = NULL,
 *};
 */

/*
 *struct nand_chips_ops rawnand_chips_ops = {
 *    .nand_chips_init = rawnand_chips_init,
 *    .nand_chips_cleanup = rawnand_chips_cleanup,
 *#ifdef SUPPORT_SUPER_STANDBY
 *    .nand_chips_standby = rawnand_chips_super_standby,
 *    .nand_chips_resume = rawnand_chips_super_resume,
 *#else
 *    .nand_chips_standby = rawnand_chips_super_standby,
 *    .nand_chips_resume = rawnand_chips_super_resume,
 *#endif
 *};
 */
