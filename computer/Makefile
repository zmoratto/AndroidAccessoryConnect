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

%.o : %.cc
	$(CXX) -c -o $@ $(CXXFLAGS) $^

usb_search: usb_search.o AndroidUSB.o
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LDFLAGS)

clean:
	rm -f *.o *~ usb_search