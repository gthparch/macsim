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

#ifndef _SIM_TECHNOLOGY_V2_H
#define _SIM_TECHNOLOGY_V2_H

#include <sys/types.h>

/* This file contains parameters for 65nm, 45nm and 32nm */
#if ( PARM(TECH_POINT) <= 90 ) 
#define Vbitpre     (Vdd)   
#define Vbitsense   (0.08)    

#define SensePowerfactor3 (PARM(Freq))*(Vbitsense)*(Vbitsense)
#define SensePowerfactor2 (PARM(Freq))*(Vbitpre-Vbitsense)*(Vbitpre-Vbitsense)
#define SensePowerfactor  (PARM(Freq))*Vdd*(Vdd/2)
#define SenseEnergyFactor (Vdd*Vdd/2)

/* scaling factors from 65nm to 45nm and 32nm*/
#if (PARM(TECH_POINT) == 45 && PARM(TRANSISTOR_TYPE) == LVT)
#define SCALE_T (0.9123404) 
#define SCALE_M (0.6442105) 
#define SCALE_S (2.3352694)
#define SCALE_W (0.51) 
#define SCALE_H (0.88) 
#define SCALE_BW (0.73) 
#define SCALE_Crs (0.7) 
#elif (PARM(TECH_POINT) == 45 && PARM(TRANSISTOR_TYPE) == NVT)
#define SCALE_T (0.8233582)
#define SCALE_M (0.6442105)
#define SCALE_S (2.1860558)
#define SCALE_W (0.51) 
#define SCALE_H (0.88) 
#define SCALE_BW (0.73) 
#define SCALE_Crs (0.7) 
#elif (PARM(TECH_POINT) == 45 && PARM(TRANSISTOR_TYPE) == HVT)
#define SCALE_T (0.73437604)
#define SCALE_M (0.6442105)
#define SCALE_S (2.036842)
#define SCALE_W (0.51) 
#define SCALE_H (0.88) 
#define SCALE_BW (0.73) 
#define SCALE_Crs (0.7) 
#elif (PARM(TECH_POINT) == 32 && PARM(TRANSISTOR_TYPE) == LVT)
#define SCALE_T (0.7542128)
#define SCALE_M (0.4863158)
#define SCALE_S (2.9692334)
#define SCALE_W (0.26) 
#define SCALE_H (0.77) 
#define SCALE_BW (0.53) 
#define SCALE_Crs (0.49) 
#elif (PARM(TECH_POINT) == 32 && PARM(TRANSISTOR_TYPE) == NVT)
#define SCALE_T (0.6352095)
#define SCALE_M (0.4863158)
#define SCALE_S (3.1319851)
#define SCALE_W (0.26) 
#define SCALE_H (0.77) 
#define SCALE_BW (0.53) 
#define SCALE_Crs (0.49) 
#elif (PARM(TECH_POINT) == 32 && PARM(TRANSISTOR_TYPE) == HVT)
#define SCALE_T (0.5162063)
#define SCALE_M (0.4863158)
#define SCALE_S (3.294737)
#define SCALE_W (0.26) 
#define SCALE_H (0.77) 
#define SCALE_BW (0.53) 
#define SCALE_Crs (0.49) 
#else /* for 65nm and 90nm */
#define SCALE_T (1)
#define SCALE_M (1)
#define SCALE_S (1)
#define SCALE_W (1)
#define SCALE_H (1)
#define SCALE_BW (1)
#define SCALE_Crs (1)
#endif /* PARM(TECH_POINT) */


#if(PARM(TECH_POINT) == 90)

#define LSCALE 0.125 
#define MSCALE  (LSCALE * .624 / .2250) 

/* bit width of RAM cell in um */
#define BitWidth    (2.0)      

/* bit height of RAM cell in um */
#define BitHeight   (2.0)      

#define Cout        (6.25e-14)      

#define BitlineSpacing   1.1 
#define WordlineSpacing  1.1 

#define RegCellHeight    2.8 
#define RegCellWidth     1.9 

#define Cwordmetal   (1.936e-15) 
#define Cbitmetal    (3.872e-15) 

#define Cmetal        (Cbitmetal/16)
#define CM2metal      (Cbitmetal/16) 
#define CM3metal      (Cbitmetal/16) 

/* minimal spacing metal cap per unit length */
#define CCmetal       (0.18608e-15)   
#define CCM2metal     (0.18608e-15) 
#define CCM3metal     (0.18608e-15) 
/* 2x minimal spacing metal cap per unit length */
#define CC2metal      (0.12529e-15)  
#define CC2M2metal    (0.12529e-15)  
#define CC2M3metal    (0.12529e-15) 
/* 3x minimal spacing metal cap per unit length */
#define CC3metal      (0.11059e-15) 
#define CC3M2metal    (0.11059e-15)  
#define CC3M3metal    (0.11059e-15)  

