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
 * File         : dram_sst_vaultsim.cc 
 * Author       : HPArch Research Group
 * Date         : 05/12/2014
 * Description  : Memory Controller for SST-VaultSim Component
 *********************************************************************************************/


#include "dram_sst_vaultsim.h"
#include "assert_macros.h"
#include "debug_macros.h"
#include "bug_detector.h"
#include "memory.h"
#include "all_knobs.h"

#undef DEBUG
#define DEBUG(args...) _DEBUG(*m_simBase->m_knobs->KNOB_DEBUG_DRAM, ## args)

///////////////////////////////////////////////////////////////////////////////////////////////
// wrapper functions to allocate dram controller object

dram_c* vaultsim_controller(macsim_c* simBase)
{
  dram_c* vaultsim = new dram_sst_vaultsim_c(simBase);
  return vaultsim;
}

dram_sst_vaultsim_c::dram_sst_vaultsim_c(macsim_c* simBase) : dram_c(simBase)
{
  m_output_buffer = new list<mem_req_s*>;
}

dram_sst_vaultsim_c::~dram_sst_vaultsim_c()
{
  delete m_output_buffer;
}

void dram_sst_vaultsim_c::print_req(void)
{
}

void dram_sst_vaultsim_c::init(int id)
{
  m_id = id;
}

void dram_sst_vaultsim_c::run_a_cycle(bool pll_lock)
{
  if (pll_lock) {
    ++m_cycle;
    return;
  }

  receive_packet();

  send();
  receive();
  ++m_cycle;
}

void dram_sst_vaultsim_c::receive_packet()
{
  for (auto it = m_pending_request.begin(), et = m_pending_request.end(); it != et; ++it) { 
    uint64_t key = it->first;
    mem_req_s* req = it->second;

    bool responseArrived = (*(m_simBase->strobeCubeRespQ))(key);
    if (responseArrived) {
      if (req->m_type == MRT_DSTORE) {
        write_callback(key);
      } else {
        read_callback(key);
      }
    }
  }
}

void dram_sst_vaultsim_c::read_callback(uint64_t key)
{
  auto it = m_pending_request.find(key);
  if (it != m_pending_request.end()) {
    m_output_buffer->push_back(it->second);
    m_pending_request.erase(it);
  }
}

void dram_sst_vaultsim_c::write_callback(uint64_t key)
{
  auto it = m_pending_request.find(key);
  if (it != m_pending_request.end()) {
    mem_req_s* req = it->second;
    MEMORY->free_req(req->m_core_id, req);
    m_pending_request.erase(it);
  }
}

void dram_sst_vaultsim_c::send_packet(mem_req_s* req)
{
  uint64_t key = static_cast<uint64_t>(req->m_id);
  uint64_t addr = static_cast<uint64_t>(req->m_addr);
  int size = req->m_size;
  int type = req->m_type == MRT_DSTORE ? MEM_ST : MEM_LD;

  (*(m_simBase->sendCubeReq))(key, addr, size, type);
}

void dram_sst_vaultsim_c::receive(void)
{
  mem_req_s* req = NETWORK->receive(MEM_MC, m_id);
  if (!req) return;

  send_packet(req);
  m_pending_request[req->m_id] = req;
  NETWORK->receive_pop(MEM_MC, m_id);
  if (*KNOB(KNOB_BUG_DETECTOR_ENABLE)) {
    m_simBase->m_bug_detector->deallocate_noc(req);
  }
}

void dram_sst_vaultsim_c::send(void)
{
  vector<mem_req_s*> temp_list;
  for (auto I = m_output_buffer->begin(), E = m_output_buffer->end(); I != E; ++I) {
    mem_req_s* req = (*I);
    req->m_msg_type = NOC_FILL;
    bool insert_packet = NETWORK->send(req, MEM_MC, m_id, MEM_L3, req->m_cache_id[MEM_L3]);

    if (!insert_packet) {
      DEBUG("MC[%d] req:%d addr:0x%llx type:%s noc busy\n", 
          m_id, req->m_id, req->m_addr, mem_req_c::mem_req_type_name[req->m_type]);
      break;
    }

    temp_list.push_back(req);
    if (*KNOB(KNOB_BUG_DETECTOR_ENABLE) && *KNOB(KNOB_ENABLE_NEW_NOC)) {
      m_simBase->m_bug_detector->allocate_noc(req);
    }
  }

  for (auto I = temp_list.begin(), E = temp_list.end(); I != E; ++I) {
    m_output_buffer->remove((*I));
  }
}
