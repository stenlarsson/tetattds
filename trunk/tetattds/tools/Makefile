CC=gcc
CXX=g++
CFLAGS=-Wall -O2
ifneq (,$(findstring CYGWIN,$(shell uname)))
CFLAGS+=-mno-cygwin
endif
CXXFLAGS=$(CFLAGS)

all: pimpmybmp lzgb ima2raw

