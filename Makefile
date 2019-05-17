SRCS_SERVER = group.cpp err.cpp server.cpp netstoreserver.cpp

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

server : netstoreserver.o server.o group.o err.o
	$(CXX) $^ $(CXXFLAGS) $(LDFLAGS) -o $@

clean:
	-rm *.o server