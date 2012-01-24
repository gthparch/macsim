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


#ifndef TRACE_READER_H
#define TRACE_READER_H

#include "trace_read.h"
#include "global_types.h"
#include <vector>
#include <unordered_map>


extern all_knobs_c* g_knobs;


class trace_reader_c
{
  public:
    trace_reader_c();
    virtual ~trace_reader_c();
    virtual void inst_event(trace_info_s* inst);
    virtual void print();
    virtual void reset();

    void init();
    static trace_reader_c Singleton;

  protected:
    std::vector<trace_reader_c*> m_tracer;
    std::string m_name;
};


class reuse_distance_c : public trace_reader_c
{
  public:
    reuse_distance_c();
    ~reuse_distance_c();
    void inst_event(trace_info_s* inst);
    void print();
    void reset();

  private:
    int m_self_counter;
    std::unordered_map<Addr, int> m_reuse_map;
    std::unordered_map<Addr, int> m_reuse_pc_map;
    std::map<Addr, bool> m_static_pc;
    std::unordered_map<Addr, std::unordered_map<Addr, std::unordered_map<int, bool> *> *> m_result;
};


class static_pc_c : public trace_reader_c
{
  public:
    static_pc_c();
    ~static_pc_c();
    void inst_event(trace_info_s* inst);


  private:
    std::unordered_map<Addr, bool> m_static_pc;
    std::unordered_map<Addr, bool> m_static_mem_pc;
    uint64_t m_total_inst_count;
    uint64_t m_total_store_count;
};


#endif
