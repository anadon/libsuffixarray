# Makefile for suftest

# sources
OBJS					= archon.o direct.o suftest.o
TARGET				= suftest
MAKEFILE			= Makefile

# options
#CC						= cc
#OUTPUT_OPTION	= -o $@
CFLAGS				= -O3 -fomit-frame-pointer
CXXFLAGS			= -O3 -fomit-frame-pointer
ARCHONOPTIONS	= -DUSE_FWAN -DUSE_BKAN -DUSE_IT2 -DREVERSED # -DVERIFY -DVERBOSE -DHAVE_MALLOC_H
CPPFLAGS			= -Wall -DNDEBUG $(ARCHONOPTIONS)
LDFLAGS				=
LDLIBS				=
#TARGET_ARCH		=


# targets
.PHONY: all
all: $(TARGET)
$(TARGET): $(OBJS)

distclean: clean
clean:
	$(RM) $(TARGET) $(OBJS)

# dependencies
$(OBJS): *.h $(MAKEFILE)
