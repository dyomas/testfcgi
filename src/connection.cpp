#include <stdlib.h>
#include <string.h>
#include <iostream>

#include "connection.h"
#include "libev_wrapper.h"
#include "fastcgi_response.h"
#include "socket_async_tcp_server.h"
#include "libev_wrapper.h"
#include "logger.h"


using namespace std;

void Connection::m_response(ostream &os, const FCGIRequest &request)
{
  request.dump(os);

  const FCGIRequest::params_t::const_iterator iter = request.params().find("QUERY_STRING");
  if (iter != request.params().end())
  {
    const string query = iter->second;
    if (query.substr(0, 7) == "padding")
    {
      char *ep;
      const size_t cnt = strtoul(query.c_str() + 7, &ep, 10);
      for (size_t pos = 0; pos != cnt; pos ++)
      {
        os << ep;
      }
      os
        << endl
      ;
    }
    else
    {
      os
        << "Query: " << query << endl
      ;
    }
  }
}

Connection::Connection(const int fd, LibevWrapper &ev, SocketAsyncTCPServer &server)
  : BufferedSender(ev)
  , m_fd(fd)
  , m_server(server)
{
}

void Connection::failure(const int err)
{
  m_server.close(m_fd);
}

void Connection::sent(const size_t len)
{
  LOG_DEBUG("Sent " << len << " bytes to fd " << m_fd);
}

void Connection::finished()
{
  LOG_DEBUG("Sent finished");
  m_server.close(m_fd);
}

void Connection::received(const char *data, const size_t len)
{
  FCGIRequest request;
  try
  {
    if (m_input_buffer.length())
    {
      m_input_buffer.append(data, len);
      const uint8_t
          *begin = reinterpret_cast<const uint8_t *>(m_input_buffer.c_str())
        , *end = begin + m_input_buffer.length()
      ;
      request.parse(begin, end);
    }
    else
    {
      const uint8_t
          *begin = reinterpret_cast<const uint8_t *>(data)
        , *end = begin + len
      ;
      request.parse(begin, end);
    }
  }
  catch (const FCGIRequest::InsufficientData &ex)
  {
    LOG_DEBUG("Connection " << m_fd << ": " << ex.what() << ", increasing buffer");
    if (!m_input_buffer.length())
    {
      m_input_buffer.append(data, len);
    }
    return;
  }

  FCGIResponse response(this);
  response.setId(request.id());
  response.setHeader("Status: 200\r\nContent-type: text/html; charset=utf-8");
  ostringstream ostr;
  ostr
    << "<html>" << endl
    << "  <head>" << endl
    << "    <title>" << endl
    << "      Привет!" << endl
    << "    </title>" << endl
    << "  </head>" << endl
    << "  <body>" << endl
    << "    <pre>" << endl
  ;
  m_response(ostr, request);
  ostr
    << "    </pre>" << endl
    << "  </body>" << endl
    << "</html>" << endl
  ;
  response.setContent(ostr.str());
  response.writeOut(m_fd);
}
