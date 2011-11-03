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
#include <math.h>

#include "SIM_arbiter.h"
#include "SIM_static.h"
#include "SIM_util.h"
#include "SIM_time.h"

/* switch cap of request signal (round robin arbiter) */
static double SIM_rr_arbiter_req_cap(double length)
{
	double Ctotal = 0;

	/* part 1: gate cap of 2 NOR gates */
	/* FIXME: need actual size */
	Ctotal += 2 * SIM_gatecap(WdecNORn + WdecNORp, 0);

	/* part 2: inverter */
	/* FIXME: need actual size */
	Ctotal += SIM_draincap(Wdecinvn, NCH, 1) + SIM_draincap(Wdecinvp, PCH, 1) +
		SIM_gatecap(Wdecinvn + Wdecinvp, 0);

	/* part 3: wire cap */
	Ctotal += length * Cmetal;

	return Ctotal;
}


/* switch cap of priority signal (round robin arbiter) */
static double SIM_rr_arbiter_pri_cap()
{
	double Ctotal = 0;

	/* part 1: gate cap of NOR gate */
	/* FIXME: need actual size */
	Ctotal += SIM_gatecap(WdecNORn + WdecNORp, 0);

	return Ctotal;
}


/* switch cap of grant signal (round robin arbiter) */
static double SIM_rr_arbiter_grant_cap()
{
	double Ctotal = 0;

	/* part 1: drain cap of NOR gate */
	/* FIXME: need actual size */
	Ctotal += 2 * SIM_draincap(WdecNORn, NCH, 1) + SIM_draincap(WdecNORp, PCH, 2);

	return Ctotal;
}


/* switch cap of carry signal (round robin arbiter) */
static double SIM_rr_arbiter_carry_cap()
{
	double Ctotal = 0;

	/* part 1: drain cap of NOR gate (this block) */
	/* FIXME: need actual size */
	Ctotal += 2 * SIM_draincap(WdecNORn, NCH, 1) + SIM_draincap(WdecNORp, PCH, 2);

	/* part 2: gate cap of NOR gate (next block) */
	/* FIXME: need actual size */
	Ctotal += SIM_gatecap(WdecNORn + WdecNORp, 0);

	return Ctotal;
}


/* switch cap of internal carry node (round robin arbiter) */
static double SIM_rr_arbiter_carry_in_cap()
{
	double Ctotal = 0;

	/* part 1: gate cap of 2 NOR gates */
	/* FIXME: need actual size */
	Ctotal += 2 * SIM_gatecap(WdecNORn + WdecNORp, 0);

	/* part 2: drain cap of NOR gate */
	/* FIXME: need actual size */
	Ctotal += 2 * SIM_draincap(WdecNORn, NCH, 1) + SIM_draincap(WdecNORp, PCH, 2);

	return Ctotal;
}


/* the "huge" NOR gate in matrix arbiter model is an approximation */
/* switch cap of request signal (matrix arbiter) */
static double SIM_matrix_arbiter_req_cap(u_int req_width, double length)
{
	double Ctotal = 0;

	/* FIXME: all need actual sizes */
	/* part 1: gate cap of NOR gates */
	Ctotal += (req_width - 1) * SIM_gatecap(WdecNORn + WdecNORp, 0);

	/* part 2: inverter */
	Ctotal += SIM_draincap(Wdecinvn, NCH, 1) + SIM_draincap(Wdecinvp, PCH, 1) +
		SIM_gatecap(Wdecinvn + Wdecinvp, 0);

	/* part 3: gate cap of the "huge" NOR gate */
	Ctotal += SIM_gatecap(WdecNORn + WdecNORp, 0);

	/* part 4: wire cap */
	Ctotal += length * Cmetal;

	return Ctotal;
}


/* switch cap of priority signal (matrix arbiter) */
static double SIM_matrix_arbiter_pri_cap(u_int req_width)
{
	double Ctotal = 0;

	/* part 1: gate cap of NOR gates (2 groups) */
	Ctotal += 2 * SIM_gatecap(WdecNORn + WdecNORp, 0);

	/* no inverter because priority signal is kept by a flip flop */
	return Ctotal;
}


/* switch cap of grant signal (matrix arbiter) */
static double SIM_matrix_arbiter_grant_cap(u_int req_width)
{
	/* drain cap of the "huge" NOR gate */
	return (req_width * SIM_draincap(WdecNORn, NCH, 1) + SIM_draincap(WdecNORp, PCH, req_width));
}


