/**********************************************************************************************
 * File         : exec.h
 * Author       : Hyesoon Kim
 * Date         : 1/26/2008
 * SVN          : $Id: exec.h,v 1.4 2008-09-10 02:19:37 kacear Exp $:
 * Description  : execution unit 
                  origial author: Jared W.Stark  imported from 
 *********************************************************************************************/

#ifndef EXEC_H_INCLUDED
#define EXEC_H_INCLUDED


#include "exec_interface.h"
#include "macsim.h"
#include "global_defs.h"
#include "global_types.h"
#include "uop.h"


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Execution stage
///////////////////////////////////////////////////////////////////////////////////////////////
class exec_c
{
  public:
    /*! \fn exec_c(EXEC_INTERFACE_PARAMS())
     *  \brief Constructor to exec_c class    
     *  \param simBase - Pointer to base simulation class for perf/stat counters
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

    EXEC_INTERFACE_DECL();

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

