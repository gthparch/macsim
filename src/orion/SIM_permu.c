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

/*
 * FIXME: (1) overestimate one-hot number generation
 *        (2) ignore data signal driving
 *        (3) OMFLIP internal node is always reachable now
 */

#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "SIM_permu.h"
#include "SIM_time.h"
#include "SIM_util.h"

/* GRP transistor size */
#define WgrpL1NOTn	(3 * Lamda)
#define WgrpL1NOTp	(6 * Lamda)
#define WgrpL2NOTn	(9 * Lamda)
#define WgrpL2NOTp	(18 * Lamda)
#define WgrpL3NOTn	(27 * Lamda)
#define WgrpL3NOTp	(54 * Lamda)
#define WgrpL4NOTn	(81 * Lamda)
#define WgrpL4NOTp	(162 * Lamda)
#define WgrpPASSn	(6 * Lamda)
#define WgrpPASSp	(6 * Lamda)
#define Wgrp2NANDn	(3 * Lamda)
#define Wgrp2NANDp	(3 * Lamda)
#define Wgrp2NORn	(3 * Lamda)
#define Wgrp2NORp	(12 * Lamda)

/* OMFLIP transistor size */
#define WomfPASSn	(6 * Lamda)
#define WomfPASSp	(6 * Lamda)
#define WomfNOTn	(3 * Lamda)
#define WomfNOTp	(6 * Lamda)


/*============================== omflip permutation unit ==============================*/

static double SIM_omflip_out_cap(void)
{
	double Ctotal = 0;

	/* part 1: fourth stage output cap */
	Ctotal += SIM_draincap(WomfPASSn, NCH, 1) + SIM_draincap(WomfPASSp, PCH, 1);

	/* part 2: output driver */
	Ctotal += SIM_draincap(WomfNOTn, NCH, 1) + SIM_draincap(WomfNOTp, PCH, 1) +
		SIM_gatecap(WomfNOTn + WomfNOTp, 0);

	return Ctotal;
}


static double SIM_omflip_in_cap(double length)
{
	double Ctotal = 0;

	/* part 1: input driver */
	Ctotal += SIM_draincap(WomfNOTn, NCH, 1) + SIM_draincap(WomfNOTp, PCH, 1) +
		SIM_gatecap(WomfNOTn + WomfNOTp, 0);

	/* part 2: wire cap */
	Ctotal += CCmetal * length;

	/* part 3: first stage input cap */
	Ctotal += 2 * SIM_draincap(WomfPASSn, NCH, 1) + SIM_draincap(WomfPASSp, PCH, 1);

	return Ctotal;
}


static double SIM_omflip_stg_cap(double length)
{
	double Ctotal = 0;

	/* part 1: previous stage output cap */
	Ctotal += SIM_draincap(WomfPASSn, NCH, 1) + SIM_draincap(WomfPASSp, PCH, 1);

	/* part 2: inverter */
	Ctotal += SIM_draincap(WomfNOTn, NCH, 1) + SIM_draincap(WomfNOTp, PCH, 1) +
		SIM_gatecap(WomfNOTn + WomfNOTp, 0);

	/* part 3: wire cap */
	Ctotal += CCmetal * length;

	/* part 4: next stage input cap */
	Ctotal += 2 * SIM_draincap(WomfPASSn, NCH, 1) + SIM_draincap(WomfPASSp, PCH, 1);

	return Ctotal;
}


static double SIM_omflip_int_cap(void)
{
	double Ctotal = 0;

	/* part 1: inverter */
	Ctotal += SIM_draincap(WomfNOTn, NCH, 1) + SIM_draincap(WomfNOTp, PCH, 1) +
		SIM_gatecap(WomfNOTn + WomfNOTp, 0);

	/* part 2: drain cap of pass transistors */
	Ctotal += SIM_draincap(WomfPASSn, NCH, 1) + 2 * SIM_draincap(WomfPASSp, PCH, 1);

	return Ctotal;
}


/* pass signal and control signal cap */
static double SIM_omflip_ctr_cap(u_int n_ctr, double length)
{
	double Ctotal = 0;

	/* part 1: gate cap of pass transistors */
	Ctotal += n_ctr * SIM_gatecap(WomfPASSn + WomfPASSp, 0);

	/* part 2: wire cap */
	Ctotal += CCmetal * length;

	return Ctotal;
}


static int SIM_omflip_reset(SIM_omflip_t *omflip)
{
	omflip->pass = ALL_PASS;
	omflip->ctr[0] = omflip->ctr[1] = omflip->ctr[2] = omflip->ctr[3] = 0;
	omflip->in = omflip->out = 0;
	omflip->stg[0] = omflip->stg[1] = omflip->stg[2] = 0;
	omflip->inn[0] = omflip->inn[1] = omflip->inn[2] = omflip->inn[3] = 0;

	return 0;
}


