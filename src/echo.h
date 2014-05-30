#ifndef __echo_h__
#define __echo_h__


#include <stdint.h>
#include "socket_async_tcp_server.h"
#include "buffered_sender.h"


class Echo: public SocketAsyncTCPServer, private BufferedSender
{

public:
  Echo(LibevWrapper &ev);

  virtual void sent(const size_t);
  virtual void finished();

  virtual void connected(const int /*fd*/);
  virtual void disconnected(const int /*fd*/, const SocketAsyncBase::disconnector);
  virtual void received(const int /*fd*/, const char *, const size_t);
  virtual void error(const int /*fd*/, const SocketAsyncBase::errorSource es, const int /*err*/);

};

#endif //__echo_h__
