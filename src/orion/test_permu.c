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

#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "SIM_parameter.h"
#include "SIM_permu.h"


int main(int argc, char **argv)
{
	u_int in_p, mode_p, op_p, in_c, mode_c, op_c;
	SIM_permu_t permu;
	double Emax, Etmp;
	u_int d_tab[150];
	u_int64_t cnt;
	int model;

	if (argc < 2) {
		fprintf(stderr, "usage: test_permu <model>\n");
		exit(1);
	}

	model = atoi(argv[1]);
	if (!(model && model < PERMU_MAX_MODEL)) {
		fprintf(stderr, "invalid model %d\n", model);
		exit(1);
	}

	SIM_init();
	SIM_permu_init(&permu, model, 8);
	Emax = SIM_permu_max_energy(&permu);
	printf("%g\n", Emax);
	SIM_permu_init(&permu, model, 16);
	Emax = SIM_permu_max_energy(&permu);
	printf("%g\n", Emax);
	SIM_permu_init(&permu, model, 32);
	Emax = SIM_permu_max_energy(&permu);
	printf("%g\n", Emax);
	SIM_permu_init(&permu, model, 64);
	Emax = SIM_permu_max_energy(&permu);
	printf("%g\n", Emax);
	exit(0);

	memset(d_tab, 0, 150 * sizeof(u_int));

	switch (model) {
		case OMFLIP_PERMU:
			for (in_p = 0; in_p < 256; in_p ++)
				for (op_p = 0; op_p < 256; op_p ++)
					for (mode_p = 0; mode_p < 4; mode_p ++) {
						SIM_permu_record(&permu, in_p, mode_p, op_p, 0);

						for (in_c = 0; in_c < 256; in_c ++)
							for (op_c = 0; op_c < 256; op_c ++) {
								SIM_permu_record_test(&permu, in_c, 0, op_c);
								Etmp = SIM_permu_report(&permu);
								d_tab[(u_int)ceil(Etmp / Emax * 100)] ++;

								SIM_permu_record_test(&permu, in_c, 1, op_c);
								Etmp = SIM_permu_report(&permu);
								d_tab[(u_int)ceil(Etmp / Emax * 100)] ++;

								SIM_permu_record_test(&permu, in_c, 2, op_c);
								Etmp = SIM_permu_report(&permu);
								d_tab[(u_int)ceil(Etmp / Emax * 100)] ++;

								SIM_permu_record_test(&permu, in_c, 3, op_c);
								Etmp = SIM_permu_report(&permu);
								d_tab[(u_int)ceil(Etmp / Emax * 100)] ++;
							}
					}
			break;

		case GRP_PERMU:
			for (in_p = 0; in_p < 256; in_p ++)
				for (op_p = 0; op_p < 256; op_p ++) {
					SIM_permu_record(&permu, in_p, 0, op_p, 0);

					for (in_c = 0; in_c < 256; in_c ++)
						for (op_c = 0; op_c < 256; op_c ++) {
							SIM_permu_record_test(&permu, in_c, 0, op_c);
							Etmp = SIM_permu_report(&permu);
							d_tab[(u_int)ceil(Etmp / Emax * 100)] ++;
						}
				}
			break;
	}

	/* print stat */
	for (cnt = 0, in_p = 0; in_p < 150; in_p ++) {
		printf("%d\n", d_tab[in_p]);
		cnt += d_tab[in_p];
	}
	printf("total: %lu\n", cnt);

	exit(0);
}