/* corresponds to clock network*/
#define Clockwire (404.8e-12) 
#define Reswire (36.66e3) 
#define invCap (3.816e-14)   
#define Resout (213.6) 

/* um */
#define Leff          (0.1) 
/* length unit in um */
#define Lamda         (Leff * 0.5)

/* fF/um */
#define Cpolywire   (2.6317875e-15) 

/* ohms*um of channel width */
#define Rnchannelstatic (3225)	

/* ohms*um of channel width */
#define Rpchannelstatic (7650)	

#if( PARM(TRANSISTOR_TYPE) == LVT)   //derived from Cacti 5.3
#define Rnchannelon (1716)
#define Rpchannelon (4202)
#elif (PARM(TRANSISTOR_TYPE) == NVT)
#define Rnchannelon (4120)
#define Rpchannelon (10464)
#elif (PARM(TRANSISTOR_TYPE) == HVT)
#define Rnchannelon (4956)
#define Rpchannelon (12092)
#endif

#define Rbitmetal   (1.38048)   
#define Rwordmetal   (0.945536)     

#if(PARM(TRANSISTOR_TYPE) == LVT) 
#define Vt      0.237
#elif(PARM(TRANSISTOR_TYPE) == NVT)
#define Vt      0.307
#elif (PARM(TRANSISTOR_TYPE) == HVT)
#define Vt      0.482
#endif

/* transistor widths in um (as described in Cacti 1.0 tech report, appendix 1) */
#if(PARM(TRANSISTOR_TYPE) == LVT)
#define Wdecdrivep  (12.50) 
#define Wdecdriven  (6.25) 
#define Wdec3to8n   (11.25)
#define Wdec3to8p   (7.5) 
#define WdecNORn    (0.30) 
#define WdecNORp    (1.5)  
#define Wdecinvn    (0.63)
#define Wdecinvp    (1.25)
#define Wdff 	    (12.29)

#define Wworddrivemax   (12.50)
#define Wmemcella   (0.35)
#define Wmemcellr   (0.50)
#define Wmemcellw   (0.26)
#define Wmemcellbscale  (2)     
#define Wbitpreequ  (1.25) 

#define Wbitmuxn    (1.25)
#define WsenseQ1to4 (0.55)
#define Wcompinvp1  (1.25)
#define Wcompinvn1  (0.75)
#define Wcompinvp2  (2.50)
#define Wcompinvn2  (1.50)
#define Wcompinvp3  (5.15)
#define Wcompinvn3  (3.25)
#define Wevalinvp   (2.50)
#define Wevalinvn   (9.45)

#define Wcompn      (1.25) 
#define Wcompp      (3.75)
#define Wcomppreequ     (5.15)
#define Wmuxdrv12n  (3.75)
#define Wmuxdrv12p  (6.25)
#define WmuxdrvNANDn    (2.50)
#define WmuxdrvNANDp    (10.33)
#define WmuxdrvNORn (7.33)
#define WmuxdrvNORp (10.66)
#define Wmuxdrv3n   (24.85)
#define Wmuxdrv3p   (60.25)
#define Woutdrvseln (1.55)
#define Woutdrvselp (2.33)
#define Woutdrvnandn    (3.27)
#define Woutdrvnandp    (1.25)
#define Woutdrvnorn (0.75)
#define Woutdrvnorp (5.33)
#define Woutdrivern (6.16)
#define Woutdriverp (9.77)
#define Wbusdrvn    (6.16)
#define Wbusdrvp    (10.57)

#define Wcompcellpd2    (0.33)
#define Wcompdrivern    (50.95)
#define Wcompdriverp    (102.67)
#define Wcomparen2      (5.13)
#define Wcomparen1      (2.5)
#define Wmatchpchg      (1.25)
#define Wmatchinvn      (1.33)
#define Wmatchinvp      (2.77)
#define Wmatchnandn     (2.33)
#define Wmatchnandp     (1.76)
#define Wmatchnorn      (2.66)
#define Wmatchnorp      (1.15)

#define WSelORn         (1.25)
#define WSelORprequ     (5.15)
#define WSelPn          (1.86)
#define WSelPp          (1.86)
#define WSelEnn         (0.63)
#define WSelEnp         (1.25)

#define Wsenseextdrv1p  (5.15)
#define Wsenseextdrv1n  (3.05)
#define Wsenseextdrv2p  (25.20)
#define Wsenseextdrv2n  (15.65)

#elif(PARM(TRANSISTOR_TYPE) == NVT)
#define Wdecdrivep  (11.57) 
#define Wdecdriven  (5.74) 
#define Wdec3to8n   (10.31)
#define Wdec3to8p   (6.87) 
#define WdecNORn    (0.28) 
#define WdecNORp    (1.38)  
#define Wdecinvn    (0.58)
#define Wdecinvp    (1.15)
#define Wdff	    (6.57)

