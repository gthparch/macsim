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
#include <unordered_map>
#include <vector>

#include "macsim.h"
#include "global_types.h"

using namespace std;

class TLB
{
private:
  struct PageDescriptor {
    Addr frame_number;
  };

  struct Entry {
    Addr page_number;
    PageDescriptor page_desc;
    Entry *prev;
    Entry *next;
  };

public:
  TLB(macsim_c *simBase, long _num_entries, long _page_size);
  ~TLB();

  bool lookup(Addr addr);
  void update(Addr addr);
  Addr translate(Addr addr);
  void insert(Addr addr, Addr frame_number);
  void invalidate(Addr page_number);

private:
  void detach(Entry *node) {
    node->prev->next = node->next;
    node->next->prev = node->prev;
  }

  void attach(Entry *node) {
    node->next = m_head->next;
    node->prev = m_head;
    m_head->next = node;
    node->next->prev = node;
  }

  long calc_log2(Addr val) {
    long n = 0;
    while ((val >>= 1)) n++;
    return n;
  }

  Addr get_page_number(Addr addr) {
    return addr >> m_offset_bits;
  }

private:
  unordered_map<long, Entry *> m_table;
  vector<Entry *> m_free_entries;
  Entry *m_head;
  Entry *m_tail;
  Entry *m_entries;

  long m_max_entries;
  long m_page_size;
  long m_offset_bits;

  macsim_c *m_simBase;
};

#endif  // TLB_H
