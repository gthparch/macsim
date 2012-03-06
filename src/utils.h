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
 * File         : utils.h
 * Author       : Hyesoon Kim + Jaekyu Lee 
 * Date         : 12/18/2007 
 * CVS          : $Id: utils.h,v 1.3 2008-04-10 00:54:07 hyesoon Exp $:
 * Description  : utilities 
                  based on utils.h at scarab 
                  other utility classes added
**********************************************************************************************/

#ifndef UTILS_H_INCLUDED
#define UTILS_H_INCLUDED 


#include <cstring>
#include <stdio.h>
#include <time.h>
#include <string>
#include <list>
#include <unordered_map>
#include <fstream>

#include "global_types.h"
#include "global_defs.h"
#include "core.h"
#include "uop.h"
#include "map.h"
#include "bp.h"


///////////////////////////////////////////////////////////////////////////////////////////////


// macro definitions
#define ToString(var, arg) \
  { \
    stringstream sstr; \
    sstr << arg; \
    sstr >> var; \
  }

//string ToString(int);

#define N_BIT_MASK(N)			((0x1ULL << (N)) - 1)
#define N_BIT_MASK_64			(0xffffffffffffffffULL)
#define N_BIT_MASK(N)                    ((0x1ULL << (N)) - 1)

#define MIN2(v0, v1)			(((v0) < (v1)) ? (v0) : (v1))
#define MAX2(v0, v1)			(((v0) > (v1)) ? (v0) : (v1))

#define BANK(a, num, int)		((a) >> log2_int(int) & N_BIT_MASK(log2_int(num)))

#define L(x) left << setw(x)


///////////////////////////////////////////////////////////////////////////////////////////////


// function declarations in utils.h. Please refer to utils.cc for the description
int get_next_set_bit64(uns64 val, uns pos);
int get_num_set_bits64(uns64 val);
const char *hexstr64(uns64 );
const char *hexstr64s(uns64 );
const char *unsstr64(uns64 );
const char *intstr64(int64 );
void  breakpoint(const char [], const int);
// get log value
uns log2_int (uns n);

FILE *file_tag_fopen(std::string , char const *const, macsim_c*);


///////////////////////////////////////////////////////////////////////////////////////////////


using namespace std;


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief pool class
///////////////////////////////////////////////////////////////////////////////////////////////
template <class T>
class pool_c
{
  public:
    /**
     * Constructor
     */
    pool_c()
    {
      m_pool            = new list<T*>;
      m_poolsize        = 0;
      m_poolexpand_unit = 1;
      m_name            = "none";
    }

    /**
     * Constructor
     * @param pool_expand_unit number of entries when pool expands
     * @param name pool name
     */
    pool_c(int pool_expand_unit, string name)
    {
      m_pool            = new list<T*>;
      m_poolsize        = 0;
      m_poolexpand_unit = pool_expand_unit;
      m_name            = name;
    }

    /**
     * Destructor
     */
    ~pool_c()
    {
      while (!m_pool->empty()) {
        T* entry = m_pool->front();
        m_pool->pop_front();
        delete entry;
      }
      delete m_pool;
    }

    /**
     * Acquire a new entry
     */
    T* acquire_entry(void)
    {
      if (m_pool->empty()) {
        expand_pool();
      }
      T* entry = m_pool->front();
      m_pool->pop_front();
      return entry;
    }
    
    /**
     * Acquire a new entry
     *   whose class requires simBase reference
     */
    T* acquire_entry(macsim_c* m_simBase) 
    {
      if (m_pool->empty()) {
        expand_pool(m_simBase);
      }
      T* entry = m_pool->front();
      m_pool->pop_front();
      return entry;
    }

    /**
     * Release a new entry
     */
    void release_entry(T* entry)
    {
      m_pool->push_front(entry);
    }

    /**
     * Expand the pool
     */
    void expand_pool(void)
    {
      for (int ii = 0; ii < m_poolexpand_unit; ++ii) {
        T* entries = new T;
        m_pool->push_back(entries);
      }
      m_poolsize += m_poolexpand_unit;
    }

   /**
     * Expand the pool
     *  whose class requires simBase reference
     */
    void expand_pool(macsim_c* m_simBase)
    {
      for (int ii = 0; ii < m_poolexpand_unit; ++ii) {
        T* entries = new T(m_simBase);
        m_pool->push_back(entries);
      }
      m_poolsize += m_poolexpand_unit;
    }

    /**
     * Return the size of a pool
     */
    int size(void)
    {
      return m_poolsize; 
    }

  private:
    list<T*>* m_pool; /**< pool */
    int       m_poolsize; /**< pool size */
    int       m_poolexpand_unit; /**< pool expand unit */
    string    m_name; /**< pool name */
};


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief hash table class
///////////////////////////////////////////////////////////////////////////////////////////////
template <typename T>
class hash_c
{
  public:
    /**
     * Constructor
     */
    hash_c() 
    {
      m_pool = new pool_c<T>(100, "hash");
    }

