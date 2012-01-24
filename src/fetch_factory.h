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
#include "frontend.h"


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
