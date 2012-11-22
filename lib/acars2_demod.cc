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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gr_io_signature.h>
#include "acars2_demod.h"


acars2_demod_sptr
acars2_make_demod (int samp_rate)
{
	return gnuradio::get_initial_sptr (new acars2_demod(samp_rate));
}

// multiply-accumulate
static inline float
mac (const float *a, const float *b, uint32_t size) {
	float sum = 0;
	uint32_t i;
	
	for (i = 0; i < size; i++) {
		sum += (*a++) * (*b++);
	}
	return sum;
}

// inline square function for floats
static inline float
fsqr (float f) {
	return f*f;
}

/*
 * The private constructor
 */
acars2_demod::acars2_demod (int samp_rate)
  : gr_block ("acars2_demod_fb",
		   gr_make_io_signature(1, 1, sizeof(float)),
		   gr_make_io_signature(1, 1, sizeof(uint8_t))),
	freq_shreg(0), curbit_shreg(0), bit_count(0),
	consecutive(0), sphase(0), state(PRE_KEY)
{
	float f;
	int i;
	
	sphase_inc = 0x10000u*BAUD/samp_rate;

	corrlen = 2*samp_rate/BAUD;
	corr_mark_i = (float*) calloc(corrlen, sizeof(float));
	corr_mark_q = (float*) calloc(corrlen, sizeof(float));
	corr_space_i = (float*) calloc(corrlen, sizeof(float));
	corr_space_q = (float*) calloc(corrlen, sizeof(float));

	for (f = 0.0f, i = 0; i < corrlen; i++) {
		corr_mark_i[i] = cos(f);
		corr_mark_q[i] = sin(f);
		f += 2.0f*M_PI*FREQ_MARK/samp_rate;
	}
	for (f = 0.0f, i = 0; i < corrlen; i++) {
		corr_space_i[i] = cos(f);
		corr_space_q[i] = sin(f);
		f += 2.0f*M_PI*FREQ_SPACE/samp_rate;
	}

	// what is this, and how will it help?
	/*
	for (i = 0; i < corrlen; i++) {
		f = 0.54 - 0.46*cos(2*M_PI*i/(float)(corrlen-1));
		corr_mark_i[i] *= f;
		corr_mark_q[i] *= f;
		corr_space_i[i] *= f;
		corr_space_q[i] *= f;
	}
	*/

	printf(" ACARS demod\n"
		"samp_rate:\t%d\n"
		"baud_rate:\t%d\n"
		"sphase_inc:\t%d\n",
		samp_rate,
		BAUD,
		sphase_inc);

	set_fixed_rate(false);
	set_relative_rate(1.0/corrlen/8);
}


/*
 * Our virtual destructor.
 */
acars2_demod::~acars2_demod()
{
	free(corr_mark_i);
	free(corr_mark_q);
	free(corr_space_i);
	free(corr_space_q);
}


int
acars2_demod::general_work(
	int noutput_items,
	gr_vector_int &ninput_items,
	gr_vector_const_void_star &input_items,
	gr_vector_void_star &output_items)
{
	uint32_t nin = ninput_items[0];
	uint32_t nout = 0;
	const float *in = (const float *) input_items[0];
	char *out = (char *) output_items[0];

	float f, f_mark, f_space;

	for (; nin > 0; nin--, in++) {
		f_mark = fsqr(mac(in, corr_mark_i, corrlen)) + fsqr(mac(in, corr_mark_q, corrlen));
		f_space = fsqr(mac(in, corr_space_i, corrlen)) + fsqr(mac(in, corr_space_q, corrlen));
		f = f_mark - f_space;
		
		freq_shreg <<= 1;
		freq_shreg |= (f > 0);
		
		// adapt window on transition
		if (TRANSITION(freq_shreg)) {
			if (sphase < (0x8000u - (sphase_inc/2)))
				sphase += sphase_inc/8;
			else
				sphase -= sphase_inc/8;
		}
		sphase += sphase_inc;

		if (sphase >= 0x10000u) {
			// past symbol boundary, feed a bit
			sphase &= 0xffffu;
			curbit_shreg <<= 1;
			curbit_shreg |= (curbit_shreg ^ freq_shreg) & 1;

			switch (state) {
			case PRE_KEY:
				// scan for pre-key, expect >= 128bits of 1s
				if (TRANSITION(curbit_shreg)) {
					if (consecutive >= 128) {
						if (!(curbit_shreg & 1)) {
							// wrong bit interpretation, flip bits
							curbit_shreg = ~curbit_shreg;
						}
						
						// start with .....110
						bit_count = 3;
						state = SYNC;
					}
					consecutive = 0;
				} else {
					consecutive++;
				}
				break;
			case SYNC:
				if (bit_count < 32) {
					 // feed in more bits
					if (TRANSITION(curbit_shreg)) {
						consecutive = 0;
					} else {
						consecutive++;
					}
					break;
				}
				// match sync chars: '+' (0x2b), '*' (0x2a), <SYN> (0x16), <SYN> (0x16)
				if (curbit_shreg == 0xffffffff) ;
				break;
			case SOH:
				
				break;
			case BCS:
			
				break;
			}

			*out++ = '0' + (curbit_shreg & 1);
			nout++;
		}
		
	}
	// Tell runtime system how many output items we produced.
	consume_each(ninput_items[0]);
	return nout;
}

