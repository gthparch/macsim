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

#include <math.h>
#include <stdio.h>

#include "SIM_parameter.h"
#include "SIM_misc.h"
#include "SIM_static.h"
#include "SIM_time.h"
#include "SIM_util.h"


/*============================== dependency check logic ==============================*/

/* Register renaming unit should be a separate module, which falls in array structure
 * category.  Dependency check logic is just the tag comparator of that module.
 * Following code is excerpted from Wattch as a reference in the future */

  // num_comparators = (PARM(ruu_decode_width) - 1) * (PARM(ruu_decode_width));

/*============================== dependency check logic ==============================*/



/*============================== instruction selection logic ==============================*/

/* no 1/2 due to precharging */
static double SIM_iwin_sel_anyreq_cap(u_int width)
{
	double Ctotal;

	/* part 1: drain cap of precharging gate */
	Ctotal = SIM_draincap(WSelORprequ, PCH, 1);

	/* part 2: drain cap of NOR gate */
	Ctotal += width * SIM_draincap(WSelORn, NCH, 1);

	/* part 3: inverter cap */
	/* WHS: no size information, assume decinv */
	Ctotal += SIM_draincap(Wdecinvn, NCH, 1) + SIM_draincap(Wdecinvp, PCH, 1) +
		SIM_gatecap(Wdecinvn + Wdecinvp, 1);

	return Ctotal;
}


/* no 1/2 due to req precharging */
static double SIM_iwin_sel_chgreq_cap(void)
{
	/* gate cap of priority encoder */
	return (SIM_gatecap(WSelPn + WSelPp, 1));
}


/* switching cap when priority encoder generates 1 (some req is 1) */
/* no 1/2 due to req precharging */
static double SIM_iwin_sel_enc_cap(u_int level)
{
	double Ctotal;

	/* part 1: drain cap of priority encoder */
	Ctotal = level * SIM_draincap(WSelPn, NCH, 1) + SIM_draincap(WSelPp, PCH, level);

	/* part 2: gate cap of next-level NAND gate */
	/* WHS: 20 should go to PARM */
	Ctotal += SIM_gatecap(WSelEnn + WSelEnp, 20); 

	return Ctotal;
}
 

/* no 1/2 due to req precharging */
static double SIM_iwin_sel_grant_cap(u_int width)
{
	double Ctotal;

	/* part 1: drain cap of NAND gate */
	Ctotal = SIM_draincap(WSelEnn, NCH, 2) + 2 * SIM_draincap(WSelEnp, PCH, 1);

	/* part 2: inverter cap */
	/* WHS: no size information, assume decinv */
	Ctotal += SIM_draincap(Wdecinvn, NCH, 1) + SIM_draincap(Wdecinvp, PCH, 1) +
		SIM_gatecap(Wdecinvn + Wdecinvp, 1);

	/* part 3: gate cap of enable signal */
	/* grant signal is enable signal to the lower-level arbiter */
	/* WHS: 20 should go to PARM */
	Ctotal += width * SIM_gatecap(WSelEnn + WSelEnp, 20); 

	return Ctotal;
}


static int SIM_iwin_sel_init(SIM_sel_t *sel, int model, u_int width)
{
	u_int i;

	if ((sel->model = model) && model < SEL_MAX_MODEL) {
		sel->width = width;

		sel->n_anyreq = sel->n_chgreq = sel->n_grant = 0;
		for (i = 0; i < width; i++)
			sel->n_enc[i] = 0;

		sel->e_anyreq = SIM_iwin_sel_anyreq_cap(width) * EnergyFactor;
		sel->e_chgreq = SIM_iwin_sel_chgreq_cap() * EnergyFactor;
		sel->e_grant = SIM_iwin_sel_grant_cap(width);

		for (i = 0; i < width; i++)
			sel->e_enc[i] = SIM_iwin_sel_enc_cap(i + 1);    

		return 0;
	}
	else
		return -1;
}


/* WHS: maybe req should be of type LIB_Type_max_uint and use BIGONE instead of 1 */
int SIM_iwin_sel_record(SIM_sel_t *sel, u_int req, int en, int last_level)
{
	u_int i;
	int pre_grant = 0;

	if (req) {
		if (! last_level)
			sel->n_anyreq ++;

		/* assume LSB has the highest priority */
		for (i = 0; i < sel->width; i++) {
			if (req & 1 << i) {
				sel->n_chgreq += sel->width - i;

				if (! pre_grant) {
					pre_grant = 1;

					sel->n_enc[i] ++;
					if (en) sel->n_grant ++;
				}
			}
		}
	}

	return 0;
}


