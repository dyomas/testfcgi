#include <errno.h>
#include <unistd.h>
#include <getopt.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <stdexcept>

#include "libev_wrapper.h"
#include "daemon.h"
#include "echo.h"
#include "storage.h"
#include "searcher.h"
#include "socket_async_base.h"

using namespace std;
typedef vector<string> strings_t;

const uint16_t port_default = 1024;
const size_t buff_size_default = 32;
const string
    mode_default = "go"
  , data_part_default = "data"
;

bool _G_verbose = false;
uint16_t _G_port = port_default;

void usage(ostream &os, const char *me)
{
  os
    << "Test FastCGI daemon" << endl
    << "Usage: " << me << " [Options] [STRINGS]" << endl
    << "  -P - port, mode specific, default " << port_default << endl
    << "  -S - CSV-file with data to be searched" << endl
    << "  -b - server buffer size, default " << SocketAsyncBase::buff_size_default << endl
    << "  -l - server backlog length, default " << SocketAsyncBase::backlog_default << endl
    << "  -p - part of data to dump, default `" << data_part_default << "`" << endl
    << "    " << data_part_default << " - raw data as is, but structured " << endl
    << "    dict - lexemes list" << endl
    << "    index - coordinate information" << endl
    << "  -m - mode (test name), default `" << mode_default << "`:" << endl
    << "    " << mode_default << " - run daemon, (-Pbl)" << endl
    << "    echo - run simple echo client, (-Pbl)" << endl
    << "    dump - dump search data, (-Sp)" << endl
    << "    search - run searcher locally, (-S,<STRINGS>)" << endl
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
  Daemon daemon(ev);

  daemon.startup_inet(_G_port);
  ev.run();
}

void echo()
{
  LibevWrapper ev;
  Echo echo(ev);

  echo.startup_inet(_G_port);
  ev.run();
}

void dump(const string &data, const string &data_part)
{
  Storage storage(data);
  if (data_part == data_part_default)
  {
    storage.dump_data(cout);
  }
  else if (data_part == "dict")
  {
    storage.dump_dict(cout);
  }
  else if (data_part == "index")
  {
    storage.dump_index(cout);
  }
  else
  {
    throw runtime_error("Data part `" + data_part + "` invalid");
  }
}

void search(const strings_t &queries, const string &data)
{
  Storage storage(data);

  for (strings_t::const_iterator iter = queries.begin(), iter_end = queries.end(); iter != iter_end; iter ++)
  {
    Searcher searcher(storage, *iter);
    cout
      << searcher.lexemes_found() << '/' << searcher.lexemes_total() << endl
    ;
    searcher.search();
    searcher.dump_results(cout);
  }
}

int main(int argc, char *argv[])
{
  int ret_val = EXIT_FAILURE;
  char ch;
  string
      mode = mode_default
    , data_part = data_part_default
    , data
  ;
  strings_t strings;

  try
  {
    while ((ch = getopt(argc, argv, "m:P:S:b:l:p:vh")) != -1)
    {
      switch (ch)
      {
        case 'm':
          mode = optarg;
          break;
        case 'P':
          _G_port = c_string_to_uint(optarg);
          break;
        case 'S':
          data = optarg;
          break;
        case 'b':
          SocketAsyncBase::buff_size = c_string_to_uint(optarg);
          break;
        case 'l':
          SocketAsyncBase::backlog = c_string_to_uint(optarg);
          break;
        case 'p':
          data_part = optarg;
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

    while (optind < argc)
    {
      strings.push_back(argv[optind ++]);
    }

    if (mode == mode_default)
    {
      go();
    }
    else if (mode == "echo")
    {
      echo();
    }
    else if (mode == "dump")
    {
      dump(data, data_part);
    }
    else if (mode == "search")
    {
      search(strings, data);
    }
    else
    {
      ostringstream ostr;
      ostr << "Mode \"" << mode << "\" invalid";
      throw runtime_error(ostr.str());
    }
    ret_val = EXIT_SUCCESS;
  }
  catch (const exception &ex)
  {
    cerr
      << "*** std::exception! " << ex.what() << endl
    ;
  }

  return ret_val;
}
