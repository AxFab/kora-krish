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
NAME = Krish
VERSION := $(GIT_DESC)

# F L A G S -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
CFLAGS += -Wall -Wextra -Wno-unused-parameter -fPIC -Wno-multichar
CFLAGS += -Iinclude
CFLAGS += -D__GUM_X11

LFLAGS += -L/usr/X11R6/lib -lX11


# C O M P I L E   M O D E -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
std_CFLAGS := $(CFLAGS) -ggdb -fno-builtin
$(eval $(call ccpl,std))


# D E L I V E R I E S -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
krish_src-y += $(wildcard $(srcdir)/*.c)
krish_src-y += $(wildcard $(srcdir)/kdb/US_international.c)
krish_LFLAGS := $(LFLAGS)
$(eval $(call link,krish,std))
DV_UTILS += $(bindir)/krish

