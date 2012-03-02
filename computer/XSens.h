/* -*- C++ -*- */
// File: XSens.h
// Description: A second XSens Hw interface header file
//   This interface is very simple for now.  Features can be added later
//   as needed.
//
// Author: Brendon Justin
// Date: 8/1/2011
// Based on code by: Ted Morse
// Date: 2/12/2009

#ifndef kn_XSens_h
#define kn_XSens_h

//#include "knXSens_Export.h"
#include "XSensData.h"
#include "XSensCmd.h"

#include "serialstream.h"

//#include <ace/OS_NS_unistd.h>

#include <string>
#include <iosfwd>


namespace kn 
{
  /** A class to get data from an XSens MTi-G.
   */
  class XSens
  {
  public:
    static char const * XSENS_DEFAULT_PORT;
    static int const XSENS_DEFAULT_BAUD = 115200;
    
    XSens(std::string const& portname = XSENS_DEFAULT_PORT, int baudRate = XSENS_DEFAULT_BAUD);
    
    void setDebug(bool flag) throw() { m_debug = flag; }
    int collectDeviceState();
    void dumpDeviceState(std::ostream& ostr);

    void configure();

    // xsens commands
    int goToConfig() throw();
    int goToMeasurement() throw() { return command(XSensCmd::GoToMeasurement); }
    //    int reset();
    int reqID() throw() { return command(XSensCmd::ReqDID); }
    int reqProductCode() throw() { return command(XSensCmd::ReqProductCode); }
    int reqFWRev() throw() { return command(XSensCmd::ReqFWRev); }
    int reqDataLength() throw() { return command(XSensCmd::ReqDataLength); }
    //    int runSelftest() throw() { return command(XSensCmd::RunSelftest); }
    //    int reqGpsStatus() throw() { return command(XSensCmd::ReqGPSStatus); }
    int reqBaudrate() throw() { return command(XSensCmd::ReqBaudrate); }
    //    int setBaudrate(XSensData::Baudrate const& data);
    int reqErrorMode() throw() { return command(XSensCmd::ReqErrorMode); }
    int setErrorMode(XSensData::ErrorMode const& data) throw() {
      return command(XSensCmd::SetErrorMode, data);
    }
    int reqConfiguration() throw() { return command(XSensCmd::ReqConfiguration); }
    int reqPeriod() throw() { return command(XSensCmd::ReqPeriod); }
    int setPeriod(XSensData::Period const& data) throw() { return command(XSensCmd::SetPeriod, data); }
    int reqOutputSkipFactor() throw() { return command(XSensCmd::ReqOutputSkipFactor); }
    int setOutputSkipFactor(XSensData::OutputSkipFactor const& data) throw() { 
      return command(XSensCmd::SetOutputSkipFactor, data);
    }
    int reqObjectAlignment() throw() { return command(XSensCmd::ReqObjectAlignment); }
    int setObjectAlignment(XSensData::ObjectAlignment const& data) throw() { 
      return command(XSensCmd::SetObjectAlignment, data); 
    }
    int reqOutputMode() throw() { return command(XSensCmd::ReqOutputMode); }
    int setOutputMode(XSensData::OutputMode const& data) throw() { return command(XSensCmd::SetOutputMode, data); }
    int reqOutputSetting() throw() { return command(XSensCmd::ReqOutputSettings); }
    int setOutputSettings(XSensData::OutputSettings const& data) throw() {
      return command(XSensCmd::SetOutputSettings, data);
    }
    //    int reqData();
    int reqMagneticDeclination() throw() { return command(XSensCmd::ReqMagneticDeclination); }
    int setMagneticDeclination(XSensData::MagneticDeclination const& data) throw() { 
      return command(XSensCmd::SetMagneticDeclination, data); 
    }
    //    int reqAvailableScenarios() throw() { return command(XSensCmd::ReqAvailableScenarios); }
    int reqCurrentScenario() throw() { return command(XSensCmd::ReqCurrentScenario); }
    int setCurrentScenario(XSensData::Scenario const& data) throw() {
      return command(XSensCmd::SetCurrentScenario, data);
    }
    int reqGravityMagnitude() throw() { return command(XSensCmd::ReqGravityMagnitudeAck); }
    int setGravityMagnitude(XSensData::GravityMagnitude const& data) {
      return command(XSensCmd::SetGravityMagnitude, data);
    }
    //    int reqProcessingFlags();
    //    int setProcessingFlags(XSensData::ProcessingFlags const& data);
    int reqLeverArmGps() throw() { return command(XSensCmd::ReqLeverArmGps); }
    int setLeverArmGps(XSensData::LeverArmGps const& data) throw() {
      return command(XSensCmd::SetLeverArmGps, data); 
    }
    int resetOrienation() throw() { return command(XSensCmd::ResetOrientation); }
    int reqUTCTime() throw() { return command(XSensCmd::ReqUTCTime); }

    /** Read the data off of the serial port */
    int readData();
    unsigned char parseData();
    /** Get the file descriptor that data will arrives on */
    //ACE_HANDLE getHandle();

    //ACE_Time_Value const& timestamp() { return m_timestamp; }

