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
#include "acars2_decode.h"


static const char *non_printables[] = {
	"<NUL>", "<SOH>", "<STX>", "<ETX>", "<EOT>", "<ENQ>", "<ACK>", "<BEL>",
	"<BS>",  "<TAB>", "<LF>",  "<VT>",  "<FF>",  "<CR>",  "<SO>",  "<SI>",
	"<DLE>", "<DC1>", "<DC2>", "<DC3>", "<DC4>", "<NAK>", "<SYN>", "<ETB>",
	"<CAN>", "<EM>",  "<SUB>", "<ESC>", "<FS>",  "<GS>",  "<RS>",  "<US>"
};

static const uint32_t
_strncpy (char** dest, const char *src, size_t n)
{
	size_t i;
	for (i = 0; i < n && *src != '\0'; i++) {
		*(*dest)++ = *src++;
	}
	return i;
}

acars2_decode_sptr
acars2_make_decode ()
{
	return gnuradio::get_initial_sptr (new acars2_decode());
}

/*
 * The private constructor
 */
acars2_decode::acars2_decode ()
  : gr_block ("acars2_decode",
		   gr_make_io_signature(1, 1, sizeof (uint8_t)),
		   gr_make_io_signature(1, 1, sizeof (uint8_t)))
{
	// Put in <+constructor stuff+> here
	set_fixed_rate(false);
	set_relative_rate(4.0); // worst case if all characters are non-printables
}


/*
 * Our virtual destructor.
 */
acars2_decode::~acars2_decode()
{
	// Put in <+destructor stuff+> here
}


int
acars2_decode::general_work (int noutput_items,
				   gr_vector_int &ninput_items,
				   gr_vector_const_void_star &input_items,
				   gr_vector_void_star &output_items)
{
	const char *in = (const char *) input_items[0];
	char *out = (char *) output_items[0];
	uint32_t i, j;
	uint32_t nout = 0;
	uint32_t nin = ninput_items[0];
	char c;

	// Do <+signal processing+>
	for (i = 0; i < nin; i++) {
		c = *in++;
		if (c < 0x20) {
			nout += _strncpy(&out, non_printables[c], 5);
		} else {
			*out++ = c;
			nout++;
		}
	}

	// Tell runtime system how many input items we consumed on
	// each input stream.
	consume_each (nin);

	// Tell runtime system how many output items we produced.
	return nout;
}