#define Wworddrivemax   (11.57)
#define Wmemcella   (0.33)
#define Wmemcellr   (0.46)
#define Wmemcellw   (0.24)
#define Wmemcellbscale  (2)     
#define Wbitpreequ  (1.15) 

#define Wbitmuxn    (1.15)
#define WsenseQ1to4 (0.49)
#define Wcompinvp1  (1.17)
#define Wcompinvn1  (0.69)
#define Wcompinvp2  (2.29)
#define Wcompinvn2  (1.38)
#define Wcompinvp3  (4.66)
#define Wcompinvn3  (2.88)
#define Wevalinvp   (2.29)
#define Wevalinvn   (8.89)

#define Wcompn      (1.15) 
#define Wcompp      (3.44)
#define Wcomppreequ     (4.66)
#define Wmuxdrv12n  (3.44)
#define Wmuxdrv12p  (5.74)
#define WmuxdrvNANDn    (2.29)
#define WmuxdrvNANDp    (9.33)
#define WmuxdrvNORn (6.79)
#define WmuxdrvNORp (9.49)
#define Wmuxdrv3n   (22.83)
#define Wmuxdrv3p   (55.09)
#define Woutdrvseln (1.40)
#define Woutdrvselp (2.21)
#define Woutdrvnandn    (2.89)
#define Woutdrvnandp    (1.15)
#define Woutdrvnorn (0.69)
#define Woutdrvnorp (4.75)
#define Woutdrivern (5.58)
#define Woutdriverp (9.05)
#define Wbusdrvn    (5.58)
#define Wbusdrvp    (9.45)

#define Wcompcellpd2    (0.29)
#define Wcompdrivern    (46.28)
#define Wcompdriverp    (92.94)
#define Wcomparen2      (4.65)
#define Wcomparen1      (2.29)
#define Wmatchpchg      (1.15)
#define Wmatchinvn      (1.19)
#define Wmatchinvp      (2.43)
#define Wmatchnandn     (2.21)
#define Wmatchnandp     (1.42)
#define Wmatchnorn      (2.37)
#define Wmatchnorp      (1.10)

#define WSelORn         (1.15)
#define WSelORprequ     (4.66)
#define WSelPn          (1.45)
#define WSelPp          (1.71)
#define WSelEnn         (0.58)
#define WSelEnp         (1.15)

#define Wsenseextdrv1p  (4.66)
#define Wsenseextdrv1n  (2.78)
#define Wsenseextdrv2p  (23.02)
#define Wsenseextdrv2n  (14.07)

#elif(PARM(TRANSISTOR_TYPE) == HVT)
#define Wdecdrivep  (10.64) 
#define Wdecdriven  (5.23) 
#define Wdec3to8n   (9.36)
#define Wdec3to8p   (6.24) 
#define WdecNORn    (0.25) 
#define WdecNORp    (1.25)  
#define Wdecinvn    (0.52)
#define Wdecinvp    (1.04)
#define Wdff	    (5.43)

#define Wworddrivemax   (10.64)
#define Wmemcella   (0.25)
#define Wmemcellr   (0.42)
#define Wmemcellw   (0.22)
#define Wmemcellbscale  (2)     
#define Wbitpreequ  (1.04) 

#define Wbitmuxn    (1.04)
#define WsenseQ1to4 (0.42)
#define Wcompinvp1  (1.08)
#define Wcompinvn1  (0.62)
#define Wcompinvp2  (2.08)
#define Wcompinvn2  (1.25)
#define Wcompinvp3  (4.16)
#define Wcompinvn3  (2.50)
#define Wevalinvp   (2.08)
#define Wevalinvn   (8.32)

#define Wcompn      (1.04) 
#define Wcompp      (3.12)
#define Wcomppreequ     (4.16)
#define Wmuxdrv12n  (3.12)
#define Wmuxdrv12p  (5.23)
#define WmuxdrvNANDn    (2.08)
#define WmuxdrvNANDp    (8.32)
#define WmuxdrvNORn (6.24)
#define WmuxdrvNORp (8.32)
#define Wmuxdrv3n   (20.80)
#define Wmuxdrv3p   (49.92)
#define Woutdrvseln (1.25)
#define Woutdrvselp (2.08)
#define Woutdrvnandn    (2.50)
#define Woutdrvnandp    (1.04)
#define Woutdrvnorn (0.62)
#define Woutdrvnorp (4.16)
#define Woutdrivern (4.99)
#define Woutdriverp (8.32)
#define Wbusdrvn    (4.99)
#define Wbusdrvp    (8.32)

