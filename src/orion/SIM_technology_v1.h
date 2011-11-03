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

#ifndef _SIM_TECHNOLOGY_V1_H
#define _SIM_TECHNOLOGY_V1_H

#include <sys/types.h>

/* This file contains parameters for 110nm and above*/
#if ( PARM(TECH_POINT) >= 110 )
/*
 * Cache layout parameters and process parameters 
 * Thanks to Glenn Reinman for the technology scaling factors
 */

#if ( PARM(TECH_POINT) == 110 ) /* copied from TECH_POINT 90 except LSCALE */
#define CSCALE		(84.2172)	/* wire capacitance scaling factor */
#define RSCALE		(80.0000)	/* wire resistance scaling factor */
#define LSCALE		(0.1375)		/* length (feature) scaling factor */
#define ASCALE		(LSCALE*LSCALE)	/* area scaling factor */
#define VSCALE		0.38		/* voltage scaling factor */
#define VTSCALE		0.49		/* threshold voltage scaling factor */
#define SSCALE		0.80		/* sense voltage scaling factor */
#define MCSCALE		5.2277		/* metal coupling capacitance scaling factor */
#define MCSCALE2	3		/* metal coupling capacitance scaling factor (2X) */
#define MCSCALE3	1.5		/* metal coupling capacitance scaling factor (3X) */
#define GEN_POWER_SCALE (1/PARM(GEN_POWER_FACTOR))
#elif ( PARM(TECH_POINT) == 180 )
#define CSCALE		(19.7172)	/* wire capacitance scaling factor */
#define RSCALE		(20.0000)	/* wire resistance scaling factor */
#define LSCALE		(0.2250)		/* length (feature) scaling factor */
#define ASCALE		(LSCALE*LSCALE)	/* area scaling factor */
#define VSCALE		0.4		/* voltage scaling factor */
#define VTSCALE		0.5046		/* threshold voltage scaling factor */
#define SSCALE		0.85		/* sense voltage scaling factor */
#define MCSCALE		4.1250		/* metal coupling capacitance scaling factor */
#define MCSCALE2	2.4444		/* metal coupling capacitance scaling factor (2X) */
#define MCSCALE3	1.2		/* metal coupling capacitance scaling factor (3X) */
#define GEN_POWER_SCALE 1
#elif ( PARM(TECH_POINT) == 250 )
#define CSCALE		(10.2197)	/* wire capacitance scaling factor */
#define RSCALE		(10.2571)	/* wire resistance scaling factor */
#define LSCALE		(0.3571)		/* length (feature) scaling factor */
#define ASCALE		(LSCALE*LSCALE)	/* area scaling factor */
#define VSCALE		0.45		/* voltage scaling factor */
#define VTSCALE		0.5596		/* threshold voltage scaling factor */
#define SSCALE		0.90		/* sense voltage scaling factor */
#define MCSCALE		1.0		/* metal coupling capacitance scaling factor */
#define MCSCALE2	1.0		/* metal coupling capacitance scaling factor (2X) */
#define MCSCALE3	1.0		/* metal coupling capacitance scaling factor (3X) */
#define GEN_POWER_SCALE PARM(GEN_POWER_FACTOR)
#elif ( PARM(TECH_POINT) == 350 )
#define CSCALE		(5.2197)	/* wire capacitance scaling factor */
#define RSCALE		(5.2571)	/* wire resistance scaling factor */
#define LSCALE		(0.4375	)	/* length (feature) scaling factor */
#define ASCALE		(LSCALE*LSCALE)	/* area scaling factor */
#define VSCALE		0.5		/* voltage scaling factor */
#define VTSCALE		0.6147		/* threshold voltage scaling factor */
#define SSCALE		0.95		/* sense voltage scaling factor */
#define MCSCALE		1.0		/* metal coupling capacitance scaling factor */
#define MCSCALE2	1.0		/* metal coupling capacitance scaling factor (2X) */
#define MCSCALE3	1.0		/* metal coupling capacitance scaling factor (3X) */
#define GEN_POWER_SCALE (PARM(GEN_POWER_FACTOR)*PARM(GEN_POWER_FACTOR))
#elif ( PARM(TECH_POINT) == 400 )
#define CSCALE		1.0		/* wire capacitance scaling factor */
#define RSCALE		1.0		/* wire resistance scaling factor */
#define LSCALE		0.5		/* length (feature) scaling factor */
#define ASCALE		(LSCALE*LSCALE)	/* area scaling factor */
#define VSCALE		1.0		/* voltage scaling factor */
#define VTSCALE		1.0		/* threshold voltage scaling factor */
#define SSCALE		1.0		/* sense voltage scaling factor */
#define MCSCALE		1.0		/* metal coupling capacitance scaling factor */
#define MCSCALE2	1.0		/* metal coupling capacitance scaling factor (2X) */
#define MCSCALE3	1.0		/* metal coupling capacitance scaling factor (3X) */
#define GEN_POWER_SCALE (PARM(GEN_POWER_FACTOR)*PARM(GEN_POWER_FACTOR)*PARM(GEN_POWER_FACTOR))
#else /* ( PARM(TECH_POINT) == 800 ) */
#define CSCALE		1.0		/* wire capacitance scaling factor */
#define RSCALE		1.0		/* wire resistance scaling factor */
#define LSCALE		1.0		/* length (feature) scaling factor */
#define ASCALE		(LSCALE*LSCALE)	/* area scaling factor */
#define VSCALE		1.0		/* voltage scaling factor */
#define VTSCALE		1.0		/* threshold voltage scaling factor */
#define SSCALE		1.0		/* sense voltage scaling factor */
#define MCSCALE		1.0		/* metal coupling capacitance scaling factor */
#define MCSCALE2	1.0		/* metal coupling capacitance scaling factor (2X) */
#define MCSCALE3	1.0		/* metal coupling capacitance scaling factor (3X) */
#define GEN_POWER_SCALE (PARM(GEN_POWER_FACTOR)*PARM(GEN_POWER_FACTOR)*PARM(GEN_POWER_FACTOR)*PARM(GEN_POWER_FACTOR))
#endif

