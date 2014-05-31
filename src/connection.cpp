#include <stdlib.h>
#include <string.h>
#include <iostream>

#include "connection.h"
#include "libev_wrapper.h"
#include "fastcgi_response.h"
#include "socket_async_tcp_server.h"
#include "libev_wrapper.h"
#include "storage.h"
#include "searcher.h"
#include "logger.h"


using namespace std;

bool h2c(char &res, const string &arg, const string::size_type pos)
{
  char ret_val;
  bool pair = false;
  unsigned char digit = arg[pos];

  while (true)
  {
    if (digit >= 'A' && digit <= 'F')
    {
      digit -= 'A' - '\x0A'; //\x37
    }
    else if (digit >= 'a' && digit <= 'f')
    {
      digit -= 'a' - '\x0A'; //\x57
    }
    else if (digit >= '0' && digit <= '9')
    {
      digit -= '0';
    }
    else
    {
      return false;
    }
    if (!pair)
    {
      ret_val = digit << 4;
      digit = arg[pos + 1U];
    }
    else
    {
      ret_val += digit;
      break;
    }
    pair = !pair;
  }

  res = ret_val;
  return true;
}

const string unescape(const string &arg)
{
  const string::size_type arg_length = arg.length();
  string::size_type
      pos = 0U
    , pos_prev = pos
  ;
  char symbol;
  string ret_val;

  while (true)
  {
    if ((pos = arg.find('%', pos_prev)) != string::npos)
    {
      ret_val.append(arg, pos_prev, pos - pos_prev);
      if (pos + 3U <= arg_length && h2c(symbol, arg, pos + 1U))
      {
        ret_val += (static_cast<uint8_t>(symbol) > '\x20' ? symbol : '\x20');
        pos_prev = pos + 3U;
      }
      else
      {
        ret_val += '%';
        pos_prev = pos + 1U;
      }
    }
    else
    {
      ret_val.append(arg.substr(pos_prev));
      break;
    }
  }
  return ret_val;
}

void Connection::m_response(ostream &os, const FCGIRequest &request)
{
  const FCGIRequest::params_t::const_iterator iter = request.params().find("QUERY_STRING");
  if (iter != request.params().end())
  {
    const string query = iter->second;
    if (query.substr(0, 7) == "padding")
    {
      request.dump(os);

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
      const string queryUnescaped = unescape(query);
      LOG_DEBUG("Query unescaped: `" << query << "` -> `" << queryUnescaped << "`");
      Searcher searcher(m_storage, queryUnescaped);
      searcher.search();
      if (searcher.results().size())
      {
        os
          << "Запрос `" << queryUnescaped << "`, нашлось записей: " << searcher.results().size() << endl
          << endl
        ;
        searcher.dump_results_pre(os);
      }
      else
      {
        os
          << "На запрос `" << queryUnescaped << "` ничего не найдено" << endl
        ;
      }
    }
  }
}

Connection::Connection(const int fd, LibevWrapper &ev, SocketAsyncTCPServer &server, const Storage &storage)
  : BufferedSender(ev)
  , m_fd(fd)
  , m_server(server)
  , m_storage(storage)
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
    << "      Это тестовый поиск" << endl
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