#define Wcompcellpd2    (0.25)
#define Wcompdrivern    (41.60)
#define Wcompdriverp    (83.20)
#define Wcomparen2      (4.16)
#define Wcomparen1      (2.08)
#define Wmatchpchg      (1.04)
#define Wmatchinvn      (1.04)
#define Wmatchinvp      (2.08)
#define Wmatchnandn     (2.08)
#define Wmatchnandp     (1.08)
#define Wmatchnorn      (2.08)
#define Wmatchnorp      (1.04)

#define WSelORn         (1.04)
#define WSelORprequ     (4.16)
#define WSelPn          (1.04)
#define WSelPp          (1.56)
#define WSelEnn         (0.52)
#define WSelEnp         (1.04)

#define Wsenseextdrv1p  (4.16)
#define Wsenseextdrv1n  (2.50)
#define Wsenseextdrv2p  (20.83)
#define Wsenseextdrv2n  (12.48)

#endif /* 90nm PARM(TRANSISTOR_TYPE)*/

#define CamCellHeight    (4.095)   /*derived from Cacti 5.3 */ 
#define CamCellWidth     (3.51)    /*derived from Cacti 5.3 */ 

#define MatchlineSpacing (0.75) 
#define TaglineSpacing   (0.75) 

#define CrsbarCellHeight 2.94   
#define CrsbarCellWidth  2.94   

#define krise       (0.5e-10)     
#define tsensedata  (0.725e-10)    
#define tsensetag   (0.325e-10)   
#define tfalldata   (0.875e-10)  
#define tfalltag    (0.875e-10) 

/*=============Above are the parameters for 90nm ========================*/

/*=============Below are the parameters for 65nm ========================*/
#elif(PARM(TECH_POINT) <= 65) 

#define LSCALE 0.087 
#define MSCALE  (LSCALE * .624 / .2250) 

/* bit width of RAM cell in um */
#define BitWidth    (1.4)      

/* bit height of RAM cell in um */
#define BitHeight   (1.4)     

#define Cout        (4.35e-14) 

/* Sizing of cells and spacings */
#define BitlineSpacing   (0.8 * SCALE_BW)
#define WordlineSpacing  (0.8 * SCALE_BW)

#define RegCellHeight    (2.1 * SCALE_H)
#define RegCellWidth     (1.4 * SCALE_W)

#define Cwordmetal    (1.63e-15 * SCALE_M)  
#define Cbitmetal     (3.27e-15 * SCALE_M)  

#define Cmetal        (Cbitmetal/16)
#define CM2metal      (Cbitmetal/16) 
#define CM3metal      (Cbitmetal/16) 

// minimum spacing
#define CCmetal       (0.146206e-15) 
#define CCM2metal     (0.146206e-15)  
#define CCM3metal     (0.146206e-15)  
// 2x minimum spacing
#define CC2metal      (0.09844e-15) 
#define CC2M2metal    (0.09844e-15)  
#define CC2M3metal    (0.09844e-15)  
// 3x minimum spacing
#define CC3metal      (0.08689e-15) 
#define CC3M2metal    (0.08689e-15)  
#define CC3M3metal    (0.08689e-15)  


/* corresponds to clock network*/
#define Clockwire (323.4e-12 * SCALE_M) 
#define Reswire (61.11e3 * (1/SCALE_M)) 
#define invCap (3.12e-14) 
#define Resout (361.00)    

/* um */
#define Leff          (0.0696)	
/* length unit in um */
#define Lamda         (Leff * 0.5)

/* fF/um */
#define Cpolywire   (1.832e-15)  
/* ohms*um of channel width */
#define Rnchannelstatic (2244.6)  

/* ohms*um of channel width */
#define Rpchannelstatic (5324.4)  

#if(PARM(TRANSISTOR_TYPE) == LVT)   /* derived from Cacti 5.3 */
#define Rnchannelon (1370)
#define Rpchannelon (3301)
#elif (PARM(TRANSISTOR_TYPE) == NVT)
#define Rnchannelon (2540)
#define Rpchannelon (5791)
#elif (PARM(TRANSISTOR_TYPE) == HVT)
#define Rnchannelon (4530)
#define Rpchannelon (10101)
#endif

#define Rbitmetal   (1.92644)   /* derived from Cacti 5.3 */  
#define Rwordmetal   (1.31948)  /* derived from Cacti 5.3 */

#if(PARM(TRANSISTOR_TYPE) == LVT) 
#define Vt      0.195     
#elif (PARM(TRANSISTOR_TYPE) == NVT)
#define Vt      0.285
#elif (PARM(TRANSISTOR_TYPE) == HVT)
#define Vt      0.524
#endif

/* transistor widths in um for 65nm. (as described in Cacti 1.0 tech report, appendix 1) */
#if(PARM(TRANSISTOR_TYPE) == LVT)  
#define Wdecdrivep  	(8.27) 
#define Wdecdriven  	(6.70)
#define Wdec3to8n	(2.33) 
#define Wdec3to8p	(2.33)
#define WdecNORn	(1.50)
#define WdecNORp	(3.82)
#define Wdecinvn	(8.46)
#define Wdecinvp	(10.93)
#define Wdff		(8.6)

