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
 * This file defines atomic-level and structural-level power models for array
 * structure, e.g. cache, RF, etc.
 *
 * Authors: Hangsheng Wang
 *
 * NOTES: (1) use Wmemcellr for shared r/w because Wmemcellr > Wmemcellw
 *        (2) end == 1 implies register file
 *
 * TODO:  (1) add support for "RatCellHeight"
 *        (2) add support for dynamic decoders
 *
 * FIXME: (1) should use different pre_size for different precharging circuits
 *        (2) should have a lower bound when auto-sizing drivers
 *        (3) sometimes no output driver
 */

#include <stdio.h>
#include <math.h>

#include "SIM_parameter.h"
#include "SIM_array.h"
#include "SIM_static.h"
#include "SIM_time.h"
#include "SIM_util.h"
#include "SIM_cam.h"

	/*============================== decoder ==============================*/

/*#
 * compute switching cap when decoder changes output (select signal)
 *
 * Parameters:
 *   n_input -- fanin of 1 gate of last level decoder
 *
 * Return value: switching cap
 *
 * NOTES: 2 select signals switch, so no 1/2
 */
static double SIM_array_dec_select_cap( u_int n_input )
{
	double Ctotal = 0;
	/* FIXME: why? */
	// if ( numstack > 5 ) numstack = 5;

	/* part 1: drain cap of last level decoders */
	Ctotal = n_input * SIM_draincap( WdecNORn, NCH, 1 ) + SIM_draincap( WdecNORp, PCH, n_input );

	/* part 2: output inverter */
	/* WHS: 20 should go to PARM */
	Ctotal += SIM_draincap( Wdecinvn, NCH, 1 ) + SIM_draincap( Wdecinvp, PCH, 1) +
		SIM_gatecap( Wdecinvn + Wdecinvp, 20 );

	return Ctotal;
}


/*#
 * compute switching cap when 1 input bit of decoder changes
 *
 * Parameters:
 *   n_gates -- fanout of 1 addr signal
 *
 * Return value: switching cap
 *
 * NOTES: both addr and its complement change, so no 1/2
 */
static double SIM_array_dec_chgaddr_cap( u_int n_gates )
{
	double Ctotal;

	/* stage 1: input driver */
	Ctotal = SIM_draincap( Wdecdrivep, PCH, 1 ) + SIM_draincap( Wdecdriven, NCH, 1 ) +
		SIM_gatecap( Wdecdrivep, 1 ) + SIM_gatecap( Wdecdriven, 1 );
	/* inverter to produce complement addr, this needs 1/2 */
	/* WHS: assume Wdecinv(np) for this inverter */
	Ctotal += ( SIM_draincap( Wdecinvp, PCH, 1 ) + SIM_draincap( Wdecinvn, NCH, 1 ) +
			SIM_gatecap( Wdecinvp, 1 ) + SIM_gatecap( Wdecinvn, 1 )) / 2;

	/* stage 2: gate cap of level-1 decoder */
	/* WHS: 10 should go to PARM */
	Ctotal += n_gates * SIM_gatecap( Wdec3to8n + Wdec3to8p, 10 );

	return Ctotal;
}


/*#
 * compute switching cap when 1st-level decoder changes output
 *
 * Parameters:
 *   n_in_1st -- fanin of 1 gate of 1st-level decoder
 *   n_in_2nd -- fanin of 1 gate of 2nd-level decoder
 *   n_gates  -- # of gates of 2nd-level decoder, i.e.
 *               fanout of 1 gate of 1st-level decoder
 *
 * Return value: switching cap
 *
 * NOTES: 2 complementary signals switch, so no 1/2
 */
static double SIM_array_dec_chgl1_cap( u_int n_in_1st, u_int n_in_2nd, u_int n_gates )
{
	double Ctotal;

	/* part 1: drain cap of level-1 decoder */
	Ctotal = n_in_1st * SIM_draincap( Wdec3to8p, PCH, 1 ) + SIM_draincap( Wdec3to8n, NCH, n_in_1st );

	/* part 2: gate cap of level-2 decoder */
	/* WHS: 40 and 20 should go to PARM */
	Ctotal += n_gates * SIM_gatecap( WdecNORn + WdecNORp, n_in_2nd * 40 + 20 );

	return Ctotal;
}


static int SIM_array_dec_clear_stat(SIM_array_dec_t *dec)
{
	dec->n_chg_output = dec->n_chg_l1 = dec->n_chg_addr = 0;

	return 0;
}


/*#
 * initialize decoder 
 *
 * Parameters:
 *   dec    -- decoder structure
 *   model  -- decoder model type
 *   n_bits -- decoder width
 *
 * Side effects:
 *   initialize dec structure if model type is valid
 *
 * Return value: -1 if model type is invalid
 *               0 otherwise
 */
static int SIM_array_dec_init( SIM_array_dec_t *dec, int model, u_int n_bits )
{
	if ((dec->model = model) && model < DEC_MAX_MODEL) {
		dec->n_bits = n_bits;
		/* redundant field */
		dec->addr_mask = HAMM_MASK(n_bits);

		SIM_array_dec_clear_stat(dec);
		dec->e_chg_output = dec->e_chg_l1 = dec->e_chg_addr = 0;

		/* compute geometry parameters */
		if ( n_bits >= 4 ) {		/* 2-level decoder */
			/* WHS: inaccurate for some n_bits */
			dec->n_in_1st = ( n_bits == 4 ) ? 2:3;
			dec->n_out_0th = BIGONE << ( dec->n_in_1st - 1 );
			dec->n_in_2nd = (u_int)ceil((double)n_bits / dec->n_in_1st );
			dec->n_out_1st = BIGONE << ( n_bits - dec->n_in_1st );
		}
		else if ( n_bits >= 2 ) {	/* 1-level decoder */
			dec->n_in_1st = n_bits;
			dec->n_out_0th = BIGONE << ( n_bits - 1 );
			dec->n_in_2nd = dec->n_out_1st = 0;
		}
		else {			/* no decoder basically */
			dec->n_in_1st = dec->n_in_2nd = dec->n_out_0th = dec->n_out_1st = 0;
		}

		/* compute energy constants */
		if ( n_bits >= 2 ) {
			dec->e_chg_l1 = SIM_array_dec_chgl1_cap( dec->n_in_1st, dec->n_in_2nd, dec->n_out_1st ) * EnergyFactor;
			if ( n_bits >= 4 )
				dec->e_chg_output = SIM_array_dec_select_cap( dec->n_in_2nd ) * EnergyFactor;
		}
		dec->e_chg_addr = SIM_array_dec_chgaddr_cap( dec->n_out_0th ) * EnergyFactor;

		return 0;
	}
	else
		return -1;
}


/*#
 * record decoder power stats
 *
 * Parameters:
 *   dec       -- decoder structure
 *   prev_addr -- previous input
 *   curr_addr -- current input
 *
 * Side effects:
 *   update counters in dec structure
 *
 * Return value: 0
 */
int SIM_array_dec_record( SIM_array_dec_t *dec, LIB_Type_max_uint prev_addr, LIB_Type_max_uint curr_addr )
{
	u_int n_chg_bits, n_chg_l1 = 0, n_chg_output = 0;
	u_int i;
	LIB_Type_max_uint mask;

	/* compute Hamming distance */
	n_chg_bits = SIM_Hamming( prev_addr, curr_addr, dec->addr_mask );
	if ( n_chg_bits ) {
		if ( dec->n_bits >= 4 ) {		/* 2-level decoder */
			/* WHS: inaccurate for some n_bits */
			n_chg_output ++;
			/* count addr group changes */
			mask = HAMM_MASK(dec->n_in_1st);
			for ( i = 0; i < dec->n_in_2nd; i ++ ) {
				if ( SIM_Hamming( prev_addr, curr_addr, mask ))
					n_chg_l1 ++;
				mask = mask << dec->n_in_1st;
			}
		}
		else if ( dec->n_bits >= 2 ) {	/* 1-level decoder */
			n_chg_l1 ++;
		}

		dec->n_chg_addr += n_chg_bits;
		dec->n_chg_l1 += n_chg_l1;
		dec->n_chg_output += n_chg_output;
	}

	return 0;
}


/*#
 * report decoder power stats
 *
 * Parameters:
 *   dec -- decoder structure
 *
 * Return value: total energy consumption of this decoder
 *
 * TODO: add more report functionality, currently only total energy is reported
 */
