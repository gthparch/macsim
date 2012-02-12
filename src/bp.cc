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
 * File         : bp.cc
 * Author       : Hyesoon Kim 
 * Date         : 1/26/2008
 * CVS          : $Id: bp.cc 867 2009-11-05 02:28:12Z kacear $:
 * Description  : branch predictor base class
 *********************************************************************************************/


/*
 * Summary: Branch predictor framework
 * based on the policy, different branch predictor will be enabled.
 */


#include "bp.h"
#include "bp_targ.h"
#include "bp_gshare.h"
#include "uop.h"
#include "factory_class.h"
#include "assert.h"

#include "all_knobs.h"

// register base branch predictor
bp_dir_base_c *default_bp(macsim_c* simBase) 
{
  string bp_type = simBase->m_knobs->KNOB_BP_DIR_MECH->getValue();
  bp_dir_base_c* new_bp;
  if (bp_type == "gshare")
    new_bp = new bp_gshare_c(simBase);
  else
    assert(0);

  return new_bp;
}



// bp_recovery_info_c constructor
bp_recovery_info_c::bp_recovery_info_c()
{
  m_recovery_info = new recovery_info_c;
}


// bp_recovery_info_c destructor
bp_recovery_info_c::~bp_recovery_info_c()
{
}


///////////////////////////////////////////////////////////////////////////////////////////////


// bp_data_c constructor
bp_data_c::bp_data_c(int core_id, macsim_c* simBase) 
{
  m_core_id = core_id;
  string bp_type = simBase->m_knobs->KNOB_BP_DIR_MECH->getValue();
  m_bp      = bp_factory_c::get()->allocate(bp_type, simBase);
  m_bp_targ_pred = new bp_targ_c(m_core_id, simBase); 
}


// bp_data_c destructor
bp_data_c::~bp_data_c()
{
}


///////////////////////////////////////////////////////////////////////////////////////////////


// bp_dir_base_c constructor
bp_dir_base_c::bp_dir_base_c(macsim_c* simBase) 
{
  m_simBase        = simBase;
  m_global_hist    = 0;
  m_global_hist_64 = 0; 
}


// bp_dir_base_c destructor
bp_dir_base_c::~bp_dir_base_c()
{
}


