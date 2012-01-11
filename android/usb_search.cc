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
    libusb_exit(NULL);
    exit(1);
  }

  // Return the specific device and free the rest.
  libusb_free_device_list(devs, 1);
  return libusb_open_device_with_vid_pid(NULL, vendor_id, product_id);
}

libusb_device_handle* find_android_accessory() {

  // Search for VID PID that corresponds to an android in accessory
  libusb_device_handle* devh;
  devh = libusb_open_device_with_vid_pid(NULL, 6353u, 11520u);
  if ( devh ) return devh;
  libusb_open_device_with_vid_pid(NULL, 6353u, 11521u);
  if ( devh ) return devh;

  // It doesn't look like an android accessory exists. Let's just look
  // for a google device.
  devh = find_android_device();
  if ( !devh ) {
    // We didn't manage to find an android device on the bus. Lets
    // exit every thing.
    libusb_exit(NULL);
    exit(1);
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

int main( int argc, char **argv ) {

  if ( libusb_init(NULL) < 0 ) {
    std::cout << "Something happened during intialization\n";
    return 1;
  }

  //libusb_set_debug(NULL, 3);

  // Claim a USB interface for I/O
  libusb_device_handle *devh = NULL;
  while (!devh) {
    devh = find_android_accessory();
  }

  // Do stuff to setup our accessories.
  std::cout << "YOU GOT AN ACCESSORY!\n";

  // Find more information on this device. What interfaces are available?
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
    std::cout << " Number Interface: " << int(desc->bNumInterfaces) << std::endl;
    std::cout << " Max Power: " << int(desc->MaxPower) << std::endl;
  }

  delete[] config_desc;
  libusb_unref_device( dev );

  // Disconnect
  libusb_close(devh);
  libusb_exit(NULL);

  return 0;
}
