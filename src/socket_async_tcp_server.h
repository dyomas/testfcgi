#ifndef __socket_async_tcp_server_h__
#define __socket_async_tcp_server_h__

#include "socket_async_base.h"
#include <string>
#include <map>


class LibevWrapper;

class SocketAsyncTCPServer
{
public:
  SocketAsyncTCPServer(LibevWrapper& eventLoop);
  ~SocketAsyncTCPServer();

  bool is_running() const;

  void startup_inet(const uint16_t port);

  void shutdown();
  bool close();
  bool close(const int /*fd*/);

  virtual void connected(const int /*fd*/) = 0;
  virtual void disconnected(const int /*fd*/, const SocketAsyncBase::disconnector) = 0;
  virtual void received(const int /*fd*/, const char *, const size_t) = 0;
  virtual void error(const int /*fd*/, const SocketAsyncBase::errorSource es, const int /*err*/) = 0;

protected:
  typedef std::map<int, void *> handlers_t;
  handlers_t m_handlers;
  LibevWrapper &m_ev;

private:
  void close(handlers_t::const_iterator iter);
  void m_on_receive_client(const int);
  void m_on_receive_server(const int);
  bool m_running;
  int m_fd_main;
};

#endif //__socket_async_tcp_server_h__
