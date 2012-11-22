/* -*- c++ -*- */

#define ACARS2_API

%include "gnuradio.i"			// the common stuff

//load generated python docstrings
%include "acars2_swig_doc.i"


%{
#include "acars2_demod.h"
#include "acars2_decode.h"
%}


GR_SWIG_BLOCK_MAGIC(acars2,demod);
%include "acars2_demod.h"

GR_SWIG_BLOCK_MAGIC(acars2,decode);
%include "acars2_decode.h"
