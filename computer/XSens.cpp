/*! Another class to read and write from the XSens MTi sensor */
//  IMPORTANT: See note about stop bits, marked "IMPORTANT"
//  See "TODO: unused data" comments for data received but not stored
//  Also see other "TODO:" comments

#include "XSens.h"
#include "XSensCmd.h"

//#include "miro/Log.h"

//#include <ace/OS_NS_sys_time.h>

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cstdio>
#include <cmath>
#include <errno.h>
#include <assert.h>

namespace
{
  char const XSENS_VENDOR_ID_STR[] = "0403";
  char const XSENS_PRODUCT_ID_STR[] = "d38b";
}
namespace kn
{
  using namespace std;

  char const * XSens::XSENS_DEFAULT_PORT = "/dev/ttyUSB0";

  XSens::XSens(std::string const& portname, int baudRate) :
    m_serial(), 
    m_debug(false),
    m_streamSync(false),
    m_wrtPtr(m_buffer)
  {
    // init the buffer
    memset(m_buffer, 0, XSENS_BUF_SIZE + 1);
    m_config.dataLength.value = 0;

    m_magneticDeclination.value = 180.;

    this->connect(portname, baudRate);
  }

  bool
  XSens::connect(std::string const& portname, int baudRate) 
  {
    try {
      if (!m_serial.connect(portname)) {
        //MIRO_LOG_OSTR(LL_CRITICAL, "Couldn't connect to port: " << portname << " " << baudRate);
        return false;
      }
    
      m_serial.setSpeed(baudRate);

      // IMPORTANT: SEE PAGE 55 OF MANUAL
      // 8 data bits, no parity, 2 stop bits
      // The device tested against needs 2 stop bits, but later devices 
      // need 1 stop bit.
      m_serial.setFormat(8, 'N', 2);
    }
    catch (SerialStreamError) {
      //MIRO_LOG_OSTR(LL_CRITICAL, "XSens - Couldn't connect to port: " << portname << " " << baudRate);
      return false;
    }

    // switch to config mode
    goToConfig();

    return true;
  }

  int
  XSens::collectDeviceState()
  {
    int rc = 0;
    do { // define common exit point
      if ((rc = reqProductCode()) < 0) {
        //MIRO_LOG(LL_ERROR, "XSens - Failed to issue ReqProductCode");
        break;
      }
      if ((rc = reqFWRev()) < 0) {
        //MIRO_LOG(LL_ERROR, "XSens - Failed to issue ReqFWRev");
        break;
      }
      // @FIXME: why does this not work?
//       if ((rc = runSelftest()) < 0) {
//         cout << "runSelftest" << endl;
//         break;
//       }
      if ((rc = reqErrorMode()) < 0) {
        //MIRO_LOG(LL_ERROR, "XSens - Failed to issue ReqErrorMode");
        break;
      }
      if ((rc = reqBaudrate()) < 0) {
        //MIRO_LOG(LL_ERROR, "XSens - Failed to issue ReqBaudrate");
        break;
      }
      if ((rc = reqObjectAlignment()) < 0) {
        //MIRO_LOG(LL_ERROR, "XSens - Failed to issue ReqObjectAlignment");
        break;
      }
      if ((rc = reqMagneticDeclination()) < 0) {
        //MIRO_LOG(LL_ERROR, "XSens - Failed to issue ReqMagneticDeclination");
        break;
      }
      
      if (false && m_firmwareRev.major >= 2) {
        if ((rc = reqGravityMagnitude()) < 0) {
          //MIRO_LOG(LL_ERROR, "XSens - Failed to issue ReqGravityMagnitude");
          break;
        }
      }
      if ((rc = reqCurrentScenario()) < 0) {
        //MIRO_LOG(LL_ERROR, "XSens - Failed to issue ReqCurrentScenario");
       break;
      }
      if ((rc = reqLeverArmGps()) < 0) {
        //MIRO_LOG(LL_ERROR, "XSens - Failed to issue ReqLeverArmGps");
        break;
      }
      if ((rc = reqConfiguration()) < 0) {
        //MIRO_LOG(LL_ERROR, "XSens - Failed to issue ReqConfiguration");
        break;
      }
    }
    while (false);
      
    return rc;
  }

