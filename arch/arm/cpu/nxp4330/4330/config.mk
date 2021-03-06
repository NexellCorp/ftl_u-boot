#
# (C) Copyright 2009
# jung hyun kim, Nexell Co, <jhkim@nexell.co.kr>
#
# See file CREDITS for list of people who contributed to this
# project.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston,
# MA 02111-1307 USA
#

# =========================================================================
# Add cpu options
# =========================================================================
PROTOTYPE 	:= prototype
MODULES		:= module
BASEDIR		:= base

ARCH_CFLAGS	+= 	-I$(TOPDIR)/arch/arm/cpu/$(CPU)/$(PROTOTYPE)/$(MODULES)	\
			 	-I$(TOPDIR)/arch/arm/cpu/$(CPU)/$(PROTOTYPE)/$(BASEDIR)	\
			 	-I$(TOPDIR)/arch/arm/include/asm/arch-$(CPU)			\
			 	-I$(TOPDIR)/arch/arm/cpu/$(CPU)/common					\
			 	-I$(TOPDIR)/arch/arm/cpu/$(CPU)/devices					\
			 	-I$(TOPDIR)/board/$(VENDOR)/common

ifeq ($(CONFIG_PROTOTYPE_DEBUG),y)
ARCH_CFLAGS += -D__LINUX__ -DNX_DEBUG
else
ARCH_CFLAGS += -D__LINUX__ -DNX_RELEASE
endif

# =========================================================================
#   EWS FTL Build Option
# =========================================================================
ARCH_CFLAGS += -D__SUPPORT_MIO_UBOOT__

# =========================================================================
#	Build options for HOSTCC
# =========================================================================
PLATFORM_RELFLAGS  += $(ARCH_CFLAGS)

