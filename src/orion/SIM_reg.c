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
 * FIXME: (1) change SIM_ARRAY_NO_MODEL to SIM_NO_MODEL
 *
 * NOTES: (1) no tag array
 *        (2) no column decoder
 */

#include "SIM_port.h"

#if ( PARM( POWER_STATS ))

#include <stdio.h>
#include <math.h>
#include "SIM_reg_power.h"


/* global variables */
GLOBDEF( SIM_power_array_t, reg_power );
GLOBDEF( SIM_power_array_info_t, reg_info );
GLOBDEF( SIM_reg_port_state_t, reg_read_port )[PARM(read_port)];
GLOBDEF( SIM_reg_port_state_t, reg_write_port )[PARM(write_port)];


static int SIM_reg_port_state_init(SIM_power_array_info_t *info, SIM_reg_port_state_t *port, u_int n_port);


int FUNC(SIM_reg_power_init, SIM_power_array_info_t *info, SIM_power_array_t *arr, SIM_reg_port_state_t *read_port, SIM_reg_port_state_t *write_port)
{
  /* ==================== set parameters ==================== */
  /* general parameters */
  info->share_rw = 0;
  info->read_ports = PARM(read_port);
  info->write_ports = PARM(write_port);
  info->n_set = PARM(n_regs);
  info->blk_bits = PARM(reg_width);
  info->assoc = 1;
  info->data_width = PARM(reg_width);
  info->data_end = PARM(data_end);

  /* sub-array partition parameters */
  info->data_ndwl = PARM( ndwl );
  info->data_ndbl = PARM( ndbl );
  info->data_nspd = PARM( nspd );

  info->data_n_share_amp = 1;

  /* model parameters */
  info->row_dec_model = PARM( row_dec_model );
  info->row_dec_pre_model = PARM( row_dec_pre_model );
  info->data_wordline_model = PARM( wordline_model );
  info->data_bitline_model = PARM( bitline_model );
  info->data_bitline_pre_model = PARM( bitline_pre_model );
  info->data_mem_model = PARM( mem_model );
#if (PARM(data_end) == 2)
  info->data_amp_model = PARM(amp_model);
#else
  info->data_amp_model = SIM_NO_MODEL;
#endif	/* PARM(data_end) == 2 */
  info->outdrv_model = PARM( outdrv_model );

  info->data_colsel_pre_model = SIM_NO_MODEL;
  info->col_dec_model = SIM_NO_MODEL;
  info->col_dec_pre_model = SIM_NO_MODEL;
  info->mux_model = SIM_NO_MODEL;

  /* no tag array */
  info->tag_wordline_model = SIM_NO_MODEL;
  info->tag_bitline_model = SIM_NO_MODEL;
  info->tag_bitline_pre_model = SIM_NO_MODEL;
  info->tag_mem_model = SIM_NO_MODEL;
  info->tag_attach_mem_model = SIM_NO_MODEL;
  info->tag_amp_model = SIM_NO_MODEL;
  info->tag_colsel_pre_model = SIM_NO_MODEL;
  info->comp_model = SIM_NO_MODEL;
  info->comp_pre_model = SIM_NO_MODEL;

  
  /* ==================== set flags ==================== */
  info->write_policy = 0;	/* no dirty bit */

  
  /* ==================== compute redundant fields ==================== */
  info->n_item = info->blk_bits / info->data_width;
  info->eff_data_cols = info->blk_bits * info->assoc * info->data_nspd;

  
  /* ==================== call back functions ==================== */
  info->get_entry_valid_bit = NULL;
  info->get_entry_dirty_bit = NULL;
  info->get_entry_tag = NULL;
  info->get_set_tag = NULL;
  info->get_set_use_bit = NULL;


  /* initialize state variables */
  if (read_port) SIM_reg_port_state_init(info, read_port, info->read_ports);
  if (write_port) SIM_reg_port_state_init(info, write_port, info->write_ports);
  
  return SIM_array_power_init( info, arr );
}


int FUNC(SIM_reg_power_report, SIM_power_array_info_t *info, SIM_power_array_t *arr)
{
  fprintf(stderr, "register file power stats:\n");
  SIM_array_power_report(info, arr);

  return 0;
}



/* ==================== state manipulation functions ==================== */
static int SIM_reg_port_state_init(SIM_power_array_info_t *info, SIM_reg_port_state_t *port, u_int n_port)
{
  u_int i;

  for ( i = 0; i < n_port; i ++ ) {
    port[i].data_line_size = sizeof(port[i].data_line);
    SIM_array_port_state_init(info, (SIM_array_port_state_t *)port);
  }

  return 0;
}



/* ==================== wrapper functions ==================== */
/* record row decoder and wordline activity */
inline int FUNC( SIM_reg_power_dec, SIM_power_array_info_t *info, SIM_power_array_t *arr, u_int port, LIB_Type_max_uint row_addr, int rw )
{
  if (rw)	/* write */
    return SIM_power_array_dec( info, arr, (SIM_array_port_state_t *)&GLOB(reg_write_port)[port], row_addr, rw );
  else		/* read */
    return SIM_power_array_dec( info, arr, (SIM_array_port_state_t *)&GLOB(reg_read_port)[port], row_addr, rw );
}


/* record read data activity */
inline int FUNC( SIM_reg_power_data_read, SIM_power_array_info_t *info, SIM_power_array_t *arr, P_DATA_T data )
{
  return SIM_power_array_data_read( info, arr, data );
}


/* record write data bitline and memory cell activity */
inline int FUNC( SIM_reg_power_data_write, SIM_power_array_info_t *info, SIM_power_array_t *arr, u_int port, LIB_Type_max_uint old_data, LIB_Type_max_uint new_data )
{
  return SIM_power_array_data_write( info, arr, NULL, sizeof(old_data), (u_char *)&GLOB(reg_write_port)[port].data_line, (u_char *)&old_data, (u_char *)&new_data );
}


/* record output driver activity */
inline int FUNC( SIM_reg_power_output, SIM_power_array_info_t *info, SIM_power_array_t *arr, P_DATA_T data )
{
  return SIM_power_array_output( info, arr, sizeof(P_DATA_T), 1, &data, NULL );
}

#endif	/* PARM( POWER_STATS ) */
