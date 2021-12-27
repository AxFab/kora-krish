#      This file is part of the KoraOS project.
#  Copyright (C) 2015-2021  <Fabien Bavent>
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

install: $(prefix)/bin/krish

include $(topdir)/make/build.mk
include $(topdir)/make/check.mk
include $(topdir)/make/targets.mk

CFLAGS ?= -Wall -Wextra -ggdb

# -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

disto ?= kora

SRCS_sh += $(wildcard $(srcdir)/*.c)
SRCS_sh += $(wildcard $(srcdir)/$(disto)/*.c)

CFLAGS_sh += $(CFLAGS)
CFLAGS_sh += -I$(topdir)/$(disto)

ifneq ($(sysdir),)
CFLAGS_sh += -I$(sysdir)/include
LFLAGS_sh += -L$(sysdir)/lib
LFLAGS_sh += -Wl,-rpath-link,$(sysdir)/lib
endif

LFLAGS_sh += -lgfx


ifeq ($(disto),linux)
LFLAGS_sh += -lpthread
endif

$(eval $(call comp_source,sh,CFLAGS_sh))
$(eval $(call link_bin,krish,SRCS_sh,LFLAGS_sh,sh))

# -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

ifeq ($(NODEPS),)
-include $(call fn_deps,SRCS_sh,sh)
endif
