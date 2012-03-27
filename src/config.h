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
 * File         : config.h 
 * Author       : HPArch
 * Date         : 3/9/2011
 * SVN          : $Id: dram.h 912 2009-11-20 19:09:21Z kacear $
 * Description  : Macros for configurations (to avoid dirty codes in core files)
 *********************************************************************************************/


#define CREATE_CACHE_CONFIGURATION() \
  { \
    if (type == UNIT_SMALL) { \
      if (level == MEM_L1) { \
        m_num_set   = *m_simBase->m_knobs->KNOB_L1_SMALL_NUM_SET; \
        m_assoc     = *m_simBase->m_knobs->KNOB_L1_SMALL_ASSOC; \
        m_line_size = *m_simBase->m_knobs->KNOB_L1_SMALL_LINE_SIZE; \
        m_banks     = *m_simBase->m_knobs->KNOB_L1_SMALL_NUM_BANK; \
        m_latency   = *m_simBase->m_knobs->KNOB_L1_SMALL_LATENCY; \
        m_bypass    = *m_simBase->m_knobs->KNOB_L1_SMALL_BYPASS; \
        m_ptx_sim   =  m_simBase->m_knobs->KNOB_CORE_TYPE->getValue() == "ptx" ? true : false; \
        m_num_read_port = *KNOB(KNOB_L1_READ_PORTS); \
        m_num_write_port = *KNOB(KNOB_L1_WRITE_PORTS); \
      } \
      else if (level == MEM_L2) { \
        m_num_set   = *m_simBase->m_knobs->KNOB_L2_SMALL_NUM_SET; \
        m_assoc     = *m_simBase->m_knobs->KNOB_L2_SMALL_ASSOC; \
        m_line_size = *m_simBase->m_knobs->KNOB_L2_SMALL_LINE_SIZE; \
        m_banks     = *m_simBase->m_knobs->KNOB_L2_SMALL_NUM_BANK; \
        m_latency   = *m_simBase->m_knobs->KNOB_L2_SMALL_LATENCY; \
        m_bypass    = *m_simBase->m_knobs->KNOB_L2_SMALL_BYPASS; \
        m_ptx_sim   =  m_simBase->m_knobs->KNOB_CORE_TYPE->getValue() == "ptx" ? true : false; \
        m_num_read_port = *KNOB(KNOB_L2_READ_PORTS); \
        m_num_write_port = *KNOB(KNOB_L2_WRITE_PORTS); \
      } \
      else if (m_level == MEM_L3) { \
        m_num_set   = *m_simBase->m_knobs->KNOB_L3_NUM_SET; \
        m_assoc     = *m_simBase->m_knobs->KNOB_L3_ASSOC; \
        m_line_size = *m_simBase->m_knobs->KNOB_L3_LINE_SIZE; \
        m_banks     = *m_simBase->m_knobs->KNOB_L3_NUM_BANK; \
        m_latency   = *m_simBase->m_knobs->KNOB_L3_LATENCY; \
        m_ptx_sim   =  m_simBase->m_knobs->KNOB_CORE_TYPE->getValue() == "ptx" ? true : false; \
        m_num_read_port = *KNOB(KNOB_L3_READ_PORTS); \
        m_num_write_port = *KNOB(KNOB_L3_WRITE_PORTS); \
      } \
    } \
    else if (type == UNIT_MEDIUM) { \
      if (level == MEM_L1) { \
        m_num_set   = *m_simBase->m_knobs->KNOB_L1_MEDIUM_NUM_SET; \
        m_assoc     = *m_simBase->m_knobs->KNOB_L1_MEDIUM_ASSOC; \
        m_line_size = *m_simBase->m_knobs->KNOB_L1_MEDIUM_LINE_SIZE; \
        m_banks     = *m_simBase->m_knobs->KNOB_L1_MEDIUM_NUM_BANK; \
        m_latency   = *m_simBase->m_knobs->KNOB_L1_MEDIUM_LATENCY; \
        m_bypass    = *m_simBase->m_knobs->KNOB_L1_MEDIUM_BYPASS; \
        m_ptx_sim   =  m_simBase->m_knobs->KNOB_MEDIUM_CORE_TYPE->getValue() == "ptx" ? true : false; \
        m_num_read_port = *KNOB(KNOB_L1_READ_PORTS); \
        m_num_write_port = *KNOB(KNOB_L1_WRITE_PORTS); \
      } \
      else if (level == MEM_L2) { \
        m_num_set   = *m_simBase->m_knobs->KNOB_L2_MEDIUM_NUM_SET; \
        m_assoc     = *m_simBase->m_knobs->KNOB_L2_MEDIUM_ASSOC; \
        m_line_size = *m_simBase->m_knobs->KNOB_L2_MEDIUM_LINE_SIZE; \
        m_banks     = *m_simBase->m_knobs->KNOB_L2_MEDIUM_NUM_BANK; \
        m_latency   = *m_simBase->m_knobs->KNOB_L2_MEDIUM_LATENCY; \
        m_bypass    = *m_simBase->m_knobs->KNOB_L2_MEDIUM_BYPASS; \
        m_ptx_sim   =  m_simBase->m_knobs->KNOB_MEDIUM_CORE_TYPE->getValue() == "ptx" ? true : false; \
        m_num_read_port = *KNOB(KNOB_L2_READ_PORTS); \
        m_num_write_port = *KNOB(KNOB_L2_WRITE_PORTS); \
      } \
      else if (level == MEM_L3) { \
        m_num_set   = *m_simBase->m_knobs->KNOB_L3_NUM_SET; \
        m_assoc     = *m_simBase->m_knobs->KNOB_L3_ASSOC; \
        m_line_size = *m_simBase->m_knobs->KNOB_L3_LINE_SIZE; \
        m_banks     = *m_simBase->m_knobs->KNOB_L3_NUM_BANK; \
        m_latency   = *m_simBase->m_knobs->KNOB_L3_LATENCY; \
        m_ptx_sim   =  m_simBase->m_knobs->KNOB_MEDIUM_CORE_TYPE->getValue() == "ptx" ? true : false; \
        m_num_read_port = *KNOB(KNOB_L3_READ_PORTS); \
        m_num_write_port = *KNOB(KNOB_L3_WRITE_PORTS); \
      } \
    } \
    else if (type == UNIT_LARGE) { \
      if (level == MEM_L1) { \
        m_num_set   = *m_simBase->m_knobs->KNOB_L1_LARGE_NUM_SET; \
        m_assoc     = *m_simBase->m_knobs->KNOB_L1_LARGE_ASSOC; \
        m_line_size = *m_simBase->m_knobs->KNOB_L1_LARGE_LINE_SIZE; \
        m_banks     = *m_simBase->m_knobs->KNOB_L1_LARGE_NUM_BANK; \
        m_latency   = *m_simBase->m_knobs->KNOB_L1_LARGE_LATENCY; \
        m_bypass    = *m_simBase->m_knobs->KNOB_L1_LARGE_BYPASS; \
        m_ptx_sim   =  m_simBase->m_knobs->KNOB_LARGE_CORE_TYPE->getValue() == "ptx" ? true : false; \
        m_num_read_port = *KNOB(KNOB_L1_READ_PORTS); \
        m_num_write_port = *KNOB(KNOB_L1_WRITE_PORTS); \
      } \
      else if (level == MEM_L2) { \
        m_num_set   = *m_simBase->m_knobs->KNOB_L2_LARGE_NUM_SET; \
        m_assoc     = *m_simBase->m_knobs->KNOB_L2_LARGE_ASSOC; \
        m_line_size = *m_simBase->m_knobs->KNOB_L2_LARGE_LINE_SIZE; \
        m_banks     = *m_simBase->m_knobs->KNOB_L2_LARGE_NUM_BANK; \
        m_latency   = *m_simBase->m_knobs->KNOB_L2_LARGE_LATENCY; \
        m_bypass    = *m_simBase->m_knobs->KNOB_L2_LARGE_BYPASS; \
        m_ptx_sim   =  m_simBase->m_knobs->KNOB_LARGE_CORE_TYPE->getValue() == "ptx" ? true : false; \
        m_num_read_port = *KNOB(KNOB_L2_READ_PORTS); \
        m_num_write_port = *KNOB(KNOB_L2_WRITE_PORTS); \
      } \
      else if (level == MEM_L3) { \
        m_num_set   = *m_simBase->m_knobs->KNOB_L3_NUM_SET; \
        m_assoc     = *m_simBase->m_knobs->KNOB_L3_ASSOC; \
        m_line_size = *m_simBase->m_knobs->KNOB_L3_LINE_SIZE; \
        m_banks     = *m_simBase->m_knobs->KNOB_L3_NUM_BANK; \
        m_latency   = *m_simBase->m_knobs->KNOB_L3_LATENCY; \
        m_ptx_sim   =  m_simBase->m_knobs->KNOB_LARGE_CORE_TYPE->getValue() == "ptx" ? true : false; \
        m_num_read_port = *KNOB(KNOB_L3_READ_PORTS); \
        m_num_write_port = *KNOB(KNOB_L3_WRITE_PORTS); \
      } \
    } \
  } 