inline static LIB_Type_max_uint SIM_omflip_omega(LIB_Type_max_uint in, u_int width, LIB_Type_max_uint mask, int pass, LIB_Type_max_uint ctr)
{
	LIB_Type_max_uint low_in, high_in, low_out, high_out;
	LIB_Type_max_uint rval = 0;
	u_int i;

	if (pass)
		/* WHS: equivalent in switch activity (~in) */
		return in;
	else {
		low_in = in & mask;
		high_in = (in >> width) & mask;
		low_out = low_in & ~ctr | high_in & ctr;
		high_out = low_in & ctr | high_in & ~ctr;

		/* mangle low half and high half */
		for (i = 0; i < width; i++) {
			rval += (low_out & 1) << (i + i) | (high_out & 1) << (i + i + 1);
			low_out = low_out >> 1;
			high_out = high_out >> 1;
		}

		return rval;
	}
}


inline static LIB_Type_max_uint SIM_omflip_flip(LIB_Type_max_uint in, u_int width, int pass, LIB_Type_max_uint ctr)
{
	LIB_Type_max_uint low_in = 0, high_in = 0, low_out, high_out;
	u_int i;

	if (pass)
		/* WHS: equivalent in switch activity (~in) */
		return in;
	else {
		/* split low half and high half */
		for (i = 0; i < width; i++) {
			low_in += (in & 1) << i;
			in = in >> 1;
			high_in += (in & 1) << i;
			in = in >> 1;
		}

		low_out = low_in & ~ctr | high_in & ctr;
		high_out = low_in & ctr | high_in & ~ctr;

		return low_out + (high_out << width);
	}
}

/*============================== omflip permutation unit ==============================*/



/*============================== GRP permutation unit ==============================*/

static double SIM_grp_in_cap(void)
{
	return 2 * SIM_gatecap(Wgrp2NANDn + Wgrp2NANDp, 0);
}


static double SIM_grp_zin_cap(u_int stg)
{
	double Ctotal = 0;
	u_int n_track;

	/* part 1: previous stage output cap */
	if (stg == 1)
		Ctotal += SIM_draincap(Wgrp2NANDn, NCH, 2) + 2 * SIM_draincap(Wgrp2NANDp, PCH, 1);
	else {
		n_track = (1 << stg - 2) + 1;
		/* wire cap */
		/* WHS: 2 tracks per unit height */
		Ctotal += n_track * 2 * WordlineSpacing * CCmetal;
		/* drain cap of pass transistors */
		Ctotal += n_track * (SIM_draincap(WgrpPASSn, NCH, 1) + SIM_draincap(WgrpPASSp, PCH, 1));
	}

	/* part 2: this stage wire cap */
	n_track = (1 << stg - 1) + 1;
	/* WHS: use average track: (3/4 * n), and include horizontal wires */
	Ctotal += (0.75 * n_track * 2 * WordlineSpacing + (n_track - 1) * BitlineSpacing) * CCmetal;

	/* part 3: this stage drain cap */
	/* WHS: use average track: (3/4 * n) */
	Ctotal += 0.75 * n_track * (SIM_draincap(WgrpPASSn, NCH, 1) + SIM_draincap(WgrpPASSp, PCH, 1));

	return Ctotal;
}


