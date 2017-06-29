# Project Name (executable)
PROJECT = cryptor
# Compiler
CC = g++ 

# Run Options       
COMMANDLINE_OPTIONS = /dev/ttyS0

# Compiler options during compilation
COMPILE_OPTIONS = -pedantic -Wall -g

#Header include directories
HEADERS =
#Libraries for linking
LIBS = -lpthread -lssl -lcrypto -static-libgcc

# Dependency options
DEPENDENCY_OPTIONS = -MM

#-- Do not edit below this line --

# Subdirs to search for additional source files

#SOURCE_FILES := $(foreach d, $(DIRS), $(wildcard $(d)*/*.cpp) )
SOURCE_FILES := $(shell find ./src -type f -name "*.cpp" )

$(info $$SOURCE_FILES is [${SOURCE_FILES}])

# Create an object file of every cpp file
OBJECTS = $(patsubst %.cpp, %.o, $(SOURCE_FILES))

# Dependencies
DEPENDENCIES = $(patsubst %.cpp, %.d, $(SOURCE_FILES))

# Create .d files
%.d: %.cpp
	$(CC) $(DEPENDENCY_OPTIONS) $< -MT "$*.o $*.d" -MF $*.d

# Make $(PROJECT) the default target
all: $(DEPENDENCIES) $(PROJECT)
	cp $(PROJECT) "./OUT/"

$(PROJECT): $(OBJECTS)
	$(CC) -o $(PROJECT) $(OBJECTS) $(LIBS)

# Include dependencies (if there are any)
ifneq "$(strip $(DEPENDENCIES))" ""
-include $(DEPENDENCIES)
endif

# Compile every cpp file to an object
%.o: %.cpp
	$(CC) -c $(COMPILE_OPTIONS) -o $@ $< $(HEADERS)

# Build & Run Project
run: $(PROJECT)
	./$(PROJECT) $(COMMANDLINE_OPTIONS)

# Clean & Debug
.PHONY: makefile-debug
	makefile-debug:

.PHONY: clean
clean:
	rm -f $(PROJECT) $(OBJECTS)
	find ./src -type f -name "*.d" -exec rm {} \;
	find ./bin/Debug/ -mindepth 1 -exec rm -r {} \; 2> /dev/null

.PHONY: depclean
depclean:
	rm -f $(DEPENDENCIES)

clean-all: clean depclean
