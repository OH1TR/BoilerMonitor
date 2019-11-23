##
## This file is part of the libopencm3 project.
##
## Copyright (C) 2009 Uwe Hermann <uwe@hermann-uwe.de>
##
## This library is free software: you can redistribute it and/or modify
## it under the terms of the GNU Lesser General Public License as published by
## the Free Software Foundation, either version 3 of the License, or
## (at your option) any later version.
##
## This library is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU Lesser General Public License for more details.
##
## You should have received a copy of the GNU Lesser General Public License
## along with this library.  If not, see <http://www.gnu.org/licenses/>.
##

BINARY = main

LDSCRIPT = stm32f100.ld

OBJS = serial.o usb.o console.o enc28j60.o ip_arp_udp_tcp.o ds18x20.o crc8.o onewire.o max6674.o webserver.o delay.o lcd.o flash.o

include Makefile.include


install:	images
	../stm32flash_new/stm32flash -R /dev/ttyACM1 -w main.hex -g 0 /dev/ttyACM0

