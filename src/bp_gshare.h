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
 * File         : bp_dir_mech.h 
 * Author       : Hyesoon Kim 
 * Date         : 10/27/2008
 * SVN          : $Id: schedule_io.h,v 1.1 2008-04-01 04:44:48 hyesoon Exp $:
 * Description  : branch predictor 
                  imported from scarab 
 *********************************************************************************************/

#ifndef BP_GSHARE_H_INCLUDED 
#define BP_GSHARE_H_INCLUDED 


#include "global_defs.h"
#include "bp.h"


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Gshare branch predictor
///
/// Gshare branch predictor class
///////////////////////////////////////////////////////////////////////////////////////////////
class bp_gshare_c : public bp_dir_base_c 
{
  public:
    /**
     * Gshare BP constructor
     */
    bp_gshare_c(macsim_c* simBase);

    /**
     * Gshare BP destructor
     */
    ~bp_gshare_c(void) {} 

    /**
     * Predict a branch instruction
     */
    uns8 pred(uop_c *uop);
    
    /**
     * Update branch predictor when a branch instruction is resolved.
     */
    void update(uop_c *uop);
    
    /**
     * Called to recover the bp when a misprediction is realized 
     */
    void recover(recovery_info_c* recovery_info); 

  private:
    /**
     * Private constructor
     * Do not implement
     */
    bp_gshare_c(const bp_gshare_c& rhs);

    /**
     * Overridden operator =
     */
    const bp_gshare_c& operator=(const bp_gshare_c& rhs);
};

#endif // BP_GSHARE_H_INCLUDED 

