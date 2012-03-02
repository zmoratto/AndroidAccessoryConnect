#include "XSensData.h"

#include <iostream>
#include <cmath>

#include <stdio.h>
#include <cstdlib>
#include <string.h>

using namespace std;

namespace
{
  using namespace kn::XSensData;

  struct BitNames8
  {
    uint8_t bit;
    char const * name;
  };

  struct BitNames16
  {
    uint16_t bit;
    char const * name;
  };

  struct BitNames32
  {
    uint32_t bit;
    char const * name;
  };

  BitNames16 selftestBits[] = {
    { SelftestAck::AccX, "AccX" },
    { SelftestAck::AccY, "AccY" },
    { SelftestAck::AccZ, "AccZ" },
    { SelftestAck::GyrX, "GyrX" },
    { SelftestAck::GyrY, "GyrY" },
    { SelftestAck::GyrZ, "GyrZ" },
    { SelftestAck::MagX, "MagX" },
    { SelftestAck::MagY, "MagY" },
    { SelftestAck::MagZ, "MagZ" }
  };
  int const NumSelftestBits = sizeof(selftestBits) / sizeof(BitNames16);

  BitNames8 errorBits[] = {
    { Error::InvalidPeriod,    "InvalidPeriod" },
    { Error::InvalidMessage,   "InvalidMessage" },
    { Error::TimerOverflow,    "TimerOverflow" },
    { Error::InvalidBaudRate,  "InvalidBaudRate" },
    { Error::InvalidParameter, "InvalidParameter" }
  };
  int const NumErrorBits = sizeof(errorBits) / sizeof(BitNames8);

  BitNames16 errorModeBits[] = {
    { ErrorMode::Ignore,  "Ignore" },
    { ErrorMode::Counter, "Counter" },
    { ErrorMode::Message, "Message" },
    { ErrorMode::Config,  "Config" }
  };
  int const NumErrorModeBits = sizeof(errorModeBits) / sizeof(BitNames16);

  BitNames16 outputModeBits[] = {
    { OutputMode::TemperatureData, "TemperatureData" },
    { OutputMode::CalibratedData,  "CalibratedData" },
    { OutputMode::OrientationData, "OrientationData" },
    { OutputMode::AuxiliaryData,   "AuxiliaryData" },
    { OutputMode::PositionData,    "PositionData" },
    { OutputMode::VelocityData,    "VelocityData" },
    { OutputMode::StatusData,      "StatusData" },
    { OutputMode::GpsPvtData,      "GpsPvtData" },
    { OutputMode::RawInertiaData,  "RawInertiaData" }
  };
  int const NumOutputModeBits = sizeof(outputModeBits) / sizeof(BitNames16);

  BitNames32 outputSettingsBits[] = {
    { OutputSettings::SampleCounter,    "SampleCounter" },
    { OutputSettings::UTCTime,          "UTCTime" },
    { OutputSettings::RotEuler,         "RotEuler" },
    { OutputSettings::RotMatrix,        "RotMatrix" },
    { OutputSettings::CalibratedNoAcc,  "CalibratedNoAcc" },
    { OutputSettings::CalibratedNoGyr,  "CalibratedNoGyr" },
    { OutputSettings::CalibratedNoMag,  "CalibratedNoMag" },
    { OutputSettings::OutputFixed12_20, "OutputFixed12_20" },
    { OutputSettings::OutputFixed16_32, "OutputFixed16_32" },
    { OutputSettings::AuxDisable_1,     "AuxDisable_1" },
    { OutputSettings::AuxDisable_2,     "AuxDisable_2" },
    { OutputSettings::CooridnatesNed,   "CooridnatesNed" }
  };
  int const NumOutputSettingsBits = sizeof(outputSettingsBits) / sizeof(BitNames32);

  BitNames8 statusBits[] = {
    { StatusData::Selftest, "Selftest" },
    { StatusData::XkfValid, "XkfValid"}, 
    { StatusData::GpsFix,   "GpsFix" }
  };
  int const NumStatusBits = sizeof(statusBits) / sizeof(BitNames8);

  BitNames8 utcTimeBits[] = {
    { UTCTime::ValidTimeOfWeek, "ValidTimeOfWeek" },
    { UTCTime::ValidWeek,       "ValidWeek"}, 
    { UTCTime::ValidUTC,        "ValidUTC" }
  };
  int const NumUTCTimeBits = sizeof(utcTimeBits) / sizeof(BitNames8);

