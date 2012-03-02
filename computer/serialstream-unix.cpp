// Copyright (c) 2001 Randy Sargent
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without 
// modification, are permitted provided that the following conditions are 
// met:
//
// Redistributions of source code must retain the above copyright notice, 
// this list of conditions and the following disclaimer. 
//
// Redistributions in binary form must reproduce the above copyright notice, 
// this list of conditions and the following disclaimer in the documentation 
// and/or other materials provided with the distribution. 
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED 
// TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR 
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE 
// LIABLE FOR ANY DIRECT, INDIRECT, INIDENTAL, SPECIAL, EXEMPLARY, OR 
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF 
// THE POSSIBILITY OF SUCH DAMAGE.

// included from serialstream.cpp
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#ifdef linux
#include <linux/serial.h>
#endif
#ifdef __sun__
#include <strings.h>
#endif
#include <sys/ioctl.h>
#include <sys/time.h>
#include <errno.h>

#include <fstream>
#include <iostream>

using namespace std;

void
SerialStream::msleep(int msec)
{
  usleep(msec*1000);
}


namespace SerialStreamHelper {
  string findBetween(const string &line,
                     const string &before,
                     const string &after)
  {
    string::size_type before_idx= line.find(before);
    if (before_idx == line.npos) return "";
    before_idx += before.size();
    string::size_type after_idx= line.find(after, before_idx);
    if (after_idx == line.npos) return line.substr(before_idx);
    return line.substr(before_idx, after_idx-before_idx);
  }
  
  string decodePortname(const string &portname)
  {
    int debug= 1;
    vector<string> tokens;
    string tmp= portname;
    while (tmp.find(':') != tmp.npos) {
      tokens.push_back(tmp.substr(0, tmp.find(':')));
      tmp= tmp.substr(tmp.find(':')+1, tmp.npos);
    }
    tokens.push_back(tmp);
    if (tokens.size() < 2 || tokens[0] != "/dev/ttyUSB") return portname;
    
    ifstream in("/var/local/usb-serial");
    if (!in.good()) {
      fprintf(stderr,
	      "Can't open /var/local/usb-serial to decode portname '%s'",
	      portname.c_str());
      fprintf(stderr, "Trying /var/local/usb-serial\n");
      in.close();
      in.clear();
      in.open("/var/local/usb-serial");
      if (!in.good()) {
	fprintf(stderr, "Can't open /var/local/usb-serial either - bailing\n");
	return (portname);		// failed, return the orginal name
      }
    }
    
    string name= tokens.size() >= 2 ? tokens[1] : "";
    string path= tokens.size() >= 3 ? tokens[2] : "";
    string port= tokens.size() >= 4 ? tokens[3] : "";
    
    if (debug>1) fprintf(stderr,
                         "Looking for name= '%s', path= '%s', port= '%s'\n",
                         name.c_str(), path.c_str(), port.c_str());
    
    while (in.good()) {
      string line;
      // Get line
      while (1) {
        int c= in.get();
        if (c == EOF || c == '\n') break;
        line.append(1,(char)c);
      }
      //cout << "Line= " << line << "\n";
      string ttyno= line.substr(0, line.find(':'));
      
      string this_name= findBetween(line, "name:\"", "\"");
      string this_path= findBetween(line, "path:", " ");
      string this_port= findBetween(line, "port:", " ");
      
      if (debug>1)
        cout << "  name="<< this_name << ", path=" << this_path << ", port=" <<
          this_port << "\n";
      
      // Check that name is contained in this_name
      if (this_name.find(name) == this_name.npos) continue;
      
      // Check that path matches last part of this_path
      if (path.size() > this_path.size()) continue;
      if (this_path.substr(this_path.size()-path.size()) != path) continue;
      
      // Check that port is equal
      if (port != this_port) continue;
      
      string ret= "/dev/ttyUSB" + ttyno;
      if (debug) {
        fprintf(stderr, "'%s' resolves to '%s'\n",
                portname.c_str(), ret.c_str());
      }
      return ret;
    }
    fprintf(stderr, "Couldn't find serial port in /var/local/usb-serial:\n");
    fprintf(stderr, "Requested port name: '%s'\n", portname.c_str());
    fprintf(stderr, "Looked in /var/local/usb-serial for the following:\n");
    fprintf(stderr, "  'name:' containing '%s'\n", name.c_str());
    fprintf(stderr, "  'path:' ending with '%s'\n", path.c_str());
    fprintf(stderr, "  'port:' equals '%s'\n", port.c_str());
    return portname;
  }

