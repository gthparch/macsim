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
 * File         : tlb.cc
 * Author       : HPArch Research Group
 * Date         : 1/30/2019
 * Description  : Translation Lookaside Buffer (TLB)
 *********************************************************************************************/

#include "tlb.h"
#include "debug_macros.h"
#include "assert_macros.h"
#include "all_knobs.h"

#define DEBUG(args...) _DEBUG(*m_simBase->m_knobs->KNOB_DEBUG_TLB, ##args)
#define DEBUG_CORE(m_core_id, args...)                        \
  if (m_core_id == *m_simBase->m_knobs->KNOB_DEBUG_CORE_ID) { \
    _DEBUG(*m_simBase->m_knobs->KNOB_DEBUG_TLB, ##args);      \
  }

TLB::TLB(macsim_c *simBase, long _num_entries, long _page_size)
  : m_simBase(simBase), m_max_entries(_num_entries), m_page_size(_page_size) {
  m_entries = new Entry[m_max_entries];
  for (long i = 0; i < m_max_entries; ++i) m_free_entries.push_back(m_entries + i);

  m_head = new Entry;
  m_tail = new Entry;

  m_head->prev = NULL;
  m_head->next = m_tail;
  m_tail->next = NULL;
  m_tail->prev = m_head;

  m_offset_bits = calc_log2(m_page_size);
}

TLB::~TLB() {
  delete m_head;
  delete m_tail;
  delete[] m_entries;
}

bool TLB::lookup(Addr addr) {
  Addr page_number = get_page_number(addr);
  auto table_iter = m_table.find(page_number);
  return (table_iter != m_table.end());
}

void TLB::update(Addr addr) {
  Addr page_number = get_page_number(addr);
  auto table_iter = m_table.find(page_number);
  ASSERT(table_iter != m_table.end());
  Entry *node = table_iter->second;
  detach(node);
  attach(node);
}

Addr TLB::translate(Addr addr) {
  Addr page_number = get_page_number(addr);
  auto table_iter = m_table.find(page_number);
  ASSERT(table_iter != m_table.end());
  Entry *node = table_iter->second;
  if (!node) DEBUG("page:%llx failed due to node being NULL\n", page_number);
  ASSERT(node);
  detach(node);
  attach(node);
  return node->page_desc.frame_number;
}

void TLB::insert(Addr addr, Addr frame_number) {
  Addr page_number = get_page_number(addr);
  auto table_iter = m_table.find(page_number);
  ASSERT(table_iter == m_table.end());

  // insert an entry into the MRU position
  if (!m_free_entries.empty()) {  // free entry available
                                  // insert a new entry into the MRU position
    Entry *node = m_free_entries.back();
    m_free_entries.pop_back();
    node->page_number = page_number;
    node->page_desc.frame_number = frame_number;
    attach(node);
    m_table.emplace(page_number, node);
    DEBUG("page:%llx inserted - free_entries:%zu\n", page_number, m_free_entries.size());
  } else {  // free entry not available
            // replace the entry in the LRU position
    Entry *node = m_tail->prev;
    detach(node);
    m_table.erase(node->page_number);
    DEBUG("page:%llx replaced - free_entries:%zu\n", node->page_number, m_free_entries.size());
    node->page_number = page_number;
    node->page_desc.frame_number = frame_number;
    m_table.emplace(page_number, node);
    attach(node);
    DEBUG("page:%llx inserted - free_entries:%zu\n", page_number, m_free_entries.size());
  }
}

void TLB::invalidate(Addr page_number) {
  auto table_iter = m_table.find(page_number);
  if (table_iter != m_table.end()) {
    Entry *node = table_iter->second;
    ASSERT(node->page_number == page_number);

    detach(node);
    node->page_number = -1;
    node->page_desc.frame_number = -1;
    m_free_entries.push_back(node);

    m_table.erase(page_number);
    DEBUG("page:%llx invalidated - free_entries:%zu\n", page_number, m_free_entries.size());
  }
}