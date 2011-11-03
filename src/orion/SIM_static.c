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

#include "SIM_static.h"

#if (PARM(TECH_POINT) == 90 && PARM(TRANSISTOR_TYPE) == LVT)
double NMOS_TAB[1] = {19.9e-9};
double PMOS_TAB[1] = {16.6e-9};
double NAND2_TAB[4] = {7.8e-9, 24.6e-9, 14.1e-9, 34.3e-9};
double NOR2_TAB[4] = {51.2e-9, 23.9e-9, 19.5e-9, 8.4e-9};
double DFF_TAB[1] = {219.7e-9};
#elif (PARM(TECH_POINT) == 90 && PARM(TRANSISTOR_TYPE) == NVT)
double NMOS_TAB[1] = {15.6e-9};
double PMOS_TAB[1] = {11.3e-9};
double NAND2_TAB[4] = {2.8e-9, 19.6e-9, 10.4e-9, 29.3e-9};
double NOR2_TAB[4] = {41.5e-9, 13.1e-9, 14.5e-9, 1.4e-9};
double DFF_TAB[1] = {194.7e-9};
#elif (PARM(TECH_POINT) == 90 && PARM(TRANSISTOR_TYPE) == HVT)
double NMOS_TAB[1] = {12.2e-9};
double PMOS_TAB[1] = {9.3e-9};
double NAND2_TAB[4] = {1.8e-9, 12.4e-9, 8.9e-9, 19.3e-9};
double NOR2_TAB[4] = {29.5e-9, 8.3e-9, 11.1e-9, 0.9e-9};
double DFF_TAB[1] = {194.7e-9};
#elif (PARM(TECH_POINT) <= 65 && PARM(TRANSISTOR_TYPE) == LVT) 
double NMOS_TAB[1] = {311.7e-9};
double PMOS_TAB[1] = {674.3e-9};
double NAND2_TAB[4] = {303.0e-9, 423.0e-9, 498.3e-9, 626.3e-9};
double NOR2_TAB[4] ={556.0e-9, 393.7e-9, 506.7e-9, 369.7e-9};
double DFF_TAB[1] = {970.4e-9};
#elif (PARM(TECH_POINT) <= 65 && PARM(TRANSISTOR_TYPE) == NVT) 
double NMOS_TAB[1] = {115.1e-9};
double PMOS_TAB[1] = {304.8e-9};
double NAND2_TAB[4] = {111.4e-9, 187.2e-9, 230.7e-9, 306.9e-9};
double NOR2_TAB[4] ={289.7e-9, 165.7e-9, 236.9e-9, 141.4e-9};
double DFF_TAB[1] ={400.3e-9};
#elif (PARM(TECH_POINT) <= 65 && PARM(TRANSISTOR_TYPE) == HVT) 
double NMOS_TAB[1] = {18.4e-9};
double PMOS_TAB[1] = {35.2e-9};
double NAND2_TAB[4] = {19.7e-9, 51.3e-9, 63.0e-9, 87.6e-9};
double NOR2_TAB[4] ={23.4e-9, 37.6e-9, 67.9e-9, 12.3e-9};
double DFF_TAB[1] = {231.3e-9};
#elif(PARM(TECH_POINT) >= 110 ) 
/* HACK: we assume leakage power above 110nm is small, so we don't provide leakage power for 110nm and above */
double NMOS_TAB[1] = {0};
double PMOS_TAB[1] = {0};
double NAND2_TAB[4] = {0,0,0,0};
double NOR2_TAB[4] ={0,0,0,0};
double DFF_TAB[1] = {0};
#endif

