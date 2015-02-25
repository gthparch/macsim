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
* File         : dyfr.h
* Author       : HPArch
* Date         : 23/2/2015
* SVN          : 
* Description  : dynamic frequency feature
*********************************************************************************************/

#include "macsim.h"

#ifndef DYFR_C
#define DYFR_C

class dyfr_c {
  public:
    /**
     * Constructor
     */
    dyfr_c(macsim_c* simBase, int num_sim_cores);

    /**
     * Destructor 
     */
    ~dyfr_c();

    /**
     * Reset monitors 
     */
    void reset();

    /**
     * Monitors function update 
     */
    void monitor_function_example(int core_id);

    /**
     * Monitors value print
     */
    void print();

    /**
     * Main dynamic frequency function
     * to update frequencies based on
     * monitors
     */
    void update();

  private:
    dyfr_c();


  private:
    macsim_c* m_simBase;
    int m_num_sim_cores;

    // Monitors Variables
    

    // Frequencies
    int m_gpu_freq_min;
    int m_gpu_freq_max;
    int m_cpu_freq_min;
    int m_cpu_freq_max;

    // Power Budgets
    int m_cpu_power;
    int m_gpu_power;
    int m_mem_power;

    int m_cpu_appl;  /**< number of cpu applications*/
};


#endif
