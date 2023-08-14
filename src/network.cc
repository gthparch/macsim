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
 * File         : network.cc
 * Author       : HPArch Research Group
 * Date         : 2/18/2013
 * SVN          : $Id: dram.h 867 2009-11-05 02:28:12Z kacear $:
 * Description  : Network interface
 *********************************************************************************************/

#include "network.h"
#include "network_ring.h"
#include "network_mesh.h"
#include "network_simple.h"

#include "memreq_info.h"
#include "debug_macros.h"
#include "all_knobs.h"
#include "all_stats.h"
#include "assert_macros.h"

#define DEBUG(args...) _DEBUG(*m_simBase->m_knobs->KNOB_DEBUG_NOC, ##args)

/////////////////////////////////////////////////////////////////////////////////////////

int g_total_packet = 0;
int g_total_cpu_packet = 0;
int g_total_gpu_packet = 0;

/////////////////////////////////////////////////////////////////////////////////////////

// flit_c constructor
flit_c::flit_c() {
  init();
}

// flit_c desctructor
flit_c::~flit_c() {
}

// flit_c initialization
void flit_c::init(void) {
  m_src = -1;
  m_dst = -1;
  m_head = false;
  m_tail = false;
  m_req = NULL;
  m_state = INIT;
  m_rdy_cycle = ULLONG_MAX;
  m_vc_batch = false;
  m_sw_batch = false;
  m_id = 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

credit_c::credit_c() {
}

credit_c::~credit_c() {
}

/////////////////////////////////////////////////////////////////////////////////////////

// register base branch predictor
network_c* default_network(macsim_c* m_simBase) {
  string policy = KNOB(KNOB_NOC_TOPOLOGY)->getValue();
  network_c* new_network;
  if (policy == "ring")
    new_network = new network_ring_c(m_simBase);
  else if (policy == "mesh")
    new_network = new network_mesh_c(m_simBase);
  else if (policy == "simple_noc")
    new_network = new network_simple_c(m_simBase);
  else
    assert(0);

  return new_network;
}

/////////////////////////////////////////////////////////////////////////////////////////
network_c::network_c(macsim_c* simBase) {
  m_simBase = simBase;

  m_total_packet = 0;
  m_num_router = 0;

  m_flit_pool = new pool_c<flit_c>(100, "flit");
  m_credit_pool = new pool_c<credit_c>(100, "credit");
}

network_c::~network_c() {
  delete m_flit_pool;
  delete m_credit_pool;
}

bool network_c::send(mem_req_s* req, int src_level, int src_id, int dst_level,
                     int dst_id) {
  req->m_msg_src = m_router_map[src_level * 1000 + src_id];
  req->m_msg_dst = m_router_map[dst_level * 1000 + dst_id];

  DEBUG("src_level:%d src_id:%d (%d)  dst_level:%d dst_id:%d (%d)\n", src_level,
        src_id, req->m_msg_src, dst_level, dst_id, req->m_msg_dst);

  return m_router[req->m_msg_src]->inject_packet(req);
}

mem_req_s* network_c::receive(int level, int id) {
  //  cout << "in receive " << level << " " << id << " " << m_router_map[level*1000+id] << " " <<
  //  m_router[m_router_map[level*1000+id]]->get_id() << "\n";
  return m_router[m_router_map[level * 1000 + id]]->receive_req(0);
}

void network_c::receive_pop(int level, int id) {
  return m_router[m_router_map[level * 1000 + id]]->pop_req(0);
}

/////////////////////////////////////////////////////////////////////////////////////////

router_c::router_c(macsim_c* simBase, int type, int id, int num_port)
  : m_simBase(simBase), m_type(type), m_id(id), m_num_port(num_port) {
  // configuration
  m_num_vc = *KNOB(KNOB_NUM_VC);
  m_link_latency = *KNOB(KNOB_LINK_LATENCY);
  m_arbitration_policy = *KNOB(KNOB_ARBITRATION_POLICY);
  m_link_width = *KNOB(KNOB_LINK_WIDTH);
  m_num_vc_cpu = *KNOB(KNOB_CPU_VC_PARTITION);
  m_next_vc = 0;

  // link setting
  m_opposite_dir[LOCAL] = LOCAL;
  m_opposite_dir[LEFT] = RIGHT;
  m_opposite_dir[RIGHT] = LEFT;
  m_opposite_dir[UP] = DOWN;
  m_opposite_dir[DOWN] = UP;

  // memory allocations
  m_req_buffer = new queue<mem_req_s*>;

  m_injection_buffer = new list<mem_req_s*>;
  m_injection_buffer_max_size = 32;

  m_input_buffer = new list<flit_c*>*[m_num_port];
  m_output_buffer = new list<flit_c*>*[m_num_port];
  m_route = new bool***[m_num_port];
  m_route_fixed = new int*[m_num_port];
  m_output_vc_avail = new bool*[m_num_port];
  m_output_vc_id = new int*[m_num_port];
  m_output_port_id = new int*[m_num_port];
  m_credit = new int*[m_num_port];

  for (int ii = 0; ii < m_num_port; ++ii) {
    m_input_buffer[ii] = new list<flit_c*>[m_num_vc];
    m_output_buffer[ii] = new list<flit_c*>[m_num_vc];
    m_route_fixed[ii] = new int[m_num_vc];
    fill_n(m_route_fixed[ii], m_num_vc, -1);
    m_route[ii] = new bool**[m_num_vc];
    for (int jj = 0; jj < m_num_vc; ++jj) {
      m_route[ii][jj] = new bool*[2];
      for (int kk = 0; kk < 2; ++kk) {
        m_route[ii][jj][kk] = new bool[m_num_port];
        // fill_n(m_route[ii][jj][kk], m_num_vc, false);
        fill_n(m_route[ii][jj][kk], m_num_port, false);
      }
    }

    m_output_vc_avail[ii] = new bool[m_num_vc];
    fill_n(m_output_vc_avail[ii], m_num_vc, true);

    m_output_vc_id[ii] = new int[m_num_vc];
    fill_n(m_output_vc_id[ii], m_num_vc, -1);

    m_output_port_id[ii] = new int[m_num_vc];
    fill_n(m_output_port_id[ii], m_num_vc, -1);

    m_credit[ii] = new int[m_num_vc];
    fill_n(m_credit[ii], m_num_vc, 10);
  }

  m_buffer_max_size = 10;

  // switch
  m_sw_avail = new Counter[m_num_port];
  fill_n(m_sw_avail, m_num_port, 0);

  m_link_avail = new Counter[m_num_port];
  fill_n(m_link_avail, m_num_port, 0);

  m_pending_credit = new list<credit_c*>;
}

router_c::~router_c() {
  delete m_req_buffer;
  delete m_injection_buffer;
  for (int ii = 0; ii < m_num_port; ++ii) {
    delete[] m_input_buffer[ii];
    delete[] m_output_buffer[ii];
    delete[] m_route[ii];
    delete[] m_route_fixed[ii];
    delete[] m_output_vc_avail[ii];
    delete[] m_output_vc_id[ii];
    delete[] m_credit[ii];
  }
  delete[] m_input_buffer;
  delete[] m_output_buffer;
  delete[] m_route;
  delete[] m_route_fixed;
  delete[] m_output_vc_avail;
  delete[] m_output_vc_id;
  delete[] m_credit;

  delete[] m_sw_avail;
  delete[] m_link_avail;

  delete m_pending_credit;
}

int router_c::get_id(void) {
  return m_id;
}

void router_c::set_id(int id) {
  m_id = id;
}

void router_c::run_a_cycle(bool pll_lock) {
  if (pll_lock) {
    ++m_cycle;
    return;
  }

  if (*KNOB(KNOB_IDEAL_NOC)) {
    stage_vca();
    stage_rc();
    local_packet_injection();
  } else {
    //  check_starvation();
    process_pending_credit();
    stage_lt();
    stage_sa();
    stage_st();
    stage_rc();
    stage_vca();
    local_packet_injection();
  }

  // stats
  //  check_channel();

  ++m_cycle;
}

// insert a packet from the network interface (NI)
bool router_c::inject_packet(mem_req_s* req) {
  if (m_injection_buffer->size() < m_injection_buffer_max_size) {
    m_injection_buffer->push_back(req);
    return true;
  }

  return false;
}

// insert a packet from the injection buffer
void router_c::local_packet_injection(void) {
  while (1) {
    if (m_injection_buffer->empty()) break;

    bool req_inserted = false;
#ifdef GPU_VALIDATION
    int last_tried_vc = -1;
#endif
    for (int ii = 0; ii < m_num_vc; ++ii) {
      // check buffer availability to insert a new request
      bool cpu_queue = false;
      mem_req_s* req;
      if (m_injection_buffer->empty()) continue;
      req = m_injection_buffer->front();
      cpu_queue = false;

      assert(req);
      int num_flit = 1;
      if ((req->m_msg_type == NOC_NEW_WITH_DATA) ||
          (req->m_msg_type == NOC_FILL))
        num_flit += req->m_size / m_link_width;

      if (*KNOB(KNOB_IDEAL_NOC)) num_flit = 1;

#ifdef GPU_VALIDATION
      last_tried_vc = (m_next_vc + ii) % m_num_vc;
      if (m_input_buffer[0][(m_next_vc + ii) % m_num_vc].size() + num_flit <=
          m_buffer_max_size) {
#else
      if (m_input_buffer[0][ii].size() + num_flit <= m_buffer_max_size) {
#endif
        // flit generation and insert into the buffer
        STAT_EVENT(TOTAL_PACKET_CPU + req->m_acc);
        req->m_noc_cycle = m_cycle;

        // stat handling
        ++g_total_packet;
        if (req->m_acc) {
          ++g_total_gpu_packet;
          STAT_EVENT(NOC_AVG_ACTIVE_PACKET_BASE_GPU);
          STAT_EVENT_N(NOC_AVG_ACTIVE_PACKET_GPU, g_total_gpu_packet);
        } else {
          ++g_total_cpu_packet;
          STAT_EVENT(NOC_AVG_ACTIVE_PACKET_BASE_CPU);
          STAT_EVENT_N(NOC_AVG_ACTIVE_PACKET_CPU, g_total_cpu_packet);
        }

        STAT_EVENT(NOC_AVG_ACTIVE_PACKET_BASE);
        STAT_EVENT_N(NOC_AVG_ACTIVE_PACKET, g_total_packet);

        // packet generation
        for (int jj = 0; jj < num_flit; ++jj) {
          flit_c* new_flit = m_flit_pool->acquire_entry();
          new_flit->m_req = req;
          new_flit->m_src = req->m_msg_src;
          new_flit->m_dst = req->m_msg_dst;

          if (jj == 0)
            new_flit->m_head = true;
          else
            new_flit->m_head = false;

          if (jj == num_flit - 1)
            new_flit->m_tail = true;
          else
            new_flit->m_tail = false;

          new_flit->m_state = IB;
          new_flit->m_timestamp = m_cycle;
          new_flit->m_rdy_cycle = m_cycle;
          new_flit->m_id = jj;
          new_flit->m_dir = -1;

// insert all flits to input_buffer[LOCAL][vc]
#ifdef GPU_VALIDATION
          m_input_buffer[0][(m_next_vc + ii) % m_num_vc].push_back(new_flit);
#else
          m_input_buffer[0][ii].push_back(new_flit);
#endif
        }

        // pop a request from the injection queue
        m_injection_buffer->pop_front();
        req_inserted = true;

        DEBUG("cycle:%-10lld node:%d [IB] req_id:%d src:%d dst:%d\n", m_cycle,
              m_id, req->m_id, req->m_msg_src, req->m_msg_dst);

        break;  // why this break?
      }
    }

#ifdef GPU_VALIDATION
    if (last_tried_vc != -1 && *KNOB(KNOB_USE_RR_FOR_NOC_INSERTION)) {
      m_next_vc = (last_tried_vc + 1) % m_num_vc;
    }
#endif

    // Nothing was scheduled. Stop inserting!
    if (!req_inserted) break;
  }
}

void router_c::stage_vca(void) {
  if (*KNOB(KNOB_IDEAL_NOC)) {
    for (int ip = 0; ip < m_num_port; ++ip) {
      for (int ivc = 0; ivc < m_num_vc; ++ivc) {
        if (m_input_buffer[ip][ivc].empty()) {
          continue;
        }

        flit_c* flit = m_input_buffer[ip][ivc].front();
        if (flit->m_state != RC ||
            flit->m_timestamp + *KNOB(KNOB_IDEAL_NOC_LATENCY) > m_cycle) {
          continue;
        }
        // insert to next router
        int port = -1;
        for (int ii = 0; ii < m_num_port; ++ii) {
          if (m_route[ip][ivc][0][ii]) {
            port = ii;
            break;
          }
        }
        assert(port != -1);

        if (port != LOCAL) {
          m_link[port]->insert_packet(flit, m_opposite_dir[port], ivc);
          flit->m_rdy_cycle = m_cycle + 1;
        }

        // delete flit in the buffer
        m_input_buffer[ip][ivc].pop_front();

        // free 1) switch, 2) ivc_avail[ip][ivc], 3) rc, 4) vc
        if (flit->m_tail) {
          if (port == LOCAL) {
            m_req_buffer->push(flit->m_req);
          }
        }

        if (port == LOCAL) {
          flit->init();
          m_flit_pool->release_entry(flit);
        }
      }
    }
    return;
  }

  // VCA (Virtual Channel Allocation) stage
  // for each output port
  for (int oport = 0; oport < m_num_port; ++oport) {
    for (int ovc = 0; ovc < m_num_vc; ++ovc) {
      // if there is available vc in the output port
      if (m_output_vc_avail[oport][ovc]) {
        int iport, ivc;
        stage_vca_pick_winner(oport, ovc, iport, ivc);
        if (iport != -1) {
          flit_c* flit = m_input_buffer[iport][ivc].front();
          flit->m_state = VCA;

          m_output_port_id[iport][ivc] = oport;
          m_output_vc_id[iport][ivc] = ovc;
          m_output_vc_avail[oport][ovc] = false;
          m_route_fixed[iport][ivc] = oport;

          DEBUG(
            "cycle:%-10lld node:%d [VA] req_id:%d flit_id:%d src:%d dst:%d "
            "ip:%d ic:%d "
            "op:%d oc:%d ptx:%d\n",
            m_cycle, m_id, flit->m_req->m_id, flit->m_id,
            flit->m_req->m_msg_src, flit->m_req->m_msg_dst, iport, ivc,
            m_route_fixed[iport][ivc], ovc, flit->m_req->m_acc);
        }
      }
    }
  }
}

int router_c::get_ovc_occupancy(int port, bool type) {
  int search_begin = 0;
  int search_end = m_num_vc;

  if (m_enable_vc_partition) {
    if (type == false) {
      search_end = m_num_vc_cpu;
    } else {
      search_begin = m_num_vc_cpu;
    }
  }

  int count = 0;

  for (int vc = search_begin; vc < search_end; ++vc)
    if (m_output_vc_avail[port][vc]) ++count;

  return count;
}

// SA (Switch Allocation) stage
void router_c::stage_sa(void) {
  for (int op = 0; op < m_num_port; ++op) {
    if (m_sw_avail[op] <= m_cycle) {
      int ip, ivc;
      stage_sa_pick_winner(op, ip, ivc, 0);

      if (ip != -1) {
        flit_c* flit = m_input_buffer[ip][ivc].front();
        flit->m_state = SA;

        m_sw_avail[op] = m_cycle + 1;

        DEBUG(
          "cycle:%-10lld node:%d [SA] req_id:%d flit_id:%d src:%d dst:%d ip:%d "
          "ic:%d op:%d oc:%d route:%d port:%d\n",
          m_cycle, m_id, flit->m_req->m_id, flit->m_id, flit->m_req->m_msg_src,
          flit->m_req->m_msg_dst, ip, ivc, m_route_fixed[ip][ivc],
          m_output_vc_id[ip][ivc], m_route_fixed[ip][ivc],
          m_output_port_id[ip][ivc]);
      }
    }
  }
}

void router_c::stage_sa_pick_winner(int op, int& ip, int& ivc, int sw_id) {
  if (m_arbitration_policy == OLDEST_FIRST) {
    Counter oldest_timestamp = ULLONG_MAX;
    ip = -1;
    for (int ii = 0; ii < m_num_port; ++ii) {
      if (ii == op) continue;

      for (int jj = 0; jj < m_num_vc; ++jj) {
        // find a flit that acquires a vc in current output port
        if (m_route_fixed[ii][jj] == op && m_output_port_id[ii][jj] == op &&
            !m_input_buffer[ii][jj].empty()) {
          flit_c* flit = m_input_buffer[ii][jj].front();

          // VCA & Header or IB & Body/Tail
          if (((flit->m_head && flit->m_state == VCA) ||
               (!flit->m_head && flit->m_state == IB)) &&
              flit->m_timestamp < oldest_timestamp) {
            oldest_timestamp = flit->m_timestamp;
            ip = ii;
            ivc = jj;
          }
        }
      }
    }
  }
}

// ST-stage
void router_c::stage_st(void) {
  for (int op = 0; op < m_num_port; ++op) {
    int ip = -1;
    int ivc = -1;
    for (int ii = 0; ii < m_num_port; ++ii) {
      if (ii == op) continue;

      for (int jj = 0; jj < m_num_vc; ++jj) {
        if (m_route_fixed[ii][jj] == op && m_output_port_id[ii][jj] == op &&
            !m_input_buffer[ii][jj].empty()) {
          flit_c* flit = m_input_buffer[ii][jj].front();
          if (flit->m_state == SA) {
            ip = ii;
            ivc = jj;
            break;
          }
        }
      }
    }

    if (ip != -1) {
      flit_c* flit = m_input_buffer[ip][ivc].front();
      flit->m_state = ST;

      // insert a flit to the output buffer
      int ovc = m_output_vc_id[ip][ivc];
      m_output_buffer[op][ovc].push_back(flit);

      // pop a flit from the input buffer
      m_input_buffer[ip][ivc].pop_front();

      // all flits traversed, so need to free a input vc
      if (flit->m_tail) {
        m_route_fixed[ip][ivc] = -1;
        m_output_vc_id[ip][ivc] = -1;
        m_output_port_id[ip][ivc] = -1;
      }

      // send a credit back to previous router
      if (ip != LOCAL) {
        credit_c* credit = m_credit_pool->acquire_entry();
        credit->m_port = m_opposite_dir[ip];
        credit->m_vc = ivc;
        credit->m_rdy_cycle = m_cycle + 1;
        m_link[ip]->insert_credit(credit);
      }

      DEBUG(
        "cycle:%-10lld node:%d [ST] req_id:%d flit_id:%d src:%d dst:%d ip:%d "
        "ic:%d port:%d ovc:%d\n",
        m_cycle, m_id, flit->m_req->m_id, flit->m_id, flit->m_req->m_msg_src,
        flit->m_req->m_msg_dst, ip, ivc, m_output_port_id[ip][ivc], ovc);
    }
  }
}

void router_c::stage_lt(void) {
  for (int port = 0; port < m_num_port; ++port) {
    if (m_link_avail[port] > m_cycle) continue;

    Counter oldest_cycle = ULLONG_MAX;
    flit_c* f = NULL;
    int vc = -1;
    for (int ii_d = 0; ii_d < m_num_vc; ++ii_d) {
      int ii = (ii_d + m_cycle) % m_num_vc;
      if (m_output_buffer[port][ii].empty() || m_credit[port][ii] == 0)
        continue;

      flit_c* flit = m_output_buffer[port][ii].front();
      assert(flit->m_state == ST);

      if (flit->m_timestamp < oldest_cycle) {
        oldest_cycle = flit->m_timestamp;
        vc = ii;
        f = flit;
        break;
      }
    }

    if (vc != -1) {
      // insert to next router
      if (port != LOCAL) {
        DEBUG(
          "cycle:%-10lld node:%d [LT] req_id:%d flit_id:%d src:%d dst:%d "
          "port:%d vc:%d\n",
          m_cycle, m_id, f->m_req->m_id, f->m_id, f->m_req->m_msg_src,
          f->m_req->m_msg_dst, port, vc);

        m_link[port]->insert_packet(f, m_opposite_dir[port], vc);
        f->m_rdy_cycle = m_cycle + m_link_latency + 1;
        m_link_avail[port] = m_cycle + m_link_latency;  // link busy
        STAT_EVENT(NOC_LINK_ACTIVE);
      }

      // delete flit in the buffer
      m_output_buffer[port][vc].pop_front();

      if (port != LOCAL) --m_credit[port][vc];

      // free 1) switch, 2) ivc_avail[ip][ivc], 3) rc, 4) vc
      if (f->m_tail) {
        STAT_EVENT(NOC_AVG_WAIT_IN_ROUTER_BASE);
        STAT_EVENT_N(NOC_AVG_WAIT_IN_ROUTER, m_cycle - f->m_timestamp);

        STAT_EVENT(NOC_AVG_WAIT_IN_ROUTER_BASE_CPU + m_type);
        STAT_EVENT_N(NOC_AVG_WAIT_IN_ROUTER_CPU + m_type,
                     m_cycle - f->m_timestamp);

        m_output_vc_avail[port][vc] = true;

        if (port == LOCAL) {
          --g_total_packet;
          if (f->m_req->m_acc) {
            --g_total_gpu_packet;
          } else {
            --g_total_cpu_packet;
          }
          m_req_buffer->push(f->m_req);
          DEBUG(
            "cycle:%-10lld node:%d [TT] req_id:%d flit_id:%d src:%d dst:%d "
            "vc:%d\n",
            m_cycle, m_id, f->m_req->m_id, f->m_id, f->m_req->m_msg_src,
            f->m_req->m_msg_dst, vc);

          STAT_EVENT(NOC_AVG_LATENCY_BASE);
          STAT_EVENT_N(NOC_AVG_LATENCY, m_cycle - f->m_req->m_noc_cycle);

          STAT_EVENT(NOC_AVG_LATENCY_BASE_CPU + f->m_req->m_acc);
          STAT_EVENT_N(NOC_AVG_LATENCY_CPU + f->m_req->m_acc,
                       m_cycle - f->m_req->m_noc_cycle);
        }
      }

      if (port == LOCAL) {
        f->init();
        m_flit_pool->release_entry(f);
      }
    }
  }
}

void router_c::check_channel(void) {
}

void router_c::insert_packet(flit_c* flit, int port, int vc) {
  // IB (Input Buffering) stage
  if (flit->m_head)
    DEBUG("cycle:%-10lld node:%d [IB] req_id:%d src:%d dst:%d ip:%d vc:%d\n",
          m_cycle, m_id, flit->m_req->m_id, flit->m_req->m_msg_src,
          flit->m_req->m_msg_dst, port, vc);

  m_input_buffer[port][vc].push_back(flit);
  flit->m_state = IB;
}

void router_c::insert_packet(mem_req_s* req) {
  m_req_buffer->push(req);
}

void router_c::insert_credit(credit_c* credit) {
  m_pending_credit->push_back(credit);
}

void router_c::process_pending_credit(void) {
  if (m_pending_credit->empty()) return;

  auto I = m_pending_credit->begin();
  auto E = m_pending_credit->end();
  do {
    credit_c* credit = (*I++);
    if (credit->m_rdy_cycle <= CYCLE) {
      ++m_credit[credit->m_port][credit->m_vc];
      m_pending_credit->remove(credit);
      m_credit_pool->release_entry(credit);
    }
  } while (I != E);
}

mem_req_s* router_c::receive_req(int dir) {
  if (m_req_buffer->empty())
    return NULL;
  else
    return m_req_buffer->front();
}

void router_c::pop_req(int dir) {
  m_req_buffer->pop();
}

void router_c::init(int total_router, int* total_packet,
                    pool_c<flit_c>* flit_pool, pool_c<credit_c>* credit_pool) {
  m_total_router = total_router;
  m_total_packet = total_packet;
  m_flit_pool = flit_pool;
  m_credit_pool = credit_pool;
}

void router_c::print(ofstream& out) {
}

void router_c::check_starvation(void) {
  return;
}

void router_c::set_link(int dir, router_c* link) {
  m_link[dir] = link;
}

router_c* router_c::get_router(int dir) {
  return m_link[dir];
}

void router_c::set_router_map(deque<router_c*>& router_map) {
  m_router_map = router_map;
}

void router_c::reset(void) {
}

int* router_c::get_num_packet_inserted(void) {
  return NULL;
}
