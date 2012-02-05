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
 * File         : utils.cc
 * Author       : HPArch
 * Date         : 10/19/1997
 * CVS          : $Id: utils.cc,v 1.2 2008-04-04 22:58:50 gth672e Exp $:
 * Description  : Utility functions.
 *********************************************************************************************/


#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "global_defs.h"
#include "global_types.h"
#include "assert.h"
#include "utils.h"
#include "trace_read.h"
#include "debug_macros.h"
#include "zlib.h"
#include "core.h"

#include <sys/time.h>
#include <stdarg.h>

#include <iostream>
#include <fstream>
#include <iomanip>
#include "statistics.h"
#include "core.h"
#include "retire.h"

#include "all_knobs.h"

///////////////////////////////////////////////////////////////////////////////////////////////


stringstream util_sstr;
char str_buffer[1000][100];
int str_buffer_index = 0;


// breakpoint: A function to help debugging
void breakpoint (const char file [], const int line)
{
  ;
}


// return string from 64-bit unsigned (with leading zeros)
const char *hexstr64 (uns64 value)
{
  util_sstr.clear();
  util_sstr << hex << value;

  if (str_buffer_index == 1000)
    str_buffer_index = 0;
  util_sstr >> setfill('0') >> setw(16) >> str_buffer[str_buffer_index];

  return str_buffer[str_buffer_index++];
}


// return string from 64-bit unsigned (without leading zeros)
const char *hexstr64s (uns64 value)
{
  util_sstr.clear();
  util_sstr << hex << value;

  if (str_buffer_index == 1000)
    str_buffer_index = 0;
  util_sstr >> str_buffer[str_buffer_index];

  return str_buffer[str_buffer_index++];
}


// unsstr64:  Prints a 64-bit number in decimal format.
const char *unsstr64 (uns64 value)
{
  util_sstr.clear();
  util_sstr << dec << value;

  if (str_buffer_index == 1000)
    str_buffer_index = 0;
  util_sstr >> dec >> str_buffer[str_buffer_index];

  return str_buffer[str_buffer_index++];
}


// intstr64:  Prints a 64-bit number in decimal format.
const char *intstr64 (int64 value)
{
  util_sstr.clear();
  util_sstr << dec << value;

  if (str_buffer_index == 1000)
    str_buffer_index = 0;
  util_sstr >> dec >> str_buffer[str_buffer_index];

  return str_buffer[str_buffer_index++];
}


// get log value
uns log2_int (uns n)
{
  uns power;
  for (power = 0 ; n >>= 1 ; ++power);
  return power;
}


FILE *file_tag_fopen (std::string path, char const *const mode, macsim_c* m_simBase)
{
  FILE* file = NULL;
  string file_name;
  string file_tag = *KNOB(KNOB_FILE_TAG); 
  string output_dir = *KNOB(KNOB_STATISTICS_OUT_DIRECTORY); 
  if (!strcmp(file_tag.c_str(), "NULL")) {
    file_tag="";
  } 	
  stringstream sstr;
  sstr << output_dir << "/" << file_tag.c_str() << path.c_str() << ".out";
  sstr >> file_name;
  file = fopen(file_name.c_str(), mode);
  if(!file) {
    char* pErr = strerror(errno);
    file = NULL;
    pErr = pErr;
  }
  return file;
}


// get next set bit (out of 64-bit)
int get_next_set_bit64(uns64 val, uns pos)
{
  val >>= pos;
  for (int i = pos; i < 64; ++i) {
    if (val & 0x1ULL) {
      return i;
    }
    val >>= 1;
  }
  return -1;
}


// get number of set bits (out of 64-bit)
int get_num_set_bits64(uns64 val)
{
  int count  = 0;
  while (val > 0) {
    if (val & 1) {
      ++count;
    }
    val >>= 1;
  }
  return count;
}


///////////////////////////////////////////////////////////////////////////////////////////////


// constructor
multi_key_map_c::multi_key_map_c()
{
  m_size = 0;
}


// destructor
multi_key_map_c::~multi_key_map_c()
{
}


// find an entry with (key1, key2)
int multi_key_map_c::find(int key1, int key2)
{
  if (m_table.find(key1) == m_table.end()) {
    return -1;
  }

  unordered_map<int, int> *sub_table = m_table[key1];

  if (sub_table->find(key2) == sub_table->end()) {
    return -1;
  }
  else {
    return (*sub_table)[key2];
  }
}


