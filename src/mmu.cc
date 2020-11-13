/*
Copyright (c) <2012>, <Georgia Institute of Technology> All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted
provided that the following conditions are met:

Redistributions of source code must retain the above copyright notice, this list
of conditions
and the following disclaimer.

Redistributions in binary form must reproduce the above copyright notice, this
list of
conditions and the following disclaimer in the documentation and/or other
materials provided
with the distribution.

Neither the name of the <Georgia Institue of Technology> nor the names of its
contributors
may be used to endorse or promote products derived from this software without
specific prior
written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY
AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
COPYRIGHT HOLDER OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
OF THE
POSSIBILITY OF SUCH DAMAGE.
*/

/**********************************************************************************************
 * File         : mmu.cc
 * Author       : HPArch Research Group
 * Date         : 1/30/2019
 * Description  : Memory Management Unit
 *********************************************************************************************/

#include <algorithm>
#include <assert.h>
#include <fstream>
#include <iostream>
#include <iterator>
#include <list>

/* macsim */
#include "all_knobs.h"
#include "assert_macros.h"
#include "core.h"
#include "debug_macros.h"
#include "frontend.h"
#include "memory.h"
#include "mmu.h"
#include "statistics.h"

using namespace std;

#define DEBUG(args...) _DEBUG(*m_simBase->m_knobs->KNOB_DEBUG_MMU, ##args)
#define DEBUG_CORE(m_core_id, args...)                        \
  if (m_core_id == *m_simBase->m_knobs->KNOB_DEBUG_CORE_ID) { \
    _DEBUG(*m_simBase->m_knobs->KNOB_DEBUG_MMU, ##args);      \
  }

void MMU::ReplacementUnit::update(Addr page_number) {
  auto it = m_table.find(page_number);
  assert(it != m_table.end());  // found
  // replace the page to the MRU position
  Entry *node = it->second;
  detach(node);
  attach(node);
}

void MMU::ReplacementUnit::insert(Addr page_number) {
  auto it = m_table.find(page_number);
  assert(it == m_table.end());  // not found
  // insert the page into the MRU position

  if (!m_free_entries.empty()) {  // free entry available
    // insert a new entry into the MRU position
    Entry *node = m_free_entries.back();
    m_free_entries.pop_back();
    node->page_number = page_number;
    m_table[page_number] = node;
    attach(node);
  } else {  // free entry not available
    // replace the entry in the LRU position
    Entry *node = m_tail->prev;
    detach(node);
    m_table.erase(node->page_number);
    node->page_number = page_number;
    m_table[page_number] = node;
    attach(node);
  }
}

Addr MMU::ReplacementUnit::getVictim() {
  Entry *node = m_tail->prev;
  detach(node);
  Addr page_number = node->page_number;
  m_table.erase(node->page_number);
  m_free_entries.push_back(node);

  return page_number;
}

void MMU::initialize(macsim_c *simBase) {
  m_simBase = simBase;

  m_page_size = m_simBase->m_knobs->KNOB_PAGE_SIZE->getValue();
  m_offset_bits = (long)log2(m_page_size);

  m_memory_size = m_simBase->m_knobs->KNOB_MEMORY_SIZE->getValue();

  m_free_frames_remaining = m_memory_size >> m_offset_bits;
  m_free_frames.resize(m_free_frames_remaining, true);  // true: page is free

  m_frame_to_allocate = 0;

  m_replacement_unit =
    make_unique<ReplacementUnit>(m_simBase, m_free_frames_remaining);
  m_TLB = make_unique<TLB>(
    m_simBase, m_simBase->m_knobs->KNOB_TLB_NUM_ENTRY->getValue(), m_page_size);

  m_walk_latency = m_simBase->m_knobs->KNOB_PAGE_TABLE_WALK_LATENCY->getValue();
  m_fault_latency = m_simBase->m_knobs->KNOB_PAGE_FAULT_LATENCY->getValue();
  m_eviction_latency =
    m_simBase->m_knobs->KNOB_PAGE_EVICTION_LATENCY->getValue();

  m_batch_processing = false;
  m_batch_processing_start_cycle = 0;
  m_batch_processing_transfer_start_cycle = 0;
  m_batch_processing_overhead =
    m_simBase->m_knobs->KNOB_BATCH_PROCESSING_OVERHEAD->getValue();

  m_fault_buffer_size = m_simBase->m_knobs->KNOB_FAULT_BUFFER_SIZE->getValue();
}

void MMU::finalize() {
  STAT_EVENT_N(UNIQUE_PAGE, m_unique_pages.size());
}

bool MMU::translate(uop_c *cur_uop) {
  if (cur_uop->m_translated) return true;

  Addr addr = cur_uop->m_vaddr;
  Addr page_number = get_page_number(addr);
  Addr page_offset = get_page_offset(addr);

  cur_uop->m_state = OS_TRANS_BEGIN;

  Addr frame_number = -1;
  bool tlb_hit = m_TLB->lookup(addr);

  STAT_CORE_EVENT(cur_uop->m_core_id, TLB_MISS + tlb_hit);
  
  if (tlb_hit) {
    frame_number = m_TLB->translate(addr);
    cur_uop->m_paddr = (frame_number << m_offset_bits) | page_offset;
    cur_uop->m_state = OS_TRANS_DONE;
    cur_uop->m_translated = true;

    DEBUG(
      "TLB hit at %llu - core_id:%d thread_id:%d inst_num:%llu "
      "uop_num:%llu\n",
      m_cycle, cur_uop->m_core_id, cur_uop->m_thread_id, cur_uop->m_inst_num,
      cur_uop->m_uop_num);
    return true;
  }

  DEBUG(
    "TLB miss at %llu - core_id:%d thread_id:%d inst_num:%llu uop_num:%llu\n",
    m_cycle, cur_uop->m_core_id, cur_uop->m_thread_id, cur_uop->m_inst_num,
    cur_uop->m_uop_num);

  // TLB miss occurs
  // Put this request into page table walk queue so that it can be handled later
  // in time
  auto it = m_walk_queue_page.find(page_number);
  if (it !=
      m_walk_queue_page
        .end())  // this page is already being serviced, so piggyback
    it->second.emplace_back(cur_uop);
  else {
    Counter ready_cycle = m_cycle + m_walk_latency;
    m_walk_queue_cycle.emplace(ready_cycle, list<Addr>());
    m_walk_queue_cycle[ready_cycle].emplace_back(page_number);
    m_walk_queue_page.emplace(page_number, list<uop_c *>());
    m_walk_queue_page[page_number].emplace_back(cur_uop);
  }

  cur_uop->m_state = OS_TRANS_WALK_QUEUE;
  if (cur_uop->m_parent_uop) ++cur_uop->m_parent_uop->m_num_page_table_walks;
  return false;
}

void MMU::run_a_cycle(bool pll_lock) {
  if (pll_lock) {
    ++m_cycle;
    return;
  }

  // re-access dcache now that translation is done
  if (!m_retry_queue.empty()) {
    for (auto it = m_retry_queue.begin(); it != m_retry_queue.end();
         /* do nothing */) {
      uop_c *uop = *it;
      DEBUG(
        "retry at %llu - core_id:%d thread_id:%d inst_num:%llu "
        "uop_num:%llu\n",
        m_cycle, uop->m_core_id, uop->m_thread_id, uop->m_inst_num,
        uop->m_uop_num);

      int latency = m_simBase->m_memory->access(uop);
      if (0 != latency) {  // successful execution
        if (latency > 0) {  // cache hit
          DEBUG(
            "cache hit at %llu - core_id:%d thread_id:%d inst_num:%llu "
            "uop_num:%llu\n",
            m_cycle, uop->m_core_id, uop->m_thread_id, uop->m_inst_num,
            uop->m_uop_num);

          if (uop->m_parent_uop) {
            uop_c *puop = uop->m_parent_uop;
            ++puop->m_num_child_uops_done;
            if (puop->m_num_child_uops_done == puop->m_num_child_uops) {
              if (*m_simBase->m_knobs->KNOB_FETCH_ONLY_LOAD_READY) {
                m_simBase->m_core_pointers[puop->m_core_id]
                  ->get_frontend()
                  ->set_load_ready(puop->m_thread_id, puop->m_uop_num);
              }

              puop->m_done_cycle = m_simBase->m_core_cycle[uop->m_core_id] + 1;
              puop->m_state = OS_SCHEDULED;
            }
          } else {
            if (*m_simBase->m_knobs->KNOB_FETCH_ONLY_LOAD_READY) {
              m_simBase->m_core_pointers[uop->m_core_id]
                ->get_frontend()
                ->set_load_ready(uop->m_thread_id, uop->m_uop_num);
            }
          }
        } else {  // TLB miss or cache miss
          if (uop->m_translated)
            DEBUG(
              "cache miss at %llu - core_id:%d thread_id:%d inst_num:%llu "
              "uop_num:%llu\n",
              m_cycle, uop->m_core_id, uop->m_thread_id, uop->m_inst_num,
              uop->m_uop_num);
        }

        it = m_retry_queue.erase(it);
        DEBUG(
          "retry success at %llu - core_id:%d thread_id:%d inst_num:%llu "
          "uop_num:%llu\n",
          m_cycle, uop->m_core_id, uop->m_thread_id, uop->m_inst_num,
          uop->m_uop_num);
      } else {
        ++it;  // for some reason dcache access is not successful.
        // retry later
      }
    }
  }

  // retry page faults once a new batch processing begins
  // i.e., right after the fault buffer drains
  if (m_fault_buffer.empty() && !m_fault_retry_queue.empty()) {
    list<uop_c *> m_fault_retry_queue_processing;
    std::move(m_fault_retry_queue.begin(), m_fault_retry_queue.end(),
              std::back_inserter(m_fault_retry_queue_processing));
    m_fault_retry_queue.clear();

    for (auto &&uop : m_fault_retry_queue_processing) do_page_table_walks(uop);

    m_fault_retry_queue_processing.clear();
  }

  // do page table walks
  for (auto it = m_walk_queue_cycle.begin(); it != m_walk_queue_cycle.end();
       /* do nothing */) {
    if (it->first <= m_cycle) {
      auto &page_list = it->second;
      for (auto &&p : page_list) {
        auto &uop_list = m_walk_queue_page[p];
        for (auto &&uop : uop_list) do_page_table_walks(uop);
        m_walk_queue_page.erase(p);
      }
      it = m_walk_queue_cycle.erase(it);
    } else
      break;
  }

  ++m_cycle;
}

void MMU::do_page_table_walks(uop_c *cur_uop) {
  Addr addr = cur_uop->m_vaddr;
  Addr page_number = get_page_number(addr);
  Addr page_offset = get_page_offset(addr);

  auto it = m_page_table.find(page_number);
  if (it != m_page_table.end()) {  // page table hit
    Addr frame_number = it->second.frame_number;
    cur_uop->m_paddr = (frame_number << m_offset_bits) | page_offset;
    cur_uop->m_state = OS_TRANS_RETRY_QUEUE;
    cur_uop->m_translated = true;

    if (cur_uop->m_parent_uop && cur_uop->m_parent_uop->m_num_page_table_walks)
      --cur_uop->m_parent_uop->m_num_page_table_walks;

    if (!m_TLB->lookup(addr)) m_TLB->insert(addr, frame_number);
    m_TLB->update(addr);

    m_replacement_unit->update(page_number);

    STAT_EVENT(PAGETABLE_HIT);

    DEBUG(
      "page table hit at %llu - core_id:%d thread_id:%d inst_num:%llu "
      "uop_num:%llu\n",
      m_cycle, cur_uop->m_core_id, cur_uop->m_thread_id, cur_uop->m_inst_num,
      cur_uop->m_uop_num);

    // insert uops into retry queue
    m_retry_queue.emplace_back(cur_uop);
  } else {  // page fault
    STAT_EVENT(PAGETABLE_MISS);

    core_c *core = m_simBase->m_core_pointers[cur_uop->m_core_id];
    if (cur_uop->m_parent_uop) {
      core->m_per_thread_fault_parent_uops.emplace(cur_uop->m_thread_id,
                                                   unordered_set<Counter>());
      auto it = core->m_per_thread_fault_parent_uops[cur_uop->m_thread_id].find(
        cur_uop->m_parent_uop->m_uop_num);
      if (it ==
          core->m_per_thread_fault_parent_uops[cur_uop->m_thread_id].end()) {
        // try page table walks later since fault buffer is full
        if (!m_fault_buffer.count(page_number) &&
            m_fault_buffer_size <= m_fault_buffer.size()) {
          cur_uop->m_state = OS_TRANS_FAULT_RETRY_QUEUE;
          m_fault_retry_queue.emplace_back(cur_uop);
          return;
        }
      }
    }

    // put fault request into fault buffer
    m_fault_buffer.emplace(page_number);
    m_fault_uops.emplace(page_number, list<uop_c *>());
    m_fault_uops[page_number].emplace_back(cur_uop);

    if (cur_uop->m_parent_uop) {
      --cur_uop->m_parent_uop->m_num_page_table_walks;
      if (cur_uop->m_parent_uop->m_num_page_table_walks)
        core->m_per_thread_fault_parent_uops[cur_uop->m_thread_id].emplace(
          cur_uop->m_parent_uop->m_uop_num);
      else {
        core->m_per_thread_fault_parent_uops[cur_uop->m_thread_id].erase(
          cur_uop->m_parent_uop->m_uop_num);
        core->m_per_thread_fault_parent_uops.erase(cur_uop->m_thread_id);
      }
    }

    DEBUG(
      "page fault at %llu - core_id:%d thread_id:%d inst_num:%llu "
      "uop_num:%llu, page_number:%llx\n",
      m_cycle, cur_uop->m_core_id, cur_uop->m_thread_id, cur_uop->m_inst_num,
      cur_uop->m_uop_num, page_number);

    cur_uop->m_state = OS_TRANS_FAULT_BUFFER;
  }
}

void MMU::handle_page_faults() {
  // do batch processing if it has started
  if (m_batch_processing) {
    bool ended = do_batch_processing();
    if (!ended) return;
  }

  // begin next batch processing if page faults have occurred during previous
  // batch processing
  // return if no page faults have occurred
  if (m_fault_buffer.empty()) return;

  begin_batch_processing();
}

bool MMU::do_batch_processing() {
  // time between batch processing initialization and first transfer
  if (m_cycle < m_batch_processing_transfer_start_cycle) return false;

  // preparation for the first transfer after overhead
  if (!m_batch_processing_first_transfer_started) {
    if (m_free_frames_remaining > 0)
      m_batch_processing_next_event_cycle = m_cycle + m_fault_latency;
    else {
      m_batch_processing_next_event_cycle =
        m_cycle + m_eviction_latency + m_fault_latency;

      // evict a page
      Addr victim_page = m_replacement_unit->getVictim();
      auto it = m_page_table.find(victim_page);
      assert(it != m_page_table.end());
      Addr victim_frame = it->second.frame_number;
      m_page_table.erase(victim_page);
      m_TLB->invalidate(victim_page);

      // invalidate cache lines of this page
      Addr frame_addr = victim_frame << m_offset_bits;
      m_simBase->m_memory->invalidate(frame_addr);

      m_free_frames[victim_frame] = true;
      ++m_free_frames_remaining;

      m_frame_to_allocate = victim_frame;  // for faster simulation

      STAT_EVENT(EVICTION);
    }

    m_batch_processing_first_transfer_started = true;
    return false;
  }

  assert(m_batch_processing_first_transfer_started);

  // transfer time (+ eviction if needed)
  if (m_cycle < m_batch_processing_next_event_cycle) return false;

  // on m_batch_processing_next_event_cycle, a free page is gauranteed
  assert(m_free_frames_remaining > 0);

  // if this randomly picked page was already assigned
  if (!m_free_frames[m_frame_to_allocate]) {
    // iterate through the list until a free page is found
    Addr starting_page_of_search = m_frame_to_allocate;
    do {
      ++m_frame_to_allocate;
      m_frame_to_allocate %= m_free_frames.size();
    } while ((m_frame_to_allocate != starting_page_of_search) &&
             (!m_free_frames[m_frame_to_allocate]));
  }

  assert(m_free_frames[m_frame_to_allocate]);

  // allocate a new page
  Addr page_number = m_fault_buffer_processing.front();
  m_fault_buffer_processing.pop_front();
  Addr frame_number = m_frame_to_allocate;

  DEBUG("fault resolved page_number:%llx at %llu\n", page_number, m_cycle);

  // allocate an entry in the page table
  m_page_table.emplace(piecewise_construct, forward_as_tuple(page_number),
                       forward_as_tuple(frame_number));

  // update replacement unit
  m_replacement_unit->insert(page_number);

  m_free_frames[frame_number] = false;
  --m_free_frames_remaining;

  // insert uops that tried to access this page into retry queue
  {
    auto it = m_fault_uops_processing.find(page_number);
    assert(it != m_fault_uops_processing.end());
    auto &&uop_list = it->second;
    for (auto &&uop : uop_list) {
      uop->m_state = OS_TRANS_RETRY_QUEUE;
      m_retry_queue.emplace_back(uop);
    }

    uop_list.clear();
    m_fault_uops_processing.erase(page_number);

    // reallocation stats
    {
      size_t old = m_unique_pages.size();
      m_unique_pages.emplace(page_number);
      if (m_unique_pages.size() == old) STAT_EVENT(REALLOCATION);
    }
  }

  // insert uops that tried to access this page during batch processing into
  // retry queue
  {
    auto it = m_fault_uops.find(page_number);
    if (it != m_fault_uops.end()) {
      auto &&uop_list = it->second;
      for (auto &&uop : uop_list) {
        uop->m_state = OS_TRANS_RETRY_QUEUE;
        m_retry_queue.emplace_back(uop);
      }

      uop_list.clear();
      m_fault_uops.erase(page_number);
      m_fault_buffer.erase(page_number);
    }
  }

  // this is the end of current page fault handling

  // handle next page fault below

  if (!m_fault_buffer_processing.empty()) {
    // evict a page if page is full
    if (m_free_frames_remaining > 0)
      m_batch_processing_next_event_cycle = m_cycle + m_fault_latency;
    else {
      m_batch_processing_next_event_cycle =
        m_cycle + m_eviction_latency + m_fault_latency;

      // evict a page
      Addr victim_page = m_replacement_unit->getVictim();
      auto it = m_page_table.find(victim_page);
      assert(it != m_page_table.end());
      Addr victim_frame = it->second.frame_number;
      m_page_table.erase(victim_page);
      m_TLB->invalidate(victim_page);

      // invalidate cache lines of this page
      Addr frame_addr = victim_frame << m_offset_bits;
      m_simBase->m_memory->invalidate(frame_addr);

      m_free_frames[victim_frame] = true;
      ++m_free_frames_remaining;

      m_frame_to_allocate = victim_frame;  // for faster simulation

      STAT_EVENT(EVICTION);
    }

    return false;
  }

  // this is the end of current batch processing

  assert(m_fault_buffer_processing.empty());

  m_batch_processing = false;
  m_batch_processing_first_transfer_started = false;
  m_batch_processing_start_cycle = -1;
  m_batch_processing_transfer_start_cycle = -1;
  m_batch_processing_next_event_cycle = -1;

  DEBUG("batch processing ends at %llu\n", m_cycle);
  return true;
}

void MMU::begin_batch_processing() {
  assert(m_batch_processing == false);

  std::move(m_fault_buffer.begin(), m_fault_buffer.end(),
            std::back_inserter(m_fault_buffer_processing));
  m_fault_buffer_processing.sort();
  m_fault_uops_processing = m_fault_uops;

  m_fault_buffer.clear();
  m_fault_uops.clear();

  m_batch_processing = true;
  m_batch_processing_first_transfer_started = false;
  m_batch_processing_start_cycle = m_cycle;
  m_batch_processing_transfer_start_cycle =
    m_cycle + m_batch_processing_overhead;

  m_batch_processing_next_event_cycle = -1;

  DEBUG("batch processing begins at %llu fault buffer size %zu\n", m_cycle,
        m_fault_buffer_processing.size());
}
