#ifndef __AAC_ANDROID_USB_H__
#define __AAC_ANDROID_USB_H__

#include <libusb-1.0/libusb.h>
#include <string>

// Value interpretation helpers
bool is_android_vendor( uint16_t vid );
std::string android_state( uint16_t product );
std::string usb_error( int error );

// Loading device helpers
libusb_device_handle* find_android_device();

// Calls find_android_device and then puts the phone into accessory
// mode.
libusb_device_handle*
find_android_accessory( std::string const& manufacturer,
			std::string const& model,
			std::string const& description,
			std::string const& version,
			std::string const& uri,
			std::string const& serial );

#endif//__AAC_ANDROID_USB_H__
