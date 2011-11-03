/*-------------------------------------------------------------------------
 *                             ORION 2.0 
 *
 *         					Copyright 2009 
 *  	Princeton University, and Regents of the University of California 
 *                         All Rights Reserved
 *
 *                         
 *  ORION 2.0 was developed by Bin Li at Princeton University and Kambiz Samadi at
 *  University of California, San Diego. ORION 2.0 was built on top of ORION 1.0. 
 *  ORION 1.0 was developed by Hangsheng Wang, Xinping Zhu and Xuning Chen at 
 *  Princeton University.
 *
 *  If your use of this software contributes to a published paper, we
 *  request that you cite our paper that appears on our website 
 *  http://www.princeton.edu/~peh/orion.html
 *
 *  Permission to use, copy, and modify this software and its documentation is
 *  granted only under the following terms and conditions.  Both the
 *  above copyright notice and this permission notice must appear in all copies
 *  of the software, derivative works or modified versions, and any portions
 *  thereof, and both notices must appear in supporting documentation.
 *
 *  This software may be distributed (but not offered for sale or transferred
 *  for compensation) to third parties, provided such third parties agree to
 *  abide by the terms and conditions of this notice.
 *
 *  This software is distributed in the hope that it will be useful to the
 *  community, but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
 *
 *-----------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "SIM_parameter.h"
#include "SIM_router.h"

char path[2048];
extern char *optarg;
extern int optind;

int main(int argc, char **argv)
{
  int max_flag = AVG_ENERGY, plot_flag = 0;
  u_int print_depth = 0;
  double load = 1;
  char *name, opt;
  
  /* parse options */
  while ((opt = getopt(argc, argv, "+pmd:l:")) != -1) {
    switch (opt) {
      case 'p': plot_flag = 1; break;
      case 'm': max_flag = MAX_ENERGY; break;
      case 'd': print_depth = atoi(optarg); break;
      case 'l': load = atof(optarg); break;
    }
  }

  if (optind >= argc) {
    fprintf(stderr, "orion_router_power: [-pm] [-d print_depth] [-l load] <router_name>\n");
    return 1;
  }
  else {
    name = argv[optind];
  }

  SIM_router_init(&GLOB(router_info), &GLOB(router_power), NULL);

  SIM_router_stat_energy(&GLOB(router_info), &GLOB(router_power), print_depth, name, max_flag, load, plot_flag, PARM(Freq));

  return 0;
}
