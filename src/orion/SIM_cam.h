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

#ifndef _SIM_CAM_H
#define _SIM_CAM_H

extern double SIM_cam_wordline_cap( u_int cols, double wire_cap, double tx_width );
extern double SIM_cam_comp_tagline_cap( u_int rows, double taglinelength );
extern double SIM_cam_comp_mismatch_cap( u_int n_bits, u_int n_pre, double matchline_len );
extern double SIM_cam_comp_miss_cap( u_int assoc );
extern double SIM_cam_tag_mem_cap( u_int read_ports, u_int write_ports, int share_rw, u_int end, int only_write );
extern double SIM_cam_data_mem_cap( u_int read_ports, u_int write_ports );

#endif /* _SIM_CAM_H */

