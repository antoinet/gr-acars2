#!/usr/bin/env python
##################################################
# Gnuradio Python Flow Graph
# Title: Top Block
# Generated: Thu Nov 22 14:18:45 2012
##################################################

from gnuradio import eng_notation
from gnuradio import gr
from gnuradio.eng_option import eng_option
from gnuradio.gr import firdes
from grc_gnuradio import wxgui as grc_wxgui
from optparse import OptionParser
import acars2
import wx

class top_block(grc_wxgui.top_block_gui):

	def __init__(self):
		grc_wxgui.top_block_gui.__init__(self, title="Top Block")
		_icon_path = "/usr/share/icons/hicolor/32x32/apps/gnuradio-grc.png"
		self.SetIcon(wx.Icon(_icon_path, wx.BITMAP_TYPE_ANY))

		##################################################
		# Variables
		##################################################
		self.samp_rate = samp_rate = 48000

		##################################################
		# Blocks
		##################################################
		self.gr_wavfile_source_0 = gr.wavfile_source("/home/user/cno/gnuradio/gr-acars2/samples/acars06.wav", False)
		self.gr_file_sink_0 = gr.file_sink(gr.sizeof_char*1, "/home/user/cno/gnuradio/gr-acars2/samples/output.txt")
		self.gr_file_sink_0.set_unbuffered(True)
		self.acars2_demod_0 = acars2.demod(samp_rate)

		##################################################
		# Connections
		##################################################
		self.connect((self.gr_wavfile_source_0, 0), (self.acars2_demod_0, 0))
		self.connect((self.acars2_demod_0, 0), (self.gr_file_sink_0, 0))

	def get_samp_rate(self):
		return self.samp_rate

	def set_samp_rate(self, samp_rate):
		self.samp_rate = samp_rate

if __name__ == '__main__':
	parser = OptionParser(option_class=eng_option, usage="%prog: [options]")
	(options, args) = parser.parse_args()
	tb = top_block()
	tb.Run(True)

