#ifndef __socket_async_base_h__
#define __socket_async_base_h__

#include <netdb.h>
#include <iostream>
#include <string>

class SocketAsyncBase
{
public:
  static const size_t
      buff_size_default
    , backlog_default
  ;
  static size_t
      buff_size
    , backlog
  ;
  enum disconnector
  {
      dcnUnknown
    , dcnSelf
    , dcnRemote
  };
  enum errorSource
  {
      esAccept
    , esConnect
    , esRead
    , esWrite
    , esClose
  };
  enum socketEnds
  {
      seLocal
    , seRemote
  };

  static const std::string m_port_2_string(const uint16_t);
  /* NOTE TODO Deprecated parameter
  At the moment of fully integrated asynchronous DNS resolving this function will always accepts raw IP and numeric port, thus getaddrinfo must not try to resolve it; at this moment remove paramter pure_ip and change code as it always true
  */
  static struct addrinfo *init_client_tcp_socket(int &fd, const std::string &/*host*/, const std::string &/*port*/, const bool pure_ip);
  static int init_client_tcp_socket();
  static int init_server_inet_socket(const uint16_t /*port*/);
  static int init_client_unix_socket();
  static int describe_socket(std::string &description, const int fd, const socketEnds se);
  static int describe_socket(std::string &description, const int fd);
};

std::ostream &operator << (std::ostream &/*os*/, const SocketAsyncBase::disconnector /*dcn*/);
std::ostream &operator << (std::ostream &/*os*/, const SocketAsyncBase::errorSource /*es*/);

#endif //__socket_async_base_h__
