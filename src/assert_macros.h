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
 * File         : assert.h
 * Author       : HPArch
 * Date         : 10/15/1997
 * SVN          : $Id: assert.h,v 1.2 2008-09-12 03:07:28 kacear Exp $:
 * Description  : This is my own set of assert macros.  They print out the usual 'assert'
 *                stuff, but can be turned off globally.  ASSERTM also allows you
 *                to specify some printf arguments in addition to the condition.  The
 *                text will be printed if the assertion fails.
 *********************************************************************************************/

#ifndef __ASSERT_H__
#define __ASSERT_H__


#include <stdlib.h>
#include <stdio.h>
#include "utils.h"
#include <cassert>


#ifdef NO_ASSERT
#define ENABLE_ASSERTIONS   false	/* default false */
#else
#define ENABLE_ASSERTIONS   true	/* default true */
#endif

#define WRITE_STATUS(args...) \
    { \
        if (m_simBase->g_mystatus) { \
            fprintf(m_simBase->g_mystatus, ## args); \
            fprintf(m_simBase->g_mystatus, " %d\n", (int) time(NULL));	\
            fflush(m_simBase->g_mystatus); \
        } \
    }

//#define assert(cond) ASSERT(cond)


///////////////////////////////////////////////////////////////////////////////////////////////


#define ASSERT(cond)										\
  do {												\
    if (ENABLE_ASSERTIONS && !(cond)) {								\
      fflush(m_simBase->g_mystdout);									\
      fprintf(m_simBase->g_mystderr, "\n");								\
      fprintf(m_simBase->g_mystderr, "%s:%d: ASSERT FAILED (I=%s  C=%s):  ", __FILE__, __LINE__,	\
          intstr64(m_simBase->m_core0_inst_count), intstr64(m_simBase->m_simulation_cycle));		\
      fprintf(m_simBase->g_mystderr, "%s\n", #cond);							\
      breakpoint(__FILE__, __LINE__);								\
      WRITE_STATUS("ASSERT");									\
      exit(15);                                                                               \
    }												\
  } while (0)


///////////////////////////////////////////////////////////////////////////////////////////////


#ifndef NO_REPORT
#define ASSERTM(cond, args...)									\
  do {												\
    \
    if (ENABLE_ASSERTIONS && !(cond)) {								\
      fflush(m_simBase->g_mystdout);									\
      fprintf(m_simBase->g_mystderr, "\n");								\
      fprintf(m_simBase->g_mystderr, "%s:%d: ASSERT FAILED (I=%s  C=%s):  ", __FILE__, __LINE__,	\
          intstr64(m_simBase->m_core0_inst_count), intstr64(m_simBase->m_simulation_cycle));		\
      fprintf(m_simBase->g_mystderr, "%s\n", #cond);							\
      fprintf(m_simBase->g_mystderr, "%s:%d: ASSERT FAILED (I=%s  C=%s):  ", __FILE__, __LINE__,	\
          intstr64(m_simBase->m_core0_inst_count), intstr64(m_simBase->m_simulation_cycle));		\
      fprintf(m_simBase->g_mystderr, ## args);								\
      breakpoint(__FILE__, __LINE__);								\
      WRITE_STATUS("ASSERT");									\
      exit(15);               \
    }												\
  } while (0)
#else
#define ASSERTM(cond, args...)  assert(cond);
#endif


///////////////////////////////////////////////////////////////////////////////////////////////


#define ASSERTU(cond)										\
  do {												\
    \
    if (!(cond)) {										\
      fflush(m_simBase->g_mystdout);									\
      fprintf(m_simBase->g_mystderr, "\n");								\
      fprintf(m_simBase->g_mystderr, "%s:%d: ASSERT FAILED (I=%s  C=%s):  ", __FILE__, __LINE__,	\
          intstr64(m_simBase->m_core0_inst_count), intstr64(m_simBase->m_simulation_cycle));		\
      fprintf(m_simBase->g_mystderr, "%s\n", #cond);							\
      breakpoint(__FILE__, __LINE__);								\
      WRITE_STATUS("ASSERT");									\
      exit(15);										\
    }												\
  } while (0)


///////////////////////////////////////////////////////////////////////////////////////////////


#define ASSERTUM(cond, args...)									\
  do {												\
    if (!(cond)) {										\
      fflush(m_simBase->g_mystdout);									\
      fprintf(m_simBase->g_mystderr, "\n");								\
      fprintf(m_simBase->g_mystderr, "%s:%d: ASSERT FAILED (I=%s  C=%s):  ", __FILE__, __LINE__,	\
          intstr64(m_simBase->m_core0_inst_count), intstr64(m_simBase->m_simulation_cycle));		\
      fprintf(m_simBase->g_mystderr, "%s\n", #cond);							\
      fprintf(m_simBase->g_mystderr, "%s:%d: ASSERT FAILED (I=%s  C=%s):  ", __FILE__, __LINE__,	\
          intstr64(m_simBase->m_core0_inst_count), intstr64(m_simBase->m_simulation_cycle));		\
      fprintf(m_simBase->g_mystderr, ## args);								\
      breakpoint(__FILE__, __LINE__);								\
      WRITE_STATUS("ASSERT");									\
      exit(15);										\
    }												\
  } while (0)


///////////////////////////////////////////////////////////////////////////////////////////////

#endif  /* #ifndef __ASSERT_H__ */
