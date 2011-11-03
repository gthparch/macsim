/*-------------------------------------------------------------------------
 *                             ORION 2.0 
 *
 *         					Copyright 2009 
 *  	Princeton University, and Regents of the University of California 
 *                         All Rights Reserved
 *
 *                         
 *  ORION 2.0 was developed by Bin Li at Princeton University and Kambiz Samadi at
 *  University of California, San Diego. ORION 2.0 was built on top of ORION 1.0. 
 *  ORION 1.0 was developed by Hangsheng Wang, Xinping Zhu and Xuning Chen at 
 *  Princeton University.
 *
 *  If your use of this software contributes to a published paper, we
 *  request that you cite our paper that appears on our website 
 *  http://www.princeton.edu/~peh/orion.html
 *
 *  Permission to use, copy, and modify this software and its documentation is
 *  granted only under the following terms and conditions.  Both the
 *  above copyright notice and this permission notice must appear in all copies
 *  of the software, derivative works or modified versions, and any portions
 *  thereof, and both notices must appear in supporting documentation.
 *
 *  This software may be distributed (but not offered for sale or transferred
 *  for compensation) to third parties, provided such third parties agree to
 *  abide by the terms and conditions of this notice.
 *
 *  This software is distributed in the hope that it will be useful to the
 *  community, but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
 *
 *-----------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "SIM_router.h"
#include "SIM_util.h"

/* global variables */
GLOBDEF(SIM_router_power_t, router_power);
GLOBDEF(SIM_router_info_t, router_info);
GLOBDEF(SIM_router_area_t, router_area);

