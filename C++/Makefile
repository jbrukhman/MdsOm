#
# Make the helper class
#

.SUFFIXES:
.SUFFIXES: .cpp .o

THE_LIB = $(MDSOM_OUTDIR)/libmdsom.so

THE_HEADERS = MdsOmConfig.h MdsOmEnv.h MdsOm.h MdsOmPublisher.h MdsOmStatus.h MdsOmAction.h MdsOmSubscription.h MdsOmTransportCallback.h MdsOmTransports.h MdsOmTransportTopicCallback.h MdsOmLock.h MdsOmList.h

SOURCES = MdsOmConfig.cpp MdsOm.cpp MdsOmEnv.cpp MdsOmPublisher.cpp MdsOmStatus.cpp MdsOmSubscription.cpp MdsOmUtil.cpp MdsOmTransports.cpp MdsOmLock.cpp MdsOmList.cpp 

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
	$(CXX) $(LDFLAGS) -o $(THE_LIB) -L$(OPENMAMA_HOME)/lib -lmama -lmamacpp -lpthread $(OBJECTS)
	cp $(THE_LIB) $(OPENMAMA_HOME)/lib
	cp $(THE_HEADERS) $(OPENMAMA_HOME)/include/MdsOm

$(OBJECTS):	$(SOURCES) $(THE_HEADERS)

$(MDSOM_OUTDIR)/%.o: %.cpp
	$(CXX) $(CFLAGS) $(OM_241) -I$(OPENMAMA_HOME)/include -o $@ $<