/* switch cap of internal node (matrix arbiter) */
static double SIM_matrix_arbiter_int_cap()
{
	double Ctotal = 0;

	/* part 1: drain cap of NOR gate */
	Ctotal += 2 * SIM_draincap(WdecNORn, NCH, 1) + SIM_draincap(WdecNORp, PCH, 2);

	/* part 2: gate cap of the "huge" NOR gate */
	Ctotal += SIM_gatecap(WdecNORn + WdecNORp, 0);

	return Ctotal;
}
  

static int SIM_arbiter_clear_stat(SIM_arbiter_t *arb)
{
	arb->n_chg_req = arb->n_chg_grant = arb->n_chg_mint = 0;
	arb->n_chg_carry = arb->n_chg_carry_in = 0;

	SIM_array_clear_stat(&arb->queue);
	SIM_fpfp_clear_stat(&arb->pri_ff);

	return 0;
}


int SIM_arbiter_init(SIM_arbiter_t *arb, int arbiter_model, int ff_model, u_int req_width, double length, SIM_array_info_t *info)
{
	if ((arb->model = arbiter_model) && arbiter_model < ARBITER_MAX_MODEL) {
		arb->req_width = req_width;
		SIM_arbiter_clear_stat(arb);
		/* redundant field */
		arb->mask = HAMM_MASK(req_width);
		double I_static;
		switch (arbiter_model) {
			case RR_ARBITER:
				arb->e_chg_req = SIM_rr_arbiter_req_cap(length) / 2 * EnergyFactor;
				/* two grant signals switch together, so no 1/2 */
				arb->e_chg_grant = SIM_rr_arbiter_grant_cap() * EnergyFactor;
				arb->e_chg_carry = SIM_rr_arbiter_carry_cap() / 2 * EnergyFactor;
				arb->e_chg_carry_in = SIM_rr_arbiter_carry_in_cap() / 2 * EnergyFactor;
				arb->e_chg_mint = 0;

				if (SIM_fpfp_init(&arb->pri_ff, ff_model, SIM_rr_arbiter_pri_cap()))
					return -1;

				/*arbiter static power */
				I_static = 0;
				/* NOR */
				I_static += (6 * arb->req_width * ((WdecNORp*NOR2_TAB[0] + WdecNORn*(NOR2_TAB[1] + NOR2_TAB[2] + NOR2_TAB[3]))/4));

				/* inverter */
				I_static += 2 * arb->req_width * ((Wdecinvn*NMOS_TAB[0] + Wdecinvp*PMOS_TAB[0])/2);
				/* DFF */
				I_static += (arb->req_width * Wdff*DFF_TAB[0]);
				arb->I_static = I_static;

				break;

			case MATRIX_ARBITER:
				arb->e_chg_req = SIM_matrix_arbiter_req_cap(req_width, length) / 2 * EnergyFactor;
				/* 2 grant signals switch together, so no 1/2 */
				arb->e_chg_grant = SIM_matrix_arbiter_grant_cap(req_width) * EnergyFactor;
				arb->e_chg_mint = SIM_matrix_arbiter_int_cap() / 2 * EnergyFactor;
				arb->e_chg_carry = arb->e_chg_carry_in = 0;

				if (SIM_fpfp_init(&arb->pri_ff, ff_model, SIM_matrix_arbiter_pri_cap(req_width)))
					return -1;
				/*arbiter static power */
				I_static = 0;
				/* NOR */
				I_static += ((2 * arb->req_width - 1) * arb->req_width) * ((WdecNORp*NOR2_TAB[0] + WdecNORn*(NOR2_TAB[1] + NOR2_TAB[2] + NOR2_TAB[3]))/4);
				/* inverter */
				I_static += arb->req_width * ((Wdecinvn*NMOS_TAB[0] + Wdecinvp*PMOS_TAB[0])/2);
				/* DFF */
				I_static += (arb->req_width * (arb->req_width - 1) / 2) * (Wdff*DFF_TAB[0]);
				arb->I_static = I_static;

				break;

			case QUEUE_ARBITER:
				arb->e_chg_req = arb->e_chg_grant = arb->e_chg_mint = 0;
				arb->e_chg_carry = arb->e_chg_carry_in = 0;

				return SIM_array_power_init(info, &arb->queue);
				break;

			default:	printf ("error\n"); /* some error handler */
		}

		return 0;
	}
	else
		return -1;
}


