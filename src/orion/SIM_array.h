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

#ifndef _SIM_POWER_ARRAY_H
#define _SIM_POWER_ARRAY_H

#include <sys/types.h>

#include "SIM_parameter.h"
#include "SIM_array_internal.h"
#include "SIM_misc.h"

typedef struct {
	SIM_array_dec_t row_dec;
	SIM_array_dec_t col_dec;
	SIM_array_wordline_t data_wordline;
	SIM_array_wordline_t tag_wordline;
	SIM_array_bitline_t data_bitline;
	SIM_array_bitline_t tag_bitline;
	SIM_array_mem_t data_mem;
	/* tag address memory cells */
	SIM_array_mem_t tag_mem;
	/* tag various bits memory cells, different with tag_mem for fully-associative cache */
	SIM_array_mem_t tag_attach_mem;
	SIM_array_amp_t data_amp;
	SIM_array_amp_t tag_amp;
	SIM_array_comp_t comp;
	SIM_array_mux_t mux;
	SIM_array_out_t outdrv;
	SIM_array_pre_t row_dec_pre;
	SIM_array_pre_t col_dec_pre;
	SIM_array_pre_t data_bitline_pre;
	SIM_array_pre_t tag_bitline_pre;
	SIM_array_pre_t data_colsel_pre;
	SIM_array_pre_t tag_colsel_pre;
	SIM_array_pre_t comp_pre;
	SIM_ff_t ff;
	double I_static;
} SIM_array_t;

typedef struct {
	/*array type parameter*/
	int arr_buf_type;
	/* parameters shared by data array and tag array */
	int share_rw;
	u_int read_ports;
	u_int write_ports;
	u_int n_set;
	u_int blk_bits;
	u_int assoc;
	int row_dec_model;
	/* parameters specific to data array */
	u_int data_width;
	int col_dec_model;
	int mux_model;
	int outdrv_model;
	/* parameters specific to tag array */
	u_int tag_addr_width;
	u_int tag_line_width;
	int comp_model;
	/* data set of common parameters */
	u_int data_ndwl;
	u_int data_ndbl;
	u_int data_nspd;
	u_int data_n_share_amp;
	u_int data_end;
	int data_wordline_model;
	int data_bitline_model;
	int data_amp_model;
	int data_mem_model;
	/* tag set of common parameters */
	u_int tag_ndwl;
	u_int tag_ndbl;
	u_int tag_nspd;
	u_int tag_n_share_amp;
	u_int tag_end;
	int tag_wordline_model;
	int tag_bitline_model;
	int tag_amp_model;
	int tag_mem_model;
	int tag_attach_mem_model;
	/* parameters for precharging */
	int row_dec_pre_model;
	int col_dec_pre_model;
	int data_bitline_pre_model;
	int tag_bitline_pre_model;
	int data_colsel_pre_model;
	int tag_colsel_pre_model;
	int comp_pre_model;
	/* some redundant fields */
	u_int n_item;		/* # of items in a block */
	u_int eff_data_cols;	/* # of data columns driven by one wordline */
	u_int eff_tag_cols;	/* # of tag columns driven by one wordline */
	/* flags used by prototype array model */
	u_int use_bit_width;
	u_int valid_bit_width;
	int write_policy;	/* 1 if write back (have dirty bits), 0 otherwise */
	/* call back functions */
	/* these fields have no physical counterparts, they are
	 * here just because this is the most convenient place */
	u_int (*get_entry_valid_bit)( void* );
	u_int (*get_entry_dirty_bit)( void* );
	u_int (*get_set_use_bit)( void*, int );
	LIB_Type_max_uint (*get_entry_tag)( void* );
	LIB_Type_max_uint (*get_set_tag)( void*, int );
	/* fields which will be filled by initialization */
	double data_arr_width;
	double tag_arr_width;
	double data_arr_height;
	double tag_arr_height;
} SIM_array_info_t;

