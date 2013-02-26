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
 * File         : exec.h
 * Author       : HPArch
 * Date         : 1/26/2008
 * SVN          : $Id: exec.h,v 1.4 2008-09-10 02:19:37 kacear Exp $:
 * Description  : execution unit 
                  origial author: Jared W.Stark  imported from 
 *********************************************************************************************/

#ifndef EXEC_H_INCLUDED
#define EXEC_H_INCLUDED


#include "macsim.h"
#include "global_defs.h"
#include "global_types.h"
#include "uop.h"


#define EXEC_INTERFACE_PARAMS() \
    int m_core_id, \
    pqueue_c<int>** m_q_iaq, \
    pqueue_c<gpu_allocq_entry_s>** m_gpu_q_iaq, \
    rob_c* m_rob, \
    smc_rob_c* m_gpu_rob, \
    bp_data_c* m_bp_data, \
    frontend_c* m_frontend, \
    Unit_Type m_unit_type \
// end macro


#define EXEC_INTERFACE_DECL() \
    int m_core_id; /**< core id */ \
    pqueue_c<int>** m_q_iaq; /**< ia queue */ \
    pqueue_c<gpu_allocq_entry_s>** m_gpu_q_iaq; /**< gpu ia queue */ \
    rob_c* m_rob; /**< reorder buffer */ \
    smc_rob_c* m_gpu_rob; /**< gpu reorder buffer */ \
    bp_data_c* m_bp_data; /**< branch prediction data */ \
    frontend_c* m_frontend; /**< frontend pointer */ \
    Unit_Type m_unit_type; /**< unit type */ \
// end macro


#define EXEC_INTERFACE_ARGS() \
    m_core_id, \
    m_q_iaq, \
    m_gpu_q_iaq, \
    m_rob, \
    m_gpu_rob, \
    m_bp_data, \
    m_frontend, \
    m_unit_type \
// end macro


#define EXEC_INTERFACE_INIT() \
    m_core_id ( m_core_id ), \
    m_q_iaq ( m_q_iaq ), \
    m_gpu_q_iaq ( m_gpu_q_iaq ), \
    m_rob ( m_rob ), \
    m_gpu_rob ( m_gpu_rob ), \
    m_bp_data ( m_bp_data ), \
    m_frontend ( m_frontend ), \
    m_unit_type ( m_unit_type ) \
// end macro


#define EXEC_INTERFACE_CAST() \
    static_cast<void>(m_core_id); \
    static_cast<void>(m_q_iaq); \
    static_cast<void>(m_gpu_q_iaq); \
    static_cast<void>(m_rob); \
    static_cast<void>(m_gpu_rob); \
    static_cast<void>(m_bp_data); \
    static_cast<void>(m_frontend); \
    static_cast<void>(m_unit_type); \
// end macro


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Execution stage
///////////////////////////////////////////////////////////////////////////////////////////////
class exec_c
{
  public:
    /**
     * \brief Constructor to exec_c class    
     * \param simBase - Pointer to base simulation class for perf/stat counters
     */
    exec_c(EXEC_INTERFACE_PARAMS(), macsim_c* simBase);

    /*! \fn ~exec_c()
     *  \brief Destructor to exec_c class
     *  \return void
     */
    ~exec_c();

    /*! \fn void clear_ports()
     *  \brief Function to mark all ports as available
     *  \return void
     */
    void clear_ports();

    /*! \fn bool port_available(int iaq)
     *  \brief Function to check if a port is available
     *  \param iaq
     *  \return void
     */
    bool port_available(int iaq);

    /*! \fn bool exec(int thread_id, int entry, uop_c* cur_uop)
     *  \brief Function to run the exec stage
     *  \param thread_id - Thread id
     *  \param entry - Rob entry
     *  \param cur_uop - Pointer to uop
     *  \return bool - True if successful
     */
    bool exec(int thread_id, int entry, uop_c* cur_uop);

    /*! \fn int get_latency(Uop_Type)
     *  \brief Function to get latency for an uop type
     *  \param type - uop type
     *  \return int - Latency for the uop
     */
    int get_latency(Uop_Type type);

    /**
     * run a cycle
     */
    void run_a_cycle(void);
    
    /**
     * Return bank busy array
     */
    bool** get_bank_busy_array(void) { return &m_bank_busy; }

  private:
    /**
     *  \brief Use a execution port
     *  \param thread_id - Thread id
     *  \param entry - ROB id
     *  \return void
     */
    void use_port(int thread_id, int entry);

    /*! \fn void br_exec(uop_c *)
     *  \brief Function to execute branch uop
     *  \param uop - Pointer to uop
     *  \return void
     */
    void br_exec(uop_c *uop);

    /**
     * update memory related stats
     */
    void update_memory_stats(uop_c* uop);

  private:
    /**
     * Private constructor.
     */
    exec_c (const exec_c& rhs); // do not implement

    /**
     * Overridden operator =
     */
    exec_c& operator=(const exec_c& rhs);

    EXEC_INTERFACE_DECL(); /**< declaration macro */

    uns16   m_int_sched_rate; /**< int schedule rate */
    uns16   m_mem_sched_rate; /**< memory schedule rate */
    uns16   m_fp_sched_rate; /**< fp schedule rate */
    uns8    m_dcache_cycles; /**< L1 cache latency */
    bool    m_ptx_sim; /**< gpu simulation */
    int     m_latency[NUM_UOP_TYPES]; /**< latency map */
    Counter m_cur_core_cycle;  /**< current core cycle */
    int     m_max_port[max_ALLOCQ]; /**< maximum port */ 
    int     m_port_used[max_ALLOCQ]; /**< number of currently used port */
    bool*    m_bank_busy; /**< indicate dcache bank busy */
    
    macsim_c* m_simBase;         /**< macsim_c base class for simulation globals */
};

#endif // EXEC_H_INCLUDED