double SIM_array_dec_report( SIM_array_dec_t *dec )
{
	double Etotal;

	Etotal = dec->n_chg_output * dec->e_chg_output + dec->n_chg_l1 * dec->e_chg_l1 +
		dec->n_chg_addr * dec->e_chg_addr;

	/* bonus energy for dynamic decoder :) */
	//if ( is_dynamic_dec( dec->model )) Etotal += Etotal;

	return Etotal;
}

/*============================== decoder ==============================*/



/*============================== wordlines ==============================*/

/*#
 * compute wordline switching cap
 *
 * Parameters:
 *   cols           -- # of pass transistors, i.e. # of bitlines
 *   wordlinelength -- length of wordline
 *   tx_width       -- width of pass transistor
 *
 * Return value: switching cap
 *
 * NOTES: upon address change, one wordline 1->0, another 0->1, so no 1/2
 */
static double SIM_array_wordline_cap( u_int cols, double wire_cap, double tx_width )
{
	double Ctotal, Cline, psize, nsize;

	/* part 1: line cap, including gate cap of pass tx's and metal cap */ 
	Ctotal = Cline = SIM_gatecappass( tx_width, BitWidth / 2 - tx_width ) * cols + wire_cap;

	/* part 2: input driver */
	psize = SIM_driver_size( Cline, Period / 16 );
	nsize = psize * Wdecinvn / Wdecinvp; 
	/* WHS: 20 should go to PARM */
	Ctotal += SIM_draincap( nsize, NCH, 1 ) + SIM_draincap( psize, PCH, 1 ) +
		SIM_gatecap( nsize + psize, 20 );

	return Ctotal;
}


static int SIM_array_wordline_clear_stat(SIM_array_wordline_t *wordline)
{
	wordline->n_read = wordline->n_write = 0;

	return 0;
}


/*#
 * initialize wordline
 *
 * Parameters:
 *   wordline -- wordline structure
 *   model    -- wordline model type
 *   share_rw -- 1 if shared R/W wordlines, 0 if separate R/W wordlines
 *   cols     -- # of array columns, NOT # of bitlines
 *   wire_cap -- wordline wire capacitance
 *   end      -- end of bitlines
 *
 * Return value: -1 if invalid model type
 *               0 otherwise
 *
 * Side effects: 
 *   initialize wordline structure if model type is valid
 *
 * TODO: add error handler
 */
static int SIM_array_wordline_init( SIM_array_wordline_t *wordline, int model, int share_rw, u_int cols, double wire_cap, u_int end )
{
	if ((wordline->model = model) && model < WORDLINE_MAX_MODEL) {
		SIM_array_wordline_clear_stat(wordline);

		switch ( model ) {
			case CAM_RW_WORDLINE:
				wordline->e_read = SIM_cam_wordline_cap( cols * end, wire_cap, Wmemcellr ) * EnergyFactor;
				if ( wordline->share_rw = share_rw )
					wordline->e_write = wordline->e_read;
				else
					/* write bitlines are always double-ended */
					wordline->e_write = SIM_cam_wordline_cap( cols * 2, wire_cap, Wmemcellw ) * EnergyFactor;
				break;

			case CAM_WO_WORDLINE:	/* only have write wordlines */
				wordline->share_rw = 0;
				wordline->e_read = 0;
				wordline->e_write = SIM_cam_wordline_cap( cols * 2, wire_cap, Wmemcellw ) * EnergyFactor;
				break;

			case CACHE_WO_WORDLINE:	/* only have write wordlines */
				wordline->share_rw = 0;
				wordline->e_read = 0;
				wordline->e_write = SIM_array_wordline_cap( cols * 2, wire_cap, Wmemcellw ) * EnergyFactor;
				break;

			case CACHE_RW_WORDLINE:
				wordline->e_read = SIM_array_wordline_cap( cols * end, wire_cap, Wmemcellr ) * EnergyFactor;
				if ( wordline->share_rw = share_rw )
					wordline->e_write = wordline->e_read;
				else
					wordline->e_write = SIM_array_wordline_cap( cols * 2, wire_cap, Wmemcellw ) * EnergyFactor;

				/* static power */
				/* input driver */
				wordline->I_static = (Woutdrivern * NMOS_TAB[0] + Woutdriverp * PMOS_TAB[0]);
				break;

			default: printf ("error\n");	/* some error handler */
		}

		return 0;
	}
	else
		return -1;
}


/*#
 * record wordline power stats
 *
 * Parameters:
 *   wordline -- wordline structure
 *   rw       -- 1 if write operation, 0 if read operation
 *   n_switch -- switching times
 *
 * Return value: 0
 *
 * Side effects: 
 *   update counters of wordline structure
 */
int SIM_array_wordline_record( SIM_array_wordline_t *wordline, int rw, LIB_Type_max_uint n_switch )
{
	if ( rw ) wordline->n_write += n_switch;
	else wordline->n_read += n_switch;

	return 0;
}


/*#
 * report wordline power stats
 *
 * Parameters:
 *   wordline -- wordline structure
 *
 * Return value: total energy consumption of all wordlines of this array
 *
 * TODO: add more report functionality, currently only total energy is reported
 */
double SIM_array_wordline_report( SIM_array_wordline_t *wordline )
{
	return ( wordline->n_read * wordline->e_read +
			wordline->n_write * wordline->e_write );
}

/*============================== wordlines ==============================*/



/*============================== bitlines ==============================*/

/*#
 * compute switching cap of reading 1 separate bitline column
 *
 * Parameters:
 *   rows          -- # of array rows, i.e. # of wordlines
 *   wire_cap      -- bitline wire capacitance
 *   end           -- end of bitlines
 *   n_share_amp   -- # of columns who share one sense amp
 *   n_bitline_pre -- # of precharge transistor drains for 1 bitline column
 *   n_colsel_pre  -- # of precharge transistor drains for 1 column selector, if any
 *   pre_size      -- width of precharge transistors
 *   outdrv_model  -- output driver model type
 *
 * Return value: switching cap
 *
 * NOTES: one bitline 1->0, then 0->1 on next precharging, so no 1/2
 */
static double SIM_array_column_read_cap(u_int rows, double wire_cap, u_int end, u_int n_share_amp, u_int n_bitline_pre, u_int n_colsel_pre, double pre_size, int outdrv_model)
{
	double Ctotal;

	/* part 1: drain cap of precharge tx's */
	Ctotal = n_bitline_pre * SIM_draincap( pre_size, PCH, 1 );

	/* part 2: drain cap of pass tx's */
	Ctotal += rows * SIM_draincap( Wmemcellr, NCH, 1 );

	/* part 3: metal cap */
	Ctotal += wire_cap;

	/* part 4: column selector or bitline inverter */
	if ( end == 1 ) {		/* bitline inverter */
		/* FIXME: magic numbers */
		Ctotal += SIM_gatecap( MSCALE * ( 29.9 + 7.8 ), 0 ) +
			SIM_gatecap( MSCALE * ( 47.0 + 12.0), 0 );
	}
	else if ( n_share_amp > 1 ) {	/* column selector */
		/* drain cap of pass tx's */
		Ctotal += ( n_share_amp + 1 ) * SIM_draincap( Wbitmuxn, NCH, 1 );
		/* drain cap of column selector precharge tx's */
		Ctotal += n_colsel_pre * SIM_draincap( pre_size, PCH, 1 );
		/* FIXME: no way to count activity factor on gates of column selector */
	}

	/* part 5: gate cap of sense amplifier or output driver */
	if (end == 2)			/* sense amplifier */
		Ctotal += 2 * SIM_gatecap( WsenseQ1to4, 10 );
	else if (outdrv_model)	/* end == 1, output driver */
		Ctotal += SIM_gatecap( Woutdrvnandn, 1 ) + SIM_gatecap( Woutdrvnandp, 1 ) +
			SIM_gatecap( Woutdrvnorn, 1 ) + SIM_gatecap( Woutdrvnorp, 1 );

	return Ctotal;
}


/*#
 * compute switching cap of selecting 1 column selector
 *
 * Parameters:
 *
 * Return value: switching cap
 *
 * NOTES: select one, deselect another, so no 1/2
 */
static double SIM_array_column_select_cap( void )
{
	return SIM_gatecap( Wbitmuxn, 1 );
}

	
/*#
 * compute switching cap of writing 1 separate bitline column
 *
 * Parameters:
 *   rows          -- # of array rows, i.e. # of wordlines
 *   wire_cap      -- bitline wire capacitance
 *
 * Return value: switching cap
 *
 * NOTES: bit and bitbar switch simultaneously, so no 1/2
 */
