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
#include <math.h>

#include "SIM_parameter.h"
#include "SIM_array.h"
#include "SIM_router.h"
#include "SIM_util.h"

int SIM_router_area_init(SIM_router_info_t *info, SIM_router_area_t *router_area)
{
	double bitline_len, wordline_len, xb_in_len, xb_out_len;
	double depth, nMUX, boxArea;
	int req_width;
	router_area->buffer = 0;
	router_area->crossbar = 0;
	router_area->vc_allocator = 0;
	router_area->sw_allocator = 0;

	/* buffer area */
	/* input buffer area */
	if (info->in_buf) {
		switch (info->in_buffer_model) {
			case SRAM:
				bitline_len = info->in_buf_info.n_set * (RegCellHeight + 2 * WordlineSpacing);
				wordline_len = info->flit_width * (RegCellWidth + 2 * (info->in_buf_info.read_ports 
							+ info->in_buf_info.write_ports) * BitlineSpacing);

				/* input buffer area */
				router_area->buffer = info->n_in * info->n_v_class * (bitline_len * wordline_len) *
										(info->in_share_buf ? 1 : info->n_v_channel );
				break;

			case REGISTER:
				router_area->buffer = AreaDFF * info->n_in * info->n_v_class * info->flit_width *
					info->in_buf_info.n_set * (info->in_share_buf ? 1 : info->n_v_channel );
				break;

			default: printf ("error\n");  /* some error handler */
		}
	}

	/* output buffer area */
	if (info->out_buf) {
		switch (info->out_buffer_model) {
			case SRAM:
				bitline_len = info->out_buf_info.n_set * (RegCellHeight + 2 * WordlineSpacing);
				wordline_len = info->flit_width * (RegCellWidth + 2 * (info->out_buf_info.read_ports 
							+ info->out_buf_info.write_ports) * BitlineSpacing);

				/* output buffer area */
				router_area->buffer += info->n_out * info->n_v_class * (bitline_len * wordline_len) *
									(info->out_share_buf ? 1 : info->n_v_channel );
				break;

			case REGISTER:
				router_area->buffer += AreaDFF * info->n_out * info->n_v_class * info->flit_width * 
					info->out_buf_info.n_set * (info->out_share_buf ? 1 : info->n_v_channel ) ; 
				break;

			default: printf ("error\n");  /* some error handler */
		}
	}

	/* crossbar area */
	if (info->crossbar_model && info->crossbar_model < CROSSBAR_MAX_MODEL) {
		switch (info->crossbar_model) {
			case MATRIX_CROSSBAR:
				xb_in_len = info->n_switch_in * info->flit_width * CrsbarCellWidth;  
				xb_out_len = info->n_switch_out * info->flit_width * CrsbarCellHeight;  
				router_area->crossbar = xb_in_len * xb_out_len;
				break;

			case MULTREE_CROSSBAR:
				if(info->degree == 2) {
					depth = ceil((log(info->n_switch_in) / log(2)));  
					nMUX = pow(2,depth) - 1;
					boxArea = 1.5 *nMUX * AreaMUX2;
					router_area->crossbar = info->n_switch_in * info->flit_width *boxArea * info->n_switch_out; 
				}
				else if( info->degree == 3 ) {
					depth = ceil((log(info->n_switch_in) / log(3))); 
					nMUX = ((pow(3,depth) - 1) / 2);
					boxArea = 1.5 * nMUX * AreaMUX3;
					router_area->crossbar = info->n_switch_in * info->flit_width *boxArea * info->n_switch_out; 
				}
				else if( info->degree == 4 ) {
					depth = ceil((log(info->n_switch_in) / log(4)));
					nMUX = ((pow(4,depth) - 1) / 3);
					boxArea = 1.5 * nMUX * AreaMUX4;
					router_area->crossbar = info->n_switch_in * info->flit_width * boxArea * info->n_switch_out; 
				}
				break;

			default: printf ("error\n");  /* some error handler */

		}
	}

	if (info->exp_xb_model){ //only support for MATRIX_CROSSBAR type
		xb_in_len = (2 *info->n_switch_in - 1) * info->flit_width * CrsbarCellWidth; 
		xb_out_len = (2 * info->n_switch_out - 1) * info->flit_width * CrsbarCellHeight; 
		router_area->crossbar = xb_in_len * xb_out_len;
	}

	/* switch allocator area */
	if (info->sw_in_arb_model) {
		req_width = info->n_v_channel * info->n_v_class;

		switch (info->sw_in_arb_model) {	
			case MATRIX_ARBITER:  //assumes 30% spacing for each arbiter
				router_area->sw_allocator += ((AreaNOR * 2 * (req_width - 1) * req_width) + (AreaINV * req_width) 
						+ (AreaDFF * (req_width * (req_width - 1)/2))) * 1.3 * info->in_n_switch * info->n_in;
				break;

			case RR_ARBITER: //assumes 30% spacing for each arbiter
				router_area->sw_allocator += ((6 *req_width * AreaNOR) + (2 * req_width * AreaINV) 
											+ (req_width * AreaDFF)) * 1.3 * info->in_n_switch * info->n_in;
				break;

			case QUEUE_ARBITER: 
				router_area->sw_allocator += AreaDFF * info->sw_in_arb_queue_info.n_set * info->sw_in_arb_queue_info.data_width
					* info->in_n_switch * info->n_in;

				break;

			default: printf ("error\n");  /* some error handler */	
		}
	}

	if (info->sw_out_arb_model) {
		req_width = info->n_total_in - 1;

		switch (info->sw_out_arb_model) {
			case MATRIX_ARBITER: //assumes 30% spacing for each arbiter
				router_area->sw_allocator += ((AreaNOR * 2 * (req_width - 1) * req_width) + (AreaINV * req_width)
						+ (AreaDFF * (req_width * (req_width - 1)/2))) * 1.3 * info->n_switch_out;
				break;

			case RR_ARBITER: //assumes 30% spacing for each arbiter
				router_area->sw_allocator += ((6 *req_width * AreaNOR) + (2 * req_width * AreaINV) + (req_width * AreaDFF)) * 1.3 * info->n_switch_out;
				break;

			case QUEUE_ARBITER:
				router_area->sw_allocator += AreaDFF * info->sw_out_arb_queue_info.data_width
					* info->sw_out_arb_queue_info.n_set * info->n_switch_out;
				break;

			default: printf ("error\n");  /* some error handler */  


		}
	}


	/* virtual channel allocator area */
	if(info->vc_allocator_type == ONE_STAGE_ARB && info->n_v_channel > 1 && info->n_in > 1){
		if (info->vc_out_arb_model){
			req_width = (info->n_in - 1) * info->n_v_channel;
			switch (info->vc_out_arb_model){
				case MATRIX_ARBITER: //assumes 30% spacing for each arbiter
					router_area->vc_allocator = ((AreaNOR * 2 * (req_width - 1) * req_width) + (AreaINV * req_width)
							+ (AreaDFF * (req_width * (req_width - 1)/2))) * 1.3 * info->n_out * info->n_v_channel * info->n_v_class;
					break;

				case RR_ARBITER: //assumes 30% spacing for each arbiter
					router_area->vc_allocator = ((6 *req_width * AreaNOR) + (2 * req_width * AreaINV) + (req_width * AreaDFF)) * 1.3  
												* info->n_out * info->n_v_channel * info->n_v_class;
					break;

				case QUEUE_ARBITER:
					router_area->vc_allocator = AreaDFF * info->vc_out_arb_queue_info.data_width 
						* info->vc_out_arb_queue_info.n_set * info->n_out * info->n_v_channel * info->n_v_class;

					break;

				default: printf ("error\n");  /* some error handler */
			}
		}

	}
	else if(info->vc_allocator_type == TWO_STAGE_ARB && info->n_v_channel > 1 && info->n_in > 1){
		if (info->vc_in_arb_model && info->vc_out_arb_model){
			/*first stage*/
			req_width = info->n_v_channel;
			switch (info->vc_in_arb_model) {
				case MATRIX_ARBITER: //assumes 30% spacing for each arbiter
					router_area->vc_allocator = ((AreaNOR * 2 * (req_width - 1) * req_width) + (AreaINV * req_width)
							+ (AreaDFF * (req_width * (req_width - 1)/2))) * 1.3 * info->n_in * info->n_v_channel * info->n_v_class;
					break;

				case RR_ARBITER: //assumes 30% spacing for each arbiter
					router_area->vc_allocator = ((6 *req_width * AreaNOR) + (2 * req_width * AreaINV) + (req_width * AreaDFF)) * 1.3 
										* info->n_in * info->n_v_channel * info->n_v_class ;
					break;

				case QUEUE_ARBITER:
					router_area->vc_allocator = AreaDFF * info->vc_in_arb_queue_info.data_width
						* info->vc_in_arb_queue_info.n_set * info->n_in * info->n_v_channel * info->n_v_class ; 

					break;

				default: printf ("error\n");  /* some error handler */
			}

			/*second stage*/
			req_width = (info->n_in - 1) * info->n_v_channel;
			switch (info->vc_out_arb_model) {
				case MATRIX_ARBITER: //assumes 30% spacing for each arbiter
				router_area->vc_allocator += ((AreaNOR * 2 * (req_width - 1) * req_width) + (AreaINV * req_width)
						+ (AreaDFF * (req_width * (req_width - 1)/2))) * 1.3 * info->n_out * info->n_v_channel * info->n_v_class;
				break;

				case RR_ARBITER: //assumes 30% spacing for each arbiter
				router_area->vc_allocator += ((6 *req_width * AreaNOR) + (2 * req_width * AreaINV) + (req_width * AreaDFF)) * 1.3
										* info->n_out * info->n_v_channel * info->n_v_class;
				break;

				case QUEUE_ARBITER:
				router_area->vc_allocator += AreaDFF * info->vc_out_arb_queue_info.data_width
					* info->vc_out_arb_queue_info.n_set * info->n_out * info->n_v_channel * info->n_v_class;

				break;

				default: printf ("error\n");  /* some error handler */
			}


		}
	}
	else if(info->vc_allocator_type == VC_SELECT && info->n_v_channel > 1) {
		switch (info->vc_select_buf_type) {
			case SRAM:
				bitline_len = info->n_v_channel * (RegCellHeight + 2 * WordlineSpacing);
				wordline_len = SIM_logtwo(info->n_v_channel) * (RegCellWidth + 2 * (info->vc_select_buf_info.read_ports
							+ info->vc_select_buf_info.write_ports) * BitlineSpacing);
				router_area->vc_allocator = info->n_out * info->n_v_class * (bitline_len * wordline_len);

				break;

			case REGISTER:
				router_area->vc_allocator = AreaDFF * info->n_out * info->n_v_class* info->vc_select_buf_info.data_width
					* info->vc_select_buf_info.n_set;

				break;

			default: printf ("error\n");  /* some error handler */

		}
	}

	return 0;

}

double SIM_router_area(SIM_router_area_t *router_area)
{
	double Atotal;
	Atotal = router_area->buffer + router_area->crossbar + router_area->vc_allocator + router_area->sw_allocator;

#if( PARM(TECH_POINT) <= 90 )
	fprintf(stdout, "Abuffer:%g\t ACrossbar:%g\t AVCAllocator:%g\t ASWAllocator:%g\t Atotal:%g\n", router_area->buffer, router_area->crossbar, router_area->vc_allocator, router_area->sw_allocator,  Atotal);	

#else
    fprintf(stderr, "Router area is only supported for 90nm, 65nm, 45nm and 32nm\n");
#endif
	return Atotal;
}

