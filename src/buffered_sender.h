#ifndef __buffered_sender_h__
#define __buffered_sender_h__

#include "send_buffer_trivial.h"

class LibevWrapper;

class BufferedSender
{
  void *m_write_handler;
  SendBufferTrivial mBuffer;
  LibevWrapper &mEventLoop;

  void m_on_send(const int fd);

public:
  BufferedSender(LibevWrapper &);
  ~BufferedSender();

  virtual void failure(const int) = 0;
  virtual void sent(const size_t) = 0;
  virtual void finished() = 0;

  void cash(const char *data, const size_t length);
  void send(const int fd, const char *data, const size_t length);
  void send_cashed(const int fd);
  void cancel();
};

#endif //__buffered_sender_h__