  void
  XSens::dumpDeviceState(ostream& ostr)
  {
    ostr << "product code: " << productCode() << endl
         << "firmware revision: " << firmwareRev() << endl
      //         << "selftest results: " << selftestAck() << endl
         << "error mode: " << errorMode() << endl
         << "baudrate: " << baudrate() << endl
         << "object alignment: " << objectAlignment() << endl
         << "magnetic declination: " << (magneticDeclination().value / M_PI * 180.) << "deg" << endl;
    if (m_firmwareRev.major >= 2) {
      ostr << "gravity magnitude: " << gravityMagnitude() << endl;
    }
    ostr << "current scenario: " << currentScenario() << endl
         << "lever arm gps: " << leverArmGps() << endl
         << "configuration: " << endl << config();
  }

  void XSens::disconnect() 
  {
    m_serial.disconnect();
  }

  void XSens::configure()
  {
    // set connection settings
    setOutputMode(XSensData::OutputMode::TemperatureData |
		  XSensData::OutputMode::CalibratedData |
		  XSensData::OutputMode::OrientationData |
		  XSensData::OutputMode::PositionData |
		  XSensData::OutputMode::VelocityData |
		  XSensData::OutputMode::StatusData);
    cout << "set output settings" << endl;
    setOutputSettings(XSensData::OutputSettings::SampleCounter |
		      // XSensData::OutputSettings::UTCTime |
		      XSensData::OutputSettings::RotEuler |
		      XSensData::OutputSettings::AuxDisable_1 |
		      XSensData::OutputSettings::AuxDisable_2 |
		      XSensData::OutputSettings::CooridnatesNed);

    // Magnetic Declination
    setMagneticDeclination( 5. / 180. * M_PI);
    setCurrentScenario(2);
    XSensData::Float3Data d = { {0.f, 1.f, 2.f} };
    setLeverArmGps(d);
    setPeriod(0.01);
    setOutputSkipFactor(3); // 25 Hz
    XSensData::ObjectAlignment m = {
      { 1.f,  0.f,  0.f,
	0.f, -1.f,  0.f,
	0.f,  0.f, -1.f}
    };
    setObjectAlignment(m);
  }

  /*
  ACE_HANDLE
  XSens::getHandle() 
  {
    if (!m_serial.isConnected()) {
      //MIRO_LOG(LL_NOTICE, "kn::XSens::getHandle tried to getHandle, but not connected");
      return -1;
    }

    return m_serial.handle();
  }
  */

  int 
  XSens::goToConfig() throw() { 
    //  return command(XSensCmd::GoToConfig); 
    // the above is too simplistic:
    // the xsens actually aborts output in MeasurementMode right away
    // and spits out the ack 
    // this can lead to an incomplete MTData-Message been send
    // so we just read everything and parse for a GoToConfigAck from the back

    unsigned int const MsgSize =  5;
    char buffer[MsgSize];
    
    buffer[0] = XSENS_PREAMBLE;
    buffer[1] = XSENS_BUS_IDENTIFIER;
    buffer[2] = XSensCmd::GoToConfig;
    buffer[3] = 0;
    
    setChecksum(buffer, MsgSize);
    
    //int rc = ACE_OS::write_n(m_serial.handle(), buffer, MsgSize);
    int rc = 0;
    m_serial.write((const unsigned char *)buffer, MsgSize);
    if (rc > -1) {
      char const m[] = { 0xfa, 0xff, 0x31, 0x00, 0xd0 };
      // convert to string for easier matching
      string const match(m, sizeof(m));

      while ((rc = readData()) > 0) {

        string buffer(m_buffer, m_wrtPtr - m_buffer);
        unsigned int pos = buffer.rfind(match);
        if (pos != string::npos) {
          // copy to front
          copyToFront(m_buffer + pos + sizeof(m));
          m_streamSync = true;
          return 0;
        }
      }
      if (rc == 0) {
        errno = EPIPE;
        rc = -1;
      }
    }

    return rc;
  }


