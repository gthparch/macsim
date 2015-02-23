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
 * File         : network_mesh.cc 
 * Author       : HPArch Research Group
 * Date         : 2/18/2013
 * SVN          : $Id: dram.h 867 2009-11-05 02:28:12Z kacear $:
 * Description  : Mesh network
 *********************************************************************************************/


#include <fstream>
#include <cmath>

#include "network_mesh.h"
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


network_mesh_c::network_mesh_c(macsim_c* simBase)
  : network_c(simBase)
{
}


network_mesh_c::~network_mesh_c()
{
}


// run one cycle for all routers
void network_mesh_c::run_a_cycle(bool pll_lock)
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
    router_c* new_router = new router_mesh_c(m_simBase, type, m_num_router++); \
    m_router.push_back(new_router); \
  }


void network_mesh_c::init(int num_cpu, int num_gpu, int num_l3, int num_mc)
{
  m_num_router = 0;
  m_num_cpu = num_cpu;
  m_num_gpu = num_gpu;
  m_num_l3  = num_l3;
  m_num_mc  = num_mc;

  CREATE_ROUTER(m_num_cpu, CPU_ROUTER, MEM_L2, 0);
  CREATE_ROUTER(m_num_l3,  L3_ROUTER, MEM_L3, 0);
  CREATE_ROUTER(m_num_mc,  MC_ROUTER, MEM_MC, 0);
  CREATE_ROUTER(m_num_gpu, GPU_ROUTER, MEM_L2, m_num_cpu);
  
  int width = sqrt(m_num_router);
  if ((width * width) != m_num_router) {
    for (; m_num_router < (width+1)*(width+1) ; ++m_num_router) {
      report("router:" << m_num_router << " type:dummy created");

      router_c* new_router = new router_mesh_c(m_simBase, 0, m_num_router);
      m_router.push_back(new_router);
    }
    ++width;
  }
  
  report("TOTAL_ROUTER:" << m_num_router << " CPU:" << m_num_cpu << " GPU:" << m_num_gpu
      << " L3:" << m_num_l3 << " MC:" << m_num_mc);

  for (int ii = 0; ii < m_num_router; ++ii) {
    m_router[ii]->init(m_num_router, &g_total_packet, m_flit_pool, m_credit_pool);
  }


  // connect routers
  int* mapping = new int[m_num_router];
  stringstream sstr;
  int num_large_core = *KNOB(KNOB_NUM_SIM_LARGE_CORES);
  int num_small_core = *KNOB(KNOB_NUM_SIM_SMALL_CORES);

  int count = 0;

  /*
  for (int ii = 0; ii < num_large_core; ++ii) {
    mapping[count++] = ii;
  } 

  if (num_large_core == 0) {
    for (int ii = num_large_core; ii < num_large_core+num_small_core; ++ii) {
      mapping[count++] = ii;
    }
  }

  int start_index = num_large_core + num_small_core;
  int end_index = start_index + *KNOB(KNOB_NUM_L3); 
  for (int ii = start_index; ii < end_index; ++ii) {
    mapping[count++] = ii;
  }

  start_index = end_index;
  end_index += *KNOB(KNOB_DRAM_NUM_MC);
  for (int ii = start_index; ii < end_index; ++ii) {
    mapping[count++] = ii;
  }

  if (num_large_core != 0) {
    for (int ii = num_large_core; ii < num_large_core+num_small_core; ++ii) {
      mapping[count++] = ii;
    }
  }

  for (int ii = end_index; ii < m_num_router; ++ii) {
    mapping[count++] = ii;
  }
  */
  for (int ii = 0; ii < m_num_router; ++ii)
    mapping[ii] = ii;

  for (int ii = 0; ii < m_num_router; ++ii) {
    if (ii / width > 0)  // north link
      m_router[mapping[ii]]->set_link(UP, m_router[mapping[ii-width]]);

    if (ii / width < (width - 1))  // south link
      m_router[mapping[ii]]->set_link(DOWN, m_router[mapping[ii+width]]);

    if (ii % width != 0)  // west link
      m_router[mapping[ii]]->set_link(LEFT, m_router[mapping[ii-1]]);

    if (ii % width != (width - 1))  // east link
      m_router[mapping[ii]]->set_link(RIGHT, m_router[mapping[ii+1]]);

//    m_router[mapping[ii]]->set_id(ii);
  }

  m_router[0]->print_link_info();
}


