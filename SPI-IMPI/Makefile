#CROSS_COMPILE = arm-linux-gnueabihf-
CC := $(CROSS_COMPILE)gcc
CXX := $(CROSS_COMPILE)g++
LD := $(CROSS_COMPILE)ld
AR := $(CROSS_COMPILE)ar

TARGET := demo

CFLAGS += -fPIC

# OBJS := ringbuffer.o spi_dev.o spi_protocol.o demo.o
OBJS := ringbuffer.o spi_dev.o spi_protocol.o

SO : $(OBJS)
	$(CC) $(CFLAGS) -shared $^ -o libspi.so
	$(CC) demo.c -o demo -L./ -lspi

# all : $(OBJS)
# 	$(CC) $^ -I ./ -o demo -lpthread

%o : %c 
	$(CC) -I ./ -c -o $@ $<


install:
	cp -rvf ./*.h ./lib
	cp -rvf ./*.so ./lib

clean:
	rm demo *.o *.so -f