#define Wworddrivemax   (9.27)
#define Wmemcella   (0.2225)
#define Wmemcellr   (0.3708)
#define Wmemcellw   (0.1947)
#define Wmemcellbscale  (1.87)      
#define Wbitpreequ  (0.927)

#define Wbitmuxn    (0.927)
#define WsenseQ1to4 (0.371)
#define Wcompinvp1  (0.927)
#define Wcompinvn1  (0.5562)
#define Wcompinvp2  (1.854)
#define Wcompinvn2  (1.1124)
#define Wcompinvp3  (3.708)
#define Wcompinvn3  (2.2248)
#define Wevalinvp   (1.854)
#define Wevalinvn   (7.416)


#define Wcompn      (1.854)
#define Wcompp      (2.781)
#define Wcomppreequ (3.712)
#define Wmuxdrv12n  (2.785)
#define Wmuxdrv12p  (4.635)
#define WmuxdrvNANDn (1.860)
#define WmuxdrvNANDp (7.416)
#define WmuxdrvNORn (5.562)
#define WmuxdrvNORp (7.416)
#define Wmuxdrv3n   (18.54)
#define Wmuxdrv3p   (44.496)
#define Woutdrvseln (1.112)
#define Woutdrvselp (1.854)
#define Woutdrvnandn    (2.225)
#define Woutdrvnandp    (0.927)
#define Woutdrvnorn (0.5562)
#define Woutdrvnorp (3.708)
#define Woutdrivern (4.450)
#define Woutdriverp (7.416)
#define Wbusdrvn    (4.450)
#define Wbusdrvp    (7.416)

#define Wcompcellpd2    (0.222)
#define Wcompdrivern    (37.08)
#define Wcompdriverp    (74.20)
#define Wcomparen2      (3.708)
#define Wcomparen1      (1.854)
#define Wmatchpchg      (0.927)
#define Wmatchinvn      (0.930)
#define Wmatchinvp      (1.854)
#define Wmatchnandn     (1.854)
#define Wmatchnandp     (0.927)
#define Wmatchnorn      (1.860)
#define Wmatchnorp      (0.930)

#define WSelORn         (0.930)
#define WSelORprequ     (3.708)
#define WSelPn          (0.927)
#define WSelPp          (1.391)
#define WSelEnn         (0.434)
#define WSelEnp         (0.930)

#define Wsenseextdrv1p  (3.708)
#define Wsenseextdrv1n  (2.225)
#define Wsenseextdrv2p  (18.54)
#define Wsenseextdrv2n  (11.124)

#elif (PARM(TRANSISTOR_TYPE) == NVT) 
#define Wdecdrivep      (6.7) 
#define Wdecdriven      (4.7)
#define Wdec3to8n       (1.33) 
#define Wdec3to8p       (1.33)
#define WdecNORn        (1.20)
#define WdecNORp        (2.62)
#define Wdecinvn        (1.46)
#define Wdecinvp        (3.93)
#define Wdff            (4.6)

#define Wworddrivemax   (9.225)
#define Wmemcella   (0.221)
#define Wmemcellr   (0.369)
#define Wmemcellw   (0.194)
#define Wmemcellbscale  1.87       
#define Wbitpreequ  (0.923)

#define Wbitmuxn    (0.923)
#define WsenseQ1to4 (0.369)
#define Wcompinvp1  (0.924)
#define Wcompinvn1  (0.554)
#define Wcompinvp2  (1.845)
#define Wcompinvn2  (1.107)
#define Wcompinvp3  (3.69)
#define Wcompinvn3  (2.214)
#define Wevalinvp   (1.842)
#define Wevalinvn   (7.368)

#define Wcompn      (1.845)
#define Wcompp      (2.768)
#define Wcomppreequ     (3.692)
#define Wmuxdrv12n  (2.773)
#define Wmuxdrv12p  (4.618)
#define WmuxdrvNANDn    (1.848)
#define WmuxdrvNANDp    (7.38)
#define WmuxdrvNORn (5.535)
#define WmuxdrvNORp (7.380)
#define Wmuxdrv3n   (18.45)
#define Wmuxdrv3p   (44.28)
#define Woutdrvseln (1.105)
#define Woutdrvselp (1.842)
#define Woutdrvnandn    (2.214)
#define Woutdrvnandp    (0.923)
#define Woutdrvnorn (0.554)
#define Woutdrvnorp (3.69)
#define Woutdrivern (4.428)
#define Woutdriverp (7.380)
#define Wbusdrvn    (4.421)
#define Wbusdrvp    (7.368)

