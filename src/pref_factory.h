/**********************************************************************************************
 * File         : pref_factory.h
 * Author       : Jaekyu Lee
 * Date         : 1/27/2011 
 * SVN          : $Id: frontend.h 915 2009-11-20 19:13:07Z kacear $:
 * Description  : prefetcher factory
 *********************************************************************************************/

#ifndef PREF_FACTORY_H
#define PREF_FACTORY_H


#include <string>
#include <functional>
#include <list>
#include <vector>

#include "global_types.h"
#include "global_defs.h"


// wrapper function to allocate prefetcher objects
void pref_factory(vector<pref_base_c *> &, hwp_common_c *, Unit_Type, macsim_c*);


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief pref policy factory
/// Hardware prefetcher factory class : allocate prefetchers based on the configuration
///////////////////////////////////////////////////////////////////////////////////////////////
class pref_factory_c
{
  public:
    /**
     * Constructor
     */
    pref_factory_c();

    /**
     * Destructor
     */
    ~pref_factory_c();

    /**
     * Register a new hardware prefetcher class
     */
    void register_class(function<void (vector<pref_base_c *> &, hwp_common_c *, Unit_Type, macsim_c*)>);

    /**
     * Allocate hardware prefetchers which are registerd
     */
    void allocate_pref(vector<pref_base_c *> &, hwp_common_c *, Unit_Type, macsim_c*);

    /**
     * Get singleton prefetch factory object
     */
    static pref_factory_c *get();
    
  public:
    static pref_factory_c *instance; /**< Singleton factory */

  private:
    /** wrapper function table */
    list<function<void (vector<pref_base_c *> &, hwp_common_c *, Unit_Type, macsim_c*)> > m_func_table;
};


#endif
