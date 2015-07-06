PROGRAM := daw
SOURCES := main.cpp JSONParser.cpp Logger.cpp tests/Tests.cpp
SOURCES += Project.cpp Track.cpp Playlist.cpp Send.cpp
SOURCES += AudioEngine.cpp AudioBuffer.cpp BufferManager.cpp MessageQueue.cpp
OBJECTS_DIR := objects

-include Makefile.local

ifdef TEST_JSON_PARSER
	SOURCES += tests/JSONParser.cpp
	SWITCHES += TEST_JSON_PARSER
endif
ifdef TEST_READ_PROJECT
	SOURCES += tests/ReadProject.cpp
	SWITCHES += TEST_READ_PROJECT
endif

######

OBJECTS = $(foreach source,$(SOURCES),$(OBJECTS_DIR)/$(source:.cpp=.o))

ifndef VERBOSE_MAKE
	QUIET := @
endif

all: runnit

CFLAGS += -std=c++11 -pthread
LINK_FLAGS += -pthread

CPP := g++
CFLAGS += -MMD
CFLAGS += -g
CFLAGS += $(foreach switch,$(SWITCHES),-D$(switch))

$(OBJECTS_DIR)/%.o: %.cpp
	@echo Compiling $<...
	$(QUIET) $(CPP) -c $< -g $(CFLAGS) -o $@

$(PROGRAM): $(OBJECTS_DIR) $(OBJECTS)
	@echo "Linking $@..."
	$(QUIET) $(CPP) $(filter-out $(OBJECTS_DIR),$^) -g $(LINK_FLAGS) -o $@
	@echo "---------------------------------------------"
	@echo

$(OBJECTS_DIR):
	@echo "Making $@..."
	$(QUIET) mkdir $(OBJECTS_DIR)
	$(QUIET) mkdir -p $(OBJECTS_DIR)/tests

-include $(OBJECTS_DIR)/*.d


.PHONY: runnit
runnit: $(PROGRAM)
	@./$(PROGRAM)


.PHONY: clean
clean:
	rm -rf $(PROGRAM) $(OBJECTS_DIR)