static double SIM_grp_lin_cap(u_int stg)
{
	double Ctotal = 0;
	u_int n_track;

	/* part 1: previous stage output cap */
	if (stg == 1) {
		/* input inverter */
		Ctotal += SIM_draincap(WgrpL1NOTn, NCH, 1) + SIM_draincap(WgrpL1NOTp, PCH, 1) +
			SIM_gatecap(WgrpL1NOTn + WgrpL1NOTp, 0);
		/* gate cap of NAND gate */
		Ctotal += 0.5 * SIM_gatecap(Wgrp2NANDn + Wgrp2NANDp, 0);
	}
	else {
		n_track = (1 << stg - 2) + 1;
		/* wire cap */
		Ctotal += n_track * 2 * WordlineSpacing * CCmetal;
		/* drain cap of pass transistors */
		Ctotal += n_track * (SIM_draincap(WgrpPASSn, NCH, 1) + SIM_draincap(WgrpPASSp, PCH, 1));
	}

	/* part 2: this stage wire cap */
	n_track = (1 << stg + 1) + 1;
	/* WHS: 2 tracks per unit width */
	Ctotal += n_track * 2 * BitlineSpacing * CCmetal;

	/* part 3: this stage gate cap */
	Ctotal += n_track * SIM_gatecap(WgrpPASSn + WgrpPASSp, 0);

	/* part 4: this stage wire driving */
	if (stg > 1)
		Ctotal += SIM_draincap(WgrpL1NOTn, NCH, 1) + SIM_draincap(WgrpL1NOTp, PCH, 1) +
			SIM_gatecap(WgrpL1NOTn + WgrpL1NOTp, 0) +
			SIM_draincap(WgrpL2NOTn, NCH, 1) + SIM_draincap(WgrpL2NOTp, PCH, 1) +
			SIM_gatecap(WgrpL2NOTn + WgrpL2NOTp, 0) +
			SIM_draincap(WgrpL3NOTn, NCH, 1) + SIM_draincap(WgrpL3NOTp, PCH, 1) +
			SIM_gatecap(WgrpL3NOTn + WgrpL3NOTp, 0);
	if (stg > 4)
		Ctotal += SIM_draincap(WgrpL4NOTn, NCH, 1) + SIM_draincap(WgrpL4NOTp, PCH, 1) +
			SIM_gatecap(WgrpL4NOTn + WgrpL4NOTp, 0);

	return Ctotal;
}


static double SIM_grp_rin_cap(u_int stg)
{
	double Ctotal = 0;
	u_int n_track;

	/* part 1: previous stage output cap */
	if (stg == 1) {
		/* input inverter */
		Ctotal += SIM_draincap(WgrpL1NOTn, NCH, 1) + SIM_draincap(WgrpL1NOTp, PCH, 1) +
			SIM_gatecap(WgrpL1NOTn + WgrpL1NOTp, 0);
		/* gate cap of NAND gate */
		Ctotal += 0.5 * SIM_gatecap(Wgrp2NANDn + Wgrp2NANDp, 0);
	}
	else {
		n_track = (1 << stg - 2) + 1;
		/* wire cap */
		Ctotal += n_track * 2 * WordlineSpacing * CCmetal;
		/* drain cap of pass transistors */
		Ctotal += n_track * (SIM_draincap(WgrpPASSn, NCH, 1) + SIM_draincap(WgrpPASSp, PCH, 1));
	}

	/* part 2: this stage wire cap */
	n_track = (1 << stg - 1) + 1;
	Ctotal += (n_track * 2 * WordlineSpacing + (n_track - 1) * 2 * BitlineSpacing) * CCmetal;

	/* part 3: this stage drain cap */
	Ctotal += n_track * (SIM_draincap(WgrpPASSn, NCH, 1) + SIM_draincap(WgrpPASSp, PCH, 1));

	return Ctotal;
}


static double SIM_grp_oin_cap(u_int stg)
{
	double Ctotal = 0;
	u_int n_track;

	/* part 1: previous stage output cap */
	if (stg == 1)
		Ctotal += SIM_draincap(Wgrp2NANDn, NCH, 2) + 2 * SIM_draincap(Wgrp2NANDp, PCH, 1);
	else {
		n_track = (1 << stg - 2) + 1;
		/* wire cap */
		Ctotal += n_track * 2 * WordlineSpacing * CCmetal;
		/* drain cap of pass transistors */
		Ctotal += n_track * (SIM_draincap(WgrpPASSn, NCH, 1) + SIM_draincap(WgrpPASSp, PCH, 1));
	}

	/* gate cap of NOR gate */
	Ctotal += SIM_gatecap(Wgrp2NORn + Wgrp2NORp, 0);

	return Ctotal;
}


static double SIM_grp_out_cap(void)
{
	return (2 * SIM_draincap(Wgrp2NORn, NCH, 1) + SIM_draincap(Wgrp2NORp, PCH, 2));
}


static int SIM_grp_reset(SIM_grp_t *grp)
{
	grp->in = grp->out = grp->zoin = grp->woin = 0;
	grp->zin[0] = grp->zin[1] = grp->zin[2] = grp->zin[3] = grp->zin[4] = grp->zin[5] = 0;
	grp->win[0] = grp->win[1] = grp->win[2] = grp->win[3] = grp->win[4] = grp->win[5] = 0;
	memset(grp->lin, 0, sizeof(u_int) * N_ONEHOT);
	memset(grp->rin, 0, sizeof(u_int) * N_ONEHOT);

	return 0;
}

/*============================== GRP permutation unit ==============================*/