#define Wcompcellpd2    (0.221)
#define Wcompdrivern    (36.84)
#define Wcompdriverp    (73.77)
#define Wcomparen2      (3.684)
#define Wcomparen1      (1.842)
#define Wmatchpchg      (0.921)
#define Wmatchinvn      (0.923)
#define Wmatchinvp      (1.852)
#define Wmatchnandn     (1.852)
#define Wmatchnandp     (0.921)
#define Wmatchnorn      (1.845)
#define Wmatchnorp      (0.923)

#define WSelORn         (0.923)
#define WSelORprequ     (3.684)
#define WSelPn          (0.921)
#define WSelPp          (1.382)
#define WSelEnn         (0.446)
#define WSelEnp         (0.923)

#define Wsenseextdrv1p  (3.684)
#define Wsenseextdrv1n  (2.211)
#define Wsenseextdrv2p  (18.42)
#define Wsenseextdrv2n  (11.052)

#elif (PARM(TRANSISTOR_TYPE) == HVT)  
#define Wdecdrivep      (3.11) 
#define Wdecdriven      (1.90)
#define Wdec3to8n       (1.33) 
#define Wdec3to8p       (1.33)
#define WdecNORn        (0.90)
#define WdecNORp        (1.82)
#define Wdecinvn        (0.46)
#define Wdecinvp        (0.93)
#define Wdff            (3.8)

#define Wworddrivemax   (9.18)
#define Wmemcella   (0.220)
#define Wmemcellr   (0.367)
#define Wmemcellw   (0.193)
#define Wmemcellbscale  1.87       
#define Wbitpreequ  (0.918)

#define Wbitmuxn    (0.918)
#define WsenseQ1to4 (0.366)
#define Wcompinvp1  (0.920)
#define Wcompinvn1  (0.551)
#define Wcompinvp2  (1.836)
#define Wcompinvn2  (1.102)
#define Wcompinvp3  (3.672)
#define Wcompinvn3  (2.203)
#define Wevalinvp   (1.83)
#define Wevalinvn   (7.32)

#define Wcompn      (1.836)
#define Wcompp      (2.754)
#define Wcomppreequ (3.672)
#define Wmuxdrv12n  (2.760)
#define Wmuxdrv12p  (4.60)
#define WmuxdrvNANDn    (1.836)
#define WmuxdrvNANDp    (7.344)
#define WmuxdrvNORn (5.508)
#define WmuxdrvNORp (7.344)
#define Wmuxdrv3n   (18.36)
#define Wmuxdrv3p   (44.064)
#define Woutdrvseln (1.098)
#define Woutdrvselp (1.83)
#define Woutdrvnandn    (2.203)
#define Woutdrvnandp    (0.918)
#define Woutdrvnorn (0.551)
#define Woutdrvnorp (3.672)
#define Woutdrivern (4.406)
#define Woutdriverp (7.344)
#define Wbusdrvn    (4.392)
#define Wbusdrvp    (7.32)

#define Wcompcellpd2    (0.220)
#define Wcompdrivern    (36.6)
#define Wcompdriverp    (73.33)
#define Wcomparen2      (3.66)
#define Wcomparen1      (1.83)
#define Wmatchpchg      (0.915)
#define Wmatchinvn      (0.915)
#define Wmatchinvp      (1.85)
#define Wmatchnandn     (1.85)
#define Wmatchnandp     (0.915)
#define Wmatchnorn      (1.83)
#define Wmatchnorp      (0.915)

#define WSelORn         (0.915)
#define WSelORprequ     (3.66)
#define WSelPn          (0.915)
#define WSelPp          (1.373)
#define WSelEnn         (0.458)
#define WSelEnp         (0.915)

#define Wsenseextdrv1p  (3.66)
#define Wsenseextdrv1n  (2.196)
#define Wsenseextdrv2p  (18.3)
#define Wsenseextdrv2n  (10.98)

#endif /* PARM(TRANSISTOR_TYPE) */

#define CamCellHeight    (2.9575)  	/* derived from Cacti 5.3 */ 
#define CamCellWidth     (2.535)    /* derived from Cacti 5.3 */

#define MatchlineSpacing (0.522)   
#define TaglineSpacing   (0.522)   

#define CrsbarCellHeight (2.06 * SCALE_Crs)   
#define CrsbarCellWidth  (2.06 * SCALE_Crs)  

#define krise       (0.348e-10) 
#define tsensedata  (0.5046e-10)   
#define tsensetag   (0.2262e-10)  
#define tfalldata   (0.609e-10)  
#define tfalltag    (0.6609e-10) 

#endif /* PARM(TECH_POINT) */

/*=======================PARAMETERS for Link===========================*/

#if(PARM(TECH_POINT) == 90) /* PARAMETERS for Link at 90nm */
#if (WIRE_LAYER_TYPE == LOCAL)
#define WireMinWidth            214e-9
#define WireMinSpacing          214e-9
#define WireMetalThickness      363.8e-9
#define WireBarrierThickness    10e-9
#define WireDielectricThickness 363.8e-9
#define WireDielectricConstant  3.3

