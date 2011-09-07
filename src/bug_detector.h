/**********************************************************************************************
 * File         : bug_detector.h
 * Author       : Jaekyu Lee
 * Date         : 9/8/2010
 * SVN          : $Id: dram.h 868 2009-11-05 06:28:01Z kacear $
 * Description  : Bug Detector
 *********************************************************************************************/

#ifndef BUG_DETECTOR_H_INCLUDED
#define BUG_DETECTOR_H_INCLUDED


#include <vector>
#include <map>

#include "macsim.h"
#include "global_types.h"
#include "global_defs.h"


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief bug detection mechanism
///
/// Track each uop and memory reqeust to check where it is stuck 
///////////////////////////////////////////////////////////////////////////////////////////////
class bug_detector_c 
{
  public:
    /**
     * Bug detector class constructor
     */
    bug_detector_c(macsim_c* simBase);

    /**
     * Bug detector class destructor
     */
    ~bug_detector_c();

    /**
     * When a new uop starts exectuion, record it to the table.
     */
    void allocate(uop_c *uop);

    /**
     * When a new uop retires, take this uop out from the table.
     */
    void deallocate(uop_c *uop);

    /**
     * Print all un-retired (might be bug) uops
     */
    void print(int core_id, int thread_id);

  private:
    int m_num_core; /**< number of simulating cores */
    vector<map<uop_c*, uint64_t> *> m_uop_table; /**< uop table */
    uint64_t *m_latency_sum; /**< sum of each uop's execution latency */
    uint64_t *m_latency_count; /**< total uop count */
    
    macsim_c* m_simBase;
};
#endif
