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

#ifndef _SIM_PARAMETER_H
#define _SIM_PARAMETER_H

#include <unistd.h>
#include <sys/types.h>

/*Useful macros, wrappers and functions */
#define __INSTANCE__ mainpe__power
#define GLOBDEF(t,n) t mainpe__power___ ## n
#define GLOB(n) mainpe__power___ ## n
#define FUNC(n, args...) mainpe__power___ ## n (args)
#define FUNCPTR(n)  mainpe__power___ ## n 
#define PARM(x) PARM_ ## x

#define LIB_Type_max_uint       u_int64_t
#define LIB_Type_max_int        int64_t

#ifndef MAX
#define MAX(a,b) (((a)>(b))?(a):(b))
#endif
#ifndef MIN
#define MIN(a,b) (((a)>(b))?(b):(a))
#endif

#define NEXT_DEPTH(d)	((d) > 0 ? (d) - 1 : (d))

#define BIGNUM	(1e30)
#define BIGONE	((LIB_Type_max_uint)1)
#define BIGNONE	((LIB_Type_max_uint)-1)
#define HAMM_MASK(w)	((u_int)w < (sizeof(LIB_Type_max_uint) << 3) ? (BIGONE << (w)) - 1 : BIGNONE)

#define FALSE	0
#define TRUE	1
/* Used to communicate with the horowitz model */
#define RISE 1
#define FALL 0
#define NCH  1
#define PCH  0

#define MAX_ENERGY	1
#define AVG_ENERGY	0
/*End useful macros, wrappers and functions */

/*Available models for technology, router and link */
/*transistor type model */
#define LVT 1
#define NVT 2
#define HVT 3
#define	SIM_NO_MODEL	0
#include "SIM_ALU_model.h"
#include "SIM_array_model.h"
#include "SIM_link_model.h"
#include "SIM_misc_model.h"
#include "SIM_permu_model.h"
#include "SIM_router_model.h"
/* End available models for router and link */

/* User-defined router with selected tech node and operating freq and Vdd
 * CAUTION: Please do not alter the location of this SIM_port.h file, sequency tricky
 */ 
#include "SIM_port.h"
/*End user-defined router with selected tech node and operating freq and Vdd */

/* Useful macros related to parameters defined by users */
#define Vdd             (PARM(Vdd))
#define Period          ((double)1/(double)PARM(Freq))

#define Powerfactor		((PARM(Freq))*Vdd*Vdd)
#define EnergyFactor	(Vdd*Vdd)
/* End useful macros related to parameters defined by users */

/* Technology file */
#include "SIM_technology.h"
/* End technology file */

#endif /* _SIM_PARAMETER_H */