#define ROB_CONFIG() \
  uns16 m_knob_meu_nsb = 0; \
  uns16 m_knob_meu_nlb = 0; \
  switch (m_unit_type) {\
    case UNIT_SMALL: \
      m_knob_rob_size = *m_simBase->m_knobs->KNOB_ROB_SIZE; \
      m_knob_meu_nsb  = *m_simBase->m_knobs->KNOB_MEU_NSB; \
      m_knob_meu_nlb  = *m_simBase->m_knobs->KNOB_MEU_NLB; \
      break; \
    case UNIT_MEDIUM: \
      m_knob_rob_size = *m_simBase->m_knobs->KNOB_ROB_MEDIUM_SIZE; \
      m_knob_meu_nsb  = *m_simBase->m_knobs->KNOB_MEU_MEDIUM_NSB; \
      m_knob_meu_nlb  = *m_simBase->m_knobs->KNOB_MEU_MEDIUM_NLB; \
      break; \
    case UNIT_LARGE: \
      m_knob_rob_size = *m_simBase->m_knobs->KNOB_ROB_LARGE_SIZE; \
      m_knob_meu_nsb  = *m_simBase->m_knobs->KNOB_MEU_LARGE_NSB; \
      m_knob_meu_nlb  = *m_simBase->m_knobs->KNOB_MEU_LARGE_NLB; \
      break; \
  } 


