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

/*
 * FIXME: (2) can I use u_int in .xml file?
 * 	  (3) address decomposition does not consider column selector and sub-array now
 *
 * NOTES: (1) power stats function should not be called before its functional counterpart
 *            except data_write
 *
 * TODO:  (2) How to deal with RF's like AH/AL/AX?  Now I assume output with 0 paddings.
 *        (1) make _dec handle both row dec and col dec, no wordline
 *        (3) move wordline stats to _read and _write
 */

#include <math.h>
#include <string.h>
#include <stdio.h>

#include "SIM_parameter.h"
#include "SIM_array.h"
#include "SIM_cam.h"
#include "SIM_util.h"
#include "SIM_time.h"

/* local macros */
#define IS_DIRECT_MAP( info )		((info)->assoc == 1)
#define IS_FULLY_ASSOC( info )		((info)->n_set == 1 && (info)->assoc > 1)
#define IS_WRITE_THROUGH( info )	(! (info)->write_policy)
#define IS_WRITE_BACK( info )		((info)->write_policy)

/* sufficient (not necessary) condition */
#define HAVE_TAG( info )		((info)->tag_mem_model)
#define HAVE_USE_BIT( info )		((info)->use_bit_width)
#define HAVE_COL_DEC( info )		((info)->col_dec_model)
#define HAVE_COL_MUX( info )		((info)->mux_model)


/* for now we simply initialize all fields to 0, which should not
 * add too much error if the program runtime is long enough :) */
int SIM_array_port_state_init( SIM_array_info_t *info, SIM_array_port_state_t *port )
{
	if ( IS_FULLY_ASSOC( info ) || !(info->share_rw))
		bzero( port->data_line, port->data_line_size );

	port->tag_line = 0;
	port->row_addr = 0;
	port->col_addr = 0;
	port->tag_addr = 0;

	return 0;
}


int SIM_array_set_state_init( SIM_array_info_t *info, SIM_array_set_state_t *set )
{
	set->entry = NULL;
	set->entry_set = NULL;

	if ( IS_FULLY_ASSOC( info )) {
		set->write_flag = 0;
		set->write_back_flag = 0;
	}

	/* no default value for other fields */
	return 0;
}


/* record row decoder and wordline activity */
/* only used by non-fully-associative array, but we check it anyway */
int SIM_array_dec( SIM_array_info_t *info, SIM_array_t *arr, SIM_array_port_state_t *port, LIB_Type_max_uint row_addr, int rw )
{
	if ( ! IS_FULLY_ASSOC( info )) {
		/* record row decoder stats */
		if (info->row_dec_model) {
			SIM_array_dec_record( &arr->row_dec, port->row_addr, row_addr );

			/* update state */
			port->row_addr = row_addr;
		}

		/* record wordline stats */
		SIM_array_wordline_record( &arr->data_wordline, rw, info->data_ndwl );
		if ( HAVE_TAG( info ))
			SIM_array_wordline_record( &arr->tag_wordline, rw, info->tag_ndwl );

		return 0;
	}
	else
		return -1;
}


/* record read data activity (including bitline and sense amplifier) */
/* only used by non-fully-associative array, but we check it anyway */
/* data only used by RF array */
int SIM_array_data_read( SIM_array_info_t *info, SIM_array_t *arr, LIB_Type_max_uint data )
{
	if (info->data_end == 1) {
		SIM_array_bitline_record( &arr->data_bitline, SIM_ARRAY_READ, info->eff_data_cols, 0, data );

		return 0;
	}
	else if ( ! IS_FULLY_ASSOC( info )) {
		SIM_array_bitline_record( &arr->data_bitline, SIM_ARRAY_READ, info->eff_data_cols, 0, 0 );
		SIM_array_amp_record( &arr->data_amp, info->eff_data_cols );

		return 0;
	}
	else
		return -1;
}


