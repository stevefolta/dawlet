PROGRAM := daw
SOURCES := main.cpp BufferManager.cpp
OBJECTS_DIR := objects

######

OBJECTS = $(foreach source,$(SOURCES),$(OBJECTS_DIR)/$(source:.cpp=.o))

all: runnit

CPP := g++

$(OBJECTS_DIR)/%.o: %.cpp
	$(CPP) -c $^ -g -o $@

$(PROGRAM): $(OBJECTS_DIR) $(OBJECTS)
	$(CPP) $(filter-out $(OBJECTS_DIR),$^) -g -o $@
	@echo "---------------------------------------------"
	@echo

$(OBJECTS_DIR):
	mkdir $(OBJECTS_DIR)


.PHONY: runnit
runnit: $(PROGRAM)
	@./$(PROGRAM)

