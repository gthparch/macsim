/**********************************************************************************************
 * File         : global_types.h
 * Author       : Hyesoon Kim 
 * Date         : 12/16/2007 
 * CVS          : $Id: global_types.h,v 1.1 2008-02-22 22:51:06 hyesoon Exp $:
 * Description  : Global type declarations intended to be included in every source file.
                  origial author: Robert S. Chappell  imported from 
 *********************************************************************************************/

#ifndef GLOBALS_H_INCLUDED 
#define GLOBALS_H_INCLUDED 

#include <cstdint>


// Renames
// Try to use these rather than built-in C types in order to preserve portability
typedef unsigned           uns;
typedef unsigned char      uns8;
typedef unsigned short     uns16;
typedef unsigned           uns32;
typedef unsigned long long uns64;
typedef char               int8;
typedef short              int16;
typedef int                int32;
typedef int long long      int64;
typedef int                Generic_Enum;
typedef uns64              Counter; 
typedef int64              Quad;
typedef uns64              UQuad;

/* power & hotleakage  */  /* please CHECKME Hyesoon 6-15-2009 */ 
typedef uns64 tick_t; 
typedef uns64 counter_t;  

/* Conventions */
typedef uns64 Addr;
typedef uns32 Binary;


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Core type (for heterogeneous simulation)
///////////////////////////////////////////////////////////////////////////////////////////////
typedef enum _Unit_Type_enum
{
  UNIT_SMALL = 0, /**< small core */
  UNIT_MEDIUM, /**< medium core */
  UNIT_LARGE /**< large core */
} Unit_Type;

#endif 
