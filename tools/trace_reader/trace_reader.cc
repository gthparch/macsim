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


#include "trace_reader.h"
#include <iostream>
#include "all_knobs.h"

trace_reader_c trace_reader_c::Singleton;


trace_reader_c::trace_reader_c()
{
}

void trace_reader_c::init()
{
  if (g_knobs->KNOB_ENABLE_REUSE_DIST->getValue()) {
    trace_reader_c* reuse_distance = new reuse_distance_c;
    m_tracer.push_back(reuse_distance);
  }

  if (g_knobs->KNOB_ENABLE_COUNT_STATIC_PC->getValue()) {
    trace_reader_c* static_pc = new static_pc_c;
    m_tracer.push_back(static_pc);
  }
}

void trace_reader_c::reset()
{
  for (int ii = 0; ii < m_tracer.size(); ++ii) 
    m_tracer[ii]->reset();
}


trace_reader_c::~trace_reader_c()
{
  for (int ii = 0; ii < m_tracer.size(); ++ii) {
    delete m_tracer[ii];
  }

  m_tracer.clear();
}

void trace_reader_c::inst_event(trace_info_s* inst)
{
  for (int ii = 0; ii < m_tracer.size(); ++ii) {
    m_tracer[ii]->inst_event(inst);
  }
}


void trace_reader_c::print()
{
}


/////////////////////////////////////////////////////////////////////////////////////////


reuse_distance_c::reuse_distance_c()
{
  m_name = "reuse_distance";
  m_self_counter = 0;
}


reuse_distance_c::~reuse_distance_c()
{
}


void reuse_distance_c::inst_event(trace_info_s* inst)
{
  if (static_cast<int>(inst->m_num_ld) > 0 or inst->m_has_st == true) {
    ++m_self_counter;
    Addr addr;
    if (inst->m_has_st) {
      addr = inst->m_st_vaddr;
    } else {
      addr = inst->m_ld_vaddr1;
    }

    addr = addr >> 6;

    if (m_reuse_map.find(addr) != m_reuse_map.end()) {
      bool same_pc = false;
      if (inst->m_instruction_addr == m_reuse_pc_map[addr]) same_pc = true;
      cout << dec << m_self_counter << " dist:" << dec << m_self_counter - m_reuse_map[addr] << " addr:" 
           << hex << addr << " pc:" << hex << m_reuse_pc_map[addr] << " " << hex << inst->m_instruction_addr << "\n";
    }
    else {
      cout << hex << inst->m_instruction_addr << "\n";
    }
    m_reuse_map[addr] = m_self_counter;
    m_reuse_pc_map[addr] = inst->m_instruction_addr;
  }
}


void reuse_distance_c::print()
{
  std::cout << m_name << "\n";
}


void reuse_distance_c::reset()
{
  m_reuse_map.clear();
}


/////////////////////////////////////////////////////////////////////////////////////////


static_pc_c::static_pc_c() 
{
  m_total_inst_count = 0;
  m_total_store_count = 0;
}

static_pc_c::~static_pc_c() 
{
  cout << "Total static pc:" << m_static_pc.size() << "\n";
  cout << "Total static memory pc:" << m_static_mem_pc.size() << "\n";

  cout << "Total instruction count:" << m_total_inst_count << "\n";
  cout << "Total store count:" << m_total_store_count << "\n";

  m_static_pc.clear();
  m_static_mem_pc.clear();
}

void static_pc_c::inst_event(trace_info_s* inst)
{
  m_static_pc[inst->m_instruction_addr] = true;
  if (static_cast<int>(inst->m_num_ld) > 0 or inst->m_has_st == true) {
    m_static_mem_pc[inst->m_instruction_addr] = true;
  }


  ++m_total_inst_count;
  if (inst->m_has_st == true)
    ++m_total_store_count;
}