  BitNames8 baudrateBits[] = {
      { Baudrate::Baud4k8   , "Baud4k8" },
      { Baudrate::Baud9k6   , "Baud9k6" },
      { Baudrate::Baud14k4  , "Baud14k4" },
      { Baudrate::Baud19k2  , "Baud19k2" },
      { Baudrate::Baud28k8  , "Baud28k8" },
      { Baudrate::Baud38k4  , "Baud38k4" },
      { Baudrate::Baud57k6  , "Baud57k6" },
      { Baudrate::Baud115k2 , "Baud115k2" },
      { Baudrate::Baud230k4 , "Baud230k4" },
      { Baudrate::Baud460k8 , "Baud460k8" },
      { Baudrate::Baud921k6 , "Baud921k6" }
  };
  int const NumBaudrateBits = sizeof(baudrateBits) / sizeof(BitNames8);

  BitNames16 scenarioBits[] = {
    { Scenario::MTiG_General,         "MTiG_General" },
    { Scenario::MTiG_Automotiv,       "MTiG_Automotiv"}, 
    { Scenario::MTiG_Aerospace,       "MTiG_Aerospace"}, 
    { Scenario::MTiG_GeneralNoBaro,   "MTiG_GeneralNoBaro"}, 
    { Scenario::MTiG_AutomotivNoBaro, "MTiG_AutomotivNoBaro"}, 
    { Scenario::MTiG_AerospaceNoBaro, "MTiG_AerospaceNoBaro"}, 
    { Scenario::MTiG_Marine,          "MTiG_Marine"}
  };
  int const NumScenarioBits = sizeof(scenarioBits) / sizeof(BitNames16);
}

namespace kn
{
  namespace XSensData
  {
    using namespace std;

    std::ostream& operator<<(std::ostream& ostr, UInt16Data rhs) {
      ostr << rhs.value;
      return ostr;
    }
    std::ostream& operator<<(std::ostream& ostr, UInt32Data rhs) {
      ostr << rhs.value;
      return ostr;
    }
    std::ostream& operator<<(std::ostream& ostr, FloatData rhs) {
      ostr << rhs.value;
      return ostr;
    }
    std::ostream& operator<<(std::ostream& ostr, Float3Data const& rhs) {  
      ostr << "{" 
           << rhs.xyz[0] << ", " 
           << rhs.xyz[1] << ", " 
           << rhs.xyz[2] << "}";
      return ostr;
    }
    std::ostream& operator<<(std::ostream& ostr, OrientationEuler const& rhs) {
      ostr << "{" 
           << rhs.rpy[0] * 180./M_PI << ", " 
           << rhs.rpy[1] * 180./M_PI << ", " 
           << rhs.rpy[2] * 180./M_PI << "}";
      return ostr;
    }
    std::ostream& operator<<(std::ostream& ostr, OrientationQuaternion const& rhs) {
      ostr << "{" 
           << rhs.q[0] << ", " 
           << rhs.q[1] << ", " 
           << rhs.q[2] << ", " 
           << rhs.q[3] << "}";
      return ostr;
    }
    std::ostream& operator<<(std::ostream& ostr, OrientationMatrix const& rhs) {
      ostr << "{";
      for (int i = 0; i < 9; ++i) {
        if (i != 0)
          ostr << ", ";
        ostr << rhs.matrix[i];
      }
      ostr << "}";
      return ostr;
    }
    std::ostream& operator<<(std::ostream& ostr, ProductCode const& rhs) {
      char code[21];
      memset(code, 0, sizeof(code));
      strncpy(code, rhs.code, 20);
      ostr << code;
      return ostr;
    }
    std::ostream& operator<<(std::ostream& ostr, FirmwareRev const& rhs) {
      ostr << "{" << (int)rhs.major << ", " << (int)rhs.minor << ", " << (int)rhs.rev << "}";
      return ostr;
    }
    std::ostream& operator<<(std::ostream& ostr, SelftestAck const& rhs) {
      int bits = 0;
      cout << "[";
      for (int i = 0; i < NumSelftestBits; ++i) { 
        if (rhs.value & selftestBits[i].bit) {
          if (bits != 0)
            ostr << "|";
          ostr << selftestBits[i].name;
          ++bits;
        }
      }
      cout << "]";
      return ostr;
    }
    std::ostream& operator<<(std::ostream& ostr, Error const& rhs) {
      for (int i = 0; i < NumErrorBits; ++i) { 
        if (rhs.value == errorBits[i].bit) {
          ostr << errorBits[i].name;
          break;
        }
      }
      return ostr;
    }
    std::ostream& operator<<(std::ostream& ostr, Baudrate const& rhs) {
      ostr << rhs.value << ": ";
      for (int i = 0; i < NumBaudrateBits; ++i) { 
        if (rhs.value == baudrateBits[i].bit) {
          ostr << baudrateBits[i].name;
          break;
        }
      }
      return ostr;
    }
    std::ostream& operator<<(std::ostream& ostr, ErrorMode const& rhs) {
      for (int i = 0; i < NumErrorModeBits; ++i) { 
        if (rhs.value == errorModeBits[i].bit) {
          ostr << errorModeBits[i].name;
          break;
        }
      }
      return ostr;
    }  
    std::ostream& operator<<(std::ostream& ostr, Period const& rhs) {
      ostr << ((double)rhs.value / (double)Period::Sec2Period) << "s";
      return ostr;
    }
    std::ostream& operator<<(std::ostream& ostr, OutputSkipFactor const& rhs) {
      if (rhs.value == 0xffff)
        ostr << "Disabled";
      else 
        ostr << rhs.value;
        
      return ostr;
    }
    std::ostream& operator<<(std::ostream& ostr, OutputMode const& rhs) {
      int bits = 0;
      cout << "[";
      for (int i = 0; i < NumOutputModeBits; ++i) { 
        if (rhs.value & outputModeBits[i].bit) {
          if (bits != 0)
            ostr << "|";
          ostr << outputModeBits[i].name;
          ++bits;
        }
      }
      cout << "]";
      return ostr;
    }