double SIM_iwin_sel_report(SIM_sel_t *sel)
{
	u_int i;
	double Etotal;

	Etotal = sel->n_anyreq * sel->e_anyreq + sel->n_chgreq * sel->e_chgreq +
		sel->n_grant * sel->e_grant;

	for (i = 0; i < sel->width; i++)
		Etotal += sel->n_enc[i] * sel->e_enc[i];

	return Etotal;
}

  /* BEGIN: legacy */
  /* WHS: 4 should go to PARM */
  // while(win_entries > 4)
  //   {
  //     win_entries = (int)ceil((double)win_entries / 4.0);
  //     num_arbiter += win_entries;
  //   }
  // Ctotal += PARM(ruu_issue_width) * num_arbiter*(Cor+Cpencode);
  /* END: legacy */

/*============================== instruction selection logic ==============================*/



/*============================== bus (link) ==============================*/

static int SIM_bus_bitwidth(int encoding, u_int data_width, u_int grp_width)
{
	if (encoding && encoding < BUS_MAX_ENC)
		switch (encoding) {
			case IDENT_ENC:
			case TRANS_ENC:	return data_width;
			case BUSINV_ENC:	return data_width + data_width / grp_width + (data_width % grp_width ? 1:0);
			default: printf ("error\n");	/* some error handler */
		}
	else
		return -1;
}


/*
 * this function is provided to upper layers to compute the exact binary bus representation
 * only correct when grp_width divides data_width
 */
LIB_Type_max_uint SIM_bus_state(SIM_bus_t *bus, LIB_Type_max_uint old_data, LIB_Type_max_uint old_state, LIB_Type_max_uint new_data)
{
	LIB_Type_max_uint mask_bus, mask_data;
	LIB_Type_max_uint new_state = 0;
	u_int done_width = 0;

	switch (bus->encoding) {
		case IDENT_ENC:	return new_data;
		case TRANS_ENC:	return new_data ^ old_data;

		case BUSINV_ENC:
				/* FIXME: this function should be re-written for boundary checking */
				mask_data = (BIGONE << bus->grp_width) - 1;
				mask_bus = (mask_data << 1) + 1;

				while (bus->data_width > done_width) {
					if (SIM_Hamming(old_state & mask_bus, new_data & mask_data, mask_bus) > bus->grp_width / 2)
						new_state += (~(new_data & mask_data) & mask_bus) << done_width + done_width / bus->grp_width;
					else
						new_state += (new_data & mask_data) << done_width + done_width / bus->grp_width;

					done_width += bus->grp_width;
					old_state >>= bus->grp_width + 1;
					new_data >>= bus->grp_width;
				}

				return new_state;

		default: printf ("error\n");	/* some error handler */
	}
}


static double SIM_resultbus_cap(void)
{
	double Cline, reg_height;

	/* compute size of result bus tags */
	reg_height = PARM(RUU_size) * (RegCellHeight + WordlineSpacing * 3 * PARM(ruu_issue_width)); 

	/* assume num alu's = ialu */
	/* FIXME: generate a more detailed result bus network model */
	/* WHS: 3200 should go to PARM */
	/* WHS: use minimal pitch for buses */
	Cline = CCmetal * (reg_height + 0.5 * PARM(res_ialu) * 3200 * LSCALE);

	/* or use result bus length measured from 21264 die photo */
	// Cline = CCmetal * 3.3 * 1000;

	return Cline;
}


static double SIM_generic_bus_cap(u_int n_snd, u_int n_rcv, double length, double time)
{
	double Ctotal = 0;
	double n_size, p_size;

	/* part 1: wire cap */
	/* WHS: use minimal pitch for buses */
	Ctotal += CC2metal * length;

	if ((n_snd == 1) && (n_rcv == 1)) {
		/* directed bus if only one sender and one receiver */

		/* part 2: repeater cap */
		/* FIXME: ratio taken from Raw, does not scale now */
		n_size = Lamda * 10;
		p_size = n_size * 2;

		Ctotal += SIM_gatecap(n_size + p_size, 0) + SIM_draincap(n_size, NCH, 1) + SIM_draincap(p_size, PCH, 1);

		n_size *= 2.5;
		p_size *= 2.5;

		Ctotal += SIM_gatecap(n_size + p_size, 0) + SIM_draincap(n_size, NCH, 1) + SIM_draincap(p_size, PCH, 1);
	}
	else {
		/* otherwise, broadcasting bus */

		/* part 2: input cap */
		/* WHS: no idea how input interface is, use an inverter for now */
		Ctotal += n_rcv * SIM_gatecap(Wdecinvn + Wdecinvp, 0);

		/* part 3: output driver cap */
		if (time) {
			p_size = SIM_driver_size(Ctotal, time);
			n_size = p_size / 2;
		}
		else {
			p_size = Wbusdrvp;
			n_size = Wbusdrvn;
		}

		Ctotal += n_snd * (SIM_draincap(Wdecinvn, NCH, 1) + SIM_draincap(Wdecinvp, PCH, 1));
	}

	return Ctotal;
}


/*
 * n_snd -> # of senders
 * n_rcv -> # of receivers
 * time  -> rise and fall time, 0 means using default transistor sizes
 * grp_width only matters for BUSINV_ENC
 */
