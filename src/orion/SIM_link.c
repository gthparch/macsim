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
#include <stdlib.h>

#include "SIM_parameter.h"
#include "SIM_link.h"

/* Link power and area model is only supported for 90nm, 65nm, 45nm and 32nm */
#if ( PARM(TECH_POINT) <= 90 )

// The following function computes the wire resistance considering
// width-spacing combination and a width-dependent resistivity model
double computeResistance(double Length) { 
    double r, rho;
	int widthSpacingConfig = PARM(width_spacing);

    if (widthSpacingConfig == SWIDTH_SSPACE) {
        // rho is in "ohm.m"
        rho = 2.202e-8 + (1.030e-15  / (WireMinWidth - 2*WireBarrierThickness));
        r = ((rho * Length) / ((WireMinWidth - 2*WireBarrierThickness) *
            (WireMetalThickness - WireBarrierThickness))); // r is in "ohm" 
    } else if (widthSpacingConfig == SWIDTH_DSPACE) {
        // rho is in "ohm.m"
        rho = 2.202e-8 + (1.030e-15  / (WireMinWidth - 2*WireBarrierThickness));
        r = ((rho * Length) / ((WireMinWidth - 2*WireBarrierThickness) *
            (WireMetalThickness - WireBarrierThickness))); // r is in "ohm"
    } else if (widthSpacingConfig == DWIDTH_SSPACE) {
        // rho is in "ohm.m"
        rho = 2.202e-8 + (1.030e-15  / (2*WireMinWidth - 2*WireBarrierThickness));
        r = ((rho * Length) / ((2*WireMinWidth - 2*WireBarrierThickness) *
            (WireMetalThickness - WireBarrierThickness))); // r is in "ohm"
    } else if (widthSpacingConfig == DWIDTH_DSPACE){
        // rho is in "ohm.m"
        rho = 2.202e-8 + (1.030e-15  / (2*WireMinWidth - 2*WireBarrierThickness));
        r = ((rho * Length) / ((2*WireMinWidth - 2*WireBarrierThickness) *
            (WireMetalThickness - WireBarrierThickness))); // r is in "ohm"
    } else {
		fprintf(stderr, "ERROR, Specified width spacing configuration is not supported.\n");
        exit( 1 );
    }

    return r;
}


// COMPUTE WIRE CAPACITANCE using PTM models
// The parameter "Length" has units of "m"  
double computeGroundCapacitance(double Length) {
    double c_g;
	int widthSpacingConfig = PARM(width_spacing);

    if (widthSpacingConfig == SWIDTH_SSPACE) {
        double A = (WireMinWidth/WireDielectricThickness);
        double B = 2.04*pow((WireMinSpacing/(WireMinSpacing +
                    0.54*WireDielectricThickness)), 1.77);
        double C = pow((WireMetalThickness/(WireMetalThickness +
                    4.53*WireDielectricThickness)), 0.07);
        c_g = WireDielectricConstant*8.85e-12*(A+(B*C))*Length; // c_g is in "F"
    } else if (widthSpacingConfig == SWIDTH_DSPACE) {
        double minSpacingNew = 2*WireMinSpacing + WireMinWidth;
        double A = (WireMinWidth/WireDielectricThickness);
        double B = 2.04*pow((minSpacingNew/(minSpacingNew +
                    0.54*WireDielectricThickness)), 1.77);
        double C = pow((WireMetalThickness/(WireMetalThickness +
                    4.53*WireDielectricThickness)), 0.07);
        c_g = WireDielectricConstant*8.85e-12*(A+(B*C))*Length; // c_g is in "F"    
    } else if (widthSpacingConfig == DWIDTH_SSPACE) {
        double minWidthNew = 2*WireMinWidth;
        double A = (minWidthNew/WireDielectricThickness);
        double B = 2.04*pow((WireMinSpacing/(WireMinSpacing +
                    0.54*WireDielectricThickness)), 1.77);
        double C = pow((WireMetalThickness/(WireMetalThickness +
                    4.53*WireDielectricThickness)), 0.07);
        c_g = WireDielectricConstant*8.85e-12*(A+(B*C))*Length; // c_g is in "F"    
    } else if (widthSpacingConfig == DWIDTH_DSPACE) {
        double minWidthNew = 2*WireMinWidth;
        double minSpacingNew = 2*WireMinSpacing;
        double A = (minWidthNew/WireDielectricThickness);
        double B = 2.04*pow((minSpacingNew/(minSpacingNew+
                    0.54*WireDielectricThickness)), 1.77);
        double C = pow((WireMetalThickness/(WireMetalThickness +
                    4.53*WireDielectricThickness)), 0.07);
        c_g = WireDielectricConstant*8.85e-12*(A+(B*C))*Length; // c_g is in "F"    
    } else {
        fprintf(stderr, "ERROR, Specified width spacing configuration is not supported.\n");
        exit( 1 );
    }

    return c_g; // Ground capacitance is in "F"     
}

