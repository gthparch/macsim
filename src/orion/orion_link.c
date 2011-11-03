/*-------------------------------------------------------------------------
 *                              ORION 2.0 
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

#include "SIM_parameter.h"
#include "SIM_link.h"

/* Link power and area model is only supported for 90nm, 65nm, 45nm and 32nm */

/*
 * link length in micro-meter - For consistency, user will enter the length in micro-meters, but in the code
   we multiply it by "1e-6" to convert it back to meters for internal calculations.
 */

int main(int argc, char **argv)
{
	double link_len;
	u_int data_width;
	double Pdynamic, Pleakage, Ptotal;
	double freq;
	double load;
	double link_area;

#if ( PARM(TECH_POINT) <= 90 )
	if (argc < 3) {
		fprintf(stderr, "orion_link: [length] [load]\n");
		exit(1);
	}

	/* read arguments */
	/* link length is also the core size*/
	link_len = atof(argv[1]); //unit micro-meter
	link_len = link_len * 1e-6; //unit meter
	load = atof(argv[2]);

	freq = PARM(Freq);
	data_width = PARM(flit_width);

	Pdynamic = 0.5 * load * LinkDynamicEnergyPerBitPerMeter(link_len, Vdd) * freq * link_len * (double)data_width;
	Pleakage = LinkLeakagePowerPerMeter(link_len, Vdd) * link_len * data_width;
	Ptotal = (Pdynamic + Pleakage) * PARM(in_port);

	link_area = LinkArea(link_len, data_width) * PARM(in_port); 

	fprintf(stdout, "Link power is %g\n", Ptotal);
	fprintf(stdout, "Link area is %g\n", link_area);

#else 
	fprintf(stderr, "Link power and area are only supported for 90nm, 65nm, 45nm and 32nm\n");
#endif

	return 0;
}