/* record write data bitline and memory cell activity */
/* assume no alignment restriction on write, so (u_char *) */
/* set only used by fully-associative array */
/* data_line only used by fully-associative or RF array */
int SIM_array_data_write( SIM_array_info_t *info, SIM_array_t *arr, SIM_array_set_state_t *set, u_int n_item, u_char *data_line, u_char *old_data, u_char *new_data )
{
	u_int i;

	/* record bitline stats */
	if ( IS_FULLY_ASSOC( info )) {
		/* wordline should be driven only once */
		if ( ! set->write_flag ) {
			SIM_array_wordline_record( &arr->data_wordline, SIM_ARRAY_WRITE, 1 );
			set->write_flag = 1;
		}

		/* for fully-associative array, data bank has no read 
		 * bitlines, so bitlines not written have no activity */
		for ( i = 0; i < n_item; i ++ ) {
			SIM_array_bitline_record( &arr->data_bitline, SIM_ARRAY_WRITE, 8, data_line[i], new_data[i] );
			/* update state */
			data_line[i] = new_data[i];
		}
	}
	else if (info->share_rw) {
		/* there is some subtlety here: write width may not be as wide as block size,
		 * bitlines not written are actually read, but column selector should be off,
		 * so read energy per bitline is the same as write energy per bitline */
		SIM_array_bitline_record( &arr->data_bitline, SIM_ARRAY_WRITE, info->eff_data_cols, 0, 0 );

		/* write in all sub-arrays if direct-mapped, which implies 1 cycle write latency,
		 * in those sub-arrays wordlines are not driven, so only n items columns switch */
		if ( IS_DIRECT_MAP( info ) && info->data_ndbl > 1 )
			SIM_array_bitline_record( &arr->data_bitline, SIM_ARRAY_WRITE, n_item * 8 * ( info->data_ndbl - 1 ), 0, 0 );
	}
	else {	/* separate R/W bitlines */
		/* same arguments as in the previous case apply here, except that when we say
		 * read_energy = write_energy, we omit the energy of write driver gate cap */
		for ( i = 0; i < n_item; i ++ ) {
			SIM_array_bitline_record( &arr->data_bitline, SIM_ARRAY_WRITE, 8, data_line[i], new_data[i] );
			/* update state */
			data_line[i] = new_data[i];
		}
	}

	/* record memory cell stats */
	for ( i = 0; i < n_item; i ++ )
		SIM_array_mem_record( &arr->data_mem, old_data[i], new_data[i], 8 );

	return 0;
}


/* record read tag activity (including bitline and sense amplifier) */
/* only used by non-RF array */
/* set only used by fully-associative array */
int SIM_array_tag_read( SIM_array_info_t *info, SIM_array_t *arr, SIM_array_set_state_t *set )
{
	if ( IS_FULLY_ASSOC( info )) {
		/* the only reason to read a fully-associative array tag is writing back */
		SIM_array_wordline_record( &arr->tag_wordline, SIM_ARRAY_READ, 1 );
		set->write_back_flag = 1;
	}

	SIM_array_bitline_record( &arr->tag_bitline, SIM_ARRAY_READ, info->eff_tag_cols, 0, 0 );
	SIM_array_amp_record( &arr->tag_amp, info->eff_tag_cols );

	return 0;
}


