// $Date: 2010-02-11 01:13:55 +0300 (Thu, 11 Feb 2010) $
// $Revision: 404 $
// $Author: saltaev $
// $HeadURL: file:///spool/SVN/v256ru/trunk/include/fastcgi_enums.h $

#ifndef __fastcgi_enums_h__
#define __fastcgi_enums_h__

//About comments

#include <sstream>

#include "fastcgi_common.h"

using namespace std;

//#define's

enum fcgiRequestTypes
{
    frtBegin = FCGI_BEGIN_REQUEST
  , frtAbort = FCGI_ABORT_REQUEST
  , frtGetValues = FCGI_GET_VALUES
  , frtUnknown = FCGI_UNKNOWN_TYPE
};
ostream &operator << (ostream &os, const fcgiRequestTypes);

enum fcgiRoles
{
    frlUnspecified = 0
  , frlResponder = FCGI_RESPONDER
  , frlAuthorizer = FCGI_AUTHORIZER
  , frlFilter = FCGI_FILTER
};
ostream &operator << (ostream &os, const fcgiRoles);

enum fcgiRequestFillingStates
{
    frfsInitial
  , frfsStrangeId
  , frfsInsufficientData
  , frfsReady
};
ostream &operator << (ostream &os, const fcgiRequestFillingStates);

#endif //__fastcgi_enums_h__

