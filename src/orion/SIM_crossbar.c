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
#include <math.h>

#include "SIM_crossbar.h"
#include "SIM_static.h"
#include "SIM_time.h"
#include "SIM_util.h"

static double SIM_crossbar_in_cap(double wire_cap, u_int n_out, double n_seg, int connect_type, int trans_type, double *Nsize)
{
	double Ctotal = 0, Ctrans, psize, nsize;

	/* part 1: wire cap */
	Ctotal += wire_cap;

	/* part 2: drain cap of transmission gate or gate cap of tri-state gate */
	if (connect_type == TRANS_GATE) {
		/* FIXME: resizing strategy */
		nsize = Nsize ? *Nsize : Wmemcellr;
		psize = nsize * Wdecinvp / Wdecinvn;
		Ctrans = SIM_draincap(nsize, NCH, 1);
		if (trans_type == NP_GATE)
			Ctrans += SIM_draincap(psize, PCH, 1);
	}
	else if (connect_type == TRISTATE_GATE) {
		Ctrans = SIM_gatecap(Woutdrvnandn + Woutdrvnandp, 0) +
			SIM_gatecap(Woutdrvnorn + Woutdrvnorp, 0);
	}
	else {/* some error handler */}

	Ctotal += n_out * Ctrans;

	/* segmented crossbar */
	if (n_seg > 1) {
		Ctotal *= (n_seg + 1)/(n_seg * 2);
		/* input capacitance of tri-state buffer */
		Ctotal += (n_seg + 2)*(n_seg - 1)/(n_seg * 2)*(SIM_gatecap(Woutdrvnandn + Woutdrvnandp, 0) +
				SIM_gatecap(Woutdrvnorn + Woutdrvnorp, 0));
		/* output capacitance of tri-state buffer */
		Ctotal += (n_seg - 1) / 2 * (SIM_draincap(Woutdrivern, NCH, 1) +
				SIM_draincap(Woutdriverp, PCH, 1));
	}

	/* part 3: input driver */
	/* FIXME: how to specify timing? */
	psize = SIM_driver_size(Ctotal, Period / 3);
	nsize = psize * Wdecinvn / Wdecinvp;
	Ctotal += SIM_draincap(nsize, NCH, 1) + SIM_draincap(psize, PCH, 1) +
		SIM_gatecap(nsize + psize, 0);

	return Ctotal / 2;
}


static double SIM_crossbar_out_cap(double length, u_int n_in, double n_seg, int connect_type, int trans_type, double *Nsize)
{
	double Ctotal = 0, Ctrans, psize, nsize;

	/* part 1: wire cap */
	Ctotal += CC3metal * length;

	/* part 2: drain cap of transmission gate or tri-state gate */
	if (connect_type == TRANS_GATE) {
		/* FIXME: resizing strategy */
		if (Nsize) {
			/* FIXME: how to specify timing? */
			psize = SIM_driver_size(Ctotal, Period / 3);
			*Nsize = nsize = psize * Wdecinvn / Wdecinvp;
		}
		else {
			nsize = Wmemcellr;
			psize = nsize * Wdecinvp / Wdecinvn;
		}
		Ctrans = SIM_draincap(nsize, NCH, 1);
		if (trans_type == NP_GATE)
			Ctrans += SIM_draincap(psize, PCH, 1);
	}
	else if (connect_type == TRISTATE_GATE) {
		Ctrans = SIM_draincap(Woutdrivern, NCH, 1) + SIM_draincap(Woutdriverp, PCH, 1);
	}
	else {/* some error handler */}

	Ctotal += n_in * Ctrans;

	/* segmented crossbar */
	if (n_seg > 1) {
		Ctotal *= (n_seg + 1)/(n_seg * 2);
		/* input capacitance of tri-state buffer */
		Ctotal += (n_seg + 2)*(n_seg - 1)/(n_seg * 2)*(SIM_gatecap(Woutdrvnandn + Woutdrvnandp, 0) +
				SIM_gatecap(Woutdrvnorn + Woutdrvnorp, 0));
		/* output capacitance of tri-state buffer */
		Ctotal += (n_seg - 1) / 2 * (SIM_draincap(Woutdrivern, NCH, 1) +
				SIM_draincap(Woutdriverp, PCH, 1));
	}

	/* part 3: output driver */
	Ctotal += SIM_draincap(Woutdrivern, NCH, 1) + SIM_draincap(Woutdriverp, PCH, 1) +
		SIM_gatecap(Woutdrivern + Woutdriverp, 0);

	return Ctotal / 2;
}


