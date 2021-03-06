PROGRAM := dawlet
SOURCES := main.cpp DAW.cpp
SOURCES += ProjectReader.cpp JSONParser.cpp FieldParser.cpp
SOURCES += Logger.cpp Base64.cpp Stats.cpp
SOURCES += Project.cpp Track.cpp Playlist.cpp Clip.cpp Send.cpp
SOURCES += AudioEngine.cpp AudioBuffer.cpp BufferManager.cpp MessageQueue.cpp
SOURCES += AudioFileRead.cpp Recorder.cpp RecordBuffers.cpp StartRecordingProcess.cpp
SOURCES += web/Server.cpp web/Connection.cpp web/Request.cpp
SOURCES += APIHandler.cpp APIHandlers.cpp
SOURCES += Process.cpp
SOURCES += SeekProcess.cpp SupplyReadsProcess.cpp InstallProjectProcess.cpp
SOURCES += GetProcess.cpp GetPBHeadProcess.cpp GetStatsProcess.cpp
SOURCES += SendMeteringProcess.cpp
SOURCES += SelectInterfaceProcess.cpp
SOURCES += MutatingProcess.cpp SetTrackStateProcesses.cpp
SOURCES += NewTrackProcess.cpp DeleteTrackProcess.cpp RestoreTrackProcess.cpp
SOURCES += AudioFile.cpp OpenAudioFile.cpp WAVFile.cpp RIFF.cpp SampleConversion.cpp
SOURCES += AudioSystem.cpp AudioInterface.cpp
SOURCES += ALSAAudioSystem.cpp ALSAAudioInterface.cpp
SOURCES += IndentedOStream.cpp Date.cpp
SOURCES += third-party/sha1-0.2/sha1.cpp
SOURCES += tests/Tests.cpp
OBJECTS_DIR := objects
SUBDIRS := web tests third-party/sha1-0.2
THIRD_PARTY_INCLUDES := third-party/sha1-0.2

-include Makefile.local

ifdef TEST_JSON_PARSER
	SOURCES += tests/JSONParser.cpp
	SWITCHES += TEST_JSON_PARSER
endif
ifdef TEST_READ_PROJECT
	SOURCES += tests/ReadProject.cpp
	SWITCHES += TEST_READ_PROJECT
endif
ifdef TEST_SMALL_TESTS
	SOURCES += tests/SmallTests.cpp
	SWITCHES += TEST_SMALL_TESTS
endif

CFLAGS += -Wall -Wno-reorder

######

OBJECTS = $(foreach source,$(SOURCES),$(OBJECTS_DIR)/$(source:.cpp=.o))
OBJECTS_SUBDIRS = $(foreach dir,$(SUBDIRS),$(OBJECTS_DIR)/$(dir))

ifndef VERBOSE_MAKE
	QUIET := @
endif

all: runnit

CFLAGS += -std=c++11 -pthread
LINK_FLAGS += -pthread -lrt
LINK_FLAGS += -lasound

CPP := g++
CFLAGS += -I. -Isrc/
CFLAGS += $(foreach dir,$(THIRD_PARTY_INCLUDES),-Isrc/$(dir))
CFLAGS += -MMD
CFLAGS += -g
CFLAGS += $(foreach switch,$(SWITCHES),-D$(switch))

$(OBJECTS_DIR)/%.o: src/%.cpp
	@echo Compiling $<...
	$(QUIET) $(CPP) -c $< -g $(CFLAGS) -o $@

$(PROGRAM): $(OBJECTS_DIR) $(OBJECTS)
	@echo "Linking $@..."
	$(QUIET) $(CPP) $(filter-out $(OBJECTS_DIR),$^) -g $(LINK_FLAGS) -o $@
	@echo "---------------------------------------------"
	@echo

$(OBJECTS_DIR):
	@echo "Making $@..."
	$(QUIET) mkdir -p $(OBJECTS_DIR) $(OBJECTS_SUBDIRS)

-include $(OBJECTS_DIR)/*.d


.PHONY: runnit
runnit: $(PROGRAM)
	@./$(PROGRAM) $(RUN_ARGS)


.PHONY: clean
clean:
	rm -rf $(PROGRAM) $(OBJECTS_DIR)

.PHONY: tags
tags:
	ctags -R .

