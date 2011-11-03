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

#ifndef	_SIM_TECHNOLOGY_H
#define	_SIM_TECHNOLOGY_H

#define PARM_AF (5.000000e-01)
#define PARM_MAXN (8)
#define PARM_MAXSUBARRAYS (8)
#define PARM_MAXSPD (8)
#define PARM_VTHOUTDRNOR (4.310000e-01)	
#define PARM_VTHCOMPINV (4.370000e-01)
#define PARM_BITOUT (64)
#define PARM_ruu_issue_width (4)
#define PARM_amp_Idsat (5.000000e-04) 
#define PARM_VSINV (4.560000e-01)
#define PARM_GEN_POWER_FACTOR (1.310000e+00)
#define PARM_VTHNAND60x90 (5.610000e-01)
#define PARM_FUDGEFACTOR (1.000000e+00)
#define PARM_VTHOUTDRIVE (4.250000e-01)
#define PARM_VTHMUXDRV1 (4.370000e-01)
#define PARM_VTHMUXDRV2 (4.860000e-01)
#define PARM_NORMALIZE_SCALE (6.488730e-10)
#define PARM_VTHMUXDRV3 (4.370000e-01)
#define PARM_ADDRESS_BITS (64)
#define PARM_RUU_size (16)
#define PARM_VTHNOR12x4x1 (5.030000e-01)
#define PARM_VTHNOR12x4x2 (4.520000e-01)
#define PARM_VTHOUTDRINV (4.370000e-01)
#define PARM_VTHNOR12x4x3 (4.170000e-01)
#define PARM_VTHEVALINV (2.670000e-01)
#define PARM_VTHNOR12x4x4 (3.900000e-01)
#define PARM_res_ialu (4)
#define PARM_VTHOUTDRNAND (4.410000e-01)
#define PARM_VTHINV100x60 (4.380000e-01)

#if (PARM(TECH_POINT) >= 110 )
#define PARM_Cgatepass (1.450000e-15) 
#define PARM_Cpdiffarea (6.060000e-16) 
#define PARM_Cpdiffside (2.400000e-16) 
#define PARM_Cndiffside (2.400000e-16)
#define PARM_Cndiffarea (6.600000e-16) 
#define PARM_Cnoverlap (1.320000e-16)
#define PARM_Cpoverlap (1.210000e-16)
#define PARM_Cgate (9.040000e-15) 
#define PARM_Cpdiffovlp (1.380000e-16)
#define PARM_Cndiffovlp (1.380000e-16)
#define PARM_Cnoxideovlp (2.230000e-16)
#define PARM_Cpoxideovlp (3.380000e-16)

#elif (PARM(TECH_POINT) <= 90 )
#if (PARM(TRANSISTOR_TYPE) == LVT)
#define PARM_Cgatepass (1.5225000e-14) 
#define PARM_Cpdiffarea (6.05520000e-15) 
#define PARM_Cpdiffside (2.38380000e-15)
#define PARM_Cndiffside (2.8500000e-16)  
#define PARM_Cndiffarea (5.7420000e-15) 
#define PARM_Cnoverlap (1.320000e-16) 
#define PARM_Cpoverlap (1.210000e-16)
#define PARM_Cgate (7.8648000e-14) 
#define PARM_Cpdiffovlp (1.420000e-16)
#define PARM_Cndiffovlp (1.420000e-16)
#define PARM_Cnoxideovlp (2.580000e-16)
#define PARM_Cpoxideovlp (3.460000e-16)

#elif (PARM(TRANSISTOR_TYPE) == NVT)
#define PARM_Cgatepass (8.32500e-15) 
#define PARM_Cpdiffarea (3.330600e-15) 
#define PARM_Cpdiffside (1.29940000e-15)
#define PARM_Cndiffside (2.5500000e-16) 
#define PARM_Cndiffarea (2.9535000e-15) 
#define PARM_Cnoverlap (1.270000e-16)
#define PARM_Cpoverlap (1.210000e-16)
#define PARM_Cgate (3.9664000e-14)
#define PARM_Cpdiffovlp (1.31000e-16) 
#define PARM_Cndiffovlp (1.310000e-16)
#define PARM_Cnoxideovlp (2.410000e-16)  
#define PARM_Cpoxideovlp (3.170000e-16)

#elif  (PARM(TRANSISTOR_TYPE) == HVT)
#define PARM_Cgatepass (1.45000e-15) 
#define PARM_Cpdiffarea (6.06000e-16) 
#define PARM_Cpdiffside (2.150000e-16)  
#define PARM_Cndiffside (2.25000e-16)   
#define PARM_Cndiffarea (1.650000e-16)  
#define PARM_Cnoverlap (1.220000e-16)
#define PARM_Cpoverlap (1.210000e-16)
#define PARM_Cgate (6.8000e-16) 	
#define PARM_Cpdiffovlp (1.20000e-16) 
#define PARM_Cndiffovlp (1.20000e-16) 
#define PARM_Cnoxideovlp (2.230000e-16)
#define PARM_Cpoxideovlp (2.880000e-16)
#endif /*PARM(TRANSISTOR_TYPE) */

#endif /*PARM(TECH_POINT)*/

#include "SIM_technology_v1.h"
#include "SIM_technology_v2.h"

#endif /* _SIM_TECHNOLOGY_H */
