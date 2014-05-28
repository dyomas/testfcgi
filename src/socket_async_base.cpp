#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
#include <fcntl.h>
#include <netdb.h>
#include <errno.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <sstream>
#include <stdexcept>

#include "socket_async_base.h"
#include "logger.h"


using namespace std;

const size_t
    SocketAsyncBase::buff_size_default = 1024
  , SocketAsyncBase::backlog_default = 1
;
size_t
    SocketAsyncBase::buff_size = SocketAsyncBase::buff_size_default
  , SocketAsyncBase::backlog = SocketAsyncBase::backlog_default
;

const string SocketAsyncBase::m_port_2_string(const uint16_t port)
{
  ostringstream ostr;
  ostr << port;
  return ostr.str();
}

int SocketAsyncBase::init_client_tcp_socket()
{
  bool error_happened = false;
  int fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  if (fd < 0)
  {
    const int errno_copy = errno;
    LOG_ERROR("\"socket\" failed, " << errno_copy << ", " << strerror(errno_copy));
    error_happened = true;
  }
  else
  {
    int res;
    if ((res = fcntl (fd, F_GETFL)) != -1)
    {
      res = fcntl (fd, F_SETFL, res | O_NONBLOCK);
    }

    if (res == -1)
    {
      const int errno_copy = errno;
      LOG_ERROR("\"fcntl\" failed, " << errno_copy << ", " << strerror(errno_copy));
      error_happened = true;
    }
  }

  if (error_happened && fd >= 0)
  {
    if (close(fd))
    {
      const int errno_copy = errno;
      LOG_ERROR("\"close\" failed, " << errno_copy << ", " << strerror(errno_copy));
    }
    fd = -1;
  }

  return fd;
}

struct addrinfo *SocketAsyncBase::init_client_tcp_socket(int &fd, const string &host, const string &port, const bool pure_ip)
{
  bool error_happened = false;
  struct addrinfo hints;
  int res;

  bzero(&hints, sizeof(hints));
  hints.ai_flags = pure_ip ? (AI_NUMERICHOST | AI_NUMERICSERV) : 0;
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;

  struct addrinfo *addr = NULL;
  if ((res = getaddrinfo(host.c_str(), port.c_str(), &hints, &addr)))
  {
    LOG_ERROR("\"getaddrinfo\" failed, " << res << ", " << gai_strerror(res));
    error_happened = true;
  }

  fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  if (fd < 0)
  {
    const int errno_copy = errno;
    LOG_ERROR("\"socket\" failed, " << errno_copy << ", " << strerror(errno_copy));
    error_happened = true;
  }
  else
  {
    if ((res = fcntl (fd, F_GETFL)) != -1)
    {
      res = fcntl (fd, F_SETFL, res | O_NONBLOCK);
    }

    if (res == -1)
    {
      const int errno_copy = errno;
      LOG_ERROR("\"fcntl\" failed, " << errno_copy << ", " << strerror(errno_copy));
      error_happened = true;
    }
  }

  if (error_happened)
  {
    if (addr)
    {
      freeaddrinfo(addr);
      addr = NULL;
    }
    if (fd >= 0)
    {
      if (close(fd))
      {
        const int errno_copy = errno;
        LOG_ERROR("\"close\" failed, " << errno_copy << ", " << strerror(errno_copy));
      }
      fd = -1;
    }
  }

  return addr;
}

int SocketAsyncBase::init_server_inet_socket(const uint16_t port)
{
  int res;
  int fd;
  struct addrinfo hints;
  bool error_happened = false;

  bzero(&hints, sizeof(hints));
  hints.ai_flags = AI_PASSIVE;
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;

  struct addrinfo *addr = NULL;
  if ((res = getaddrinfo(NULL, m_port_2_string(port).c_str(), &hints, &addr)))
  {
    LOG_ERROR("\"getaddrinfo\" failed, " << res << ", " << gai_strerror(res));
  }

  if ((fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) >= 0)
  {
    if ((res = fcntl (fd, F_GETFL)) != -1)
    {
      res = fcntl (fd, F_SETFL, res | O_NONBLOCK);
    }

    if (res == -1)
    {
      const int errno_copy = errno;
      LOG_ERROR("\"fcntl\" failed for fd " << fd << ", " << errno_copy << ", " << strerror(errno_copy));
      error_happened = true;
    }
  }
  else
  {
    const int errno_copy = errno;
    LOG_ERROR("\"socket\" failed for fd " << fd << ", " << errno_copy << ", " << strerror(errno_copy));
    error_happened = true;
  }

  if (!error_happened)
  {
    const int optval = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)))
    {
      const int errno_copy = errno;
      LOG_ERROR("\"setsockopt\" failed for fd " << fd << ", " << errno_copy << ", " << strerror(errno_copy));
      error_happened = true;
    }
  }

  if (!error_happened)
  {
    if (bind (fd, addr->ai_addr, addr->ai_addrlen))
    {
      const int errno_copy = errno;
      LOG_ERROR("\"bind\" failed for fd " << fd << ", " << errno_copy << ", " << strerror(errno_copy) << ", port " << port);
      error_happened = true;
    }
    else if (listen(fd, SocketAsyncBase::backlog))
    {
      const int errno_copy = errno;
      LOG_ERROR("\"listen\" failed for fd " << fd << ", " << errno_copy << ", " << strerror(errno_copy) << ", port " << port);
      error_happened = true;
    }
  }

  if (addr)
  {
    freeaddrinfo(addr);
    addr = NULL;
  }

  if (error_happened && fd >= 0)
  {
    if (close(fd))
    {
      const int errno_copy = errno;
      LOG_ERROR("\"close\" failed for fd " << fd << ", " << errno_copy << ", " << strerror(errno_copy));
    }
    fd = -1;
  }

  return fd;
}

