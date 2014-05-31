// $Date: 2010-02-11 01:13:55 +0300 (Thu, 11 Feb 2010) $
// $Revision: 404 $
// $Author: saltaev $
// $HeadURL: file:///spool/SVN/v256ru/trunk/cpp/fastcgi_enums.cpp $

#include "fastcgi_enums.h"

ostream &operator << (ostream &os, const fcgiRequestTypes frt)
{
  switch (frt)
  {
    case frtBegin: //FCGI_BEGIN_REQUEST
      os << "FCGI_BEGIN_REQUEST";
      break;
    case frtAbort: //FCGI_ABORT_REQUEST
      os << "FCGI_ABORT_REQUEST";
      break;
    case frtGetValues: //FCGI_GET_VALUES
      os << "FCGI_GET_VALUES";
      break;
    case frtUnknown: //FCGI_UNKNOWN_TYPE
      os << "FCGI_UNKNOWN_TYPE";
      break;
    default:
      os << "<" << (int)frt << ">";
  }
  return os;
}

ostream &operator << (ostream &os, const fcgiRoles frl)
{
  switch (frl)
  {
    case frlUnspecified: //0
      os << "Unspecified";
      break;
    case frlResponder: //FCGI_RESPONDER
      os << "Responder";
      break;
    case frlAuthorizer: //FCGI_AUTHORIZER
      os << "Authorizer";
      break;
    case frlFilter: //FCGI_FILTER
      os << "Filter";
      break;
    default:
      os << "<" << (int)frl << ">";
  }
  return os;
}

ostream &operator << (ostream &os, const fcgiRequestFillingStates frfs)
{
  switch (frfs)
  {
    case frfsInitial:
      os << "Initial";
      break;
    case frfsStrangeId:
      os << "StrangeId";
      break;
    case frfsInsufficientData:
      os << "InsufficientData";
      break;
    case frfsReady:
      os << "Ready";
      break;
/*    case frfsParamsError:
      os << "ParamsError";
      break;*/
    default:
      os << "<" << (int)frfs << ">";
  }
  return os;
}
