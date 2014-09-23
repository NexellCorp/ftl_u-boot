
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

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