  int tcpConnect(const string &hostname, int portnum) {
    int stream=socket(AF_INET,SOCK_STREAM,0);
    if (stream < 0) {
      fprintf(stderr, "Couldn't create socket\n");
      return -1;
    }
    
    struct hostent *host;
    host= gethostbyname(hostname.c_str());
    if (!host) {
      fprintf(stderr, "Couldn't resolve host %s\n", hostname.c_str());
      return -1;
    }
    
    struct sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    sin.sin_family=AF_INET;
    sin.sin_port= htons(portnum);
    sin.sin_addr.s_addr = ((struct in_addr *)(host->h_addr))->s_addr;
    sin.sin_addr.s_addr = ((struct in_addr *)(host->h_addr))->s_addr;
    

    if (::connect(stream, (sockaddr*)&sin, sizeof(sin)) < 0) {
      fprintf(stderr, "Couldn't connect socket to remote host\n");
      return -1;
    }

    return stream;
  }

  bool decodeTCPPortname(const string &portname, string &hostname,
                         int &portnum) {
    if (portname.length() > 4 && portname.substr(0,4) == "tcp:") {
      unsigned int delimpos= portname.find(':', 4);
      if (delimpos == string::npos) {
        fprintf(stderr, "ERROR: port should be of form 'tcp:hostname:port'\n");
        return false;
      }
      hostname= portname.substr(4, delimpos-4);
      portnum= atoi(portname.substr(delimpos+1).c_str());
      return true;
    }
    return false;
  }
}

using namespace SerialStreamHelper;

// Linux-specific
void
SerialStream::osOpen()
{
  string decoded_portname= decodePortname(portname);
  // Is this a network-connected port?
  string hostname;
  int portnum;
  if (decodeTCPPortname(portname, hostname, portnum)) {
    stream= tcpConnect(hostname, portnum);
    if (stream < 0) return;
//  if (stream < 0) THROW SerialStreamError(SerialStreamError::IO_ERROR);
    msleep(500);			// why is this here???
    streamValid= 1;
  } else {
    int begin=mtime();
    debugf("%d: about to open\n", mtime()-begin);
#if defined(O_NDELAY)
    /* Open with NDELAY to make sure we don't hang
       because of handshaking lines! */
    stream= open(decoded_portname.c_str(), O_RDWR | O_NDELAY);
    if (stream < 0) {
	if (debug) {
	    fprintf(stderr, "Error opening '%s': %d\n",
		    decoded_portname.c_str(), stream);
	}
	THROW SerialStreamError(SerialStreamError::IO_ERROR);
    }
    debugf("%d: about to take off O_NDELAY\n", mtime()-begin);
    /* Now take the O_NDELAY back off so we actually wait
       for buffers to flush when we do writes */
    fcntl(stream, F_SETFL, ~O_NDELAY & fcntl(stream, F_GETFL, 0));
#else
#error "first time this has been used.  be sure to test."
    stream= open(decoded_portname.c_str(), O_RDWR);
    if (stream < 0) {
	if (debug) {
	    fprintf(stderr, "Error opening '%s': %d\n",
		    decoded_portname.c_str(), stream);
	}
	THROW SerialStreamError(SerialStreamError::IO_ERROR);
    }
#endif
    debugf("%d: done\n", mtime()-begin);
    streamValid= 1;
  }
}

