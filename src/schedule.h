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
 * File         : schedule.h
 * Author       : Hyesoon Kim
 * Date         : 1/1/2008 
 * SVN          : $Id: main.cc,v 1.26 2008-09-21 00:02:54 kacear Exp $:
 * Description  : schedule instructions
 *********************************************************************************************/

#ifndef SCHEDULE_H_INCLUDED 
#define SCHEDULE_H_INCLUDED 


#include "global_types.h"
#include "global_defs.h"
#include "uop.h"
#include "pqueue.h"


#define SCHEDULE_INTERFACE_PARAMS() \
    int core_id, \
    pqueue_c<int>** q_iaq, \
    pqueue_c<gpu_allocq_entry_s>** gpu_q_iaq, \
    rob_c* rob, \
    smc_rob_c* gpu_rob, \
    int num_iaq, \
    exec_c* exec, \
    Unit_Type unit_type, \
    frontend_c* frontend \
// end macro


#define SCHEDULE_INTERFACE_DECL() \
    int core_id; \
    pqueue_c<int>** q_iaq; \
    pqueue_c<gpu_allocq_entry_s>** gpu_q_iaq; \
    rob_c* rob; \
    smc_rob_c* gpu_rob; \
    int num_iaq; \
    exec_c* exec; \
    Unit_Type unit_type; \
    frontend_c* frontend; \
// end macro


#define SCHEDULE_INTERFACE_ARGS() \
    core_id, \
    q_iaq, \
    gpu_q_iaq, \
    rob, \
    gpu_rob, \
    num_iaq, \
    exec, \
    unit_type, \
    frontend \
// end macro


#define SCHEDULE_INTERFACE_INIT() \
    core_id ( core_id ), \
    q_iaq ( q_iaq ), \
    gpu_q_iaq ( gpu_q_iaq ), \
    rob ( rob ), \
    gpu_rob ( gpu_rob ), \
    num_iaq ( num_iaq ), \
    exec ( exec ), \
    unit_type ( unit_type ), \
    frontend ( frontend ) \
// end macro


#define SCHEDULE_INTERFACE_CAST() \
    static_cast<void>(core_id); \
    static_cast<void>(q_iaq); \
    static_cast<void>(gpu_q_iaq); \
    static_cast<void>(rob); \
    static_cast<void>(gpu_rob); \
    static_cast<void>(num_iaq); \
    static_cast<void>(exec); \
    static_cast<void>(unit_type); \
    static_cast<void>(frontend); \
// end macro


/**
 * Scheduling failure type
 */
enum SCHED_FAIL_TYPE
{
  SCHED_SUCCESS,
  SCHED_FAIL_OPERANDS_NOT_READY,
  SCHED_FAIL_NO_AVAILABLE_PORTS,
  SCHED_FAIL_NO_MEM_REQ_SLOTS
};  


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Instruction scheduler base class
///////////////////////////////////////////////////////////////////////////////////////////////
class schedule_c
{
  public:
    /**
     * Constructor
     */
    schedule_c(exec_c* exec, int core_id, Unit_Type unit_type, frontend_c* frontend,
        pqueue_c<int>** alloc_q, macsim_c* simBase);

    /**
     * Destructor
     */
    virtual ~schedule_c();

    /**
     * Tick a cycle
     */
    virtual void run_a_cycle(void) = 0;
    
    /*! \fn void start()
     *  \brief Function to indicate the starting of the scheduler.
     *  \return void 
     */
    virtual void start(void);

    /*! \fn void stop()
     *  \brief Function to indicate the stopping of the scheduler.
     *  \return void 
     */
    virtual void stop(void);

    /*! \fn void is_running()
     *  \brief Function to check if the simulator is running.
     *  \return void 
     */
    virtual bool is_running(void);
    
  protected:
    /**
     *  \brief Function to check if the source of an uop are yet tp be scheduled
     *  \param entry - ROB entry of the uop
     *  \return bool - True if no dependency found
     */
    virtual bool check_srcs(int entry);
    
    /**
     *  \brief Function to schedule uops 
     *  \param entry - ROB entry of the uop
     *  \param sched_fail_reason - Reason of uop schedule failure
     *  \return bool - True on success in scheduling
     */
    virtual bool uop_schedule(int entry, SCHED_FAIL_TYPE* sched_fail_reason);
    
    /*! \fn void advance(int ALLOCQ_index)
     *  \brief Function to move the allocation queue ahead.
     *  \param ALLOCQ_index - Aloocation queue number to be moved ahead
     *  \return void 
     */
    virtual void advance(int ALLOCQ_index);

  protected:
    static const int MAX_SCHED_SIZE = 8192; /**< maximum scheduler table size */

    int             m_core_id;  /**< core id */ 
    pqueue_c<int>** m_alloc_q;  /**< allocation queue */
    rob_c*          m_rob;      /**< reorder buffer */
    exec_c*         m_exec;     /**< execution stage pointer */
    Unit_Type       m_unit_type; /**< core type */
    frontend_c*     m_frontend; /**< frontend pointer */
    Counter         m_cur_core_cycle; /**< current core cycle */
    uns16           m_knob_width; /**< width */
    int             m_num_in_sched; /**< number of uops in scheduler */
    bool            m_schedule_running; /**< enabled scheduler */
    int             m_count[max_ALLOCQ]; /**< total scheduled uop */
    int*            m_sched_size; /**< schedule size per inst. type */
    int*            m_sched_rate; /**< schedule rate per inst. type */
    int*            m_num_per_sched; /**< number of uops in each sched. */
    int             m_schedule_list[MAX_SCHED_SIZE]; /**< schedule list in OOO */
    int             m_first_schlist_ptr; /**< first index to sched list in OOO */
    int             m_last_schlist_ptr; /**< last index to sched list in OOO */
    uns16           m_knob_sched_to_width; /**< knob sched to width FIXME */

   macsim_c* m_simBase;         /**< macsim_c base class for simulation globals */


  public:
    /**
     * Private constructor
     */
    schedule_c(); // do not implement
};

#endif
