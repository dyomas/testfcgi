#include <stdexcept>

#include "logger.h"
#include "fastcgi_request.h"

FCGIRequest::requestProps::requestProps()
  : id(1)
  , type(frtBegin)
  , role(frlResponder)
  , keepConnection(true)
{
}

void FCGIRequest::m_setup_parameters(const uint8_t *begin, const uint8_t *end)
{
  while (end - begin)
  {
    const size_t name_len = *begin >> 7 ? (((*begin) & 0x7F) << 24) + (begin[1] << 16) + (begin[2] << 8) + begin[3] : *begin;
    begin += *begin >> 7 ? 4 : 1;
    const size_t value_len = *begin >> 7 ? (((*begin) & 0x7F) << 24) + (begin[1] << 16) + (begin[2] << 8) + begin[3] : *begin;
    string name;

    begin += *begin >> 7 ? 4 : 1;

    name.assign((const char *)begin, (const char *)begin + name_len);

    begin += name_len;
    const string value((const char *)begin, (const char *)begin + value_len);
    begin += value_len;

    LOG_DEBUG("FCGI: `" << name << "` = `" << value << "`");
    m_params[name] = value;
  }
}

void FCGIRequest::m_setup_stdin(const uint8_t *begin, const uint16_t len)
{
  m_stdin.assign((const char *)begin, len);
  LOG_DEBUG("FCGI: `" << m_stdin << "`");
}

void FCGIRequest::requestProps::init(const uint8_t *&begin, const uint8_t *
#ifdef _DEBUG
  end
#endif //_DEBUG
)
{
  ostringstream ostr;
  const FCGI_Header *pheader = (FCGI_Header *)begin;
  const FCGI_BeginRequestBody *pbody;

#ifdef _DEBUG
  if ((end - begin) < ssize_t(sizeof(FCGI_Header)))
  {
    ostr << "Insufficient header size (" << end - begin << " < " << sizeof(FCGI_Header) << ")";
    throw logic_error(ostr.str());
  }
#endif //_DEBUG

  if (pheader->version != FCGI_VERSION_1)
  {
    ostr << "Unsupported protocol version (" << pheader->version << ")";
    throw runtime_error(ostr.str());
  }

  type = fcgiRequestTypes(pheader->type);
  id = (pheader->requestIdB1 << 8) + pheader->requestIdB0;

  begin += sizeof(FCGI_Header);
  if (type == frtBegin)
  {
  #ifdef _DEBUG
    if ((end - begin) < ssize_t(sizeof(FCGI_BeginRequestBody)))
    {
      ostr << "Insufficient size of begin request body (" << end - begin << " < " << sizeof(FCGI_BeginRequestBody) << ")";
      throw logic_error(ostr.str());
    }
  #endif //_DEBUG
    pbody = (FCGI_BeginRequestBody *)begin;
    role = fcgiRoles((pbody->roleB1 << 8) + pbody->roleB0);
    keepConnection = pbody->flags & FCGI_KEEP_CONN;
    begin += sizeof(FCGI_BeginRequestBody);
  }
  else if (type != frtAbort && type != frtGetValues)
  {
    ostr << "Unexpected request type (" << type << ")";
    throw runtime_error(ostr.str());
  }
}

const string FCGIRequest::InsufficientData::m_describe(const uint16_t obtained, const uint16_t required)
{
  ostringstream ostr;
  ostr << "Insufficient data length; remainder " << obtained << " while " << required << " declared";
  return ostr.str();
}

FCGIRequest::InsufficientData::InsufficientData()
  : underflow_error("Insufficient data length")
{
}

FCGIRequest::InsufficientData::InsufficientData(const uint16_t obtained, const uint16_t required)
  : underflow_error(m_describe(obtained, required))
{
}

void FCGIRequest::dump(ostream &os) const
{
  os
    << "  m_props" << endl
    << "  {" << endl
    << "    id = " << m_props.id << endl
    << "    type = " << m_props.type << endl
    << "    role = " << m_props.role << endl
    << "    keepConnection = " << boolalpha << m_props.keepConnection << noboolalpha << endl
    << "  }" << endl
  ;
}

const uint16_t FCGIRequest::id() const
{
  return m_props.id;
}

const fcgiRequestTypes FCGIRequest::type() const
{
  return m_props.type;
}

const FCGIRequest::params_t &FCGIRequest::params() const
{
  return m_params;
}

const string &FCGIRequest::stdin() const
{
  return m_stdin;
}

void FCGIRequest::parse(const uint8_t *begin, const uint8_t *end)
{
  bool cin_obtained = false;
  uint16_t
      len
    , id
  ;
  const FCGI_Header *pheader;
  const uint8_t *begin_data;

  m_props.init(begin, end);
  if (m_props.type == frtAbort)
  {
    throw runtime_error("Request aborted");
  }

  while ((end - begin) >= ssize_t(sizeof(FCGI_Header)))
  {
    pheader = (const FCGI_Header *)begin;
    if ((id = (pheader->requestIdB1 << 8) + pheader->requestIdB0) != m_props.id)
    {
      ostringstream ostr;
      ostr << "Unexpected request id: " << id << " != " << m_props.id;
      throw runtime_error(ostr.str());
    }
    len = (pheader->contentLengthB1 << 8) + pheader->contentLengthB0;
    begin_data = begin + sizeof(FCGI_Header);
    if (end - begin_data < len)
    {
      throw InsufficientData((end - begin_data), len);
    }

    switch (pheader->type)
    {
      case FCGI_PARAMS:
      case FCGI_GET_VALUES:
        if (len)
        {
          m_setup_parameters(begin_data, begin_data + len);
        }
        begin = begin_data + len + pheader->paddingLength;
        break;
      case FCGI_STDIN:
        cin_obtained = true;
        if (!len)
        {
          begin = begin_data;
          return;
        }
        else
        {
          m_setup_stdin(begin_data, len);
          begin = begin_data + len + pheader->paddingLength;
        }
        break;
      default:
        {
          ostringstream ostr;
          ostr << "Unexpected record type (" << pheader->type << ")";
          throw runtime_error(ostr.str());
        }
    }
  }

  if (!cin_obtained)
  {
    throw InsufficientData();
  }
}
