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

#include <string.h>
#include <assert.h>

#include "SIM_ALU.h"
#include "SIM_util.h"
#include "SIM_time.h"

#define is_logic(x)	(x & 1)


/* PLX gate type */
enum { PLX_MUX2, PLX_MUX3, PLX_MUX4, PLX_PASS, PLX_NAND2, PLX_NAND3, PLX_NAND4,
	PLX_NOR2, PLX_NOR3, PLX_NOR4, PLX_NOT, PLX_XOR, PLX_GATE };

/* transistor count table of PLX ALU */
static u_int plx_tc[PLX_BLK][PLX_GATE] =
      {  0,  0,  0,  0, 23,  0,  0,  0,  0,  0, 11,  0,
	16,  0,  0, 32,  0,  0,  0,  0,  0,  0,  0,  0,
	16,  0,  0, 32,  8,  0,  0,  8,  0,  0,  8,  8,
	0,  0,  0,  0, 16,  0,  0,  0,  0,  0,  0, 16,
	16,  0, 16, 96,  0,  0,  0,  0,  0,  0,  0,  0,
	8,  0,  0, 16,  0,  0,  0,  0,  0,  0,  8,  0,
	0,  0,  0,  0,  6,  4,  3,  2,  2,  2, 19,  8,
	3,  0,  0,  6,  0,  0,  0,  0,  0,  0,  0,  0,
	8,  8,  0, 40,  0,  0,  0,  0,  0,  0,  0,  0,
	8,  0,  0, 16,  0,  0,  0,  1,  0,  0,  1,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 };
	  /* WHS: ignore for fair comparison */
	  //0,  0,  0,  0,  0,  0,  0,  8,  0,  0, 16,  0 };

/* capacitance table of PLX ALU */
static double plx_c[PLX_GATE];


static double c_NOT(void)
{
	return (SIM_draincap(WaluNOTp, PCH, 1) + SIM_draincap(WaluNOTn, NCH, 1) +
			SIM_gatecap(WaluNOTp + WaluNOTn, 0));
}


static double c_NAND(u_int n_in)
{
	if (n_in == 2)
		return (2 * SIM_draincap(Walu2NANDp, PCH, 1) + SIM_draincap(Walu2NANDn, NCH, 2) +
				2 * SIM_gatecap(Walu2NANDp + Walu2NANDn, 0));
	else if (n_in == 3)
		return (3 * SIM_draincap(Walu3NANDp, PCH, 1) + SIM_draincap(Walu3NANDn, NCH, 3) +
				3 * SIM_gatecap(Walu3NANDp + Walu3NANDn, 0));
	else if (n_in == 4)
		return (4 * SIM_draincap(Walu4NANDp, PCH, 1) + SIM_draincap(Walu4NANDn, NCH, 4) +
				4 * SIM_gatecap(Walu4NANDp + Walu4NANDn, 0));
	else return -1;
}


static double c_NOR(u_int n_in)
{
	if (n_in == 2)
		return (SIM_draincap(Walu2NORp, PCH, 2) + 2 * SIM_draincap(Walu2NORn, NCH, 1) +
				2 * SIM_gatecap(Walu2NORp + Walu2NORn, 0));
	else if (n_in == 3)
		return (SIM_draincap(Walu3NORp, PCH, 3) + 3 * SIM_draincap(Walu3NORn, NCH, 1) +
				3 * SIM_gatecap(Walu3NORp + Walu3NORn, 0));
	else if (n_in == 4)
		return (SIM_draincap(Walu4NORp, PCH, 4) + 4 * SIM_draincap(Walu4NORn, NCH, 1) +
				4 * SIM_gatecap(Walu4NORp + Walu4NORn, 0));
	else return -1;
}


static double c_XOR(void)
{
	/* FIXME: ??? */
	return (SIM_draincap(WaluXORp, PCH, 2) + SIM_draincap(WaluXORn, NCH, 2) +
			2 * SIM_gatecap(WaluXORp + WaluXORn, 0));
}


static double c_PASS(void)
{
	return (2 * (SIM_draincap(WaluPASSp, PCH, 1) + SIM_draincap(WaluPASSn, NCH, 1)) +
			SIM_gatecap(WaluPASSp + WaluPASSn, 0));
}


int SIM_ALU_init(SIM_ALU_t *alu, int model, u_int data_width)
{
	u_int i, j;

	switch (alu->model = model) {
		case PLX_ALU:
			assert(data_width && !(data_width % 8));

			alu->data_width = data_width;
			memset(alu->n_chg_blk, 0, sizeof(alu->n_chg_blk));

			/* initialize energy table */
			plx_c[PLX_NOT] = c_NOT();
			plx_c[PLX_NAND2] = c_NAND(2);
			plx_c[PLX_NAND3] = c_NAND(3);
			plx_c[PLX_NAND4] = c_NAND(4);
			plx_c[PLX_NOR2] = c_NOR(2);
			plx_c[PLX_NOR3] = c_NOR(3);
			plx_c[PLX_NOR4] = c_NOR(4);
			plx_c[PLX_XOR] = c_XOR();
			plx_c[PLX_PASS] = c_PASS();
			/* FIXME: ??? */
			plx_c[PLX_MUX2] = 0;
			plx_c[PLX_MUX3] = 0;
			plx_c[PLX_MUX4] = 0;

			for (i = 0; i < PLX_BLK; i++) {
				alu->e_blk[i] = 0;
				for (j = 0; j < PLX_GATE; j++)
					alu->e_blk[i] += plx_tc[i][j] * plx_c[j];

				/* WHS: assume switch probability 0.5 */
				alu->e_blk[i] = alu->e_blk[i] / 2 * EnergyFactor * 0.5;
			}
			break;

		default: return -1;
	}

	return 0;
}


