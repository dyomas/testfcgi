#ifndef __connection_h__
#define __connection_h__


#include <stdint.h>
#include <string>
#include <ostream>

#include "buffered_sender.h"
#include "fastcgi_request.h"

class LibevWrapper;
class FCGIRequest;
class SocketAsyncTCPServer;
class Storage;

class Connection: private BufferedSender
{
  const int m_fd;
  SocketAsyncTCPServer &m_server;
  const Storage &m_storage;
  std::string m_input_buffer;

  void m_response(std::ostream &, const FCGIRequest &);

public:
  Connection(const int, LibevWrapper &ev, SocketAsyncTCPServer &, const Storage &);

  virtual void failure(const int);
  virtual void sent(const size_t);
  virtual void finished();

  void received(const char *data, const size_t len);
};

#endif //__connection_h__
