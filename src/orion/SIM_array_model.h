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

#ifndef _SIM_ARRAY_MODEL_H
#define _SIM_ARRAY_MODEL_H

/* WHS: all model types begin with 1 because 0 is reserved for SIM_ARRAY_NO_MODEL */
#define SIM_ARRAY_NO_MODEL	0

/*@
 * data type: decoder model types
 *
 *   GENERIC_DEC   -- default type
 *   DEC_MAX_MODEL -- upper bound of model type
 */
typedef enum {
	GENERIC_DEC = 1,
	DEC_MAX_MODEL
} SIM_dec_model_t;

/*@
 * data type: multiplexor model types
 *
 *   GENERIC_MUX   -- default type
 *   MUX_MAX_MODEL -- upper bound of model type
 */
typedef enum {
	GENERIC_MUX = 1,
	MUX_MAX_MODEL
} SIM_mux_model_t;

/*@
 * data type: sense amplifier model types
 *
 *   GENERIC_AMP   -- default type
 *   AMP_MAX_MODEL -- upper bound of model type
 */
typedef enum {
	GENERIC_AMP = 1,
	AMP_MAX_MODEL
} SIM_amp_model_t;

/*@
 * data type: wordline model types
 *
 *   CACHE_RW_WORDLINE  -- default type
 *   CACHE_WO_WORDLINE  -- write data wordline only, for fully-associative data bank
 *   CAM_RW_WORDLINE    -- both R/W tag wordlines, for fully-associative write-back
 *                         tag bank
 *   CAM_WO_WORDLINE    -- write tag wordline only, for fully-associative write-through
 *                         tag bank
 *   WORDLINE_MAX_MODEL -- upper bound of model type
 */
typedef enum {
	CACHE_RW_WORDLINE = 1,
	CACHE_WO_WORDLINE,
	CAM_RW_WORDLINE,
	CAM_WO_WORDLINE,
	WORDLINE_MAX_MODEL
} SIM_wordline_model_t;

/*@
 * data type: bitline model types
 * 
 *   RW_BITLINE	       -- default type
 *   WO_BITLINE        -- write bitline only, for fully-associative data bank and
 *                        fully-associative write-through tag bank
 *   BITLINE_MAX_MODEL -- upper bound of model type
 */
typedef enum {
	RW_BITLINE = 1,
	WO_BITLINE,
	BITLINE_MAX_MODEL
} SIM_bitline_model_t;

/*@
 * data type: precharge model types
 *
 *   PRE_MAX_MODEL -- upper bound of model type
 */
typedef enum {
	SINGLE_BITLINE = 1,
	EQU_BITLINE,
	SINGLE_OTHER,
	PRE_MAX_MODEL
} SIM_pre_model_t;

/*@
 * data type: memory cell model types
 *
 *   NORMAL_MEM     -- default type
 *   CAM_TAG_RW_MEM -- read/write memory cell connected with tag comparator, for
 *                     fully-associative write-back tag bank
 *   CAM_TAG_WO_MEM -- write-only memory cell connected with tag comparator, for
 *                     fully-associative write-through tag bank
 *   CAM_DATA_MEM   -- memory cell connected with output driver, for fully-associative
 *                     data bank
 *   CAM_ATTACH_MEM -- memory cell of fully-associative array valid bit, use bit, etc.
 *   MEM_MAX_MODEL  -- upper bound of model type
 */
typedef enum {
	NORMAL_MEM = 1,
	CAM_TAG_RW_MEM,
	CAM_TAG_WO_MEM,
	CAM_DATA_MEM,
	CAM_ATTACH_MEM,
	MEM_MAX_MODEL
} SIM_mem_model_t;

/*@
 * data type: tag comparator model types
 *
 *   CACHE_COMP     -- default type
 *   CAM_COMP       -- cam-style tag comparator, for fully-associative array
 *   COMP_MAX_MODEL -- upper bound of model type
 */
typedef enum {
	CACHE_COMP = 1,
	CAM_COMP,
	COMP_MAX_MODEL
} SIM_comp_model_t;

/*@
 * data type: output driver model types
 *
 *   CACHE_OUTDRV     -- default type
 *   CAM_OUTDRV       -- output driver connected with memory cell, for fully-associative
 *                       array
 *   REG_OUTDRV       -- output driver connected with bitline, for register files
 *   OUTDRV_MAX_MODEL -- upper bound of model type
 */
typedef enum {
	CACHE_OUTDRV = 1,
	CAM_OUTDRV,
	REG_OUTDRV,
	OUTDRV_MAX_MODEL
} SIM_outdrv_model_t;


#endif /* _SIM_ARRAY_MODEL_H */