  /** Read (non-blocking) data from a buffer, and return the number of bytes read
   */
  int 
  XSens::readData()
  {
    int numBytes = 0;
    int remainingBufferSize = XSENS_BUF_SIZE - (m_wrtPtr - m_buffer);

    if (remainingBufferSize < 1) {
      /*
      //MIRO_LOG_OSTR(LL_CRITICAL,
		   "XSENS file descriptor read buffer overrun. Message of size > " << 
		   XSENS_BUF_SIZE);
      */
      m_wrtPtr = m_buffer;
      m_streamSync = false;
      return 0;
    }
    
    // whenever we for sure start a new message
    // we set the timestamp
    if (m_wrtPtr == m_buffer) {
      //m_timestamp = ACE_OS::gettimeofday();
    }

    //numBytes = ACE_OS::read(m_serial.handle(), m_wrtPtr, remainingBufferSize);
    numBytes = m_serial.readTimeout((unsigned char *) m_wrtPtr, remainingBufferSize, 50);

    if (numBytes == -1) {
      /*
      MIRO_LOG_OSTR(LL_CRITICAL,
		   "Error on XSENS file descriptor read: " << 
		   errno << ": " << strerror(errno));
      */
      return -1;
    }

    m_wrtPtr[numBytes] = 0; // NULL terminate

    // debug output
    if (m_debug && numBytes > 0) {
      stringstream ostr;
      ostr << "xsens: {";
      for (int i = 0; i < numBytes; ++i)
        if (false && isprint(m_wrtPtr[i])) 
          ostr << m_wrtPtr[i];
        else
          ostr << '[' << hex << (unsigned int)((unsigned char)m_wrtPtr[i]) << dec << ']';
      ostr << "}";
      cout << ostr.str() << endl;
      // MIRO_LOG(LL_PRATTLE, ostr.str().c_str());
    }

    // advance write pointer
    m_wrtPtr += numBytes;

    return numBytes;
  }


  namespace
  {
    template<typename T>
    void 
    copyFixedLen(T& dst, char const * src, int len)
    {
      assert(len == sizeof(T));
      src >> dst;
    }
    
    template<typename T>
    void copyIfNonZeorLen(T& dst, char const * src, int len)
    {
      assert(len == 0 || len == sizeof(T));
      if (len == sizeof(T)) {
        src >> dst;
      }
    }
  }


inline void endian_swap(unsigned short& x)
{
    x = (x>>8) | 
        (x<<8);
}

