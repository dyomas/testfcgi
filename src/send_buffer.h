#ifndef __send_buffer_h__
#define __send_buffer_h__

#include <sys/types.h>

class SendBuffer
{
  struct bufferItem
  {
    char * const data;
    const size_t length;
    size_t offset;
    bufferItem *next;
    bufferItem(const char *_data, const size_t _length);
  };

  bufferItem *mItemFirst;
  bufferItem *mItemLast;

  SendBuffer(const SendBuffer &);
  SendBuffer& operator=(const SendBuffer&);
  void mClear();

public:
  SendBuffer();
  ~SendBuffer();

  void append(const char *data, const size_t length);
  void advance(size_t size);
  void clear();

  const bool empty() const;
  const char* data() const;
  const size_t size() const;
  const char* data(size_t &length) const;
};

#endif //__send_buffer_h__