int SIM_permu_init(SIM_permu_t *permu, int model, u_int data_width)
{
	double unit_height = 3 * WordlineSpacing;
	double unit_dist = 1.5 * data_width * BitlineSpacing;
	u_int stg;

	assert(data_width && data_width <= 8 * sizeof(LIB_Type_max_uint) && !(data_width % 2));

	switch (permu->model = model) {
		case OMFLIP_PERMU:
			permu->u.omflip.data_width = data_width;
			permu->u.omflip.n_chg_in = permu->u.omflip.n_chg_out = permu->u.omflip.n_chg_stg = 0;
			permu->u.omflip.n_chg_int = permu->u.omflip.n_chg_pass = permu->u.omflip.n_chg_ctr = 0;
			/* initial state */
			SIM_omflip_reset(&permu->u.omflip);
			/* redundant field */
			permu->u.omflip.data_mask = HAMM_MASK(data_width);
			permu->u.omflip.ctr_mask = HAMM_MASK(data_width / 2);

			/* WHS: use average line length */
			permu->u.omflip.e_chg_in = SIM_omflip_in_cap(unit_dist + (0.5 + 0.25 * data_width) * unit_height) / 2 * EnergyFactor;
			permu->u.omflip.e_chg_out = SIM_omflip_out_cap() / 2 * EnergyFactor;
			/* WHS: use average line length */
			permu->u.omflip.e_chg_stg = SIM_omflip_stg_cap(unit_dist + (0.5 + 1.25 * data_width) * WordlineSpacing) / 2 * EnergyFactor;
			permu->u.omflip.e_chg_int = SIM_omflip_int_cap() / 2 * EnergyFactor;
			permu->u.omflip.e_chg_pass = SIM_omflip_ctr_cap(data_width, data_width * unit_height) / 2 * EnergyFactor;
			/* WHS: use average control line length */
			permu->u.omflip.e_chg_ctr = SIM_omflip_ctr_cap(2, data_width * unit_height / 2) / 2 * EnergyFactor;
			break;

		case GRP_PERMU:
			permu->u.grp.data_width = data_width;
			permu->u.grp.n_chg_in = permu->u.grp.n_chg_oin = permu->u.grp.n_chg_out = 0;
			for (stg = 0; stg < 6; stg ++) {
				permu->u.grp.n_chg_zin[stg] = 0;
				permu->u.grp.n_chg_lin[stg] = 0;
				permu->u.grp.n_chg_rin[stg] = 0;
			}
			/* initial state */
			SIM_grp_reset(&permu->u.grp);
			/* redundant field */
			permu->u.grp.n_stg = SIM_logtwo(data_width);
			permu->u.grp.mask = HAMM_MASK(data_width);

			permu->u.grp.e_chg_in = SIM_grp_in_cap() / 2 * EnergyFactor;
			permu->u.grp.e_chg_out = SIM_grp_out_cap() / 2 * EnergyFactor;

			for (stg = 1; stg <= permu->u.grp.n_stg; stg++) {
				permu->u.grp.e_chg_zin[stg - 1] = SIM_grp_zin_cap(stg) / 2 * EnergyFactor;
				/* 2 one-hot signals always switch simultaneously, and shared by z/w */
				permu->u.grp.e_chg_lin[stg - 1] = SIM_grp_lin_cap(stg) * 2 * EnergyFactor;
				permu->u.grp.e_chg_rin[stg - 1] = SIM_grp_rin_cap(stg) * 2 * EnergyFactor;
			}
			permu->u.grp.e_chg_oin = SIM_grp_oin_cap(stg) / 2 * EnergyFactor;
			for (; stg <= 6; stg++) {
				permu->u.grp.e_chg_zin[stg - 1] = 0;
				permu->u.grp.e_chg_lin[stg - 1] = 0;
				permu->u.grp.e_chg_rin[stg - 1] = 0;
			}
			break;

		default: return -1;
	}

	return 0;
}


