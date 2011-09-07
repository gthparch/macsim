/**********************************************************************************************
 * File         : pqueue.h 
 * Author       : Jaekyu Lee
 * Date         : 2/14/2011
 * SVN          : $Id: dram.cc 912 2009-11-20 19:09:21Z kacear $
 * Description  : Priority Queue
 *********************************************************************************************/


/*
 * Summary: Model priority queue
 *
 *   Priority queue models latency + priority.
 *   There are N+1 (same as modeled latency) slots.
 *   Each slot is a list that may have several elements.
 *   Elemented are sorted by priority (highest priority is in the front)
 *   New entry is inserted in the back.
 *   Each cycle (if possible), each list is moved one slot ahead.
 *   Dequeue() will get highest priority + oldest entry from the first list.
 */



#ifndef PQUEUE_H_INCLUDED
#define PQUEUE_H_INCLUDED


#include <list>
#include <string>
#include <sys/types.h>
#include <typeinfo>

#include "macsim.h"
#include "utils.h"
#include "global_types.h"
#include "assert_macros.h"


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Priority queue class
///////////////////////////////////////////////////////////////////////////////////////////////
template <class T>
class pqueue_c
{
  /**
   * pqueue entry
   */
  typedef struct pqueue_entry_s {
    int64_t m_priority; /**< entry priority */
    T m_data;  /**< entry data */
  } pqueue_entry_s;

  public:
    /**
     * pqueue constructor
     */
    pqueue_c(const int& size, const int& latency, const string name, macsim_c* simBase)
    {
      m_capacity      = size;
      m_last_index    = latency;
      m_current_index = 0;
      m_num_entry     = 0;
      m_name          = name;
      m_size          = latency + 1;

      m_simBase       = simBase;

      m_entry = new list<pqueue_entry_s *>[m_size];
      m_entry_pool = new pool_c<pqueue_entry_s>(100, name + "_pqueue_entry_pool");
    }

    /**
     * pqueue destructor
     */
    ~pqueue_c()
    {
      flush();
      delete[] m_entry;
    }

    /**
     * Check ready entry
     */
    bool ready()
    {
      return !m_entry[m_current_index].empty();
    }

    /**
     * Insert a new entry
     */
    bool enqueue(int64_t priority, const T& data)
    {
      ASSERT(m_num_entry < m_capacity);

      m_num_entry++;

      pqueue_entry_s *new_entry = m_entry_pool->acquire_entry(); 
      new_entry->m_data     = data;
      new_entry->m_priority = priority;

      bool insert = false;
      int count = 0;
      for (auto I = m_entry[m_last_index].begin(), E = m_entry[m_last_index].end(); 
          I != E; ++I) {
          count++;
        if ((*I)->m_priority < priority) {
          m_entry[m_last_index].insert(I, new_entry);
          insert = true;
          break;
        }
      }

      if (!insert)
        m_entry[m_last_index].push_back(new_entry);

      return true;
    }
  
    /**
     * Dequeue an entry
     */
    T dequeue(int64_t *priority = 0)
    {
      pqueue_entry_s *entry = m_entry[m_current_index].front();
      if (priority)
        *priority = entry->m_priority;

      T data = entry->m_data;

      entry->m_data     = T(0);
      entry->m_priority = -1;

      m_entry[m_current_index].pop_front();
      m_entry_pool->release_entry(entry); 

      --m_num_entry;
      

      return data;
    } 

    /**
     * Advance queues (aging)
     */
    bool advance()
    {
      if (m_num_entry == 0)
        return true;

      if (!m_entry[m_current_index].empty())
        return true;

      m_last_index    = m_current_index;
      m_current_index = (m_current_index + 1) % m_size;

      return true;
    }

    /**
     * Search N-th priority entry
     */
    T peek(int entry)
    {
      ASSERT(entry <= m_num_entry);

      T data;
      int count = 0;
      for (int ii = m_current_index; ii < m_current_index + m_size; ++ii) {
        int index = ii % m_size;
        for (auto I = m_entry[index].begin(), E = m_entry[index].end(); I != E; ++I) {
          if (count++ == entry) {
            data = (*I)->m_data;
            break;
          }
        }
      }
      
      return data;
    }

    /**
     * Return available spaces
     */
    int space()
    {
      return m_capacity - m_num_entry;
    }

    /**
     * Flush all queue entries
     */
    void flush()
    {
      for (int ii = 0; ii < m_size; ++ii) {
        while (!m_entry[ii].empty()) {
          pqueue_entry_s *entry = m_entry[ii].front();

          m_entry[ii].pop_front();

          entry->m_data = T(0);
          entry->m_priority = -1;

          m_entry_pool->release_entry(entry);
        }
      }

      m_num_entry     = 0;
      m_current_index = 0;
      m_last_index    = m_size;
    }

    /**
     * Return pool size
     */
    int pool_size(void)
    {
      return m_entry_pool->size();
    }

  private:
    pqueue_c(); // do not implement

    list<pqueue_entry_s *>* m_entry; /**< queue buckets */
    pool_c<pqueue_entry_s>* m_entry_pool; /**< queue entry full */
    string m_name; /**< queue name */
    int m_size; /**< queue bucket size */
    int m_capacity; /**< queue capacity */
    int m_num_entry; /**< current queue entries */
    int m_last_index; /**< last bucket index */
    int m_current_index; /**< current bucket index */
    
    macsim_c* m_simBase; /**< macsim_c base class for simulation globals */
};

#endif