static double SIM_array_column_write_cap( u_int rows, double wire_cap )
{
	double Ctotal, psize, nsize;

	/* part 1: line cap, including drain cap of pass tx's and metal cap */
	Ctotal = rows * SIM_draincap( Wmemcellw, NCH, 1 ) + wire_cap;

	/* part 2: write driver */
	psize = SIM_driver_size( Ctotal, Period / 8 );
	nsize = psize * Wdecinvn / Wdecinvp; 
	Ctotal += SIM_draincap( psize, PCH, 1 ) + SIM_draincap( nsize, NCH, 1 ) +
		SIM_gatecap( psize + nsize, 1 );

	return Ctotal;
}


/* one bitline switches twice in one cycle, so no 1/2 */
static double SIM_array_share_column_write_cap( u_int rows, double wire_cap, u_int n_share_amp, u_int n_bitline_pre, double pre_size ) 
{
	double Ctotal, psize, nsize;

	/* part 1: drain cap of precharge tx's */
	Ctotal = n_bitline_pre * SIM_draincap( pre_size, PCH, 1 );

	/* part 2: drain cap of pass tx's */
	Ctotal += rows * SIM_draincap( Wmemcellr, NCH, 1 );

	/* part 3: metal cap */
	Ctotal += wire_cap;

	/* part 4: column selector or sense amplifier */
	if ( n_share_amp > 1 ) Ctotal += SIM_draincap( Wbitmuxn, NCH, 1 );
	else Ctotal += 2 * SIM_gatecap( WsenseQ1to4, 10 );

	/* part 5: write driver */
	psize = SIM_driver_size( Ctotal, Period / 8 );
	nsize = psize * Wdecinvn / Wdecinvp;
	/* WHS: omit gate cap of driver due to modeling difficulty */
	Ctotal += SIM_draincap( psize, PCH, 1 ) + SIM_draincap( nsize, NCH, 1 );

	return Ctotal;
}


/* one bitline switches twice in one cycle, so no 1/2 */
static double SIM_array_share_column_read_cap( u_int rows, double wire_cap, u_int n_share_amp, u_int n_bitline_pre, u_int n_colsel_pre, double pre_size )
{
	double Ctotal;

	/* part 1: same portion as write */
	Ctotal = SIM_array_share_column_write_cap( rows, wire_cap, n_share_amp, n_bitline_pre, pre_size );

	/* part 2: column selector and sense amplifier */
	if ( n_share_amp > 1 ) {
		/* bottom part of drain cap of pass tx's */
		Ctotal += n_share_amp * SIM_draincap( Wbitmuxn, NCH, 1 );
		/* drain cap of column selector precharge tx's */
		Ctotal += n_colsel_pre * SIM_draincap( pre_size, PCH, 1 );

		/* part 3: gate cap of sense amplifier */
		Ctotal += 2 * SIM_gatecap( WsenseQ1to4, 10 );
	}

	return Ctotal;
}


static int SIM_array_bitline_clear_stat(SIM_array_bitline_t *bitline)
{
	bitline->n_col_write = bitline->n_col_read = bitline->n_col_sel = 0;

	return 0;
}


static int SIM_array_bitline_init(SIM_array_bitline_t *bitline, int model, int share_rw, u_int end, u_int rows, double wire_cap, u_int n_share_amp, u_int n_bitline_pre, u_int n_colsel_pre, double pre_size, int outdrv_model)
{
	if ((bitline->model = model) && model < BITLINE_MAX_MODEL) {
		bitline->end = end;
		SIM_array_bitline_clear_stat(bitline);

		switch ( model ) {
			case RW_BITLINE:
				if ( end == 2 )
					bitline->e_col_sel = SIM_array_column_select_cap() * EnergyFactor;
				else		/* end == 1 implies register file */
					bitline->e_col_sel = 0;

				if ( bitline->share_rw = share_rw ) {
					/* shared bitlines are double-ended, so SenseEnergyFactor */
					bitline->e_col_read = SIM_array_share_column_read_cap( rows, wire_cap, n_share_amp, n_bitline_pre, n_colsel_pre, pre_size ) * SenseEnergyFactor;
					bitline->e_col_write = SIM_array_share_column_write_cap( rows, wire_cap, n_share_amp, n_bitline_pre, pre_size ) * EnergyFactor;
				}
				else {
					bitline->e_col_read = SIM_array_column_read_cap(rows, wire_cap, end, n_share_amp, n_bitline_pre, n_colsel_pre, pre_size, outdrv_model) * (end == 2 ? SenseEnergyFactor : EnergyFactor);
					bitline->e_col_write = SIM_array_column_write_cap( rows, wire_cap ) * EnergyFactor;

					/* static power */
					bitline->I_static = 2 * (Wdecinvn * NMOS_TAB[0] + Wdecinvp * PMOS_TAB[0]);

				}
				break;

			case WO_BITLINE:	/* only have write bitlines */
				bitline->share_rw = 0;
				bitline->e_col_sel = bitline->e_col_read = 0;
				bitline->e_col_write = SIM_array_column_write_cap( rows, wire_cap ) * EnergyFactor;
				break;

			default: printf ("error\n");	/* some error handler */
		}

		return 0;
	}
	else
		return -1;
}


static int is_rw_bitline( int model )
{
	return ( model == RW_BITLINE );
}


/* WHS: no way to count activity factor on column selector gates */
int SIM_array_bitline_record( SIM_array_bitline_t *bitline, int rw, u_int cols, LIB_Type_max_uint old_value, LIB_Type_max_uint new_value )
{
	/* FIXME: should use variable rather than computing each time */
	LIB_Type_max_uint mask = HAMM_MASK(cols);

	if ( rw ) {	/* write */
		if ( bitline->share_rw )	/* share R/W bitlines */
			bitline->n_col_write += cols;
		else			/* separate R/W bitlines */
			bitline->n_col_write += SIM_Hamming( old_value, new_value, mask );
	}
	else {	/* read */
		if ( bitline->end == 2 )	/* double-ended bitline */
			bitline->n_col_read += cols;
		else			/* single-ended bitline */
			/* WHS: read ~new_value due to the bitline inverter */
			bitline->n_col_read += SIM_Hamming( mask, ~new_value, mask );
	}

	return 0;
}


double SIM_array_bitline_report( SIM_array_bitline_t *bitline )
{
	return ( bitline->n_col_write * bitline->e_col_write +
			bitline->n_col_read * bitline->e_col_read +
			bitline->n_col_sel * bitline->e_col_sel );
}

/*============================== bitlines ==============================*/



/*============================== sense amplifier ==============================*/

/* estimate senseamp power dissipation in cache structures (Zyuban's method) */
static double SIM_array_amp_energy( void )
{
	return ( Vdd / 8 * Period * PARM( amp_Idsat ));
}


static int SIM_array_amp_clear_stat(SIM_array_amp_t *amp)
{
	amp->n_access = 0;

	return 0;
}


static int SIM_array_amp_init( SIM_array_amp_t *amp, int model )
{
	if ((amp->model = model) && model < AMP_MAX_MODEL) {
		SIM_array_amp_clear_stat(amp);
		amp->e_access = SIM_array_amp_energy();

		return 0;
	}
	else
		return -1;
}


int SIM_array_amp_record( SIM_array_amp_t *amp, u_int cols )
{
	amp->n_access += cols;

	return 0;
}


double SIM_array_amp_report( SIM_array_amp_t *amp )
{
	return ( amp->n_access * amp->e_access );
}

/*============================== sense amplifier ==============================*/



/*============================== tag comparator ==============================*/

/* eval switches twice per cycle, so no 1/2 */
/* WHS: assume eval = 1 when no cache operation */
static double SIM_array_comp_base_cap( void )
{
	/* eval tx's: 4 inverters */
	return ( SIM_draincap( Wevalinvp, PCH, 1 ) + SIM_draincap( Wevalinvn, NCH, 1 ) +
			SIM_gatecap( Wevalinvp, 1 ) + SIM_gatecap( Wevalinvn, 1 ) +
			SIM_draincap( Wcompinvp1, PCH, 1 ) + SIM_draincap( Wcompinvn1, NCH, 1 ) +
			SIM_gatecap( Wcompinvp1, 1 ) + SIM_gatecap( Wcompinvn1, 1 ) +
			SIM_draincap( Wcompinvp2, PCH, 1 ) + SIM_draincap( Wcompinvn2, NCH, 1 ) +
			SIM_gatecap( Wcompinvp2, 1 ) + SIM_gatecap( Wcompinvn2, 1 ) +
			SIM_draincap( Wcompinvp3, PCH, 1 ) + SIM_draincap( Wcompinvn3, NCH, 1 ) +
			SIM_gatecap( Wcompinvp3, 1 ) + SIM_gatecap( Wcompinvn3, 1 ));
}


