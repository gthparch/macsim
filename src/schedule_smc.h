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
 * File         : schedule_smc.cc
 * Author       : Hyesoon Kim
 * Date         : 1/1/2008 
 * SVN          : $Id: main.cc,v 1.26 2008-09-21 00:02:54 kacear Exp $:
 * Description  : scheduler for gpu (small many cores)
 *********************************************************************************************/

#ifndef SCHEDULE_ORIG_GPU_H_INCLUDED
#define SCHEDULE_ORIG_GPU_H_INCLUDED


#include "rob.h"
#include "schedule.h"
#include "uop.h"
#include "frontend.h"


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Scheduler class for GPU simulation
///////////////////////////////////////////////////////////////////////////////////////////////
class schedule_smc_c : public schedule_c 
{
  public:
    /**
     *  \brief Constructor for the gpu scheluder class.
     *  \param m_core_id - Core identifier number
     *  \param gpu_allocq - Pointer to be updated with the alloc stage queues
     *  \param gpu_m_rob - Pointer to the Reorder buffer  
     *  \param m_exec - Pointer to m_execution unit
     *  \param m_unit_type - Parameter used to identify knob width
     *  \param m_frontend - Pointer to front end queue
     *  \param simBase - simulation base class pointer
     *  \return void 
     */
    schedule_smc_c(int m_core_id, pqueue_c<gpu_allocq_entry_s>** gpu_allocq, 
        smc_rob_c* gpu_m_rob, exec_c* m_exec, Unit_Type m_unit_type,
        frontend_c* m_frontend, macsim_c* simBase); 

    /*! \fn void ~schedule_smc_c()
     *  \brief Destructor for the gpu scheduler
     *  \return void 
     */
    ~schedule_smc_c(void);

    /*! \fn void run_a_cycle()
     *  \brief Function to perform the activities of a cycle for the scheduler.
     *  \return void 
     */
    void run_a_cycle();

  private:
    /*! \fn void advance(int ALLOCQ_index)
     *  \brief Function to move the allocation queue ahead.
     *  \param ALLOCQ_index - Aloocation queue number to be moved ahead
     *  \return void 
     */
    void advance(int ALLOCQ_index);

    /*! \fn bool check_srcs_smc(int thread_id, int entry)
     *  \brief Function to check if the sources of an uop are ready
     *  \param thread_id - Thread id
     *  \param entry - ROB entry of the uop
     *  \return bool - True if no dependency found
     */
    bool check_srcs_smc(int thread_id, int entry);

    /*! \fn bool uop_schedule_smc(int thread_id, int entry, SCHED_FAIL_TYPE* sched_fail_reason)
     *  \brief Function to schedule uops 
     *  \param thread_id - Thread id
     *  \param entry - ROB entry of the uop
     *  \param sched_fail_reason - Reason of uop schedule failure
     *  \return bool - True on success in scheduling
     */
    bool uop_schedule_smc(int thread_id, int entry, SCHED_FAIL_TYPE* sched_fail_reason);

  private:
    static const int MAX_GPU_SCHED_SIZE = 128; /**< max sched table size */

    smc_rob_c* m_gpu_rob;  /**< gpu rob */
    pqueue_c<gpu_allocq_entry_s>** m_gpu_allocq; /**< gpu allocation queue */ 
    int knob_num_threads; /**< number of maximum thread per core */
    int m_schedule_modulo; /**< modulo to schedule next thread */
    int* m_schlist_entry; /**< schedule list entry */
    int* m_schlist_tid; /**< schedule list thread id */
    int m_first_schlist; /**< current index in schedule list */
    int m_last_schlist; /**< last index in schedule list */
    int m_schlist_size; /**< schedule list size */

    macsim_c* m_simBase; /**< macsim_c base class for simulation globals */
   
};
#endif // SCHEDULE_ORIG_H_INCLUDED

