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

#ifndef _SIM_CROSSBAR_H
#define _SIM_CROSSBAR_H

#include "SIM_parameter.h"

typedef struct {
	int model;
	u_int n_in;
	u_int n_out;
	u_int in_seg;	/* only used by segmented crossbar */
	u_int out_seg;	/* only used by segmented crossbar */
	u_int data_width;
	u_int degree;	/* only used by multree crossbar */
	int connect_type;
	int trans_type;	/* only used by transmission gate connection */
	LIB_Type_max_uint n_chg_in;
	LIB_Type_max_uint n_chg_int;
	LIB_Type_max_uint n_chg_out;
	LIB_Type_max_uint n_chg_ctr;
	double e_chg_in;
	double e_chg_int;
	double e_chg_out;
	double e_chg_ctr;
	/* redundant field */
	u_int depth;	/* only used by multree crossbar */
	LIB_Type_max_uint mask;
	double I_static;
} SIM_crossbar_t;


int SIM_crossbar_init(SIM_crossbar_t *crsbar, int model, u_int n_in, u_int n_out, u_int in_seg, u_int out_seg, u_int data_width, u_int degree, int connect_type, int trans_type, double in_len, double out_len, double *req_len);

int SIM_crossbar_record(SIM_crossbar_t *xb, int io, LIB_Type_max_uint new_data, LIB_Type_max_uint old_data, u_int new_port, u_int old_port);

double SIM_crossbar_report(SIM_crossbar_t *crsbar);

double SIM_crossbar_stat_energy(SIM_crossbar_t *crsbar, int print_depth, char *path, int max_avg, double n_data);

#endif /* _SIM_CROSSBAR_H */

