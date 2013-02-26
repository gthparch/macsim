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
    void register_class(function<void (vector<pref_base_c *> &, hwp_common_c *, Unit_Type, 
          macsim_c*)>);

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
    list<function<void (vector<pref_base_c *>&, hwp_common_c*, Unit_Type, macsim_c*)> > m_func_table;
};


#endif