#define RETIRE_CONFIG() \
  switch (m_unit_type) { \
    case UNIT_SMALL: \
      m_knob_width   = *m_simBase->m_knobs->KNOB_WIDTH; \
      m_knob_ptx_sim = \
        static_cast<string>(*m_simBase->m_knobs->KNOB_CORE_TYPE) == "ptx" ? true : false; \
      break; \
    case UNIT_MEDIUM: \
      m_knob_width   = *m_simBase->m_knobs->KNOB_MEDIUM_WIDTH; \
      m_knob_ptx_sim = \
        static_cast<string>(*m_simBase->m_knobs->KNOB_MEDIUM_CORE_TYPE) == "ptx" ? true : false; \
      break; \
    case UNIT_LARGE: \
      m_knob_width   = *m_simBase->m_knobs->KNOB_LARGE_WIDTH; \
      m_knob_ptx_sim = \
        static_cast<string>(*m_simBase->m_knobs->KNOB_LARGE_CORE_TYPE) == "ptx" ? true : false; \
      break; \
  }


#define EXEC_CONFIG() \
  int int_sched_rate = 0; \
  int mem_sched_rate = 0; \
  int fp_sched_rate  = 0; \
  switch (m_unit_type) { \
    case UNIT_SMALL: \
      int_sched_rate  = *m_simBase->m_knobs->KNOB_ISCHED_RATE; \
      mem_sched_rate  = *m_simBase->m_knobs->KNOB_MSCHED_RATE; \
      fp_sched_rate   = *m_simBase->m_knobs->KNOB_FSCHED_RATE; \
      m_dcache_cycles = *m_simBase->m_knobs->KNOB_L1_SMALL_LATENCY; \
      m_ptx_sim       = \
        static_cast<string>(*m_simBase->m_knobs->KNOB_CORE_TYPE) == "ptx" ? true : false; \
      break; \
    \
    case UNIT_MEDIUM: \
      int_sched_rate  = *m_simBase->m_knobs->KNOB_ISCHED_MEDIUM_RATE; \
      mem_sched_rate  = *m_simBase->m_knobs->KNOB_MSCHED_MEDIUM_RATE; \
      fp_sched_rate   = *m_simBase->m_knobs->KNOB_FSCHED_MEDIUM_RATE; \
      m_dcache_cycles = *m_simBase->m_knobs->KNOB_L1_MEDIUM_LATENCY; \
      m_ptx_sim       = \
        static_cast<string>(*m_simBase->m_knobs->KNOB_MEDIUM_CORE_TYPE) == "ptx" ? true : false; \
      break; \
    \
    case UNIT_LARGE: \
      int_sched_rate = *m_simBase->m_knobs->KNOB_ISCHED_LARGE_RATE; \
      mem_sched_rate = *m_simBase->m_knobs->KNOB_MSCHED_LARGE_RATE; \
      fp_sched_rate  = *m_simBase->m_knobs->KNOB_FSCHED_LARGE_RATE; \
      m_dcache_cycles  = *m_simBase->m_knobs->KNOB_L1_LARGE_LATENCY; \
      m_ptx_sim        = \
        static_cast<string>(*m_simBase->m_knobs->KNOB_LARGE_CORE_TYPE) == "ptx" ? true : false; \
      break; \
  } \
  m_max_port[gen_ALLOCQ] = int_sched_rate; \
  m_max_port[mem_ALLOCQ] = mem_sched_rate; \
  m_max_port[fp_ALLOCQ]  = fp_sched_rate;
  