  /** Parses the data currently held in the buffer.
   * Returns true if a complete message was found, in which case that message is removed from the buffer
   */
  unsigned char 
  XSens::parseData()
  {
    namespace Cmd=XSensCmd;
    namespace Data=XSensData;

    unsigned char cmd = 0x00;

    char * first = m_buffer;
    char * last =  m_wrtPtr;

    // if we are not yet syncronized
    if (!m_streamSync) {
      // skip everything till we get a preamble/bid match
      for (; first != (last - 1); ++first) {
        if (((unsigned char)(*first) == 0xfa) && ((unsigned char)(*(first + 1)) == 0xff)) {
          m_streamSync = true;
          //MIRO_LOG(LL_NOTICE, "XSens - found stream sync.");
          break;
        }
      }
    }

    // the minimum message size is 5, so if the message is shorter than
    // 5 bytes, it is not a complete message
    unsigned int baseLength = 5;
    do { // define common bail-out point
      if (last - first < (int)baseLength) {
        // not a complete message
        break;
      }

      if (((unsigned char)(*first) != 0xfa) ||
          ((unsigned char)(*(first + 1)) != 0xff)) {
        //MIRO_LOG(LL_ERROR, "XSens - lost stream sync.");
        m_streamSync = false;
        cmd = 0xff;
        ++first;
        break;
      }
      
      unsigned char messageId = first[2];
      unsigned int len = first[3];

      if (len == 0xff) {
        uint16_t l;
        memcpy(&l, &first[3], sizeof(l));
        //l = ACE_NTOHS(l);
        //endian_swap(l);
        len = l;
        
        baseLength = 6;
      }
      
      if (last - first < (int)(baseLength + len)) {
        // not a complete message
        break;      
      }
      
      if (correctChecksum(first + 1, baseLength + len - 1)) { // process command
        
        // complete message with correct checksum!!!

        first += baseLength - 1; // go to data section
        
        // we have a return value
        cmd = messageId;

        switch (messageId) {
        case Cmd::WakeUp:
          // @TODO initialize!
          break;

          // replies with
          // no data
        case Cmd::GoToConfigAck:
        case Cmd::GoToMeasurementAck:
        case Cmd::ResetAck:
        case Cmd::ReqDataAck:
        case Cmd::ResetOrientationAck:
          break;
        case Cmd::DeviceID:
        case Cmd::InitBusResults:
          copyFixedLen(m_config.deviceID, first, len);
          break;
        case Cmd::ProductCode:
          assert(len <= sizeof(Data::ProductCode));
          memcpy(&m_productCode, first, len);
          m_productCode.code[len] = '\0';
          break;
        case Cmd::FirmwareRev:
          copyFixedLen(m_firmwareRev, first, len);
          break;
        case Cmd::DataLength:
          copyFixedLen(m_config.dataLength, first, len);
          break;
        case Cmd::SelftestAck:
          copyFixedLen(m_selftestAck, first, len);
          break;
        case Cmd::Error:
          copyFixedLen(m_error, first, len);
          break;
        case Cmd::GPSStatus:
          // @TODO: implement
          break;
        case Cmd::ReqBaudrateAck:
          copyIfNonZeorLen(m_baudrate, first, len);
          break;
        case Cmd::ReqErrorModeAck:
          copyIfNonZeorLen(m_errorMode, first, len);
          break;
        case Cmd::Configuration:
          copyFixedLen(m_config, first, len);
          break;
        case Cmd::ReqPeriodAck:
          copyIfNonZeorLen(m_config.samplingPeriod, first, len);
          break;
        case Cmd::ReqObjectAlignmentAck:
          copyIfNonZeorLen(m_objectAlignment, first, len);
          break;
        case Cmd::ReqOutputModeAck:
          copyIfNonZeorLen(m_config.outputMode, first,len);
          break;
        case Cmd::ReqOutputSettingsAck:
          copyIfNonZeorLen(m_config.outputSettings, first, len);
          break;
        case Cmd::ReqOutputSkipFactorAck:
          copyIfNonZeorLen(m_config.outputSkipFactor, first, len);
          break;
        case Cmd::MTData:
          assert(m_config.dataLength.value == 0 || len == m_config.dataLength.value);
          if (m_config.dataLength.value > 0) {
            
            char const * src = first;
            if (m_config.outputMode.value & Data::OutputMode::RawInertiaData) {
//               memcpy(&m_rawInertiaData, src, sizeof(Data::RawInertiaData));
//               m_rawInertiaData.n2h();
//               src += sizeof(Data::RawInertiaData);
            }
            if (m_config.outputMode.value & Data::OutputMode::GpsPvtData) {
              src >> m_gpsPvtData;
            }
            if (m_config.outputMode.value & Data::OutputMode::TemperatureData) {
              src >> m_temperatureData;
            }
            if (m_config.outputMode.value & Data::OutputMode::CalibratedData) {
              src >> m_calibratedData;
            }
            if (m_config.outputMode.value & Data::OutputMode::OrientationData) {
              // @TODO: support other precision modes
              src >> m_orientationData;
            }
            if (m_config.outputMode.value & Data::OutputMode::AuxiliaryData) {
              // @TODO: support suppressed aux channels
//               memcpy(&m_auxiliaryData, src, sizeof(Data::AuxiliaryData));
//               m_auxiliaryData.n2h();
//               src += sizeof(Data::AuxiliaryData);
            }
            if (m_config.outputMode.value & Data::OutputMode::PositionData) {
              src >> m_positionData;
            }
            if (m_config.outputMode.value & Data::OutputMode::VelocityData) {
              src >> m_velocityData;
            }
            if (m_config.outputMode.value & Data::OutputMode::StatusData) {
              src >> m_statusData;
            }
            if (m_config.outputSettings.value & Data::OutputSettings::SampleCounter) {
              src >> m_sampleCounter;
            }
            if (m_config.outputSettings.value & Data::OutputSettings::UTCTime) {
              src >> m_utcTime;
            }
          }
          break;
        case Cmd::ReqMagneticDeclinationAck:
          copyIfNonZeorLen(m_magneticDeclination, first, len);
          break;
        case Cmd::ReqAvailableScenariosAck:
          // @TODO: implement
          break;
        case Cmd::ReqCurrentScenarioAck:
          copyIfNonZeorLen(m_currentScenario, first, len);
          break;
        case Cmd::ReqGravityMagnitudeAck:
          copyIfNonZeorLen(m_gravityMagnitude, first, len);
          break;
        case Cmd::ReqProcessingFlagsAck:
          //          copyIfNonZeorLen(&m_processingFlags, first, len);
          break;
        case Cmd::ReqLeverArmGpsAck:
          copyIfNonZeorLen(m_leverArmGps, first, len);
          break;
        case Cmd::UTCTime:
          copyFixedLen(m_utcTime, first, len);
          break;
        default:
          //MIRO_LOG_OSTR(LL_ERROR, "XSens - Received unknown command id: " << hex << (int)messageId);
          cout << "XSens - Received unknown command id: " << hex << (int)messageId << endl;
        }
      

        first += len;
        // skip checksum
        ++first;
      }
      else {
        //MIRO_LOG(LL_ERROR, "XSens - checksum error");
        m_streamSync = false;
        cmd = 0xff;
        ++first;        
      }
    }
    while (false);
    
    // copy trailing message to the buffer front
    copyToFront(first);

    return cmd;
  }


