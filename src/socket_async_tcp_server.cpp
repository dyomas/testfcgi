#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <stdexcept>

#include "logger.h"
#include "libev_wrapper.h"
#include "socket_async_tcp_server.h"

using namespace std;

SocketAsyncTCPServer::SocketAsyncTCPServer(LibevWrapper &ev)
  : m_ev(ev)
  , m_running(false)
  , m_fd_main(-1)
{
}

SocketAsyncTCPServer::~SocketAsyncTCPServer()
{
  shutdown();
}

bool SocketAsyncTCPServer::is_running() const
{
  return m_running;
}

void SocketAsyncTCPServer::startup_inet(const uint16_t port)
{
  if (!m_running && (m_fd_main = SocketAsyncBase::init_server_inet_socket(port)) >= 0)
  {
    m_handlers[m_fd_main] = m_ev.init_io_reader(this, reinterpret_cast<LibevWrapper::cbm_io_t>(&SocketAsyncTCPServer::m_on_receive_server), m_fd_main);
    m_running = true;
  }
}

void SocketAsyncTCPServer::m_close(handlers_t::const_iterator iter)
{
  m_ev.stop_io_handler(iter->second);
  const int fd = iter->first;
  if (::close(fd))
  {
    const int errno_copy = errno;
    LOG_ERROR("\"close\" failed for fd " << fd << ", " << errno_copy << ", " << strerror(errno_copy));
  }
  if (fd == m_fd_main)
  {
    m_fd_main = -1;
  }
  else
  {
    disconnected(fd, SocketAsyncBase::dcnSelf);
  }
}

void SocketAsyncTCPServer::shutdown()
{
  for (handlers_t::const_iterator iter = m_handlers.begin(); iter != m_handlers.end(); ++iter)
  {
    m_close(iter);
  }
  m_handlers.clear();
  m_running = false;
}

bool SocketAsyncTCPServer::close()
{
  if (m_fd_main >= 0)
  {
    close(m_fd_main);
    return true;
  }
  return false;
}

bool SocketAsyncTCPServer::close(const int fd)
{
  handlers_t::const_iterator iter = m_handlers.find(fd);
  if (iter != m_handlers.end())
  {
    m_close(iter);
    m_handlers.erase(fd);
    return true;
  }
  return false;
}

void SocketAsyncTCPServer::m_on_receive_client(const int fd)
{
  char buff[SocketAsyncBase::buff_size];
  const ssize_t len = ::recv(fd, buff, SocketAsyncBase::buff_size, 0);
  const int errno_copy = errno;
  if (len < 0)
  {
    error(fd, SocketAsyncBase::esRead, errno_copy);
  }

  if (len > 0)
  {
    received(fd, buff, len);
  }
  else if (!len or (len < 0 and (errno_copy == EBADF || errno_copy == ECONNRESET || errno_copy == ENOTCONN || errno_copy == ENOTSOCK)))
  {
    m_ev.stop_io_handler(m_handlers.at(fd));
    m_handlers.erase(fd);
    if (::close(fd))
    {
      error(fd, SocketAsyncBase::esClose, errno);
    }
    if (fd != m_fd_main)
    {
      disconnected(fd, !len ? SocketAsyncBase::dcnRemote : SocketAsyncBase::dcnUnknown);
    }
    if (m_handlers.empty())
    {
      m_running = false;
    }
    if (fd == m_fd_main)
    {
      m_fd_main = -1;
    }
  }
}

void SocketAsyncTCPServer::m_on_receive_server(const int fd)
{
  const int fd_new = ::accept(fd, NULL, NULL);
  if (fd_new >= 0)
  {
    connected(fd_new);
    m_handlers[fd_new] = m_ev.init_io_reader(this, reinterpret_cast<LibevWrapper::cbm_io_t>(&SocketAsyncTCPServer::m_on_receive_client), fd_new);
  }
  else
  {
    error(fd, SocketAsyncBase::esAccept, errno);
  }
}
