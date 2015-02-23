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
 * File         : network_simple.cc
 * Author       : HPArch Research Group
 * Date         : 4/28/2013
 * SVN          : $Id: dram.h 867 2009-11-05 02:28:12Z kacear $:
 * Description  : Simple single-hop network
 *********************************************************************************************/


#include "network_simple.h"
#include "memreq_info.h"
#include "debug_macros.h"


#define DEBUG(args...) _DEBUG(*m_simBase->m_knobs->KNOB_DEBUG_NOC, ## args)


extern int g_total_packet;
extern int g_total_cpu_packet;
extern int g_total_gpu_packet;


/////////////////////////////////////////////////////////////////////////////////////////


network_simple_c::network_simple_c(macsim_c* simBase)
  : network_c(simBase)
{
}


network_simple_c::~network_simple_c()
{
}


void network_simple_c::run_a_cycle(bool pll_lock)
{
  // reset
  for (int ii = 0; ii < m_num_router; ++ii)
    m_router[ii]->reset();

  // randomized tick function
  int index = CYCLE % m_num_router;
  for (int ii = index; ii < index + m_num_router; ++ii) {
    m_router[ii % m_num_router]->run_a_cycle(pll_lock);
  }
  ++m_cycle;
}



#define CREATE_ROUTER(index, type, level, offset) \
  for (int ii = 0; ii < index; ++ii) { \
    m_router_map[level*1000+ii+offset] = m_num_router; \
    router_c* new_router = new router_simple_c(m_simBase, type, m_num_router++); \
    m_router.push_back(new_router); \
  }


void network_simple_c::init(int num_cpu, int num_gpu, int num_l3, int num_mc)
{
  m_num_router = 0;
  m_num_cpu = num_cpu;
  m_num_gpu = num_gpu;
  m_num_l3  = num_l3;
  m_num_mc  = num_mc;

  CREATE_ROUTER(m_num_cpu, CPU_ROUTER, MEM_L2, 0);
  CREATE_ROUTER(m_num_gpu, GPU_ROUTER, MEM_L2, m_num_cpu);
  CREATE_ROUTER(m_num_l3,  L3_ROUTER, MEM_L3, 0);
  CREATE_ROUTER(m_num_mc,  MC_ROUTER, MEM_MC, 0);
  
  report("TOTAL_ROUTER:" << m_num_router << " CPU:" << m_num_cpu << " GPU:" << m_num_gpu
      << " L3:" << m_num_l3 << " MC:" << m_num_mc);

  for (int ii = 0; ii < m_num_router; ++ii) {
    m_router[ii]->init(m_num_router, &g_total_packet, m_flit_pool, m_credit_pool);
    m_router[ii]->set_router_map(m_router);
  }

  // no need to connect routers
}


void network_simple_c::print(void)
{
}


/////////////////////////////////////////////////////////////////////////////////////////


router_simple_c::router_simple_c(macsim_c* simBase, int type, int id)
  : router_c(simBase, type, id, 1)
{
  m_topology = "simple_noc";
  m_max_num_injection_from_src = 2;
  m_max_num_accept_in_dst = 2;
}


router_simple_c::~router_simple_c()
{
}


void router_simple_c::reset(void)
{
  m_packet_inserted = 0;
}


void router_simple_c::run_a_cycle(bool pll_lock)
{
  process();
}


// destination should be global
void router_simple_c::process(void)
{
  int num_count = 0;

  for (auto I = m_injection_buffer->begin(), E = m_injection_buffer->end(); I != E; ) {
    if (num_count >= m_max_num_injection_from_src)
      break; 
    
    auto I_tmp = I++;
    int dst_id = (*I_tmp)->m_msg_dst;
    int* packet_inserted = m_router_map[dst_id]->get_num_packet_inserted();
    if (*packet_inserted < m_max_num_accept_in_dst) {
      // insert a packet to the destination
      m_router_map[dst_id]->insert_packet((*I_tmp));

      // increase injection count in dst
      ++(*packet_inserted);
      
      // remove from the current buffer
      m_injection_buffer->erase(I_tmp);

      // increment current injection count from src
      ++num_count;
    }
  }
}

int* router_simple_c::get_num_packet_inserted(void)
{
  return &m_packet_inserted;
}


// dummy functions
// wrong oop design - should be removed
void router_simple_c::stage_rc(void)
{
}

void router_simple_c::stage_vca_pick_winner(int oport, int ovc, int& iport, int& ivc)
{
}

void router_simple_c::print_link_info(void)
{
}


