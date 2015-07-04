PROGRAM := daw
SOURCES := main.cpp BufferManager.cpp
OBJECTS_DIR := objects

-include Makefile.local

######

OBJECTS = $(foreach source,$(SOURCES),$(OBJECTS_DIR)/$(source:.cpp=.o))

all: runnit

CPP := g++
CFLAGS += $(foreach switch,$(SWITCHES),-D$(switch))

$(OBJECTS_DIR)/%.o: %.cpp
	$(CPP) -c $^ -g $(CFLAGS) -o $@

$(PROGRAM): $(OBJECTS_DIR) $(OBJECTS)
	$(CPP) $(filter-out $(OBJECTS_DIR),$^) -g -o $@
	@echo "---------------------------------------------"
	@echo

$(OBJECTS_DIR):
	mkdir $(OBJECTS_DIR)


.PHONY: runnit
runnit: $(PROGRAM)
	@./$(PROGRAM)


.PHONY: clean
clean:
	rm -r $(PROGRAM) $(OBJECTS_DIR)

