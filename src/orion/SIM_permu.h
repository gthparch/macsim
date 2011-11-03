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

#ifndef _SIM_POWER_PERMU_H
#define _SIM_POWER_PERMU_H

#include "SIM_parameter.h"

#define FIRST_STG	0x01
#define SECOND_STG	0x02
#define THIRD_STG	0x04
#define FOURTH_STG	0x08
#define ALL_PASS	0x0F
#define is_omega(x)	(!(x))
#define is_flip(x)	(x)

typedef struct {
	u_int data_width;
	LIB_Type_max_uint n_chg_in;
	LIB_Type_max_uint n_chg_out;
	LIB_Type_max_uint n_chg_stg;
	LIB_Type_max_uint n_chg_int;
	LIB_Type_max_uint n_chg_pass;
	LIB_Type_max_uint n_chg_ctr;
	double e_chg_in;
	double e_chg_out;
	double e_chg_stg;
	double e_chg_int;
	double e_chg_pass;
	double e_chg_ctr;
	/* state variable */
	LIB_Type_max_uint in;
	LIB_Type_max_uint out;
	LIB_Type_max_uint stg[3];
	LIB_Type_max_uint inn[4];
	uint pass;
	LIB_Type_max_uint ctr[4];
	/* redundant field */
	LIB_Type_max_uint data_mask;
	LIB_Type_max_uint ctr_mask;
} SIM_omflip_t;

#define ZBIT	1
#define WBIT	0
#define N_ONEHOT	(32 + 16 + 8 + 4 + 2 + 1)

/* WHS: maximum bit width: 64 */
typedef struct {
	u_int data_width;
	LIB_Type_max_uint n_chg_in;
	LIB_Type_max_uint n_chg_zin[6];	/* z/w input node */
	LIB_Type_max_uint n_chg_lin[6];	/* left one-hot node */
	LIB_Type_max_uint n_chg_rin[6];	/* right one-hot node */
	LIB_Type_max_uint n_chg_oin;	/* OR gate input node */
	LIB_Type_max_uint n_chg_out;
	double e_chg_in;
	double e_chg_zin[6];
	double e_chg_lin[6];
	double e_chg_rin[6];
	double e_chg_oin;
	double e_chg_out;
	/* state variable */
	LIB_Type_max_uint in;
	LIB_Type_max_uint zin[6];
	LIB_Type_max_uint win[6];
	/* one-hot numbers */
	u_int lin[N_ONEHOT];
	u_int rin[N_ONEHOT];
	LIB_Type_max_uint zoin;
	LIB_Type_max_uint woin;
	LIB_Type_max_uint out;
	/* redundant field */
	u_int n_stg;
	LIB_Type_max_uint mask;
} SIM_grp_t;

typedef struct {
	int model;
	union {
		SIM_omflip_t omflip;
		SIM_grp_t grp;
	} u;
} SIM_permu_t;


extern int SIM_permu_init(SIM_permu_t *permu, int model, u_int data_width);
extern int SIM_permu_record(SIM_permu_t *permu, LIB_Type_max_uint in, u_int mode, LIB_Type_max_uint op, int reset);
extern LIB_Type_max_uint SIM_permu_record_test(SIM_permu_t *permu, LIB_Type_max_uint in, u_int mode, LIB_Type_max_uint op);
extern double SIM_permu_report(SIM_permu_t *permu);
extern double SIM_permu_max_energy(SIM_permu_t *permu);

#endif	/* _SIM_POWER_PERMU_H */
