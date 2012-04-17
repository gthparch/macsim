/*
 * =====================================================================================
 *
 *       Filename:  genericHeader.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  02/05/2011 01:20:13 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Mitchelle Rasquinha
 *        Company:  Georgia Institute of Technology
 *
 * =====================================================================================
 */

#ifndef  GENERICHEADER_H_INC
#define  GENERICHEADER_H_INC

#include	<iostream>
#include	<fstream>
#include	<string>
#include	<sys/time.h>
#include	<algorithm>
#include	<vector>
#include 	<stdio.h>
#include 	<stdlib.h>
#include 	<time.h>
#include 	<setjmp.h>
#include 	<signal.h>
#include 	<sys/types.h>
#include 	<unistd.h>
#include 	<sys/time.h>
#include 	<sys/io.h>
#include	<sstream>
#include	<fstream>
#include	"kernel/clock.h"
#include	"kernel/manifold.h"

/* Macro for exit(1) */
#define _print_file fprintf(stderr,"\nERROR  %s:%d \n",__FILE__,__LINE__);  
#define _sim_exit_now(fmt,...) _print_file(void) fprintf(stderr,fmt,__VA_ARGS__);  exit(1);

/*  macro for general debug print statements. */
//#define _GRANK  int MPI_Comm_rank(MPI_COMM_WORLD,&i); return i; 
#define _GRANK  return 0;

#define _HEADER fprintf(m_simBase->log_file,"\nProc:%d Time:%lld Nid:%d Cid:%d ",m_simBase->Mytid, static_cast<long long int>(manifold::kernel::Manifold::NowTicks()),this->node_id,this->GetComponentId());
#define _DBG(fmt,...) {_HEADER (void)fprintf(m_simBase->log_file,fmt,__VA_ARGS__); fflush(m_simBase->log_file);}


//(...) (void)fprintf (m_simBase->log_file,"%d %d ... ", __FILE__, __LINE__, __VA_ARGS__);

//#define LOC cout << "\n" << m_simBase->Mytid << " Time:" << std::dec << manifold::kernel::Manifold::NowTicks() <<" " << node_id << " ";
//#define _DBG(fmt,...) LOC printf(fmt,__VA_ARGS__);
#define _DBG_NOARG(fmt) LOC printf(fmt);
#define LOC_log debug_log<< "\n" << m_simBase->Mytid <<" Time:" << std::dec << manifold::kernel::Manifold::NowTicks() <<" " << node_id << " ";
#define _DBG_log(fmt,...) LOC_log printf(fmt,__VA_ARGS__);

typedef unsigned long long int ullint;
typedef unsigned int uint;

/*  Enum for component types: Used to instantiate sub-classes */
enum component_type {PKTGEN,SIMPLEMC,SIMPLEROUTER,GENERICINTERFACE, IRIS_ROUTER, IRIS_INTERFACE, IRIS_TERMINAL};
enum topology_type {TWO_NODE,RING};

enum { SEND_DATA, RECV_DATA, SEND_SIG, RECV_SIG };
enum message_class { INVALID_PKT, PROC_REQ, L1_REQ, L2_REQ, L3_REQ, MC_RESP };
enum link_arrival_data_type { FLIT, CREDIT };
enum SW_ARBITRATION { ROUND_ROBIN, FCFS, ROUND_ROBIN_PRIORITY };
enum ROUTING_SCHEME { TWONODE_ROUTING, XY_ROUTING, XY_ROUTING_HETERO, NEGATIVE_FIRST, ODD_EVEN, WEST_FIRST, NORTH_LAST, NORTH_LAST_NON_MINIMAL, TORUS_ROUTING, RING_ROUTING, RING_ROUTING_UNI };

enum RouterPipeStage { 
   /* 0 */ INVALID, 
   /* 1 */ EMPTY,
   /* 2 */ IB, 
   /* 3 */ FULL, 
   /* 4 */ ROUTED,
   /* 5 */ VCA_REQUESTED, 
   /* 6 */ SWA_REQUESTED, 
   /* 7 */ SW_ALLOCATED, 
   /* 8 */ SW_TRAVERSAL, 
   /* 9 */ REQ_OUTVC_ARB, 
   /* 10 */ VCA_COMPLETE 
};

#endif   /* ----- #ifndef GENERICHEADER_H_INC  ----- */