    std::ostream& operator<<(std::ostream& ostr, OutputSettings const& rhs) {
      int bits = 0;
      cout << "[";
      for (int i = 0; i < NumOutputSettingsBits; ++i) { 
        if (rhs.value & outputSettingsBits[i].bit) {
          if (bits != 0)
            ostr << "|";
          ostr << outputSettingsBits[i].name;
          ++bits;
        }
      }
      cout << "]";
      return ostr;
    }

    std::ostream& operator<<(std::ostream& ostr, Configuration const& rhs) {
      ostr << "XSensData::Configuration={" << endl
           << "  masterDeviceID: " <<rhs. masterDeviceID << endl
           << "  samplingPeriod: " << rhs.samplingPeriod << endl
           << "  outputSkipFactor: " << rhs.outputSkipFactor << endl
           << "  syncInSettingsMode: " << rhs.syncInSettingsMode << endl
           << "  syncInSettingsSkipFactor: " << rhs.syncInSettingsSkipFactor << endl
           << "  syncInSettingsOffset: " << rhs.syncInSettingsOffset << endl
           << "  dateYear: " << rhs.dateYear << endl
           << "  dateMonth: " << rhs.dateMonth << endl
           << "  dateDay: " << rhs.dateDay << endl
           << "  timeHours: " << rhs.timeHours << endl
           << "  timeMinutes: " << rhs.timeMinutes << endl
           << "  timeSeconds: " << rhs.timeSeconds << endl
           << "  timeHSec: " << rhs.timeHSec << endl
           << "  numDevices: " << rhs.numDevices << endl
           << "  deviceID: " << rhs.deviceID << endl
           << "  dataLength: " << rhs.dataLength << endl
           << "  outputMode: " << rhs.outputMode << endl
           << "  outputSettings: " << rhs.outputSettings << endl
           << "}";

      return ostr;
    }
    std::ostream& operator<<(std::ostream& ostr, GpsPvtData const& rhs) {
      return ostr;
    }
    std::ostream& operator<<(std::ostream& ostr, CalibratedData const& rhs) {
      ostr << "{" << rhs.acc << ", " << rhs.gyr << ", " << rhs.mag << "}";
      return ostr;
    }
    std::ostream& operator<<(std::ostream& ostr, PositionData const& rhs) {
      ostr << "{" << rhs.lat << "deg, " << rhs.lon << "deg, " << rhs.alt << "m}";
      return ostr;
    }
    std::ostream& operator<<(std::ostream& ostr, StatusData const& rhs) {
      int bits = 0;
      ostr << "[";
      for (int i = 0; i < NumStatusBits; ++i) { 
        if (rhs.value & statusBits[i].bit) {
          if (bits != 0)
            ostr << "|";
          ostr << statusBits[i].name;
          ++bits;
        }
      }
      ostr << "]";
      return ostr;
    }
    std::ostream& operator<<(std::ostream& ostr, UTCTime const& rhs) {
      int bits = 0;
      ostr << '[';
      for (int i = 0; i < NumUTCTimeBits; ++i) { 
        if (rhs.valid == utcTimeBits[i].bit) {
          if (bits != 0)
            ostr << "|";
          ostr << utcTimeBits[i].name;
          ++bits;
        }
      }
      if (bits == 0) {
        ostr << "invalid";
      }
      ostr << "] ";

      if (true || (rhs.valid & UTCTime::ValidUTC)) {
        //ostr << Miro::timeString(rhs.timestamp());
      }

      return ostr;
    }
    std::ostream& operator<<(std::ostream& ostr, Scenario const& rhs) {
      ostr << rhs.value << ": ";
      for (int i = 0; i < NumScenarioBits; ++i) { 
        if (rhs.value == scenarioBits[i].bit) {
          ostr << scenarioBits[i].name;
          break;
        }
      }
      return ostr;
    }

