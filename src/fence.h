#ifndef _FENCE_H
#define _FENCE_H

#include <iostream>

using namespace std;

typedef enum fence_type {
  NOT_FENCE,
  FENCE_ACQUIRE,
  FENCE_RELEASE,
  FENCE_FULL,
  FENCE_NUM
} fence_type;

class fence_list {
 private:
  unordered_set<int> m_fence_set; /**< fence active or inactive */
  list<int> m_fence_list;
 public:

  /* 
   * Insert a fence entry in the list
   */
  void ins_fence_entry(int entry)
  {
    auto ret = m_fence_set.insert(entry);

    if (ret.second == true)
      m_fence_list.push_back(entry);
  }

  /*
   * Remove fence entry from the list
   */
  void del_fence_entry(void)
  {
    int first_entry = m_fence_list.front();
    m_fence_list.pop_front();
    m_fence_set.erase(first_entry);
  }

  /*
   * Print all the stores fence entries, for debugging
   */
  void print_fence_entries(void)
  {
    for (auto it = m_fence_list.begin(); it != m_fence_list.end(); ++it)
      cout << *it << endl;

    fflush(NULL);
  }

  /*
   * Get the index entry for the first fence in the list
   */
  int get_front_index(void)
  {
    if (m_fence_list.empty())
      return -1;

    return m_fence_list.front();
  }

  /*
   * Check if the list has any fence
   */
  bool is_list_empty(void) 
  { 
    return m_fence_list.empty();
  }

  list<int>::const_iterator cbegin() const
  {
    return m_fence_list.cbegin();
  }

  list<int>::const_iterator cend() const
  {
    return m_fence_list.cend();
  }
};

class fence_c {
  private:
    fence_list m_full_fence_list;
    fence_list m_acq_fence_list;
    fence_list m_rel_fence_list;

    macsim_c*  m_simBase;
  public:

    fence_c(macsim_c* simBase) 
    {
      m_simBase = simBase;
    }

    /*
     * Get the entry index of the corresponding fence
     */
    int get_front_index(fence_type ft)
    {
      switch (ft) {
      case FENCE_FULL:
        return m_full_fence_list.get_front_index();
      case FENCE_ACQUIRE:
        return m_acq_fence_list.get_front_index();
      case FENCE_RELEASE:
        return m_rel_fence_list.get_front_index();
      case NOT_FENCE:
      default:
        ASSERT(0);
      }
    }

    /*
     * Check if any fence is active. If it is present in the list it is active
     */
    bool is_fence_active()
    {
      return !m_full_fence_list.is_list_empty() ||
             !m_acq_fence_list.is_list_empty()  ||
             !m_rel_fence_list.is_list_empty();
    }

    /*
     * Insert a fence entry into corresponding list
     */
    void ins_fence_entry(int entry, fence_type ft)
    {
      if (ft == FENCE_ACQUIRE)
        m_acq_fence_list.ins_fence_entry(entry);
      else if (ft == FENCE_RELEASE)
        m_rel_fence_list.ins_fence_entry(entry);
      else
        m_full_fence_list.ins_fence_entry(entry);
    }

    /*
     * Remove a fence entry from corresponding list
     */
    void del_fence_entry(fence_type ft)
    {
      if (ft == FENCE_ACQUIRE)
        m_acq_fence_list.del_fence_entry();
      else if (ft == FENCE_RELEASE)
        m_rel_fence_list.del_fence_entry();
      else
        m_full_fence_list.del_fence_entry();
    }

    /*
     * Print all fences entries of specified type
     */
    void print_fence_entries(fence_type ft)
    {
      if (ft == FENCE_ACQUIRE)
        m_acq_fence_list.print_fence_entries();
      else if (ft == FENCE_RELEASE)
        m_rel_fence_list.print_fence_entries();
      else
        m_full_fence_list.print_fence_entries();
    }

    /*
     * Check if all the fence lists are empty
     */
    bool is_list_empty(void)
    {
      return m_full_fence_list.is_list_empty() &&
             m_acq_fence_list.is_list_empty()  &&
             m_rel_fence_list.is_list_empty();
    }

    list<int>::const_iterator cbegin(fence_type ft)
    {
      if (ft == FENCE_ACQUIRE)
        return m_acq_fence_list.cbegin();
      else if (ft == FENCE_RELEASE)
        return m_rel_fence_list.cbegin();
      else if (ft == FENCE_FULL)
        return m_full_fence_list.cbegin();
    }

    list<int>::const_iterator cend(fence_type ft)
    {
      if (ft == FENCE_ACQUIRE)
        return m_acq_fence_list.cend();
      else if (ft == FENCE_RELEASE)
        return m_rel_fence_list.cend();
      else if (ft == FENCE_FULL)
        return m_full_fence_list.cend();
    }
};

#endif /* _FENCE_H */
