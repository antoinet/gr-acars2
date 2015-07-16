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

#include <gnuradio/io_signature.h>
#include "demod_impl.h"

namespace gr {
	namespace acars2 {

		static const unsigned char bit_reverse_table[256] = 
		{
		#   define R2(n)     n,     n + 2*64,     n + 1*64,     n + 3*64
		#   define R4(n) R2(n), R2(n + 2*16), R2(n + 1*16), R2(n + 3*16)
		#   define R6(n) R4(n), R4(n + 2*4 ), R4(n + 1*4 ), R4(n + 3*4 )
		    R6(0), R6(2), R6(1), R6(3)
		};

		static const unsigned char parity_table[256] = 
		{
		#   define P2(n) n, n^1, n^1, n
		#   define P4(n) P2(n), P2(n^1), P2(n^1), P2(n)
		#   define P6(n) P4(n), P4(n^1), P4(n^1), P4(n)
		    P6(0), P6(1), P6(1), P6(0)
		};

		static uint8_t bit_reverse (uint8_t b) {
			return bit_reverse_table[b];
		}

		static uint8_t parity (uint8_t b) {
			return parity_table[b];
		}

		demod::sptr
		demod::make (int samp_rate, int debug)
		{
			return gnuradio::get_initial_sptr (new demod_impl(samp_rate, debug));
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
		demod_impl::demod_impl (int samp_rate, int debug)
		  : gr::block ("acars2_demod_fb",
				   gr::io_signature::make(1, 1, sizeof(float)),
				   gr::io_signature::make(1, 1, sizeof(uint8_t))),
			freq_shreg(0), curbit_shreg(0), bit_count(0), acars2_debug(0),
			consecutive(0), sphase(0), state(PRE_KEY)
		{
			float f;
			int i;
	
			sphase_inc = 0x10000u*BAUD/samp_rate;
			acars2_debug = debug;

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

			if (acars2_debug)
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
		demod_impl::~demod_impl()
		{
			free(corr_mark_i);
			free(corr_mark_q);
			free(corr_space_i);
			free(corr_space_q);
		}


		int
		demod_impl::general_work(
			int noutput_items,
			gr_vector_int &ninput_items,
			gr_vector_const_void_star &input_items,
			gr_vector_void_star &output_items)
		{
			uint32_t i;
			uint32_t nin = ninput_items[0];
			uint32_t nout = 0;
			const float *in = (const float *) input_items[0];
			char *out = (char *) output_items[0];

			float f, f_mark, f_space;

			for (i = 0; i < nin; i++) {
				f_mark = fsqr(mac(in, corr_mark_i, corrlen)) + fsqr(mac(in, corr_mark_q, corrlen));
				f_space = fsqr(mac(in, corr_space_i, corrlen)) + fsqr(mac(in, corr_space_q, corrlen));
				f = f_mark - f_space;
				in++;
		
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
					curbit_shreg |= ((curbit_shreg >> 1) ^ freq_shreg) & 1;
					bit_count++;

					
					if (acars2_debug && state != PRE_KEY) {
						printf("%c", '0' + (curbit_shreg & 1));
						if ((bit_count % 8) == 0) {
							printf("\n");
						}
					}

					switch (state) {

					// scan for pre-key, expect >= 128bits of 1s
					case PRE_KEY:
						if (TRANSITION(curbit_shreg)) {
							if (consecutive >= 128) {
								if (curbit_shreg & 1) {
									// wrong bit interpretation, flip bits
									curbit_shreg = ~curbit_shreg;
								}
						
								// start with ...11111|110
								bit_count = 3;
								state = SYNC;
								if (acars2_debug)
									printf("\nSYNC\n110");
							}
							consecutive = 0;
						} else {
							consecutive++;
						}
						break;

					// match sync chars: '+' (0x2b), '*' (0x2a), <SYN> (0x16), <SYN> (0x16)
					case SYNC:
						if (bit_count < 32) {
							 // feed in more bits, consider consecutive bit count if sync chars not found
							if (TRANSITION(curbit_shreg)) {
								consecutive = 0;
							} else {
								consecutive++;
							}
							break;
						}

						// note the reversed bit order and (odd) parity bit
						if (curbit_shreg == 0xd5546868) {
							bit_count = 0;
							state = SOH;
							if (acars2_debug)
								printf("\nSOH\n");
							break;
						}

						// fail
						state = PRE_KEY;
						break;
			
					// match <SOH> (0x01)
					case SOH:
						if (bit_count < 8) break; // feed in more bits
						if ((curbit_shreg & 0xff) == 0x80) { // note the reversed bit ordering

							// output all headers
							*out++ = '+';
							*out++ = '*';
							*out++ = 0x16;
							*out++ = 0x16;
							*out++ = 0x01;
							nout += 5;
							bit_count = 0;
							state = ETX;

							if (acars2_debug)
								printf("\nETX\n");
							break;
						}

						// fail
						state = PRE_KEY;
						break;

					// output bytes and match <ETX> (0x03) or <ETB> (0x17)
					case ETX:
						if (bit_count < 8) break; // feed in more bits

						// output byte value
						*out++ = bit_reverse(curbit_shreg) & 0x7f; // chop off parity bit
						nout++;

						if (((curbit_shreg & 0xff) == 0xc1) || ((curbit_shreg & 0xff) == 0xe9)) {
							bit_count = 0;
							state = BCS;

							if (acars2_debug)
								printf("\nBCS\n");
							break;
						}
						bit_count = 0;
						break;

					// read in 16bit CRC and <DEL> suffix
					case BCS:
						if (bit_count < 24) break; // feed in more bits
						// TODO process CRC and <DEL>
						bit_count = 0;
						state = PRE_KEY;
						if (acars2_debug){
							printf("\n---- END OF TRANSMISSION ----\n");
							printf("PRE_KEY\n");
						}
						break;
					}
				}
		
			}
			// Tell runtime system how many output items we produced.
			consume_each(ninput_items[0]);
			return nout;
		}

	} // namespace acars2
} // namespace gr

