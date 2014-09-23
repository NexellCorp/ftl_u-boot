
include $(TOPDIR)/config.mk

LIB	:= $(obj)libmio.o

COBJS-y += mio.uboot.o
COBJS-y += mio.uboot.rwtest.o
COBJS-y += media/exchange.o
COBJS-y += media/nfc/phy/nfc.phy.o
COBJS-y += media/nfc/phy/nfc.phy.scan.o
COBJS-y += media/nfc/phy/nfc.phy.lowapi.rawread.o
COBJS-y += media/nfc/phy/nfc.phy.lowapi.o
COBJS-y += media/nfc/phy/nfc.phy.readretry.o
COBJS-y += media/nfc/phy/nfc.phy.rand.o

COBJS	:= $(COBJS-y)
SRCS	:= $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS))

all:	$(LIB)

$(LIB):	$(obj).depend $(OBJS)
	$(call cmd_link_o_target, $(OBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
