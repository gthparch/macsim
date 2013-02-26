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
 * File         : page_mapping.h
 * Author       : HPArch Research Group
 * Date         : 2/25/2013
 * Description  : Virtual-to-Physical Page Mapping
 *********************************************************************************************/
#ifndef PAGE_MAPPING_H_INCLUDED
#define PAGE_MAPPING_H_INCLUDED

#include <cmath>
#include <map>

#include "macsim.h"

class PageMapper
{
  public:
    virtual ~PageMapper() {}
    virtual uint64_t translate(uint64_t virtual_address) = 0;
    uint64_t getNumPhysicalPages() { return m_page_table.size(); }  //!< get the number of physical pages allocated

  protected:
    macsim_c* m_simBase;                            //!< macsim base class for simulation globals
    std::map<uint64_t, uint64_t> m_page_table;      //!< page table (virtual page && physical page)
    uint32_t m_page_size;                           //!< page size (4KB default)
    uint64_t m_physical_tag;                        //!< physical page/region number allocated to a new virtual page/region

    PageMapper(macsim_c* simBase, uint32_t page_size, uint64_t physical_tag)
      : m_simBase(simBase), m_page_size(page_size), m_physical_tag(physical_tag) {} 
};

///////////////////////////////////////////////////////////////////////////////
//! First-come-first-serve virtual-to-physical mapping
///////////////////////////////////////////////////////////////////////////////
class FCFSPageMapper : public PageMapper
{
  public:
    FCFSPageMapper(macsim_c* simBase, uint32_t page_size = 4096);  //!< default page size is 4KB
    ~FCFSPageMapper();

  public:
    uint64_t translate(uint64_t virtual_address);   //!< provide physical translation for virtual address
};

///////////////////////////////////////////////////////////////////////////////
//! Region-Based FCFS Page Mapper
//! Physical pages are allocated sequentially within a region
//! However, the order is not sequential across regions and is based on FCFS
///////////////////////////////////////////////////////////////////////////////
class RegionBasedFCFSPageMapper : public PageMapper
{
  public:
    RegionBasedFCFSPageMapper(macsim_c* simBase, uint32_t page_size, uint64_t region_size);
    ~RegionBasedFCFSPageMapper();

  public:
    uint64_t getNumPhysicalRegions() { return m_region_table.size(); }  //!< get the number of physical regions allocated
    uint64_t translate(uint64_t virtual_address);   //!< provide physical translation for virtual address

  private:
    std::map<uint64_t, uint64_t> m_region_table;    //!< region mapping table
    uint32_t m_region_size;                         //!< region size 
};

#endif //!PAGE_MAPPING_H_INCLUDED
