BIN?=bin

LIBS+=-lzip
CFLAGS+=-Werror -Wall -g
CFLAGS+=-O3
LDFLAGS += $(LIBS)

.PHONY: clean all

all: $(BIN)/jaremap

SRCS:=$(wildcard src/*.cpp)
$(BIN)/jaremap: $(SRCS:%=$(BIN)/%.o)
	$(CXX) $^ -o $@ $(LDFLAGS)

#------------------------------------------------------------------------------
# Generic rules

$(BIN)/%.cpp.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) -c -o "$@" $(CFLAGS) $<
	@$(CXX) -MM -c -MT "$@" -o "$@.dep" $(CFLAGS) $<

clean:
	rm -rf $(BIN)

include $(shell test -d $(BIN) && find $(BIN) -name "*.dep")

