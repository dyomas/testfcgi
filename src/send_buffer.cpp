#include <stdlib.h>
#include <string.h>
#include <iostream>

#include "send_buffer.h"
#include "logger.h"


SendBuffer::bufferItem::bufferItem(const char *_data, const size_t _length)
  : data(static_cast<char *>(malloc(_length)))
  , length(_length)
  , offset(0)
  , next(NULL)
{
  if (data)
  {
    memcpy(data, _data, _length);
  }
  else
  {
    LOG_ERROR("\"malloc\" failed");
  }
}

void SendBuffer::mClear()
{
  const bufferItem *pitemCur = mItemFirst;
  while (pitemCur)
  {
    free(pitemCur->data);
    const bufferItem *pitemNext = pitemCur->next;
    delete pitemCur;
    pitemCur = pitemNext;
  }
  mItemFirst = mItemLast = NULL;
}

SendBuffer::SendBuffer(const SendBuffer &)
{
}

SendBuffer::SendBuffer()
  : mItemFirst(NULL)
  , mItemLast(NULL)
{
}

SendBuffer::~SendBuffer()
{
  mClear();
}

void SendBuffer::append(const char *data, const size_t length)
{
  if (!data || !length)
  {
    return;
  }

  bufferItem *pitemNew = new bufferItem(data, length);

  if (!mItemFirst)
  {
    mItemFirst = pitemNew;
  }
  if (mItemLast)
  {
    mItemLast->next = pitemNew;
  }
  mItemLast = pitemNew;
}

void SendBuffer::advance(size_t size)
{
  if (mItemFirst)
  {
    if (mItemFirst->length - mItemFirst->offset >= size)
    {
      mItemFirst->offset += size;
    }
    else
    {
      LOG_WARNING("Buffer size less than decrease size (" << (mItemFirst->length - mItemFirst->offset) << " < " << size << ")");
      mItemFirst->offset = mItemFirst->length;
    }
    if (mItemFirst->offset == mItemFirst->length)
    {
      free(mItemFirst->data);
      bufferItem *pitemNext = mItemFirst->next;
      delete mItemFirst;
      mItemFirst = pitemNext;
      if (!pitemNext)
      {
        mItemLast = NULL;
      }
    }
  }
}

void SendBuffer::clear()
{
  mClear();
}

const bool SendBuffer::empty() const
{
  return !mItemFirst || mItemFirst->length == mItemFirst->offset;
}

const char* SendBuffer::data() const
{
  const char* ret_val = NULL;
  if (mItemFirst)
  {
    ret_val = &mItemFirst->data[mItemFirst->offset];
  }
  return ret_val;
}

const size_t SendBuffer::size() const
{
  size_t ret_val = 0;
  if (mItemFirst)
  {
    ret_val = mItemFirst->length - mItemFirst->offset;
  }
  return ret_val;
}

const char* SendBuffer::data(size_t &length) const
{
  const char* ret_val = NULL;
  if (mItemFirst)
  {
    ret_val = &mItemFirst->data[mItemFirst->offset];
    length = mItemFirst->length - mItemFirst->offset;
  }
  return ret_val;
}