inline static int SIM_omflip_record(SIM_omflip_t *omflip, LIB_Type_max_uint in, u_int mode, LIB_Type_max_uint op, int reset)
{
	u_int pass = 0;
	LIB_Type_max_uint stg, ctr;

	if (reset) SIM_omflip_reset(omflip);

	/* WHS: use AEAP policy */

	/* first stage */
	if (is_omega(mode & FIRST_STG)) {
		ctr = op & omflip->ctr_mask;
		omflip->n_chg_ctr += SIM_Hamming(ctr, omflip->ctr[0], omflip->ctr_mask);
		omflip->n_chg_int += SIM_Hamming(in, omflip->inn[0], omflip->data_mask);
		omflip->ctr[0] = ctr;
		omflip->inn[0] = in;
		stg = SIM_omflip_omega(in, omflip->data_width / 2, omflip->ctr_mask, 0, ctr);
	}
	else {
		pass = pass | FIRST_STG;
		stg = SIM_omflip_omega(in, omflip->data_width / 2, omflip->ctr_mask, 1, 0);
	}
	omflip->n_chg_in += SIM_Hamming(in, omflip->in, omflip->data_mask);
	omflip->n_chg_stg += SIM_Hamming(stg, omflip->stg[0], omflip->data_mask);
	omflip->in = in;
	omflip->stg[0] = stg;

	/* second stage */
	if (is_flip(mode & FIRST_STG) || is_flip(mode & SECOND_STG)) {
		if (is_flip(mode & FIRST_STG))
			ctr = op & omflip->ctr_mask;
		else
			ctr = (op >> omflip->data_width / 2) & omflip->ctr_mask;
		omflip->n_chg_ctr += SIM_Hamming(ctr, omflip->ctr[1], omflip->ctr_mask);
		omflip->n_chg_int += SIM_Hamming(stg, omflip->inn[1], omflip->data_mask);
		omflip->ctr[1] = ctr;
		omflip->inn[1] = stg;
		stg = SIM_omflip_flip(stg, omflip->data_width / 2, 0, ctr);
	}
	else {
		pass = pass | SECOND_STG;
		stg = SIM_omflip_flip(stg, omflip->data_width / 2, 1, 0);
	}
	omflip->n_chg_stg += SIM_Hamming(stg, omflip->stg[1], omflip->data_mask);
	omflip->stg[1] = stg;

	/* third stage */
	if (is_flip(mode & FIRST_STG) && is_flip(mode & SECOND_STG)) {
		ctr = (op >> omflip->data_width / 2) & omflip->ctr_mask;
		omflip->n_chg_ctr += SIM_Hamming(ctr, omflip->ctr[2], omflip->ctr_mask);
		omflip->n_chg_int += SIM_Hamming(stg, omflip->inn[2], omflip->data_mask);
		omflip->ctr[2] = ctr;
		omflip->inn[2] = stg;
		stg = SIM_omflip_flip(stg, omflip->data_width / 2, 0, ctr);
	}
	else {
		pass = pass | THIRD_STG;
		stg = SIM_omflip_flip(stg, omflip->data_width / 2, 1, 0);
	}
	omflip->n_chg_stg += SIM_Hamming(stg, omflip->stg[2], omflip->data_mask);
	omflip->stg[2] = stg;

	/* fourth stage */
	if (is_omega(mode & SECOND_STG)) {
		ctr = (op >> omflip->data_width / 2) & omflip->ctr_mask;
		omflip->n_chg_ctr += SIM_Hamming(ctr, omflip->ctr[3], omflip->ctr_mask);
		omflip->n_chg_int += SIM_Hamming(stg, omflip->inn[3], omflip->data_mask);
		omflip->ctr[3] = ctr;
		omflip->inn[3] = stg;
		stg = SIM_omflip_omega(stg, omflip->data_width / 2, omflip->ctr_mask, 0, ctr);
	}
	else {
		pass = pass | FOURTH_STG;
		stg = SIM_omflip_omega(stg, omflip->data_width / 2, omflip->ctr_mask, 1, 0);
	}
	omflip->n_chg_out += SIM_Hamming(stg, omflip->out, omflip->data_mask);
	omflip->out = stg;

	omflip->n_chg_pass += SIM_Hamming(pass, omflip->pass, ALL_PASS);
	omflip->pass = pass;

	return 0;
}