#define SCHED_CONFIG() \
  uns16 int_sched_size = 0; \
  uns16 mem_sched_size = 0; \
  uns16 fp_sched_size  = 0; \
  uns16 int_sched_rate = 0; \
  uns16 mem_sched_rate = 0; \
  uns16 fp_sched_rate  = 0; \
  \
  switch (m_unit_type) { \
    case UNIT_SMALL: \
      m_knob_width          = *m_simBase->m_knobs->KNOB_WIDTH; \
      int_sched_size        = *m_simBase->m_knobs->KNOB_ISCHED_SIZE; \
      mem_sched_size        = *m_simBase->m_knobs->KNOB_MSCHED_SIZE; \
      fp_sched_size         = *m_simBase->m_knobs->KNOB_FSCHED_SIZE; \
      int_sched_rate        = *m_simBase->m_knobs->KNOB_ISCHED_RATE; \
      mem_sched_rate        = *m_simBase->m_knobs->KNOB_MSCHED_RATE; \
      fp_sched_rate         = *m_simBase->m_knobs->KNOB_FSCHED_RATE; \
      m_knob_sched_to_width = *m_simBase->m_knobs->KNOB_SCHED_TO_WIDTH; \
      break; \
    \
    case UNIT_MEDIUM: \
      m_knob_width          = *m_simBase->m_knobs->KNOB_MEDIUM_WIDTH; \
      int_sched_size        = *m_simBase->m_knobs->KNOB_ISCHED_MEDIUM_SIZE; \
      mem_sched_size        = *m_simBase->m_knobs->KNOB_MSCHED_MEDIUM_SIZE; \
      fp_sched_size         = *m_simBase->m_knobs->KNOB_FSCHED_MEDIUM_SIZE; \
      int_sched_rate        = *m_simBase->m_knobs->KNOB_ISCHED_MEDIUM_RATE; \
      mem_sched_rate        = *m_simBase->m_knobs->KNOB_MSCHED_MEDIUM_RATE; \
      fp_sched_rate         = *m_simBase->m_knobs->KNOB_FSCHED_MEDIUM_RATE; \
      m_knob_sched_to_width = *m_simBase->m_knobs->KNOB_SCHED_TO_MEDIUM_WIDTH; \
      break; \
    \
    case UNIT_LARGE: \
      m_knob_width          = *m_simBase->m_knobs->KNOB_LARGE_WIDTH; \
      int_sched_size        = *m_simBase->m_knobs->KNOB_ISCHED_LARGE_SIZE; \
      mem_sched_size        = *m_simBase->m_knobs->KNOB_MSCHED_LARGE_SIZE; \
      fp_sched_size         = *m_simBase->m_knobs->KNOB_FSCHED_LARGE_SIZE; \
      int_sched_rate        = *m_simBase->m_knobs->KNOB_ISCHED_LARGE_RATE; \
      mem_sched_rate        = *m_simBase->m_knobs->KNOB_MSCHED_LARGE_RATE; \
      fp_sched_rate         = *m_simBase->m_knobs->KNOB_FSCHED_LARGE_RATE; \
      m_knob_sched_to_width = *m_simBase->m_knobs->KNOB_SCHED_TO_LARGE_WIDTH; \
      break; \
  } \
  \
  m_sched_size[gen_ALLOCQ] = int_sched_size; \
  m_sched_size[mem_ALLOCQ] = mem_sched_size; \
  m_sched_size[fp_ALLOCQ]  = fp_sched_size; \
  m_sched_rate[gen_ALLOCQ] = int_sched_rate; \
  m_sched_rate[mem_ALLOCQ] = mem_sched_rate; \
  m_sched_rate[fp_ALLOCQ]  = fp_sched_rate; \
  \
  m_num_per_sched[gen_ALLOCQ] = 0; \
  m_num_per_sched[mem_ALLOCQ] = 0; \
  m_num_per_sched[fp_ALLOCQ]  = 0;