/* no 1/2 for the same reason with SIM_array_comp_base_cap */
static double SIM_array_comp_match_cap( u_int n_bits )
{
	return ( n_bits * ( SIM_draincap( Wcompn, NCH, 1 ) + SIM_draincap( Wcompn, NCH, 2 )));
}


/* upon mismatch, select signal 1->0, then 0->1 on next precharging, so no 1/2 */
static double SIM_array_comp_mismatch_cap( u_int n_pre )
{
	double Ctotal;

	/* part 1: drain cap of precharge tx */
	Ctotal = n_pre * SIM_draincap( Wcomppreequ, PCH, 1 );

	/* part 2: nor gate of valid output */
	Ctotal += SIM_gatecap( WdecNORn, 1 ) + SIM_gatecap( WdecNORp, 3 );

	return Ctotal;
}


/* upon miss, valid output switches twice in one cycle, so no 1/2 */
static double SIM_array_comp_miss_cap( u_int assoc )
{
	/* drain cap of valid output */
	return ( assoc * SIM_draincap( WdecNORn, NCH, 1 ) + SIM_draincap( WdecNORp, PCH, assoc ));
}


/* no 1/2 for the same reason as base_cap */
static double SIM_array_comp_bit_match_cap( void )
{
	return ( 2 * ( SIM_draincap( Wcompn, NCH, 1 ) + SIM_draincap( Wcompn, NCH, 2 )));
}


/* no 1/2 for the same reason as base_cap */
static double SIM_array_comp_bit_mismatch_cap( void )
{
	return ( 3 * SIM_draincap( Wcompn, NCH, 1 ) + SIM_draincap( Wcompn, NCH, 2 ));
}


/* each addr bit drives 2 nmos pass transistors, so no 1/2 */
static double SIM_array_comp_chgaddr_cap( void )
{
	return ( SIM_gatecap( Wcompn, 1 ));
}


static int SIM_array_comp_clear_stat(SIM_array_comp_t *comp)
{
	comp->n_access = comp->n_miss = comp->n_chg_addr = comp->n_match = 0;
	comp->n_mismatch = comp->n_bit_match = comp->n_bit_mismatch = 0;

	return 0;
}


static int SIM_array_comp_init( SIM_array_comp_t *comp, int model, u_int n_bits, u_int assoc, u_int n_pre, double matchline_len, double tagline_len )
{
	if ((comp->model = model) && model < COMP_MAX_MODEL) {
		comp->n_bits = n_bits;
		comp->assoc = assoc;
		/* redundant field */
		comp->comp_mask = HAMM_MASK(n_bits);

		SIM_array_comp_clear_stat(comp);

		switch ( model ) {
			case CACHE_COMP:
				comp->e_access = SIM_array_comp_base_cap() * EnergyFactor;
				comp->e_match = SIM_array_comp_match_cap( n_bits ) * EnergyFactor;
				comp->e_mismatch = SIM_array_comp_mismatch_cap( n_pre ) * EnergyFactor;
				comp->e_miss = SIM_array_comp_miss_cap( assoc ) * EnergyFactor;
				comp->e_bit_match = SIM_array_comp_bit_match_cap() * EnergyFactor;
				comp->e_bit_mismatch = SIM_array_comp_bit_mismatch_cap() * EnergyFactor;
				comp->e_chg_addr = SIM_array_comp_chgaddr_cap() * EnergyFactor;
				break;

			case CAM_COMP:
				comp->e_access = comp->e_match = comp->e_chg_addr = 0;
				comp->e_bit_match = comp->e_bit_mismatch = 0;
				/* energy consumption of tagline */
				comp->e_chg_addr = SIM_cam_comp_tagline_cap( assoc, tagline_len ) * EnergyFactor;
				comp->e_mismatch = SIM_cam_comp_mismatch_cap( n_bits, n_pre, matchline_len ) * EnergyFactor;
				comp->e_miss = SIM_cam_comp_miss_cap( assoc ) * EnergyFactor;
				break;

			default: printf ("error\n");	/* some error handler */
		}

		return 0;
	}
	else
		return -1;
}


int SIM_array_comp_global_record( SIM_array_comp_t *comp, LIB_Type_max_uint prev_value, LIB_Type_max_uint curr_value, int miss )
{
	if ( miss ) comp->n_miss ++;

	switch ( comp->model ) {
		case CACHE_COMP:
			comp->n_access ++;
			comp->n_chg_addr += SIM_Hamming( prev_value, curr_value, comp->comp_mask ) * comp->assoc;
			break;

		case CAM_COMP:
			comp->n_chg_addr += SIM_Hamming( prev_value, curr_value, comp->comp_mask );
			break;

		default: printf ("error\n");	/* some error handler */
	}

	return 0;
}


/* recover means prev_tag will recover on next cycle, e.g. driven by sense amplifier */
/* return value: 1 if miss, 0 if hit */
int SIM_array_comp_local_record( SIM_array_comp_t *comp, LIB_Type_max_uint prev_tag, LIB_Type_max_uint curr_tag, LIB_Type_max_uint input, int recover )
{
	u_int H_dist;
	int mismatch;

	if ( mismatch = ( curr_tag != input )) comp->n_mismatch ++;

	/* for cam, input changes are reflected in memory cells */
	if ( comp->model == CACHE_COMP ) {
		if ( recover )
			comp->n_chg_addr += 2 * SIM_Hamming( prev_tag, curr_tag, comp->comp_mask );
		else
			comp->n_chg_addr += SIM_Hamming( prev_tag, curr_tag, comp->comp_mask );

		if ( mismatch ) {
			H_dist = SIM_Hamming( curr_tag, input, comp->comp_mask );
			comp->n_bit_mismatch += H_dist;
			comp->n_bit_match += comp->n_bits - H_dist;
		}
		else comp->n_match ++;
	}

	return mismatch;
}


double SIM_array_comp_report( SIM_array_comp_t *comp )
{
	return ( comp->n_access * comp->e_access + comp->n_match * comp->e_match +
			comp->n_mismatch * comp->e_mismatch + comp->n_miss * comp->e_miss +
			comp->n_bit_match * comp->e_bit_match + comp->n_chg_addr * comp->e_chg_addr +
			comp->n_bit_mismatch * comp->e_bit_mismatch );
}

/*============================== tag comparator ==============================*/



/*============================== multiplexor ==============================*/

/* upon mismatch, 1 output of nor gates 1->0, then 0->1 on next cycle, so no 1/2 */
static double SIM_array_mux_mismatch_cap( u_int n_nor_gates )
{
	double Cmul;

	/* stage 1: inverter */
	Cmul = SIM_draincap( Wmuxdrv12n, NCH, 1 ) + SIM_draincap( Wmuxdrv12p, PCH, 1 ) +
		SIM_gatecap( Wmuxdrv12n, 1 ) + SIM_gatecap( Wmuxdrv12p, 1 );

	/* stage 2: nor gates */
	/* gate cap of nor gates */
	Cmul += n_nor_gates * ( SIM_gatecap( WmuxdrvNORn, 1 ) + SIM_gatecap( WmuxdrvNORp, 1 ));
	/* drain cap of nor gates, only count 1 */
	Cmul += SIM_draincap( WmuxdrvNORp, PCH, 2 ) + 2 * SIM_draincap( WmuxdrvNORn, NCH, 1 );

	/* stage 3: output inverter */
	Cmul += SIM_gatecap( Wmuxdrv3n, 1 ) + SIM_gatecap( Wmuxdrv3p, 1 ) +
		SIM_draincap( Wmuxdrv3n, NCH, 1 ) + SIM_draincap( Wmuxdrv3p, PCH, 1 );

	return Cmul;
}