// Linux-specific
void
SerialStream::osClose()
{
  assert(streamValid);
  streamValid= 0;
  if (close(stream) == -1) {
      if (debug) {
	  fprintf(stderr, "Error closing fd %d\n", stream);
      }
      THROW SerialStreamError(SerialStreamError::IO_ERROR);
  }
}

// Unix-specific
// Returns -1 if unrecognized baud
int
ss_baud2B(int baud)
{
   switch (baud) {
    case 300: return B300;
    case 1200: return B1200;
    case 2400: return B2400;
    case 4800: return B4800;
    case 9600: return B9600;
#ifdef B19200
    case 19200: return B19200;
#else
    case 19200: return EXTA;
#endif
#ifdef B38400
    case 38400: return B38400;
#else
    case 38400: return EXTB;
#endif
#ifdef B57600
    case 57600: return B57600;
#endif
#ifdef B115200
    case 115200: return B115200;
#endif
#ifdef B230400
    case 230400: return B230400;
#endif
#ifdef B460800
    case 460800: return B460800;
#endif
    default:
      return -1;
   }
}
      
// Linux-specific

void
SerialStream::osUpdateProperties()
{
  assert(streamValid);

  string hostname;
  int portnum;
  if (decodeTCPPortname(portname, hostname, portnum)) {
    int ctlstream= tcpConnect(hostname, portnum+100);
    if (ctlstream < 0) return;
    
    char buf[100];
    sprintf(buf, "%d\n", baud);
    int current= 0, len= strlen(buf);
    while (current < len) {
      int nwrote= ::write(ctlstream, buf+current, len-current);
      if (nwrote > 0) current += nwrote;
    }
    msleep(500);			// why is this here??? flush?
    close(ctlstream);
  } else {
    struct termios tio;
    int baudb = ss_baud2B(baud);
    
    if (tcgetattr(stream, &tio) == -1)
      THROW SerialStreamError(SerialStreamError::IO_ERROR);
    
    tio.c_iflag= IGNBRK;
    tio.c_oflag= 0;
    tio.c_cflag= CREAD | CLOCAL;
    switch (nDataBits) {
    case 5: tio.c_cflag |= CS5; break;
    case 6: tio.c_cflag |= CS6; break;
    case 7: tio.c_cflag |= CS7; break;
    case 8: tio.c_cflag |= CS8; break;
    default:
      THROW SerialStreamError(SerialStreamError::ILLEGAL_ARG);
    }
    switch (nStopBits) {
    case 1: break;
    case 2: tio.c_cflag |= CSTOPB; break;
    default:
      THROW SerialStreamError(SerialStreamError::ILLEGAL_ARG);
    }
    switch (parity) {
    case 'e': case 'E': tio.c_cflag |= PARENB; break;
    case 'o': case 'O': tio.c_cflag |= PARENB | PARODD; break;
    case 'n': case 'N': break;
    default:
      THROW SerialStreamError(SerialStreamError::ILLEGAL_ARG);
    }
    tio.c_lflag= 0;
    bzero(&tio.c_cc, sizeof(tio.c_cc)); /* clear control characters */
#if defined(VMIN)
    tio.c_cc[VMIN]=1; // wait for at least one char when performing read
#endif    
    
    cfsetispeed(&tio, (baudb == -1 ? B38400 : baudb));
    cfsetospeed(&tio, (baudb == -1 ? B38400 : baudb));
    
    if (tcsetattr(stream, TCSADRAIN, &tio) == -1)
      THROW SerialStreamError(SerialStreamError::IO_ERROR);
  }
}

// Linux-specific
void
SerialStream::transmitBreakFor(int mseconds)
{
  if (!streamValid) THROW SerialStreamError(SerialStreamError::NOT_CONNECTED);
  int duration=(mseconds + 249) / 250;
  if (duration < 1) duration= 1;
  tcsendbreak(stream, duration);
  msleep(duration*500);
}

// Linux-specific
long long
ss_utime()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return ((long long) tv.tv_sec * (long long) 1000000 +
          (long long) tv.tv_usec);
}

