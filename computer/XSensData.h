/* -*- C++ -*- */
// File: XSensData.h
// Description: A second XSens Hw interface header file
//   This interface is very simple for now.  Features can be added later
//   as needed.
//
// Author: Hans Utz
// Date: 9/1/2011

#ifndef kn_XSensData_h
#define kn_XSensData_h

#include <iosfwd>
#include <iostream>
#include <stdint.h>
#include <netinet/in.h>


//class ACE_Time_Value;

namespace kn 
{
  namespace XSensData
  {
    struct __attribute__ ((__packed__)) UInt8Data
    {
      uint8_t value;

      UInt8Data() {}
      UInt8Data(uint8_t v) : value(v) {}
    };
    inline
    char * operator<<(char *& ostr, UInt8Data rhs) {
      *ostr = rhs.value;
      ostr += sizeof(rhs.value);
      return ostr;
    }
    inline
    char const * operator>>(char const *& istr, UInt8Data& rhs) {
      rhs.value = *istr;
      istr += sizeof(rhs.value);
      return istr;
    }

    // Basic Types

    struct __attribute__ ((__packed__)) UInt16Data
    {
      uint16_t value;

      UInt16Data() {}
      UInt16Data(uint16_t v) : value(v) {}
    };
    std::ostream& operator<<(std::ostream& ostr, UInt16Data rhs);   
    inline
    char * operator<<(char *& ostr, UInt16Data rhs) {
      char const * d = reinterpret_cast<char const *>(&rhs);

      ostr[1] = d[0];
      ostr[0] = d[1];

      ostr += sizeof(rhs);
      return ostr;
    }
    inline
    char const * operator>>(char const *& istr, UInt16Data& rhs) {
      rhs.value = htons(rhs.value);
      memcpy(&rhs, istr, sizeof(rhs));
      char * d = reinterpret_cast<char *>(&rhs);
     
      d[1] = istr[0];
      d[0] = istr[1];

      istr += sizeof(rhs);
      return istr;
    }

    struct  __attribute__ ((__packed__)) UInt32Data
    {
      uint32_t value;

      UInt32Data() {}
      UInt32Data(uint32_t v) : value(v) {}
    };
    std::ostream& operator<<(std::ostream& ostr, UInt32Data rhs);
    inline
    char * operator<<(char *& ostr, UInt32Data rhs) {

      char const * d = reinterpret_cast<char const *>(&rhs);
      ostr[3] = d[0];
      ostr[2] = d[1];
      ostr[1] = d[2];
      ostr[0] = d[3];

      ostr += sizeof(rhs);
      return ostr;
    }
    inline
    char const * operator>>(char const *& istr, UInt32Data& rhs) {
      char * d = reinterpret_cast<char *>(&rhs);
      d[3] = istr[0];
      d[2] = istr[1];
      d[1] = istr[2];
      d[0] = istr[3];

      istr += sizeof(rhs);
      return istr;
    }

    struct __attribute__ ((__packed__)) FloatData
    {
      float value;

      FloatData() {}
      FloatData(float v) : value(v) {}
    };
    std::ostream& operator<<(std::ostream& ostr, FloatData rhs);
    inline
    char * operator<<(char *& ostr, FloatData rhs) {
      char const * d = reinterpret_cast<char const *>(&rhs);
      ostr[3] = d[0];
      ostr[2] = d[1];
      ostr[1] = d[2];
      ostr[0] = d[3];

      ostr += sizeof(rhs);
      return ostr;
    }
    inline
    char const * operator>>(char const *& istr, FloatData& rhs) {
      char * d = reinterpret_cast<char *>(&rhs);
      d[3] = istr[0];
      d[2] = istr[1];
      d[1] = istr[2];
      d[0] = istr[3];

      istr += sizeof(rhs);
      return istr;
    }

