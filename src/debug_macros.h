/*
Copyright (c) <2012>, <Georgia Institute of Technology> All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted 
provided that the following conditions are met:

Redistributions of source code must retain the above copyright notice, this list of conditions 
and the following disclaimer.

Redistributions in binary form must reproduce the above copyright notice, this list of 
conditions and the following disclaimer in the documentation and/or other materials provided 
with the distribution.

Neither the name of the <Georgia Institue of Technology> nor the names of its contributors 
may be used to endorse or promote products derived from this software without specific prior 
written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR 
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY 
AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR 
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
POSSIBILITY OF SUCH DAMAGE.
*/


/**********************************************************************************************
 * File         : debug_macros.h
 * Author       : HPArch
 * Date         : 4/14/1998
 * CVS          : $Id: debug_macros.h,v 1.4 2008-09-12 03:07:29 kacear Exp $:
 * Description  : This file contains a debugging macro that is intended to be used in
		    another macro.  For example, put something like this in your .c
		    file:
    #define DEBUG(args...)		_DEBUG(DEBUG_IMAGE, ## args)
 *********************************************************************************************/

#ifndef __DEBUG_MACROS_H__


#include <stdio.h>
#include <iostream>

#include "utils.h"

///////////////////////////////////////////////////////////////////////////////////////////////


// cout statement + file and line information
#ifndef NO_REPORT
#define report(x) cout << __FILE__ << ":" << __LINE__ \
  << ": (I=" << m_simBase->m_core0_inst_count \
  << "  C="  << m_simBase->m_simulation_cycle << "):  " << x << "\n";
#else
#define report(x) {}
#endif

#ifndef NO_REPORT
// printf statement + file and line information
#define REPORT(args...)                                     \
  do {                                                                  \
    fprintf(m_simBase->g_mystdout, "%s:%u: (I=%llu  C=%llu):  ", __FILE__, __LINE__, \
        m_simBase->m_core0_inst_count, \
        m_simBase->m_simulation_cycle); \
    fprintf(GLOBAL_DEBUG_STREAM, ## args);                            \
    fflush(GLOBAL_DEBUG_STREAM);                                      \
  } while (0)
#else
#define REPORT(args...) {}
#endif


///////////////////////////////////////////////////////////////////////////////////////////////


#define DEBUG_RANGE_COND                                                \
  (((*KNOB(KNOB_DEBUG_INST_START) && m_simBase->m_core0_inst_count >= *KNOB(KNOB_DEBUG_INST_START)) && \
   (!*KNOB(KNOB_DEBUG_INST_STOP)  || m_simBase->m_core0_inst_count <= *KNOB(KNOB_DEBUG_INST_STOP))) \
   ||                                                                   \
   ((*KNOB(KNOB_DEBUG_CYCLE_START) && m_simBase->m_simulation_cycle >= *KNOB(KNOB_DEBUG_CYCLE_START)) && \
   (!*KNOB(KNOB_DEBUG_CYCLE_STOP) || m_simBase->m_simulation_cycle <= *KNOB(KNOB_DEBUG_CYCLE_STOP)))) 
 

#ifdef NO_DEBUG
#define ENABLE_GLOBAL_DEBUG_PRINT	false	    /* default false */
#else
#define ENABLE_GLOBAL_DEBUG_PRINT	true	    /* default true */
#endif

#define GLOBAL_DEBUG_STREAM		m_simBase->g_mystdout    /* default g_mystdout */


///////////////////////////////////////////////////////////////////////////////////////////////


#if ENABLE_GLOBAL_DEBUG_PRINT
//  original code (hyesoon 3.19.06) 
#define _DEBUG(debug_flag, args...)                                     \
  do {                                                                  \
    if (debug_flag && DEBUG_RANGE_COND) {                               \
      fprintf(GLOBAL_DEBUG_STREAM, "%s:%u: " # debug_flag " (I=%llu  C=%llu):  ", \
          __FILE__, __LINE__, m_simBase->m_core0_inst_count, m_simBase->m_simulation_cycle); \
      fprintf(GLOBAL_DEBUG_STREAM, ## args);                            \
      fflush(GLOBAL_DEBUG_STREAM);                                      \
    }                                                                   \
  } while (0)
#else
#define _DEBUG(debug_flag, args...)                   do {} while (0)
#endif


#define __DEBUG_MACROS_H__
#endif  /* #ifndef __DEBUG_MACROS_H__ */
