#include <unistd.h>
#include <string.h>
#include <sstream>

#include "logger.h"
#include "fastcgi_response.h"
#include "buffered_sender.h"

using namespace std;

const uint16_t FCGIResponse::m_max_data_length = 8000;

template <class T> inline void FCGIResponse::m_write(const int socket, const T &src)
{
  typename T::const_iterator iter = src.begin();
  const typename T::const_iterator iter_end = src.end();
  typename T::size_type data_portion_len;

  while (iter != iter_end)
  {
    data_portion_len = iter_end - iter;
    if (data_portion_len > m_max_data_length)
    {
      data_portion_len = m_max_data_length;
    }

    const size_t padding = (data_portion_len % 8) ? (8 - data_portion_len % 8) : 0;
    m_fcgi_header.contentLengthB1 = data_portion_len >> 8;
    m_fcgi_header.contentLengthB0 = data_portion_len & 0xff;
    m_fcgi_header.paddingLength = padding;

    m_sender->cash(reinterpret_cast<const char *>(&m_fcgi_header), sizeof(FCGI_Header));
    if (padding)
    {
      string tmp(reinterpret_cast<const char *>(&*iter), data_portion_len);
      tmp.append(padding, '\0');
      m_sender->cash(tmp.c_str(), tmp.length());
    }
    else
    {
      m_sender->cash(reinterpret_cast<const char *>(&*iter), data_portion_len);
    }

    iter += data_portion_len;
  }
}

void FCGIResponse::setId(const uint16_t id)
{
  m_id = id;
}

void FCGIResponse::setAppStatus(const int status)
{
  m_status = status;
}

void FCGIResponse::setHeader(const string &str)
{
  m_str1 = str;
  m_str1 += "\r\n\r\n";
  m_source_type = sourceType(m_source_type & (stContentGenerator | stContentFile | stContentString) | stHeaderString);
}

void FCGIResponse::setHeader(const ostringstream &ostr)
{
  m_str1 = ostr.str();
  m_str1 += "\r\n\r\n";
  m_source_type = sourceType(m_source_type & (stContentGenerator | stContentFile | stContentString) | stHeaderString);
}

void FCGIResponse::setContent(const string &str)
{
  m_str2 = str;
  m_source_type = sourceType(m_source_type & (stHeaderGenerator | stHeaderFile | stHeaderString) | stContentString);
}

void FCGIResponse::setContent(const ostringstream &ostr)
{
  m_str2 = ostr.str();
  m_source_type = sourceType(m_source_type & (stHeaderGenerator | stHeaderFile | stHeaderString) | stContentString);
}

void FCGIResponse::setError(const string &str)
{
  m_error = str;
}

void FCGIResponse::setError(const ostringstream &ostr)
{
  m_error = ostr.str();
}

FCGIResponse & FCGIResponse::operator= (const string &str)
{
  m_str1 = str;
  m_source_type = stRaw;
  return *this;
}

FCGIResponse & FCGIResponse::operator= (const ostringstream &ostr)
{
  m_str1 = ostr.str();
  m_source_type = stRaw;
  return *this;
}

void FCGIResponse::writeOut(const int socket)
{
  m_fcgi_header.requestIdB1 = (m_id >> 8);
  m_fcgi_header.requestIdB0 = (m_id & 0xff);

  if (m_error.length())
  {
    m_fcgi_header.type = FCGI_STDERR;
    m_write(socket, m_error);
  }

  if (m_source_type != stNo)
  {
    m_fcgi_header.type = FCGI_STDOUT;
    if (m_source_type == stRaw)
    {
      m_write(socket, m_str1);
    }
    else
    {
      if (m_source_type & stHeaderString)
      {
        m_write(socket, m_str1);
      }

      if (m_source_type & stContentString)
      {
        m_write(socket, m_str2);
      }
    }
  }

  m_fcgi_header.type = FCGI_END_REQUEST;
  m_fcgi_header.contentLengthB1 = 0;
  m_fcgi_header.contentLengthB0 = sizeof(FCGI_EndRequestBody);
  m_fcgi_header.paddingLength = 0;

  m_end_request.appStatusB3 = (m_status >> 24) & 0xff;
  m_end_request.appStatusB2 = (m_status >> 16) & 0xff;
  m_end_request.appStatusB1 = (m_status >> 8) & 0xff;
  m_end_request.appStatusB0 = m_status & 0xff;
  m_end_request.protocolStatus = FCGI_REQUEST_COMPLETE; //{FCGI_CANT_MPX_CONN | FCGI_OVERLOADED | FCGI_UNKNOWN_ROLE}

  m_sender->cash(reinterpret_cast<const char *>(&m_fcgi_header), sizeof(FCGI_Header));
  m_sender->cash(reinterpret_cast<const char *>(&m_end_request), sizeof(FCGI_EndRequestBody));
  m_sender->send_cashed(socket);
}

FCGIResponse::FCGIResponse (BufferedSender *sender)
  : m_id(1)
  , m_status(0)
  , m_source_type(stNo)
  , m_sender(sender)
{
  bzero(&m_fcgi_header, sizeof(FCGI_Header));
  bzero(&m_end_request, sizeof(FCGI_EndRequestBody));
  bzero(&m_unknown_type, sizeof(FCGI_UnknownTypeBody));
  m_fcgi_header.version = FCGI_VERSION_1;
}
