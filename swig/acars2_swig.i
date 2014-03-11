/* -*- c++ -*- */

#define ACARS2_API

%include "gnuradio.i"			// the common stuff

//load generated python docstrings
%include "acars2_swig_doc.i"


%{
#include "acars2/demod.h"
#include "acars2/decode.h"
%}


%include "acars2/demod.h"
GR_SWIG_BLOCK_MAGIC2(acars2,demod);

%include "acars2/decode.h"
GR_SWIG_BLOCK_MAGIC2(acars2,decode);
