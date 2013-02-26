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

