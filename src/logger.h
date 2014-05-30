#ifndef __logger_h__
#define __logger_h__

extern bool _G_verbose;

#include <iostream>

#define LOG_ERROR(args_)\
do {\
  std::cerr << "*** " << args_ << std::endl;\
} while (0)

#define LOG_WARNING(args_)\
do {\
  std::cerr << "+++ " << args_ << std::endl;\
} while (0)

#define LOG_DEBUG(args_)\
do {\
  if (_G_verbose)\
    std::cout << "--- " << args_ << std::endl;\
} while (0)

#define LOG_INFO(args_)\
do {\
  if (_G_verbose)\
    std::cout << "=== " << args_ << std::endl;\
} while (0)

#endif //__logger_h__