/* record write tag bitline and memory cell activity */
/* WHS: assume update of use bit, valid bit, dirty bit and tag will be coalesced */
/* only used by non-RF array */
/* port only used by fully-associative array */
int SIM_array_tag_update( SIM_array_info_t *info, SIM_array_t *arr, SIM_array_port_state_t *port, SIM_array_set_state_t *set )
{
	u_int i;
	LIB_Type_max_uint curr_tag;
	SIM_array_mem_t *tag_attach_mem;

	/* get current tag */
	if ( set->entry )
		curr_tag = (*info->get_entry_tag)( set->entry );

	if ( IS_FULLY_ASSOC( info ))
		tag_attach_mem = &arr->tag_attach_mem;
	else
		tag_attach_mem = &arr->tag_mem;

	/* record tag bitline stats */
	if ( IS_FULLY_ASSOC( info )) {
		if ( set->entry && curr_tag != set->tag_bak ) {
			/* shared wordline should be driven only once */
			if ( ! set->write_back_flag )
				SIM_array_wordline_record( &arr->tag_wordline, SIM_ARRAY_WRITE, 1 );

			/* WHS: value of tag_line doesn't matter if not write_through */
			SIM_array_bitline_record( &arr->tag_bitline, SIM_ARRAY_WRITE, info->eff_tag_cols, port->tag_line, curr_tag );
			/* update state */
			if ( IS_WRITE_THROUGH( info ))
				port->tag_line = curr_tag;
		}
	}
	else {
		/* tag update cannot occur at the 1st cycle, so no other sub-arrays */
		SIM_array_bitline_record( &arr->tag_bitline, SIM_ARRAY_WRITE, info->eff_tag_cols, 0, 0 );
	}

	/* record tag memory cell stats */
	if ( HAVE_USE_BIT( info ))
		for ( i = 0; i < info->assoc; i ++ )
			SIM_array_mem_record( tag_attach_mem, set->use_bak[i], (*info->get_set_use_bit)( set->entry_set, i ), info->use_bit_width );

	if ( set->entry ) {
		SIM_array_mem_record( tag_attach_mem, set->valid_bak, (*info->get_entry_valid_bit)( set->entry ), info->valid_bit_width );
		SIM_array_mem_record( &arr->tag_mem, set->tag_bak, curr_tag, info->tag_addr_width );

		if ( IS_WRITE_BACK( info ))
			SIM_array_mem_record( tag_attach_mem, set->dirty_bak, (*info->get_entry_dirty_bit)( set->entry ), 1 );
	}

	return 0;
}


/* record tag compare activity (including tag comparator, column decoder and multiplexor) */
/* NOTE: this function may be called twice during ONE array operation, remember to update
 *       states at the end so that call to *_record won't add erroneous extra energy */
/* only used by non-RF array */
int SIM_array_tag_compare( SIM_array_info_t *info, SIM_array_t *arr, SIM_array_port_state_t *port, LIB_Type_max_uint tag_input, LIB_Type_max_uint col_addr, SIM_array_set_state_t *set )
{
	int miss = 0;
	u_int i;

	/* record tag comparator stats */
	for ( i = 0; i < info->assoc; i ++ ) {
		/* WHS: sense amplifiers output 0 when idle */
		if ( SIM_array_comp_local_record( &arr->comp, 0, (*info->get_set_tag)( set->entry_set, i ), tag_input, SIM_ARRAY_RECOVER ))
			miss = 1;
	}

	SIM_array_comp_global_record( &arr->comp, port->tag_addr, tag_input, miss );

	/* record column decoder stats */
	if ( HAVE_COL_DEC( info ))
		SIM_array_dec_record( &arr->col_dec, port->col_addr, col_addr );

	/* record multiplexor stats */
	if ( HAVE_COL_MUX( info ))
		SIM_array_mux_record( &arr->mux, port->col_addr, col_addr, miss );

	/* update state */
	port->tag_addr = tag_input;
	if ( HAVE_COL_DEC( info ))
		port->col_addr = col_addr;

	return 0;
}


/* record output driver activity */
/* assume alignment restriction on read, so specify data_size */
/* WHS: it's really a mess to use data_size to specify data type */
/* data_all only used by non-RF and non-fully-associative array */
/* WHS: don't support 128-bit or wider integer */
int SIM_array_output( SIM_array_info_t *info, SIM_array_t *arr, u_int data_size, u_int length, void *data_out, void *data_all )
{
	u_int i, j;

	/* record output driver stats */
	for ( i = 0; i < length; i ++ ) {
		switch ( data_size ) {
			case 1: SIM_array_outdrv_global_record( &arr->outdrv, ((u_int8_t *)data_out)[i] );
				break;
			case 2: SIM_array_outdrv_global_record( &arr->outdrv, ((u_int16_t *)data_out)[i] );
				break;
			case 4: SIM_array_outdrv_global_record( &arr->outdrv, ((u_int32_t *)data_out)[i] );
				break;
			case 8: SIM_array_outdrv_global_record( &arr->outdrv, ((u_int64_t *)data_out)[i] );
				break;
			default: printf ("error\n");	/* some error handler */
		}
	}

	if ( ! IS_FULLY_ASSOC( info )) {
		for ( i = 0; i < info->assoc; i ++ )
			for ( j = 0; j < info->n_item; j ++ )
				/* sense amplifiers output 0 when idle */
				switch ( data_size ) {
					case 1: SIM_array_outdrv_local_record( &arr->outdrv, 0, ((u_int8_t **)data_all)[i][j], SIM_ARRAY_RECOVER );
						break;
					case 2: SIM_array_outdrv_local_record( &arr->outdrv, 0, ((u_int16_t **)data_all)[i][j], SIM_ARRAY_RECOVER );
						break;
					case 4: SIM_array_outdrv_local_record( &arr->outdrv, 0, ((u_int32_t **)data_all)[i][j], SIM_ARRAY_RECOVER );
						break;
					case 8: SIM_array_outdrv_local_record( &arr->outdrv, 0, ((u_int64_t **)data_all)[i][j], SIM_ARRAY_RECOVER );
						break;
					default: printf ("error\n");	/* some error handler */
				}
	}

	return 0;
}