/* 2 nor gates switch gate signals, so no 1/2 */
/* WHS: assume address changes won't propagate until matched or mismatched */
static double SIM_array_mux_chgaddr_cap( void )
{
	return ( SIM_gatecap( WmuxdrvNORn, 1 ) + SIM_gatecap( WmuxdrvNORp, 1 ));
}


static int SIM_array_mux_clear_stat(SIM_array_mux_t *mux)
{
	mux->n_mismatch = mux->n_chg_addr = 0;

	return 0;
}


static int SIM_array_mux_init( SIM_array_mux_t *mux, int model, u_int n_gates, u_int assoc )
{
	if ((mux->model = model) && model < MUX_MAX_MODEL) {
		mux->assoc = assoc;

		SIM_array_mux_clear_stat(mux);

		mux->e_mismatch = SIM_array_mux_mismatch_cap( n_gates ) * EnergyFactor;
		mux->e_chg_addr = SIM_array_mux_chgaddr_cap() * EnergyFactor;

		return 0;
	}
	else
		return -1;
}


int SIM_array_mux_record( SIM_array_mux_t *mux, LIB_Type_max_uint prev_addr, LIB_Type_max_uint curr_addr, int miss )
{
	if ( prev_addr != curr_addr )
		mux->n_chg_addr += mux->assoc;

	if ( miss )
		mux->n_mismatch += mux->assoc;
	else
		mux->n_mismatch += mux->assoc - 1;

	return 0;
}


double SIM_array_mux_report( SIM_array_mux_t *mux )
{
	return ( mux->n_mismatch * mux->e_mismatch + mux->n_chg_addr * mux->e_chg_addr );
}

/*============================== multiplexor ==============================*/



/*============================== output driver ==============================*/

/* output driver should be disabled somehow when no access occurs, so no 1/2 */
static double SIM_array_outdrv_select_cap( u_int data_width )
{
	double Ctotal;

	/* stage 1: inverter */
	Ctotal = SIM_gatecap( Woutdrvseln, 1 ) + SIM_gatecap( Woutdrvselp, 1 ) +
		SIM_draincap( Woutdrvseln, NCH, 1 ) + SIM_draincap( Woutdrvselp, PCH, 1 );

	/* stage 2: gate cap of nand gate and nor gate */
	/* only consider 1 gate cap because another and drain cap switch depends on data value */
	Ctotal += data_width *( SIM_gatecap( Woutdrvnandn, 1 ) + SIM_gatecap( Woutdrvnandp, 1 ) +
			SIM_gatecap( Woutdrvnorn, 1 ) + SIM_gatecap( Woutdrvnorp, 1 ));

	return Ctotal;
}


/* WHS: assume data changes won't propagate until enabled */
static double SIM_array_outdrv_chgdata_cap( void )
{
	return (( SIM_gatecap( Woutdrvnandn, 1 ) + SIM_gatecap( Woutdrvnandp, 1 ) +
				SIM_gatecap( Woutdrvnorn, 1 ) + SIM_gatecap( Woutdrvnorp, 1 )) / 2 );
}
	

/* no 1/2 for the same reason as outdrv_select_cap */
static double SIM_array_outdrv_outdata_cap( u_int value )
{
	double Ctotal;

	/* stage 1: drain cap of nand gate or nor gate */
	if ( value )
		/* drain cap of nand gate */
		Ctotal = SIM_draincap( Woutdrvnandn, NCH, 2 ) + 2 * SIM_draincap( Woutdrvnandp, PCH, 1 );
	else
		/* drain cap of nor gate */
		Ctotal = 2 * SIM_draincap( Woutdrvnorn, NCH, 1 ) + SIM_draincap( Woutdrvnorp, PCH, 2 );

	/* stage 2: gate cap of output inverter */
	if ( value )
		Ctotal += SIM_gatecap( Woutdriverp, 1 );
	else
		Ctotal += SIM_gatecap( Woutdrivern, 1 );

	/* drain cap of output inverter should be included into bus cap */
	return Ctotal;
}


static int SIM_array_outdrv_clear_stat(SIM_array_out_t *outdrv)
{
	outdrv->n_select = outdrv->n_chg_data = 0;
	outdrv->n_out_0 = outdrv->n_out_1 = 0;

	return 0;
}


static int SIM_array_outdrv_init( SIM_array_out_t *outdrv, int model, u_int item_width )
{
	if ((outdrv->model = model) && model < OUTDRV_MAX_MODEL) {
		outdrv->item_width = item_width;
		/* redundant field */
		outdrv->out_mask = HAMM_MASK(item_width);

		SIM_array_outdrv_clear_stat(outdrv);

		outdrv->e_select = SIM_array_outdrv_select_cap( item_width ) * EnergyFactor;
		outdrv->e_out_1 = SIM_array_outdrv_outdata_cap( 1 ) * EnergyFactor;
		outdrv->e_out_0 = SIM_array_outdrv_outdata_cap( 0 ) * EnergyFactor;

		switch ( model ) {
			case CACHE_OUTDRV:
				outdrv->e_chg_data = SIM_array_outdrv_chgdata_cap() * EnergyFactor;
				break;

			case CAM_OUTDRV:
				/* input changes are reflected in memory cells */
			case REG_OUTDRV:
				/* input changes are reflected in bitlines */
				outdrv->e_chg_data = 0;
				break;

			default: printf ("error\n");	/* some error handler */
		}

		return 0;
	}
	else
		return -1;
}


int SIM_array_outdrv_global_record( SIM_array_out_t *outdrv, LIB_Type_max_uint data )
{
	u_int n_1;

	outdrv->n_select ++;

	n_1 = SIM_Hamming( data, 0, outdrv->out_mask );

	outdrv->n_out_1 += n_1;
	outdrv->n_out_0 += outdrv->item_width - n_1;

	return 0;
}


/* recover means prev_data will recover on next cycle, e.g. driven by sense amplifier */
/* NOTE: this function SHOULD not be called by a fully-associative cache */
int SIM_array_outdrv_local_record( SIM_array_out_t *outdrv, LIB_Type_max_uint prev_data, LIB_Type_max_uint curr_data, int recover )
{
	if ( recover )
		outdrv->n_chg_data += 2 * SIM_Hamming( prev_data, curr_data, outdrv->out_mask );
	else
		outdrv->n_chg_data += SIM_Hamming( prev_data, curr_data, outdrv->out_mask );

	return 0;
}

  
double SIM_array_outdrv_report( SIM_array_out_t *outdrv )
{
	return ( outdrv->n_select * outdrv->e_select + outdrv->n_chg_data * outdrv->e_chg_data +
			outdrv->n_out_1 * outdrv->e_out_1 + outdrv->n_out_0 * outdrv->e_out_0 );
}

/*============================== output driver ==============================*/



/*============================== memory cell ==============================*/

/* WHS: use Wmemcella and Wmemcellbscale to compute tx width of memory cell */
static double SIM_array_mem_cap( u_int read_ports, u_int write_ports, int share_rw, u_int end )
{
	double Ctotal;

	/* part 1: drain capacitance of pass transistors */
	Ctotal = SIM_draincap( Wmemcellr, NCH, 1 ) * read_ports * end / 2;
	if ( ! share_rw )
		Ctotal += SIM_draincap( Wmemcellw, NCH, 1 ) * write_ports;

	/* has coefficient ( 1/2 * 2 ) */
	/* part 2: drain capacitance of memory cell */
	Ctotal += SIM_draincap( Wmemcella, NCH, 1 ) + SIM_draincap( Wmemcella * Wmemcellbscale, PCH, 1 );

	/* has coefficient ( 1/2 * 2 ) */
	/* part 3: gate capacitance of memory cell */
	Ctotal += SIM_gatecap( Wmemcella, 1 ) + SIM_gatecap( Wmemcella * Wmemcellbscale, 1 );

	return Ctotal;
}


static int SIM_array_mem_clear_stat(SIM_array_mem_t *mem)
{
	mem->n_switch = 0;

	return 0;
}


