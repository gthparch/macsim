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


/***************************************************************************************
 * File         : bp_targ.h 
 * Author       : Hyesoon Kim 
 * Date         : 4/5/2010 
 * SVN          : $Id: bp.h 867 2009-11-05 02:28:12Z kacear $:
 * Description  : branch target predictor 
 ***************************************************************************************/

#ifndef BP_TARG_H_INCLUDED
#define BP_TARG_H_INCLUDED 


#include "macsim.h"
#include "global_types.h"
#include "global_defs.h"


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Branch Target Buffer 
/// BTB modeling 
///////////////////////////////////////////////////////////////////////////////////////////////
class bp_targ_c
{
   public:
     /**
      * Constructor
      */
     bp_targ_c(uns core_id, macsim_c* simBase); 

     /**
      * branch prediction
      */
     Addr pred (uop_c *uop); 

     /**
      * bp update after it is resolved
      */
     void update (uop_c *uop); 

   protected:
     macsim_c* m_simBase; /**< macsim_c base class for simulation globals */ 

   private:
     cache_c* btb; /**< BTB */
     uns m_core_id; /**< Core id */
};



#endif 
