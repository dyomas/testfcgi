#ifndef __daemon_h__
#define __daemon_h__


#include <stdint.h>
#include "socket_async_tcp_server.h"
#include "buffered_sender.h"


class Daemon: public SocketAsyncTCPServer, private BufferedSender
{

public:
  Daemon(LibevWrapper &ev);

  virtual void sent(const size_t);
  virtual void finished();

  virtual void connected(const int /*fd*/);
  virtual void disconnected(const int /*fd*/, const SocketAsyncBase::disconnector);
  virtual void received(const int /*fd*/, const char *, const size_t);
  virtual void error(const int /*fd*/, const SocketAsyncBase::errorSource es, const int /*err*/);

};

#endif //__daemon_h__