#define FRONTEND_CONFIG() \
  switch (m_unit_type) { \
    case UNIT_SMALL: \
      m_knob_width            = *m_simBase->m_knobs->KNOB_WIDTH; \
      m_knob_fetch_width      = *m_simBase->m_knobs->KNOB_FETCH_WDITH; \
      m_knob_icache_line_size = *m_simBase->m_knobs->KNOB_ICACHE_LINE_SIZE; \
      m_fetch_modulo          = *m_simBase->m_knobs->KNOB_GPU_FETCH_RATIO - 1; \
      if (static_cast<string>(*m_simBase->m_knobs->KNOB_CORE_TYPE) == "ptx") { \
        m_knob_ptx_sim = true; \
        m_fetch_ratio = *m_simBase->m_knobs->KNOB_GPU_FETCH_RATIO; \
      } \
      else { \
        m_knob_ptx_sim = false; \
        m_fetch_ratio = *m_simBase->m_knobs->KNOB_CPU_FETCH_RATIO; \
      } \
      break; \
    \
    case UNIT_MEDIUM: \
      m_knob_width            = *m_simBase->m_knobs->KNOB_MEDIUM_WIDTH; \
      m_knob_fetch_width      = *m_simBase->m_knobs->KNOB_FETCH_MEDIUM_WDITH; \
      m_knob_icache_line_size = *m_simBase->m_knobs->KNOB_ICACHE_MEDIUM_LINE_SIZE; \
      m_fetch_modulo          = *m_simBase->m_knobs->KNOB_GPU_FETCH_RATIO - 1; \
      if (static_cast<string>(*m_simBase->m_knobs->KNOB_MEDIUM_CORE_TYPE) == "ptx") { \
        m_knob_ptx_sim = true; \
        m_fetch_ratio = *m_simBase->m_knobs->KNOB_GPU_FETCH_RATIO; \
      } \
      else { \
        m_knob_ptx_sim = false; \
        m_fetch_ratio = *m_simBase->m_knobs->KNOB_CPU_FETCH_RATIO; \
      } \
      break; \
    \
    case UNIT_LARGE: \
      m_knob_width            = *m_simBase->m_knobs->KNOB_LARGE_WIDTH; \
      m_knob_fetch_width      = *m_simBase->m_knobs->KNOB_FETCH_LARGE_WDITH; \
      m_knob_icache_line_size = *m_simBase->m_knobs->KNOB_ICACHE_LARGE_LINE_SIZE; \
      m_fetch_modulo          = *m_simBase->m_knobs->KNOB_GPU_FETCH_RATIO - 1; \
      if (static_cast<string>(*m_simBase->m_knobs->KNOB_LARGE_CORE_TYPE) == "ptx") { \
        m_knob_ptx_sim = true; \
        m_fetch_ratio = *m_simBase->m_knobs->KNOB_GPU_FETCH_RATIO; \
      } \
      else { \
        m_knob_ptx_sim = false; \
        m_fetch_ratio = *m_simBase->m_knobs->KNOB_CPU_FETCH_RATIO; \
      } \
      break; \
  }