inline static LIB_Type_max_uint SIM_omflip_record_test(SIM_omflip_t *omflip, LIB_Type_max_uint in, u_int mode, LIB_Type_max_uint op)
{
	u_int pass = 0;
	LIB_Type_max_uint stg, ctr;

	/* WHS: use AEAP policy */

	/* first stage */
	if (is_omega(mode & FIRST_STG)) {
		ctr = op & omflip->ctr_mask;
		omflip->n_chg_ctr = SIM_Hamming(ctr, omflip->ctr[0], omflip->ctr_mask);
		omflip->n_chg_int = SIM_Hamming(in, omflip->inn[0], omflip->data_mask);
		stg = SIM_omflip_omega(in, omflip->data_width / 2, omflip->ctr_mask, 0, ctr);
	}
	else {
		pass = pass | FIRST_STG;
		stg = SIM_omflip_omega(in, omflip->data_width / 2, omflip->ctr_mask, 1, 0);
	}
	omflip->n_chg_in = SIM_Hamming(in, omflip->in, omflip->data_mask);
	omflip->n_chg_stg = SIM_Hamming(stg, omflip->stg[0], omflip->data_mask);

	/* second stage */
	if (is_flip(mode & FIRST_STG) || is_flip(mode & SECOND_STG)) {
		if (is_flip(mode & FIRST_STG))
			ctr = op & omflip->ctr_mask;
		else
			ctr = (op >> omflip->data_width / 2) & omflip->ctr_mask;
		omflip->n_chg_ctr += SIM_Hamming(ctr, omflip->ctr[1], omflip->ctr_mask);
		omflip->n_chg_int += SIM_Hamming(stg, omflip->inn[1], omflip->data_mask);
		stg = SIM_omflip_flip(stg, omflip->data_width / 2, 0, ctr);
	}
	else {
		pass = pass | SECOND_STG;
		stg = SIM_omflip_flip(stg, omflip->data_width / 2, 1, 0);
	}
	omflip->n_chg_stg += SIM_Hamming(stg, omflip->stg[1], omflip->data_mask);

	/* third stage */
	if (is_flip(mode & FIRST_STG) && is_flip(mode & SECOND_STG)) {
		ctr = (op >> omflip->data_width / 2) & omflip->ctr_mask;
		omflip->n_chg_ctr += SIM_Hamming(ctr, omflip->ctr[2], omflip->ctr_mask);
		omflip->n_chg_int += SIM_Hamming(stg, omflip->inn[2], omflip->data_mask);
		stg = SIM_omflip_flip(stg, omflip->data_width / 2, 0, ctr);
	}
	else {
		pass = pass | THIRD_STG;
		stg = SIM_omflip_flip(stg, omflip->data_width / 2, 1, 0);
	}
	omflip->n_chg_stg += SIM_Hamming(stg, omflip->stg[2], omflip->data_mask);

	/* fourth stage */
	if (is_omega(mode & SECOND_STG)) {
		ctr = (op >> omflip->data_width / 2) & omflip->ctr_mask;
		omflip->n_chg_ctr += SIM_Hamming(ctr, omflip->ctr[3], omflip->ctr_mask);
		omflip->n_chg_int += SIM_Hamming(stg, omflip->inn[3], omflip->data_mask);
		stg = SIM_omflip_omega(stg, omflip->data_width / 2, omflip->ctr_mask, 0, ctr);
	}
	else {
		pass = pass | FOURTH_STG;
		stg = SIM_omflip_omega(stg, omflip->data_width / 2, omflip->ctr_mask, 1, 0);
	}
	omflip->n_chg_out = SIM_Hamming(stg, omflip->out, omflip->data_mask);

	omflip->n_chg_pass = SIM_Hamming(pass, omflip->pass, ALL_PASS);

	return stg;
}


/* stg <= 6 */
inline static LIB_Type_max_uint SIM_grp_stg(LIB_Type_max_uint zin, u_int *lin, u_int width, u_int stg, int zw)
{
	LIB_Type_max_uint rval, mask;
	u_int i, p_wid;

	rval = 0;
	p_wid = 1 << stg - 1;
	mask = HAMM_MASK(p_wid);

	for (i = 0; i < width >> stg; i++) {
		rval += (zin & mask) << (i << stg);
		zin = zin >> p_wid;
		if (zw == ZBIT)
			rval += ((zin & mask) << p_wid - lin[i]) << (i << stg);
		else	/* WBIT */
			rval += ((zin & mask) << lin[i]) << (i << stg);
		zin = zin >> p_wid;
	}

	return rval;
}


