SRCS_SERVER = err.cpp server.cpp Connection.cpp Message.cpp netstoreserver.cpp
SRCS_CLIENT = err.cpp client.cpp Connection.cpp Message.cpp netstoreclient.cpp

DEPDIR := .d
$(shell mkdir -p $(DEPDIR) >/dev/null)
DEPFLAGS = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.Td

CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17
LDFLAGS := -L/usr/lib/x86_64-linux-gnu/ -lboost_program_options -lstdc++fs

COMPILE.cc = $(CXX) $(DEPFLAGS) $(CXXFLAGS) -c
POSTCOMPILE = @mv -f $(DEPDIR)/$*.Td $(DEPDIR)/$*.d && touch $@

%.o : %.cpp
%.o : %.cpp $(DEPDIR)/%.d
	$(COMPILE.cc) $(OUTPUT_OPTION) $<
	$(POSTCOMPILE)

$(DEPDIR)/%.d: ;
.PRECIOUS: $(DEPDIR)/%.d

include $(wildcard $(patsubst %,$(DEPDIR)/%.d,$(basename $(SRCS_SERVER))))
include $(wildcard $(patsubst %,$(DEPDIR)/%.d,$(basename $(SRCS_CLIENT))))

server : err.o server.o Message.o Connection.o netstoreserver.o
	$(CXX) $^ $(CXXFLAGS) $(LDFLAGS) -o $@

client : err.o client.o Message.o Connection.o netstoreclient.o
	$(CXX) $^ $(CXXFLAGS) $(LDFLAGS) -o $@

clean:
	-rm *.o server client