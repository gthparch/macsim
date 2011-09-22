/**********************************************************************************************
 * File         : factory_class.h
 * Author       : Jaekyu Lee
 * Date         : 1/27/2011 
 * SVN          : $Id: frontend.h 915 2009-11-20 19:13:07Z kacear $:
 * Description  : factory class
 *********************************************************************************************/

#ifndef FACTORY_CLASS_H
#define FACTORY_CLASS_H


#include <unordered_map>
#include <functional>

#include "assert.h"
#include "macsim.h"
#include "global_defs.h"


///////////////////////////////////////////////////////////////////////////////////////////////
/// Few factory classes share exactly same structure. However, since all these factories have
/// singleton pattern, they need separate classes. In this file, define common codes, and 
/// replace with name and type
///
///
/// ComponentMACSIM inherently cannot use singleton, therefore, while this macro is still
/// general, it is no longer a singleton implementation
///////////////////////////////////////////////////////////////////////////////////////////////
#define FACTORY_DECLARE(name, type) \
  class name \
  { \
    public: \
      name(); \
      ~name(); \
      void register_class(string policy, function<type (macsim_c*)> func); \
      type allocate(string policy, macsim_c*); \
      static name* get(); \
    \
    public: \
      static name* instance; \
    \
    private: \
      unordered_map<string, function<type (macsim_c*)> > m_func_table; \
  };



// declare factories
FACTORY_DECLARE(dram_factory_c, dram_controller_c*);
FACTORY_DECLARE(bp_factory_c, bp_dir_base_c*);
FACTORY_DECLARE(mem_factory_c, memory_c*);
FACTORY_DECLARE(llc_factory_c, cache_c*);


// declare wrapper functions
bp_dir_base_c *default_bp(macsim_c*);
memory_c *default_mem(macsim_c*);
cache_c *default_llc(macsim_c*);


#endif
