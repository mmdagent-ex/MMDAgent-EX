
TARGET   = lib/JPEG.a

SOURCES  = src/jaricom.c \
           src/jcapimin.c \
           src/jcapistd.c \
           src/jcarith.c \
           src/jccoefct.c \
           src/jccolor.c \
           src/jcdctmgr.c \
           src/jchuff.c \
           src/jcinit.c \
           src/jcmainct.c \
           src/jcmarker.c \
           src/jcmaster.c \
           src/jcomapi.c \
           src/jcparam.c \
           src/jcprepct.c \
           src/jcsample.c \
           src/jctrans.c \
           src/jdapimin.c \
           src/jdapistd.c \
           src/jdarith.c \
           src/jdatadst.c \
           src/jdatasrc.c \
           src/jdcoefct.c \
           src/jdcolor.c \
           src/jddctmgr.c \
           src/jdhuff.c \
           src/jdinput.c \
           src/jdmainct.c \
           src/jdmarker.c \
           src/jdmaster.c \
           src/jdmerge.c \
           src/jdpostct.c \
           src/jdsample.c \
           src/jdtrans.c \
           src/jerror.c \
           src/jfdctflt.c \
           src/jfdctfst.c \
           src/jfdctint.c \
           src/jidctflt.c \
           src/jidctfst.c \
           src/jidctint.c \
           src/jquant1.c \
           src/jquant2.c \
           src/jutils.c \
           src/jmemmgr.c \
           src/jmemnobs.c

OBJECTS  = $(SOURCES:.c=.o)

CXX      = gcc
AR       = ar
CXXFLAGS = -Wall -O3 -fomit-frame-pointer -fPIC \
           -Wno-main
INCLUDE  = -I src

all: $(TARGET)

$(TARGET): src/jconfig.h $(OBJECTS)
	$(AR) cru $(TARGET) $(OBJECTS)
	cp src/*.h include

src/jconfig.h:
	cp src/jconfig.txt src/jconfig.h

.c.o:
	$(CXX) $(CXXFLAGS) $(INCLUDE) -o $(<:.c=.o) -c $<

clean:
	rm -f $(OBJECTS) $(TARGET)
	rm -f src/jconfig.h