    struct __attribute__ ((__packed__)) Float3Data
    {
      float xyz[3];
    };
    std::ostream& operator<<(std::ostream& ostr, Float3Data const& rhs);
    inline
    char * operator<<(char *& ostr, Float3Data const& rhs) {

      Float3Data& data = const_cast<Float3Data&>(rhs);
      char * d = reinterpret_cast<char *>(&data);
      ostr[3] = d[0];
      ostr[2] = d[1];
      ostr[1] = d[2];
      ostr[0] = d[3];

      ostr[7] = d[4];
      ostr[6] = d[5];
      ostr[5] = d[6];
      ostr[4] = d[7];

      ostr[11] = d[8];
      ostr[10] = d[9];
      ostr[8] = d[10];
      ostr[9] = d[11];

      ostr += sizeof(rhs);
      return ostr;
    }
    inline
    char const * operator>>(char const *& istr, Float3Data& rhs) {

      char * d = reinterpret_cast<char *>(&rhs);
      d[3] = istr[0];
      d[2] = istr[1];
      d[1] = istr[2];
      d[0] = istr[3];

      d[7] = istr[4];
      d[6] = istr[5];
      d[5] = istr[6];
      d[4] = istr[7];

      d[11] = istr[8];
      d[10] = istr[9];
      d[9] = istr[10];
      d[8] = istr[11];

      istr += sizeof(rhs);
      return istr;
    }

    struct __attribute__ ((__packed__)) OrientationEuler
    {
      float rpy[3];
    };
    std::ostream& operator<<(std::ostream& ostr, OrientationEuler const& rhs);
    inline
    char * operator<<(char *& ostr, OrientationEuler const& rhs) {
#ifdef ACE_LITTLE_ENDIAN
      OrientationEuler& data = const_cast<OrientationEuler&>(rhs);
      ACE_CDR::swap_4_array(reinterpret_cast<char *>(&data), ostr, 3);
#else

      OrientationEuler& data = const_cast<OrientationEuler&>(rhs);
      char * d = reinterpret_cast<char *>(&data);
      ostr[3] = d[0];
      ostr[3] = d[1];
      ostr[2] = d[2];
      ostr[0] = d[3];

      ostr[7] = d[4];
      ostr[6] = d[5];
      ostr[5] = d[6];
      ostr[4] = d[7];

      ostr[11] = d[8];
      ostr[10] = d[9];
      ostr[9] = d[10];
      ostr[8] = d[11];

//memcpy(ostr, &rhs, sizeof(rhs));
#endif
      ostr += sizeof(rhs);
      return ostr;
    }
    inline
    char const * operator>>(char const *& istr, OrientationEuler& rhs) {
#ifdef ACE_LITTLE_ENDIAN
      ACE_CDR::swap_4_array(istr, reinterpret_cast<char *>(&rhs), 3);
#else
      char * d = reinterpret_cast<char *>(&rhs);
      d[3] = istr[0];
      d[2] = istr[1];
      d[1] = istr[2];
      d[0] = istr[3];

      d[7] = istr[4];
      d[6] = istr[5];
      d[5] = istr[6];
      d[4] = istr[7];

      d[11] = istr[8];
      d[10] = istr[9];
      d[9] = istr[10];
      d[8] = istr[11];
//memcpy(&rhs, istr, sizeof(rhs));
#endif
      istr += sizeof(rhs);
      return istr;
    }

    struct __attribute__ ((__packed__)) OrientationQuaternion
    {
      float q[4];
    };
    std::ostream& operator<<(std::ostream& ostr, OrientationQuaternion const& rhs);
    inline
    char * operator<<(char *& ostr, OrientationQuaternion const& rhs) {
#ifdef ACE_LITTLE_ENDIAN
      OrientationQuaternion& data = const_cast<OrientationQuaternion&>(rhs);
      ACE_CDR::swap_4_array(reinterpret_cast<char *>(&data), ostr, 4);
#else
      memcpy(ostr, &rhs, sizeof(rhs));
#endif
      ostr += sizeof(rhs);
      return ostr;
    }
    inline
    char const * operator>>(char const *& istr, OrientationQuaternion& rhs) {
#ifdef ACE_LITTLE_ENDIAN
      ACE_CDR::swap_4_array(istr, reinterpret_cast<char *>(&rhs), 4);
#else
      memcpy(&rhs, istr, sizeof(rhs));
#endif
      istr += sizeof(rhs);
      return istr;
    }

