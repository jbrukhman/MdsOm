#
# Make the helper class into a static archive
#

.SUFFIXES:
.SUFFIXES: .cpp .o

THE_LIB = $(MDSOM_OUTDIR)/libmdsomchain.so

THE_HEADERS=MdsOmChainApp.h MdsOmChainLinkType.h MdsOmChainCallback.h MdsOmDataLink.h MdsOmPubChain.h MdsOmSubChain.h

SOURCES=MdsOmChainCallback.cpp MdsOmChainConfig.cpp MdsOmChain.cpp MdsOmChainName.cpp MdsOmPubChain.cpp MdsOmSubChain.cpp MdsOmDataLink.cpp MdsOmElement.cpp MdsOmLink.cpp MdsOmOs.cpp

OBJS = $(SOURCES:.cpp=.o)
OBJECTS = $(addprefix $(MDSOM_OUTDIR)/, $(OBJS))

CFLAGS = -c -g -O2 -fPIC
LDFLAGS = -shared
ifeq ($(OPENMAMA_BITS), 32)
	CFLAGS += -m32
	LDFLAGS += -m32
endif

all: pre-build $(THE_LIB)

pre-build:
	mkdir -p $(MDSOM_OUTDIR)
	mkdir -p $(OPENMAMA_HOME)/lib
	mkdir -p $(OPENMAMA_HOME)/include/MdsOm

install: $(THE_LIB)
	cp $(THE_LIB) $(OPENMAMA_HOME)/lib
	cp $(THE_HEADERS) $(OPENMAMA_HOME)/include/MdsOm

pre-install: pre-build
	cp -f $(THE_HEADERS) $(OPENMAMA_HOME)/include/MdsOm

clean:
	$(RM) -r $(MDSOM_OUTDIR)

$(THE_LIB): $(OBJECTS)
	$(CXX) $(LDFLAGS) -o $(THE_LIB) -L$(OPENMAMA_HOME)/lib -lmdsom -lmama -lmamacpp -lpthread $(OBJECTS)
	cp $(THE_LIB) $(OPENMAMA_HOME)/lib
	cp $(THE_HEADERS) $(OPENMAMA_HOME)/include/MdsOm

$(OBJECTS):	$(SOURCES) $(THE_HEADERS)

$(MDSOM_OUTDIR)/%.o: %.cpp
	$(CXX) $(CFLAGS) $(OM_241)  -I$(OPENMAMA_HOME)/include -o $@ $<
