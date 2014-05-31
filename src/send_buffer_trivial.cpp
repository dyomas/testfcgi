#include <stdlib.h>
#include <string.h>
#include <iostream>

#include "send_buffer_trivial.h"
#include "logger.h"

void SendBufferTrivial::append(const char *data, const size_t length)
{
  if (!data || !length)
  {
    return;
  }

  m_data.append(data, length);
}

void SendBufferTrivial::advance(size_t size)
{
  m_data = m_data.substr(size);
}

void SendBufferTrivial::clear()
{
  m_data.clear();
}

const bool SendBufferTrivial::empty() const
{
  return m_data.empty();
}

const char* SendBufferTrivial::data() const
{
  return m_data.c_str();
}

const size_t SendBufferTrivial::size() const
{
  return m_data.length();
}

const char* SendBufferTrivial::data(size_t &length) const
{
  const char* ret_val = NULL;
  if ((length = m_data.length()))
  {
    ret_val = m_data.c_str();
  }
  return ret_val;
}
