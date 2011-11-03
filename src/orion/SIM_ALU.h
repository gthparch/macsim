/*-------------------------------------------------------------------------
 *                              ORION 2.0 
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

#ifndef _SIM_POWER_ALU_H
#define _SIM_POWER_ALU_H

#include "SIM_parameter.h"

/* PLX ALU transistor size */
#define WaluNOTp	(2 * Lamda)
#define WaluNOTn	(1 * Lamda)
#define Walu2NANDp	(2 * Lamda)
#define Walu2NANDn	(2 * Lamda)
#define Walu3NANDp	(3 * Lamda)
#define Walu3NANDn	(2 * Lamda)
#define Walu4NANDp	(4 * Lamda)
#define Walu4NANDn	(2 * Lamda)
#define Walu2NORp	(4 * Lamda)
#define Walu2NORn	(1 * Lamda)
#define Walu3NORp	(6 * Lamda)
#define Walu3NORn	(1 * Lamda)
#define Walu4NORp	(8 * Lamda)
#define Walu4NORn	(1 * Lamda)
#define WaluXORp	(8 * Lamda)
#define WaluXORn	(2 * Lamda)
#define WaluPASSp	(3 * Lamda)
#define WaluPASSn	(3 * Lamda)

typedef struct {
	int model;
	u_int data_width;
	LIB_Type_max_uint n_chg_blk[PLX_BLK];
	double e_blk[PLX_BLK];
	/* state */
	LIB_Type_max_uint a_d1;
	LIB_Type_max_uint a_d2;
	LIB_Type_max_uint l_d1;
	LIB_Type_max_uint l_d2;
	u_int type;
} SIM_ALU_t;

extern int SIM_ALU_init(SIM_ALU_t *alu, int model, u_int data_width);
extern int SIM_ALU_record(SIM_ALU_t *alu, LIB_Type_max_uint d1, LIB_Type_max_uint d2, u_int type);
extern double SIM_ALU_report(SIM_ALU_t *alu);
extern double SIM_ALU_stat_energy(SIM_ALU_t *alu, int max);

#endif	/* _SIM_POWER_ALU_H */