inline static int SIM_grp_record(SIM_grp_t *grp, LIB_Type_max_uint in, LIB_Type_max_uint op, int reset)
{
	u_int i, stg, base = 0, lin[32], rin[32];
	LIB_Type_max_uint zin, win;

	if (reset) SIM_grp_reset(grp);

	/* stage 0 */
	grp->n_chg_in += SIM_Hamming(in, grp->in, grp->mask);
	grp->in = in;

	zin = in & ~op;
	/* WHS: equivalent in switch activity (reverse bit order) */
	win = in & op;

	/* stage 1 */
	grp->n_chg_zin[0] += SIM_Hamming(zin, grp->zin[0], grp->mask) +
		SIM_Hamming(win, grp->win[0], grp->mask);
	grp->zin[0] = zin;
	grp->win[0] = win;

	for (i = 0; i < grp->data_width / 2; i++) {
		lin[i] = op & 1;
		op = op >> 1;
		rin[i] = op & 1;
		op = op >> 1;

		grp->n_chg_lin[0] += lin[i] != grp->lin[i];
		grp->n_chg_rin[0] += rin[i] != grp->rin[i];
		grp->lin[i] = lin[i];
		grp->rin[i] = rin[i];
	}
	zin = SIM_grp_stg(zin, lin, grp->data_width, 1, ZBIT);
	win = SIM_grp_stg(win, lin, grp->data_width, 1, WBIT);

	/* stage 2-6 */
	for (stg = 2; 1 << stg <= grp->data_width; stg++) {
		grp->n_chg_zin[stg - 1] += SIM_Hamming(zin, grp->zin[stg - 1], grp->mask) +
			SIM_Hamming(win, grp->win[stg - 1], grp->mask);
		grp->zin[stg - 1] = zin;
		grp->win[stg - 1] = win;
		/* update array base index */
		base += 64 >> stg - 1;

		for (i = 0; i < grp->data_width >> stg; i++) {
			lin[i] = lin[i + i] + rin[i + i];
			rin[i] = lin[i + i + 1] + rin[i + i + 1];

			grp->n_chg_lin[stg - 1] += lin[i] != grp->lin[i + base];
			grp->n_chg_rin[stg - 1] += rin[i] != grp->rin[i + base];
			grp->lin[i + base] = lin[i];
			grp->rin[i + base] = rin[i];
		}

		zin = SIM_grp_stg(zin, lin, grp->data_width, stg, ZBIT);
		win = SIM_grp_stg(win, lin, grp->data_width, stg, WBIT);
	}

	/* output stage */
	grp->n_chg_oin += SIM_Hamming(zin, grp->zoin, grp->mask) +
		SIM_Hamming(win, grp->woin, grp->mask);
	grp->zoin = zin;
	grp->woin = win;

	zin += win << grp->data_width - lin[0] - rin[0];
	grp->n_chg_out += SIM_Hamming(zin, grp->out, grp->mask);
	grp->out = zin;

	return 0;
}


inline static LIB_Type_max_uint SIM_grp_record_test(SIM_grp_t *grp, LIB_Type_max_uint in, LIB_Type_max_uint op)
{
	u_int i, stg, base = 0, lin[32], rin[32];
	LIB_Type_max_uint zin, win;

	/* stage 0 */
	grp->n_chg_in = SIM_Hamming(in, grp->in, grp->mask);

	zin = in & ~op;
	/* WHS: equivalent in switch activity (reverse bit order) */
	win = in & op;

	/* stage 1 */
	grp->n_chg_zin[0] = SIM_Hamming(zin, grp->zin[0], grp->mask) +
		SIM_Hamming(win, grp->win[0], grp->mask);
	grp->n_chg_lin[0] = 0;
	grp->n_chg_rin[0] = 0;

	for (i = 0; i < grp->data_width / 2; i++) {
		lin[i] = op & 1;
		op = op >> 1;
		rin[i] = op & 1;
		op = op >> 1;

		grp->n_chg_lin[0] += lin[i] != grp->lin[i];
		grp->n_chg_rin[0] += rin[i] != grp->rin[i];
	}
	zin = SIM_grp_stg(zin, lin, grp->data_width, 1, ZBIT);
	win = SIM_grp_stg(win, lin, grp->data_width, 1, WBIT);

	/* stage 2-6 */
	for (stg = 2; 1 << stg <= grp->data_width; stg++) {
		grp->n_chg_zin[stg - 1] = SIM_Hamming(zin, grp->zin[stg - 1], grp->mask) +
			SIM_Hamming(win, grp->win[stg - 1], grp->mask);
		grp->n_chg_lin[stg - 1] = 0;
		grp->n_chg_rin[stg - 1] = 0;
		/* update array base index */
		base += 64 >> stg - 1;

		for (i = 0; i < grp->data_width >> stg; i++) {
			lin[i] = lin[i + i] + rin[i + i];
			rin[i] = lin[i + i + 1] + rin[i + i + 1];

			grp->n_chg_lin[stg - 1] += lin[i] != grp->lin[i + base];
			grp->n_chg_rin[stg - 1] += rin[i] != grp->rin[i + base];
		}

		zin = SIM_grp_stg(zin, lin, grp->data_width, stg, ZBIT);
		win = SIM_grp_stg(win, lin, grp->data_width, stg, WBIT);
	}

	/* output stage */
	grp->n_chg_oin = SIM_Hamming(zin, grp->zoin, grp->mask) +
		SIM_Hamming(win, grp->woin, grp->mask);

	zin += win << grp->data_width - lin[0] - rin[0];
	grp->n_chg_out = SIM_Hamming(zin, grp->out, grp->mask);

	return zin;
}


