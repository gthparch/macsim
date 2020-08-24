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
#define FACTORY_DECLARE(name, type)                                      \
  class name                                                             \
  {                                                                      \
  public:                                                                \
    name();                                                              \
    ~name();                                                             \
    void register_class(string policy, function<type(macsim_c *)> func); \
    type allocate(string policy, macsim_c *);                            \
    static name *get();                                                  \
                                                                         \
  public:                                                                \
    static name *instance;                                               \
                                                                         \
  private:                                                               \
    unordered_map<string, function<type(macsim_c *)> > m_func_table;     \
  };

// declare factories
FACTORY_DECLARE(dram_factory_c, dram_c *);
FACTORY_DECLARE(bp_factory_c, bp_dir_base_c *);
FACTORY_DECLARE(mem_factory_c, memory_c *);
FACTORY_DECLARE(llc_factory_c, cache_c *);
FACTORY_DECLARE(network_factory_c, network_c *);

// declare wrapper functions
bp_dir_base_c *default_bp(macsim_c *);
memory_c *default_mem(macsim_c *);
cache_c *default_llc(macsim_c *);
network_c *default_network(macsim_c *);

#endif
