SOURCES = $(wildcard *.c)
OBJETS = $(SOURCES:.c=.o)
BIBLIOTHEQUES = -L ../../libs/Reseau -lreseau -L ../../libs/SMTP -lSMTP -L ../../libs/Flux -lflux -L ../../libs/Journal -ljournal -lpthread
EXECUTABLE = SMTPout
CFLAGS += -Wall -I ../../libs/Reseau -I ../../libs/SMTP -I ../../libs/Flux -I ../../libs/Journal

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJETS)
	$(CC) -o $@ $^ $(BIBLIOTHEQUES)

clean:
	rm -rf $(EXECUTABLE) *.o
