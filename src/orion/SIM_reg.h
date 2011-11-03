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

#ifndef _SIM_REG_POWER_H
#define _SIM_REG_POWER_H

#include "SIM_parameter.h"

#if ( PARM( POWER_STATS ))

#include "SIM_array.h"

#define P_DATA_T	int


/*@
 * data type: register file port state
 * 
 *    row_addr       -- row address
 *    col_addr       -- column address
 *    tag_addr       -- unused placeholder
 *    tag_line       -- unused placeholder
 *    data_line_size -- register size in char
 *    data_line      -- value of data bitline
 *
 * NOTE:
 *   (1) data_line is only used by write port
 */
typedef struct {
	LIB_Type_max_uint row_addr;
	LIB_Type_max_uint col_addr;
	LIB_Type_max_uint tag_addr;
	LIB_Type_max_uint tag_line;
	u_int data_line_size;
	P_DATA_T data_line;
} SIM_reg_port_state_t;


/* global variables */
extern GLOBDEF( SIM_array_t, reg_power );
extern GLOBDEF( SIM_array_info_t, reg_info );
extern GLOBDEF( SIM_reg_port_state_t, reg_read_port )[PARM(read_port)];
extern GLOBDEF( SIM_reg_port_state_t, reg_write_port )[PARM(write_port)];


/* function prototypes */
extern int FUNC(SIM_reg_power_init, SIM_array_info_t *info, SIM_array_t *arr, SIM_reg_port_state_t *read_port, SIM_reg_port_state_t *write_port);
extern int FUNC(SIM_reg_power_report, SIM_array_info_t *info, SIM_array_t *arr);
extern int FUNC(SIM_reg_avg_power, SIM_array_info_t *info, SIM_array_t *arr, int rw);
/* wrapper functions */
extern int FUNC( SIM_reg_power_dec, SIM_array_info_t *info, SIM_array_t *arr, u_int port, LIB_Type_max_uint row_addr, int rw );
extern int FUNC( SIM_reg_power_data_read, SIM_array_info_t *info, SIM_array_t *arr, P_DATA_T data );
extern int FUNC( SIM_reg_power_data_write, SIM_array_info_t *info, SIM_array_t *arr, u_int port, LIB_Type_max_uint old_data, LIB_Type_max_uint new_data );
extern int FUNC( SIM_reg_power_output, SIM_array_info_t *info, SIM_array_t *arr, P_DATA_T data );

#endif	/* PARM( POWER_STATS ) */

#endif	/* _SIM_REG_POWER_H */
