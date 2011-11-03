/*-------------------------------------------------------------------------
 *                            ORION 2.0 
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

#ifndef _SIM_ARBITER_H
#define _SIM_ARBITER_H

#include "SIM_parameter.h"
#include "SIM_array.h"
#include "SIM_misc.h"

/* carry_in is the internal carry transition node */
typedef struct {
	int model;
	u_int req_width;
	LIB_Type_max_uint n_chg_req;
	LIB_Type_max_uint n_chg_grant;
	/* internal node of round robin arbiter */
	LIB_Type_max_uint n_chg_carry;
	LIB_Type_max_uint n_chg_carry_in;
	/* internal node of matrix arbiter */
	LIB_Type_max_uint n_chg_mint;
	double e_chg_req;
	double e_chg_grant;
	double e_chg_carry;
	double e_chg_carry_in;
	double e_chg_mint;
	/* priority flip flop */
	SIM_ff_t pri_ff;
	/* request queue */
	SIM_array_t queue;
	/* redundant field */
	LIB_Type_max_uint mask;
	double I_static;
} SIM_arbiter_t;

int SIM_arbiter_init(SIM_arbiter_t *arb, int arbiter_model, int ff_model, u_int req_width, double length, SIM_array_info_t *info);

int SIM_arbiter_record(SIM_arbiter_t *arb, LIB_Type_max_uint new_req, LIB_Type_max_uint old_req, u_int new_grant, u_int old_grant);

double SIM_arbiter_report(SIM_arbiter_t *arb);
    
double SIM_arbiter_stat_energy(SIM_arbiter_t *arb, SIM_array_info_t *info, double n_req, int print_depth, char *path, int max_avg);

#endif /* _SIM_ARBITER_H */

