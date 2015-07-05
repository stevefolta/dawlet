PROGRAM := daw
SOURCES := main.cpp BufferManager.cpp MessageQueue.cpp AudioEngine.cpp Logger.cpp
OBJECTS_DIR := objects

-include Makefile.local

######

OBJECTS = $(foreach source,$(SOURCES),$(OBJECTS_DIR)/$(source:.cpp=.o))

all: runnit

CFLAGS += -std=c++11 -pthread
LINK_FLAGS += -pthread

CPP := g++
CFLAGS += -MMD
CFLAGS += -g
CFLAGS += $(foreach switch,$(SWITCHES),-D$(switch))

$(OBJECTS_DIR)/%.o: %.cpp
	$(CPP) -c $< -g $(CFLAGS) -o $@

$(PROGRAM): $(OBJECTS_DIR) $(OBJECTS)
	$(CPP) $(filter-out $(OBJECTS_DIR),$^) -g $(LINK_FLAGS) -o $@
	@echo "---------------------------------------------"
	@echo

$(OBJECTS_DIR):
	mkdir $(OBJECTS_DIR)

-include $(OBJECTS_DIR)/*.d


.PHONY: runnit
runnit: $(PROGRAM)
	@./$(PROGRAM)


.PHONY: clean
clean:
	rm -rf $(PROGRAM) $(OBJECTS_DIR)

