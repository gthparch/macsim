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
 * File         : cache.h
 * Author       : HPArch 
 * Date         : 03/03/2010 
 * SVN          : $Id: cache.h,
 * Description  : cache structure (based on cache_lib at scarab) 
 *********************************************************************************************/

#ifndef CACHE_H
#define CACHE_H


#include <string>

#include "macsim.h"
#include "global_types.h" 
#include "global_defs.h"

typedef enum Cache_Type_enum 
{
  CACHE_IL1 = 1,
  CACHE_DL1,
  CACHE_IL2,
  CACHE_DL2,
  CACHE_DL3, 
  CACHE_CONST,
  CACHE_TEXTURE,
  CACHE_SW_MANAGED,
  CACHE_BTB
} Cache_Type;

/* set data pointers to this initially */
#define INIT_CACHE_DATA_VALUE ((void *)0x8badbeef) 


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Cache entry class
///////////////////////////////////////////////////////////////////////////////////////////////
class cache_entry_c 
{
  public:
    bool          m_valid;        //!< valid bit for the line 
    Addr          m_tag;          //!< tag for the line 
    Addr          m_base;         //!< address of first element 
    Counter       m_last_access_time; //!< for replacement policy 
    Counter       m_access_counter; //!< access counter 
    void         *m_data;         //!< poiter to arbitrary data 
    bool          m_pref;         //!< data is brought by a prefetcher 
    bool          m_dirty;        //!< data is dirty 
    int           m_appl_id;      //!< application id
    bool          m_gpuline;      //!< gpu cache line
    bool          m_skip; //!< skip LLC
    friend class  cache_c; 

    cache_entry_c();
};


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Cache set class
///////////////////////////////////////////////////////////////////////////////////////////////
class cache_set_c
{
  friend class cache_c;

  public:
    /**
     * Constructor
     */
    cache_set_c(int assoc);

    /**
     * Destructor
     */
    ~cache_set_c();

  public:
    cache_entry_c* m_entry; /**< cache entries */
    int m_assoc; /**< associativity */
    int m_num_cpu_line; /**< number of cpu cache line */
    int m_num_gpu_line; /**< number of gpu cache line */
    
  private:
    cache_set_c();
    
};


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Cache library class
///////////////////////////////////////////////////////////////////////////////////////////////
class cache_c 
{
  public:
    /**
     *  \brief Create a new cache using the configuration sent by the caller.
     *  \param name - Name of the cache
     *  \param num_set - Cache Size
     *  \param assoc - Cache Associativity
     *  \param line_size - Line Size
     *  \param data_size - Data Size
     *  \param bank_num - Bank Asoociated
     *  \param cache_by_pass- Cache by pass flag 
     *  \param core_id - Core id
     *  \param cache_type_info - Cache Type
     *  \param enable_partition - Enable cache partitioning
     *  \param simBase - Pointer to base simulation class for perf/stat counters
     *  \return void (Called with new operator)
     */
    cache_c(string name, int num_set, int assoc, int line_size, int data_size, int bank_num, 
        bool cache_by_pass, int core_id, Cache_Type cache_type_info, bool enable_partition,
        macsim_c* simBase); 

    virtual ~cache_c();

    /**
     *  \brief Function to assign a core id
     *  \param c_id - Core id
     *  \return void 
     */
    void set_core_id(int c_id) 
    { 
      m_core_id = c_id; 
    } 

    /**
     *  \brief Function to find tag and set from a given address.
     *  \param addr - Address
     *  \param tag - Tag extracted from the address(updated by the function)
     *  \param set - set associated to the address(updated by the function)
     *  \return void 
     */
    void find_tag_and_set(Addr addr, Addr *tag, int *set);

    /**
     *  \brief Cache look-up based on address.
     *  \param addr - Address
     *  \param line_addr - Base cache line 
     *  \param update_repl - update lru counter
     *  \param appl_id - application id to access cache
     *  \return void* - Pointer to the cache line data (if found) 
     */
    void* access_cache (Addr addr, Addr *line_addr, bool update_repl, int appl_id);

    /**
     * Update a cache line on access
     */
    virtual void update_cache_on_access(Addr tag, int set, int appl_id);

    /**
     * Update LRU value on cache hit
     */
    virtual void update_line_on_hit(cache_entry_c* line, int set, int appl_id);

    /**
     * Update cache on misses - set dueling
     */
    virtual void update_cache_on_miss(int set_id, int appl_id);

    /**
     * Update set on replacement
     */
    virtual void update_set_on_replacement(Addr tag, int appl_id, int set_id, bool gpuline);

