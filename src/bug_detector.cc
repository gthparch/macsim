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
 * File         : bug_detector.h
 * Author       : Jaekyu Lee
 * Date         : 9/8/2010
 * SVN          : $Id: dram.h 868 2009-11-05 06:28:01Z kacear $
 * Description  : Bug Detector
 *********************************************************************************************/


#include <fstream>
#include <iomanip>
#include <list>

#include "bug_detector.h"
#include "memreq_info.h"
#include "dram.h"
#include "assert_macros.h"

#include "uop.h"
#include "trace_read.h"

#include "all_knobs.h"

// constructor
bug_detector_c::bug_detector_c(macsim_c* simBase)
{
  m_simBase = simBase;

  if (!*m_simBase->m_knobs->KNOB_BUG_DETECTOR_ENABLE)
    return ;

  m_num_core = *m_simBase->m_knobs->KNOB_NUM_SIM_CORES;

  for (int ii = 0; ii < m_num_core; ++ii) {
    map<uop_c*, uint64_t> *new_map = new map<uop_c*, uint64_t>;
    m_uop_table.push_back(new_map);
  }

  m_latency_sum   = new uint64_t[m_num_core];
  m_latency_count = new uint64_t[m_num_core];
  
  std::fill_n(m_latency_sum, m_num_core, 0);
  std::fill_n(m_latency_count, m_num_core, 0);

  // noc request
  m_packet_table = new unordered_map<mem_req_s*, uint64_t>; 
}


// destructor
bug_detector_c::~bug_detector_c()
{
  if (!*m_simBase->m_knobs->KNOB_BUG_DETECTOR_ENABLE)
    return ;

  delete m_latency_sum;
  delete m_latency_count;

  while (!m_uop_table.empty()) {
    auto *new_map = m_uop_table.back();
    m_uop_table.pop_back();

    new_map->clear();
    delete new_map;
  }


  // noc request
  m_packet_table->clear();
  delete m_packet_table;
}


// insert a new uop to the table
void bug_detector_c::allocate(uop_c *uop)
{
  if (!*m_simBase->m_knobs->KNOB_BUG_DETECTOR_ENABLE)
    return ;

  int core_id = uop->m_core_id;

  ASSERT(m_uop_table[core_id]->find(uop) == m_uop_table[core_id]->end());
  (*m_uop_table[core_id])[uop] = CYCLE;
}


// delete an uop from the table
void bug_detector_c::deallocate(uop_c *uop)
{
  if (!*m_simBase->m_knobs->KNOB_BUG_DETECTOR_ENABLE)
    return ;

  int core_id = uop->m_core_id;

  ASSERT(m_uop_table[core_id]->find(uop) != m_uop_table[core_id]->end());

  int latency = CYCLE - (*m_uop_table[core_id])[uop];
  m_latency_sum[core_id] += latency;
  ++m_latency_count[core_id];

  m_uop_table[core_id]->erase(uop);
}


// sort uop table based on uop id in ascending order
bool sort_uop(uop_c* a, uop_c* b)
{
  return a->m_uop_num < b->m_uop_num;
}