/* cut-through crossbar only supports 4x4 now */
static double SIM_crossbar_io_cap(double length)
{
	double Ctotal = 0, psize, nsize;

	/* part 1: wire cap */
	Ctotal += CC3metal * length;

	/* part 2: gate cap of tri-state gate */
	Ctotal += 2 * (SIM_gatecap(Woutdrvnandn + Woutdrvnandp, 0) +
			SIM_gatecap(Woutdrvnorn + Woutdrvnorp, 0));

	/* part 3: drain cap of tri-state gate */
	Ctotal += 2 * (SIM_draincap(Woutdrivern, NCH, 1) + SIM_draincap(Woutdriverp, PCH, 1));

	/* part 4: input driver */
	/* FIXME: how to specify timing? */
	psize = SIM_driver_size(Ctotal, Period * 0.8);
	nsize = psize * Wdecinvn / Wdecinvp;
	Ctotal += SIM_draincap(nsize, NCH, 1) + SIM_draincap(psize, PCH, 1) +
		SIM_gatecap(nsize + psize, 0);

	/* part 5: output driver */
	Ctotal += SIM_draincap(Woutdrivern, NCH, 1) + SIM_draincap(Woutdriverp, PCH, 1) +
		SIM_gatecap(Woutdrivern + Woutdriverp, 0);

	/* HACK HACK HACK */
	/* this HACK is to count a 1:4 mux and a 4:1 mux, so we have a 5x5 crossbar */
	return Ctotal / 2 * 1.32;
}


static double SIM_crossbar_int_cap(u_int degree, int connect_type, int trans_type)
{
	double Ctotal = 0, Ctrans;

	if (connect_type == TRANS_GATE) {
		/* part 1: drain cap of transmission gate */
		/* FIXME: Wmemcellr and resize */
		Ctrans = SIM_draincap(Wmemcellr, NCH, 1);
		if (trans_type == NP_GATE)
			Ctrans += SIM_draincap(Wmemcellr * Wdecinvp / Wdecinvn, PCH, 1);
		Ctotal += (degree + 1) * Ctrans;
	}
	else if (connect_type == TRISTATE_GATE) {
		/* part 1: drain cap of tri-state gate */
		Ctotal += degree * (SIM_draincap(Woutdrivern, NCH, 1) + SIM_draincap(Woutdriverp, PCH, 1));

		/* part 2: gate cap of tri-state gate */
		Ctotal += SIM_gatecap(Woutdrvnandn + Woutdrvnandp, 0) +
			SIM_gatecap(Woutdrvnorn + Woutdrvnorp, 0);
	}
	else {/* some error handler */}

	return Ctotal / 2;
}


