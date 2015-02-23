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
 * File         : network_ring.cc
 * Author       : HPArch Research Group
 * Date         : 2/18/2013
 * SVN          : $Id: dram.h 867 2009-11-05 02:28:12Z kacear $:
 * Description  : Ring network
 *********************************************************************************************/


#include <fstream>
#include <cmath>

#include "network_ring.h"
#include "all_knobs.h"
#include "all_stats.h"
#include "utils.h"
#include "debug_macros.h"
#include "assert_macros.h"
#include "memreq_info.h"


#define DEBUG(args...) _DEBUG(*m_simBase->m_knobs->KNOB_DEBUG_NOC, ## args)


extern int g_total_packet;
extern int g_total_cpu_packet;
extern int g_total_gpu_packet;


/////////////////////////////////////////////////////////////////////////////////////////


network_ring_c::network_ring_c(macsim_c* simBase)
  : network_c(simBase)
{
}


network_ring_c::~network_ring_c()
{
}


// run one cycle for all routers
void network_ring_c::run_a_cycle(bool pll_lock)
{
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
    router_c* new_router = new router_ring_c(m_simBase, type, m_num_router++); \
    m_router.push_back(new_router); \
  }


void network_ring_c::init(int num_cpu, int num_gpu, int num_l3, int num_mc)
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
  }


  // connect routers
  string mapping;
  stringstream sstr;
  for (int ii = 0; ii < m_num_router; ++ii) {
    sstr << ii << ",";
  }
  sstr >> mapping;
  mapping = mapping.substr(0, mapping.length()-1);

  int search_pos = 0;
  int pos;
  vector<int> map_func;
  while (1) {
    pos = mapping.find(',', search_pos);
    if (pos == string::npos) {
      string sub = mapping.substr(search_pos);
      map_func.push_back(atoi(sub.c_str()));
      break;
    }

    string sub = mapping.substr(search_pos, pos - search_pos);
    map_func.push_back(atoi(sub.c_str()));
    search_pos = pos + 1;
  }

  assert(m_num_router == map_func.size());

  for (int ii = 0; ii < m_num_router; ++ii) {
    m_router[map_func[ii]]->set_link(LEFT,  m_router[map_func[(ii-1+m_num_router)%m_num_router]]);
    m_router[map_func[ii]]->set_link(RIGHT, m_router[map_func[(ii+1)%m_num_router]]);
    m_router[map_func[ii]]->set_id(ii);
  }

  m_router[0]->print_link_info();
}


void network_ring_c::print(void)
{
  ofstream out("router.out");
  out << "total_packet:" << m_total_packet << "\n";
  for (int ii = 0; ii < m_num_router; ++ii) {
    m_router[ii]->print(out);
  }
}


/////////////////////////////////////////////////////////////////////////////////////////


// constructor
router_ring_c::router_ring_c(macsim_c* simBase, int type, int id)
  : router_c(simBase, type, id, 3)
{
  // configurations
  m_topology = "ring";
  assert(*KNOB(KNOB_NOC_DIMENSION) == 1);
  assert(m_num_vc >= 2);
}


// destructor
router_ring_c::~router_ring_c()
{
}


void router_ring_c::stage_rc(void)
{
  for (int port = 0; port < m_num_port; ++port) {
    for (int vc = 0; vc < m_num_vc; ++vc) {
      if (m_route_fixed[port][vc] != -1 || m_input_buffer[port][vc].empty())
        continue;

      flit_c* flit = m_input_buffer[port][vc].front();
      if (flit->m_head == true && flit->m_state == IB && flit->m_rdy_cycle <= m_cycle) {
        fill_n(m_route[port][vc][0], m_num_port, false);
        // ------------------------------
        // shortest distance routing
        // ------------------------------
        if (flit->m_dst == m_id) {
          m_route[port][vc][0][LOCAL] = true;
        }
        else if (flit->m_dir != -1) {
          m_route[port][vc][0][flit->m_dir] = true;
        }
        else {
          // ------------------------------
          // Ring
          // ------------------------------
          int left;
          int right;
          if (m_id > flit->m_dst) {
            left = m_id - flit->m_dst;
            right = flit->m_dst - m_id + m_total_router;
          }
          else {
            left = m_id - flit->m_dst + m_total_router;
            right = flit->m_dst - m_id;
          }

          if (left < right) {
            m_route[port][vc][0][LEFT] = true;
            flit->m_dir = LEFT;
          }
          else {
            m_route[port][vc][0][RIGHT] = true;
            flit->m_dir = RIGHT;
          }
        }

        flit->m_state = RC;
        DEBUG("cycle:%-10lld node:%d [RC] req_id:%d flit_id:%d src:%d dst:%d ip:%d vc:%d\n",
            m_cycle, m_id, flit->m_req->m_id, flit->m_id, flit->m_req->m_msg_src,
            flit->m_req->m_msg_dst, port, vc);
      }
    }
  }
}


void router_ring_c::stage_vca_pick_winner(int oport, int ovc, int& iport, int& ivc)
{
  int rc_index = 0;

  // Oldest-first arbitration
  if (m_arbitration_policy == OLDEST_FIRST) {
    // search all input ports for the winner
    Counter oldest_timestamp = ULLONG_MAX;
    iport = -1;
    for (int ii = 0; ii < m_num_port; ++ii) {
      if (ii == oport)
        continue;

      for (int jj = 0; jj < m_num_vc; ++jj) {
        if (m_input_buffer[ii][jj].empty() || !m_route[ii][jj][rc_index][oport])
          continue;

        flit_c* flit = m_input_buffer[ii][jj].front();


        // DUATO's deadlock prevention protocol
        if (ovc < m_num_vc - 2) {
          // do nothing
        }
        else if (ovc == m_num_vc - 2) {
          if (m_id > flit->m_dst)
            continue;
        }
        else if (ovc == m_num_vc - 1) {
          if (m_id < flit->m_dst)
            continue;
        }


        // header && RC stage && oldest
        if (flit->m_head == true && flit->m_state == RC && flit->m_timestamp < oldest_timestamp) {
          oldest_timestamp = flit->m_timestamp;
          iport = ii;
          ivc   = jj;
        }
      }
    }
  }
#if 0
  // round-robin
  else if (1) {
    // search all input ports for the winner
    Counter oldest_timestamp = ULLONG_MAX;
    iport = -1;
    for (int rr = 0; rr < m_num_port*m_num_vc; ++rr) {
      int ii = (CYCLE + rr) % m_num_port;
      int jj = ((CYCLE + rr) % (m_num_port*m_num_vc)) / m_num_port;
      
      if (ii == oport)
        continue;

      if (m_input_buffer[ii][jj].empty() || !m_route[ii][jj][rc_index][oport])
        continue;

      flit_c* flit = m_input_buffer[ii][jj].front();

      // header && RC stage && oldest
      if (flit->m_head == true && flit->m_state == RC && flit->m_timestamp < oldest_timestamp) {
        oldest_timestamp = flit->m_timestamp;
        iport = ii;
        ivc   = jj;
      }
    }
  }
#endif
  else
    assert(0);
}


void router_ring_c::print_link_info(void)
{
  report("RING topology");
  cout << m_id << " <-> ";
  router_c* current = m_link[RIGHT];
  do {
    cout << current->get_id() << " <-> ";
    current = current->get_router(RIGHT);//current->m_link[RIGHT];
  } while (current != this);
  cout << "\n";
}


/////////////////////////////////////////////////////////////////////////////////////////

