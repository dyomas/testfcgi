#include <string.h>
#include <algorithm>
#include "echo.h"
#include "libev_wrapper.h"


using namespace std;

Echo::Echo(LibevWrapper &ev)
  : SocketAsyncTCPServer(ev)
  , BufferedSender(ev)
{
}

void Echo::failure(const int err)
{
  
}

void Echo::sent(const size_t len)
{
  cout
    << "~ " << len << " bytes" << endl
  ;
}

void Echo::finished()
{
  cout
    << "~ <all>" << endl
  ;
}

void Echo::connected(const int fd)
{
  string descr;
  SocketAsyncBase::describe_socket(descr, fd);
  cout
    << "+ " << fd << ' ' << descr << endl
  ;
}

void Echo::disconnected(const int fd, const SocketAsyncBase::disconnector dc)
{
  cout
    << "- " << fd << ", " << dc << endl
  ;
}

void Echo::received(const int fd, const char *data, const size_t len)
{
  const string s(data, len);
  cout
    << "> " << fd << ", " << s << endl
  ;

  string sr(len, 0);
  reverse_copy(s.begin(), s.end(), sr.begin());
  send(fd, sr.c_str(), sr.length());
}

void Echo::error(const int fd, const SocketAsyncBase::errorSource es, const int err)
{
  cout
    << "* " << fd << ", " << es << ", " << err << strerror(err) << endl
  ;
}
