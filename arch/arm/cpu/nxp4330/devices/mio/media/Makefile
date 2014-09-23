
include $(TOPDIR)/config.mk

LIB	:= $(obj)libmioftl.o

COBJS-y += include/include.o
COBJS-y += ftl/ftl.o
COBJS-y += nfc/nfc.sche.o

COBJS	:= $(COBJS-y)
SRCS	:= $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS))

all:	$(LIB)

$(LIB):	$(OBJS)
	$(call cmd_link_o_target, $(OBJS))

include/include.o:
	$(shell cp include/include.o.prebuilt include/include.o)
ftl/ftl.o:
	$(shell cp ftl/ftl.o.prebuilt ftl/ftl.o)
nfc/nfc.sche.o:
	$(shell cp nfc/nfc.sche.o.prebuilt nfc/nfc.sche.o)

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