void network_mesh_c::print(void)
{
  ofstream out("router.out");
  out << "total_packet:" << m_total_packet << "\n";
  for (int ii = 0; ii < m_num_router; ++ii) {
    m_router[ii]->print(out);
  }
}


/////////////////////////////////////////////////////////////////////////////////////////


// constructor
router_mesh_c::router_mesh_c(macsim_c* simBase, int type, int id)
  : router_c(simBase, type, id, 5)
{
  // configurations
  m_topology = "mesh";
  assert(*KNOB(KNOB_NOC_DIMENSION) == 2);
  assert(m_num_vc >= 1);
}


// destructor
router_mesh_c::~router_mesh_c()
{
}


void router_mesh_c::stage_rc(void)
{
  for (int port = 0; port < m_num_port; ++port) {
    for (int vc = 0; vc < m_num_vc; ++vc) {
      if (m_input_buffer[port][vc].empty())
        continue;

      flit_c* flit = m_input_buffer[port][vc].front();
      if (flit->m_head == true && flit->m_state == IB && flit->m_rdy_cycle <= m_cycle) {
        fill_n(m_route[port][vc][0], m_num_port, false);
        fill_n(m_route[port][vc][1], m_num_port, false);
        // local
        if (flit->m_dst == m_id) {
          m_route[port][vc][0][LOCAL] = true;
          m_route[port][vc][1][LOCAL] = true;
        }
        else {
          int width = sqrt(m_total_router);
          int x_src = m_id % width;
          int y_src = m_id / width;
          int x_dst = flit->m_dst % width;
          int y_dst = flit->m_dst / width;

          // adaptive routing
          if (x_src > x_dst)
            m_route[port][vc][0][LEFT] = true;
          else if (x_src < x_dst)
            m_route[port][vc][0][RIGHT] = true;

          if (y_src > y_dst) 
            m_route[port][vc][0][UP] = true;
          else if (y_src < y_dst)
            m_route[port][vc][0][DOWN] = true;

          // escape routing
          if (x_src > x_dst)
            m_route[port][vc][1][LEFT] = true;
          else if (x_src < x_dst)
            m_route[port][vc][1][RIGHT] = true;
          else if (y_src > y_dst) 
            m_route[port][vc][1][UP] = true;
          else if (y_src < y_dst)
            m_route[port][vc][1][DOWN] = true;
        }
        flit->m_state = RC;
        DEBUG("cycle:%-10lld node:%d [RC] req_id:%d flit_id:%d src:%d dst:%d ip:%d vc:%d\n",
            m_cycle, m_id, flit->m_req->m_id, flit->m_id, flit->m_req->m_msg_src, 
            flit->m_req->m_msg_dst, port, vc);
      }
    }
  }
}


void router_mesh_c::stage_vca_pick_winner(int oport, int ovc, int& iport, int& ivc)
{
  int rc_index = ovc / (m_num_vc - 1);

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


void router_mesh_c::print_link_info(void)
{
  report("MESH topology");
  cout << m_id << " <-> ";
  int dir = RIGHT; 
  router_c* current = m_link[dir];
  while (1) {
    cout << current->get_id() << " <-> ";
    if (current->get_router(dir) == NULL) {
      cout << "\n";
      dir = m_opposite_dir[dir];
      if (current->get_router(DOWN) == NULL) {
        break;
      }
      current = current->get_router(DOWN);
    }
    else {
      current = current->get_router(dir);
    }
  }
}


/////////////////////////////////////////////////////////////////////////////////////////