/* FIXME: segment control signals are not handled yet */
static double SIM_crossbar_ctr_cap(double length, u_int data_width, int prev_ctr, int next_ctr, u_int degree, int connect_type, int trans_type)
{
	double Ctotal = 0, Cgate;

	/* part 1: wire cap */
	Ctotal = Cmetal * length;

	/* part 2: gate cap of transmission gate or tri-state gate */
	if (connect_type == TRANS_GATE) {
		/* FIXME: Wmemcellr and resize */
		Cgate = SIM_gatecap(Wmemcellr, 0);
		if (trans_type == NP_GATE)
			Cgate += SIM_gatecap(Wmemcellr * Wdecinvp / Wdecinvn, 0);
	}
	else if (connect_type == TRISTATE_GATE) {
		Cgate = SIM_gatecap(Woutdrvnandn + Woutdrvnandp, 0) +
			SIM_gatecap(Woutdrvnorn + Woutdrvnorp, 0);
	}
	else {/* some error handler */}

	Ctotal += data_width * Cgate;

	/* part 3: inverter */
	if (!(connect_type == TRANS_GATE && trans_type == N_GATE && !prev_ctr))
		/* FIXME: need accurate size, use minimal size for now */
		Ctotal += SIM_draincap(Wdecinvn, NCH, 1) + SIM_draincap(Wdecinvp, PCH, 1) +
			SIM_gatecap(Wdecinvn + Wdecinvp, 0);

	/* part 4: drain cap of previous level control signal */
	if (prev_ctr)
		/* FIXME: need actual size, use decoder data for now */
		Ctotal += degree * SIM_draincap(WdecNORn, NCH, 1) + SIM_draincap(WdecNORp, PCH, degree);

	/* part 5: gate cap of next level control signal */
	if (next_ctr)
		/* FIXME: need actual size, use decoder data for now */
		Ctotal += SIM_gatecap(WdecNORn + WdecNORp, degree * 40 + 20);

	return Ctotal;
}


