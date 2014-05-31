#ifndef __fastcgi_request_h__
#define __fastcgi_request_h__

//Initially used Peter Simons (simons@cryp.to) files (http://git.cryp.to/?p=fastcgi;a=snapshot;h=HEAD)

#include <stdint.h>
#include <errno.h>
#include <sys/socket.h>
#include <stdexcept>
#include <map>

#include "fastcgi_enums.h"

class FCGIRequest
{
public:
  typedef std::map<std::string, std::string> params_t;

  class InsufficientData: public std::underflow_error
  {
    static const std::string m_describe(const uint16_t, const uint16_t);
  public:
    InsufficientData(const uint16_t, const uint16_t);
  };

  void dump(std::ostream &) const;

  const uint16_t id() const;
  const fcgiRequestTypes type() const;
  const params_t &params() const;
  const std::string &stdin() const;

  void parse(const uint8_t *begin, const uint8_t *end);

private:
  struct requestProps
  {
    uint16_t id;
    fcgiRequestTypes type;
    fcgiRoles role;
    bool keepConnection;
    void init(const uint8_t *&/*begin*/, const uint8_t */*end*/);
    requestProps();
  } m_props;

  void m_setup_parameters(const uint8_t *begin, const uint8_t *end);
  void m_setup_stdin(const uint8_t *begin, const uint16_t len);

  params_t m_params;
  std::string m_stdin;
};

#endif //__fastcgi_request_h__
