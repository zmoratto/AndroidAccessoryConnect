// Copied from kvhTest in knKVH

#include "XSens.h"
#include "XSensCmd.h"

//#include "miro/TimeHelper.h"

//#include <ace/OS_NS_unistd.h>

#include <curses.h>

#include <iostream>
#include <sstream>
#include <cmath>
#include <errno.h>

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
using namespace std;
using namespace kn;

char const * port = kn::XSens::XSENS_DEFAULT_PORT;

int main(int argc, char* argv[])
{
  if (argc < 2) {
    cout << "Usage: " << argv[0] << " portname\n"
	 << "Ctrl-C or q to quit\n";
    exit(0);
  } 
  
  if (argc > 1) {
    port = argv[1];
  }

  cout << port << endl;

  XSens xsens(argv[1], 115200);

  cout << "collect state" << endl;
  int rc;
  if ((rc = xsens.collectDeviceState()) != 0) {
    cerr << "failed to obtain device state: " << rc << endl;
    if (rc == -2) {
      cerr << xsens.error() << endl;
    }
  }
  xsens.dumpDeviceState(cout);

  cout << "set output mode" << endl;
  // set connection settings
  xsens.setOutputMode(XSensData::OutputMode::TemperatureData |
                      XSensData::OutputMode::CalibratedData |
                      XSensData::OutputMode::OrientationData |
                      XSensData::OutputMode::PositionData |
                      XSensData::OutputMode::VelocityData |
                      XSensData::OutputMode::StatusData);
  cout << "set output settings" << endl;
  xsens.setOutputSettings(XSensData::OutputSettings::SampleCounter |
                          // XSensData::OutputSettings::UTCTime |
                          XSensData::OutputSettings::RotEuler |
                          XSensData::OutputSettings::AuxDisable_1 |
                          XSensData::OutputSettings::AuxDisable_2 |
                          XSensData::OutputSettings::CooridnatesNed);

  // Magnetic Declination
  xsens.setMagneticDeclination( 5. / 180. * M_PI);
  xsens.setCurrentScenario(2);
  XSensData::Float3Data d = { {0.f, 1.f, 2.f} };
  xsens.setLeverArmGps(d);
  xsens.setPeriod(0.01);
  xsens.setOutputSkipFactor(3); // 25 Hz
  XSensData::ObjectAlignment m = {
    { 1.f,  0.f,  0.f,
      0.f, -1.f,  0.f,
      0.f,  0.f, -1.f}
  };
  xsens.setObjectAlignment(m);

  cout << "collect state" << endl;
  if ((rc = xsens.collectDeviceState()) != 0) {
    cerr << "failed to obtain device state: " << rc << endl;
    if (rc == -2) {
      cerr << xsens.error() << endl;
    }
  }
  xsens.dumpDeviceState(cout);
  cout << endl << endl;
  cout << "sleep 3s before switching to measurement mode" << flush;
  for (int i = 0; i < 3; ++i) {
    //ACE_OS::sleep(ACE_Time_Value(1));
    sleep(3);
    cout << "." << flush;
  }
  cout << endl;

  // start measurement mode
  xsens.goToMeasurement();

  initscr();
  noecho();
  nodelay(stdscr, 1);
  keypad(stdscr, 1);
  raw();

  bool userQuit = false;
  while (!userQuit) {
    int rc = xsens.readData();
    if (rc == -1) {
      cerr << "error: " << strerror(errno) << endl;
      break;
    }
    if (rc == 0) {
      cerr << "." << flush;
      //ACE_OS::sleep(ACE_Time_Value::zero);
      sleep(0);
    }
    else {
      unsigned char cmd;
      while ((cmd = xsens.parseData()) != 0) {
        switch (cmd)
        {
        case XSensCmd::Error:
          cout << "error = " << xsens.error() << endl;
          break;
        case XSensCmd::MTData:
          if (xsens.config().dataLength.value > 0) {

            mvprintw(0,0,"Acceleration:\tX: %3.6f", xsens.calibratedData().acc.xyz[0]);
            mvprintw(1,0,"Acceleration:\tY: %3.6f", xsens.calibratedData().acc.xyz[1]);
            mvprintw(2,0,"Acceleration:\tZ: %3.6f", xsens.calibratedData().acc.xyz[2]);
            mvprintw(3,0,"Gyros:\t\tRoll: %3.6f", xsens.calibratedData().gyr.xyz[0]);
            mvprintw(4,0,"Gyros:\t\tPitch: %3.6f", xsens.calibratedData().gyr.xyz[1]);
            mvprintw(5,0,"Gyros:\t\tYaw: %3.6f", xsens.calibratedData().gyr.xyz[2]);
            mvprintw(6,0,"Magnetometers:\tX: %3.6f", xsens.calibratedData().mag.xyz[0]);
            mvprintw(7,0,"Magnetometers:\tY: %3.6f", xsens.calibratedData().mag.xyz[1]);
            mvprintw(8,0,"Magnetometers:\tZ: %3.6f", xsens.calibratedData().mag.xyz[2]);
            mvprintw(9,0,"Velocity:\tX: %3.6f", xsens.velocityData().xyz[0]);
            mvprintw(10,0,"Velocity:\tY: %3.6f", xsens.velocityData().xyz[1]);
            mvprintw(11,0,"Velocity:\tZ: %3.6f", xsens.velocityData().xyz[2]);
            //mvprintw(12,0,"Position:\tLat: %3.6f", xsens.positionData().lat);
            //mvprintw(13,0,"Position:\tLong: %3.6f", xsens.positionData().lon);
            //mvprintw(14,0,"Position:\tAlt: %3.6f", xsens.positionData().alt);
            mvprintw(15,0,"Orientation:\tRoll: %3.6f", xsens.orientationData().rpy[0]);
            mvprintw(16,0,"Orientation:\tPitch: %3.6f", xsens.orientationData().rpy[1]);
            mvprintw(17,0,"Orientation:\tYaw: %3.6f", xsens.orientationData().rpy[2]);
            mvprintw(18,0,"Temperature:\t%3.6f", xsens.temperatureData().value);
            stringstream status;
            status << xsens.statusData();
            mvprintw(19,0,"Status:\t%s", status.str().c_str());
            stringstream utcTime;
            utcTime << xsens.utcTime();
            mvprintw(20,0,"UTC time:\t%s", utcTime.str().c_str());
            //stringstream timestamp;
            //timestamp << xsens.timestamp();
            //mvprintw(21,0,"timestamp:\t%s", timestamp.str().c_str());
          }
          break;
        case 0xff:
          cout << "more parsing" << endl;
          break;
        default:
          cout << "cmd - 0x" << hex << (int)cmd << dec << endl;
        }

      }
    }
    int chr = getch();
    if (chr == 3 || chr == 'q')
      userQuit = true;
  }

  echo();
  endwin();
  
  xsens.goToConfig();
  cout << "collect state" << endl;
  if ((rc = xsens.collectDeviceState()) != 0) {
    cerr << "failed to obtain device state: " << rc << endl;
    if (rc == -2) {
      cerr << xsens.error() << endl;
    }
  }
  xsens.dumpDeviceState(cout);
  cout << endl << endl;


  return 0;
}
