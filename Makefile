# vim: set ts=2 sw=2:
# =====================================================================================
#
#       Filename:  Makefile
#
#    Description:  makefile for pme's small 'iotop' called procio
#
#        Version:  1.0
#        Created:  2011-06-24 14:41
#       Revision:  none
#       Compiler:  gcc
#
#         Author:  Peter Meszaros (pme), hauptadler@gmail.com
#      Copyright:  Copyright (c) 2011
#
# =====================================================================================
# 
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
# 
# =====================================================================================

.PHONY : all clean tags

CFLAGS = -Wall -g

PROGRAMS = procio writer getdio

all: $(PROGRAMS)

clean:
	rm -f $(PROGRAMS) *~ tags

tags:
	ctags -R .
