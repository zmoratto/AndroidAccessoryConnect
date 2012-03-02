#warning "XXX FIXME XXX this is merely a stub to allow compilation on OSX"
#warning "was there a serialstream file for OSX that wasn't added to roversw svn?"

vector<string>
SerialStream::candidatePortnames()
{
  char name[20];
  vector<string> ret;

  for (int i= 0; i< 64; i++) {
    sprintf(name, "/dev/ttyS%d", i); // Linux
    ret.push_back(name);
  }
  return ret;
}


