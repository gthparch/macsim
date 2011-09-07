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