    /**
     * Constructor
     * @param name hash table name
     */
    hash_c(string name)
    {
      m_pool = new pool_c<T>(100, name);
    }

    /**
     * Constructor
     * @param pool use specified pool for entries
     */
    hash_c(pool_c<T>* pool)
    {
      m_pool = pool;
    }

    /**
     * Destructor
     */
    ~hash_c()
    {
      clear();
      delete m_pool;
    }

    /**
     * Access the hash table. If not found, create a new entry
     *   whose class requires simBase reference
     */
    T* hash_table_access_create(int64 key, bool* new_entry, macsim_c* simBase)
    {
      T* result;
      if (m_table.find(key) == m_table.end()) {
        result = m_pool->acquire_entry(simBase);
        m_table.insert(pair<int64, T*>(key, result));
        *new_entry = true;
      }
      else {
        result = m_table[key];
        *new_entry = false;
      }

      return result;
    }

   /**
     * Access the hash table. If not found, create a new entry
     */
    T* hash_table_access_create(int64 key, bool* new_entry)
    {
      T* result;
      if (m_table.find(key) == m_table.end()) {
        result = m_pool->acquire_entry();
        m_table.insert(pair<int64, T*>(key, result));
        *new_entry = true;
      }
      else {
        result = m_table[key];
        *new_entry = false;
      }

      return result;
    }

    /**
     * Access the hash table with the key
     */
    T* hash_table_access(int64 key)
    {
      if (m_table.find(key) == m_table.end()) {
        return NULL;
      }
      else {
        return m_table[key];
      }
    }

    /**
     * Delete a hash entry with the key
     */
    bool hash_table_access_delete(int64 key)
    {
      if (m_table.find(key) == m_table.end()) {
        return false;
      }
      else {
        T* entry = m_table[key];
        m_table.erase(key);
        m_pool->release_entry(entry);
        return true;
      }
    }

    /**
     * Return hash table size
     */
    int size(void)
    {
      return m_table.size();
    }

    /**
     * Clear hash table
     */
    void clear(void)
    {
      while (!m_table.empty()) {
        T* entry = (*m_table.begin()).second;
        m_pool->release_entry(entry);
        m_table.erase(m_table.begin());
      }
      m_table.clear();
    }

  private:
    unordered_map<int64, T*> m_table; /**< hash table */
    pool_c<T>* m_pool; /**< hash table entry pool */
};


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief hash with multiple keys
///
/// This class is used for remapping (application id, block id) to new unique block id.
/// We need this feature to repeat same traces.
///////////////////////////////////////////////////////////////////////////////////////////////
class multi_key_map_c 
{
  public:
    /**
     * Constructor
     */
    multi_key_map_c();

    /**
     * Destructor
     */
    ~multi_key_map_c();

    /**
     * Find an existing entry with two keys
     */
    int find(int key1, int key2);

    /**
     * Insert a new entry with key1 and key2
     */
    int insert(int key1, int key2);

    /**
     * Delete an entry with key1
     */
    void delete_table(int key1);

  private:
    unordered_map<int, unordered_map<int, int> *> m_table; /**< hash table */
    int m_size; /**< hash table size. to get unique id */
};


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief cp framework
///////////////////////////////////////////////////////////////////////////////////////////////
class cache_partition_framework_c
{
  public:
    /**
     * Constructor
     */
    cache_partition_framework_c(macsim_c* simBase);

    /**
     * Destructor
     */
    ~cache_partition_framework_c();
  
    /**
     * Run a cycle (tick)
     */
    void run_a_cycle();

    /** 
     * Get PSEL mask value
     */
    inline bool get_psel_mask(void)
    {
      return m_psel_mask[0];
    }

    /**
     * Get PSEL mask value with application id
     */
    inline bool get_psel_mask(int appl_id)
    {
      return m_psel_mask[appl_id];
    }

    /**
     * Get performance mask
     */
    inline bool get_performance_mask(void)
    {
      return m_performance_mask[0];
    }

    /**
     * Get performance mask with application id
     */
    inline bool get_performance_mask(int appl_id)
    {
      return m_performance_mask[appl_id];
    }

    /**
     * set application type
     */
    inline void set_appl_type(int appl_id, bool ptx)
    {
      m_appl_type[appl_id] = ptx;
    }

    /**
     * get application type
     */
    inline bool get_appl_type(int appl_id)
    {
      return m_appl_type[appl_id];
    }


  private:
    ofstream m_file; /**< output stream */
    bool m_psel_mask[10]; /**< PSEL mask */
    bool m_performance_mask[10]; /**< true: huge memory-intensity */
    bool m_appl_type[100]; /**< false: cpu, true: gpu */
    
    macsim_c* m_simBase;         /**< macsim_c base class for simulation globals */
};


#endif // UTILS_H_INCLUDED 