    struct __attribute__ ((__packed__)) OrientationMatrix
    {
      float matrix[9];
    };
    std::ostream& operator<<(std::ostream& ostr, OrientationMatrix const& rhs);
    inline
    char * operator<<(char *& ostr, OrientationMatrix const& rhs) {
#ifdef ACE_LITTLE_ENDIAN
      OrientationMatrix& data = const_cast<OrientationMatrix&>(rhs);
      ACE_CDR::swap_4_array(reinterpret_cast<char *>(&data), ostr, 9);
#else
      OrientationMatrix& data = const_cast<OrientationMatrix&>(rhs);
      char * d = reinterpret_cast<char *>(&data);
      ostr[35] = d[0];
      ostr[34] = d[1];
      ostr[33] = d[2];
      ostr[32] = d[3];

      ostr[31] = d[4];
      ostr[30] = d[5];
      ostr[29] = d[6];
      ostr[28] = d[7];

      ostr[27] = d[8];
      ostr[26] = d[9];
      ostr[25] = d[10];
      ostr[24] = d[11];

      ostr[23] = d[12];
      ostr[22] = d[13];
      ostr[21] = d[14];
      ostr[20] = d[15];

      ostr[19] = d[16];
      ostr[18] = d[17];
      ostr[17] = d[18];
      ostr[16] = d[19];

      ostr[15] = d[20];
      ostr[14] = d[21];
      ostr[13] = d[22];
      ostr[12] = d[23];

      ostr[11] = d[24];
      ostr[10] = d[25];
      ostr[9] = d[26];
      ostr[8] = d[27];

      ostr[7] = d[28];
      ostr[6] = d[29];
      ostr[5] = d[30];
      ostr[4] = d[31];

      ostr[3] = d[32];
      ostr[2] = d[33];
      ostr[1] = d[34];
      ostr[0] = d[35];
//memcpy(ostr, &rhs, sizeof(rhs));
#endif
      ostr += sizeof(rhs);
      return ostr;
    }
    inline
    char const * operator>>(char const *& istr, OrientationMatrix& rhs) {
#ifdef ACE_LITTLE_ENDIAN
      ACE_CDR::swap_4_array(istr, reinterpret_cast<char *>(&rhs), 9);
#else
      char * d = reinterpret_cast<char *>(&rhs);

      d[35] = istr[0];
      d[34] = istr[1];
      d[33] = istr[2];
      d[32] = istr[3];

      d[31] = istr[4];
      d[30] = istr[5];
      d[29] = istr[6];
      d[28] = istr[7];

      d[27] = istr[8];
      d[26] = istr[9];
      d[25] = istr[10];
      d[24] = istr[11];

      d[23] = istr[12];
      d[22] = istr[13];
      d[21] = istr[14];
      d[20] = istr[15];

      d[19] = istr[16];
      d[18] = istr[17];
      d[17] = istr[18];
      d[16] = istr[19];

      d[15] = istr[20];
      d[14] = istr[21];
      d[13] = istr[22];
      d[12] = istr[23];

      d[11] = istr[24];
      d[10] = istr[25];
      d[9] = istr[26];
      d[8] = istr[27];

      d[7] = istr[28];
      d[6] = istr[29];
      d[5] = istr[30];
      d[4] = istr[31];

      d[3] = istr[32];
      d[2] = istr[33];
      d[1] = istr[34];
      d[0] = istr[35];


//memcpy(&rhs, istr, sizeof(rhs));
#endif
      istr += sizeof(rhs);
      return istr;
    }

    typedef UInt32Data DeviceId;
    typedef UInt16Data DataLength;
    typedef UInt16Data SampleCounter;
    typedef FloatData  MagneticDeclination;
    typedef FloatData  TemperatureData;
    typedef FloatData  GravityMagnitude;
    typedef Float3Data VelocityData;
    typedef Float3Data LeverArmGps;
    typedef OrientationMatrix ObjectAlignment;

    struct __attribute__ ((__packed__)) ProductCode
    {
      char code[20];
    };
    std::ostream& operator<<(std::ostream& ostr, ProductCode const& rhs);
    
    struct __attribute__ ((__packed__)) FirmwareRev
    {
      char major;
      char minor;
      char rev;
    };
    std::ostream& operator<<(std::ostream& ostr, FirmwareRev const& rhs);
    inline
    char const * operator>>(char const *& istr, FirmwareRev& rhs) {
      memcpy(&rhs, istr, sizeof(rhs));
      istr += sizeof(rhs);
      return istr;
    }
    
    struct __attribute__ ((__packed__)) SelftestAck : public UInt16Data
    {
      static uint16_t const AccX = 0x0001;
      static uint16_t const AccY = 0x0002;
      static uint16_t const AccZ = 0x0004;
      
