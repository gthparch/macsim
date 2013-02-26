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
 * File         : page_mapping.cc
 * Author       : HPArch Research Group
 * Date         : 2/25/2013
 * Description  : Virtual-to-Physical Page Mapping
 *********************************************************************************************/
#include <iostream>
#include <list>
#include "page_mapping.h"

/* macsim */
#include "statistics.h"
#include "debug_macros.h"
#include "assert_macros.h"

using namespace std;

//////////////////////////////////////////////////////////////////////
// First-Come-First-Serve Page Mapper
//////////////////////////////////////////////////////////////////////

// Constructor
FCFSPageMapper::FCFSPageMapper(macsim_c* simBase, uint32_t page_size)
    : PageMapper(simBase, page_size, 0)
{
  REPORT("## FCFS virtual to physical translation enabled");
}

// Destructor
FCFSPageMapper::~FCFSPageMapper() 
{
}

// This function provides a physical address (not ppn) for a given virtual address.
// If there does not exist mapping for the virtual address, a new ppn is allocated and we return it.
uint64_t FCFSPageMapper::translate(uint64_t virtual_address)
{
  // get virtual page number (vpn) and offset for the given virtual address
  uint64_t vpn    = virtual_address >> (int)log2(m_page_size);
  uint64_t offset = virtual_address & (m_page_size - 1);

  std::map<uint64_t, uint64_t>::iterator it;
  it = m_page_table.find(vpn);    // see if there is a matching entry

  if (it == m_page_table.end())
  {
    // did not find a matching entry. allocate a new ppn to this vpn
    uint64_t physical_address = (m_physical_tag << (int)log2(m_page_size)) | offset;
    m_page_table[vpn] = m_physical_tag++;

    // increment the number of pages allocated
    STAT_EVENT(NUM_PHYSICAL_PAGES);

    return physical_address;
  }
  else
  {
    // there already exists a PPN entry for the virtual address, so just use it
    return (it->second << (int)log2(m_page_size)) | offset;
  }
}

//////////////////////////////////////////////////////////////////////
// Region-Based FCFS Page Mapper
//////////////////////////////////////////////////////////////////////

// Constructor
RegionBasedFCFSPageMapper::RegionBasedFCFSPageMapper(
    macsim_c* simBase, uint32_t page_size, uint64_t region_size)
    : PageMapper(simBase, page_size, 0), m_region_size(region_size)
{
  REPORT("## Region-based FCFS virtual to physical translation enabled");
  ASSERTM(region_size >= page_size, "The page size should be smaller than the region size.\n");
}

// Destructor
RegionBasedFCFSPageMapper::~RegionBasedFCFSPageMapper() 
{
}

// This function provides a physical address (not ppn) for a given virtual address.
// If there does not exist mapping for the virtual address, a new ppn is allocated and we return it.
uint64_t RegionBasedFCFSPageMapper::translate(uint64_t virtual_address)
{
  // get virtual region and offset for the given virtual address
  uint64_t vpn    = virtual_address >> (int)log2(m_page_size);
  uint64_t region = virtual_address >> (int)log2(m_region_size);

  uint64_t offset = virtual_address & (m_region_size - 1); // region offset

  std::map<uint64_t, uint64_t>::iterator it;
  it = m_region_table.find(region);    // see if this region is touched before

  if (it == m_region_table.end())
  {
    // did not find a matching entry. allocate a new region to this vpn
    uint64_t physical_address = (m_physical_tag << (int)log2(m_region_size)) | offset;
    m_region_table[region] = m_physical_tag++;

    // maintain a separate page table for later use
    m_page_table[vpn] = physical_address >> (int)log2(m_page_size);

    // increment the number of pages allocated
    STAT_EVENT(NUM_PHYSICAL_PAGES);

    return physical_address;
  }
  else
  {
    // There already exists a physical region for this virtual region, so we can know the 
    // physical address. But, the corresponding physical address may not be new one in this region.
    // We maintain the page table for later use.
    uint64_t physical_address = (it->second << (int)log2(m_region_size)) | offset;

    it = m_page_table.find(vpn);

    if (it == m_page_table.end())
    {
      // put the ppn corresponding to the vpn of this virtual address
      m_page_table[vpn] = physical_address >> (int)log2(m_page_size);

      // increment the number of pages allocated
      STAT_EVENT(NUM_PHYSICAL_PAGES);
    }

    return physical_address;
  }
}