#define MSCALE	(LSCALE * .624 / .2250)

#define SCALE_T 	(1.0)
#define SCALE_M 	(1.0)
#define SCALE_S 	(1.0)

/*
 * CMOS 0.8um model parameters
 *   - from Appendix II of Cacti tech report
 */
/* corresponds to 8um of m3 @ 225ff/um */
#define Cwordmetal    (1.8e-15 * (CSCALE * ASCALE) * SCALE_M)

/* corresponds to 16um of m2 @ 275ff/um */
#define Cbitmetal     (4.4e-15 * (CSCALE * ASCALE) * SCALE_M)

/* corresponds to 1um of m2 @ 275ff/um */
#define Cmetal        (Cbitmetal/16)
#define CM2metal      (Cbitmetal/16) 
#define CM3metal      (Cbitmetal/16) 

/* minimal spacing metal cap per unit length */
#define CCmetal       (Cmetal * MCSCALE)
#define CCM2metal     (CM2metal * MCSCALE) 
#define CCM3metal     (CM3metal * MCSCALE) 
/* 2x minimal spacing metal cap per unit length */
#define CC2metal      (Cmetal * MCSCALE2)
#define CC2M2metal    (CM2metal * MCSCALE2) 
#define CC2M3metal    (CM3metal * MCSCALE2) 
/* 3x minimal spacing metal cap per unit length */
#define CC3metal      (Cmetal * MCSCALE3)
#define CC3M2metal    (CM2metal * MCSCALE3) 
#define CC3M3metal    (CM3metal * MCSCALE3) 

/* um */
#define Leff          (0.8 * LSCALE)
/* length unit in um */
#define Lamda         (Leff * 0.5)

/* fF/um */
#define Cpolywire	(0.25e-15 * CSCALE * LSCALE)

/* ohms*um of channel width */
#define Rnchannelstatic	(25800 * LSCALE)

/* ohms*um of channel width */
#define Rpchannelstatic	(61200 * LSCALE)

#define Rnchannelon	(9723 * LSCALE)

#define Rpchannelon	(22400 * LSCALE)

/* corresponds to 16um of m2 @ 48mO/sq */
#define Rbitmetal	(0.320 * (RSCALE * ASCALE))

/* corresponds to  8um of m3 @ 24mO/sq */
#define Rwordmetal	(0.080 * (RSCALE * ASCALE))