      static uint16_t const GyrX = 0x0008;
      static uint16_t const GyrY = 0x0010;
      static uint16_t const GyrZ = 0x0020;
      
      static uint16_t const MagX = 0x0040;
      static uint16_t const MagY = 0x0080;
      static uint16_t const MagZ = 0x0100;
    };
    std::ostream& operator<<(std::ostream& ostr, SelftestAck const& rhs);
    
    struct __attribute__ ((__packed__)) Error : public UInt8Data
    {
      static uint8_t const InvalidPeriod     = 0x03;
      static uint8_t const InvalidMessage    = 0x04;
      static uint8_t const TimerOverflow     = 0x1e;
      static uint8_t const InvalidBaudRate   = 0x20;
      static uint8_t const InvalidParameter  = 0x21;
    };
    std::ostream& operator<<(std::ostream& ostr, Error const& rhs);
    
    struct __attribute__ ((__packed__)) Baudrate : public UInt8Data
    {
      static uint8_t const Baud4k8     = 0x0b;
      static uint8_t const Baud9k6     = 0x09;
      static uint8_t const Baud14k4    = 0x08;
      static uint8_t const Baud19k2    = 0x07;
      static uint8_t const Baud28k8    = 0x06;
      static uint8_t const Baud38k4    = 0x05;
      static uint8_t const Baud57k6    = 0x04;
      static uint8_t const Baud115k2   = 0x02;
      static uint8_t const Baud230k4   = 0x01;
      static uint8_t const Baud460k8   = 0x00;
      static uint8_t const Baud921k6   = 0x80;

      Baudrate() {}
      Baudrate(uint8_t v) : UInt8Data(v) {}

    };
    std::ostream& operator<<(std::ostream& ostr, Baudrate const& rhs);
    
    struct __attribute__ ((__packed__)) ErrorMode : public UInt16Data
    {
      static uint16_t const Ignore  = 0x0000;
      static uint16_t const Counter = 0x0001;
      static uint16_t const Message = 0x0002;
      static uint16_t const Config  = 0x0003;

      ErrorMode() {}
      ErrorMode(UInt16Data v) : UInt16Data(v) {}
    };
    std::ostream& operator<<(std::ostream& ostr, ErrorMode const& rhs);  
    
    struct __attribute__ ((__packed__)) Period : public UInt16Data
    {
      static uint32_t const Sec2Period = 115200;

      Period() {}
      Period(double sec) : UInt16Data((uint16_t)(sec * Sec2Period)) {}

    };
    std::ostream& operator<<(std::ostream& ostr, Period const& rhs);

    struct __attribute__ ((__packed__)) OutputSkipFactor : public UInt16Data
    {
      static uint16_t const DisableStreaming = 0xffff;

      OutputSkipFactor() {}
      OutputSkipFactor(uint16_t v) : UInt16Data(v) {}
    };
    std::ostream& operator<<(std::ostream& ostr, OutputSkipFactor const& rhs);

    struct __attribute__ ((__packed__)) OutputMode : public UInt16Data
    {
      static uint16_t const TemperatureData = 0x0001;
      static uint16_t const CalibratedData  = 0x0002;
      static uint16_t const OrientationData = 0x0004;
      static uint16_t const AuxiliaryData   = 0x0008;
      static uint16_t const PositionData    = 0x0010;
      static uint16_t const VelocityData    = 0x0020;
      static uint16_t const StatusData      = 0x0800;
      static uint16_t const GpsPvtData      = 0x1000;
      static uint16_t const RawInertiaData  = 0x4000;


      OutputMode() {}
      OutputMode(uint16_t v) : UInt16Data(v) {}
    };
    std::ostream& operator<<(std::ostream& ostr, OutputMode const& rhs);

    struct __attribute__ ((__packed__)) OutputSettings : public UInt32Data
    {
      static uint32_t const SampleCounter    = 0x00000001;
      static uint32_t const UTCTime          = 0x00000002;

      static uint32_t const RotQuaternion    = 0x00000000;
      static uint32_t const RotEuler         = 0x00000004;
      static uint32_t const RotMatrix        = 0x00000008;

      static uint32_t const CalibratedNoAcc  = 0x00000010;
      static uint32_t const CalibratedNoGyr  = 0x00000020;
      static uint32_t const CalibratedNoMag  = 0x00000040;

      static uint32_t const OutputFixed12_20 = 0x00000100;
      static uint32_t const OutputFixed16_32 = 0x00000200;