    // accessors to data
    XSensData::ProductCode const& productCode() const throw() { return m_productCode; }
    XSensData::FirmwareRev const& firmwareRev() const throw() { return m_firmwareRev; }
    XSensData::SelftestAck const& selftestAck() const throw() { return m_selftestAck; }
    XSensData::Error const& error() const throw() { return m_error; }
    XSensData::Baudrate const& baudrate() const throw() { return m_baudrate; }
    XSensData::ErrorMode const& errorMode() const throw() { return m_errorMode; }
    XSensData::Configuration const& config() const throw() { return m_config; }
    XSensData::ObjectAlignment const& objectAlignment() const throw() { return m_objectAlignment; }
    XSensData::MagneticDeclination const& magneticDeclination() const throw() { return m_magneticDeclination; }
    XSensData::Scenario currentScenario() const throw() { return m_currentScenario; }
    XSensData::GravityMagnitude gravityMagnitude() const throw() { return m_gravityMagnitude; }
    //    XSensData::ProcessingFlags processingFlags() const throw() { return m_processingFlags; }
    XSensData::LeverArmGps const& leverArmGps() const throw() { return m_leverArmGps; }

    // MT Data
    //    XSensData::RawInertialData () const throw() { return m_rawInertiaData; }
    XSensData::GpsPvtData const& gpsPvtData() const throw() { return m_gpsPvtData; }
    XSensData::TemperatureData const& temperatureData() const throw() { return m_temperatureData; }
    XSensData::CalibratedData const& calibratedData() const throw() { return m_calibratedData; }
    XSensData::OrientationEuler const& orientationData() const throw() { return m_orientationData; }
    //    XSensData::AuxiliaryData auxiliaryData() const throw() { return m_auxiliaryData; }
    XSensData::PositionData const& positionData() const throw() { return m_positionData; }
    XSensData::VelocityData const& velocityData() const throw() { return m_velocityData; }
    XSensData::StatusData const& statusData() const throw() { return m_statusData; }
    XSensData::SampleCounter const& sampleCounter() const throw() { return m_sampleCounter; }
    XSensData::UTCTime const& utcTime() const throw() { return m_utcTime; }


    /** Connect to the xsens device. If no port is specified, use the
     *  default.  Also configures the device and puts it in measurement
     *  mode.
     *
     *  Note: Uses two stop bits due to a timing issue, see note marked 
     *  "IMPORTANT" in function implementation.
     */
    bool connect(std::string const& port = XSENS_DEFAULT_PORT, int baud = XSENS_DEFAULT_BAUD);
    
    /** Disconnect from the xsens device */
    void disconnect();

    /** Look for xsens devices in the usb-serial file **/
    static int scan(std::string const& path, std::string & dev_path);

  private:
    static unsigned char const XSENS_PREAMBLE = 0xFA;
    static unsigned char const XSENS_BUS_IDENTIFIER = 0xFF;

    static unsigned int const XSENS_BUF_SIZE = 2047;
    static unsigned int const XSENS_MID_OFFSET = 2;
    static unsigned int const XSENS_DATA_OFFSET = 4;

    template<typename Parameters>
    int 
    command(uint8_t mid, Parameters const& params) throw() {
      unsigned int const MsgSize =  5 + sizeof(Parameters);
      char buffer[MsgSize];
      buffer[0] = XSENS_PREAMBLE;
      buffer[1] = XSENS_BUS_IDENTIFIER;
      buffer[2] = mid;
      buffer[3] = sizeof(Parameters);

      char * data = &buffer[XSENS_DATA_OFFSET];
      data << params;

      return command(buffer, MsgSize);
    }
    int command(uint8_t mid) throw();
    int command(char * buffer, int len) throw();
    int waitAck(uint8_t mid) throw();
    void copyToFront(char const * rdPtr);

    static bool correctChecksum(char *message, unsigned int messageSize);
    static void setChecksum(char *message, unsigned int messageSize);
    
    SerialStream m_serial;
    bool m_debug;
    bool m_streamSync;
    //ACE_Time_Value m_timestamp;
    char * m_wrtPtr;
    char m_buffer[XSENS_BUF_SIZE + 1];

    // xsens data instances
    XSensData::ProductCode m_productCode;
    XSensData::FirmwareRev m_firmwareRev;
    XSensData::SelftestAck m_selftestAck;
    XSensData::Error m_error;
    XSensData::Baudrate m_baudrate;
    XSensData::ErrorMode m_errorMode;
    XSensData::Configuration m_config;
    XSensData::ObjectAlignment m_objectAlignment;
    XSensData::MagneticDeclination m_magneticDeclination;
    XSensData::Scenario m_currentScenario;
    XSensData::GravityMagnitude m_gravityMagnitude;
    //    XSensData::ProcessingFlags m_processingFlags;
    XSensData::LeverArmGps m_leverArmGps;

    // MT Data
    //    XSensData::RawInertialData m_rawInertiaData;
    XSensData::GpsPvtData m_gpsPvtData;
    XSensData::TemperatureData m_temperatureData;
    XSensData::CalibratedData m_calibratedData;
    XSensData::OrientationEuler m_orientationData;
    //    XSensData::AuxiliaryData m_auxiliaryData;
    XSensData::PositionData m_positionData;
    XSensData::VelocityData m_velocityData;
    XSensData::StatusData m_statusData;
    XSensData::SampleCounter m_sampleCounter;
    XSensData::UTCTime m_utcTime;
  };
}

#endif // kn_XSens_h