int SIM_permu_record(SIM_permu_t *permu, LIB_Type_max_uint in, u_int mode, LIB_Type_max_uint op, int reset)
{
	switch (permu->model) {
		case OMFLIP_PERMU:
			return SIM_omflip_record(&permu->u.omflip, in, mode, op, reset);

		case GRP_PERMU:
			return SIM_grp_record(&permu->u.grp, in, op, reset);

		default: return -1;
	}
}


LIB_Type_max_uint SIM_permu_record_test(SIM_permu_t *permu, LIB_Type_max_uint in, u_int mode, LIB_Type_max_uint op)
{
	switch (permu->model) {
		case OMFLIP_PERMU:
			return SIM_omflip_record_test(&permu->u.omflip, in, mode, op);

		case GRP_PERMU:
			return SIM_grp_record_test(&permu->u.grp, in, op);

		default: printf ("error\n");	/* some error handler */
	}
}


double SIM_permu_report(SIM_permu_t *permu)
{
	u_int i;
	double rval;

	switch (permu->model) {
		case OMFLIP_PERMU:
			return (permu->u.omflip.n_chg_in * permu->u.omflip.e_chg_in +
					permu->u.omflip.n_chg_out * permu->u.omflip.e_chg_out +
					permu->u.omflip.n_chg_stg * permu->u.omflip.e_chg_stg +
					permu->u.omflip.n_chg_int * permu->u.omflip.e_chg_int +
					permu->u.omflip.n_chg_pass * permu->u.omflip.e_chg_pass +
					permu->u.omflip.n_chg_ctr * permu->u.omflip.e_chg_ctr);

		case GRP_PERMU:
			rval = permu->u.grp.n_chg_in * permu->u.grp.e_chg_in +
				permu->u.grp.n_chg_oin * permu->u.grp.e_chg_oin +
				permu->u.grp.n_chg_out * permu->u.grp.e_chg_out;
			for (i = 0; i < 6; i++)
				rval += permu->u.grp.n_chg_zin[i] * permu->u.grp.e_chg_zin[i] +
					permu->u.grp.n_chg_lin[i] * permu->u.grp.e_chg_lin[i] +
					permu->u.grp.n_chg_rin[i] * permu->u.grp.e_chg_rin[i];

			return rval;

		default: return -1;
	}
}


double SIM_permu_max_energy(SIM_permu_t *permu)
{
	double Emax = 0;
	u_int i;

	switch (permu->model) {
		case OMFLIP_PERMU:
			/* input */
			Emax += permu->u.omflip.e_chg_in * permu->u.omflip.data_width;
			/* intermediate stages */
			Emax += permu->u.omflip.e_chg_stg * permu->u.omflip.data_width * 3;
			/* internal nodes */
			Emax += permu->u.omflip.e_chg_int * permu->u.omflip.data_width * 4;
			/* output */
			Emax += permu->u.omflip.e_chg_out * permu->u.omflip.data_width;

			/* pass */
			Emax += permu->u.omflip.e_chg_pass * 4;
			/* operation */
			Emax += permu->u.omflip.e_chg_ctr * permu->u.omflip.data_width;
			return Emax;

			/* alternative WITH SIDE EFFECT */
			//SIM_omflip_record(&permu->u.omflip, 0, 0, 0, 1);
			//SIM_omflip_record_test(&permu->u.omflip, permu->u.omflip.data_mask, 0x03, permu->u.omflip.data_mask);
			//return SIM_permu_report(permu);

		case GRP_PERMU:
			/* input */
			//Emax += permu->u.grp.e_chg_in * permu->u.grp.data_width;
			for (i = 0; i < permu->u.grp.n_stg; i++) {
				/* z/w bit */
				Emax += permu->u.grp.e_chg_zin[i] * permu->u.grp.data_width * 2;
				/* left one-hot number */
				Emax += permu->u.grp.e_chg_lin[i] * (permu->u.grp.data_width >> i + 1);
				/* right one-hot number */
				Emax += permu->u.grp.e_chg_rin[i] * (permu->u.grp.data_width >> i + 1);
			}
			/* OR input */
			Emax += permu->u.grp.e_chg_oin * permu->u.grp.data_width * 2;
			/* output */
			//Emax += permu->u.grp.e_chg_out * permu->u.grp.data_width;
			return Emax;

			/* alternative WITH SIDE EFFECT */
			//SIM_grp_record(&permu->u.grp, permu->u.grp.mask, 0, 1);
			//SIM_grp_record_test(&permu->u.grp, permu->u.grp.mask, permu->u.grp.mask);
			//return SIM_permu_report(permu);

		default: return -1;
	}
}

