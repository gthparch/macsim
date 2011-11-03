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

#ifndef _SIM_ROUTER_MODEL_H
#define _SIM_ROUTER_MODEL_H

typedef enum { 
	ONE_STAGE_ARB = 1,
	TWO_STAGE_ARB,
	VC_SELECT,
	VC_ALLOCATOR_MAX_MODEL
}SIM_vc_allocator_model_t;

typedef enum {
	SRAM = 1,
	REGISTER,
	BUFFER_MAX_MODEL
} SIM_buffer_model_t;

typedef enum {
	MATRIX_CROSSBAR = 1,
	MULTREE_CROSSBAR,
	CUT_THRU_CROSSBAR,
	CROSSBAR_MAX_MODEL
} SIM_crossbar_model_t;

typedef enum {
	RR_ARBITER = 1,
	MATRIX_ARBITER,
	QUEUE_ARBITER,
	ARBITER_MAX_MODEL
} SIM_arbiter_model_t;

/* connection type */
typedef enum {
	TRANS_GATE,	/* transmission gate connection */
	TRISTATE_GATE,	/* tri-state gate connection */
} SIM_connect_t;

/* transmission gate type */
typedef enum {
	N_GATE,
	NP_GATE
} SIM_trans_t;

#endif /* _SIM_ROUTER_MODEL_H */