#define krise		(0.4e-9 * LSCALE)
#define tsensedata	(5.8e-10 * LSCALE)
#define tsensetag	(2.6e-10 * LSCALE)
#define tfalldata	(7e-10 * LSCALE)
#define tfalltag	(7e-10 * LSCALE)
#define Vbitpre		(3.3 * SSCALE)
#define Vt		(1.09 * VTSCALE)
#define Vbitsense	(0.10 * SSCALE)

#define SensePowerfactor3 (PARM(Freq))*(Vbitsense)*(Vbitsense)
#define SensePowerfactor2 (PARM(Freq))*(Vbitpre-Vbitsense)*(Vbitpre-Vbitsense)
#define SensePowerfactor  (PARM(Freq))*Vdd*(Vdd/2)
#define SenseEnergyFactor (Vdd*Vdd/2)

/* transistor widths in um (as described in tech report, appendix 1) */
#define Wdecdrivep	(57.0 * LSCALE)
#define Wdecdriven	(40.0 * LSCALE)
#define Wdec3to8n	(14.4 * LSCALE)
#define Wdec3to8p	(14.4 * LSCALE)
#define WdecNORn	(5.4 * LSCALE)
#define WdecNORp	(30.5 * LSCALE)
#define Wdecinvn	(5.0 * LSCALE)
#define Wdecinvp	(10.0  * LSCALE)
#define Wdff		(53.0 * LSCALE)

#define Wworddrivemax	(100.0 * LSCALE)
#define Wmemcella	(2.4 * LSCALE)
#define Wmemcellr	(4.0 * LSCALE)
#define Wmemcellw	(2.1 * LSCALE)
#define Wmemcellbscale	2		/* means 2x bigger than Wmemcella */
#define Wbitpreequ	(10.0 * LSCALE)

#define Wbitmuxn	(10.0 * LSCALE)
#define WsenseQ1to4	(4.0 * LSCALE)
#define Wcompinvp1	(10.0 * LSCALE)
#define Wcompinvn1	(6.0 * LSCALE)
#define Wcompinvp2	(20.0 * LSCALE)
#define Wcompinvn2	(12.0 * LSCALE)
#define Wcompinvp3	(40.0 * LSCALE)
#define Wcompinvn3	(24.0 * LSCALE)
#define Wevalinvp	(20.0 * LSCALE)
#define Wevalinvn	(80.0 * LSCALE)

#define Wcompn		(20.0 * LSCALE)
#define Wcompp		(30.0 * LSCALE)
#define Wcomppreequ     (40.0 * LSCALE)
#define Wmuxdrv12n	(30.0 * LSCALE)
#define Wmuxdrv12p	(50.0 * LSCALE)
#define WmuxdrvNANDn    (20.0 * LSCALE)
#define WmuxdrvNANDp    (80.0 * LSCALE)
#define WmuxdrvNORn	(60.0 * LSCALE)
#define WmuxdrvNORp	(80.0 * LSCALE)
#define Wmuxdrv3n	(200.0 * LSCALE)
#define Wmuxdrv3p	(480.0 * LSCALE)
#define Woutdrvseln	(12.0 * LSCALE)
#define Woutdrvselp	(20.0 * LSCALE)
#define Woutdrvnandn	(24.0 * LSCALE)
#define Woutdrvnandp	(10.0 * LSCALE)
#define Woutdrvnorn	(6.0 * LSCALE)
#define Woutdrvnorp	(40.0 * LSCALE)
#define Woutdrivern	(48.0 * LSCALE)
#define Woutdriverp	(80.0 * LSCALE)
#define Wbusdrvn	(48.0 * LSCALE)
#define Wbusdrvp	(80.0 * LSCALE)

#define Wcompcellpd2    (2.4 * LSCALE)
#define Wcompdrivern    (400.0 * LSCALE)
#define Wcompdriverp    (800.0 * LSCALE)
#define Wcomparen2      (40.0 * LSCALE)
#define Wcomparen1      (20.0 * LSCALE)
#define Wmatchpchg      (10.0 * LSCALE)
#define Wmatchinvn      (10.0 * LSCALE)
#define Wmatchinvp      (20.0 * LSCALE)
#define Wmatchnandn     (20.0 * LSCALE)
#define Wmatchnandp     (10.0 * LSCALE)
#define Wmatchnorn      (20.0 * LSCALE)
#define Wmatchnorp      (10.0 * LSCALE)

