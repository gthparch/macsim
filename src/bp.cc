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
  bp_dir_base_c *new_bp = new bp_gshare_c(simBase);

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
  m_bp      = bp_factory_c::get()->allocate(simBase->m_knobs->KNOB_BP_DIR_MECH->getValue(), simBase);
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


