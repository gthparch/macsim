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

#ifndef _SIM_POWER_MISC_INTERNAL_H
#define _SIM_POWER_MISC_INTERNAL_H

#include <sys/types.h>

/* maximum level of selection arbiter tree */
#define MAX_SEL_LEVEL	5


/*
 * FIXME: cannot handle bus wider than 64-bit
 */
typedef struct {
	int model;
	int encoding;
	u_int data_width;
	u_int grp_width;
	LIB_Type_max_uint n_switch;
	double e_switch;
	/* redundant field */
	u_int bit_width;
	LIB_Type_max_uint bus_mask;
} SIM_bus_t;

typedef struct {
	int model;
	u_int width;
	LIB_Type_max_uint n_anyreq;
	LIB_Type_max_uint n_chgreq;
	LIB_Type_max_uint n_grant;
	LIB_Type_max_uint n_enc[MAX_SEL_LEVEL];
	double e_anyreq;
	double e_chgreq;
	double e_grant;
	double e_enc[MAX_SEL_LEVEL];
} SIM_sel_t;

typedef struct {
	int model;
	LIB_Type_max_uint n_switch;
	LIB_Type_max_uint n_keep_1;
	LIB_Type_max_uint n_keep_0;
	LIB_Type_max_uint n_clock;
	double e_switch;
	double e_keep_1;
	double e_keep_0;
	double e_clock;
	double I_static;
} SIM_ff_t;

/* bus model interface */
extern int SIM_bus_init(SIM_bus_t *bus, int model, int encoding, u_int width, u_int grp_width, u_int n_snd, u_int n_rcv, double length, double time);
extern int SIM_bus_record(SIM_bus_t *bus, LIB_Type_max_uint old_state, LIB_Type_max_uint new_state);
extern double SIM_bus_report(SIM_bus_t *bus);
extern LIB_Type_max_uint SIM_bus_state(SIM_bus_t *bus, LIB_Type_max_uint old_data, LIB_Type_max_uint old_state, LIB_Type_max_uint new_data);

/* flip-flop model interface */
extern int SIM_fpfp_init(SIM_ff_t *ff, int model, double load);
extern int SIM_fpfp_clear_stat(SIM_ff_t *ff);
extern double SIM_fpfp_report(SIM_ff_t *ff);

/* instruction window interface */
extern int SIM_iwin_sel_record(SIM_sel_t *sel, u_int req, int en, int last_level);
extern double SIM_iwin_sel_report(SIM_sel_t *sel);

#endif	/* _SIM_POWER_MISC_INTERNAL_H */
