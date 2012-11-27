#
# Copyright 2008,2009 Free Software Foundation, Inc.
#
# This application is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.
#
# This application is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#


#!/usr/bin/env python

from gnuradio import gr, eng_notation, optfir, audio, uhd, blks2
from gnuradio.eng_option import eng_option
from gnuradio.wxgui import stdgui2, fftsink2, form, slider
from optparse import OptionParser
import wx
import math
import sys

class acars_rx(stdgui2.std_top_block):
	def __init__(self, frame, panel, vbox, argv):
		stdgui2.std_top_block.__init__(self, frame, panel, vbox, argv)
		
		parser=OptionParser(option_class=eng_option)
		parser.add_option("-a", "--args", type="string", default="",
						  help="UHD device address args [default=%default]")
		(options, args) = parser.parse_args()
		
		self.frame = frame
		self.panel = panel
		self.vol = 0
		self.freq = 0
		
		usrp_rate = 256e3
		demod_rate = 64e3
		audio_rate = 48e3
		
		# USRP source
		self.usrp = uhd.usrp_source(device_addr=options.args, stream_args=uhd.stream_args('fc32'))
		self.usrp.set_samp_rate(usrp_rate)
		dev_rate = self.usrp.get_samp_rate()
		rrate = usrp_rate / dev_rate
		self.resamp = blks2.pfb_arb_resampler_ccf(rrate)
		print "Using USRP sampling rate ", dev_rate
		
		# low pass filter
		chan_filt_coeffs = gr.firdes.low_pass_2(1,				# gain
												 usrp_rate,		# sampling rate
												 2e3,			# passband_cutoff
												 1e2,			# transition bw
												 60)			# stopband attenuation
		self.chan_filt = gr.fir_filter_ccf(int(usrp_rate//demod_rate), chan_filt_coeffs)
		
		# AM demodulator
		self.am_demod = gr.complex_to_mag()
		
		# volume control
		self.vol_control = gr.multiply_const_ff(self.vol)
		
		# audio sink
		self.audio_sink = audio.sink(int (audio_rate), "pulse", False)
		
		self.connect(self.usrp, self.resamp, self.chan_filt, self.am_demod, self.vol_control, self.audio_sink)
		self._build_gui(vbox, usrp_rate, demod_rate, audio_rate)
		
		if not(self.set_freq(131725000)):
			print "Failed to set freq"
		
		print "USRP intialized..."
	
	
	
	def _build_gui(self, vbox, usrp_rate, demod_rate, audio_rate):
		
		def _form_set_freq(kv):
			return self.set_freq(kv['freq'])
		
		self.post_filt_fft = fftsink2.fft_sink_c(self.panel, title="Post Channel filter",
			fft_size=512, sample_rate=demod_rate)
		self.connect(self.chan_filt, self.post_filt_fft)
		vbox.Add(self.post_filt_fft.win, 4, wx.EXPAND)
		
		self.myform = myform = form.form()
		hbox = wx.BoxSizer(wx.HORIZONTAL)
		hbox.Add((5, 0), 0)
		myform['freq'] = form.float_field(
			parent=self.panel, sizer=hbox, label="Freq", weight=1,
			callback=myform.check_input_and_call(_form_set_freq))
		
		hbox.Add((5, 0), 0)
		myform['freq_slider'] = form.quantized_slider_field(parent=self.panel,
			sizer=hbox, weight=3, range=(520.0e3, 1611.0e3, 1.0e3), callback=self.set_freq)
	
	
	
	def set_freq(self, target_freq):
		"""
		Set the center frequency we're interested in.
		@param target_freq: frequency in Hz
		@rypte: bool
		"""
		r = self.usrp.set_center_freq(target_freq, 0)
		if r:
			self.freq = target_freq
			self.myform['freq'].set_value(target_freq)				# update displayed value
			self.myform['freq_slider'].set_value(target_freq)
			self.post_filt_fft.set_baseband_freq(target_freq)
			return True
		return False


if __name__ == '__main__':
	app = stdgui2.stdapp(acars_rx, "USRP ACARS RX")
	app.MainLoop()
