/* -*- c++ -*- */
/* 
 * Copyright 2012 <+YOU OR YOUR COMPANY+>.
 * 
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 * 
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define BAUD			2400
#define FREQ_MARK		1200
#define FREQ_SPACE		2400
#define SAMP_RATE		48000
#define SYMBOL_LEN		(SAMP_RATE/BAUD)
#define DATA_LEN(X)		(X*SYMBOL_LEN*8)
#define NRAND_N			20

static const unsigned char bit_reverse_table[256] = 
{
#   define R2(n)     n,     n + 2*64,     n + 1*64,     n + 3*64
#   define R4(n) R2(n), R2(n + 2*16), R2(n + 1*16), R2(n + 3*16)
#   define R6(n) R4(n), R4(n + 2*4 ), R4(n + 1*4 ), R4(n + 3*4 )
    R6(0), R6(2), R6(1), R6(3)
};

typedef struct {
	float* data;
	int len;
} floatbuf_t;

static uint8_t bit_reverse (uint8_t b) {
	return bit_reverse_table[b];
}

static float nrand (float mean, float variance) {
	int i;
	float res = 0.0f;

	for (i = 0; i < NRAND_N; i++) {
		res += rand()/(RAND_MAX + 1.0);
	}
	
	return mean + sqrtf(variance)*res/NRAND_N;
}

static floatbuf_t siggen (char* buf, uint32_t len) {
	uint32_t num_samples = DATA_LEN(len);
	float f, *p_data, *data;
	int i, j, k, preamble, epilogue, mark = 0;
	char curbyte, curbit, *p_buf = buf;
	floatbuf_t res;
	
	preamble = (int)nrand(1.0f, 0.1f)*SAMP_RATE;
	epilogue = (int)nrand(1.0f, 0.1f)*SAMP_RATE;
	
	p_data = data = calloc(preamble + num_samples + epilogue, sizeof(float));
	
	// preamble
	for (i = 0; i < preamble; i++) {
		*p_data++ = nrand(0.0f, 0.4f);
	}
	
	// data
	for (i = 0; i < len; i++) {
		curbyte = bit_reverse(*p_buf++);
		for (j = 0; j < 8; j++) {
			curbit = (curbyte >> j) & 1;
			f = mark ? M_PI : 0.0f;
			if (curbit) { // mark
				mark = !mark;
				for (k = 0; k < SYMBOL_LEN; k++) {
					*p_data++ = 0.1f + 0.6f*sin(f);
					f += 2.0f*M_PI*FREQ_MARK/SAMP_RATE;
				}
			} else { // space
				for (k = 0; k < SYMBOL_LEN; k++) {
					*p_data++ = 0.1f + 0.6f*sin(f);
					f += 2.0f*M_PI*FREQ_SPACE/SAMP_RATE;
				}
			}
		}
	}
	
	// epilogue
	for (i = 0; i < epilogue; i++) {
	  *p_data++ = nrand(0.0f, 0.4f);
	}
	
	res.data = data;
	res.len = preamble + num_samples + epilogue;
	return res;
}


int main (int argc, char* argv[]) {
	int i;

	if (argc < 2) {
		fprintf(stderr, "Usage: siggen <data> [<sample_rate>]\n");
		exit(1);
	}

	uint32_t length = strlen(argv[1]);
	floatbuf_t buf = siggen(argv[1], length);

	for (i = 0; i < buf.len; i++) {
		printf("%f\n", *(buf.data)++);
	}
	return 0;
}