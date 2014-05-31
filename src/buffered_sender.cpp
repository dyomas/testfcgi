#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <iostream>
#include <string.h>

#include "buffered_sender.h"
#include "libev_wrapper.h"
#include "logger.h"


void BufferedSender::m_on_send(const int fd)
{
  size_t length;
  const char *pdata;

  while ((pdata = mBuffer.data(length)))
  {
    const ssize_t length_sent = ::send(fd, pdata, length, MSG_NOSIGNAL);
    const int errno_copy = errno;
    if (length_sent > 0)
    {
      mBuffer.advance(length_sent);
      sent(length_sent);
    }
    else if (length_sent < 0)
    {
      if (errno_copy != EAGAIN)
      {
        LOG_ERROR("\"send\" failed, " << errno_copy << ", \"" << strerror(errno_copy) << "\"");
        failure(errno_copy);
      }
      break;
    }
  }

  if (mBuffer.empty())
  {
    mEventLoop.stop_io_handler(m_write_handler);
    m_write_handler = NULL;
    finished();
  }
}

BufferedSender::BufferedSender(LibevWrapper &ev)
  : m_write_handler(NULL)
  , mEventLoop(ev)
{
}

BufferedSender::~BufferedSender()
{
  if (m_write_handler)
  {
    mEventLoop.stop_io_handler(m_write_handler);
  }
}

void BufferedSender::cash(const char *data, const size_t length)
{
  mBuffer.append(data, length);
}

void BufferedSender::send(const int fd, const char *data, const size_t length)
{
  if (mBuffer.empty())
  {
    const ssize_t length_sent = ::send(fd, data, length, MSG_NOSIGNAL);
    const int errno_copy = errno;
    if (length_sent > 0)
    {
      sent(length_sent);
      if (length_sent == length)
      {
        finished();
      }
    }
    if ((length_sent < 0 && errno_copy == EAGAIN || length_sent > 0 && length_sent < length))
    {
      m_write_handler = mEventLoop.init_io_writer(this, reinterpret_cast<LibevWrapper::cbm_io_t>(&BufferedSender::m_on_send), fd);
      const size_t offset = length_sent > 0 ? length_sent : 0;
      mBuffer.append(data + offset, length - offset);
    }
    else if (length_sent < 0)
    {
      LOG_ERROR("\"send\" failed, " << errno_copy << ", \"" << strerror(errno_copy) << "\"");
      failure(errno_copy);
    }
  }
  else
  {
    mBuffer.append(data, length);
  }
}

void BufferedSender::send_cashed(const int fd)
{
  size_t length;
  const char *pdata;

  while ((pdata = mBuffer.data(length)))
  {
    const ssize_t length_sent = ::send(fd, pdata, length, MSG_NOSIGNAL);
    const int errno_copy = errno;
    if (length_sent > 0)
    {
      mBuffer.advance(length_sent);
      sent(length_sent);
    }
    if (mBuffer.empty())
    {
      finished();
    }

    if ((length_sent < 0 && errno_copy == EAGAIN || length_sent > 0 && length_sent < length))
    {
      m_write_handler = mEventLoop.init_io_writer(this, reinterpret_cast<LibevWrapper::cbm_io_t>(&BufferedSender::m_on_send), fd);
      break;
    }
    else if (length_sent < 0)
    {
      LOG_ERROR("\"send\" failed, " << errno_copy << ", \"" << strerror(errno_copy) << "\"");
      failure(errno_copy);
      break;
    }
  }
}

void BufferedSender::cancel()
{
  if (m_write_handler)
  {
    mEventLoop.stop_io_handler(m_write_handler);
    m_write_handler = NULL;
  }
  mBuffer.clear();
}
