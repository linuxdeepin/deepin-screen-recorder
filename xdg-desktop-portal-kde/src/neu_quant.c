/*
* NeuQuant Neural-Net Quantization Algorithm
* ------------------------------------------
*
* Copyright (c) 1994 Anthony Dekker
*
* NEUQUANT Neural-Net quantization algorithm by Anthony Dekker, 1994. See
* "Kohonen neural networks for optimal colour quantization" in "Network:
* Computation in Neural Systems" Vol. 5 (1994) pp 351-367. for a discussion of
* the algorithm.
*
* Any party obtaining a copy of these files from the author, directly or
* indirectly, is granted, free of charge, a full and unrestricted irrevocable,
* world-wide, paid up, royalty-free, nonexclusive right and license to deal in
* this software and documentation files (the "Software"), including without
* limitation the rights to use, copy, modify, merge, publish, distribute,
* sublicense, and/or sell copies of the Software, and to permit persons who
* receive copies from any such party to do so, with the only requirement being
* that this copyright notice remain intact.
*/

#include "neu_quant.h"

NeuQuant * initNeuQuant(NeuQuant * neuQuant, uint32_t* pixels, int len, int sample) {

	neuQuant = (NeuQuant *)malloc(sizeof(NeuQuant));

	neuQuant->thePicture = pixels;
	neuQuant->lengthCount = len;
	neuQuant->sampleFac = sample;

	neuQuant->network = (int **)malloc(sizeof(int*) * NETSIZE);
	neuQuant->netindex = (int *)malloc(sizeof(int) * 256);
	neuQuant->bias = (int *)malloc(sizeof(int) * NETSIZE);
	neuQuant->freq = (int *)malloc(sizeof(int) * NETSIZE);
	neuQuant->radpower = (int *)malloc(sizeof(int) * INITRAD);

	for (int i = 0; i < NETSIZE; i++) {
		neuQuant->network[i] = (int *)malloc(sizeof(int) * 4);
		neuQuant->network[i][0] = neuQuant->network[i][1] = neuQuant->network[i][2]
			= (i << (NET_BIAS_SHIFT + 8)) / NETSIZE;
		neuQuant->freq[i] = INT_BIAS / NETSIZE;					// 1 / NETSIZE
		neuQuant->bias[i] = 0;
	}

	return neuQuant;
}

/*
* Insertion sort of network and building of netindex[0..255] (to do after
* unbias)
* -------------------------------------------------------------------------------
*/
int inxbuild(NeuQuant * neuQuant) {
	
	int i, j, smallpos, smallval;
	int *p, *q;
	int previoscol, startpos;

	previoscol = 0;
	startpos = 0;

	for (i = 0; i < NETSIZE; i++) {
		p = neuQuant->network[i];
		smallpos = i;
		smallval = p[1];			// index on g
		// find smallest in i..NETSIZE-1
		for (j = i + 1; j < NETSIZE; j++) {
			q = neuQuant->network[j];
			if (q[1] < smallval) {
				smallpos = j;		// index on g
				smallval = q[1];	// index on g
			}
		}
		q = neuQuant->network[smallpos];
		// swap p(i) and q(smallpos) entries
		if (i != smallpos) {
			j = q[0];
			q[0] = p[0];
			p[0] = j;
			j = q[1];
			q[1] = p[1];
			p[1] = j;
			j = q[2];
			q[2] = p[2];
			p[2] = j;
			j = q[3];
			q[3] = p[3];
			p[3] = j;
		}
		// smallval entry is now in position i
		if (smallval != previoscol) {
			neuQuant->netindex[previoscol] = (startpos + i) >> 1;
			for (j = previoscol + 1; j < smallval; j++) {
				neuQuant->netindex[j] = i;
				previoscol = smallval;
				startpos = i;
			}
		}
		neuQuant->netindex[previoscol] = (startpos + MAX_NET_POS) >> 1;
		for (j = previoscol + 1; j < 256; j++)
			neuQuant->netindex[j] = MAX_NET_POS;		// really 256
	}

	return 0;
}

