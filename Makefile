REPERTOIRES = libs/Journal libs/Reseau libs/Flux libs/SMTP src/SMTPin src/SMTPout
COMPILE = $(REPERTOIRES:%=all-%)
DEVERMINE = $(REPERTOIRES:%=debug-%)
NETTOYAGE = $(REPERTOIRES:%=clean-%)
export CC = gcc
export CFLAGS += -Wall 

all: $(COMPILE)
$(COMPILE):
	$(MAKE) -C $(@:all-%=%)

debug: $(DEVERMINE)
$(DEVERMINE): CFLAGS += -g -DDEVERMINE
$(DEVERMINE):
	$(MAKE) -C $(@:debug-%=%)

clean: $(NETTOYAGE)
$(NETTOYAGE):
	$(MAKE) -C $(@:clean-%=%) clean

all-Main: all-Bibliotheque
debug-Main: debug-Bibliotheque

.PHONY: $(COMPILE) $(DEVERMINE) $(NETTOYAGE)
.PHONY: all debug clean
