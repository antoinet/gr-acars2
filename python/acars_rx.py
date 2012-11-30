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
from gnuradio.wxgui import stdgui2, fftsink2, waterfallsink2, form, slider
from optparse import OptionParser
import acars2
import wx
import math
import sys

class acars_rx(stdgui2.std_top_block):
	def __init__(self, frame, panel, vbox, argv):
		stdgui2.std_top_block.__init__(self, frame, panel, vbox, argv)
		
		parser=OptionParser(option_class=eng_option)
		parser.add_option("-a", "--args", type="string", default="",
						  help="UHD device address args [default=%default]")
		parser.add_option("", "--spec", type="string", default=None,
						  help="Subdevice of UHD device where appropriate")
		parser.add_option("-A", "--antenna", type="string", default=None,
						  help="select Rx Antenna where appropriate")
		parser.add_option("-f", "--freq", type="string", default="131725000",
						  help="Frequency in Hz")
		parser.add_option("-o", "--output", type="string", default="output.txt",
						  help="ACARS log output")
		parser.add_option("-s", "--samp-rate", type="eng_float", default="250e3",
						  help="set sample rate (bandwidth) [default=%default]")
		parser.add_option("-d", "--device", type="string", default="",
						  help="PCM device name, e.g. hw:0,0  /dev/dsp  pulse  etc.")
		parser.add_option("-w", "--wav-file", type="string", default="",
						  help="WAV file output.")
		(options, args) = parser.parse_args()
		if len(args) != 0:
			parser.print_help()
			sys.exit(1)
		
		self.frame = frame
		self.panel = panel
		self.freq = float(options.freq)
		
		usrp_rate = options.samp_rate
		audio_rate = 48e3
		
		# init USRP source block
		self.usrp = uhd.usrp_source(device_addr=options.args, stream_args=uhd.stream_args('fc32'))
		if (options.spec):
			self.usrp.set_subdev_spec(options.spec, 0)
		if (options.antenna):
			self.usrp.set_antenna(options.antenna, 0)
		self.usrp.set_samp_rate(usrp_rate)
		self.usrp.set_center_freq(self.freq, 0)
		
		# FIR block (5KHz low pass filter)
		chan_filt_coeffs = gr.firdes.low_pass_2(8,				# gain
												 usrp_rate,		# sampling rate
												 5e3,			# passband_cutoff
												 1e2,			# transition bw
												 60)			# stopband attenuation
		self.chan_filt = gr.freq_xlating_fir_filter_ccf(1,						# decimation
														 chan_filt_coeffs,		# taps
														 0,						# center freq
														 usrp_rate)				# samp rate
		
		
		# AM demodulator block
		self.am_demod = gr.complex_to_mag()
		
		# resampler block
		self.resamp = blks2.rational_resampler_fff(interpolation=int(audio_rate), decimation=int(usrp_rate), taps=None, fractional_bw=None)
		
		# acars2 demod/decode blocks
		self.acars2_demod = acars2.demod(int(audio_rate))
		self.acars2_decode = acars2.decode()
		
		# file sink
		self.file_sink = gr.file_sink(gr.sizeof_char, options.output)
		
		# build flow graph
		self.connect(self.usrp, self.chan_filt, self.am_demod, self.resamp, self.acars2_demod, self.acars2_decode, self.file_sink)
		
		# add audio sink (optional)
		if (options.device):
			print "using audio device %s" % options.device
			self.audio_sink = audio.sink(int (audio_rate), options.device, True)
			self.connect(self.resamp, self.audio_sink)
		
		# add WAV file sink (optional)
		if (options.wav_file):
			print "writing to WAV file %s" % options.wav_file
			self.wavsink = gr.wavfile_sink(options.wav_file, 1, int(audio_rate), 16)
			self.connect(self.resamp, self.wavsink)
		
		
		self._build_gui(vbox, usrp_rate, audio_rate)
		
		print "built flow graph..."
	
	
	
	
	def _build_gui(self, vbox, usrp_rate, audio_rate):
		
		# Waterfall before channel filter
		self.waterfall = waterfallsink2.waterfall_sink_c(self.panel,
														  title="USRP", 
														  fft_size=512, 
														  sample_rate=usrp_rate,
														  baseband_freq=self.freq)
		self.connect(self.usrp, self.waterfall)
		vbox.Add(self.waterfall.win, 4, wx.EXPAND)
		
		# FFT after channel filter
		self.post_filt_fft = fftsink2.fft_sink_c(self.panel,
												  title="Post Channel filter",
												  fft_size=512,
												  sample_rate=usrp_rate,
												  baseband_freq=self.freq,
												  peak_hold=True)
		self.connect(self.chan_filt, self.post_filt_fft)
		vbox.Add(self.post_filt_fft.win, 4, wx.EXPAND)
		self.post_filt_fft.win.autoscale()



if __name__ == '__main__':
	app = stdgui2.stdapp(acars_rx, "USRP ACARS RX")
	app.MainLoop()