// Computes the coupling capacitance considering cross-talk
// The parameter "Length" has units of "m"
double computeCouplingCapacitance(double Length) {
    double c_c;
	int widthSpacingConfig = PARM(width_spacing);

    if (widthSpacingConfig == SWIDTH_SSPACE) {
        double A = 1.14*(WireMetalThickness/WireMinSpacing) *
                exp(-4*WireMinSpacing/(WireMinSpacing + 8.01*WireDielectricThickness));
        double B = 2.37*pow((WireMinWidth/(WireMinWidth + 0.31*WireMinSpacing)), 0.28);
        double C = pow((WireDielectricThickness/(WireDielectricThickness +
                8.96*WireMinSpacing)), 0.76) *
                    exp(-2*WireMinSpacing/(WireMinSpacing + 6*WireDielectricThickness));
        c_c = WireDielectricConstant*8.85e-12*(A + (B*C))*Length;
    } else if (widthSpacingConfig == SWIDTH_DSPACE) {
        double minSpacingNew = 2*WireMinSpacing + WireMinWidth;
        double A = 1.14*(WireMetalThickness/minSpacingNew) *
                exp(-4*minSpacingNew/(minSpacingNew + 8.01*WireDielectricThickness));
        double B = 2.37*pow((WireMinWidth/(WireMinWidth + 0.31*minSpacingNew)), 0.28);
        double C = pow((WireDielectricThickness/(WireDielectricThickness +
                8.96*minSpacingNew)), 0.76) *
                exp(-2*minSpacingNew/(minSpacingNew + 6*WireDielectricThickness));
        c_c = WireDielectricConstant*8.85e-12*(A + (B*C))*Length;
    } else if (widthSpacingConfig == DWIDTH_SSPACE) {
        double minWidthNew = 2*WireMinWidth;
        double A = 1.14*(WireMetalThickness/WireMinSpacing) *
                exp(-4*WireMinSpacing/(WireMinSpacing + 8.01*WireDielectricThickness));
        double B = 2.37*pow((2*minWidthNew/(2*minWidthNew + 0.31*WireMinSpacing)), 0.28);
        double C = pow((WireDielectricThickness/(WireDielectricThickness +
                8.96*WireMinSpacing)), 0.76) *
                exp(-2*WireMinSpacing/(WireMinSpacing + 6*WireDielectricThickness));
        c_c = WireDielectricConstant*8.85e-12*(A + (B*C))*Length;
    } else if (widthSpacingConfig == DWIDTH_DSPACE) {
        double minWidthNew = 2*WireMinWidth;
        double minSpacingNew = 2*WireMinSpacing;
        double A = 1.14*(WireMetalThickness/minSpacingNew) *
                exp(-4*minSpacingNew/(minSpacingNew + 8.01*WireDielectricThickness));
        double B = 2.37*pow((minWidthNew/(minWidthNew + 0.31*minSpacingNew)), 0.28);
        double C = pow((WireDielectricThickness/(WireDielectricThickness +
                8.96*minSpacingNew)), 0.76) *
                exp(-2*minSpacingNew/(minSpacingNew + 6*WireDielectricThickness));
        c_c = WireDielectricConstant*8.85e-12*(A + (B*C))*Length;
    } else {
		fprintf(stderr, "ERROR, Specified width spacing configuration is not supported.\n");
        exit(1);
    }

    return c_c; // Coupling capacitance is in "F"       
}

