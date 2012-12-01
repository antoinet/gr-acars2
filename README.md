gr-acars2
=========

[GNU Radio](http://www.gnuradio.org) processing block for [ACARS](http://en.wikipedia.org/wiki/Aircraft_Communications_Addressing_and_Reporting_System) transmissions.


Installing gr-acars2
--------------------

1. Install GNU Radio, follow the instructions on the [GNU Radio wiki](http://gnuradio.org/redmine/projects/gnuradio/wiki/InstallingGR). You can either install the pre-compiled binaries for your distribution, e.g. for Ubuntu: ```$ apt-get install gnuradio```, or you can build from the sources. In the latter case, you can use the fully-automated [build-gnuradio script](http://gnuradio.org/redmine/projects/gnuradio/wiki/InstallingGR#Using-the-build-gnuradio-script).
2. Build and install gr-acars2. Within the gr-acars2 directory, issue the following commands:

		$ mkdir build
		$ cd build
		$ cmake ..
		$ sudo make install
		$ sudo ldconfig

