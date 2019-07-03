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
#  This makefile is more or less generic.
#  The configuration is on `sources.mk`.
# -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
topdir ?= $(shell readlink -f $(dir $(word 1,$(MAKEFILE_LIST))))
gendir ?= $(shell pwd)

include $(topdir)/make/global.mk

all: $(bindir)/krish

DISTO ?= kora

SRCS-y += $(wildcard $(srcdir)/*.c)
SRCS-y += $(wildcard $(srcdir)/$(DISTO)/*.c)

OBJS-y = $(patsubst $(srcdir)/%.c,$(outdir)/%.o,$(SRCS-y))

CFLAGS += -I $(topdir)/$(DISTO)
CFLAGS +=  -ggdb

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

$(bindir)/krish: $(OBJS-y)
	$(S) mkdir -p $(dir $@)
	$(Q) echo "    LD $@"
	$(V) $(CC) -o $@ $^ $(LFLAGS)

$(outdir)/%.o: $(srcdir)/%.c
	$(S) mkdir -p $(dir $@)
	$(Q) echo "    CC $@"
	$(V) $(CC) -c -o $@ $< $(CFLAGS)


clean:
	$(V) rm -rf $(bindir) $(libdir) $(outdir)


