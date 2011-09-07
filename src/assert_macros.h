/**********************************************************************************************
 * File         : assert.h
 * Author       : Robert S. Chappell
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
