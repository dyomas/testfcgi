#ifndef __send_buffer_trivial_h__
#define __send_buffer_trivial_h__

#include <sys/types.h>
#include <string>

class SendBufferTrivial
{
  std::string m_data;

public:
  void append(const char *data, const size_t length);
  void advance(size_t size);
  void clear();

  const bool empty() const;
  const char* data() const;
  const size_t size() const;
  const char* data(size_t &length) const;
};

#endif //__send_buffer_trivial_h__
