/**********************************************************************************************
 * File         : retire.h
 * Author       : Hyesoon Kim 
 * Date         : 3/17/2008
 * CVS          : $Id: retire.h 867 2009-11-05 02:28:12Z kacear $:
 * Description  : retirement logic 
                  origial author: Jared W.Stark  imported from 
 *********************************************************************************************/

#ifndef RETIRE_H_INCLUDED
#define RETIRE_H_INCLUDED


#include <inttypes.h>
#include <queue>
#include <unordered_map>

#include "retire_interface.h"

#include "global_types.h"

#include "debug_macros.h"


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief retirement stage
///
/// Retire (Commit) stage in the pipeline
///////////////////////////////////////////////////////////////////////////////////////////////
class retire_c
{
  public:
    /**
     * Constructor
     */
    retire_c(RETIRE_INTERFACE_PARAMS(), macsim_c* simBase);

    /**
     * Destructor
     */
    ~retire_c();

    /**
     * Tick a cycle
     */
    void run_a_cycle();

    /**
     * Enable retirement stage
     */
    void start();

    /**
     * Disable retirement stage
     */
    void stop();

    /**
     * Check retirement stage is running
     */
    bool is_running();

    /**
     * Get number of retired instruction with thread_id
     */
    inline Counter get_instrs_retired(int thread_id) {
      return m_insts_retired[thread_id]; 
    }

    /**
     * Get number of retired uops with thread_id
     */
    Counter get_uops_retired(int thread_id);

    /**
     * Get number of total retired instructions
     */
    Counter get_total_insts_retired(); 

    /**
     * When a new thread has been scheduled, reset data
     */
    void allocate_retire_data(int);

    /**
     * Deallocate per thread data 
     */
    void deallocate_retire_data(int);

    int get_periodic_inst_count(void)
    {
      int result = m_period_inst_count;
      m_period_inst_count = 0;

      return result;
    }

  private:
    /**
     * Private constructor. Do not implement
     */
    retire_c (const retire_c& rhs);

    /**
     * Overridden operator =
     */
    retire_c& operator=(const retire_c& rhs);

    /**
     * Update stats when an application is done
     */
    void update_stats(process_s*);

    /**
     * Repeat (terminated) application
     */
    void repeat_traces(process_s*);

  private:
    RETIRE_INTERFACE_DECL();

    bool                        m_retire_running; /**< retire enabled */
    Counter                     m_total_insts_retired; /**< total retired instructions */ 
    Counter                     m_cur_core_cycle; /**< current core cycle */ 
    uns16                       m_knob_width; /**< pipeline width */
    bool                        m_knob_ptx_sim; /**< gpu simulation */
    unordered_map<int, Counter> m_insts_retired; /**< number of retired inst. per thread */
    unordered_map<int, Counter> m_uops_retired; /**< number of retired uop per thread */

    int m_period_inst_count;

    macsim_c* m_simBase;         /**< macsim_c base class for simulation globals */

};

#endif // RETIRE_H_INCLUDED