// Linux-specific
int
SerialStream::mtime()
{
  return (int) (ss_utime() / 1000);
}
  
// Linux-specific
int
SerialStream::osRead(unsigned char *buf, int len, int timeout)
{
  assert(streamValid);
  
  long long end_time= ss_utime() + (long long) timeout * 1000;
  fd_set read_set, write_set, exceptional_set;
  int n_read= 0;

  while (1) {
    int nfound;
    while (1) {
      struct timeval timeval_timeout;
      long long u_timeout= end_time - ss_utime();
      if (u_timeout < 0) u_timeout= 0;
      timeval_timeout.tv_sec= (long) (u_timeout / 1000000);
      timeval_timeout.tv_usec= (long) (u_timeout % 1000000);
      
      FD_ZERO(&read_set);
      FD_ZERO(&write_set);
      FD_ZERO(&exceptional_set);
      
      FD_SET(stream, &read_set);
      FD_SET(stream, &exceptional_set);
      
      nfound= select(stream+1, &read_set, &write_set, &exceptional_set,
                         timeout < 0 ? 0 : &timeval_timeout);
      if (nfound < 0) {
        if (errno != EINTR && errno != EAGAIN) {
          if (g_serstrDebugErrors) perror("osRead/select");
          THROW SerialStreamError(SerialStreamError::IO_ERROR);
        }
      } else {
        break;
      }
    }

    if (nfound == 0) return n_read;
    if (FD_ISSET(stream, &exceptional_set)) {
      if (n_read > 0) return n_read;
      if (g_serstrDebugErrors)
        fprintf(stderr, "ERROR: Exceptional set in select\n");
      THROW SerialStreamError(SerialStreamError::IO_ERROR);
    }
    if (FD_ISSET(stream, &read_set)) {
      int n= ::read(stream, buf, len);
      if (n < 0) {
        if (errno != EINTR && errno != EAGAIN) {
          if (g_serstrDebugErrors) perror("osRead/read");
          THROW SerialStreamError(SerialStreamError::IO_ERROR);
        }
      } else {
        n_read += n;
        buf += n;
        len -= n;
        if (len <= 0) return n_read;
      }
    }
    if (ss_utime() > end_time) return n_read; // time expired
  }
}


// Linux-specific
void
SerialStream::osWrite(const unsigned char *buf, int len)
{
  assert(streamValid);

  while (len) {
    int n_written= ::write(stream, buf, len);
    if (n_written >= len) break;
    if (n_written >= 0) {
      buf += n_written;
      len -= n_written;
    } else if (errno != EAGAIN && errno != EINTR) {
      THROW SerialStreamError(SerialStreamError::IO_ERROR);
    }
  }
  return;
}


// Set DTR state. True means DTR high (positive)
void
SerialStream::setDTR(bool high)
{
  int i= TIOCM_DTR;
  if (ioctl(stream, high ? TIOCMBIS : TIOCMBIC, &i)) {
    THROW SerialStreamError(SerialStreamError::IO_ERROR);
  }
}

  
// Set RTS state. True means RTS high (positive)
void
SerialStream::setRTS(bool high)
{
  int i= TIOCM_RTS;
  if (ioctl(stream, high ? TIOCMBIS : TIOCMBIC, &i)) {
    THROW SerialStreamError(SerialStreamError::IO_ERROR);
  }
}

  

#if defined(__MACH__) && defined(__APPLE__)
#include "serialstream-candidate-portnames-darwin.cpp"
#else
// Linux-specific
vector<string>
SerialStream::candidatePortnames()
{
  char name[20];
  vector<string> ret;
    
  for (int i= 0; i< 64; i++) {
    sprintf(name, "/dev/ttyS%d", i); // Linux
    ret.push_back(name);
  }
  for (int c= 'a'; c<='z'; c++) {
    sprintf(name, "/dev/tty%c", c); // Sun
    ret.push_back(name);
  }
  return ret;
}
#endif

