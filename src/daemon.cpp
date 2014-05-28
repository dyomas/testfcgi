#include <string.h>
#include <algorithm>
#include "daemon.h"
#include "libev_wrapper.h"


using namespace std;

Daemon::Daemon(LibevWrapper &ev)
  : SocketAsyncTCPServer(ev)
  , BufferedSender(ev)
{
}

void Daemon::sent(const size_t len)
{
  cout
    << "~ " << len << " bytes" << endl
  ;
}

void Daemon::finished()
{
  cout
    << "~ <all>" << endl
  ;
}

void Daemon::connected(const int fd)
{
  string descr;
  SocketAsyncBase::describe_socket(descr, fd);
  cout
    << "+ " << fd << ' ' << descr << endl
  ;
}

void Daemon::disconnected(const int fd, const SocketAsyncBase::disconnector dc)
{
  cout
    << "- " << fd << ", " << dc << endl
  ;
}

void Daemon::received(const int fd, const char *data, const size_t len)
{
  const string s(data, len);
  cout
    << "> " << fd << ", " << s << endl
  ;

  string sr(len, 0);
  reverse_copy(s.begin(), s.end(), sr.begin());
  send(fd, sr.c_str(), sr.length());
}

void Daemon::error(const int fd, const SocketAsyncBase::errorSource es, const int err)
{
  cout
    << "* " << fd << ", " << es << ", " << err << strerror(err) << endl
  ;
}
