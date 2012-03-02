CXXFLAGS += -O1 -g

UNAME := $(shell uname)

# Add dependency for libusb
ifeq ($(UNAME), Linux)
	CXXFLAGS += -I/usr/include
	LDFLAGS += -L/usr/lib -lusb-1.0
endif
ifeq ($(UNAME), Darwin)
	CXXFLAGS += -I/opt/local/include
	LDFLAGS += -L/opt/local/lib -lusb-1.0
endif

all: Demo001 usb_search

source.pb.cc:
	protoc ../android/Demo001/src/nasa/demo/proto/source.proto --cpp_out=. --proto_path=../android/Demo001/src/nasa/demo/proto/

%.o : %.cc
	$(CXX) -c -o $@ $(CXXFLAGS) $^

usb_search: usb_search.o AndroidUSB.o
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LDFLAGS)

Demo001: source.pb.o AndroidUSB.o Demo001.o serialstream.o XSensData.o XSens.o
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LDFLAGS) -lprotobuf

clean:
	rm -f *.o *~ usb_search source.pb.cc source.pb.h