int SIM_arbiter_record(SIM_arbiter_t *arb, LIB_Type_max_uint new_req, LIB_Type_max_uint old_req, u_int new_grant, u_int old_grant)
{
	switch (arb->model) {
		case MATRIX_ARBITER:
			arb->n_chg_req += SIM_Hamming(new_req, old_req, arb->mask);
			arb->n_chg_grant += new_grant != old_grant;
			/* FIXME: approximation */
			arb->n_chg_mint += (arb->req_width - 1) * arb->req_width / 2;
			/* priority registers */
			/* FIXME: use average instead */
			arb->pri_ff.n_switch += (arb->req_width - 1) / 2;
			break;

		case RR_ARBITER:
			arb->n_chg_req += SIM_Hamming(new_req, old_req, arb->mask);
			arb->n_chg_grant += new_grant != old_grant;
			/* FIXME: use average instead */
			arb->n_chg_carry += arb->req_width / 2;
			arb->n_chg_carry_in += arb->req_width / 2 - 1;
			/* priority registers */
			arb->pri_ff.n_switch += 2;
			break;

		case QUEUE_ARBITER:
			break;

		default: printf ("error\n");	/* some error handler */
	}

	return 0;
}

double SIM_arbiter_report(SIM_arbiter_t *arb)
{
	switch (arb->model) {
		case MATRIX_ARBITER:
			return (arb->n_chg_req * arb->e_chg_req + arb->n_chg_grant * arb->e_chg_grant +
					arb->n_chg_mint * arb->e_chg_mint +
					arb->pri_ff.n_switch * arb->pri_ff.e_switch +
					arb->pri_ff.n_keep_1 * arb->pri_ff.e_keep_1 +
					arb->pri_ff.n_keep_0 * arb->pri_ff.e_keep_0 +
					arb->pri_ff.n_clock * arb->pri_ff.e_clock);

		case RR_ARBITER:
			return (arb->n_chg_req * arb->e_chg_req + arb->n_chg_grant * arb->e_chg_grant +
					arb->n_chg_carry * arb->e_chg_carry + arb->n_chg_carry_in * arb->e_chg_carry_in +
					arb->pri_ff.n_switch * arb->pri_ff.e_switch +
					arb->pri_ff.n_keep_1 * arb->pri_ff.e_keep_1 +
					arb->pri_ff.n_keep_0 * arb->pri_ff.e_keep_0 +
					arb->pri_ff.n_clock * arb->pri_ff.e_clock);

		default: return -1;
	}
}