#define CORE_CONFIG() \
  uns icache_size; \
  uns icache_assoc; \
  uns icache_line_size; \
  uns icache_banks; \
  int icache_bypass; \
  uns giaq_size; \
  uns miaq_size; \
  uns fq_size; \
  string m_knob_schedule = ""; \
  uns m_knob_fetch_latency; \
  uns m_knob_alloc_latency; \
  switch (type) { \
    case UNIT_SMALL: \
      icache_size            = *m_simBase->m_knobs->KNOB_ICACHE_NUM_SET; \
      icache_assoc           = *m_simBase->m_knobs->KNOB_ICACHE_ASSOC; \
      icache_line_size       = *m_simBase->m_knobs->KNOB_ICACHE_LINE_SIZE; \
      icache_banks           = *m_simBase->m_knobs->KNOB_ICACHE_BANKS; \
      icache_bypass          = *m_simBase->m_knobs->KNOB_ICACHE_BY_PASS; \
      giaq_size              = *m_simBase->m_knobs->KNOB_GIAQ_SIZE; \
      miaq_size              = *m_simBase->m_knobs->KNOB_MIAQ_SIZE; \
      fq_size                = *m_simBase->m_knobs->KNOB_FQ_SIZE; \
      m_max_threads_per_core = *m_simBase->m_knobs->KNOB_MAX_THREADS_PER_CORE; \
      m_core_type            = static_cast<string>(*m_simBase->m_knobs->KNOB_CORE_TYPE); \
      m_knob_schedule        = static_cast<string>(*m_simBase->m_knobs->KNOB_SCHEDULE); \
      m_knob_fetch_latency   = *m_simBase->m_knobs->KNOB_FETCH_LATENCY; \
      m_knob_alloc_latency   = *m_simBase->m_knobs->KNOB_ALLOC_LATENCY; \
      m_knob_enable_pref     = *m_simBase->m_knobs->KNOB_ENABLE_PREF_SMALL_CORE; \
    break; \
    \
    case UNIT_MEDIUM: \
      icache_size            = *m_simBase->m_knobs->KNOB_ICACHE_MEDIUM_NUM_SET; \
      icache_assoc           = *m_simBase->m_knobs->KNOB_ICACHE_MEDIUM_ASSOC; \
      icache_line_size       = *m_simBase->m_knobs->KNOB_ICACHE_MEDIUM_LINE_SIZE; \
      icache_banks           = *m_simBase->m_knobs->KNOB_ICACHE_MEDIUM_BANKS; \
      icache_bypass          = *m_simBase->m_knobs->KNOB_ICACHE_MEDIUM_BY_PASS; \
      giaq_size              = *m_simBase->m_knobs->KNOB_GIAQ_MEDIUM_SIZE; \
      miaq_size              = *m_simBase->m_knobs->KNOB_MIAQ_MEDIUM_SIZE; \
      fq_size                = *m_simBase->m_knobs->KNOB_FQ_MEDIUM_SIZE; \
      m_max_threads_per_core = *m_simBase->m_knobs->KNOB_MAX_THREADS_PER_MEDIUM_CORE; \
      m_core_type            = static_cast<string>(*m_simBase->m_knobs->KNOB_MEDIUM_CORE_TYPE); \
      m_knob_schedule        = static_cast<string>(*m_simBase->m_knobs->KNOB_MEDIUM_CORE_SCHEDULE); \
      m_knob_fetch_latency   = *m_simBase->m_knobs->KNOB_MEDIUM_CORE_FETCH_LATENCY; \
      m_knob_alloc_latency   = *m_simBase->m_knobs->KNOB_MEDIUM_CORE_ALLOC_LATENCY; \
      m_knob_enable_pref     = *m_simBase->m_knobs->KNOB_ENABLE_PREF_MEDIUM_CORE; \
      break; \
    \
    case UNIT_LARGE: \
      icache_size            = *m_simBase->m_knobs->KNOB_ICACHE_LARGE_NUM_SET; \
      icache_assoc           = *m_simBase->m_knobs->KNOB_ICACHE_LARGE_ASSOC; \
      icache_line_size       = *m_simBase->m_knobs->KNOB_ICACHE_LARGE_LINE_SIZE; \
      icache_banks           = *m_simBase->m_knobs->KNOB_ICACHE_LARGE_BANKS; \
      icache_bypass          = *m_simBase->m_knobs->KNOB_ICACHE_LARGE_BY_PASS; \
      giaq_size              = *m_simBase->m_knobs->KNOB_GIAQ_LARGE_SIZE; \
      miaq_size              = *m_simBase->m_knobs->KNOB_MIAQ_LARGE_SIZE; \
      fq_size                = *m_simBase->m_knobs->KNOB_FQ_LARGE_SIZE; \
      m_max_threads_per_core = *m_simBase->m_knobs->KNOB_MAX_THREADS_PER_LARGE_CORE; \
      m_core_type            = static_cast<string>(*m_simBase->m_knobs->KNOB_LARGE_CORE_TYPE); \
      m_knob_schedule        = static_cast<string>(*m_simBase->m_knobs->KNOB_LARGE_CORE_SCHEDULE); \
      m_knob_fetch_latency   = *m_simBase->m_knobs->KNOB_LARGE_CORE_FETCH_LATENCY; \
      m_knob_alloc_latency   = *m_simBase->m_knobs->KNOB_LARGE_CORE_ALLOC_LATENCY; \
      m_knob_enable_pref     = *m_simBase->m_knobs->KNOB_ENABLE_PREF_LARGE_CORE; \
      break; \
    \
    default : \
      ASSERT(0); \
  }