static int SIM_array_mem_init( SIM_array_mem_t *mem, int model, u_int read_ports, u_int write_ports, int share_rw, u_int end )
{
	double I_static;

	if ((mem->model = model) && model < MEM_MAX_MODEL) {
		mem->end = end;
		SIM_array_mem_clear_stat(mem);

		switch ( model ) {
			case CAM_TAG_RW_MEM:
				mem->e_switch = SIM_cam_tag_mem_cap( read_ports, write_ports, share_rw, end, SIM_ARRAY_RW ) * EnergyFactor;
				break;

				/* FIXME: it's only an approximation using CAM_TAG_WO_MEM to emulate CAM_ATTACH_MEM */
			case CAM_ATTACH_MEM:
			case CAM_TAG_WO_MEM:
				mem->e_switch = SIM_cam_tag_mem_cap( read_ports, write_ports, share_rw, end, SIM_ARRAY_WO ) * EnergyFactor;
				break;

			case CAM_DATA_MEM:
				mem->e_switch = SIM_cam_data_mem_cap( read_ports, write_ports ) * EnergyFactor;
				break;

			default:	/* NORMAL_MEM */
				mem->e_switch = SIM_array_mem_cap( read_ports, write_ports, share_rw, end ) * EnergyFactor;

				/* static power */
				I_static = 0;
				/* memory cell */
				I_static += (Wmemcella * NMOS_TAB[0] + Wmemcella * Wmemcellbscale * PMOS_TAB[0]) * 2;
				/* read port pass tx */
				I_static += Wmemcellr * NMOS_TAB[0] * end * read_ports;
				/* write port pass tx */
				if (! share_rw)
					I_static += Wmemcellw * NMOS_TAB[0] * 2 * write_ports;

				mem->I_static = I_static;
		}

		return 0;
	}
	else
		return -1;
}


int SIM_array_mem_record( SIM_array_mem_t *mem, LIB_Type_max_uint prev_value, LIB_Type_max_uint curr_value, u_int width )
{
	mem->n_switch += SIM_Hamming( prev_value, curr_value, HAMM_MASK(width));

	return 0;
}


double SIM_array_mem_report( SIM_array_mem_t *mem )
{
	return ( mem->n_switch * mem->e_switch );
}

/*============================== memory cell ==============================*/



/*============================== precharge ==============================*/

/* consider charge then discharge, so no 1/2 */
double SIM_array_pre_cap( double width, double length )
{
	return SIM_gatecap( width, length );
}


/* return # of precharging gates per column */
u_int SIM_array_n_pre_gate( int model )
{
	switch ( model ) {
		case SINGLE_BITLINE:	return 2;
		case EQU_BITLINE:		return 3;
		case SINGLE_OTHER:		return 1;
		default: printf ("error\n");	/* some error handler */
	}

	return 0;
}


/* return # of precharging drains per line */
u_int SIM_array_n_pre_drain( int model )
{
	switch ( model ) {
		case SINGLE_BITLINE:	return 1;
		case EQU_BITLINE:		return 2;
		case SINGLE_OTHER:		return 1;
		default: printf ("error\n");	/* some error handler */
	}

	return 0;
}


static int SIM_array_pre_clear_stat(SIM_array_pre_t *pre)
{
	pre->n_charge = 0;

	return 0;
}


static int SIM_array_pre_init( SIM_array_pre_t *pre, int model, double pre_size )
{
	u_int n_gate;

	n_gate = SIM_array_n_pre_gate(model);

	if ((pre->model = model) && model < PRE_MAX_MODEL) {
		SIM_array_pre_clear_stat(pre);

		/* WHS: 10 should go to PARM */
		pre->e_charge = SIM_array_pre_cap( pre_size, 10 ) * n_gate * EnergyFactor;

		/* static power */
		pre->I_static = n_gate * pre_size * PMOS_TAB[0];

		return 0;
	}
	else
		return -1;
}


int SIM_array_pre_record( SIM_array_pre_t *pre, LIB_Type_max_uint n_charge )
{
	pre->n_charge += n_charge;

	return 0;
}


double SIM_array_pre_report( SIM_array_pre_t *pre )
{
	return ( pre->n_charge * pre->e_charge );
}

/*============================== precharge ==============================*/

int SIM_array_init(SIM_array_info_t *info, int is_fifo, u_int n_read_port, u_int n_write_port, u_int n_entry, u_int line_width, int outdrv, int arr_buf_type)
{
	/* ==================== set parameters ==================== */
	/* general parameters */
	info->share_rw = 0;
	info->read_ports = n_read_port;
	info->write_ports = n_write_port;
	info->n_set = n_entry;
	info->blk_bits = line_width;
	info->assoc = 1;
	info->data_width = line_width;
	info->data_end = PARM(data_end);
	info->arr_buf_type = arr_buf_type;

	/* no sub-array partition */
	info->data_ndwl = 1;
	info->data_ndbl = 1;
	info->data_nspd = 1;

	info->data_n_share_amp = 1;

	/* model parameters */
	if (is_fifo) {
		info->row_dec_model = SIM_NO_MODEL;
		info->row_dec_pre_model = SIM_NO_MODEL;
	}
	else {
		info->row_dec_model = PARM(row_dec_model);
		info->row_dec_pre_model = PARM(row_dec_pre_model);
	}
	info->data_wordline_model = PARM( wordline_model );
	info->data_bitline_model = PARM( bitline_model );
	info->data_bitline_pre_model = PARM( bitline_pre_model );
	info->data_mem_model = PARM( mem_model );
#if (PARM(data_end) == 2)
	info->data_amp_model = PARM(amp_model);
#else
	info->data_amp_model = SIM_NO_MODEL;
#endif	/* PARM(data_end) == 2 */
	if (outdrv)
		info->outdrv_model = PARM(outdrv_model);
	else
		info->outdrv_model = SIM_NO_MODEL;

	info->data_colsel_pre_model = SIM_NO_MODEL;
	info->col_dec_model = SIM_NO_MODEL;
	info->col_dec_pre_model = SIM_NO_MODEL;
	info->mux_model = SIM_NO_MODEL;

	/* FIXME: not true for shared buffer */
	/* no tag array */
	info->tag_wordline_model = SIM_NO_MODEL;
	info->tag_bitline_model = SIM_NO_MODEL;
	info->tag_bitline_pre_model = SIM_NO_MODEL;
	info->tag_mem_model = SIM_NO_MODEL;
	info->tag_attach_mem_model = SIM_NO_MODEL;
	info->tag_amp_model = SIM_NO_MODEL;
	info->tag_colsel_pre_model = SIM_NO_MODEL;
	info->comp_model = SIM_NO_MODEL;
	info->comp_pre_model = SIM_NO_MODEL;


	/* ==================== set flags ==================== */
	info->write_policy = 0;	/* no dirty bit */


	/* ==================== compute redundant fields ==================== */
	info->n_item = info->blk_bits / info->data_width;
	info->eff_data_cols = info->blk_bits * info->assoc * info->data_nspd;


	/* ==================== call back functions ==================== */
	info->get_entry_valid_bit = NULL;
	info->get_entry_dirty_bit = NULL;
	info->get_entry_tag = NULL;
	info->get_set_tag = NULL;
	info->get_set_use_bit = NULL;


	/* initialize state variables */
	//if (read_port) SIM_buf_port_state_init(info, read_port, info->read_ports);
	//if (write_port) SIM_buf_port_state_init(info, write_port, info->write_ports);

	return 0;
}

