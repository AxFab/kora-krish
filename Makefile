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

include $(topdir)/make/global.mk

all: krish

DISTO ?= kora

include $(topdir)/make/build.mk

SRCS-y += $(wildcard $(srcdir)/*.c)
SRCS-y += $(wildcard $(srcdir)/$(DISTO)/*.c)

CFLAGS ?= -Wall -Wextra -ggdb
CFLAGS += -I $(topdir)/$(DISTO) -D_GNU_SOURCE

LFLAGS += -L $(libdir) -lgfx

ifneq ($(LGFX_HOME),)
LFLAGS += -L $(LGFX_HOME)/lib
CFLAGS += -I $(LGFX_HOME)/include
endif

ifeq ($(DISTO),linux)
LFLAGS += -lpthread
else ifeq ($(DISTO),kora)
CFLAGS += -Dmain=_main
endif

$(eval $(call link_bin,krish,SRCS,LFLAGS))

include $(topdir)/make/check.mk

install: $(call fn_inst,$(BINS) $(LIBS))

ifeq ($(NODEPS),)
-include $(call fn_deps,SRCS-y)
endif