    /**
     * \brief Funtion to find line where a new insert could be performed.
     * \param set - Cache set under consideration
     * \param appl_id - application id
     * \return cache_entry_c - find replaceable entry in set 
     */
    virtual cache_entry_c* find_replacement_line(int set, int appl_id);

    /**
     * Find replace-line from the same type
     * @param set - set id
     * @param appl_id - application id
     * @param gpuline - gpu cache line
     */
    cache_entry_c* find_replacement_line_from_same_type(int set, int appl_id, bool gpuline);

    /**
     * \brief Funtion to initialize a new cache line.
     * \param ins_line - Line to be inserted
     * \param tag - Tag based on the address
     * \param addr - Address
     * \param appl_id - application id
     * \param gpuline - gpu cache line
     * \param set_id - set id
     * \param skip - skip LLC
     * \return void 
     */
    virtual void initialize_cache_line(cache_entry_c *ins_line, Addr tag, Addr addr, 
        int appl_id, bool gpuline, int set_id, bool skip);

    /**
     * \brief Function to insert cache line(wrapper around worker_insert_cache)
               To be used only after access_cache returned NULL. 
               Assumes no prefetching 
     * \param addr - Address
     * \param line_addr - Base line address (Updated by the function)
     * \param repl_line replaced line address
     * \param appl_id application id
     * \param gpuline line from gpu cores
     * \return void* - Pointer to the data of the new cache line
     */
    void * insert_cache (Addr addr, Addr *line_addr, Addr *repl_line, int appl_id, 
        bool gpuline);
    
    /**
     * \brief Function to insert cache line(wrapper around worker_insert_cache)
               To be used only after access_cache returned NULL. 
               Assumes no prefetching 
     * \param addr - Address
     * \param line_addr - Base line address (Updated by the function)
     * \param repl_line replaced line address
     * \param appl_id application id
     * \param gpuline line from gpu cores
     * \param skip 
     * \return void* - Pointer to the data of the new cache line
     */
    void * insert_cache (Addr addr, Addr *line_addr, Addr *repl_line, int appl_id, 
        bool gpuline, bool skip);

    /**
     * \brief Function to null out all fields in the caache line 
        being invalidated 
     * \param line - Cache line being invalidated
     * \return bool - Dirty flag
     */
    bool null_cache_line_fields(cache_entry_c *line);

    /**
     * \brief Function to invalidate cache line. 
     * \param addr - Address
     * \return bool - indicate if the cache line being deleted was dirty
     */
    bool invalidate_cache_line (Addr addr);

    /**
     * \brief Function to return base cache line. 
     * \param addr - Address
     * \return Addr - Base cache line
     */
    Addr base_cache_line (Addr addr);

    /**
     * \brief Function to null out cache fields. 
     * \return void
     */
    void invalidate_cache(void);

    /**
     * \brief Function to get bank number for the addr. 
     * \param addr - Address
     * \return int - Bank Number
     */
    int get_bank_num(Addr addr);

    /**
     * \brief Function to find the line with least access time in the set. 
     * \param set - Set
     * \return int - lower of (Least access time in the set or current cycle) 
     */
    Counter find_min_lru (int set);

    /**
     * Print out cache information
     * @param id - cache id
     */
    void print_info(int id);

  public:
    Cache_Type m_cache_type; /**< cache type */

  protected:
    string  m_name;              /**< cache name */ 
    int     m_data_size;         /**< cache data size */
    int     m_assoc;             /**< associativity */                
    int     m_num_sets;          /**< number of sets */             
    int     m_line_size;         /**< cache line size */            
    int     m_set_bits;          /**< cache set bits */    
    int     m_shift_bits;        /**< cache shift mask */
    Addr    m_set_mask;          /**< cache set mask */
    Addr    m_tag_mask;          /**< cache tag mask */
    Addr    m_offset_mask;       /**< cache offset mask */
    int     m_bank_num;          /**< number of banks */             
    bool    m_perfect;           /**< Enable perfect cache */              
    int     m_core_id;           /**< core id */
    bool    m_cache_by_pass;     /**< bypass (disable) cache */ 
    int     m_num_cpu_line;      /**< number of cpu cache lines */
    int     m_num_gpu_line;      /**< number of gpu cache lines */ 
    Counter m_insert_count;      /**< total cache line insertion */
    bool    m_enable_partition;  /**< Enable cache partition */
    
    cache_set_c** m_set;       /**< cache data structure */

    macsim_c* m_simBase; /**< macsim_c base class for simulation globals */
};

#endif // CACHE_H



