#include <iostream>
#include <libusb-1.0/libusb.h>

const char* manufacturer = "NASA Ames Research Center";
const char* model        = "Spheres 1.5";
const char* description  = "Spheres 1.5 Beagle Board";
const char* version      = "1.5.0";
const char* uri          = "http://www.nasa.gov";
const char* serial       = "0000000000000001";

std::string android_state( uint16_t product ) {
  switch (product) {
  case 0x4e20:
    return "bootloader";
  case 0x4e21:
    return "android os";
  case 0x4e22:
    return "android os w/ debugger";
  case 0x2d00:
    return "accessory mode";
  case 0x2d01:
    return "accessory mode w/ adb";
  }
  return "unknown";
}

std::string usb_error( int error ) {
  switch (error) {
  case LIBUSB_ERROR_NOT_FOUND:
    return "not found";
  case LIBUSB_ERROR_NO_DEVICE:
    return "no device";
  case LIBUSB_ERROR_IO:
    return "error IO";
  case LIBUSB_ERROR_INVALID_PARAM:
    return "invalid param";
  case LIBUSB_ERROR_ACCESS:
    return "error access";
  case LIBUSB_ERROR_BUSY:
    return "error busy";
  case LIBUSB_ERROR_TIMEOUT:
    return "timeout";
  case LIBUSB_ERROR_OVERFLOW:
    return "overflow";
  case LIBUSB_ERROR_PIPE:
    return "pipe";
  case LIBUSB_ERROR_INTERRUPTED:
    return "interrupted";
  case LIBUSB_ERROR_NO_MEM:
    return "no memory";
  case LIBUSB_ERROR_NOT_SUPPORTED:
    return "not supported";
  case LIBUSB_ERROR_OTHER:
    return "other";
  case LIBUSB_SUCCESS:
    break;
  }
  return "";
}

libusb_device_handle* find_android_device() {
  libusb_device **devs;

  ssize_t count = libusb_get_device_list(NULL, &devs);
  if ( count < 0 ) {
    std::cout << "Something happened on device list\n";
    exit(1);
  }

  uint16_t vendor_id, product_id;

  // Cycle though the USB devices
  int i;
  for ( i = 0; i < count; i++ ) {
    libusb_device *dev = devs[i];
    libusb_device_descriptor desc;
    int r = libusb_get_device_descriptor(dev, &desc);
    if ( r < 0 ) {
      std::cerr << "Failed to get device descriptor [" << usb_error(r) << "]\n";
      exit(1);
    }

    if ( desc.idVendor == 0x18d1 ) {
      std::cout << "Found potential android phone ["
                << android_state(desc.idProduct) << "]\n";
      vendor_id = desc.idVendor;
      product_id = desc.idProduct;
      break;
    }
  }
  if (count == i ) {
    std::cerr << "No android devices found\n";
    libusb_free_device_list(devs, 1);
    return NULL;
  }

  // Return the specific device and free the rest.
  libusb_free_device_list(devs, 1);
  return libusb_open_device_with_vid_pid(NULL, vendor_id, product_id);
}

libusb_device_handle* find_android_accessory() {

  // Search for VID PID that corresponds to an android in accessory
  libusb_device_handle* devh;
  devh = libusb_open_device_with_vid_pid(NULL, 0x18d1, 0x2d00);
  if ( devh ) return devh;
  devh = libusb_open_device_with_vid_pid(NULL, 0x18d1, 0x2d01);
  if ( devh ) return devh;

  // It doesn't look like an android accessory exists. Let's just look
  // for a google device.
  devh = find_android_device();
  if ( !devh ) {
    // We didn't manage to find an android device on the bus.
    return NULL;
  }

  // Lets request that this google device goes into device mode.
  // -----------------------------------------------------------

  // Request supported accessory protocol.
  const unsigned int TIMEOUT_MS = 1000;
  uint16_t protocol;
  libusb_control_transfer( devh, LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_IN,
                           51, 0, 0, (unsigned char*)&protocol, 2, TIMEOUT_MS );
  if ( protocol != 1 ) {
    std::cerr << "Failed to get correct protocol reply.\n";
    libusb_release_interface(devh, 1);
    libusb_close(devh);
    libusb_exit(NULL);
    exit(1);
  }

  // Send indentifying information about ourselves.
  libusb_control_transfer( devh, LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_OUT,
                           52, 0, 0, (unsigned char*)manufacturer,
                           strlen(manufacturer)+1, TIMEOUT_MS );
  libusb_control_transfer( devh, LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_OUT,
                           52, 0, 1, (unsigned char*)model,
                           strlen(model)+1, TIMEOUT_MS );
  libusb_control_transfer( devh, LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_OUT,
                           52, 0, 2, (unsigned char*)description,
                           strlen(description)+1, TIMEOUT_MS );
  libusb_control_transfer( devh, LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_OUT,
                           52, 0, 3, (unsigned char*)version,
                           strlen(version)+1, TIMEOUT_MS );
  libusb_control_transfer( devh, LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_OUT,
                           52, 0, 4, (unsigned char*)uri,
                           strlen(uri)+1, TIMEOUT_MS );
  libusb_control_transfer( devh, LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_OUT,
                           52, 0, 5, (unsigned char*)serial,
                           strlen(serial)+1, TIMEOUT_MS );

  // Request that the phone goes into accessory mode
  libusb_control_transfer( devh, LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_OUT,
                           53, 0, 0, NULL, 0, TIMEOUT_MS );

  // Disconnect
  libusb_close(devh);

  return NULL;
}

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
    devh = find_android_accessory();
    if ( devh )
      break;
    if ( count == 50 )
      std::cerr << "Still haven't found device. Are you sure you want to keep searching?\n";
    if ( count == 100 ) {
      std::cerr << "Failed to find device. quitting.\n";
      libusb_exit(NULL);
      return 1;
    }
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
