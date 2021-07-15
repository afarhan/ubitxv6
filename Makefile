# uBitxv6 Rhizomatica tools
# Copyright (C) 2019-2021 Rhizomatica
# Author: Rafael Diniz <rafael@riseup.net>
#
# This is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.
#
# This software is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this software; see the file COPYING.  If not, write to
# the Free Software Foundation, Inc., 51 Franklin Street,
# Boston, MA 02110-1301, USA.
#

.PHONY: clean install firmware tools

all: tools firmware

tools:
	$(MAKE) -C tools

firmware:
	$(MAKE) -C firmware

install:
	$(MAKE) -C tools install

ispload:
	$(MAKE) -C firmware ispload

clean:
	$(MAKE) -C tools clean
	$(MAKE) -C firmware clean
