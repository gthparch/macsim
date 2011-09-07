/** @file common-defs.h
 *  Common definitions for Manifold
 */
 
// George F. Riley, (and others) Georgia Tech, Fall 2010

#ifndef __COMMON_DEFS_H__
#define __COMMON_DEFS_H__

#include <stdint.h>
#include <string>
#include <map>

namespace manifold {
namespace kernel {

/** Component Id type
 */
typedef int      CompId_t;

/** Component Name type
 */
typedef std::string CompName_t;

/** Component Name Map Type
 */
typedef std::map<CompName_t,CompId_t> NameMap;

/** Link Id type
 */ 
typedef int      LinkId_t;

/** Logical process Id type (for distributed simulation)
 */
typedef int      LpId_t;

/** Clock ticks type
 */
typedef uint64_t Ticks_t;

/** Floating point time type
 */
typedef double   Time_t;    // Floating point time

/** Defining INFINITY as 1E300
 */
#ifdef INFINITY
  #undef INFINITY
  #define INFINITY 1E300
#else
  #define INFINITY 1E300
#endif


/** Defining nil as 0
 */
#define nil 0

} //namespace kernel
} //namespace manifold


#endif



  

