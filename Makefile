#      This file is part of the SmokeOS project.
#  Copyright (C) 2015  <Fabien Bavent>
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU Affero General Public License as
#  published by the Free Software Foundation, either version 3 of the
#  License, or (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU Affero General Public License for more details.
#
#  You should have received a copy of the GNU Affero General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>.
# -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
topdir ?= $(shell readlink -f $(dir $(word 1,$(MAKEFILE_LIST))))
gendir ?= $(shell pwd)

PACKAGE=krish
include $(topdir)/make/global.mk
include $(topdir)/make/build.mk


disto ?= kora



SRCS += $(wildcard $(srcdir)/*.c)
SRCS += $(wildcard $(srcdir)/$(disto)/*.c)

CFLAGS ?= -Wall -Wextra -ggdb
CFLAGS += -I $(topdir)/$(disto) $(shell $(PKC) --cflags lgfx)

LFLAGS += $(shell $(PKC) --libs lgfx)


ifeq ($(disto),linux)
CFLAGS += $(shell $(PKC) --cflags pthread)
LFLAGS += $(shell $(PKC) --libs pthread)
else ifeq ($(disto),kora)
CFLAGS += -Dmain=_main -D_GNU_SOURCE
endif

$(eval $(call link_bin,krish,SRCS,LFLAGS))



include $(topdir)/make/check.mk
include $(topdir)/make/targets.mk

install-headers:

ifeq ($(NODEPS),)
-include $(call fn_deps,SRCS)
endif
