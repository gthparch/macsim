/**********************************************************************************************
 * File         : smc_rob.h
 * Author       : Nagesh BL 
 * Date         : 03/07/2010 
 * CVS          : $Id: rob.h,v 1.2 2008-09-10 01:31:00 kacear Exp $:
 * Description  : rob structure 
                  origial author: Jared W. Stark  imported from 
 *********************************************************************************************/


#ifndef GPU_ROB_H_INCLUDED
#define GPU_ROB_H_INCLUDED


#include <cassert>
#include <unordered_map>
#include <vector>

#include "global_types.h"
#include "rob.h"


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Reorder buffer class for GPU simulation
///////////////////////////////////////////////////////////////////////////////////////////////
class smc_rob_c 
{
  public:
    /**
     *  \brief Create a Reorder Buffer data structure.
     *  \param type - Core type indicator
     *  \param core_id - core id
     *  \return void. 
     */
    smc_rob_c(Unit_Type type, int core_id, macsim_c* simBase);

    /*! \fn void ~smc_rob_c()
     *  \brief Destructor.
     *  \return void. 
     */
    ~smc_rob_c(); 

    /*! \fn void get_thread_rob(int thread_id)
     *  \brief Function to return the corresponding ROB for the thread id.
     *  \param thread_id Thread identifier
     *  \return rob_c* ROB pointer. 
     */
    rob_c* get_thread_rob(int thread_id); 

    /**
     *  \brief Function to return the corresponding ROB index for the thread id.
     *  \param thread_id Thread identifier
     *  \return int ROB index. 
     */
    int get_thread_rob_id(int thread_id);

    /*! \fn void int reserve_rob(int thread_id)
     *  \brief Function to return the index of the ROB reserved.
     *  \param thread_id Thread identifier
     *  \return int ROB index. 
     */
    int reserve_rob(int thread_id);

    /**
     *  \brief Function to free the ROB entry for the thread passed as parameter
     *  \param thread_id Thread identifier
     *  \return void. 
     */
    void free_rob(int thread_id);

    /*! \fn list<uop_c *>* get_n_uops_in_ready_order(int n, 
                                                     Counter cur_core_cycle)
     *  \brief Function to find the n most deserving uops to be retired.
     *  \param n Count of Uopsmc_rob.htrace
     *  \param cur_core_cycle Current core cycle
     *  \return std::list<uop_c *>* . 
     */
    vector<uop_c *>* get_n_uops_in_ready_order(int n, Counter cur_core_cycle);

  private:
    int             m_knob_num_threads; /**< max threads per core */
    rob_c **        m_thread_robs; /**< reorder buffer per thread */
    list<int>       m_free_list;  /**< thread rob pool */ 
    vector<uop_c *> m_uop_list;   /**< retireable uop list */ 
    int             m_core_id;    /**< core id */
    Unit_Type       m_unit_type;  /**< core type */
    
    unordered_map<int, int> m_thread_to_rob_map; /**< thread id to rob mapping */

    macsim_c* m_simBase;         /**< macsim_c base class for simulation globals */
   
};
#endif // GPU_ROB_H_INCLUDED 
