/*
Copyright (c) <2015>, <Georgia Institute of Technology> All rights reserved.

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
* File         : dyfr.cc
* Author       : HPArch
* Date         : 23/2/2015
* SVN          : 
* Description  : dynamic frequency feature
*********************************************************************************************/

#include "dyfr.h"
#include "all_knobs.h"
#include "core.h"

#include <iostream>
#include <iomanip>
#include <cmath>
#include <cassert>

// =======================================
// dyfr_c constructor
// =======================================
dyfr_c::dyfr_c(macsim_c* simBase, int num_sim_cores)
  : m_simBase(simBase), m_num_sim_cores(num_sim_cores)
{
  m_gpu_freq_min = *KNOB(KNOB_DYFR_GPU_FREQ_MIN);
  m_gpu_freq_max = *KNOB(KNOB_DYFR_GPU_FREQ_MAX);
  m_cpu_freq_min = *KNOB(KNOB_DYFR_CPU_FREQ_MIN);
  m_cpu_freq_max = *KNOB(KNOB_DYFR_CPU_FREQ_MAX);

  m_cpu_power = *KNOB(KNOB_DYFR_CPU_BUDGET);
  m_gpu_power = *KNOB(KNOB_DYFR_GPU_BUDGET);
  m_mem_power = *KNOB(KNOB_DYFR_MEM_BUDGET);

  m_cpu_appl = *KNOB(KNOB_NUM_CPU_APPLICATION);

  // Initialize monitors and new variables here
}

// =======================================
// dyfr destructor
// =======================================
dyfr_c::~dyfr_c()
{

}

// =======================================
// reset monitors
// =======================================
void dyfr_c::reset(void)
{
  // This fucntion could be used to reset monitor variables
  // eg. fill_n(m_mpki_count, m_num_sim_cores, 0);
  return;
}


// =======================================
// monitors update
// =======================================
void dyfr_c::monitor_function_example(int core_id)
{
  // This function could be called on a particular event to update a variable
  return;
}


// =======================================
// dyfr print monitors
// =======================================
void dyfr_c::print(void)
{
  // This function could be used to print monitor variables values
  return;
}


// =======================================
// main dyfr function to update frequencies
// based on monitors
// =======================================
void dyfr_c::update(void)
{
  if (*KNOB(KNOB_ENABLE_DYFR) == false)
    return;

  // This function is call every KNOB_DYFR_SAMPLE_PERIOD to upadate 
  // dynamic frequencies of cores, l3, NoC and Memory Controllers
  //
  // You could define your own monitors variable in dyfr.h and update
  // them on any event. For this, you need to include dyfr.h in the file
  // you are going to add your monitor functions.
  //
  // With the aid of monitor variables, you could use these functions to
  // change any cores and other part of the system frequencies.
  // Frequencies in macsim are int and 10X. (1.5GHz == 15)
  //    * m_simBase->get_current_frequency_core(core_id);
  //          return the current frequency of a core 
  //    * m_simBase->get_current_frequency_uncore(type);
  //          return the current frequency of a unit
  //          types: 0: l3, 1: noc, 2: mc 
  //    * m_simBase->change_frequency_core(id, freq);
  //          change the frequecy of a core based on its id
  //    * m_simBase->change_frequency_uncore(type, freq);
  //          change the frequency of the whole unit based on it type
  //          types: 0: l3, 1: noc, 2: mc
  //
  // After changing the frequency, MacSim will insert new frequency conifgs
  // in a queue and with fixed latency will apply in to the system.
  // The latency is define in macsim.cc with LATENCY_APPLY_FREQUENCY. current 
  // latency is 1us.
  //
  // When a new frequency is applied to a core, the core get stalled for the period 
  // of KNOB_DYFR_PLL_LOCK.

  // ******************************************************************************************
  // basic configuration
  // ******************************************************************************************

  // ******************************************************************************************
  // GPU throttling
  // ******************************************************************************************

  // ******************************************************************************************
  // CPU throttling
  // ******************************************************************************************

  // ******************************************************************************************
  // Units throttling
  // ******************************************************************************************
  
}

