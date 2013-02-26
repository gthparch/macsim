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
 * File         : bp.h 
 * Author       : Hyesoon Kim
 * Date         : 9/8/2008 
 * SVN          : $Id: bp.h 867 2009-11-05 02:28:12Z kacear $:
 * Description  : branch predictor structure 
 *********************************************************************************************/

#ifndef BP_H_INCLUDED
#define BP_H_INCLUDED 


#include <string>
#include <functional>
#include <unordered_map>

#include "macsim.h"
#include "global_types.h"
#include "global_defs.h"
#include "bp_targ.h"

///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Branch recovery information class
///
/// recover: when a branch is mispredicted
/// redirect: when a front-end is not fetching correct instructions such as btb miss
///////////////////////////////////////////////////////////////////////////////////////////////
class bp_recovery_info_c 
{
  public:
    /**
     * Mispredicted branch recovery information Class constructor
     */
    bp_recovery_info_c();

    /**
     * Mispredicted branch recovery information Class destructor
     */
    ~bp_recovery_info_c();

  public:
    Counter          m_recovery_cycle; /**< cycle that begins misprediction recovery */
    Addr             m_recovery_fetch_addr; /**< address to redirect the istream */
    Counter          m_recovery_op_num; /**< op_num of op that caused recovery */
    Counter          m_recovery_cf_type; /**< cf_type of op that caused recovery */
    recovery_info_c* m_recovery_info; /**< information about the op causing the recovery */
    Counter          m_redirect_cycle; /**< cycle that begins a redirection (eg. btb miss) */
    Addr             m_redirect_fetch_addr; /**< address to redirect to */
    Counter          m_redirect_op_num; /**< op_num of op that caused redirect */
};


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Branch predictor base class
///
/// Branch predictor base Class. Other branch predictors need to inherit this base class. 
///////////////////////////////////////////////////////////////////////////////////////////////
class bp_dir_base_c 
{
  public:
    /**
     * Branch predictor class constructor 
     */
    bp_dir_base_c(macsim_c* simBase);

    /**
     * Branch predictor class destructor
     */
    ~bp_dir_base_c(); 

    /**
     * Predict a branch instruction
     */
    virtual uns8 pred (uop_c *uop) = 0; 

    /**
     * Update branch predictor when a branch instruction is resolved.
     */
    virtual void update (uop_c *uop) = 0; 
    
    /**
     * Called to recover the bp when a misprediction is realized 
     */
    virtual void recover (recovery_info_c *) = 0; 

  public:
    uns8       *m_pht; /**< branch history table */
    uns64       m_global_hist_64; /**< global branch history (64-bit) */
    uns32       m_global_hist; /**< global branch history (32-bit) */
    const char *m_name; /**< branch predictor name */

  protected:
    macsim_c* m_simBase;         /**< macsim_c base class for simulation globals */

  private:
    /**
     * Private constructor
     * Do not implement
     */
    bp_dir_base_c (const bp_dir_base_c& rhs);

    /**
     * Overridden operator =
     */
    bp_dir_base_c& operator=(const bp_dir_base_c& rhs);
}; 


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Branch predictor wrapper Class
///
/// Branch predictor wrapper Class. Based on the policy, different branch predictor
/// will be allocated in this Class.
///////////////////////////////////////////////////////////////////////////////////////////////
class bp_data_c
{
  public:
    /**
     * Branch predictor class constructor
     */
    bp_data_c(int core_id, macsim_c* simBase);

    /**
     * Branch predictor class destructor
     */
    ~bp_data_c(void);

    int                m_core_id; /**< core id */
    bp_dir_base_c     *m_bp; /**< branch predictor */
    bp_targ_c         *m_bp_targ_pred;   /**< BTB */ 
    
    unordered_map<int, Counter>  m_bp_recovery_cycle; /**< bp recovery cycle per thread */
    unordered_map<int, Counter>  m_bp_redirect_cycle; /**< bp recovery cycle per thread */
    unordered_map<int, Counter>  m_bp_cause_op; /**< misprediction caused uop per thread */

    
    // FIXME : implement BTB
    // cache btb; 

  private:
    /**
     * Branch predictor class private constructor
     */
    bp_data_c();
};

#endif

