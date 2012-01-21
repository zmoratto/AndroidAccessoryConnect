#include "AndroidUSB.h"
#include <iostream>
#include <cstdlib>

bool is_android_vendor( uint16_t vid ) {
  switch (vid) {
  case 0x0502: // Acer
  case 0x0b05: // ASUS
  case 0x413c: // Dell
  case 0x0489: // Foxconn
  case 0x091e: // Garmin - Asus
  case 0x18d1: // Google
  case 0x109b: // Hisense
  case 0x0bb4: // HTC
  case 0x12d1: // Huawei
  case 0x24e3: // K-Touch
  case 0x2116: // KT Tech
  case 0x0482: // Kyocera
  case 0x17ef: // Lenevo
  case 0x1004: // LG
  case 0x22b8: // Motorola
  case 0x0409: // NEC
  case 0x2080: // Nook
  case 0x0955: // Nvidia
  case 0x2257: // OTGV
  case 0x10a9: // Pantech
  case 0x1d4d: // Pegatron
  case 0x0471: // Philips
  case 0x04da: // PMC-Sierra
  case 0x05c6: // Qualcomm
  case 0x1f53: // SK Telesys
  case 0x04e8: // Samsung
  case 0x04dd: // Sharp
  case 0x0fce: // Sony Ericsson
  case 0x2340: // Teleepoch
  case 0x0930: // Toshiba
  case 0x19d2: // ZTE
    return true;
  }
  return false;
}

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

    if ( is_android_vendor( desc.idVendor ) ) {
      std::cout << "Found potential android phone ["
                << android_state(desc.idProduct) << "]\n";
      vendor_id = desc.idVendor;
      product_id = desc.idProduct;
      break;
    }
  }
  if (count == i ) {
    libusb_free_device_list(devs, 1);
    return NULL;
  }

  // Return the specific device and free the rest.
  libusb_free_device_list(devs, 1);
  return libusb_open_device_with_vid_pid(NULL, vendor_id, product_id);
}

libusb_device_handle*
find_android_accessory( std::string const& manufacturer,
			std::string const& model,
			std::string const& description,
			std::string const& version,
			std::string const& uri,
			std::string const& serial ) {

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
                           52, 0, 0, (unsigned char*)manufacturer.c_str(),
                           manufacturer.size(), TIMEOUT_MS );
  libusb_control_transfer( devh, LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_OUT,
                           52, 0, 1, (unsigned char*)model.c_str(),
                           model.size(), TIMEOUT_MS );
  libusb_control_transfer( devh, LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_OUT,
                           52, 0, 2, (unsigned char*)description.c_str(),
                           description.size(), TIMEOUT_MS );
  libusb_control_transfer( devh, LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_OUT,
                           52, 0, 3, (unsigned char*)version.c_str(),
			   version.size(), TIMEOUT_MS );
  libusb_control_transfer( devh, LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_OUT,
                           52, 0, 4, (unsigned char*)uri.c_str(),
                           uri.size(), TIMEOUT_MS );
  libusb_control_transfer( devh, LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_OUT,
                           52, 0, 5, (unsigned char*)serial.c_str(),
                           serial.size(), TIMEOUT_MS );

  // Request that the phone goes into accessory mode
  libusb_control_transfer( devh, LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_OUT,
                           53, 0, 0, NULL, 0, TIMEOUT_MS );

  // Disconnect
  libusb_close(devh);

  return NULL;
}

