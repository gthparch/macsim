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


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief noc packet information (will be deprecated)
///////////////////////////////////////////////////////////////////////////////////////////////
typedef struct noc_entry_s {
  int     m_src; /**< packet src */
  int     m_dst; /**< packet dst */
  int     m_msg; /**< msg type */
  Counter m_rdy; /**< ready cycle */
  mem_req_s* m_req; /**< pointer to the request */
} noc_entry_s;


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief NoC interface class (will be deprecated)
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
    memory_c* m_memory; /**< pointer to the memory class */
    pool_c<noc_entry_s>* m_pool; /**< entry pool */

    // uplink
    list<noc_entry_s*>* m_cpu_entry_up; /**< cpu request list to upper level */
    list<noc_entry_s*>* m_cpu_entry_down; /**< cpu request list to lower level */
    
    list<noc_entry_s*>* m_gpu_entry_up; /**< gpu request list to upper level */
    list<noc_entry_s*>* m_gpu_entry_down; /**< cpu request list to lower level */

    macsim_c* m_simBase; /**< macsim_c base class for simulation globals */
};
#endif