#define WSelORn         (10.0 * LSCALE)
#define WSelORprequ     (40.0 * LSCALE)
#define WSelPn          (10.0 * LSCALE)
#define WSelPp          (15.0 * LSCALE)
#define WSelEnn         (5.0 * LSCALE)
#define WSelEnp         (10.0 * LSCALE)

#define Wsenseextdrv1p  (40.0*LSCALE)
#define Wsenseextdrv1n  (24.0*LSCALE)
#define Wsenseextdrv2p  (200.0*LSCALE)
#define Wsenseextdrv2n  (120.0*LSCALE)

/* bit width of RAM cell in um */
#define BitWidth	(16.0 * LSCALE)

/* bit height of RAM cell in um */
#define BitHeight	(16.0 * LSCALE)

#define Cout		(0.5e-12 * LSCALE)

/* Sizing of cells and spacings */
#define RatCellHeight    (40.0 * LSCALE)
#define RatCellWidth     (70.0 * LSCALE)
#define RatShiftRegWidth (120.0 * LSCALE)
#define RatNumShift      4
#define BitlineSpacing   (6.0 * LSCALE)
#define WordlineSpacing  (6.0 * LSCALE)

#define RegCellHeight    (16.0 * LSCALE)
#define RegCellWidth     (8.0  * LSCALE)

#define CamCellHeight    (40.0 * LSCALE)
#define CamCellWidth     (25.0 * LSCALE)
#define MatchlineSpacing (6.0 * LSCALE)
#define TaglineSpacing   (6.0 * LSCALE)

#define CrsbarCellHeight (6.0 * LSCALE)
#define CrsbarCellWidth  (6.0 * LSCALE)

/*===================================================================*/

/* ALU POWER NUMBERS for .18um 733Mhz */
/* normalize .18um cap to other gen's cap, then xPowerfactor */
#define POWER_SCALE    (GEN_POWER_SCALE * PARM(NORMALIZE_SCALE) * Powerfactor)
#define I_ADD          ((.37 - .091)*POWER_SCALE)
#define I_ADD32        (((.37 - .091)/2)*POWER_SCALE)
#define I_MULT16       ((.31-.095)*POWER_SCALE)
#define I_SHIFT        ((.21-.089)*POWER_SCALE)
#define I_LOGIC        ((.04-.015)*POWER_SCALE)
#define F_ADD          ((1.307-.452)*POWER_SCALE)
#define F_MULT         ((1.307-.452)*POWER_SCALE)

#define I_ADD_CLOCK    (.091*POWER_SCALE)
#define I_MULT_CLOCK   (.095*POWER_SCALE)
#define I_SHIFT_CLOCK  (.089*POWER_SCALE)
#define I_LOGIC_CLOCK  (.015*POWER_SCALE)
#define F_ADD_CLOCK    (.452*POWER_SCALE)
#define F_MULT_CLOCK   (.452*POWER_SCALE)

/* HACK: corresponds to clock network, not supported for 110nm and above*/
#define Clockwire 	(0) 
#define Reswire 	(0) 
#define invCap 		(0)   
#define Resout 		(0) 

/*HACK: parameters for insertion buffer for links not supported for 110nm and above */
#define BufferDriveResistance       0 
#define BufferInputCapacitance      0
#define BufferIntrinsicDelay        0
#define BufferPMOSOffCurrent        0
#define BufferNMOSOffCurrent        0
#define ClockCap                    0 

/*=====Parameters for Area (not supported for 110m and above)========*/
#define AreaNOR         (0)  
#define AreaINV         (0)  
#define AreaAND         (0)  
#define AreaDFF         (0) 
#define AreaMUX2        (0)  
#define AreaMUX3        (0) 
#define AreaMUX4        (0) 

/*=====Parameters for Link (not supported for 110m and above)========*/
#define WireMinWidth            0
#define WireMinSpacing          0
#define WireMetalThickness      0
#define WireBarrierThickness    0
#define WireDielectricThickness 0
#define WireDielectricConstant  0


#endif /* PARM(TECH_POINT) >= 110 */

#endif /* _SIM_TECHNOLOGY_V1_H */
