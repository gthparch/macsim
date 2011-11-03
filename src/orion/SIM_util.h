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

#ifndef _SIM_UTIL_H
#define _SIM_UTIL_H

extern u_int SIM_Hamming(LIB_Type_max_uint old_val, LIB_Type_max_uint new_val, LIB_Type_max_uint mask);
extern u_int SIM_Hamming_group(LIB_Type_max_uint d1_new, LIB_Type_max_uint d1_old, LIB_Type_max_uint d2_new, LIB_Type_max_uint d2_old, u_int width, u_int n_grp);

/* statistical functions */
extern int SIM_print_stat_energy(char *path, double Energy, int print_flag);
extern u_int SIM_strlen(char *s);
extern char *SIM_strcat(char *dest, const char *src);
extern int SIM_res_path(char *path, u_int id);
extern int SIM_dump_tech_para(void);

extern u_int SIM_logtwo(LIB_Type_max_uint x);

extern int SIM_squarify(int rows, int cols);
extern double SIM_driver_size(double driving_cap, double desiredrisetime);

#endif /* _SIM_UTIL_H */

