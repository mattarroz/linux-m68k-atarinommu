##################################################
#
# Makefile for 68000 core based cpus
#
# 2012.10.21, Luis Alves <ljalvs@gmail.com>
#             Merged all 68000 based cpu's config
#             files into a single directory.
#

# 68328, 68EZ328, 68VZ328

ifndef CONFIG_ATARI
obj-y			+= entry.o ints.o timers.o
endif
obj-$(CONFIG_M68328)	+= m68328.o
obj-$(CONFIG_M68EZ328)	+= m68EZ328.o
obj-$(CONFIG_M68VZ328)	+= m68VZ328.o
obj-$(CONFIG_ROM)	+= romvec.o

ifndef CONFIG_ATARI
extra-y 		:= head.o
endif