  void
  XSens::copyToFront(char const * rdPtr)
  {
    assert (m_buffer <= rdPtr && rdPtr <= m_wrtPtr);

    if (rdPtr != m_buffer) {
      char * dest = m_buffer;
      char const * first = rdPtr;
      char const * last = m_wrtPtr;
      for (; first != last; ++first, ++dest) {
        *dest = *first;
      }
      m_wrtPtr = dest;

      // print buffer if there's some data left in there
      if (m_debug && m_wrtPtr != m_buffer) {
        stringstream ostr;
        ostr << "buffer: {";
        char const * l = m_wrtPtr;
        for (char const * f = m_buffer; f != l; ++f)
          ostr << '[' << hex << (unsigned int)((unsigned char)*f) << dec << ']';
        ostr << "}";
        cout << ostr.str() << endl;
      }
    }
  }

  int 
  XSens::command(uint8_t mid) throw()
  {
    unsigned int const MsgSize =  5;
    char buffer[MsgSize];
    
    buffer[0] = XSENS_PREAMBLE;
    buffer[1] = XSENS_BUS_IDENTIFIER;
    buffer[2] = mid;
    buffer[3] = 0;
    
    return command(buffer, MsgSize);
  }

  int 
  XSens::command(char * buffer, int len) throw()
  {
    setChecksum(buffer, len);

    // debug output
    if (m_debug) {
      stringstream ostr;
      ostr << "xsens-out: {";
      for (int i = 0; i < len; ++i)
          ostr << '[' << hex << (unsigned int)((unsigned char)buffer[i]) << dec << ']';
      ostr << "}";
      cout << ostr.str() << endl;
    }


    //int rc = ACE_OS::write_n(m_serial.handle(), buffer, len);
    int rc = 0;
    m_serial.write((const unsigned char *)buffer, len);

    if (rc > -1) {
      rc = waitAck(buffer[XSENS_MID_OFFSET]);
    }
    return rc;
  }


  // rc ==  0 - okay
  // rc == -1 - cerror
  // rc == -2 - xsens erro
  // zero read -> EPIPE  

  int 
  XSens::waitAck(uint8_t mid) throw()
  {
    int rc;
    while ((rc = readData()) > 0) {
      unsigned char cmd;
      while ((cmd = parseData()) != 0) {
        if (cmd == mid + 1) 
          return  0;
        if (cmd == XSensCmd::Error)
          return -2;
        if (cmd == 0xff)  { // lost sync
          continue;
        }

        // ignore trailing MTData packets
        assert (cmd == XSensCmd::MTData);
      }
        
      if (rc == 0) {
        errno = EPIPE;
        rc = -1;
      }
    }
    return rc;
  }

  /** Calculate and set the checksum of a message.
   *  For the XSens, this is the value such that, when all message
   *  bytes are added together, the lowest eight bits are all zeroes
   */
  void
  XSens::setChecksum(char *message, unsigned int messageSize)
  {
    int sum = 0;
    unsigned int i;
    for (i = 1; i < messageSize - 1; ++i) {
      sum += message[i];
    }

    message[i] = -(sum & 0xff);
  }

  bool
  XSens::correctChecksum(char *message, unsigned int messageSize)
  {
    int sum = 0;
    for (unsigned int i = 0; i < messageSize; ++i) {
      sum += message[i];
    }
    return (sum & 0xff) == 0;
  }

  int
  XSens::scan(std::string const& path, std::string & dev_path) {
    std::ifstream usb_in;
    usb_in.open(path.c_str());

    if (!usb_in) return -1;

    std::string line;
    while (usb_in.good()) {
      while (true) {
        int c = usb_in.get();
        if (c == EOF || c == '\n') break;
        line.append(1, (char) c);
      }

      if (!line.empty()) {
        int vendor_idx = line.find("vendor:");
        int product_idx = line.find("product:");
        std::string vendor_str = line.substr(vendor_idx+7, 4);
        std::string product_str = line.substr(product_idx+8, 4);
        std::string port_str = line.substr(0, line.find(':'));
        line.clear();

        if (!vendor_str.compare(XSENS_VENDOR_ID_STR)) {
          if (!product_str.compare(XSENS_PRODUCT_ID_STR)) {
            dev_path = std::string("/dev/ttyUSB") + port_str;
            return 0;
          }
        }
      }
    }
    
    return 1;
  }
}
