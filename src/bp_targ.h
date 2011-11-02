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
/// \Branch Target Buffer 
/// BTB modeling 
///////////////////////////////////////////////////////////////////////////////////////////////


class bp_targ_c
{
   private:
     cache_c* btb;
     uns m_core_id;

   public:
     bp_targ_c(uns core_id, macsim_c* simBase); 
     Addr pred (uop_c *uop); 
     void  update (uop_c *uop); 

   protected:
     macsim_c* m_simBase; // ** macsim_c base class for simulation globals */ 
};



#endif 
