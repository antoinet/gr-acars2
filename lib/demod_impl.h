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


#ifndef INCLUDED_ACARS2_DEMOD_IMPL_H
#define INCLUDED_ACARS2_DEMOD_IMPL_H

#include <acars2/demod.h>
#include <cstdio>
#include <cstdlib>

#define FREQ_MARK		1200
#define FREQ_SPACE		2400
#define BAUD			2400
#define TRANSITION(X)	((X ^ (X >> 1)) & 1)

// #define ACARS2_DEBUG	1


namespace gr {
	namespace acars2 {
		
		/*!
		 * \brief <+description+>
		 * \ingroup block
		 *
		 */
		class demod_impl : public demod
		{
		 private:
			enum State { PRE_KEY, SYNC, SOH, ETX, BCS };

			uint32_t	sphase;							// current pos in the symbol (normalized to 65536)
			uint32_t	sphase_inc;						// position increment (normalized to 65536)
	
			uint32_t	corrlen;						// correlation length = multiple of # of samples per symbol
			float		*corr_mark_i, *corr_mark_q;		// sine/cosine reference for mark
			float		*corr_space_i, *corr_space_q;	// sine/cosine reference for space

			State		state;
			uint32_t	freq_shreg;						// shift register, holds last mark/space frequencies
			uint32_t	curbit_shreg;					// shift register, holds last 32 decoded bits
			uint32_t	consecutive;
			uint32_t	bit_count;
			uint32_t	acars2_debug;

		 public:
			 demod_impl(int samp_rate, int debug);
			 ~demod_impl();

			// Where all the action really happens
			int general_work (
				int noutput_items,
				gr_vector_int &ninput_items,
			    gr_vector_const_void_star &input_items,
			    gr_vector_void_star &output_items
			);

		};
		
	} // namespace acars2
} // namespace gr

#endif /* INCLUDED_ACARS2_DEMOD_IMPL_H */

