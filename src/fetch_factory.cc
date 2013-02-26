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
 * File         : fetch_factory.cc
 * Author       : Jaekyu Lee
 * Date         : 1/27/2011 
 * SVN          : $Id: frontend.h 915 2009-11-20 19:13:07Z kacear $:
 * Description  : fetch factory
 *********************************************************************************************/


#include "assert_macros.h"
#include "fetch_factory.h"

#include "all_knobs.h"

// Singleton pointer to fetch_factory_c
fetch_factory_c *fetch_factory_c::instance = 0;


// fetch_factory_c constructor
fetch_factory_c::fetch_factory_c() : m_func(NULL)
{
}


// fetch_factory_c destructor
fetch_factory_c::~fetch_factory_c()
{
}


// Get fetch_factory_c singleton entry
// If it is not previously allocated, allocate it
fetch_factory_c *fetch_factory_c::get()
{
  if (fetch_factory_c::instance == NULL)
    fetch_factory_c::instance = new fetch_factory_c();

  return instance;
}


// Register fetch policy
void fetch_factory_c::register_class(string policy, \
    function<frontend_c * (FRONTEND_INTERFACE_PARAMS(), macsim_c*)> func)
{
  m_func_table[policy] = func;
}


// Based on the policy, return appropriate constructor
frontend_c *fetch_factory_c::allocate_frontend(FRONTEND_INTERFACE_PARAMS(), macsim_c* m_simBase)
{
  // check function table is empty
  if (m_func_table.empty())
    ASSERTM(0, "Fetch function not registered.\n");


  // bind one fetch policy
  // TDP: Is there a point of this guard other than 
  //      preventing multiple lookups?
  //      KNOBS shouldn't be variable at runtime, right?
  static bool fetch_factory_init = false;
  if (!fetch_factory_init) {
    fetch_factory_init = true;
    string policy = m_simBase->m_knobs->KNOB_FETCH_POLICY->getValue(); 

    ASSERTM(m_func_table.find(policy) != m_func_table.end(), "Fetch function not found\n");
    m_func = m_func_table[policy]; 
  }


  // return a frontend object
  return m_func(FRONTEND_INTERFACE_ARGS(), m_simBase);
}