#elif (WIRE_LAYER_TYPE == INTERMEDIATE)
#define WireMinWidth            275e-9
#define WireMinSpacing          275e-9
#define WireMetalThickness      467.5e-9
#define WireBarrierThickness    10e-9
#define WireDielectricThickness 412.5e-9
#define WireDielectricConstant  3.3

#elif (WIRE_LAYER_TYPE == GLOBAL)
#define WireMinWidth            410e-9 
#define WireMinSpacing          410e-9
#define WireMetalThickness      861e-9 
#define WireBarrierThickness    10e-9
#define WireDielectricThickness 779e-9 
#define WireDielectricConstant  3.3

#endif /*WIRE_LAYER_TYPE for 90nm*/

#elif(PARM(TECH_POINT) == 65) /* PARAMETERS for Link at 65nm */
#if (WIRE_LAYER_TYPE == LOCAL)
#define WireMinWidth 		    136e-9
#define WireMinSpacing 		    136e-9
#define WireMetalThickness	    231.2e-9
#define WireBarrierThickness	    4.8e-9
#define WireDielectricThickness	    231.2e-9
#define WireDielectricConstant	    2.85

#elif (WIRE_LAYER_TYPE == INTERMEDIATE)
#define WireMinWidth                140e-9
#define WireMinSpacing              140e-9
#define WireMetalThickness          252e-9
#define WireBarrierThickness        5.2e-9
#define WireDielectricThickness     224e-9
#define WireDielectricConstant      2.85

#elif (WIRE_LAYER_TYPE == GLOBAL)
#define WireMinWidth                400e-9 
#define WireMinSpacing              400e-9
#define WireMetalThickness          400e-9 
#define WireBarrierThickness        5.2e-9
#define WireDielectricThickness     790e-9 
#define WireDielectricConstant      2.9

#endif /*WIRE_LAYER_TYPE for 65nm*/

#elif(PARM(TECH_POINT) == 45) /* PARAMETERS for Link at 45nm */
#if (WIRE_LAYER_TYPE == LOCAL)
#define WireMinWidth                    45e-9
#define WireMinSpacing                  45e-9
#define WireMetalThickness              129.6e-9
#define WireBarrierThickness            3.3e-9
#define WireDielectricThickness         162e-9
#define WireDielectricConstant          2.0

#elif (WIRE_LAYER_TYPE == INTERMEDIATE)
#define WireMinWidth                45e-9  
#define WireMinSpacing              45e-9	
#define WireMetalThickness          129.6e-9
#define WireBarrierThickness        3.3e-9
#define WireDielectricThickness     72e-9 	
#define WireDielectricConstant      2.0

#elif (WIRE_LAYER_TYPE == GLOBAL)
#define WireMinWidth                67.5e-9 
#define WireMinSpacing              67.5e-9
#define WireMetalThickness          155.25e-9 
#define WireBarrierThickness        3.3e-9
#define WireDielectricThickness     141.75e-9 
#define WireDielectricConstant      2.0

#endif /*WIRE_LAYER_TYPE for 45nm*/

#elif(PARM(TECH_POINT) == 32) /* PARAMETERS for Link at 32nm */
#if (WIRE_LAYER_TYPE == LOCAL)
#define WireMinWidth                    32e-9
#define WireMinSpacing                  32e-9
#define WireMetalThickness              60.8e-9
#define WireBarrierThickness            2.4e-9
#define WireDielectricThickness         60.8e-9
#define WireDielectricConstant          1.9

#elif (WIRE_LAYER_TYPE == INTERMEDIATE)
#define WireMinWidth                32e-9
#define WireMinSpacing              32e-9
#define WireMetalThickness          60.8e-9
#define WireBarrierThickness        2.4e-9
#define WireDielectricThickness     54.4e-9 
#define WireDielectricConstant      1.9

#elif (WIRE_LAYER_TYPE == GLOBAL)
#define WireMinWidth                48e-9 
#define WireMinSpacing              48e-9
#define WireMetalThickness          120e-9 
#define WireBarrierThickness        2.4e-9
#define WireDielectricThickness     110.4e-9 
#define WireDielectricConstant      1.9

#endif /* WIRE_LAYER_TYPE for 32nm*/

#endif /* PARM(TECH_POINT) */