int SIM_crossbar_init(SIM_crossbar_t *crsbar, int model, u_int n_in, u_int n_out, u_int in_seg, u_int out_seg, u_int data_width, u_int degree, int connect_type, int trans_type, double in_len, double out_len, double *req_len)
{
	double in_length, out_length, ctr_length, Nsize, in_wire_cap, I_static;

	if ((crsbar->model = model) && model < CROSSBAR_MAX_MODEL) {
		crsbar->n_in = n_in;
		crsbar->n_out = n_out;
		crsbar->in_seg = 0;
		crsbar->out_seg = 0;
		crsbar->data_width = data_width;
		crsbar->degree = degree;
		crsbar->connect_type = connect_type;
		crsbar->trans_type = trans_type;
		/* redundant field */
		crsbar->mask = HAMM_MASK(data_width);

		crsbar->n_chg_in = crsbar->n_chg_int = crsbar->n_chg_out = crsbar->n_chg_ctr = 0;

		switch (model) {
			case MATRIX_CROSSBAR:
				crsbar->in_seg = in_seg;
				crsbar->out_seg = out_seg;

				/* FIXME: need accurate spacing */
				in_length = n_out * data_width * CrsbarCellWidth;
				out_length = n_in * data_width * CrsbarCellHeight;
				if (in_length < in_len) in_length = in_len;
				if (out_length < out_len) out_length = out_len;
				ctr_length = in_length / 2;
				if (req_len) *req_len = in_length;

				in_wire_cap = in_length * CC3metal;

				crsbar->e_chg_out = SIM_crossbar_out_cap(out_length, n_in, out_seg, connect_type, trans_type, &Nsize) * EnergyFactor;
				crsbar->e_chg_in = SIM_crossbar_in_cap(in_wire_cap, n_out, in_seg, connect_type, trans_type, &Nsize) * EnergyFactor;
				/* FIXME: wire length estimation, really reset? */
				/* control signal should reset after transmission is done, so no 1/2 */
				crsbar->e_chg_ctr = SIM_crossbar_ctr_cap(ctr_length, data_width, 0, 0, 0, connect_type, trans_type) * EnergyFactor;
				crsbar->e_chg_int = 0;

				/* static power */
				I_static = 0;
				/* tri-state buffers */
				I_static += ((Woutdrvnandp * (NAND2_TAB[0] + NAND2_TAB[1] + NAND2_TAB[2]) + Woutdrvnandn * NAND2_TAB[3]) / 4 +
						(Woutdrvnorp * NOR2_TAB[0] + Woutdrvnorn * (NOR2_TAB[1] + NOR2_TAB[2] + NOR2_TAB[3])) / 4 +
						Woutdrivern * NMOS_TAB[0] + Woutdriverp * PMOS_TAB[0]) * n_in * n_out * data_width;
				/* input driver */
				I_static += (Wdecinvn * NMOS_TAB[0] + Wdecinvp * PMOS_TAB[0]) * n_in * data_width;
				/* output driver */
				I_static += (Woutdrivern * NMOS_TAB[0] + Woutdriverp * PMOS_TAB[0]) * n_out * data_width;
				/* control signal inverter */
				I_static += (Wdecinvn * NMOS_TAB[0] + Wdecinvp * PMOS_TAB[0]) * n_in * n_out;
				crsbar->I_static = I_static;

				break;

			case MULTREE_CROSSBAR:
				/* input wire horizontal segment length */
				in_length = n_in * data_width * CrsbarCellWidth * (n_out / 2);
				in_wire_cap = in_length * CCmetal;
				/* input wire vertical segment length */
				in_length = n_in * data_width * (5 * Lamda) * (n_out / 2);
				in_wire_cap += in_length * CC3metal;

				ctr_length = n_in * data_width * CrsbarCellWidth * (n_out / 2) / 2;

				crsbar->e_chg_out = SIM_crossbar_out_cap(0, degree, 0, connect_type, trans_type, NULL) * EnergyFactor;
				crsbar->e_chg_in = SIM_crossbar_in_cap(in_wire_cap, n_out, 0, connect_type, trans_type, NULL) * EnergyFactor;
				crsbar->e_chg_int = SIM_crossbar_int_cap(degree, connect_type, trans_type) * EnergyFactor;

				/* redundant field */
				crsbar->depth = (u_int)ceil(log(n_in) / log(degree));

				/* control signal should reset after transmission is done, so no 1/2 */
				if (crsbar->depth == 1)
					/* only one level of control signal */
					crsbar->e_chg_ctr = SIM_crossbar_ctr_cap(ctr_length, data_width, 0, 0, degree, connect_type, trans_type) * EnergyFactor;
				else {
					/* first level and last level control signals */
					crsbar->e_chg_ctr = SIM_crossbar_ctr_cap(ctr_length, data_width, 0, 1, degree, connect_type, trans_type) * EnergyFactor +
						SIM_crossbar_ctr_cap(0, data_width, 1, 0, degree, connect_type, trans_type) * EnergyFactor;
					/* intermediate control signals */
					if (crsbar->depth > 2)
						crsbar->e_chg_ctr += (crsbar->depth - 2) * SIM_crossbar_ctr_cap(0, data_width, 1, 1, degree, connect_type, trans_type) * EnergyFactor;
				}

				/* static power */
				I_static = 0;
				/* input driver */
				I_static += (Wdecinvn * NMOS_TAB[0] + Wdecinvp * PMOS_TAB[0]) * n_in * data_width;
				/* output driver */
				I_static += (Woutdrivern * NMOS_TAB[0] + Woutdriverp * PMOS_TAB[0]) * n_out * data_width;
				/* mux */
				I_static += (WdecNORp * NOR2_TAB[0] + WdecNORn * (NOR2_TAB[1] + NOR2_TAB[2] + NOR2_TAB[3])) / 4 * (2 * n_in - 1) * n_out * data_width;
				/* control signal inverter */
				I_static += (Wdecinvn * NMOS_TAB[0] + Wdecinvp * PMOS_TAB[0]) * n_in * n_out;
				crsbar->I_static = I_static;

				break;

			case CUT_THRU_CROSSBAR:
				/* only support 4x4 now */
				in_length = 2 * data_width * MAX(CrsbarCellWidth, CrsbarCellHeight);
				ctr_length = in_length / 2;

				crsbar->e_chg_in = SIM_crossbar_io_cap(in_length) * EnergyFactor;
				crsbar->e_chg_out = 0;
				/* control signal should reset after transmission is done, so no 1/2 */
				crsbar->e_chg_ctr = SIM_crossbar_ctr_cap(ctr_length, data_width, 0, 0, 0, TRISTATE_GATE, 0) * EnergyFactor;
				crsbar->e_chg_int = 0;
				break;

			default: printf ("error\n");	/* some error handler */
		}

		return 0;
	}
	else
		return -1;
}


