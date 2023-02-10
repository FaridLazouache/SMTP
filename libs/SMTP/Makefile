SOURCES = $(wildcard *.c)
OBJETS = $(SOURCES:.c=.o)
BIBLIOTHEQUE = libSMTP.a
AR = ar r
CFLAGS += -Wall -I ../../libs/Reseau

all: $(BIBLIOTHEQUE)

$(BIBLIOTHEQUE): $(OBJETS)
	$(AR) $@ $^

$(OBJETS): smtp.h

clean:
	rm -rf *.o *.a core