// insert a new entry : return unique number
int multi_key_map_c::insert(int key1, int key2)
{
  if (m_table.find(key1) == m_table.end()) {
    unordered_map<int, int> *new_sub_table = new unordered_map<int, int>;
    (*new_sub_table)[key2] = m_size;
    m_table[key1] = new_sub_table;
  }
  else {
    unordered_map<int, int> *sub_table = m_table[key1];
    (*sub_table)[key2] = m_size;
  }
  ++m_size;
  return m_size - 1;
}


void multi_key_map_c::delete_table(int key1)
{
  unordered_map<int, int>* sub_table = (*m_table.find(key1)).second;
  sub_table->clear();
  delete sub_table;
}


cache_partition_framework_c::cache_partition_framework_c(macsim_c* simBase)
{
  m_simBase = simBase;
  for (int ii = 0; ii < 10; ++ii) {
    m_psel_mask[ii]        = false;
    m_performance_mask[ii] = false;
  }
}

cache_partition_framework_c::~cache_partition_framework_c()
{
}


void cache_partition_framework_c::run_a_cycle()
{
  if (*KNOB(KNOB_COLLECT_CPI_INFO) > 0 && 
      CYCLE % *KNOB(KNOB_COLLECT_CPI_INFO) == 0) {
    int max = 0;
    int min = 1000000000;
    int sum = 0;
    int count = 0;
    for (int ii = 0; ii < *KNOB(KNOB_NUM_SIM_CORES); ++ii) {
      core_c* core = m_simBase->m_core_pointers[ii];
      if (core->get_core_type() == "ptx") {
        retire_c* retire = core->get_retire();
        int inst_count = retire->get_periodic_inst_count();
        if (inst_count > max)
          max = inst_count;
        if (inst_count < min)
          min = inst_count;
        sum += inst_count;
        ++count;
      }
    }

    sum -= (min + max);
    int average = sum / (count - 2);

    if (average > 0) {
      float min_delta = (average - min) * 100.0 / average;
      float max_delta = (max - average) * 100.0 / average;
      float value = max_delta > min_delta ? max_delta : min_delta;
      if (value <= 5.0) {
        m_psel_mask[0] = false;
      }
      else {
        m_psel_mask[0] = true;
      }

      if (value >= 20.0) {
        m_performance_mask[0] = true;
      }
      else {
        m_performance_mask[0] = false;
      }
      STAT_EVENT(CPI_DELTA_BASE0);
      STAT_EVENT_N(CPI_DELTA0, static_cast<int>(value));
    }
  }


  if (*KNOB(KNOB_COLLECT_CPI_INFO_FOR_MULTI_GPU) > 0 
      && CYCLE % *KNOB(KNOB_COLLECT_CPI_INFO_FOR_MULTI_GPU) == 0) {
    int num_core = *KNOB(KNOB_NUM_SIM_CORES) / 2;
    for (int appl = 0; appl < 2; ++appl) {
      int max = 0;
      int min = 1000000000;
      int sum = 0;
      int count = 0;
      for (int ii = num_core * appl; ii < (appl + 1) * num_core; ++ii) {
        core_c* core = m_simBase->m_core_pointers[ii];
        if (core->get_core_type() == "ptx") {
          retire_c* retire = core->get_retire();
          int inst_count = retire->get_periodic_inst_count();
          if (inst_count > max)
            max = inst_count;
          if (inst_count < min)
            min = inst_count;
          sum += inst_count;
          ++count;
        }
      }

      int average = sum / count;

      if (average > 0) {
        float min_delta = (average - min) * 100.0 / average;
        float max_delta = 0;
        float value = max_delta > min_delta ? max_delta : min_delta;
        if (value <= 10.0) {
          m_psel_mask[appl] = false;
        }
        else {
          m_psel_mask[appl] = true;
        }

        if (value >= 20.0) {
          m_performance_mask[appl] = true;
        }
        else {
          m_performance_mask[appl] = false;
        }
        STAT_EVENT(CPI_DELTA_BASE0 + appl);
        STAT_EVENT_N(CPI_DELTA0 + appl, static_cast<int>(value));
      }
    }
  }
}