/* FIXME: MULTREE_CROSSBAR record missing */
int SIM_crossbar_record(SIM_crossbar_t *xb, int io, LIB_Type_max_uint new_data, LIB_Type_max_uint old_data, u_int new_port, u_int old_port)
{
	switch (xb->model) {
		case MATRIX_CROSSBAR:
			if (io)	/* input port */
				xb->n_chg_in += SIM_Hamming(new_data, old_data, xb->mask);
			else {		/* output port */
				xb->n_chg_out += SIM_Hamming(new_data, old_data, xb->mask);
				xb->n_chg_ctr += new_port != old_port;
			}
			break;

		case CUT_THRU_CROSSBAR:
			if (io)	/* input port */
				xb->n_chg_in += SIM_Hamming(new_data, old_data, xb->mask);
			else {		/* output port */
				xb->n_chg_ctr += new_port != old_port;
			}
			break;

		case MULTREE_CROSSBAR:
			break;

		default: printf ("error\n");	/* some error handler */
	}

	return 0;
}


double SIM_crossbar_report(SIM_crossbar_t *crsbar)
{
	return (crsbar->n_chg_in * crsbar->e_chg_in + crsbar->n_chg_out * crsbar->e_chg_out +
			crsbar->n_chg_int * crsbar->e_chg_int + crsbar->n_chg_ctr * crsbar->e_chg_ctr);
}


double SIM_crossbar_stat_energy(SIM_crossbar_t *crsbar, int print_depth, char *path, int max_avg, double n_data)
{
	double Eavg = 0, Eatomic, Estatic;
	int next_depth;
	u_int path_len;

	if (n_data > crsbar->n_out) {
		fprintf(stderr, "%s: overflow\n", path);
		n_data = crsbar->n_out;
	}

	next_depth = NEXT_DEPTH(print_depth);
	path_len = SIM_strlen(path);

	switch (crsbar->model) {
		case MATRIX_CROSSBAR:
		case CUT_THRU_CROSSBAR:
		case MULTREE_CROSSBAR:
			/* assume 0.5 data switch probability */
			Eatomic = crsbar->e_chg_in * crsbar->data_width * (max_avg ? 1 : 0.5) * n_data;
			SIM_print_stat_energy(SIM_strcat(path, "input"), Eatomic, next_depth);
			SIM_res_path(path, path_len);
			Eavg += Eatomic;

			Eatomic = crsbar->e_chg_out * crsbar->data_width * (max_avg ? 1 : 0.5) * n_data;
			SIM_print_stat_energy(SIM_strcat(path, "output"), Eatomic, next_depth);
			SIM_res_path(path, path_len);
			Eavg += Eatomic;

			Eatomic = crsbar->e_chg_ctr * n_data;
			SIM_print_stat_energy(SIM_strcat(path, "control"), Eatomic, next_depth);
			SIM_res_path(path, path_len);
			Eavg += Eatomic;

			if (crsbar->model == MULTREE_CROSSBAR && crsbar->depth > 1) {
				Eatomic = crsbar->e_chg_int * crsbar->data_width * (crsbar->depth - 1) * (max_avg ? 1 : 0.5) * n_data;
				SIM_print_stat_energy(SIM_strcat(path, "internal node"), Eatomic, next_depth);
				SIM_res_path(path, path_len);
				Eavg += Eatomic;
			}

			/* static power */
			Estatic = crsbar->I_static * Vdd * Period * SCALE_S;

			SIM_print_stat_energy(SIM_strcat(path, "static energy"), Estatic, next_depth);
			SIM_res_path(path, path_len);
			break;

		default: printf ("error\n");	/* some error handler */
	}

	SIM_print_stat_energy(path, Eavg, print_depth);

	return Eavg;
}

