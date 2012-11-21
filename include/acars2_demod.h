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


#ifndef INCLUDED_ACARS2_DEMOD_H
#define INCLUDED_ACARS2_DEMOD_H

#include <acars2_api.h>
#include <gr_sync_block.h>

class acars2_demod;

typedef boost::shared_ptr<acars2_demod> acars2_demod_sptr;

ACARS2_API acars2_demod_sptr acars2_make_demod (int samp_rate=48000);

/*!
 * \brief <+description+>
 * \ingroup block
 *
 */
class ACARS2_API acars2_demod : public gr_sync_block
{
 private:
	friend ACARS2_API acars2_demod_sptr acars2_make_square2_ff (int samp_rate);

  acars2_demod(int samp_rate);

 public:
  ~acars2_demod();

	// Where all the action really happens
	int work (int noutput_items,
	    gr_vector_const_void_star &input_items,
	    gr_vector_void_star &output_items);
};

#endif /* INCLUDED_ACARS2_DEMOD_H */