int SIM_router_init(SIM_router_info_t *info, SIM_router_power_t *router_power, SIM_router_area_t *router_area)
{
	u_int line_width;
	int share_buf, outdrv;

	/* PHASE 1: set parameters */
	/* general parameters */
	info->n_in = PARM(in_port);
	info->n_cache_in = PARM(cache_in_port);
	info->n_mc_in = PARM(mc_in_port);
	info->n_io_in = PARM(io_in_port);
	info->n_total_in = PARM(in_port) + PARM(cache_in_port) + PARM(mc_in_port) + PARM(io_in_port);
	info->n_out = PARM(out_port);
	info->n_cache_out = PARM(cache_out_port);
	info->n_mc_out = PARM(mc_out_port);
	info->n_io_out = PARM(io_out_port);
	info->n_total_out = PARM(out_port) + PARM(cache_out_port) + PARM(mc_out_port) + PARM(io_out_port);
	info->flit_width = PARM(flit_width);

	/* virtual channel parameters */
	info->n_v_channel = MAX(PARM(v_channel), 1);
	info->n_v_class = MAX(PARM(v_class), 1); 
	info->cache_class = MAX(PARM(cache_class), 1);
	info->mc_class = MAX(PARM(mc_class), 1);
	info->io_class = MAX(PARM(io_class), 1);
	/* shared buffer implies buffer has tags */
	/* separate buffer & shared switch implies buffer has tri-state output driver*/
	if (info->n_v_class * info->n_v_channel > 1) {
		info->in_share_buf = PARM(in_share_buf);
		info->out_share_buf = PARM(out_share_buf);
		info->in_share_switch = PARM(in_share_switch);
		info->out_share_switch = PARM(out_share_switch);
	}
	else {
		info->in_share_buf = 0;
		info->out_share_buf = 0;
		info->in_share_switch = 0;
		info->out_share_switch = 0;
	}

	/* crossbar */
	info->crossbar_model = PARM(crossbar_model);
	info->degree = PARM(crsbar_degree);
	info->connect_type = PARM(connect_type);
	info->trans_type = PARM(trans_type);
	info->xb_in_seg = PARM(xb_in_seg);
	info->xb_out_seg = PARM(xb_out_seg);
	info->crossbar_in_len = PARM(crossbar_in_len);
	info->crossbar_out_len = PARM(crossbar_out_len);
	/* HACK HACK HACK */
	info->exp_xb_model = PARM(exp_xb_model);
	info->exp_in_seg = PARM(exp_in_seg);
	info->exp_out_seg = PARM(exp_out_seg);

	/* input buffer */
	info->in_buf = PARM(in_buf);
	info->in_buffer_model = PARM(in_buffer_type);
	if(info->in_buf){
		outdrv = !info->in_share_buf && info->in_share_switch;
		SIM_array_init(&info->in_buf_info, 1, PARM(in_buf_rport), 1, PARM(in_buf_set), PARM(flit_width), outdrv, info->in_buffer_model);
	}

	if (PARM(cache_in_port)){
    	info->cache_in_buf = PARM(cache_in_buf);

		if (info->cache_in_buf){
			if (PARM(cache_class) > 1){
    			share_buf = info->in_share_buf;
    			outdrv = !share_buf && info->in_share_switch;
			}	
			else{
    			outdrv = share_buf = 0;
			}
    		SIM_array_init(&info->cache_in_buf_info, 1, PARM(cache_in_buf_rport), 1, PARM(cache_in_buf_set), PARM(flit_width), outdrv, SRAM);
		}
	}

	if (PARM(mc_in_port)){
    	info->mc_in_buf = PARM(mc_in_buf);

		if (info->mc_in_buf){
			if (PARM(mc_class) > 1){
    			share_buf = info->in_share_buf;
    			outdrv = !share_buf && info->in_share_switch;
			}
			else{
    			outdrv = share_buf = 0;
			}
    		SIM_array_init(&info->mc_in_buf_info, 1, PARM(mc_in_buf_rport), 1, PARM(mc_in_buf_set), PARM(flit_width), outdrv, SRAM);
		}
	}

	if (PARM(io_in_port)){
    	info->io_in_buf = PARM(io_in_buf);

		if (info->io_in_buf){
			if (PARM(io_class) > 1){
    			share_buf = info->in_share_buf;
    			outdrv = !share_buf && info->in_share_switch;
			}
			else{
    			outdrv = share_buf = 0;
			}
    		SIM_array_init(&info->io_in_buf_info, 1, PARM(io_in_buf_rport), 1, PARM(io_in_buf_set), PARM(flit_width), outdrv, SRAM);
		}
	}

	/* output buffer */
	info->out_buf = PARM(out_buf);
	info->out_buffer_model = PARM(out_buffer_type);
	if (info->out_buf){
		/* output buffer has no tri-state buffer anyway */
		SIM_array_init(&info->out_buf_info, 1, 1, PARM(out_buf_wport), PARM(out_buf_set), PARM(flit_width), 0, info->out_buffer_model);
	}

	/* central buffer */
	info->central_buf = PARM(central_buf);
	if (info->central_buf){
		info->pipe_depth = PARM(pipe_depth);
		/* central buffer is no FIFO */
		SIM_array_init(&info->central_buf_info, 0, PARM(cbuf_rport), PARM(cbuf_wport), PARM(cbuf_set), PARM(cbuf_width) * PARM(flit_width), 0, SRAM);
		/* dirty hack */
		info->cbuf_ff_model = NEG_DFF;
	}

	/* switch allocator input port arbiter */
	if (info->n_v_class * info->n_v_channel > 1) {
		if (info->sw_in_arb_model = PARM(sw_in_arb_model)) {
			if (PARM(sw_in_arb_model) == QUEUE_ARBITER) {
				SIM_array_init(&info->sw_in_arb_queue_info, 1, 1, 1, info->n_v_class*info->n_v_channel, SIM_logtwo(info->n_v_class*info->n_v_channel), 0, REGISTER);
				if (info->cache_class > 1)
					SIM_array_init(&info->cache_in_arb_queue_info, 1, 1, 1, info->cache_class, SIM_logtwo(info->cache_class), 0, REGISTER);
				if (info->mc_class > 1)
					SIM_array_init(&info->mc_in_arb_queue_info, 1, 1, 1, info->mc_class, SIM_logtwo(info->mc_class), 0, REGISTER);
				if (info->io_class > 1)
					SIM_array_init(&info->io_in_arb_queue_info, 1, 1, 1, info->io_class, SIM_logtwo(info->io_class), 0, REGISTER);

				info->sw_in_arb_ff_model = SIM_NO_MODEL;
			}
			else
				info->sw_in_arb_ff_model = PARM(sw_in_arb_ff_model);
		}
		else
			info->sw_in_arb_ff_model = SIM_NO_MODEL;
	}
	else {
		info->sw_in_arb_model = SIM_NO_MODEL;
		info->sw_in_arb_ff_model = SIM_NO_MODEL;
	}

	/* switch allocator output port arbiter */
	if(info->n_total_in > 2){
		info->sw_out_arb_model = PARM(sw_out_arb_model);
		if (info->sw_out_arb_model) {
			if (info->sw_out_arb_model == QUEUE_ARBITER) {
				line_width = SIM_logtwo(info->n_total_in - 1);
				SIM_array_init(&info->sw_out_arb_queue_info, 1, 1, 1, info->n_total_in - 1, line_width, 0, REGISTER);
				info->sw_out_arb_ff_model = SIM_NO_MODEL;
			}
			else{
				info->sw_out_arb_ff_model = PARM(sw_out_arb_ff_model);
			}
		}
		else{
			info->sw_out_arb_ff_model = SIM_NO_MODEL;
		}
	}
	else{
		info->sw_out_arb_model = SIM_NO_MODEL;
		info->sw_out_arb_ff_model = SIM_NO_MODEL;
	}

	/* virtual channel allocator type */
	if (info->n_v_channel > 1) {
		info->vc_allocator_type = PARM(vc_allocator_type);
	} 
	else
		info->vc_allocator_type = SIM_NO_MODEL;

	/* virtual channel allocator input port arbiter */
	if ( info->n_v_channel > 1 && info->n_in > 1) {
		if (info->vc_in_arb_model = PARM(vc_in_arb_model)) {
			if (PARM(vc_in_arb_model) == QUEUE_ARBITER) { 
				SIM_array_init(&info->vc_in_arb_queue_info, 1, 1, 1, info->n_v_channel, SIM_logtwo(info->n_v_channel), 0, REGISTER);
				info->vc_in_arb_ff_model = SIM_NO_MODEL;
			}
			else{
				info->vc_in_arb_ff_model = PARM(vc_in_arb_ff_model);
			}
		}
		else {
			info->vc_in_arb_ff_model = SIM_NO_MODEL;
		}
	}
	else {
		info->vc_in_arb_model = SIM_NO_MODEL;
		info->vc_in_arb_ff_model = SIM_NO_MODEL;
	}

	/* virtual channel allocator output port arbiter */
	if(info->n_in > 1 && info->n_v_channel > 1){
		info->vc_out_arb_model = PARM(vc_out_arb_model);
		if (info->vc_out_arb_model) {
			if (info->vc_out_arb_model == QUEUE_ARBITER) {
				line_width = SIM_logtwo((info->n_total_in - 1)*info->n_v_channel);
				SIM_array_init(&info->vc_out_arb_queue_info, 1, 1, 1, (info->n_total_in -1) * info->n_v_channel, line_width, 0, REGISTER);
				info->vc_out_arb_ff_model = SIM_NO_MODEL;
			}
			else{
				info->vc_out_arb_ff_model = PARM(vc_out_arb_ff_model);
			}
		}
		else{
			info->vc_out_arb_ff_model = SIM_NO_MODEL;
		}
	}
	else{
		info->vc_out_arb_model = SIM_NO_MODEL;
		info->vc_out_arb_ff_model = SIM_NO_MODEL;
	}

	/*virtual channel allocation vc selection model */
	info->vc_select_buf_type = PARM(vc_select_buf_type);
	if(info->vc_allocator_type == VC_SELECT && info->n_v_channel > 1 && info->n_in > 1){
		info->vc_select_buf_type = PARM(vc_select_buf_type);
		SIM_array_init(&info->vc_select_buf_info, 1, 1, 1, info->n_v_channel, SIM_logtwo(info->n_v_channel), 0, info->vc_select_buf_type);
	}
	else{
		info->vc_select_buf_type = SIM_NO_MODEL;
	}


	/* redundant fields */
	if (info->in_buf) {
		if (info->in_share_buf)
			info->in_n_switch = info->in_buf_info.read_ports;
		else if (info->in_share_switch)
			info->in_n_switch = 1;
		else
			info->in_n_switch = info->n_v_class * info->n_v_channel;
	}
	else
		info->in_n_switch = 1;

	if (info->cache_in_buf) {
		if (info->in_share_buf)
			info->cache_n_switch = info->cache_in_buf_info.read_ports;
		else if (info->in_share_switch)
			info->cache_n_switch = 1;
		else
			info->cache_n_switch = info->cache_class;
	}
	else
		info->cache_n_switch = 1;

	if (info->mc_in_buf) {
		if (info->in_share_buf)
			info->mc_n_switch = info->mc_in_buf_info.read_ports;
		else if (info->in_share_switch)
			info->mc_n_switch = 1;
		else
			info->mc_n_switch = info->mc_class;
	}
	else
		info->mc_n_switch = 1;

	if (info->io_in_buf) {
		if (info->in_share_buf)
			info->io_n_switch = info->io_in_buf_info.read_ports;
		else if (info->in_share_switch)
			info->io_n_switch = 1;
		else
			info->io_n_switch = info->io_class;
	}
	else
		info->io_n_switch = 1;

	info->n_switch_in = info->n_in * info->in_n_switch + info->n_cache_in * info->cache_n_switch +
		info->n_mc_in * info->mc_n_switch + info->n_io_in * info->io_n_switch;

	/* no buffering for local output ports */
	info->n_switch_out = info->n_cache_out + info->n_mc_out + info->n_io_out;
	if (info->out_buf) {
		if (info->out_share_buf)
			info->n_switch_out += info->n_out * info->out_buf_info.write_ports;
		else if (info->out_share_switch)
			info->n_switch_out += info->n_out;
		else
			info->n_switch_out += info->n_out * info->n_v_class * info->n_v_channel;
	}
	else
		info->n_switch_out += info->n_out;

	/* clock related parameters */	
    info->pipeline_stages = PARM(pipeline_stages);
    info->H_tree_clock = PARM(H_tree_clock);
    info->router_diagonal = PARM(router_diagonal);

	/* PHASE 2: initialization */
	if(router_power){
		SIM_router_power_init(info, router_power);
	}

	if(router_area){
		SIM_router_area_init(info, router_area);
	}

	return 0;
}


/* ==================== buffer (wrapper functions) ==================== */

/* record read data activity */
int SIM_buf_power_data_read(SIM_array_info_t *info, SIM_array_t *arr, LIB_Type_max_uint data)
{
	/* precharge */
	SIM_array_pre_record(&arr->data_bitline_pre, info->blk_bits);
	/* drive the wordline */
	SIM_array_dec(info, arr, NULL, 0, SIM_ARRAY_READ);
	/* read data */
	SIM_array_data_read(info, arr, data);

	return 0;
}


/* record write data bitline and memory cell activity */
int SIM_buf_power_data_write(SIM_array_info_t *info, SIM_array_t *arr, u_char *data_line, u_char *old_data, u_char *new_data)
{
#define N_ITEM	(PARM(flit_width) / 8 + (PARM(flit_width) % 8 ? 1:0))
	/* drive the wordline */
	SIM_array_dec(info, arr, NULL, 0, SIM_ARRAY_WRITE);
	/* write data */
	SIM_array_data_write(info, arr, NULL, N_ITEM, data_line, old_data, new_data);

	return 0;
}

/* WHS: missing data output wrapper function */

/* ==================== buffer (wrapper functions) ==================== */
