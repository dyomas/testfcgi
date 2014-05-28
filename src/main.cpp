#include <errno.h>
#include <unistd.h>
#include <getopt.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <iostream>
#include <sstream>
#include <stdexcept>

#include "libev_wrapper.h"
#include "daemon.h"
#include "socket_async_base.h"

using namespace std;

const uint16_t port_default = 1024;
const size_t buff_size_default = 32;

bool _G_verbose = false;
uint16_t _G_port = port_default;

void usage(ostream &os, const char *me)
{
  os
    << "Test FastCGI daemon" << endl
    << "Usage: " << me << " [Options] [FILE]" << endl
    << "  -P - port, mode specific, default " << port_default << endl
    << "  -b - server buffer size, default " << SocketAsyncBase::buff_size_default << endl
    << "  -l - server backlog length, default " << SocketAsyncBase::backlog_default << endl
    << "  -v - verbose execution" << endl
    << "  -h - print this help and exit" << endl
  ;
}

static uint32_t c_string_to_uint(const char *src)
{
  int64_t ret_val = 0;

  char *ep;
  ret_val = strtoll(src, &ep, 10);
  const int errno_copy = errno;
  if (errno_copy)
  {
    ostringstream ostr;
    ostr << "Conversion failed from string \"" << src << "\" to uint32_t (";
    if (errno_copy)
    {
      ostr << errno_copy << ", ";
    }
    ostr << strerror(errno_copy) << "";
    if (*ep)
    {
      ostr << ", remainder " << ep;
    }
    throw runtime_error(ostr.str());
  }
}

void go()
{
  LibevWrapper ev;
  Daemon testSocketServer(ev);

  testSocketServer.startup_inet(_G_port);
  ev.run();
}

int main(int argc, char *argv[])
{
  int ret_val = EXIT_FAILURE;
  char ch;

  while ((ch = getopt(argc, argv, "P:b:l:vh")) != -1)
  {
    switch (ch)
    {
      case 'P':
        _G_port = c_string_to_uint(optarg);
        break;
      case 'b':
        SocketAsyncBase::buff_size = c_string_to_uint(optarg);
        break;
      case 'l':
        SocketAsyncBase::backlog = c_string_to_uint(optarg);
        break;
      case 'v':
        _G_verbose = true;
        break;
      case 'h':
      case '?':
      default:
        usage(cout, argv[0]);
        return ret_val;
    }
  }

  ret_val = EXIT_SUCCESS;
  go();
  return ret_val;
}