int SocketAsyncBase::init_client_unix_socket()
{
  bool error_happened = false;
  int fd = -1;

  if ((fd = socket(AF_UNIX, SOCK_DGRAM, IPPROTO_IP)) < 0)
  {
    const int errno_copy = errno;
    LOG_ERROR("\"socket\" failed, " << errno_copy << ", " << strerror(errno_copy));
    error_happened = true;
  }
  else
  {
    int res;
    if ((res = fcntl (fd, F_GETFL)) != -1)
    {
      res = fcntl (fd, F_SETFL, res | O_NONBLOCK);
    }

    if (res == -1)
    {
      const int errno_copy = errno;
      LOG_ERROR("\"fcntl\" failed, " << errno_copy << ", " << strerror(errno_copy));
      error_happened = true;
    }
  }

  if (error_happened && fd >= 0)
  {
    if (close(fd))
    {
      const int errno_copy = errno;
      LOG_ERROR("\"close\" failed for fd " << fd << ", " << errno_copy << ", " << strerror(errno_copy));
    }
    fd = -1;
  }

  return fd;
}

int SocketAsyncBase::describe_socket(string &description, const int fd, const socketEnds se)
{
  const size_t buff_size = 40;
  char buff[buff_size];
  socklen_t namelen = buff_size;
  sockaddr *paddr = reinterpret_cast<sockaddr *>(&buff);

  int res = 0;
  if (se == seLocal)
  {
    res = getsockname(fd, paddr, &namelen);
  }
  else if (se == seRemote)
  {
    res = getpeername(fd, paddr, &namelen);
  }

  if (res)
  {
    return errno;
  }
  else
  {
    ostringstream ostr;
    char buff2[buff_size];
    const char *ret_val = inet_ntop(paddr->sa_family, &paddr->sa_data[2], buff2, buff_size);
    if (ret_val)
    {
      ostr << ret_val;
    }
    else
    {
      int errno_copy = errno;
      LOG_ERROR("\"inet_ntop\" failed for fd " << fd << ", " << errno_copy << ", " << strerror(errno_copy));
    }
    ostr << '#' << ntohs(*reinterpret_cast<uint16_t *>(&paddr->sa_data[0]));
    description = ostr.str();
  }
  return 0;
}

int SocketAsyncBase::describe_socket(string &description, const int fd)
{
  string remote_description;
  string local_description;
  int errno_copy = describe_socket(remote_description, fd, SocketAsyncBase::seRemote);
  if (!errno_copy)
  {
    errno_copy = describe_socket(local_description, fd, SocketAsyncBase::seLocal);
  }
  if (!errno_copy)
  {
    description = local_description + " -> " + remote_description;
  }
  return errno_copy;
}

ostream &operator << (ostream &os, const SocketAsyncBase::disconnector dcn)
{
  switch (dcn)
  {
  case SocketAsyncBase::dcnUnknown:
    os << "Unknown";
    break;
  case SocketAsyncBase::dcnSelf:
    os << "Self";
    break;
  case SocketAsyncBase::dcnRemote:
    os << "Remote";
    break;
  default:
    os << '<' << static_cast<int>(dcn) << '>';
  };
  return os;
}

ostream &operator << (ostream &os, const SocketAsyncBase::errorSource es)
{
  switch (es)
  {
  case SocketAsyncBase::esAccept:
    os << "accept";
    break;
  case SocketAsyncBase::esConnect:
    os << "connect";
    break;
  case SocketAsyncBase::esRead:
    os << "read";
    break;
  case SocketAsyncBase::esWrite:
    os << "write";
    break;
  case SocketAsyncBase::esClose:
    os << "close";
    break;
  default:
    os << '<' << static_cast<int>(es) << '>';
  };
  return os;
}
