/**********************************************************************************************
 * File         : debug_macros.h
 * Author       : Robert S. Chappell
 * Date         : 4/14/1998
 * CVS          : $Id: debug_macros.h,v 1.4 2008-09-12 03:07:29 kacear Exp $:
 * Description  : This file contains a debugging macro that is intended to be used in
		    another macro.  For example, put something like this in your .c
		    file:
    #define DEBUG(args...)		_DEBUG(DEBUG_IMAGE, ## args)
 *********************************************************************************************/

#ifndef __DEBUG_MACROS_H__


#include <stdio.h>

#include "utils.h"

///////////////////////////////////////////////////////////////////////////////////////////////


// cout statement + file and line information
#ifndef NO_REPORT
#define report(x) cout << __FILE__ << ":" << __LINE__ \
  << ": (I=" << unsstr64(m_simBase->m_core0_inst_count) \
  << "  C="  << unsstr64(m_simBase->m_simulation_cycle) << "):  " << x << "\n";
#else
#define report(x) {}
#endif

#ifndef NO_REPORT
// printf statement + file and line information
#define REPORT(args...)                                     \
  do {                                                                  \
    fprintf(m_simBase->g_mystdout, "%s:%u: (I=%s  C=%s):  ", __FILE__, __LINE__, \
        unsstr64(m_simBase->m_core0_inst_count), \
        unsstr64(m_simBase->m_simulation_cycle)); \
    fprintf(GLOBAL_DEBUG_STREAM, ## args);                            \
    fflush(GLOBAL_DEBUG_STREAM);                                      \
  } while (0)
#else
#define REPORT(args...) {}
#endif


///////////////////////////////////////////////////////////////////////////////////////////////


#define DEBUG_RANGE_COND                                                \
  ((*m_simBase->m_knobs->KNOB_DEBUG_INST_START && m_simBase->m_core0_inst_count >= *m_simBase->m_knobs->KNOB_DEBUG_INST_START) && \
   (!*m_simBase->m_knobs->KNOB_DEBUG_INST_STOP  || m_simBase->m_core0_inst_count <= *m_simBase->m_knobs->KNOB_DEBUG_INST_STOP) \
   ||                                                                   \
   (*m_simBase->m_knobs->KNOB_DEBUG_CYCLE_START && m_simBase->m_simulation_cycle >= *m_simBase->m_knobs->KNOB_DEBUG_CYCLE_START) && \
   (!*m_simBase->m_knobs->KNOB_DEBUG_CYCLE_STOP || m_simBase->m_simulation_cycle <= *m_simBase->m_knobs->KNOB_DEBUG_CYCLE_STOP)) 
 

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
      fprintf(GLOBAL_DEBUG_STREAM, "%s:%u: " # debug_flag " (I=%s  C=%s):  ", \
          __FILE__, __LINE__, unsstr64(m_simBase->m_core0_inst_count), \
          unsstr64(m_simBase->m_simulation_cycle)); \
      fprintf(GLOBAL_DEBUG_STREAM, ## args);                            \
      fflush(GLOBAL_DEBUG_STREAM);                                      \
    }                                                                   \
  } while (0)
#else
#define _DEBUG(debug_flag, args...)                   do {} while (0)
#endif


#define __DEBUG_MACROS_H__
#endif  /* #ifndef __DEBUG_MACROS_H__ */