/*@
 * data type: array port state
 * 
 *  - row_addr       -- input to row decoder
 *    col_addr       -- input to column decoder, if any
 *    tag_addr       -- input to tag comparator
 * $+ tag_line       -- value of tag bitline
 *  # data_line_size -- size of data_line in char
 *  # data_line      -- value of data bitline
 *
 * legend:
 *   -: only used by non-fully-associative array
 *   +: only used by fully-associative array
 *   #: only used by fully-associative array or RF array
 *   $: only used by write-through array
 *
 * NOTE:
 *   (1) *_addr may not necessarily be an address
 *   (2) data_line_size is the allocated size of data_line in simulator,
 *       which must be no less than the physical size of data line
 *   (3) each instance of module should define an instance-specific data
 *       type with non-zero-length data_line and cast it to this type
 */
typedef struct {
	LIB_Type_max_uint row_addr;
	LIB_Type_max_uint col_addr;
	LIB_Type_max_uint tag_addr;
	LIB_Type_max_uint tag_line;
	u_int data_line_size;
	char data_line[0];
} SIM_array_port_state_t;

/*@
 * data type: array set state
 * 
 *   entry           -- pointer to some entry structure if an entry is selected for
 *                      r/w, NULL otherwise
 *   entry_set       -- pointer to corresponding set structure
 * + write_flag      -- 1 if entry is already written once, 0 otherwise
 * + write_back_flag -- 1 if entry is already written back, 0 otherwise
 *   valid_bak       -- valid bit of selected entry before operation
 *   dirty_bak       -- dirty bit of selected entry, if any, before operation
 *   tag_bak         -- tag of selected entry before operation
 *   use_bak         -- use bits of all entries before operation
 *
 * legend:
 *   +: only used by fully-associative array
 *
 * NOTE:
 *   (1) entry is interpreted by modules, if some module has no "entry structure",
 *       then make sure this field is non-zero if some entry is selected
 *   (2) tag_addr may not necessarily be an address
 *   (3) each instance of module should define an instance-specific data
 *       type with non-zero-length use_bit and cast it to this type
 */
typedef struct {
	void *entry;
	void *entry_set;
	int write_flag;
	int write_back_flag;
	u_int valid_bak;
	u_int dirty_bak;
	LIB_Type_max_uint tag_bak;
	u_int use_bak[0];
} SIM_array_set_state_t;


/* ==================== function prototypes ==================== */
/* prototype-level record functions */
extern int SIM_array_dec(SIM_array_info_t *info, SIM_array_t *arr, SIM_array_port_state_t *port, LIB_Type_max_uint row_addr, int rw );
extern int SIM_array_data_read(SIM_array_info_t *info, SIM_array_t *arr, LIB_Type_max_uint data );
extern int SIM_array_data_write(SIM_array_info_t *info, SIM_array_t *arr, SIM_array_set_state_t *set, u_int n_item, u_char *data_line, u_char *old_data, u_char *new_data );
extern int SIM_array_tag_read(SIM_array_info_t *info, SIM_array_t *arr, SIM_array_set_state_t *set );
extern int SIM_array_tag_update(SIM_array_info_t *info, SIM_array_t *arr, SIM_array_port_state_t *port, SIM_array_set_state_t *set );
extern int SIM_array_tag_compare(SIM_array_info_t *info, SIM_array_t *arr, SIM_array_port_state_t *port, LIB_Type_max_uint tag_input, LIB_Type_max_uint col_addr, SIM_array_set_state_t *set );
extern int SIM_array_output(SIM_array_info_t *info, SIM_array_t *arr, u_int data_size, u_int length, void *data_out, void *data_all );

/* state manupilation functions */
extern int SIM_array_port_state_init(SIM_array_info_t *info, SIM_array_port_state_t *port );
extern int SIM_array_set_state_init(SIM_array_info_t *info, SIM_array_set_state_t *set );

extern int SIM_array_init(SIM_array_info_t *info, int is_fifo, u_int n_read_port, u_int n_write_port, u_int n_entry, u_int line_width, int outdrv, int arr_buf_type);

extern int SIM_array_power_init(SIM_array_info_t *info, SIM_array_t *arr );
extern double SIM_array_power_report(SIM_array_info_t *info, SIM_array_t *arr );
extern int SIM_array_clear_stat(SIM_array_t *arr);

extern double SIM_array_stat_energy(SIM_array_info_t *info, SIM_array_t *arr, double n_read, double n_write, int print_depth, char *path, int max_avg);

#endif	/* _SIM_POWER_ARRAY_H */
