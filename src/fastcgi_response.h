#ifndef __fastcgi_response_h__
#define __fastcgi_response_h__

#include <stdint.h>
#include <errno.h>
#include <sys/socket.h>
#include <iostream>
#include <sstream>
#include <string>
#include <stdexcept>

#include "fastcgi_common.h"

class BufferedSender;

class FCGIResponse
{
  enum sourceType
  {
      stNo = 0x0
    , stRaw = 0x1
    , stHeaderGenerator = 0x10
    , stHeaderFile = 0x20
    , stHeaderString = 0x40
    , stContentGenerator = 0x80
    , stContentFile = 0x100
    , stContentString = 0x200
  };

  static const uint16_t m_max_data_length;
  uint16_t m_id;
  int m_status;
  sourceType m_source_type;
  std::string
      m_str1
    , m_str2
    , m_error
  ;
  FCGI_Header m_fcgi_header;
  FCGI_EndRequestBody m_end_request;
  FCGI_UnknownTypeBody m_unknown_type;
  BufferedSender *m_sender;

  template <class T> inline void m_write(const int /*socket*/, const T &/*src*/);

  explicit FCGIResponse(const FCGIResponse &);
  FCGIResponse &operator = (const FCGIResponse &);

public:
  void clean();
  void setId(const uint16_t);
  void setAppStatus(const int);
  void setHeader(const std::string &);
  void setHeader(const std::ostringstream &);
  void setContent(const std::string &);
  void setContent(const std::ostringstream &);
  void setError(const std::string &);
  void setError(const std::ostringstream &);
  FCGIResponse & operator= (const std::string &);
  FCGIResponse & operator= (const std::ostringstream &);
  void writeOut(const int /*socket*/);

  FCGIResponse(BufferedSender *);
};

#endif //__fastcgi_response_h__
