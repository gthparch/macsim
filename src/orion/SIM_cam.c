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

#include "SIM_parameter.h"
#include "SIM_array.h"
#include "SIM_cam.h"
#include "SIM_util.h"
#include "SIM_time.h"

/*============================== wordlines ==============================*/

/* each time one wordline 1->0, another wordline 0->1, so no 1/2 */
double SIM_cam_wordline_cap( u_int cols, double wire_cap, double tx_width )
{
	double Ctotal, Cline, psize, nsize;

	/* part 1: line cap, including gate cap of pass tx's and metal cap */ 
	Ctotal = Cline = SIM_gatecappass( tx_width, 2 ) * cols + wire_cap;

	/* part 2: input driver */
	psize = SIM_driver_size( Cline, Period / 8 );
	nsize = psize * Wdecinvn / Wdecinvp; 
	/* WHS: 20 should go to PARM */
	Ctotal += SIM_draincap( nsize, NCH, 1 ) + SIM_draincap( psize, PCH, 1 ) +
		SIM_gatecap( nsize + psize, 20 );

	return Ctotal;
}

/*============================== wordlines ==============================*/



/*============================== tag comparator ==============================*/

/* tag and tagbar switch simultaneously, so no 1/2 */
double SIM_cam_comp_tagline_cap( u_int rows, double taglinelength )
{
	double Ctotal;

	/* part 1: line cap, including drain cap of pass tx's and metal cap */
	Ctotal = rows * SIM_gatecap( Wcomparen2, 2 ) + CC3M2metal * taglinelength;

	/* part 2: input driver */
	Ctotal += SIM_draincap( Wcompdrivern, NCH, 1 ) + SIM_draincap( Wcompdriverp, PCH, 1 ) +
		SIM_gatecap( Wcompdrivern + Wcompdriverp, 1 );

	return Ctotal;
}


/* upon mismatch, matchline 1->0, then 0->1 on next precharging, so no 1/2 */
double SIM_cam_comp_mismatch_cap( u_int n_bits, u_int n_pre, double matchline_len )
{
	double Ctotal;

	/* part 1: drain cap of precharge tx */
	Ctotal = n_pre * SIM_draincap( Wmatchpchg, PCH, 1 );

	/* part 2: drain cap of comparator tx */
	Ctotal += n_bits * ( SIM_draincap( Wcomparen1, NCH, 1 ) + SIM_draincap( Wcomparen1, NCH, 2 ));

	/* part 3: metal cap of matchline */
	Ctotal += CC3M3metal * matchline_len;

	/* FIXME: I don't understand the Wattch code here */
	/* part 4: nor gate of valid output */
	Ctotal += SIM_gatecap( Wmatchnorn + Wmatchnorp, 10 );

	return Ctotal;
}


/* WHS: subtle difference of valid output between cache and inst window:
 *   fully-associative cache: nor all matchlines of the same port
 *   instruction window:      nor all matchlines of the same tag line */   
/* upon miss, valid output switches twice in one cycle, so no 1/2 */
double SIM_cam_comp_miss_cap( u_int assoc )
{
	/* drain cap of valid output */
	return ( assoc * SIM_draincap( Wmatchnorn, NCH, 1 ) + SIM_draincap( Wmatchnorp, PCH, assoc ));
}

/*============================== tag comparator ==============================*/



/*============================== memory cell ==============================*/

/* WHS: use Wmemcella and Wmemcellbscale to compute tx width of memory cell */
double SIM_cam_tag_mem_cap( u_int read_ports, u_int write_ports, int share_rw, u_int end, int only_write )
{
	double Ctotal;

	/* part 1: drain capacitance of pass transistors */
	if ( only_write )
		Ctotal = SIM_draincap( Wmemcellw, NCH, 1 ) * write_ports;
	else {
		Ctotal = SIM_draincap( Wmemcellr, NCH, 1 ) * read_ports * end / 2;
		if ( ! share_rw )
			Ctotal += SIM_draincap( Wmemcellw, NCH, 1 ) * write_ports;
	}

	/* has coefficient ( 1/2 * 2 ) */
	/* part 2: drain capacitance of memory cell */
	Ctotal += SIM_draincap( Wmemcella, NCH, 1 ) + SIM_draincap( Wmemcella * Wmemcellbscale, PCH, 1 );

	/* has coefficient ( 1/2 * 2 ) */
	/* part 3: gate capacitance of memory cell */
	Ctotal += SIM_gatecap( Wmemcella, 1 ) + SIM_gatecap( Wmemcella * Wmemcellbscale, 1 );

	/* has coefficient ( 1/2 * 2 ) */
	/* part 4: gate capacitance of comparator */
	Ctotal += SIM_gatecap( Wcomparen1, 2 ) * read_ports;

	return Ctotal;
}


double SIM_cam_data_mem_cap( u_int read_ports, u_int write_ports )
{
	double Ctotal;

	/* has coefficient ( 1/2 * 2 ) */
	/* part 1: drain capacitance of pass transistors */
	Ctotal = SIM_draincap( Wmemcellw, NCH, 1 ) * write_ports;

	/* has coefficient ( 1/2 * 2 ) */
	/* part 2: drain capacitance of memory cell */
	Ctotal += SIM_draincap( Wmemcella, NCH, 1 ) + SIM_draincap( Wmemcella * Wmemcellbscale, PCH, 1 );

	/* has coefficient ( 1/2 * 2 ) */
	/* part 3: gate capacitance of memory cell */
	Ctotal += SIM_gatecap( Wmemcella, 1 ) + SIM_gatecap( Wmemcella * Wmemcellbscale, 1 );

	/* part 4: gate capacitance of output driver */
	Ctotal += ( SIM_gatecap( Woutdrvnandn, 1 ) + SIM_gatecap( Woutdrvnandp, 1 ) +
			SIM_gatecap( Woutdrvnorn, 1 ) + SIM_gatecap( Woutdrvnorp, 1 )) / 2 * read_ports;

	return Ctotal;
}

/*============================== memory cell ==============================*/