int SIM_array_power_init( SIM_array_info_t *info, SIM_array_t *arr)
{
	u_int rows, cols, ports, dec_width, n_bitline_pre, n_colsel_pre;
	double wordline_len, bitline_len, tagline_len, matchline_len;
	double wordline_cmetal, bitline_cmetal;
	double Cline, pre_size, comp_pre_size;

	arr->I_static = 0;

	if(info->arr_buf_type == SRAM){
		/* sanity check */
		if ( info->read_ports == 0 ) info->share_rw = 0;
		if ( info->share_rw ) {
			info->data_end = 2;
			info->tag_end = 2;
		}

		if ( info->share_rw ) ports = info->read_ports;
		else ports = info->read_ports + info->write_ports;

		/* data array unit length wire cap */
		if (ports > 1) {
			/* 3x minimal spacing */
			wordline_cmetal = CC3M3metal;
			bitline_cmetal = CC3M2metal;
		}
		else if (info->data_end == 2) {
			/* wordline infinite spacing, bitline 3x minimal spacing */
			wordline_cmetal = CM3metal;
			bitline_cmetal = CC3M2metal;
		}
		else {
			/* both infinite spacing */
			wordline_cmetal = CM3metal;
			bitline_cmetal = CM2metal;
		}

		info->data_arr_width = 0;
		info->tag_arr_width = 0;
		info->data_arr_height = 0;
		info->tag_arr_height = 0;

		/* BEGIN: data array power initialization */
		if (dec_width = SIM_logtwo(info->n_set)) {
			/* row decoder power initialization */
			SIM_array_dec_init( &arr->row_dec, info->row_dec_model, dec_width );

			/* row decoder precharging power initialization */
			//if ( is_dynamic_dec( info->row_dec_model ))
			/* FIXME: need real pre_size */
			//SIM_array_pre_init( &arr->row_dec_pre, info->row_dec_pre_model, 0 );

			rows = info->n_set / info->data_ndbl / info->data_nspd;
			cols = info->blk_bits * info->assoc * info->data_nspd / info->data_ndwl;

			bitline_len = rows * ( RegCellHeight + ports * WordlineSpacing );
			if ( info->data_end == 2 )
				wordline_len = cols * ( RegCellWidth + 2 * ports * BitlineSpacing );
			else		/* info->data_end == 1 */
				wordline_len = cols * ( RegCellWidth + ( 2 * ports - info->read_ports ) * BitlineSpacing );
			info->data_arr_width = wordline_len;
			info->data_arr_height = bitline_len;

			/* compute precharging size */
			/* FIXME: should consider n_pre and pre_size simultaneously */
			Cline = rows * SIM_draincap( Wmemcellr, NCH, 1 ) + bitline_cmetal * bitline_len;
			pre_size = SIM_driver_size( Cline, Period / 8 );
			/* WHS: ?? compensate for not having an nmos pre-charging */
			pre_size += pre_size * Wdecinvn / Wdecinvp; 

			/* bitline power initialization */
			n_bitline_pre = SIM_array_n_pre_drain( info->data_bitline_pre_model );
			n_colsel_pre = ( info->data_n_share_amp > 1 ) ? SIM_array_n_pre_drain( info->data_colsel_pre_model ) : 0;
			SIM_array_bitline_init(&arr->data_bitline, info->data_bitline_model, info->share_rw, info->data_end, rows, bitline_len * bitline_cmetal, info->data_n_share_amp, n_bitline_pre, n_colsel_pre, pre_size, info->outdrv_model);
			/* static power */
			arr->I_static += arr->data_bitline.I_static * cols * info->write_ports; 

			/* bitline precharging power initialization */
			SIM_array_pre_init( &arr->data_bitline_pre, info->data_bitline_pre_model, pre_size );
			/* static power */
			arr->I_static += arr->data_bitline_pre.I_static * cols * info->read_ports;
			/* bitline column selector precharging power initialization */
			if ( info->data_n_share_amp > 1 )
				SIM_array_pre_init( &arr->data_colsel_pre, info->data_colsel_pre_model, pre_size );

			/* sense amplifier power initialization */
			SIM_array_amp_init( &arr->data_amp, info->data_amp_model );
		}
		else {
			/* info->n_set == 1 means this array is fully-associative */
			rows = info->assoc;
			cols = info->blk_bits;

			/* WHS: no read wordlines or bitlines */
			bitline_len = rows * ( RegCellHeight + info->write_ports * WordlineSpacing );
			wordline_len = cols * ( RegCellWidth + 2 * info->write_ports * BitlineSpacing );
			info->data_arr_width = wordline_len;
			info->data_arr_height = bitline_len;

			/* bitline power initialization */
			SIM_array_bitline_init(&arr->data_bitline, info->data_bitline_model, 0, info->data_end, rows, bitline_len * bitline_cmetal, 1, 0, 0, 0, info->outdrv_model);
		}

		/* wordline power initialization */
		SIM_array_wordline_init( &arr->data_wordline, info->data_wordline_model, info->share_rw, cols, wordline_len * wordline_cmetal, info->data_end );
		/* static power */
		arr->I_static += arr->data_wordline.I_static * rows * ports;
		if (dec_width = SIM_logtwo(info->n_item)) { 
			/* multiplexor power initialization */
			SIM_array_mux_init( &arr->mux, info->mux_model, info->n_item, info->assoc );

			/* column decoder power initialization */
			SIM_array_dec_init( &arr->col_dec, info->col_dec_model, dec_width );

			/* column decoder precharging power initialization */
			//if ( is_dynamic_dec( info->col_dec_model ))
			/* FIXME: need real pre_size */
			//SIM_array_pre_init( &arr->col_dec_pre, info->col_dec_pre_model, 0 );
		}

		/* memory cell power initialization */
		SIM_array_mem_init( &arr->data_mem, info->data_mem_model, info->read_ports, info->write_ports, info->share_rw, info->data_end );
		/* static power */
		arr->I_static += arr->data_mem.I_static * rows * cols;

		/* output driver power initialization */
		SIM_array_outdrv_init( &arr->outdrv, info->outdrv_model, info->data_width );
		/* END: data array power initialization */


		/* BEGIN: legacy */
		//GLOB(e_const)[CACHE_PRECHARGE] = ndwl * ndbl * SIM_array_precharge_power( PARM( n_pre_gate ), pre_size, 10 ) * cols * ((( n_share_amp > 1 ) ? ( 1.0 / n_share_amp ):0 ) + 1 );
		/* END: legacy */


		/* BEGIN: tag array power initialization */
		/* assume a tag array must have memory cells */
		if ( info->tag_mem_model ) {
			if ( info->n_set > 1 ) {
				/* tag array unit length wire cap */
				if (ports > 1) {
					/* 3x minimal spacing */
					wordline_cmetal = CC3M3metal;
					bitline_cmetal = CC3M2metal;
				}
				else if (info->data_end == 2) {
					/* wordline infinite spacing, bitline 3x minimal spacing */
					wordline_cmetal = CM3metal;
					bitline_cmetal = CC3M2metal;
				}
				else {
					/* both infinite spacing */
					wordline_cmetal = CM3metal;
					bitline_cmetal = CM2metal;
				}

				rows = info->n_set / info->tag_ndbl / info->tag_nspd;
				cols = info->tag_line_width * info->assoc * info->tag_nspd / info->tag_ndwl;

				bitline_len = rows * ( RegCellHeight + ports * WordlineSpacing );
				if ( info->tag_end == 2 )
					wordline_len = cols * ( RegCellWidth + 2 * ports * BitlineSpacing );
				else		/* info->tag_end == 1 */
					wordline_len = cols * ( RegCellWidth + ( 2 * ports - info->read_ports ) * BitlineSpacing );
				info->tag_arr_width = wordline_len;
				info->tag_arr_height = bitline_len;

				/* compute precharging size */
				/* FIXME: should consider n_pre and pre_size simultaneously */
				Cline = rows * SIM_draincap( Wmemcellr, NCH, 1 ) + bitline_cmetal * bitline_len;
				pre_size = SIM_driver_size( Cline, Period / 8 );
				/* WHS: ?? compensate for not having an nmos pre-charging */
				pre_size += pre_size * Wdecinvn / Wdecinvp; 

				/* bitline power initialization */
				n_bitline_pre = SIM_array_n_pre_drain( info->tag_bitline_pre_model );
				n_colsel_pre = ( info->tag_n_share_amp > 1 ) ? SIM_array_n_pre_drain( info->tag_colsel_pre_model ) : 0;
				SIM_array_bitline_init(&arr->tag_bitline, info->tag_bitline_model, info->share_rw, info->tag_end, rows, bitline_len * bitline_cmetal, info->tag_n_share_amp, n_bitline_pre, n_colsel_pre, pre_size, SIM_NO_MODEL);

				/* bitline precharging power initialization */
				SIM_array_pre_init( &arr->tag_bitline_pre, info->tag_bitline_pre_model, pre_size );
				/* bitline column selector precharging power initialization */
				if ( info->tag_n_share_amp > 1 )
					SIM_array_pre_init( &arr->tag_colsel_pre, info->tag_colsel_pre_model, pre_size );

				/* sense amplifier power initialization */
				SIM_array_amp_init( &arr->tag_amp, info->tag_amp_model );

				/* prepare for comparator initialization */
				tagline_len = matchline_len = 0;
				comp_pre_size = Wcomppreequ;
			}
			else {	/* info->n_set == 1 */
				/* cam cells are big enough, so infinite spacing */
				wordline_cmetal = CM3metal;
				bitline_cmetal = CM2metal;

				rows = info->assoc;
				/* FIXME: operations of valid bit, use bit and dirty bit are not modeled */
				cols = info->tag_addr_width;

				bitline_len = rows * ( CamCellHeight + ports * WordlineSpacing + info->read_ports * MatchlineSpacing );
				if ( info->tag_end == 2 )
					wordline_len = cols * ( CamCellWidth + 2 * ports * BitlineSpacing + 2 * info->read_ports * TaglineSpacing );
				else		/* info->tag_end == 1 */
					wordline_len = cols * ( CamCellWidth + ( 2 * ports - info->read_ports ) * BitlineSpacing + 2 * info->read_ports * TaglineSpacing );
				info->tag_arr_width = wordline_len;
				info->tag_arr_height = bitline_len;

				if ( is_rw_bitline ( info->tag_bitline_model )) {
					/* compute precharging size */
					/* FIXME: should consider n_pre and pre_size simultaneously */
					Cline = rows * SIM_draincap( Wmemcellr, NCH, 1 ) + bitline_cmetal * bitline_len;
					pre_size = SIM_driver_size( Cline, Period / 8 );
					/* WHS: ?? compensate for not having an nmos pre-charging */
					pre_size += pre_size * Wdecinvn / Wdecinvp; 

					/* bitline power initialization */
					n_bitline_pre = SIM_array_n_pre_drain( info->tag_bitline_pre_model );
					SIM_array_bitline_init(&arr->tag_bitline, info->tag_bitline_model, info->share_rw, info->tag_end, rows, bitline_len * bitline_cmetal, 1, n_bitline_pre, 0, pre_size, SIM_NO_MODEL);

					/* bitline precharging power initialization */
					SIM_array_pre_init( &arr->tag_bitline_pre, info->tag_bitline_pre_model, pre_size );

					/* sense amplifier power initialization */
					SIM_array_amp_init( &arr->tag_amp, info->tag_amp_model );
				}
				else {
					/* bitline power initialization */
					SIM_array_bitline_init(&arr->tag_bitline, info->tag_bitline_model, 0, info->tag_end, rows, bitline_len * bitline_cmetal, 1, 0, 0, 0, SIM_NO_MODEL);
				}

				/* memory cell power initialization */
				SIM_array_mem_init( &arr->tag_attach_mem, info->tag_attach_mem_model, info->read_ports, info->write_ports, info->share_rw, info->tag_end );

				/* prepare for comparator initialization */
				tagline_len = bitline_len;
				matchline_len = wordline_len;
				comp_pre_size = Wmatchpchg;
			}

			/* wordline power initialization */
			SIM_array_wordline_init( &arr->tag_wordline, info->tag_wordline_model, info->share_rw, cols, wordline_len * wordline_cmetal, info->tag_end );

			/* comparator power initialization */
			SIM_array_comp_init( &arr->comp, info->comp_model, info->tag_addr_width, info->assoc, SIM_array_n_pre_drain( info->comp_pre_model ), matchline_len, tagline_len );

			/* comparator precharging power initialization */
			SIM_array_pre_init( &arr->comp_pre, info->comp_pre_model, comp_pre_size );

			/* memory cell power initialization */
			SIM_array_mem_init( &arr->tag_mem, info->tag_mem_model, info->read_ports, info->write_ports, info->share_rw, info->tag_end );
		}
		/* END: tag array power initialization */


		/* BEGIN: legacy */
		/* bitline precharging energy */
		//GLOB(e_const)[CACHE_PRECHARGE] += ntwl * ntbl * SIM_array_precharge_power( PARM( n_pre_gate ), pre_size, 10 ) * cols * ((( n_share_amp > 1 ) ? ( 1.0 / n_share_amp ):0 ) + 1 );
		/* comparator precharging energy */
		//GLOB(e_const)[CACHE_PRECHARGE] += LPARM_cache_associativity * SIM_array_precharge_power( 1, Wcomppreequ, 1 );
		/* END: legacy */
	}

	else if(info->arr_buf_type == REGISTER){ 
		SIM_fpfp_init(&arr->ff, NEG_DFF, 0);
		arr->I_static = arr->ff.I_static * info->data_width * info->n_set;
	}
	return 0;
}


