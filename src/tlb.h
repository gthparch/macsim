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
 * File         : tlb.h
 * Author       : HPArch Research Group
 * Date         : 1/30/2019
 * Description  : Translation Lookaside Buffer (TLB)
 *********************************************************************************************/

#ifndef TLB_H
#define TLB_H

#include <algorithm>
#include <assert.h>
#include <map>
#include <vector>

#include "global_types.h"

using namespace std;

class TLB
{
private:
  struct PageDescriptor
  {
    Addr frame_number;
  };

  struct Entry
  {
    Addr page_number;
    PageDescriptor page_desc;
    Entry *prev;
    Entry *next;
  };

public:
  TLB(long _num_entries, long _page_size) : 
    m_max_entries(_num_entries), m_page_size(_page_size)
  {
    m_entries = new Entry[m_max_entries];
    for (long i = 0; i < m_max_entries; ++i)
      m_free_entries.push_back(m_entries + i);

    m_head = new Entry;
    m_tail = new Entry;

    m_head->prev = NULL;
    m_head->next = m_tail;
    m_tail->next = NULL;
    m_tail->prev = m_head;

    m_offset_bits = calc_log2(m_page_size);
  }

  ~TLB()
  {
    delete m_head;
    delete m_tail;
    delete[] m_entries;
  }

  bool lookup(Addr addr)
  {
    Addr page_number = get_page_number(addr);
    auto table_iter = m_table.find(page_number);
    return (table_iter != m_table.end());
  }

  Addr translate(Addr addr)
  {
    Addr page_number = get_page_number(addr);
    auto table_iter = m_table.find(page_number);
    assert(table_iter != m_table.end());
    Entry *node = table_iter->second;
    detach(node);
    attach(node);
    return node->page_desc.frame_number;
  }

  void insert(Addr addr, Addr frame_number)
  {
    Addr page_number = get_page_number(addr);
    auto table_iter = m_table.find(page_number);
    assert(table_iter == m_table.end());
    
    // insert an entry into the MRU position
    if (!m_free_entries.empty()) {  // free entry available
                                    // insert a new entry into the MRU position
      Entry *node = m_free_entries.back();
      m_free_entries.pop_back();
      node->page_number = page_number;
      node->page_desc.frame_number = frame_number;
      m_table[page_number] = node;
      attach(node);
    } else {  // free entry not available
              // replace the entry in the LRU position
      Entry *node = m_tail->prev;
      detach(node);
      m_table.erase(node->page_number);
      node->page_number = page_number;
      node->page_desc.frame_number = frame_number;
      m_table[page_number] = node;
      attach(node);
    }
  }

  void invalidate(Addr page_number)
  {
    Entry *node = m_table[page_number];
    if (node) {
      detach(node);
      m_table.erase(node->page_number);
      m_free_entries.push_back(node);
    }
  }

private:
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

  long calc_log2(Addr val)
  {
    long n = 0;
    while ((val >>= 1))
      n++;
    return n;
  }

  Addr get_page_number(Addr addr)
  {
    return addr >> m_offset_bits; 
  }

private:
  map<long, Entry *> m_table;
  vector<Entry *> m_free_entries;
  Entry *m_head;
  Entry *m_tail;
  Entry *m_entries;

  long m_max_entries;
  long m_page_size;
  long m_offset_bits;
};

#endif // TLB_H
