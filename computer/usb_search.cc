#include <iostream>
#include <cstdlib>

#include "AndroidUSB.h"

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

int main( int argc, char **argv ) {

  if ( libusb_init(NULL) < 0 ) {
    std::cout << "Something happened during intialization\n";
    return 1;
  }

  //libusb_set_debug(NULL, 3);

  // Claim a USB interface for I/O
  libusb_device_handle *devh = NULL;
  for ( size_t count = 0; count < 101; count++ ) {

    devh = find_android_accessory("NASA Ames Research Center",
				  "Spheres 1.5",
				  "Spheres 1.5 Beagle Board",
				  "1.5.0", "http://www.nasa.gov",
				  "0000000000000001");
    if ( devh )
      break;
    if ( count == 50 )
      std::cerr << "Still haven't found device. Are you sure you want to keep searching?\n";
    if ( count == 100 ) {
      std::cerr << "Failed to find device. quitting.\n";
      libusb_exit(NULL);
      return 1;
    }

#if defined __APPLE__
    // For whatever reason, device detection with libusb on OSX is
    // extremely slow. Linux is so fast in comparison that I've added
    // a sleep for that platform.
#else
    sleep(1);
#endif

    count++;
  }

  std::cout << "YOU GOT AN ACCESSORY!\n";

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

  // According to documentation, Interface 0 is supposed to be
  // standard communications, Interface 1 is for ADB. Let's claim
  // Interface 0 and pretend to be the ADK dev board.
  int r = libusb_claim_interface( devh, 0 );
  interpret_return_value("claim the accessories standard interface",
                         devh, r );

  std::cout << "Entering Bulk Transfer Loop\n";
  std::cout << "This won't do anything until you have started the DemoKit app on the connected phone.\n";
  uint8_t message[3];
  int length;
  uint16_t count = 0;
  while (1) {
    // Wait for command
    r = libusb_bulk_transfer( devh, endpoint_in, message, 3, &length, 1 );
    if ( r != LIBUSB_SUCCESS &&
         r != LIBUSB_ERROR_TIMEOUT )
      interpret_return_value( "recieve command", devh, r );

    if ( length > 0 ) {
      if ( message[0] == 0x2 ) {
        // Request to write  analog
        switch( message[1] ) {
        case 0x0:
          std::cout << "LED1_RED: " << int(message[2]) << "\n"; break;
        case 0x1:
          std::cout << "LED1_GREEN: " << int(message[2]) << "\n"; break;
        case 0x2:
          std::cout << "LED1_BLUE: " << int(message[2]) << "\n"; break;
        case 0x3:
          std::cout << "LED2_RED: " << int(message[2]) << "\n"; break;
        case 0x4:
          std::cout << "LED2_GREEN: " << int(message[2]) << "\n"; break;
        case 0x5:
          std::cout << "LED2_BLUE: " << int(message[2]) << "\n"; break;
        case 0x6:
          std::cout << "LED3_RED: " << int(message[2]) << "\n"; break;
        case 0x7:
          std::cout << "LED3_GREEN: " << int(message[2]) << "\n"; break;
        case 0x8:
          std::cout << "LED3_BLUE: " << int(message[2]) << "\n"; break;
        case 0x10:
          std::cout << "SRV1: " << int(message[2]) << "\n"; break;
        case 0x11:
          std::cout << "SRV2: " << int(message[2]) << "\n"; break;
        case 0x12:
          std::cout << "SRV3: " << int(message[2]) << "\n"; break;
        }
      } else if ( message[0] == 0x3 ) {
        // Request we write a relay
        if ( message[1] == 0x0 )
          std::cout << "RELAY1: " << int(message[2]) << "\n";
        else if ( message[1] == 0x1 )
          std::cout << "RELAY2: " << int(message[2]) << "\n";
      }
    }

    // Send them a joystick command
    message[0] = 0x6;
    message[1] = *((uint8_t*)&count);
    message[2] = *((uint8_t*)&count + 1);
    r = libusb_bulk_transfer( devh, endpoint_out, message, 3, &length, 30 );
    if ( r != LIBUSB_SUCCESS &&
         r != LIBUSB_ERROR_TIMEOUT )
      interpret_return_value( "write joystick", devh, r );

    count++;
  } // End of while loop

  // Disconnect
  libusb_unref_device( dev );
  libusb_release_interface( devh, 0 );
  libusb_close(devh);
  libusb_exit(NULL);

  return 0;
}
