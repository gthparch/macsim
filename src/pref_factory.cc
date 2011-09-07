/**********************************************************************************************
 * File         : pref_factory.cc
 * Author       : Jaekyu Lee
 * Date         : 1/27/2011 
 * SVN          : $Id: frontend.h 915 2009-11-20 19:13:07Z kacear $:
 * Description  : pref factory
 *********************************************************************************************/


#include "pref_factory.h"
#include "assert.h"
#include "pref_stride.h"


///////////////////////////////////////////////////////////////////////////////////////////////


void pref_factory(vector<pref_base_c *> &pref_table, hwp_common_c *hcc, 
                  Unit_Type type, macsim_c* simBase)
{
  pref_base_c *pref_stride = new pref_stride_c(hcc, type, simBase);
  pref_table.push_back(pref_stride);
} 


// Singleton pointer to pref_factory_c
pref_factory_c *pref_factory_c::instance = 0;


///////////////////////////////////////////////////////////////////////////////////////////////


// pref_factory_c constructor
pref_factory_c::pref_factory_c()
{
}


// pref_factory_c destructor
pref_factory_c::~pref_factory_c()
{
}


// Get pref_factory_c singleton entry
// If it is not previously allocated, allocate it
pref_factory_c *pref_factory_c::get()
{
  if (pref_factory_c::instance == NULL)
    pref_factory_c::instance = new pref_factory_c;

  return instance;
}


// Register pref policy
void pref_factory_c::register_class(
    function<void (vector<pref_base_c *> &, hwp_common_c *, Unit_Type, macsim_c*)> func)
{
  m_func_table.push_back(func);
}


// Based on the policy, return appropriate constructor
void pref_factory_c::allocate_pref(
    vector<pref_base_c *> &pref_table, hwp_common_c *hcc, Unit_Type type, macsim_c* simBase)
{
  for (auto itr = m_func_table.begin(); itr != m_func_table.end(); ++itr) {
    (*itr)(pref_table, hcc, type, simBase);
  }
}

