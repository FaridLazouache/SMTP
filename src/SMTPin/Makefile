SOURCES = $(wildcard *.c)
OBJETS = $(SOURCES:.c=.o)
BIBLIOTHEQUES = -L ../../libs/Reseau -lreseau -L ../../libs/Flux -lflux -L ../../libs/SMTP -L ../../libs/Journal -lSMTP -lpthread -ljournal -lresolv
EXECUTABLE = SMTPin
CFLAGS += -Wall -I ../../libs/Reseau -I ../../libs/Flux -I ../../libs/SMTP -I ../../libs/Journal

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJETS)
	$(CC) -o $@ $^ $(BIBLIOTHEQUES)

clean:
	rm -rf $(EXECUTABLE) *.o
