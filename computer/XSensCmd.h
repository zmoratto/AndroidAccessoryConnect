/* -*- C++ -*- */
// File: XSensCmd.h
// Description: A second XSens Hw interface header file
//
// Author: Hans Utz
// Date: 9/1/2011

#ifndef kn_XSensCmd_h
#define kn_XSensCmd_h

//#include "knXSens_Export.h"

#include <stdint.h>

namespace kn 
{
  namespace XSensCmd
  {
    static uint8_t const WakeUp                 = 0x3e;
    static uint8_t const WakeUpAck              = 0x3f;
    static uint8_t const GoToConfig             = 0x30;
    static uint8_t const GoToConfigAck          = 0x31;
    static uint8_t const GoToMeasurement        = 0x10;
    static uint8_t const GoToMeasurementAck     = 0x11;
    static uint8_t const Reset                  = 0x40;
    static uint8_t const ResetAck               = 0x41;

    static uint8_t const ReqDID                 = 0x00;
    static uint8_t const DeviceID               = 0x01;
    static uint8_t const InitBus                = 0x02;
    static uint8_t const InitBusResults         = 0x03;
    static uint8_t const ReqProductCode         = 0x1c;
    static uint8_t const ProductCode            = 0x1d;
    static uint8_t const ReqFWRev               = 0x12;
    static uint8_t const FirmwareRev            = 0x13;
    static uint8_t const ReqDataLength          = 0x0a;
    static uint8_t const DataLength             = 0x0b;
    static uint8_t const RunSelftest            = 0x24;
    static uint8_t const SelftestAck            = 0x25;

    static uint8_t const Error                  = 0x42;

    static uint8_t const ReqGPSStatus           = 0xa6;
    static uint8_t const GPSStatus              = 0xa7;

    static uint8_t const ReqBaudrate            = 0x18;
    static uint8_t const ReqBaudrateAck         = 0x19;
    static uint8_t const SetBaudrate            = 0x18;
    static uint8_t const SetBaudrateAck         = 0x19;

    static uint8_t const ReqErrorMode           = 0xda;
    static uint8_t const ReqErrorModeAck        = 0xdb;
    static uint8_t const SetErrorMode           = 0xda;
    static uint8_t const SetErrorModeAck        = 0xdb;

    static uint8_t const ReqConfiguration       = 0x0c;
    static uint8_t const Configuration          = 0x0d;

    static uint8_t const ReqPeriod              = 0x04;
    static uint8_t const ReqPeriodAck           = 0x05;
    static uint8_t const SetPeriod              = 0x04;
    static uint8_t const SetPeriodAck           = 0x05;

    static uint8_t const ReqOutputSkipFactor    = 0xd4;
    static uint8_t const ReqOutputSkipFactorAck = 0xd5;
    static uint8_t const SetOutputSkipFactor    = 0xd4;
    static uint8_t const SetOutputSkipFactorAck = 0xd5;

    static uint8_t const ReqObjectAlignment     = 0xe0;
    static uint8_t const ReqObjectAlignmentAck  = 0xe1;
    static uint8_t const SetObjectAlignment     = 0xe0;
    static uint8_t const SetObjectAlignmentAck  = 0xe1;

    static uint8_t const ReqOutputMode          = 0xd0;
    static uint8_t const ReqOutputModeAck       = 0xd1;
    static uint8_t const SetOutputMode          = 0xd0;
    static uint8_t const SetOutputModeAck       = 0xd1;

    static uint8_t const ReqOutputSettings      = 0xd2;
    static uint8_t const ReqOutputSettingsAck   = 0xd3;
    static uint8_t const SetOutputSettings      = 0xd2;
    static uint8_t const SetOutputSettingsAck   = 0xd3;

    static uint8_t const ReqData                = 0x34;
    static uint8_t const ReqDataAck             = 0x35;
    static uint8_t const MTData                 = 0x32;

    static uint8_t const ReqMagneticDeclination    = 0x6a;
    static uint8_t const ReqMagneticDeclinationAck = 0x6b;
    static uint8_t const SetMagneticDeclination    = 0x6a;
    static uint8_t const SetMagneticDeclinationAck = 0x6b;

    static uint8_t const ReqAvailableScenarios     = 0x62;
    static uint8_t const ReqAvailableScenariosAck  = 0x63;

    static uint8_t const ReqCurrentScenario        = 0x64;
    static uint8_t const ReqCurrentScenarioAck     = 0x65;
    static uint8_t const SetCurrentScenario        = 0x64;
    static uint8_t const SetCurrentScenarioAck     = 0x65;

    static uint8_t const ReqGravityMagnitude       = 0x66;
    static uint8_t const ReqGravityMagnitudeAck    = 0x67;
    static uint8_t const SetGravityMagnitude       = 0x66;
    static uint8_t const SetGravityMagnitudeAck    = 0x67;

    static uint8_t const ReqProcessingFlags        = 0x20;
    static uint8_t const ReqProcessingFlagsAck     = 0x21;
    static uint8_t const SetProcessingFlags        = 0x20;
    static uint8_t const SetProcessingFlagsAck     = 0x21;

    static uint8_t const ReqLeverArmGps            = 0x68;
    static uint8_t const ReqLeverArmGpsAck         = 0x69;
    static uint8_t const SetLeverArmGps            = 0x68;
    static uint8_t const SetLeverArmGpsAck         = 0x69;

    static uint8_t const ResetOrientation          = 0xa4;
    static uint8_t const ResetOrientationAck       = 0xa5;

    static uint8_t const ReqUTCTime                = 0x60;
    static uint8_t const UTCTime                   = 0x61;
  }
}
#endif // kn_XSensCmd_h
