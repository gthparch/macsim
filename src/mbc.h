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
 * File         : mbc.h
 * Author       : HPArch Research Group
 * Date         : 11/1/2020
 * Description  : Memory Bounds Check 
 *********************************************************************************************/

#ifndef MBC_H_INCLUDED
#define MBC_H_INCLUDED

#include <cstdlib>
#include <cmath>
#include <map>
#include <unordered_map>
#include <memory>
#include <utility>
#include <tuple>
#include <set>
#include <unordered_set>

#include "macsim.h"
#include "uop.h"
#include "tlb.h"
#include "cache.h"

class bounds_info_s
{
    public:
        Addr id; 
        Addr min_addr; 
        Addr max_addr; 
};

class mbc_c // Bounds Checking Unit 
{

public:
mbc_c(int core_id, macsim_c *simBase);
~mbc_c(void);


void finalize();

bool bounds_checking(uop_c *cur_uop);
bool bounds_insert(int core_id, Addr id, Addr min_addr, Addr max_addr); 
// static void bounds_info_read(string file_name_base);
// static void bounds_info_read(string file_name_base, set<int> &m_bounds_info);
static void bounds_info_read(string file_name_base, unordered_map<int,int> &m_bounds_info);
// static bool bounds_info_check_signed(int src1, set <int> &m_bounds_info); 
static bool bounds_info_check_signed(int src1, unordered_map <int, int> &m_bounds_info, int &reg_id); 
// need to have one for igpu as well 

private: 

int m_core_id; 
macsim_c * m_simBase; 

unordered_map<Addr, Addr> m_rbt; 
// static set<int> m_bounds_info; 
// vector<bounds_info_s *>m_free_entries; 

// std::unique_ptr<bounds_info_s> rbt_l0_cache; 
// std::unique_ptr<bounds_info_s> rbt_l1_cache; 
cache_c *m_l0_cache; 
port_c *m_l0_port; 
cache_c *m_l1_cache; 
port_c *m_l1_port; 

};
#endif //MBC_H_INCLUDED 