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
 * File         : mmu.h
 * Author       : HPArch Research Group
 * Date         : 1/30/2019
 * Description  : Memory Management Unit
 *********************************************************************************************/

#ifndef MMU_H_INCLUDED
#define MMU_H_INCLUDED

#include <cstdlib>
#include <cmath>
#include <map>
#include <memory>
#include <utility>
#include <tuple>
#include <set>

#include "macsim.h"
#include "uop.h"
#include "tlb.h"

class MMU // Memory Management Unit
{
private:
  class PageDescriptor
  {
  public:
    PageDescriptor(Addr _frame_number) : frame_number(_frame_number) {}
    PageDescriptor() {}
    Addr frame_number;
  };

  class ReplacementUnit
  {
  public:
    ReplacementUnit(long max_entries)
    {
      m_max_entries = max_entries;
      m_entries = new Entry[m_max_entries];
      for (long i = 0; i < m_max_entries; ++i)
        m_free_entries.push_back(m_entries + i);

      m_head = new Entry;
      m_tail = new Entry;

      m_head->prev = NULL;
      m_head->next = m_tail;
      m_tail->next = NULL;
      m_tail->prev = m_head;
    }

    ~ReplacementUnit()
    {
      delete m_head;
      delete m_tail;
      delete[] m_entries;
    }

    void insert(Addr page_number);
    void update(Addr page_number);
    Addr getVictim();

  private:
    struct Entry
    {
      Addr page_number;
      Entry *prev;
      Entry *next;
    };

    void detach(Entry *node)
    {
      node->prev->next = node->next;
      node->next->prev = node->prev;
    }

    void attach(Entry *node)
    {
      node->next = m_head->next;
      node->prev = m_head;
      m_head->next = node;
      node->next->prev = node;
    }

    map<Addr, Entry *> m_table;
    vector<Entry *> m_free_entries;
    Entry *m_head;
    Entry *m_tail;
    Entry *m_entries;

    long m_max_entries;
  };

public:
  MMU() {}
  ~MMU() {}

  void initialize(macsim_c *simBase);
  void run_a_cycle(bool);

  bool translate(uop_c *cur_uop);
  void handle_page_faults();

private:
  void do_page_table_walks(uop_c *cur_uop, bool update);
  
  void begin_batch_processing();
  bool do_batch_processing();

  Addr get_page_number(Addr addr) { return addr >> m_offset_bits; }
  Addr get_page_offset(Addr addr) { return addr & (m_page_size - 1); }

  macsim_c* m_simBase;
  Counter m_cycle;

  map<Addr, PageDescriptor> m_page_table;
  long m_page_size;
  long m_memory_size;
  long m_offset_bits;
  long m_free_pages_remaining;
  vector<bool> m_free_pages;

  unique_ptr<TLB> m_TLB;
  unique_ptr<ReplacementUnit> m_replacement_unit;

  long m_walk_latency;
  long m_fault_latency;
  long m_eviction_latency;

  map<Counter, list<Addr> > m_walk_queue_cycle; // indexed by cycle
                                                // e.g., cycle t - page A, B, C
  map<Addr, list<uop_c*> > m_walk_queue_page;   // indexed by page number
                                                // e.g., page A - uop 1, 2
                                                // e.g., page B - uop 3, 4, 5, 6

  list<uop_c*> m_retry_queue;
  list<uop_c*> m_fault_retry_queue;

  long m_fault_buffer_size;
  set<Addr> m_fault_buffer;
  map<Addr, list<uop_c*>> m_fault_uops;
  list<Addr> m_fault_buffer_processing;
  map<Addr, list<uop_c*>> m_fault_uops_processing;
  
  bool m_batch_processing;
  bool m_batch_processing_first_transfer_started;
  long m_batch_processing_overhead;
  Counter m_batch_processing_start_cycle;
  Counter m_batch_processing_transfer_start_cycle;
  Counter m_batch_processing_next_event_cycle;
};

#endif //MMU_H_INCLUDED