// OPTIMUM K and H CALCULATION
// Computes the optimum number and size of repeaters for the link   

void getOptBuffering(int *k , double *h , double Length) {
    if (PARM(buffering_scheme) == MIN_DELAY) {
        if (PARM(shielding)) {
			double r = computeResistance(Length);
            double c_g = 2*computeGroundCapacitance(Length);
            double c_c = 2*computeCouplingCapacitance(Length);

            *k = (int) sqrt(((0.4*r*c_g)+(0.57*r*c_c))/
                (0.7*BufferDriveResistance*BufferInputCapacitance)); //k is the number of buffers to be inserted
            *h = sqrt(((0.7*BufferDriveResistance*c_g)+
					(1.4*1.5*BufferDriveResistance*c_c))/(0.7*r*BufferInputCapacitance)); //the size of the buffers to be inserted
        } else {
            double r = computeResistance(Length);
            double c_g = 2*computeGroundCapacitance(Length);
            double c_c = 2*computeCouplingCapacitance(Length);

            *k = (int) sqrt(((0.4*r*c_g)+(1.51*r*c_c))/
                (0.7*BufferDriveResistance*BufferInputCapacitance));
            *h = sqrt(((0.7*BufferDriveResistance*c_g)+
                (1.4*2.2*BufferDriveResistance*c_c))/(0.7*r*BufferInputCapacitance));

        }
    } else if (PARM(buffering_scheme)== STAGGERED) {
            double r = computeResistance(Length);
            double c_g = 2*computeGroundCapacitance(Length);
            double c_c = 2*computeCouplingCapacitance(Length);

            *k = (int) sqrt(((0.4*r*c_g)+(0.57*r*c_c))/
                (0.7*BufferDriveResistance*BufferInputCapacitance));
            *h = sqrt(((0.7*BufferDriveResistance*c_g)+
            (1.4*1.5*BufferDriveResistance*c_c))/(0.7*r*BufferInputCapacitance));
    } else {
		fprintf(stderr, "ERROR, Specified buffering scheme is not supported.\n");
        exit(1);
    }
}

// unit will be joule/bit/meter
double LinkDynamicEnergyPerBitPerMeter(double Length, double vdd) 
{
    double cG = 2*computeGroundCapacitance(Length);
    double cC = 2*computeCouplingCapacitance(Length);
    double totalWireC = cC + cG;  // for dyn power
    int k;
    double h;
	int *ptr_k = &k;
	double *ptr_h = &h;

    getOptBuffering(ptr_k, ptr_h, Length);
    double bufferC = ((double)k) * (BufferInputCapacitance) * h;

    return (((totalWireC + bufferC) * vdd * vdd)/Length);
}

// unit will be Watt/meter
double LinkLeakagePowerPerMeter(double Length, double vdd) 
{
    int k;
    double h;
	int *ptr_k = &k;
	double *ptr_h = &h;
    getOptBuffering(ptr_k, ptr_h, Length);

    double pmosLeakage = BufferPMOSOffCurrent * h * k;
    double nmosLeakage = BufferNMOSOffCurrent * h * k;
    return ((vdd*(pmosLeakage + nmosLeakage)/2)/Length);
}

double LinkArea(double Length, unsigned NumBits) 
{
	// Link area has units of "um^2"
	double deviceArea =0, routingArea = 0;
    int k;
    double h;
    int *ptr_k = &k;
    double *ptr_h = &h;
    getOptBuffering(ptr_k, ptr_h, Length);
    #if(PARM(TECH_POINT) == 90)
    	deviceArea = NumBits * k * ((h * 0.43) + 1.38);
    #elif(PARM(TECH_POINT) <= 65)
        deviceArea = (NumBits) * k * ((h * 0.45) + 0.65) * SCALE_T;
    #endif

    routingArea = (NumBits * (WireMinWidth*1e6 + WireMinSpacing*1e6) + WireMinSpacing*1e6) * Length*1e6; 
    return deviceArea + routingArea;
}

#endif /*( PARM(TECH_POINT) <= 90 */
