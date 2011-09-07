/**********************************************************************************************
 * File         : fetch_factory.h
 * Author       : Jaekyu Lee
 * Date         : 1/27/2011 
 * SVN          : $Id: frontend.h 915 2009-11-20 19:13:07Z kacear $:
 * Description  : fetch factory
 *********************************************************************************************/

#ifndef FETCH_FACTORY_H
#define FETCH_FACTORY_H


#include <string>
#include <functional>

#include "macsim.h"
#include "global_defs.h"
#include "global_types.h"

#include "frontend_interface.h"


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief frontend fetch policy factory
///
/// Thread fetch policy class factory
///////////////////////////////////////////////////////////////////////////////////////////////
class fetch_factory_c
{
  public:
    /**
     * Fetch factory class constructor
     */
    fetch_factory_c();

    /**
     * Fetch factory class destructor
     */
    ~fetch_factory_c();

    /**
     * Register a fetch class
     */
    void register_class(string, function<frontend_c * (FRONTEND_INTERFACE_PARAMS(), macsim_c*)>);

    /**
     * Allocate a new fetch class
     */
    frontend_c *allocate_frontend(FRONTEND_INTERFACE_PARAMS(), macsim_c* simBase);

    /**
     * Get a fetch factory object
     */
    static fetch_factory_c *get();

    /**
     * Function to create a new object
     */
    function<frontend_c * (FRONTEND_INTERFACE_PARAMS(), macsim_c*)> m_func;
    
  public:
    static fetch_factory_c *instance; /**< singleton fetch factory object */

  private:
    /** fetch wrapper function table */
    map<string, function<frontend_c * (FRONTEND_INTERFACE_PARAMS(), macsim_c*)> > m_func_table;
};


#endif
