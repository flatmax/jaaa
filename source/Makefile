# ----------------------------------------------------------------------------
#
#  Copyright (C) 2004-2018 Fons Adriaensen <fons@linuxaudio.org>
#    
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# ----------------------------------------------------------------------------


PREFIX ?= /usr/local
BINDIR ?= $(PREFIX)/bin

VERSION = 0.9.2
CPPFLAGS += -MMD -MP -DVERSION=\"$(VERSION)\" -DPREFIX=\"$(PREFIX)\" -I.
CPPFLAGS += $(shell pkgconf --cflags freetype2)
CXXFLAGS += -O2 -Wall
CXXFLAGS += -march=native
LDLIBS += -lzita-alsa-pcmi -lclthreads -lclxclient -lasound -lpthread \
	-lfftw3f -ljack -lXft -lX11 -lrt


JAAA_O = jaaa.o styles.o mainwin.o audio.o rngen.o
jaaa:	$(JAAA_O)
	$(CXX) $(LDFLAGS) -o $@ $(JAAA_O) $(LDLIBS)
$(JAAA_O):
-include $(JAAA_O:%.o=%.d)


install:	jaaa
	install -d $(DESTDIR)$(BINDIR)
	install -m 755 jaaa $(DESTDIR)$(BINDIR)


uninstall:
	rm -f $(DESTDIR)$(BINDIR)/jaaa


clean:
	/bin/rm -f *~ *.o *.a *.d *.so jaaa

