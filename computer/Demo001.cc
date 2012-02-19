#include <iostream>
#include <cstdlib>

#include "AndroidUSB.h"
#include "source.pb.h"

// This application is a base station for the Demo001 app that runs on
// android. It's ultimate goal is to read and Xsens and then feed that
// information via USB to the android phone. The android phone will
// then pretend to be a ROS node.

void interpret_return_value( std::string const& action,
                             libusb_device_handle* devh,
                             int r ) {
  if ( r < 0 ) {
    std::cerr << "Failed to " << action << ". ["
              << usb_error(r) << "]\n";
    libusb_close(devh);
    libusb_exit(NULL);
    exit(1);
  }
}

char print_wheel( uint8_t counter ) {
  switch ( counter & 0x03 ) {
  case 0: return '|';
  case 1: return '/';
  case 2: return '-';
  case 3: return '\\';
  }
  return ' ';
}

int main( int arc, char **argv ) {
  if ( libusb_init(NULL) < 0 ) {
    std::cerr << "Failed to initialize libusb\n";
    return 1;
  }

  libusb_device_handle *devh = NULL;
  {
    std::cout << "Waiting for android device  " << std::flush;
    uint8_t counter = 0;
    while (true) {
      devh = find_android_accessory("NASA Ames Research Center",
                                    "Spheres 1.5",
                                    "Spheres 1.5 Beagle Board",
                                    "1.5.0", "http://www.nasa.gov",
                                    "0000000000000002");
      std::cout << "\b" << print_wheel( counter ) << std::flush;
      if (devh) break;

#if defined __APPLE__
      // For whatever reason, device detection with libusb on OSX is
      // extremely slow. Linux is so fast in comparison that I've added
      // a sleep for that platform.
#else
      sleep(1);
#endif
      counter++;
    }

    std::cout << "\n\n";
  }

  uint8_t endpoint_in, endpoint_out;
  // Find more information on this device. What interfaces are available?
  // This also find the two endpoints that we will be using for communications
  libusb_device* dev = libusb_get_device(devh);
  libusb_device_descriptor device_desc;
  libusb_get_device_descriptor(dev, &device_desc);
  std::cout << "Number of configurations: "
            << int(device_desc.bNumConfigurations) << "\n";

  libusb_config_descriptor* config_desc =
    new libusb_config_descriptor[device_desc.bNumConfigurations];
  libusb_get_active_config_descriptor(dev, &config_desc);
  for ( size_t i = 0; i < device_desc.bNumConfigurations; i++ ) {
    libusb_config_descriptor* desc = &config_desc[i];
    std::cout << "Configuration " << i << " ----------------\n";
    std::cout << " Number Interfaces: " << int(desc->bNumInterfaces) << std::endl;
    std::cout << " Max Power: " << int(desc->MaxPower) << std::endl;

    // Iterate through the number of interfaces and describe endpoints
    for ( size_t j = 0; j < desc->bNumInterfaces; j++ ) {
      const libusb_interface* interface = &desc->interface[j];
      std::cout << "  Alt Settings: " << int(interface->num_altsetting) << std::endl;;

      // Iterate through alt settings
      for ( size_t k = 0; k < interface->num_altsetting; k++ ) {
        const libusb_interface_descriptor* interface_desc =
          &interface->altsetting[k];
        std::cout << "   Num Endpoints: " << int(interface_desc->bNumEndpoints) << std::endl;
        std::cout << "   Interface Class: " << int(interface_desc->bInterfaceClass) << std::endl;
        std::cout << "   Interface Protocol: " << int(interface_desc->bInterfaceProtocol) << std::endl;

        for ( size_t l = 0; l < interface_desc->bNumEndpoints; l++ ) {
          const libusb_endpoint_descriptor* endpoint =
            &interface_desc->endpoint[l];
          std::cout << "    Interface Interval: " << int(endpoint->bInterval) << std::endl;
          std::cout << "    wMaxPacketSize: " << int(endpoint->wMaxPacketSize) << std::endl;
          std::cout << "    Direction: ";
          if ( j == 0 ) {
            if ( ( endpoint->bEndpointAddress & 0x80 ) == LIBUSB_ENDPOINT_IN ) {
              std::cout << "IN *ATTACHED*\n";
              endpoint_in = endpoint->bEndpointAddress;
            } else {
              std::cout << "OUT *ATTACHED*\n";
              endpoint_out = endpoint->bEndpointAddress;
            }
          } else {
            if ( ( endpoint->bEndpointAddress & 0x80 ) == LIBUSB_ENDPOINT_IN ) {
              std::cout << "IN\n";
            } else {
              std::cout << "OUT\n";
            }
          }
          std::cout << "    EP Address: " << std::hex << int(endpoint->bEndpointAddress) << std::dec << "\n";
        }
      }
    }
  }
  delete[] config_desc;

  // Interface 0 is supposed to be the one we use for ADK.
  int r = libusb_claim_interface( devh, 0 );
  interpret_return_value("claim the accessories standard interface",
                         devh, r);
  double counter = 0.0;
  int length;
  while (1) {
    demo::OffboardData data;
    data.set_hour(counter);

    std::string array = data.SerializeAsString();
    //std::string array = "Monkey";
    //array += "\n";

    r = libusb_bulk_transfer( devh, endpoint_out, (unsigned char*)array.c_str(),
                              array.size(), &length, 100 );
    if ( r != LIBUSB_SUCCESS &&
         r != LIBUSB_ERROR_TIMEOUT )
      interpret_return_value( "write off board", devh, r );

    if ( array.size() != length ) {
      std::cerr << "Didn't write entire structure: "
                << array.size() << " != " << length << std::endl;
      std::cerr << "Tried to send [" << array << "]\n";
    }

    sleep(1);
    counter += 0.1;
  }

  // Disconnect
  libusb_unref_device( dev );
  libusb_release_interface( devh, 0 );
  libusb_close(devh);
  libusb_exit(NULL);

  return 0;
}