      static uint32_t const AuxDisable_1     = 0x00000400;
      static uint32_t const AuxDisable_2     = 0x00000800;

      static uint32_t const CooridnatesNed   = 0x80000000;

      OutputSettings() {}
      OutputSettings(uint32_t v) : UInt32Data(v) {}

    };
    std::ostream& operator<<(std::ostream& ostr, OutputSettings const& rhs);

    struct  __attribute__ ((__packed__)) Configuration
    {
      DeviceId         masterDeviceID;
      Period           samplingPeriod;
      OutputSkipFactor outputSkipFactor;
      uint16_t         syncInSettingsMode;
      uint16_t         syncInSettingsSkipFactor;
      uint32_t         syncInSettingsOffset;
      uint32_t         dateYear;
      uint16_t         dateMonth;
      uint16_t         dateDay;
      uint16_t         timeHours;
      uint16_t         timeMinutes;
      uint16_t         timeSeconds;
      uint16_t         timeHSec;

      char             reservedHost[32];
      char             reservedClient[32];

      UInt16Data       numDevices;
      DeviceId         deviceID;
      DataLength       dataLength;
      OutputMode       outputMode;
      OutputSettings   outputSettings;

      char             reserved[8];
    };
    std::ostream& operator<<(std::ostream& ostr, Configuration const& rhs);
    char const * operator>>(char const *& istr, Configuration& rhs);

    struct  __attribute__ ((__packed__)) GpsPvtData
    {
      uint16_t     press;
      uint8_t      bPrs;
      uint32_t     itow;
      int32_t      lat;      
      int32_t      lon;      
      int32_t      alt;      
      int32_t      velN;      
      int32_t      velE;      
      int32_t      velD;      
      uint32_t     hAcc;      
      uint32_t     vAcc;      
      uint32_t     sAcc;      
      uint8_t      bGps;

    };
    std::ostream& operator<<(std::ostream& ostr, GpsPvtData const& rhs);
    char const * operator>>(char const *& istr, GpsPvtData& rhs);


    struct __attribute__ ((__packed__)) CalibratedData
    {
      Float3Data acc;
      Float3Data gyr;
      Float3Data mag;
    };
    std::ostream& operator<<(std::ostream& ostr, CalibratedData const& rhs);
    char const * operator>>(char const *& istr, CalibratedData& rhs);

    struct __attribute__ ((__packed__)) PositionData
    {
      float lat;
      float lon;
      float alt;
    };
    std::ostream& operator<<(std::ostream& ostr, PositionData const& rhs);
    char const * operator>>(char const *& istr, PositionData& rhs);

    struct __attribute__ ((__packed__)) StatusData : public UInt8Data
    {
      static uint8_t const Selftest = 0x01;
      static uint8_t const XkfValid = 0x02;
      static uint8_t const GpsFix   = 0x04;
    };
    std::ostream& operator<<(std::ostream& ostr, StatusData const& rhs);

    struct __attribute__ ((__packed__)) UTCTime
    {
      static uint8_t const ValidTimeOfWeek = 0x01;
      static uint8_t const ValidWeek       = 0x02;
      static uint8_t const ValidUTC        = 0x04;

      uint32_t nanoSeconds;
      uint16_t year;
      uint8_t  month;
      uint8_t  day;
      uint8_t  hour;
      uint8_t  minute;
      uint8_t  seconds;
      uint8_t  valid;

      //ACE_Time_Value timestamp() const;
    };
    std::ostream& operator<<(std::ostream& ostr, UTCTime const& rhs);
    char const * operator>>(char const *& istr, UTCTime& rhs);

    struct __attribute__ ((__packed__)) Scenario : public UInt16Data
    {
      static uint16_t const MTiG_General         = 1;
      static uint16_t const MTiG_Automotiv       = 2;
      static uint16_t const MTiG_Aerospace       = 3;
      static uint16_t const MTiG_GeneralNoBaro   = 9;
      static uint16_t const MTiG_AutomotivNoBaro = 10;
      static uint16_t const MTiG_AerospaceNoBaro = 11;
      static uint16_t const MTiG_Marine          = 17;

      Scenario() {}
      Scenario(uint16_t d) : UInt16Data(d) {}
    };
    std::ostream& operator<<(std::ostream& ostr, Scenario const& rhs);
  }
}
#endif // kn_XSensData_h
