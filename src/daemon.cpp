#include <string.h>
#include <iostream>
#include "daemon.h"

#include "libev_wrapper.h"
#include "connection.h"
#include "logger.h"


using namespace std;

Daemon::Daemon(LibevWrapper &ev, const string &data)
  : SocketAsyncTCPServer(ev)
  , m_storage(data)
{
}

Daemon::~Daemon()
{
  for (connections_t::const_iterator iter = m_connections.begin(), iter_end = m_connections.end(); iter != iter_end; iter ++)
  {
    delete iter->second;
  }
}

void Daemon::connected(const int fd)
{
  string descr;
  SocketAsyncBase::describe_socket(descr, fd);
  LOG_DEBUG("New connection on socket " << fd << ": " << descr);

  if (m_connections.count(fd))
  {
    LOG_ERROR("Connection on fd " << fd << " already exists, it's leaked!");
  }
  m_connections[fd] = new Connection(fd, m_ev, *this, m_storage);
}

void Daemon::disconnected(const int fd, const SocketAsyncBase::disconnector dc)
{
  LOG_DEBUG("Connection " << fd << " closed, " << dc);

  connections_t::iterator iter = m_connections.find(fd);
  if (iter != m_connections.end())
  {
    delete iter->second;
    m_connections.erase(fd);
  }
  else
  {
    LOG_ERROR("Connection to be disconnected on fd " << fd << " not found");
  }
}

void Daemon::received(const int fd, const char *data, const size_t len)
{
  LOG_DEBUG("Incoming " << len << " bytes on fd " << fd);
  connections_t::iterator iter = m_connections.find(fd);
  if (iter != m_connections.end())
  {
    try
    {
      iter->second->received(data, len);
    }
    catch (const exception &ex)
    {
      LOG_ERROR("Exception on fd " << fd << ": " << ex.what());
      close(fd);
    }
  }
  else
  {
    LOG_ERROR("Connection on fd " << fd << " not found, but receive data");
  }
}

void Daemon::error(const int fd, const SocketAsyncBase::errorSource es, const int err)
{
  LOG_ERROR("Error on fd " << fd << ", " << es << ", " << err << strerror(err));
}