int SIM_bus_init(SIM_bus_t *bus, int model, int encoding, u_int width, u_int grp_width, u_int n_snd, u_int n_rcv, double length, double time)
{
	if ((bus->model = model) && model < BUS_MAX_MODEL) {
		bus->data_width = width;
		bus->grp_width = grp_width;
		bus->n_switch = 0;

		switch (model) {
			case RESULT_BUS:
				/* assume result bus uses identity encoding */
				bus->encoding = IDENT_ENC;
				bus->e_switch = SIM_resultbus_cap() / 2 * EnergyFactor;
				break;

			case GENERIC_BUS:
				if ((bus->encoding = encoding) && encoding < BUS_MAX_ENC) {
					bus->e_switch = SIM_generic_bus_cap(n_snd, n_rcv, length, time) / 2 * EnergyFactor;
					/* sanity check */
					if (!grp_width || grp_width > width)
						bus->grp_width = width;
				}
				else return -1;
				break;

			default: printf ("error\n");	/* some error handler */
		}

		bus->bit_width = SIM_bus_bitwidth(bus->encoding, width, bus->grp_width);
		bus->bus_mask = HAMM_MASK(bus->bit_width);

		/* BEGIN: legacy */
		// npreg_width = SIM_logtwo(PARM(RUU_size));
		/* assume ruu_issue_width result busses -- power can be scaled linearly
		 * for number of result busses (scale by writeback_access) */
		// bus->static_af = 2 * (PARM(data_width) + npreg_width) * PARM(ruu_issue_width) * PARM(AF);
		/* END: legacy */

		return 0;
	}
	else
		return -1;
}


int SIM_bus_record(SIM_bus_t *bus, LIB_Type_max_uint old_state, LIB_Type_max_uint new_state)
{
	bus->n_switch += SIM_Hamming(new_state, old_state, bus->bus_mask);

	return 0;
}


double SIM_bus_report(SIM_bus_t *bus)
{
	return (bus->n_switch * bus->e_switch);
}

/*============================== bus (link) ==============================*/



/*============================== flip flop ==============================*/

/* this model is based on the gate-level design given by Randy H. Katz "Contemporary Logic Design"
 * Figure 6.24, node numbers (1-6) are assigned to all gate outputs, left to right, top to bottom
 * 
 * We should have pure cap functions and leave the decision of whether or not to have coefficient
 * 1/2 in init function.
 */
static double SIM_fpfp_node_cap(u_int fan_in, u_int fan_out)
{
	double Ctotal = 0;

	/* FIXME: all need actual sizes */
	/* part 1: drain cap of NOR gate */
	Ctotal += fan_in * SIM_draincap(WdecNORn, NCH, 1) + SIM_draincap(WdecNORp, PCH, fan_in);

	/* part 2: gate cap of NOR gates */
	Ctotal += fan_out * SIM_gatecap(WdecNORn + WdecNORp, 0);

	return Ctotal;
}


static double SIM_fpfp_clock_cap(void)
{
	/* gate cap of clock load */
	return (2 * SIM_gatecap(WdecNORn + WdecNORp, 0));
}


int SIM_fpfp_clear_stat(SIM_ff_t *ff)
{
	ff->n_switch = ff->n_keep_1 = ff->n_keep_0 = ff->n_clock = 0;

	return 0;
}


int SIM_fpfp_init(SIM_ff_t *ff, int model, double load)
{
	double c1, c2, c3, c4, c5, c6;

	if ((ff->model = model) && model < FF_MAX_MODEL) {
		switch (model) {
			case NEG_DFF:
				SIM_fpfp_clear_stat(ff);

				/* node 5 and node 6 are identical to node 1 in capacitance */
				c1 = c5 = c6 = SIM_fpfp_node_cap(2, 1);
				c2 = SIM_fpfp_node_cap(2, 3);
				c3 = SIM_fpfp_node_cap(3, 2);
				c4 = SIM_fpfp_node_cap(2, 3);

				ff->e_switch = (c4 + c1 + c2 + c3 + c5 + c6 + load) / 2 * EnergyFactor;
				/* no 1/2 for e_keep and e_clock because clock signal switches twice in one cycle */
				ff->e_keep_1 = c3 * EnergyFactor; 
				ff->e_keep_0 = c2 * EnergyFactor; 
				ff->e_clock = SIM_fpfp_clock_cap() * EnergyFactor;

				/* static power */
				ff->I_static = (WdecNORp * NOR2_TAB[0] + WdecNORn * (NOR2_TAB[1] + NOR2_TAB[2] + NOR2_TAB[3])) / 4 * 6;

				break;

			default: printf ("error\n");	/* some error handler */
		}

		return 0;
	}
	else
		return -1;
}


double SIM_fpfp_report(SIM_ff_t *ff)
{
	return (ff->e_switch * ff->n_switch + ff->e_clock * ff->n_clock +
			ff->e_keep_0 * ff->n_keep_0 + ff->e_keep_1 * ff->n_keep_1);
}

/*============================== flip flop ==============================*/
