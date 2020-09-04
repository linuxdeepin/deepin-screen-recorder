#ifndef NEU_QUANT_H
#define NEU_QUANT_H

#include <stdio.h>
#include <stdint.h>

#define		NETSIZE				256								// number of colours used

// four primes near 500 - assume no image has a length so large
// that it is divisible by all four primes
#define		PRIME1				499
#define		PRIME2				491	
#define		PRIME3				487
#define		PRIME4				503
#define		MIN_PICTURE_BYTES	3 * PRIME4						// minimum size for input image

/* minimum size for input image */

/*
* Program Skeleton ---------------- [select samplefac in range 1..30] [read
* image from input file] pic = (unsigned char*) malloc(3*width*height);
* initnet(pic,3*width*height,samplefac); learn(); unbiasnet(); [write output
* image header, using writecolourmap(f)] inxbuild(); write output image using
* inxsearch(b,g,r)
*/

/*
* Network Definitions -------------------
*/

#define		MAX_NET_POS			NETSIZE - 1
#define		NET_BIAS_SHIFT		4									// bias for colour values
#define		NCYCLES				100									// no, of learning cycles

// defs for freq and bias

#define		INT_BIAS_SHIFT		16									// bias for fractions
#define		INT_BIAS			((int) 1) << INT_BIAS_SHIFT
#define		GAMMA_SHIFT			10									// gamma = 1024
#define		GAMMA				((int) 1) << GAMMA_SHIFT
#define		BETA_SHIFT			10
#define		BETA				(INT_BIAS >> BETA_SHIFT)			// beta = 1/1024
#define		BETA_GAMMA			INT_BIAS << (GAMMA_SHIFT - BETA_SHIFT)

// defs for decreasing radius factor
#define		INITRAD				NETSIZE>>3							// for 256 cols, radius starts
#define		RADIUS_BIAS_SHIFT	6									// at 32.0 biased by 6 bits
#define		RADIUS_BIAS			(((int)1) << RADIUS_BIAS_SHIFT)
#define		INIT_RADIUS			(INITRAD * RADIUS_BIAS)				// and decreases by a
#define		RADIUS_DEC			30									// factor of 1/30 each cycle

// defs for decreasing alpha factor
#define		ALPHA_BIAS_SHIFT	10									// alpha starts at 1.0
#define		INIT_ALPHA			((int)1) << ALPHA_BIAS_SHIFT
int ALPHA_DEC;					// biased by 10 bits

// radbias and alpharadbias used for radpower calculation
#define		RAD_BIAS_SHIFT		8
#define		RAD_BIAS			((int) 1) << RAD_BIAS_SHIFT)
#define		ALPHA_RAD_B_SHIFT	ALPHA_BIAS_SHIFT + RAD_BIAS_SHIFT
#define		ALPHA_RAD_BIAS		((int) 1) << ALPHA_RAD_B_SHIFT


typedef struct _NeuQuant {

	uint32_t* thePicture;		// the input image itself
	int lengthCount;			// lengthcount = H*W*3
	int sampleFac;				// sampling factor 1..30
	int** network;				// the network itself - [NETSIZE][4]
	int* netindex;
	int* bias;
	int* freq;
	int* radpower;

} NeuQuant;

NeuQuant * initNeuQuant(NeuQuant * neuQuant, uint32_t* pixels, int len, int sample);
uint32_t * colorMap(NeuQuant * neuQuant);
int inxbuild(NeuQuant * neuQuant);
int learn(NeuQuant * neuQuant);
int map(NeuQuant * neuQuant, int b, int g, int r);
uint32_t * process(NeuQuant * neuQuant);
int unbiasnet(NeuQuant * neuQuant);
int alterneigh(NeuQuant * neuQuant, int rad, int i, int b, int g, int r);
int altersingle(NeuQuant * neuQuant, int alpha, int i, int b, int g, int r);
int contest(NeuQuant * neuQuant, int b, int g, int r);
int finishNeuQuant(NeuQuant * neuQuant);

#endif NEU_QUANT_H