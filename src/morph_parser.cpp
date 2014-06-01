#include <errno.h>
#include <stemmers.h>
#include <fuzzyukr.h>
#include <fuzzyrus.h>
#include <string.h>
#include <vector>

#include "logger.h"
#include "morph_parser.h"

using namespace std;

/*
Хорошо рассказано тут http://nlpub.ru/Stemka
Слизано отсюда http://www.keva.ru/stemka/stemka.tar.gz

Пример теста библиотеки stemka в стандартном окружении ($LANG = *.UTF-8):
  cd ${CMAKE_BINARY_DIR}/stemka-prefix/src/stemka/
  echo $WORD | iconv -f utf-8 -t cp1251 | checkrus rus | iconv -f cp1251 -t utf-8

Интересные слова $WORD: загородное, падающий, тебя, тебе

При внедрении библиотеки словарь лексем из файла ../sample_data.txt схлопнулся на 34%
*/

/*
В оригинале vowels -- гласные буквы русского алфавита в кодировке cp1251
см stemka-prefix/src/stemka/example/checkrus.cpp в ${CMAKE_BINARY_DIR}
*/
char  vowels[] = "\xe0\xe5\xe8\xee\xf3\xfb\xfd\xfe\xff";
unsigned char toLoCaseMatrix1251[256] =
{
/* Characters in range 0 - 31       */
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
  0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
  0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
  0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
/* Space till plus, characters in range 32 - 63 */
  0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
  0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
  0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
  0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
/* Characters in range 64 - 127     */
  0x40, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
  0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
  0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
  0x78, 0x79, 0x7A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,
  0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
  0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
  0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
  0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F,
/* Characters in range 0x80 - 0x9F  */
  0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
  0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F,
  0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97,
  0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F,
/* Characters in range 0xA0 - 0xA7  */
  0xA0, 0xA2, 0xA2, 0xA3, 0xA4, 0xB4, 0xA6, 0xA7,
  0xB8, 0xA9, 0xBA, 0xAB, 0xAC, 0xAD, 0xAE, 0xBF,
  0xB0, 0xB1, 0xB3, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7,
  0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF,
/* Characters in range 0xC0 - 0xDF, cyrillic capitals */
  0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7,
  0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF,
  0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7,
  0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF,
/* Regular characters in range 0xE0 - 0xFF  */
  0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7,
  0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF,
  0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7,
  0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF
};


iconv_t MorphParser::m_iconv_init(const char *to, const char *from)
{
  iconv_t retval = iconv_open(to, from);

  if (retval == (iconv_t)-1)
  {
    const int errno_copy = errno;
    LOG_ERROR("`iconv_open` failed for conversion direction `" << from << "` -> `" << to << "`, " << errno_copy << ", " << strerror(errno_copy));
  }

  return retval;
}

void MorphParser::m_iconv_free(iconv_t c)
{
  if (c != (iconv_t)-1 && iconv_close(c) == -1)
  {
    int errno_copy = errno;
    LOG_ERROR("`iconv_close` failed");
  }
}

MorphParser::MorphParser()
  : m_direct_converter(m_iconv_init("WINDOWS-1251", "UTF-8"))
  , m_reverse_converter(m_iconv_init("UTF-8", "WINDOWS-1251"))
{
}

MorphParser::~MorphParser()
{
  m_iconv_free(m_direct_converter);
  m_iconv_free(m_reverse_converter);
}

void MorphParser::split(size_t &base_len, size_t &suffix_len, const char *begin, const char *end)
{
  const size_t src_length = end - begin;
  vector<char>
      res(src_length)
    , res_back(src_length)
  ;

  char
      *inbuf = const_cast<char *>(begin)
    , *outbuf = &res[0]
  ;
  size_t
      inbytesleft = src_length
    , outbytesleft = src_length
  ;

  if (iconv (m_direct_converter, &inbuf, &inbytesleft, &outbuf, &outbytesleft) == (size_t)(-1))
  {
    const int errno_copy = errno;
    base_len = src_length;
    suffix_len = 0;
    iconv (m_direct_converter, NULL, &inbytesleft, NULL, &outbytesleft);
    LOG_WARNING("direct `iconv` failed on string `" << begin << "`, " << errno_copy << ", " << strerror(errno_copy));
    return;
  }

  inbuf = &res[0];


  //см stemka-prefix/src/stemka/example/checkrus.cpp в ${CMAKE_BINARY_DIR}
  stemScan  stscan = { fuzzyRus, toLoCaseMatrix1251, vowels, (unsigned)-1 };
  unsigned  buffer[5];
  const int lcount = GetStemLenBuffer(&stscan, buffer, 5, inbuf, src_length - outbytesleft);


  if (!lcount)
  {
    /*
    Типичная ситуация для латиницы, цифр и смешанных слов
    */
    base_len = src_length;
    suffix_len = 0;
    return;
  }

  /*
  Берется самый короткий корень
  (откусывается максимальное количество суффиксов и окончаний)
  из всех возможных
  */
  inbytesleft = buffer[0];
  outbuf = &res_back[0];
  outbytesleft = res_back.capacity();
  if (iconv (m_reverse_converter, &inbuf, &inbytesleft, &outbuf, &outbytesleft) == (size_t)(-1))
  {
    const int errno_copy = errno;
    base_len = src_length;
    suffix_len = 0;
    iconv (m_reverse_converter, NULL, &inbytesleft, NULL, &outbytesleft);
    LOG_WARNING("reverse `iconv` failed, " << errno_copy << ", " << strerror(errno_copy));
    return;
  }

  base_len = src_length - outbytesleft;
  suffix_len = src_length - base_len;
}