int SIM_ALU_record(SIM_ALU_t *alu, LIB_Type_max_uint d1, LIB_Type_max_uint d2, u_int type)
{
	u_int d_H4, d_H8, d_H16;
	LIB_Type_max_uint d1_old, d2_old;

	switch (alu->model) {
		case PLX_ALU:
			/* control logic */
			alu->n_chg_blk[PLX_CTR] += type != alu->type;

			/* global data path */
			if (is_logic(alu->type)) {
				d1_old = alu->l_d1;
				d2_old = alu->l_d2;
			}
			else {	/* arithmetic */
				d1_old = alu->a_d1;
				d2_old = alu->a_d2;
			}
			d_H8 = SIM_Hamming_group(d1, d1_old, d2, d2_old, 8, alu->data_width / 8);
			alu->n_chg_blk[PLX_IMM] += d_H8;
			alu->n_chg_blk[PLX_OUT] += d_H8;

			/* logic/arithmetic data path */
			if (is_logic(type)) {
				d_H8 = SIM_Hamming_group(d1, alu->l_d1, d2, alu->l_d2, 8, alu->data_width / 8);
				alu->n_chg_blk[PLX_LOGIC] += d_H8;

				alu->l_d1 = d1;
				alu->l_d2 = d2;
			}
			else {	/* arithmetic */
				/* WHS: hack to support 8-bit ALU */
				d_H16 = SIM_Hamming_group(d1, alu->a_d1, d2, alu->a_d2, 16, MAX(1, alu->data_width / 16));
				d_H8 = SIM_Hamming_group(d1, alu->a_d1, d2, alu->a_d2, 8, alu->data_width / 8);
				d_H4 = SIM_Hamming_group(d1, alu->a_d1, d2, alu->a_d2, 4, alu->data_width / 4);

				alu->n_chg_blk[PLX_SIGN] += d_H8;
				alu->n_chg_blk[PLX_SHIFT] += d_H16;
				alu->n_chg_blk[PLX_2COM] += d_H8;
				alu->n_chg_blk[PLX_ADD] += d_H4;
				alu->n_chg_blk[PLX_SUBW] += d_H8;
				alu->n_chg_blk[PLX_SAT] += d_H8;
				alu->n_chg_blk[PLX_AVG] += d_H8;

				alu->a_d1 = d1;
				alu->a_d2 = d2;
			}

			alu->type = type;
			break;

		default: return -1;
	}

	return 0;
}


double SIM_ALU_report(SIM_ALU_t *alu)
{
	u_int i;
	double Etotal = 0;

	switch (alu->model) {
		case PLX_ALU:
			for (i = 0; i < PLX_BLK; i++)
				Etotal += alu->e_blk[i] * alu->n_chg_blk[i];

			return Etotal;

		default: return -1;
	}
}


double SIM_ALU_stat_energy(SIM_ALU_t *alu, int max)
{
	double Etotal = 0;
	double d_H4, d_H8, d_H16;

	if (max) {	/* maximum energy */
		d_H4 = alu->data_width / 4;
		d_H8 = alu->data_width / 8;
		d_H16 = MAX(1, alu->data_width / 16);

		/* control logic */
		Etotal += alu->e_blk[PLX_CTR];

		/* arithmetic data path */
		Etotal += d_H8 * (alu->e_blk[PLX_IMM] + alu->e_blk[PLX_SAT] + alu->e_blk[PLX_AVG] + alu->e_blk[PLX_OUT] +
				alu->e_blk[PLX_SIGN] + alu->e_blk[PLX_2COM] + alu->e_blk[PLX_SUBW]) +
			d_H16 * alu->e_blk[PLX_SHIFT] +
			d_H4 * alu->e_blk[PLX_ADD];

		/* compensate for switch probability 0.5 */
		return (2 * Etotal);
	}
	else {	/* average energy */
		d_H4 = alu->data_width / 4 * (1 - 1.0 / (BIGONE << 8));
		d_H8 = alu->data_width / 8 * (1 - 1.0 / (BIGONE << 16));
		d_H16 = MAX(1, alu->data_width / 16) * (1 - 1.0 / (BIGONE << 32));

		/* control logic */
		Etotal += alu->e_blk[PLX_CTR] * (1 - 1.0 / 26.0);

		/* global data path */
		Etotal += d_H8 * (alu->e_blk[PLX_IMM] + alu->e_blk[PLX_OUT]);

		/* logic data path */
		/* WHS: assume instruction type is a uniform distribution */
		Etotal += d_H8 * alu->e_blk[PLX_LOGIC] * (8.0 / 26.0);

		/* arithmetic data path */
		Etotal += (d_H8 * (alu->e_blk[PLX_SAT] + alu->e_blk[PLX_AVG] + alu->e_blk[PLX_SIGN] +
					alu->e_blk[PLX_2COM] + alu->e_blk[PLX_SUBW]) +
				d_H16 * alu->e_blk[PLX_SHIFT] +
				d_H4 * alu->e_blk[PLX_ADD]) * (18.0 / 26.0);

		return Etotal;
	}
}

