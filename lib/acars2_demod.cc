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
		   gr_make_io_signature(1, 1, sizeof(uint8_t)))
{
	float f;
	int i;

	shreg = 0;
	curbit = 0;

	sphase = 0;
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
		"sphase:\t\t%d\n"
		"sphase_inc:\t%d\n",
		samp_rate,
		BAUD,
		sphase,
		sphase_inc);

	set_fixed_rate(false);
	set_relative_rate(1.0f/corrlen);
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
		
		shreg <<= 1;
		shreg |= (f > 0);
		
		// check if transition (0->1 or 1->0)
		if ((shreg ^ (shreg >> 1)) & 1) {
			if (sphase < (0x8000u - (sphase_inc/2)))
				sphase += sphase_inc/8;
			else
				sphase -= sphase_inc/8;
		}
		sphase += sphase_inc;

		// check if past a symbol
		if (sphase >= 0x10000u) {
			sphase &= 0xffffu;
			curbit = (curbit ^ shreg) & 1;
			*out++ = '0' + curbit;
			printf("%c", '0' + curbit);
			nout++;
		}
		
	}
	// Tell runtime system how many output items we produced.
	consume_each(ninput_items[0]);
	return nout;
}