int learn(NeuQuant * neuQuant) {

	int i, j, b, g, r;
	int radius, rad, alpha, step, delta, samplepixels;
	uint32_t * p;
	int pix, lim;

	if (neuQuant->lengthCount < MIN_PICTURE_BYTES)
		neuQuant->sampleFac = 1;
	ALPHA_DEC = 30 + ((neuQuant->sampleFac - 1) / 3);
	p = neuQuant->thePicture;
	pix = 0;
	lim = neuQuant->lengthCount;
	samplepixels = neuQuant->lengthCount / (3 * neuQuant->sampleFac);
	delta = samplepixels / NCYCLES;
	alpha = INIT_ALPHA;
	radius = INIT_RADIUS;

	rad = radius >> RADIUS_BIAS_SHIFT;
	if (rad <= 1)
		rad = 0;
	for (i = 0; i < rad; i++)
		neuQuant->radpower[i] = alpha * (((rad * rad - i * i) * RAD_BIAS) / (rad * rad);

	if (neuQuant->lengthCount < MIN_PICTURE_BYTES)
		step = 3;
	else if ((neuQuant->lengthCount % PRIME1) != 0)
		step = 3 * PRIME1;
	else {
		if ((neuQuant->lengthCount % PRIME2) != 0)
			step = 3 * PRIME2;
		else {
			if ((neuQuant->lengthCount % PRIME3) != 0)
				step = 3 * PRIME3;
			else
				step = 3 * PRIME4;
		}
	}

	i = 0;
	while (i < samplepixels) {
		b = (p[pix + 0] & 0xff) << NET_BIAS_SHIFT;
		g = (p[pix + 1] & 0xff) << NET_BIAS_SHIFT;
		r = (p[pix + 2] & 0xff) << NET_BIAS_SHIFT;
		j = contest(neuQuant, b, g, r);

		altersingle(neuQuant, alpha, j, b, g, r);
		if (rad != 0)
			alterneigh(neuQuant, rad, j, b, g, r);			// alter neighbours

		pix += step;
		if (pix >= lim)
			pix -= neuQuant->lengthCount;

		i++;
		if (delta == 0)
			delta = 1;
		if (i % delta == 0) {
			alpha -= alpha / ALPHA_DEC;
			radius -= radius / RADIUS_DEC;
			rad = radius >> RADIUS_BIAS_SHIFT;
			if (rad <= 1)
				rad = 0;
			for (j = 0; j < rad; j++)
				neuQuant->radpower[j] = alpha * (((rad * rad - j * j) * RAD_BIAS) / (rad * rad);
		}
	}

	return 0;
}

int map(NeuQuant * neuQuant, int b, int g, int r) {

	int i, j, dist, a, bestd;
	int *p;
	int best;

	bestd = 1000;						// biggest possible dist is 256*3
	best = -1;
	i = neuQuant->netindex[g];			// index on g
	j = i - 1;							// start at netindex[g] and work outwards

	while ((i < NETSIZE) || (j >= 0)) {
		if (i < NETSIZE) {
			p = neuQuant->network[i];
			dist = p[1] - g; /* inx key */
			if (dist >= bestd)
				i = NETSIZE; /* stop iter */
			else {
				i++;
				if (dist < 0)
					dist = -dist;
				a = p[0] - b;
				if (a < 0)
					a = -a;
				dist += a;
				if (dist < bestd) {
					a = p[2] - r;
					if (a < 0)
						a = -a;
					dist += a;
					if (dist < bestd) {
						bestd = dist;
						best = p[3];
					}
				}
			}
		}
		if (j >= 0) {
			p = neuQuant->network[j];
			dist = g - p[1]; /* inx key - reverse dif */
			if (dist >= bestd)
				j = -1; /* stop iter */
			else {
				j--;
				if (dist < 0)
					dist = -dist;
				a = p[0] - b;
				if (a < 0)
					a = -a;
				dist += a;
				if (dist < bestd) {
					a = p[2] - r;
					if (a < 0)
						a = -a;
					dist += a;
					if (dist < bestd) {
						bestd = dist;
						best = p[3];
					}
				}
			}
		}
	}
	return (best);
}

/*
* Move adjacent neurons by precomputed alpha*(1-((i-j)^2/[r]^2)) in
* radpower[|i-j|]
* ---------------------------------------------------------------------------------
*/
int alterneigh(NeuQuant * neuQuant, int rad, int i, int b, int g, int r) {

	int j, k, lo, hi, a, m;
	int *p;

	lo = i - rad;
	if (lo < -1)
		lo = -1;
	hi = i + rad;
	if (hi > NETSIZE)
		hi = NETSIZE;

	j = i + 1;
	k = i - 1;
	m = 1;
	while ((j < hi) || (k > lo)) {
		a = neuQuant->radpower[m++];
		if (j < hi) {
			p = neuQuant->network[j++];
			p[0] -= (a * (p[0] - b)) / ALPHA_RAD_BIAS;
			p[1] -= (a * (p[1] - g)) / ALPHA_RAD_BIAS;
			p[2] -= (a * (p[2] - r)) / ALPHA_RAD_BIAS;
		}
		if (k > lo) {
			p = neuQuant->network[k--];
			p[0] -= (a * (p[0] - b)) / ALPHA_RAD_BIAS;
			p[1] -= (a * (p[1] - g)) / ALPHA_RAD_BIAS;
			p[2] -= (a * (p[2] - r)) / ALPHA_RAD_BIAS;
		}
	}

	return 0;
}


/*
* Unbias network to give byte values 0..255 and record position i to prepare
* for sort
* -----------------------------------------------------------------------------------
*/
int unbiasnet(NeuQuant * neuQuant) {

	int i;

	for (i = 0; i < NETSIZE; i++) {
		neuQuant->network[i][0] >>= NET_BIAS_SHIFT;
		neuQuant->network[i][1] >>= NET_BIAS_SHIFT;
		neuQuant->network[i][2] >>= NET_BIAS_SHIFT;
		neuQuant->network[i][3] = i; /* record colour no */
	}
	return 0;
}

uint32_t * colorMap(NeuQuant * neuQuant) {
	uint32_t * map = (uint32_t*)malloc(sizeof(uint32_t) * 3 * NETSIZE);
	int * index = (int*)malloc(sizeof(int) * NETSIZE);
	for (int i = 0; i < NETSIZE; i++)
		index[neuQuant->network[i][3]] = i;
	int k = 0;
	for (int i = 0; i < NETSIZE; i++) {
		int j = index[i];
		map[k++] = (uint32_t)(neuQuant->network[j][0]);
		map[k++] = (uint32_t)(neuQuant->network[j][1]);
		map[k++] = (uint32_t)(neuQuant->network[j][2]);
	}
	return map;
}
uint32_t * process(NeuQuant * neuQuant) {
	learn(neuQuant);
	unbiasnet(neuQuant);
	inxbuild(neuQuant);
	return colorMap(neuQuant);
}

/*
* Move neuron i towards biased (b,g,r) by factor alpha
* ----------------------------------------------------
*/
int altersingle(NeuQuant * neuQuant, int alpha, int i, int b, int g, int r) {

	/* alter hit neuron */
	int * n = neuQuant->network[i];
	n[0] -= (alpha * (n[0] - b)) / INIT_ALPHA;
	n[1] -= (alpha * (n[1] - g)) / INIT_ALPHA;
	n[2] -= (alpha * (n[2] - r)) / INIT_ALPHA;

	return 0;
}

/*
* Search for biased BGR values ----------------------------
*/
int contest(NeuQuant * neuQuant, int b, int g, int r) {

	/* finds closest neuron (min dist) and updates freq */
	/* finds best neuron (min dist-bias) and returns position */
	/* for frequently chosen neurons, freq[i] is high and bias[i] is negative */
	/* bias[i] = gamma*((1/netsize)-freq[i]) */

	int i, dist, a, biasdist, betafreq;
	int bestpos, bestbiaspos, bestd, bestbiasd;
	int * n;

	bestd = ~(((int)1) << 31);
	bestbiasd = bestd;
	bestpos = -1;
	bestbiaspos = bestpos;

	for (i = 0; i < NETSIZE; i++) {
		n = neuQuant->network[i];
		dist = n[0] - b;
		if (dist < 0)
			dist = -dist;
		a = n[1] - g;
		if (a < 0)
			a = -a;
		dist += a;
		a = n[2] - r;
		if (a < 0)
			a = -a;
		dist += a;
		if (dist < bestd) {
			bestd = dist;
			bestpos = i;
		}
		biasdist = dist - ((neuQuant->bias[i]) >> (INT_BIAS_SHIFT - NET_BIAS_SHIFT));
		if (biasdist < bestbiasd) {
			bestbiasd = biasdist;
			bestbiaspos = i;
		}
		betafreq = (neuQuant->freq[i] >> BETA_SHIFT);
		neuQuant->freq[i] -= betafreq;
		neuQuant->bias[i] += (betafreq << GAMMA_SHIFT);
	}
	neuQuant->freq[bestpos] += BETA;
	neuQuant->bias[bestpos] -= BETA_GAMMA;
	return (bestbiaspos);
}

int finishNeuQuant(NeuQuant * neuQuant) {
	

}