double SIM_array_stat_energy(SIM_array_info_t *info, SIM_array_t *arr, double n_read, double n_write, int print_depth, char *path, int max_avg)
{
	double Eavg = 0, Eatomic, Estruct, Estatic;
	int next_depth, next_next_depth;
	u_int path_len, next_path_len;
	double avg_read, avg_write;

	/* hack to mimic central buffer */
	/* packet header probability */
	u_int NP_width = 0, NC_width = 0, cnt_width = 0;
	int share_flag = 0;

	if (path && strstr(path, "central buffer")) {
		share_flag = 1;
		NP_width = NC_width = SIM_logtwo(info->n_set);
		/* assume no multicasting */
		cnt_width = 0;
	}

	next_depth = NEXT_DEPTH(print_depth);
	next_next_depth = NEXT_DEPTH(next_depth);
	path_len = SIM_strlen(path);

	if(info->arr_buf_type == SRAM) {
		/* decoder */
		if (info->row_dec_model) {
			Estruct = 0;
			SIM_strcat(path, "row decoder");
			next_path_len = SIM_strlen(path);

			/* assume switch probability 0.5 for address bits */
			Eatomic = arr->row_dec.e_chg_addr * arr->row_dec.n_bits * (max_avg ? 1 : 0.5) * (n_read + n_write);
			SIM_print_stat_energy(SIM_strcat(path, "input"), Eatomic, next_next_depth);
			SIM_res_path(path, next_path_len);
			Estruct += Eatomic;
			Eatomic = arr->row_dec.e_chg_output * (n_read + n_write);
			SIM_print_stat_energy(SIM_strcat(path, "output"), Eatomic, next_next_depth);
			SIM_res_path(path, next_path_len);
			Estruct += Eatomic;

			/* assume all 1st-level decoders change output */
			Eatomic = arr->row_dec.e_chg_l1 * arr->row_dec.n_in_2nd * (n_read + n_write);
			SIM_print_stat_energy(SIM_strcat(path, "internal node"), Eatomic, next_next_depth);
			SIM_res_path(path, next_path_len);
			Estruct += Eatomic;

			SIM_print_stat_energy(path, Estruct, next_depth);
			SIM_res_path(path, path_len);
			Eavg += Estruct;
		}

		/* wordline */
		Estruct = 0;
		SIM_strcat(path, "wordline");
		next_path_len = SIM_strlen(path);

		Eatomic = arr->data_wordline.e_read * n_read;

		SIM_print_stat_energy(SIM_strcat(path, "read"), Eatomic, next_next_depth);
		SIM_res_path(path, next_path_len);
		Estruct += Eatomic;

		Eatomic = arr->data_wordline.e_write * n_write;
		SIM_print_stat_energy(SIM_strcat(path, "write"), Eatomic, next_next_depth);
		SIM_res_path(path, next_path_len);
		Estruct += Eatomic;

		SIM_print_stat_energy(path, Estruct, next_depth);
		SIM_res_path(path, path_len);
		Eavg += Estruct;

		/* bitlines */
		Estruct = 0;
		SIM_strcat(path, "bitline");
		next_path_len = SIM_strlen(path);

		if (arr->data_bitline.end == 2) {
			Eatomic = arr->data_bitline.e_col_read * info->eff_data_cols * n_read;
			/* dirty hack */
			if (share_flag) {
				Eatomic += arr->data_bitline.e_col_read * (NP_width + NC_width + cnt_width) * n_read;
				/* read free list */
				Eatomic += arr->data_bitline.e_col_read * (NP_width + NC_width + cnt_width) * n_write;
			}
		}
		else {
			/* assume switch probability 0.5 for single-ended bitlines */
			Eatomic = arr->data_bitline.e_col_read * info->eff_data_cols * (max_avg ? 1 : 0.5) * n_read;
			/* dirty hack */
			if (share_flag) {
				/* assume no multicasting, cnt is always 0 */
				Eatomic += arr->data_bitline.e_col_read * (NP_width + NC_width) * (max_avg ? 1 : 0.5) * n_read;
				/* read free list */
				Eatomic += arr->data_bitline.e_col_read * (NP_width + NC_width) * (max_avg ? 1 : 0.5) * n_write;
			}
		}
		SIM_print_stat_energy(SIM_strcat(path, "read"), Eatomic, next_next_depth);
		SIM_res_path(path, next_path_len);
		Estruct += Eatomic;

		/* assume switch probability 0.5 for write bitlines */
		Eatomic = arr->data_bitline.e_col_write * info->data_width * (max_avg ? 1 : 0.5) * n_write;
		/* dirty hack */
		if (share_flag) {
			/* current NP and NC */
			Eatomic += arr->data_bitline.e_col_write * (NP_width + NC_width) * (max_avg ? 1 : 0.5) * n_write;
			/* previous NP or NC */
			Eatomic += arr->data_bitline.e_col_write * NP_width * (max_avg ? 1 : 0.5) * n_write;
			/* update free list */
			Eatomic += arr->data_bitline.e_col_write * NC_width * (max_avg ? 1 : 0.5) * n_read;
		}
		SIM_print_stat_energy(SIM_strcat(path, "write"), Eatomic, next_next_depth);
		SIM_res_path(path, next_path_len);
		Estruct += Eatomic;

		Eatomic = arr->data_bitline_pre.e_charge * info->eff_data_cols * n_read;
		/* dirty hack */
		if (share_flag) {
			Eatomic += arr->data_bitline_pre.e_charge * (NP_width + NC_width + cnt_width) * n_read;
			/* read free list */
			Eatomic += arr->data_bitline_pre.e_charge * (NP_width + NC_width + cnt_width) * n_write;
		}
		SIM_print_stat_energy(SIM_strcat(path, "precharge"), Eatomic, next_next_depth);
		SIM_res_path(path, next_path_len);
		Estruct += Eatomic;

		SIM_print_stat_energy(path, Estruct, next_depth);
		SIM_res_path(path, path_len);
		Eavg += Estruct;

		/* memory cells */
		Estruct = 0;

		/* assume switch probability 0.5 for memory cells */
		Eatomic = arr->data_mem.e_switch * info->data_width * (max_avg ? 1 : 0.5) * n_write;
		/* dirty hack */
		if (share_flag) {
			/* current NP and NC */
			Eatomic += arr->data_mem.e_switch * (NP_width + NC_width) * (max_avg ? 1 : 0.5) * n_write;
			/* previous NP or NC */
			Eatomic += arr->data_mem.e_switch * NP_width * (max_avg ? 1 : 0.5) * n_write;
			/* update free list */
			Eatomic += arr->data_mem.e_switch * NC_width * (max_avg ? 1 : 0.5) * n_read;
		}
		Estruct += Eatomic;

		SIM_print_stat_energy(SIM_strcat(path, "memory cell"), Estruct, next_depth);
		SIM_res_path(path, path_len);
		Eavg += Estruct;

		/* sense amplifier */
		if (info->data_end == 2) {
			Estruct = 0;

			Eatomic = arr->data_amp.e_access * info->eff_data_cols * n_read;
			/* dirty hack */
			if (share_flag) {
				Eatomic += arr->data_amp.e_access * (NP_width + NC_width + cnt_width) * n_read;
				/* read free list */
				Eatomic += arr->data_amp.e_access * (NP_width + NC_width + cnt_width) * n_write;
			}
			Estruct += Eatomic;

			SIM_print_stat_energy(SIM_strcat(path, "sense amplifier"), Estruct, next_depth);
			SIM_res_path(path, path_len);
			Eavg += Estruct;
		}

		/* output driver */
		if (info->outdrv_model) {
			Estruct = 0;
			SIM_strcat(path, "output driver");
			next_path_len = SIM_strlen(path);

			Eatomic = arr->outdrv.e_select * n_read;
			SIM_print_stat_energy(SIM_strcat(path, "enable"), Eatomic, next_next_depth);
			SIM_res_path(path, next_path_len);
			Estruct += Eatomic;

			/* same switch probability as bitlines */
			Eatomic = arr->outdrv.e_chg_data * arr->outdrv.item_width * (max_avg ? 1 : 0.5) * info->n_item * info->assoc * n_read;
			SIM_print_stat_energy(SIM_strcat(path, "data"), Eatomic, next_next_depth);
			SIM_res_path(path, next_path_len);
			Estruct += Eatomic;

			/* assume 1 and 0 are uniformly distributed */
			if (arr->outdrv.e_out_1 >= arr->outdrv.e_out_0 || !max_avg) {
				Eatomic = arr->outdrv.e_out_1 * arr->outdrv.item_width * (max_avg ? 1 : 0.5) * n_read;
				SIM_print_stat_energy(SIM_strcat(path, "output 1"), Eatomic, next_next_depth);
				SIM_res_path(path, next_path_len);
				Estruct += Eatomic;
			}

			if (arr->outdrv.e_out_1 < arr->outdrv.e_out_0 || !max_avg) {
				Eatomic = arr->outdrv.e_out_0 * arr->outdrv.item_width * (max_avg ? 1 : 0.5) * n_read;
				SIM_print_stat_energy(SIM_strcat(path, "output 0"), Eatomic, next_next_depth);
				SIM_res_path(path, next_path_len);
				Estruct += Eatomic;
			}

			SIM_print_stat_energy(path, Estruct, next_depth);
			SIM_res_path(path, path_len);
			Eavg += Estruct;
		}

		/* static power */
		Estatic = arr->I_static * Vdd * Period * SCALE_S;

		SIM_print_stat_energy(SIM_strcat(path, "static energy"), Estatic, next_depth);
		SIM_res_path(path, path_len);

		SIM_print_stat_energy(path, Eavg, print_depth);
	}
	else if (info->arr_buf_type == REGISTER){
		Estruct = 0;

		/*average read energy for one buffer entry*/
		arr->ff.n_clock = info->data_width;

		avg_read = arr->ff.e_clock * arr->ff.n_clock * 0.5;

		/*average write energy for one buffer entry*/
		arr->ff.n_clock = info->data_width;
		arr->ff.n_switch = info->data_width* 0.5;

		avg_write = arr->ff.e_switch * arr->ff.n_switch + arr->ff.e_clock * arr->ff.n_clock ;

		/* for each read operation, the energy consists of one read operation and n write
		 * operateion. n means there is n flits in the buffer before read operation.
		 * assume n is info->n_entry * 0.25.
		 */ 
		if(info->n_set > 1){
			Eatomic = (avg_read + info->n_set * 0.25 * avg_write )* n_read;
		}
		else{
			Eatomic = avg_read * n_read;
		}
		SIM_print_stat_energy(SIM_strcat(path, "read energy"), Eatomic, next_depth);
		SIM_res_path(path, path_len);
		Estruct += Eatomic;

		/* write energy */
		Eatomic = avg_write * n_write;
		SIM_print_stat_energy(SIM_strcat(path, "write energy"), Eatomic, next_depth);
		SIM_res_path(path, path_len);
		Estruct += Eatomic;

		Eavg = Estruct; 

		/* static power */
		Estatic = arr->ff.I_static * Vdd * Period * SCALE_S;

		SIM_print_stat_energy(SIM_strcat(path, "static energy"), Estatic, next_depth);
		SIM_res_path(path, path_len);

		SIM_print_stat_energy(path, Eavg, print_depth);
	}
	return Eavg;
}

