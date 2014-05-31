#ifndef __daemon_h__
#define __daemon_h__


#include <stdint.h>
#include <map>

#include "socket_async_tcp_server.h"

class Connection;

class Daemon: public SocketAsyncTCPServer
{
  Daemon (const Daemon &);
  Daemon &operator= (const Daemon &);

  typedef std::map<int, Connection *> connections_t;
  connections_t m_connections;

public:
  Daemon(LibevWrapper &ev);
  ~Daemon();

  virtual void connected(const int /*fd*/);
  virtual void disconnected(const int /*fd*/, const SocketAsyncBase::disconnector);
  virtual void received(const int /*fd*/, const char *, const size_t);
  virtual void error(const int /*fd*/, const SocketAsyncBase::errorSource es, const int /*err*/);

};

#endif //__daemon_h__