// print all bug information
void bug_detector_c::print(int core_id, int thread_id)
{
  if (*KNOB(KNOB_BUG_DETECTOR_ENABLE) == false)
    return ;


  ofstream out("bug_detect_uop.out");
  for (int ii = 0; ii < m_num_core; ++ii) {
    unsigned int average_latency = 0;
    if (m_latency_count[ii] > 0)
      average_latency = m_latency_sum[ii] / m_latency_count[ii];

    out << "Current cycle:" << CYCLE << "\n";
    out << "Core id:" << core_id << "\n";
    out << "Last terminated thread:" << thread_id << "\n";

    out << "----------------------------------------------------------\n";
    out << "core " << ii << "\n";
    out << "average execution cycle: " << average_latency << "\n";
    out 
      << setw(10) << left << "INST_NUM"
      << setw(10) << left << "UOP_NUM"
      << setw(10) << left << "TID"
      << setw(15) << left << "CYCLE"
      << setw(15) << left << "DELTA"
      << setw(25) << left << "STATE"
      << setw(25) << left << "OPCODE"
      << setw(20) << left << "UOP_TYPE"
      << setw(20) << left << "MEM_TYPE"
      << setw(20) << left << "CF_TYPE"
      << setw(20) << left << "DEP_TYPE" 
      << setw(6)  << left << "CHILD"
      << setw(10) << left << "PARENT"
      << "\n";

    list<uop_c*> temp_list1;
    for (auto I = m_uop_table[ii]->begin(), E = m_uop_table[ii]->end(); I != E; ++I) {
      temp_list1.push_back((*I).first);
    }
    temp_list1.sort(sort_uop);

    for (auto I = temp_list1.begin(), E = temp_list1.end(); I != E; ++I) {
      if (CYCLE - (*m_uop_table[ii])[(*I)] < average_latency * 5)
        continue;

      uop_c *uop = (*I);
      ASSERT(uop);
      out
        << setw(10) << left << uop->m_inst_num
        << setw(10) << left << uop->m_uop_num
        << setw(10) << left << uop->m_thread_id
        << setw(15) << left << (*m_uop_table[ii])[(*I)]
        << setw(15) << left << CYCLE - (*m_uop_table[ii])[(*I)]
        << setw(25) << left << uop_c::g_uop_state_name[uop->m_state] 
        << setw(25) << left << trace_read_c::g_tr_opcode_names[uop->m_opcode] 
        << setw(20) << left << uop_c::g_uop_type_name[uop->m_uop_type]
        << setw(20) << left << uop_c::g_mem_type_name[uop->m_mem_type]
        << setw(20) << left << uop_c::g_cf_type_name[uop->m_cf_type]
        << setw(20) << left << uop_c::g_dep_type_name[uop->m_bar_type]
        << setw(6)  << left << uop->m_num_child_uops
        << setw(10) << left << (uop->m_parent_uop == NULL ? 0 : uop->m_parent_uop->m_uop_num)
        << endl;
    }
    out << "\n\n";
    temp_list1.clear();
  }
  out.close();
}


void bug_detector_c::allocate_noc(mem_req_s* req)
{
  assert(m_packet_table->find(req) == m_packet_table->end());

  (*m_packet_table)[req] = CYCLE;
}


void bug_detector_c::deallocate_noc(mem_req_s* req)
{
  assert(m_packet_table->find(req) != m_packet_table->end());
  m_packet_table->erase(req);
}

bool sort_noc(pair<mem_req_s*, uint64_t>& a, pair<mem_req_s*, uint64_t>& b)
{
  return a.second < b.second;
}


void bug_detector_c::print_noc()
{
  ofstream out("bug_detect_noc.out");

  list<pair<mem_req_s*, uint64_t> > result_list;
  for (auto I = m_packet_table->begin(), E = m_packet_table->end(); I != E; ++I) {
    mem_req_s* req = (*I).first;
    uint64_t cycle = (*I).second;
    result_list.push_back(pair<mem_req_s*, uint64_t>(req, cycle));
  } 
  result_list.sort(sort_noc);


  out << "Current cycle:" << CYCLE << "\n";
  out << left << setw(15) << "ID"
    << left << setw(15) << "CYCLE"
    << left << setw(15) << "DELTA"
    << left << setw(4) << "SRC"
    << left << setw(4) << "DST"
    << "\n";
  for (auto I = result_list.begin(), E = result_list.end(); I != E; ++I) {
    mem_req_s* req = (*I).first;
    uint64_t cycle = (*I).second;
    out << left << setw(15) << req->m_id
      << left << setw(15) << cycle
      << left << setw(15) << CYCLE - cycle
      << left << setw(4) << req->m_msg_src
      << left << setw(4) << req->m_msg_dst
      << "\n";
  }

  out.close();
}



