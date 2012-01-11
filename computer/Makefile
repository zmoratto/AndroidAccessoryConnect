CXXFLAGS += -I/opt/local/include -O1 -g
LDFLAGS += -L/opt/local/lib -lusb-1.0

%.o : %.cc
	$(CXX) -c -o $@ $(CXXFLAGS) $^

usb_search: usb_search.o
	$(CXX) -o $@ $(CXXFLAGS) $(LDFLAGS) $^

clean:
	rm -f *.o *~ usb_search