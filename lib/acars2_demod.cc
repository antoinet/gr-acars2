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

/*
 * The private constructor
 */
acars2_demod::acars2_demod (int samp_rate)
  : gr_sync_block ("square2_ff",
		   gr_make_io_signature(<+MIN_IN+>, <+MAX_IN+>, sizeof (<+float+>)),
		   gr_make_io_signature(0, 0, 0))
{
	// Put in <+constructor stuff+> here
}


/*
 * Our virtual destructor.
 */
acars2_demod::~acars2_demod()
{
	// Put in <+destructor stuff+> here
}


int
acars2_demod::work(int noutput_items,
		  gr_vector_const_void_star &input_items,
		  gr_vector_void_star &output_items)
{
	const float *in = (const float *) input_items[0];
	float *out = (float *) output_items[0];

	// Do <+signal processing+>

	// Tell runtime system how many output items we produced.
	return noutput_items;
}