double SIM_array_power_report(SIM_array_info_t *info, SIM_array_t *arr)
{
	double epart, etotal = 0;

	if (info->row_dec_model) {
		epart = SIM_array_dec_report(&arr->row_dec);
		//fprintf(stderr, "row decoder: %g\n", epart);
		etotal += epart;
	}
	if (info->col_dec_model) {
		epart = SIM_array_dec_report(&arr->col_dec);
		//fprintf(stderr, "col decoder: %g\n", epart);
		etotal += epart;
	}
	if (info->data_wordline_model) {
		epart = SIM_array_wordline_report(&arr->data_wordline);
		//fprintf(stderr, "data wordline: %g\n", epart);
		etotal += epart;
	}
	if (info->tag_wordline_model) {
		epart = SIM_array_wordline_report(&arr->tag_wordline);
		//fprintf(stderr, "tag wordline: %g\n", epart);
		etotal += epart;
	}
	if (info->data_bitline_model) {
		epart = SIM_array_bitline_report(&arr->data_bitline);
		//fprintf(stderr, "data bitline: %g\n", epart);
		etotal += epart;
	}
	if (info->data_bitline_pre_model) {
		epart = SIM_array_pre_report(&arr->data_bitline_pre);
		//fprintf(stderr, "data bitline precharge: %g\n", epart);
		etotal += epart;
	}
	if (info->tag_bitline_model) {
		epart = SIM_array_bitline_report(&arr->tag_bitline);
		//fprintf(stderr, "tag bitline: %g\n", epart);
		etotal += epart;
	}
	if (info->data_mem_model) {
		epart = SIM_array_mem_report(&arr->data_mem);
		//fprintf(stderr, "data memory: %g\n", epart);
		etotal += epart;
	}
	if (info->tag_mem_model) {
		epart = SIM_array_mem_report(&arr->tag_mem);
		//fprintf(stderr, "tag memory: %g\n", epart);
		etotal += epart;
	}
	if (info->data_amp_model) {
		epart = SIM_array_amp_report(&arr->data_amp);
		//fprintf(stderr, "data amp: %g\n", epart);
		etotal += epart;
	}
	if (info->tag_amp_model) {
		epart = SIM_array_amp_report(&arr->tag_amp);
		//fprintf(stderr, "tag amp: %g\n", epart);
		etotal += epart;
	}
	if (info->comp_model) {
		epart = SIM_array_comp_report(&arr->comp);
		//fprintf(stderr, "comparator: %g\n", epart);
		etotal += epart;
	}
	if (info->mux_model) {
		epart = SIM_array_mux_report(&arr->mux);
		//fprintf(stderr, "multiplexor: %g\n", epart);
		etotal += epart;
	}
	if (info->outdrv_model) {
		epart = SIM_array_outdrv_report(&arr->outdrv);
		//fprintf(stderr, "output driver: %g\n", epart);
		etotal += epart;
	}
	/* ignore other precharging for now */

	//fprintf(stderr, "total energy: %g\n", etotal);

	return etotal;
}


int SIM_array_clear_stat(SIM_array_t *arr)
{
	SIM_array_dec_clear_stat(&arr->row_dec);
	SIM_array_dec_clear_stat(&arr->col_dec);
	SIM_array_wordline_clear_stat(&arr->data_wordline);
	SIM_array_wordline_clear_stat(&arr->tag_wordline);
	SIM_array_bitline_clear_stat(&arr->data_bitline);
	SIM_array_bitline_clear_stat(&arr->tag_bitline);
	SIM_array_mem_clear_stat(&arr->data_mem);
	SIM_array_mem_clear_stat(&arr->tag_mem);
	SIM_array_mem_clear_stat(&arr->tag_attach_mem);
	SIM_array_amp_clear_stat(&arr->data_amp);
	SIM_array_amp_clear_stat(&arr->tag_amp);
	SIM_array_comp_clear_stat(&arr->comp);
	SIM_array_mux_clear_stat(&arr->mux);
	SIM_array_outdrv_clear_stat(&arr->outdrv);
	SIM_array_pre_clear_stat(&arr->row_dec_pre);
	SIM_array_pre_clear_stat(&arr->col_dec_pre);
	SIM_array_pre_clear_stat(&arr->data_bitline_pre);
	SIM_array_pre_clear_stat(&arr->tag_bitline_pre);
	SIM_array_pre_clear_stat(&arr->data_colsel_pre);
	SIM_array_pre_clear_stat(&arr->tag_colsel_pre);
	SIM_array_pre_clear_stat(&arr->comp_pre);

	return 0;
}