/* stat over one cycle */
/* info is only used by queuing arbiter */
double SIM_arbiter_stat_energy(SIM_arbiter_t *arb, SIM_array_info_t *info, double n_req, int print_depth, char *path, int max_avg)
{
	double Eavg = 0, Estruct, Eatomic, Estatic = 0;
	int next_depth, next_next_depth;
	double total_pri, n_chg_pri, n_grant;
	u_int path_len, next_path_len;

	next_depth = NEXT_DEPTH(print_depth);
	next_next_depth = NEXT_DEPTH(next_depth);
	path_len = SIM_strlen(path);
	/* energy cycle distribution */
	if (n_req > (arb->req_width + 0.001)) {
		fprintf(stderr, "arbiter overflow\n");
		n_req = arb->req_width;
	}


	if (n_req >= 1) n_grant = 1;
	else if (n_req) n_grant = 1.0 / ceil(1.0 / n_req);
	else n_grant = 0;
	switch (arb->model) {
		case RR_ARBITER:
			/* FIXME: we may overestimate request switch */
			Eatomic = arb->e_chg_req * n_req;
			SIM_print_stat_energy(SIM_strcat(path, "request"), Eatomic, next_depth);
			SIM_res_path(path, path_len);
			Eavg += Eatomic;

			Eatomic = arb->e_chg_grant * n_grant;
			SIM_print_stat_energy(SIM_strcat(path, "grant"), Eatomic, next_depth);
			SIM_res_path(path, path_len);
			Eavg += Eatomic;

			/* assume carry signal propagates half length in average case */
			/* carry does not propagate in maximum case, i.e. all carrys go down */
			Eatomic = arb->e_chg_carry * arb->req_width * (max_avg ? 1 : 0.5) * n_grant;
			SIM_print_stat_energy(SIM_strcat(path, "carry"), Eatomic, next_depth);
			SIM_res_path(path, path_len);
			Eavg += Eatomic;

			Eatomic = arb->e_chg_carry_in * (arb->req_width * (max_avg ? 1 : 0.5) - 1) * n_grant;
			SIM_print_stat_energy(SIM_strcat(path, "internal carry"), Eatomic, next_depth);
			SIM_res_path(path, path_len);
			Eavg += Eatomic;

			/* priority registers */
			Estruct = 0;
			SIM_strcat(path, "priority");
			next_path_len = SIM_strlen(path);

			Eatomic = arb->pri_ff.e_switch * 2 * n_grant;  
			SIM_print_stat_energy(SIM_strcat(path, "switch"), Eatomic, next_next_depth);
			SIM_res_path(path, next_path_len);
			Estruct += Eatomic;

			Eatomic = arb->pri_ff.e_keep_0 * (arb->req_width - 2 * n_grant); 
			SIM_print_stat_energy(SIM_strcat(path, "keep 0"), Eatomic, next_next_depth);
			SIM_res_path(path, next_path_len);
			Estruct += Eatomic;

			Eatomic = arb->pri_ff.e_clock * arb->req_width; 
			SIM_print_stat_energy(SIM_strcat(path, "clock"), Eatomic, next_next_depth); 
			SIM_res_path(path, next_path_len);
			Estruct += Eatomic;

			SIM_print_stat_energy(path, Estruct, next_depth);
			SIM_res_path(path, path_len);
			Eavg += Estruct;
			break;

		case MATRIX_ARBITER:
			total_pri = arb->req_width * (arb->req_width - 1) * 0.5;
			/* assume switch probability 0.5 for priorities */
			n_chg_pri = (arb->req_width - 1) * (max_avg ? 1 : 0.5);

			/* FIXME: we may overestimate request switch */
			Eatomic = arb->e_chg_req * n_req;
			SIM_print_stat_energy(SIM_strcat(path, "request"), Eatomic, next_depth);
			SIM_res_path(path, path_len);
			Eavg += Eatomic;

			Eatomic = arb->e_chg_grant * n_grant;
			SIM_print_stat_energy(SIM_strcat(path, "grant"), Eatomic, next_depth);
			SIM_res_path(path, path_len);
			Eavg += Eatomic;

			/* priority registers */
			Estruct = 0;
			SIM_strcat(path, "priority");
			next_path_len = SIM_strlen(path);

			Eatomic = arb->pri_ff.e_switch * n_chg_pri * n_grant; 
			SIM_print_stat_energy(SIM_strcat(path, "switch"), Eatomic, next_next_depth);
			SIM_res_path(path, next_path_len);
			Estruct += Eatomic;

			/* assume 1 and 0 are uniformly distributed */
			if (arb->pri_ff.e_keep_0 >= arb->pri_ff.e_keep_1 || !max_avg) { 
				Eatomic = arb->pri_ff.e_keep_0 * (total_pri - n_chg_pri * n_grant) * (max_avg ? 1 : 0.5); 
				SIM_print_stat_energy(SIM_strcat(path, "keep 0"), Eatomic, next_next_depth);
				SIM_res_path(path, next_path_len);
				Estruct += Eatomic;
			}

			if (arb->pri_ff.e_keep_0 < arb->pri_ff.e_keep_1 || !max_avg) { 
				Eatomic = arb->pri_ff.e_keep_1 * (total_pri - n_chg_pri * n_grant) * (max_avg ? 1 : 0.5); 
				SIM_print_stat_energy(SIM_strcat(path, "keep 1"), Eatomic, next_next_depth);
				SIM_res_path(path, next_path_len);
				Estruct += Eatomic;
			}

			Eatomic = arb->pri_ff.e_clock * total_pri;
			SIM_print_stat_energy(SIM_strcat(path, "clock"), Eatomic, next_next_depth);
			SIM_res_path(path, next_path_len);
			Estruct += Eatomic;

			SIM_print_stat_energy(path, Estruct, next_depth);
			SIM_res_path(path, path_len);
			Eavg += Estruct;

			/* based on above assumptions */
			if (max_avg)
				/* min(p,n/2)(n-1) + 2(n-1) */
				Eatomic = arb->e_chg_mint * (MIN(n_req, arb->req_width * 0.5) + 2) * (arb->req_width - 1);
			else
				/* p(n-1)/2 + (n-1)/2 */
				Eatomic = arb->e_chg_mint * (n_req + 1) * (arb->req_width - 1) * 0.5;
			SIM_print_stat_energy(SIM_strcat(path, "internal node"), Eatomic, next_depth);
			SIM_res_path(path, path_len);
			Eavg += Eatomic;
			break;

		case QUEUE_ARBITER:
			/* FIXME: what if n_req > 1? */
			Eavg = SIM_array_stat_energy(info, &arb->queue, n_req, n_grant, next_depth, SIM_strcat(path, "queue"), max_avg);
			SIM_res_path(path, path_len);
			break;


		default: printf ("error\n");	/* some error handler */
	}

	/* static power */
	Estatic = arb->I_static * Vdd * Period * SCALE_S;

	SIM_print_stat_energy(SIM_strcat(path, "static energy"), Estatic, next_depth);
	SIM_res_path(path, path_len);


	SIM_print_stat_energy(path, Eavg, print_depth);

	return Eavg;
}

