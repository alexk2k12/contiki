CONTIKI_PROJECT = hello-world
all: $(CONTIKI_PROJECT)

TARGET=avr-rcb

ifdef nodeid
CFLAGS += -DNODEID=$(nodeid)
endif

CONTIKI = ../../..
include $(CONTIKI)/Makefile.include
