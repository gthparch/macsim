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

#ifndef _SIM_PORT_H
#define _SIM_PORT_H

#include "SIM_router.h"

/* Technology node and operating freq and voltage */
#define PARM_Freq             (1e9)
#define PARM_Vdd              (1.0)
#define PARM_TECH_POINT       (65)
#define PARM_TRANSISTOR_TYPE  (NVT)   /* transistor type, HVT, NVT, or LVT */
/* End Technology node and operating freq and voltage */

#define PARM_POWER_STATS	1

/* RF module parameters */
#define PARM_read_port	1
#define PARM_write_port	1
#define PARM_n_regs	64
#define PARM_reg_width	32

#define PARM_ndwl	1
#define PARM_ndbl	1
#define PARM_nspd	1

/* router module parameters */
/* general parameters */
#define PARM_in_port		7	/* # of network input ports */
#define PARM_cache_in_port	0	/* # of cache input ports */
#define PARM_mc_in_port		0	/* # of memory controller input ports */
#define PARM_io_in_port		0	/* # of I/O device input ports */
#define PARM_out_port		7	/* # of network output ports */
#define PARM_cache_out_port	0	/* # of cache output ports */
#define PARM_mc_out_port	0	/* # of memory controller output ports */
#define PARM_io_out_port	0	/* # of I/O device output ports */
#define PARM_flit_width		32	/* flit width in bits */

/* virtual channel parameters */
#define PARM_v_channel		1	/* # of network port virtual channels */
#define PARM_v_class		0	/* # of total virtual classes */
#define PARM_cache_class	0	/* # of cache port virtual classes */
#define PARM_mc_class		0	/* # of memory controller port virtual classes */
#define PARM_io_class		0	/* # of I/O device port virtual classes */
/* ?? */
#define PARM_in_share_buf	1	/* do input virtual channels physically share buffers? */
#define PARM_out_share_buf	1	/* do output virtual channels physically share buffers? */
/* ?? */
#define PARM_in_share_switch	1	/* do input virtual channels share crossbar input ports? */
#define PARM_out_share_switch	1	/* do output virtual channels share crossbar output ports? */

/* crossbar parameters */
#define PARM_crossbar_model	MULTREE_CROSSBAR/* crossbar model type */
#define PARM_crsbar_degree	4		/* crossbar mux degree */
#define PARM_connect_type	TRANS_GATE	/* crossbar connector type */
#define PARM_trans_type		NP_GATE		/* crossbar transmission gate type */
#define PARM_crossbar_in_len	0		/* crossbar input line length, if known */
#define PARM_crossbar_out_len	0		/* crossbar output line length, if known */
#define PARM_xb_in_seg		0
#define PARM_xb_out_seg		0
/* HACK HACK HACK */
#define PARM_exp_xb_model	MATRIX_CROSSBAR
#define PARM_exp_in_seg		2
#define PARM_exp_out_seg	2


/* input buffer parameters */
#define PARM_in_buf		1	/* have input buffer? */
#define PARM_in_buf_set		4	/* # of rows */
#define PARM_in_buf_rport	1	/* # of read ports */

#define PARM_cache_in_buf	0
#define PARM_cache_in_buf_set	0
#define PARM_cache_in_buf_rport	0

#define PARM_mc_in_buf		0
#define PARM_mc_in_buf_set	0
#define PARM_mc_in_buf_rport	0

#define PARM_io_in_buf		0
#define PARM_io_in_buf_set	0
#define PARM_io_in_buf_rport	0

/* output buffer parameters */
#define PARM_out_buf		0
#define PARM_out_buf_set	16
#define PARM_out_buf_wport	1

/* central buffer parameters */
#define PARM_central_buf	0	/* have central buffer? */
#define PARM_cbuf_set		2560	/* # of rows */
#define PARM_cbuf_rport		2	/* # of read ports */
#define PARM_cbuf_wport		2	/* # of write ports */
#define PARM_cbuf_width		4	/* # of flits in one row */
#define PARM_pipe_depth		4	/* # of banks */

/* array parameters shared by various buffers */
#define PARM_wordline_model	CACHE_RW_WORDLINE
#define PARM_bitline_model	RW_BITLINE
#define PARM_mem_model		NORMAL_MEM
#define PARM_row_dec_model	GENERIC_DEC
#define PARM_row_dec_pre_model	SINGLE_OTHER
#define PARM_col_dec_model	SIM_NO_MODEL
#define PARM_col_dec_pre_model	SIM_NO_MODEL
#define PARM_mux_model		SIM_NO_MODEL
#define PARM_outdrv_model	REG_OUTDRV

/* these 3 should be changed together */
/* use double-ended bitline because the array is too large */
#define PARM_data_end		2
#define PARM_amp_model		GENERIC_AMP
#define PARM_bitline_pre_model	EQU_BITLINE
//#define PARM_data_end		1
//#define PARM_amp_model		SIM_NO_MODEL
//#define PARM_bitline_pre_model	SINGLE_OTHER

/* arbiter parameters */
#define PARM_in_arb_model	MATRIX_ARBITER	/* input side arbiter model type */
#define PARM_in_arb_ff_model	NEG_DFF		/* input side arbiter flip-flop model type */
#define PARM_out_arb_model	MATRIX_ARBITER	/* output side arbiter model type */
#define PARM_out_arb_ff_model	NEG_DFF		/* output side arbiter flip-flop model type */

#endif	/* _SIM_PORT_H */
