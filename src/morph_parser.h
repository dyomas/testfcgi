#ifndef __morph_parser_h__
#define __morph_parser_h__

#include <stdint.h>
#include <iconv.h>

class MorphParser
{
  iconv_t
      m_direct_converter
    , m_reverse_converter
  ;

  static iconv_t m_iconv_init(const char *, const char *);
  static void m_iconv_free(iconv_t);

  MorphParser(const MorphParser &);
  MorphParser &operator= (const MorphParser &);

public:
  MorphParser();
  ~MorphParser();

  void split(size_t &, size_t &, const char *begin, const char *end);
};

#endif //__morph_parser_h__

