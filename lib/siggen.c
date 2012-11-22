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

static const unsigned char bit_reverse_table[256] = 
{
#   define R2(n)     n,     n + 2*64,     n + 1*64,     n + 3*64
#   define R4(n) R2(n), R2(n + 2*16), R2(n + 1*16), R2(n + 3*16)
#   define R6(n) R4(n), R4(n + 2*4 ), R4(n + 1*4 ), R4(n + 3*4 )
    R6(0), R6(2), R6(1), R6(3)
};

static uint8_t bit_reverse (uint8_t b) {
	return bit_reverse_table[b];
}

static float* siggen (char* buf, uint32_t len) {
	uint32_t num_samples = DATA_LEN(len);
	float f, *p_data, *data;
	int i, j, k, mark = 0;
	char curbyte, curbit, *p_buf = buf;
	
	p_data = data = calloc(num_samples, sizeof(float));
	for (i = 0; i < len; i++) {
		curbyte = bit_reverse(*p_buf++);
		for (j = 0; j < 8; j++) {
			curbit = (curbyte >> j) & 1;
			f = mark ? M_PI : 0.0f;
			if (curbit) { // mark
				mark = !mark;
				for (k = 0; k < SYMBOL_LEN; k++) {
					*p_data++ = sin(f);
					f += 2.0f*M_PI*FREQ_MARK/SAMP_RATE;
				}
			} else { // space
				for (k = 0; k < SYMBOL_LEN; k++) {
					*p_data++ = sin(f);
					f += 2.0f*M_PI*FREQ_SPACE/SAMP_RATE;
				}
			}
		}
	}
	return data;
}


int main (int argc, char* argv[]) {
	int i;

	if (argc < 2) {
		fprintf(stderr, "Usage: siggen <data> [<sample_rate>]\n");
		exit(1);
	}

	uint32_t length = strlen(argv[1]);
	float* data = siggen(argv[1], length);

	for (i = 0; i < DATA_LEN(length); i++) {
		printf("%f\n", *data++);
	}
}