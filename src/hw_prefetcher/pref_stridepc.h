/***************************************************************************************
 * File         : pref_stridepc.h
 * Author       : Santhosh Srinath
 * Date         : 1/23/2005
 * CVS          : $Id: pref_stridepc.h,v 1.1 2008/07/30 14:18:16 kacear Exp $:
 * Description  : Stride Prefetcher - Based on load's PC address
 ***************************************************************************************/
#ifndef __PREF_STRIDEPC_H__

#include "../pref_common.h"
#include "../pref.h"

typedef struct StridePC_Table_Entry_Struct { 
  bool trained;
  bool valid;

  Addr last_addr;
  Addr load_addr;
  Addr start_index;
  Addr pref_last_index;
  int  stride;
  int tid;

  Counter train_num;
  Counter pref_sent;
  Counter last_access; // for lru
} StridePC_Table_Entry;


class pref_stridepc_c : public pref_base_c
{
  private:
    StridePC_Table_Entry  * stride_table;
    pref_stridepc_c();	

  public:

    pref_stridepc_c(hwp_common_c *, Unit_Type, macsim_c* );	
    /*************************************************************/
    /* HWP Interface */
    void init_func(int);
    void done_func() {}
    void l1_miss_func(int, Addr, Addr, uop_c *);
    void l1_hit_func(int, Addr, Addr, uop_c *);
    void l1_pref_hit_func(int, Addr, Addr, uop_c *) {}
    void l2_miss_func(int tid, Addr lineAddr, Addr loadPC, uop_c *uop);
    void l2_hit_func(int tid, Addr lineAddr, Addr loadPC, uop_c *uop );
    void l2_pref_hit_func(int, Addr, Addr, uop_c *) {}


    void pref_stridepc_l2_train(int tid, Addr lineAddr, Addr loadPC, uop_c *uop, bool l2_hit);
    void set_core_id(int cid) {
      core_id = cid;
    }

};



/*************************************************************/
/* Misc functions */

#define __PREF_STRIDEPC_H__
#endif /*  __PREF_STRIDEPC_H__*/