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

#ifndef _SIM_ARRAY_INTERNAL_H
#define _SIM_ARRAY_INTERNAL_H

#include <sys/types.h>

#define SIM_ARRAY_READ		0
#define SIM_ARRAY_WRITE		1

#define SIM_ARRAY_RECOVER	1

/* read/write */
#define SIM_ARRAY_RW		0
/* only write */
#define SIM_ARRAY_WO		1

/* Used to pass values around the program */
typedef struct {
	int cache_size;
	int number_of_sets;
	int associativity;
	int block_size;
} time_parameter_type;

typedef struct {
	double access_time,cycle_time;
	int best_Ndwl,best_Ndbl;
	int best_Nspd;
	int best_Ntwl,best_Ntbl;
	int best_Ntspd;
	double decoder_delay_data,decoder_delay_tag;
	double dec_data_driver,dec_data_3to8,dec_data_inv;
	double dec_tag_driver,dec_tag_3to8,dec_tag_inv;
	double wordline_delay_data,wordline_delay_tag;
	double bitline_delay_data,bitline_delay_tag;
	double sense_amp_delay_data,sense_amp_delay_tag;
	double senseext_driver_delay_data;
	double compare_part_delay;
	double drive_mux_delay;
	double selb_delay;
	double data_output_delay;
	double drive_valid_delay;
	double precharge_delay;
} time_result_type;


typedef struct {
	int model;
	u_int n_bits;
	LIB_Type_max_uint n_chg_output;
	LIB_Type_max_uint n_chg_addr;
	LIB_Type_max_uint n_chg_l1;
	double e_chg_output;
	double e_chg_addr;
	double e_chg_l1;
	u_int n_in_1st;
	u_int n_in_2nd;
	u_int n_out_0th;
	u_int n_out_1st;
	/* redundant field */
	LIB_Type_max_uint addr_mask;
} SIM_array_dec_t;

typedef struct {
	int model;
	int share_rw;
	LIB_Type_max_uint n_read;
	LIB_Type_max_uint n_write;
	double e_read;
	double e_write;
	double I_static;
} SIM_array_wordline_t;

typedef struct {
	int model;
	int share_rw;
	u_int end;
	LIB_Type_max_uint n_col_write;
	LIB_Type_max_uint n_col_read;
	LIB_Type_max_uint n_col_sel;
	double e_col_write;
	double e_col_read;
	double e_col_sel;
	double I_static;
} SIM_array_bitline_t;

typedef struct {
	int model;
	LIB_Type_max_uint n_access;
	double e_access;
} SIM_array_amp_t;

typedef struct {
	int model;
	u_int n_bits;
	u_int assoc;
	LIB_Type_max_uint n_access;
	LIB_Type_max_uint n_match;
	LIB_Type_max_uint n_mismatch;
	LIB_Type_max_uint n_miss;
	LIB_Type_max_uint n_bit_match;
	LIB_Type_max_uint n_bit_mismatch;
	LIB_Type_max_uint n_chg_addr;
	double e_access;
	double e_match;
	double e_mismatch;
	double e_miss;
	double e_bit_match;
	double e_bit_mismatch;
	double e_chg_addr;
	/* redundant field */
	LIB_Type_max_uint comp_mask;
} SIM_array_comp_t;

typedef struct {
	int model;
	u_int end;
	LIB_Type_max_uint n_switch;
	double e_switch;
	double I_static;
} SIM_array_mem_t;

typedef struct {
	int model;
	u_int assoc;
	LIB_Type_max_uint n_mismatch;
	LIB_Type_max_uint n_chg_addr;
	double e_mismatch;
	double e_chg_addr;
} SIM_array_mux_t;

typedef struct {
	int model;
	u_int item_width;
	LIB_Type_max_uint n_select;
	LIB_Type_max_uint n_chg_data;
	LIB_Type_max_uint n_out_1;
	LIB_Type_max_uint n_out_0;
	double e_select;
	double e_chg_data;
	double e_out_1;
	double e_out_0;
	/* redundant field */
	LIB_Type_max_uint out_mask;
} SIM_array_out_t;

typedef struct {
	int model;
	LIB_Type_max_uint n_charge;
	double e_charge;
	double I_static;
} SIM_array_pre_t;

/* ==================== function prototypes ==================== */
extern double SIM_array_pre_cap(double width, double length);
extern u_int SIM_array_n_pre_gate(int model);
extern u_int SIM_array_n_pre_drain(int model);

/* low-level function from cacti */
extern void SIM_calculate_time(time_result_type*, time_parameter_type*);

/* structural-level record functions */
extern int SIM_array_dec_record( SIM_array_dec_t *dec, LIB_Type_max_uint prev_addr, LIB_Type_max_uint curr_addr );
extern int SIM_array_wordline_record( SIM_array_wordline_t *wordline, int rw, LIB_Type_max_uint n_switch );
extern int SIM_array_bitline_record( SIM_array_bitline_t *bitline, int rw, u_int cols, LIB_Type_max_uint old_value, LIB_Type_max_uint new_value );
extern int SIM_array_amp_record( SIM_array_amp_t *amp, u_int cols );
extern int SIM_array_comp_global_record( SIM_array_comp_t *comp, LIB_Type_max_uint prev_value, LIB_Type_max_uint curr_value, int miss );
extern int SIM_array_comp_local_record( SIM_array_comp_t *comp, LIB_Type_max_uint prev_tag, LIB_Type_max_uint curr_tag, LIB_Type_max_uint input, int recover );
extern int SIM_array_mux_record( SIM_array_mux_t *mux, LIB_Type_max_uint prev_addr, LIB_Type_max_uint curr_addr, int miss );
extern int SIM_array_outdrv_global_record( SIM_array_out_t *outdrv, LIB_Type_max_uint data );
extern int SIM_array_outdrv_local_record( SIM_array_out_t *outdrv, LIB_Type_max_uint prev_data, LIB_Type_max_uint curr_data, int recover );
extern int SIM_array_mem_record( SIM_array_mem_t *mem, LIB_Type_max_uint prev_value, LIB_Type_max_uint curr_value, u_int cols );
extern int SIM_array_pre_record( SIM_array_pre_t *pre, LIB_Type_max_uint n_charge );

/* structural-level report functions */
extern double SIM_array_dec_report(SIM_array_dec_t *dec);
extern double SIM_array_wordline_report(SIM_array_wordline_t *wordline);
extern double SIM_array_bitline_report(SIM_array_bitline_t *bitline);
extern double SIM_array_amp_report(SIM_array_amp_t *amp);
extern double SIM_array_comp_report(SIM_array_comp_t *comp);
extern double SIM_array_mux_report(SIM_array_mux_t *mux);
extern double SIM_array_outdrv_report(SIM_array_out_t *outdrv);
extern double SIM_array_mem_report(SIM_array_mem_t *mem);
extern double SIM_array_pre_report(SIM_array_pre_t *pre);

#endif /* _SIM_ARRAY_INTERNAL_H */

