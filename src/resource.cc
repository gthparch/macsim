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
 * File         : resource.cc
 * Author       : Hyesoon Kim 
 * Date         : 1/1/2016
 * Description  : resource structure 
 *********************************************************************************************/

#include "assert.h"
#include "config.h"
#include "global_types.h"
#include "knob.h"
#include "resource.h"
#include "uop.h"
#include "debug_macros.h"

#include "all_knobs.h"


resource_c::resource_c(Unit_Type type, macsim_c* simBase)
{
	m_simBase = simBase;
	
	m_unit_type = type; 
	
	RESOURCE_CONFIG(); 

	// load and store buffers
  m_max_sb_cnt = m_knob_meu_nsb;
  m_num_sb     = m_knob_meu_nsb;
	
  m_max_lb_cnt = m_knob_meu_nlb;
  m_num_lb     = m_knob_meu_nlb;
	
  // int and fp registers
  m_max_int_regs = *m_simBase->m_knobs->KNOB_INT_REGFILE_SIZE;
  m_num_int_regs = *m_simBase->m_knobs->KNOB_INT_REGFILE_SIZE;
	
  m_max_fp_regs = *m_simBase->m_knobs->KNOB_FP_REGFILE_SIZE;
  m_num_fp_regs = *m_simBase->m_knobs->KNOB_FP_REGFILE_SIZE;
	
}

// resource_ clear :: reset 


void resource_c::reset()
{

  m_num_sb     = m_max_sb_cnt;
	
  m_num_lb     = m_max_lb_cnt; 
	
	m_num_int_regs = m_max_int_regs;
	m_num_fp_regs = m_max_fp_regs; 
}

// resource_c destructor 
resource_c::~resource_c()
{
	// do nothing 
}

