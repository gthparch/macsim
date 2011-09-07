/**********************************************************************************************
 * File         : noc.h 
 * Author       : Jaekyu Lee
 * Date         : 3/4/2011
 * SVN          : $Id: dram.h 867 2009-11-05 02:28:12Z kacear $:
 * Description  : Interface to interconnection network
 *********************************************************************************************/


#ifndef NOC_H
#define NOC_H


#include <list>

#include "global_defs.h"
#include "global_types.h"


typedef struct noc_entry_s {
  int     m_src;
  int     m_dst;
  int     m_msg;
  Counter m_rdy;
  mem_req_s* m_req;
} noc_entry_s;


///////////////////////////////////////////////////////////////////////////////////////////////
/// NoC interface class
///////////////////////////////////////////////////////////////////////////////////////////////
class noc_c
{
  public:
    /**
     * NoC interface class constructor
     */
    noc_c(macsim_c* simBase);

    /**
     * NoC interface class destructor
     */
    ~noc_c();

    /**
     * Insert a new req/msg to the NoC
     */
    bool insert(int src, int dest, int msg, mem_req_s* req);

    /**
     * Tick a cycle
     */
    void run_a_cycle();

  private:
    memory_c* m_memory;
    pool_c<noc_entry_s>* m_pool;

    // uplink
    list<noc_entry_s*>* m_cpu_entry_up;
    list<noc_entry_s*>* m_cpu_entry_down;
    
    list<noc_entry_s*>* m_gpu_entry_up;
    list<noc_entry_s*>* m_gpu_entry_down;

    macsim_c* m_simBase;         /**< macsim_c base class for simulation globals */

};
#endif