    /*
    ACE_Time_Value
    UTCTime::timestamp() const
    {
      tm stamp;
      time_t t;

      stamp.tm_sec = seconds;
      stamp.tm_min = minute;
      stamp.tm_hour = hour;
      stamp.tm_mday = day;
      stamp.tm_mon = month - 1;
      stamp.tm_year = year - 1900;
      stamp.tm_isdst = 0;

      t = ACE_OS::mktime(&stamp);
      return ACE_Time_Value(t, nanoSeconds / 1000);
    }
    */

    char const * operator>>(char const *& istr, Configuration& rhs) {

      istr >> rhs.masterDeviceID;
      istr >> rhs.samplingPeriod;
      istr >> rhs.outputSkipFactor;

      //istr >> rhs.syncInSettingsMode;
      istr += 2 * sizeof(uint16_t);
      //istr >> rhs.syncInSettingsOffset;
      istr += sizeof(uint32_t);
      //istr >> rhs.dateYear;
      istr += sizeof(uint32_t);
      //istr >> rhs.dateMonth;
      istr += 6 * sizeof(uint16_t);
      istr += 32;
      istr += 32;

      istr >> rhs.numDevices;
      istr >> rhs.deviceID;
      istr >> rhs.dataLength;
      istr >> rhs.outputMode;
      istr >> rhs.outputSettings;
      istr += 8;

      return istr;
    }

    char const * operator>>(char const *& istr, GpsPvtData& rhs) {
#ifdef ACE_LITTLE_ENDIAN
      ACE_CDR::swap_2(&istr[0], reinterpret_cast<char *>(&rhs.press));
      rhs.bPrs = istr[2];
      ACE_CDR::swap_4_array(&istr[3], reinterpret_cast<char *>(&rhs.itow), 10);
      rhs.bGps = istr[43];
#else
      memcpy(&rhs, istr, sizeof(rhs));
#endif
      istr += sizeof(rhs);      
      return istr;
    }

    char const * operator>>(char const *& istr, CalibratedData& rhs) {
      istr >> rhs.acc;
      istr >> rhs.gyr;
      istr >> rhs.mag;

      return istr;
    }

    char const * operator>>(char const *& istr, PositionData& rhs) {
#ifdef ACE_LITTLE_ENDIAN
      ACE_CDR::swap_4_array(istr, reinterpret_cast<char *>(&rhs), 3);
#else
      memcpy(&rhs, istr, sizeof(rhs));
#endif
      istr += sizeof(rhs);      
      return istr;
    }
    char const * operator>>(char const *& istr, UTCTime& rhs) {
      memcpy(&rhs, istr, sizeof(rhs));
#ifdef ACE_LITTLE_ENDIAN
      ACE_CDR::swap_4(&istr[0], reinterpret_cast<char *>(&rhs.nanoSeconds));
      ACE_CDR::swap_2(&istr[4], reinterpret_cast<char *>(&rhs.year));
#endif
      istr += sizeof(rhs);      
      return istr;
    }
  } // namespace XSensData
} // namespace kn
