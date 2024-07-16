/***************************************************************************************
 * File         : pref_2dc.h
 * Author       : Santhosh Srinath
 * Date         : 11/16/2004
 * CVS          : $Id: pref_2dc.h,v 1.1 2008/07/30 14:18:15 kacear Exp $:
 * Description  : 
 ***************************************************************************************/
#ifndef __PREF_2DC_H__

//#include "dcu.h"
#include "../pref_common.h"
#include "../pref.h"



typedef enum   Pref_2DC_HashFunc_Enum {
  PREF_2DC_HASH_FUNC_DEFAULT,
}Pref_2DC_HashFunc;


typedef struct Pref_2DC_Cache_Data_struct {
  int delta;
}Pref_2DC_Cache_Data;


typedef struct Pref_2DC_Region_Struct {
  int                 deltaA, deltaB, deltaC;
}Pref_2DC_Region;


class pref_2dc_c : public pref_base_c
{
  private:
    // 2DC Cache
    cache_c               *cache;
    uns                 cache_index_bits;

    uns                 pref_degree;
    Addr                last_access;
    Addr                last_loadPC;
    Pref_2DC_HashFunc   hash_func;
    Pref_2DC_Region   * regions;

    pref_2dc_c();

  public:

    pref_2dc_c(hwp_common_c *, Unit_Type, macsim_c* simBase);
    ~pref_2dc_c();

    /*************************************************************/
    /* HWP Interface */
    void init_func(int);
    void done_func() {}
    void l1_miss_func(int, Addr, Addr, uop_c *) {}
    void l1_hit_func(int, Addr, Addr, uop_c *) {}
    void l1_pref_hit_func(int, Addr, Addr, uop_c *) {}
    void l2_miss_func(int, Addr, Addr, uop_c *);
    void l2_hit_func(int, Addr, Addr, uop_c *) {}
    void l2_pref_hit_func(int, Addr, Addr, uop_c *);

    void pref_2dc_l2_train(int, Addr, Addr, bool);


    /*************************************************************/
    /* Misc functions */
    void pref_2dc_throttle(void);
    Addr pref_2dc_hash(Addr lineIndex, Addr loadPC, int deltaA, int deltaB);
};

#define __PREF_2DC_H__
#endif /*  __PREF_2DC_H__*/