/*===================================================================*/
/*parameters for insertion buffer for links at 90nm*/
#if(PARM(TECH_POINT) == 90)
#define BufferDriveResistance       5.12594e+03
#define BufferIntrinsicDelay        4.13985e-11
#if(PARM(TRANSISTOR_TYPE) == LVT)
#define BufferInputCapacitance      1.59e-15
#define BufferPMOSOffCurrent        116.2e-09 
#define BufferNMOSOffCurrent        52.1e-09  
#define ClockCap 		    2.7e-14
#elif(PARM(TRANSISTOR_TYPE) == NVT)
#define BufferInputCapacitance      4.7e-15
#define BufferPMOSOffCurrent        67.6e-09 
#define BufferNMOSOffCurrent        31.1e-09  
#define ClockCap                    1.0e-14
#elif(PARM(TRANSISTOR_TYPE) == HVT)
#define BufferInputCapacitance      15.0e-15//9.5e-15
#define BufferPMOSOffCurrent        19.2e-09 
#define BufferNMOSOffCurrent        10.1e-09  
#define ClockCap                    0.3e-15
#endif
/*parameters for insertion buffer for links at 65nm*/
#elif(PARM(TECH_POINT) == 65)
#define BufferDriveResistance       6.77182e+03
#define BufferIntrinsicDelay	    3.31822e-11 
#if(PARM(TRANSISTOR_TYPE) == LVT)
#define BufferPMOSOffCurrent	    317.2e-09 
#define BufferNMOSOffCurrent	    109.7e-09
#define BufferInputCapacitance      1.3e-15 
#define ClockCap                    2.6e-14
#elif(PARM(TRANSISTOR_TYPE) == NVT)
#define BufferPMOSOffCurrent        113.1e-09 
#define BufferNMOSOffCurrent        67.3e-09 
#define BufferInputCapacitance      2.6e-15
#define ClockCap                    1.56e-14
#elif(PARM(TRANSISTOR_TYPE) == HVT)
#define BufferPMOSOffCurrent        35.2e-09 
#define BufferNMOSOffCurrent        18.4e-09 
#define BufferInputCapacitance      7.8e-15
#define ClockCap                    0.9e-15
#endif

/*parameters for insertion buffer for links at 45nm*/
#elif(PARM(TECH_POINT) == 45)
#define BufferDriveResistance       7.3228e+03
#define BufferIntrinsicDelay        4.6e-11
#if(PARM(TRANSISTOR_TYPE) == LVT)
#define BufferInputCapacitance      1.25e-15
#define BufferPMOSOffCurrent        1086.75e-09 
#define BufferNMOSOffCurrent        375.84e-09 
#define ClockCap                    2.5e-14
#elif(PARM(TRANSISTOR_TYPE) == NVT)
#define BufferInputCapacitance      2.5e-15 	
#define BufferPMOSOffCurrent        382.3e-09 
#define BufferNMOSOffCurrent        195.5e-09 
#define ClockCap                    1.5e-14
#elif(PARM(TRANSISTOR_TYPE) == HVT)
#define BufferInputCapacitance      7.5e-15
#define BufferPMOSOffCurrent        76.4e-09 
#define BufferNMOSOffCurrent        39.1e-09 
#define ClockCap                    0.84e-15
#endif
/*parameters for insertion buffer for links at 32nm*/
#elif(PARM(TECH_POINT) == 32)
#define BufferDriveResistance       10.4611e+03
#define BufferIntrinsicDelay        4.0e-11
#if(PARM(TRANSISTOR_TYPE) == LVT)
#define BufferPMOSOffCurrent        1630.08e-09 
#define BufferNMOSOffCurrent        563.74e-09
#define BufferInputCapacitance      1.2e-15 
#define ClockCap                    2.2e-14
#elif(PARM(TRANSISTOR_TYPE) == NVT)
#define BufferPMOSOffCurrent        792.4e-09 
#define BufferNMOSOffCurrent        405.1e-09
#define BufferInputCapacitance      2.4e-15 
#define ClockCap                    1.44e-14
#elif(PARM(TRANSISTOR_TYPE) == HVT)
#define BufferPMOSOffCurrent        129.9e-09 
#define BufferNMOSOffCurrent        66.4e-09
#define BufferInputCapacitance      7.2e-15 
#define ClockCap                    0.53e-15
#endif
#endif /*PARM(TECH_POINT)*/


/*======================Parameters for Area===========================*/
#if(PARM(TECH_POINT) == 90)
#define AreaNOR         (4.23)  
#define AreaINV         (2.82)  
#define AreaAND     	(4.23)  
#define AreaDFF         (16.23) 
#define AreaMUX2        (7.06)  
#define AreaMUX3        (11.29) 
#define AreaMUX4        (16.93) 
#elif(PARM(TECH_POINT) <= 65)
#define AreaNOR     (2.52 * SCALE_T)   
#define AreaINV     (1.44 * SCALE_T)  
#define AreaDFF     (8.28 * SCALE_T) 
#define AreaAND     (2.52 * SCALE_T)  
#define AreaMUX2    (6.12 * SCALE_T)  
#define AreaMUX3    (9.36 * SCALE_T) 
#define AreaMUX4    (12.6 * SCALE_T) 
#endif

#endif /* PARM(TECH_POINT) <= 90 */

#endif /* _SIM_POWER_V2_H */
