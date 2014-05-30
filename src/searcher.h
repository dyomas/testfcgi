#ifndef __searcher_h__
#define __searcher_h__

#include <stdint.h>
#include <string>
#include <vector>
#include <ostream>

#include "storage.h"

class Searcher
{
public:
  typedef std::vector<uint8_t> coords_t;
  struct resultKey
  {
    const size_t
        cnt
      , raiting
    ;
    const uint8_t mark;
  };
  struct resultData
  {
    const size_t line, offset;
    const std::string raw;
    coords_t coords;
  };

  typedef std::multimap<resultKey, resultData> results_t;

  size_t lexemes_total() const;
  size_t lexemes_found() const;
  const results_t &results() const;
  void dump_results(std::ostream &) const;

  void search();

  Searcher(const Storage &, const std::string &);

private:
  struct iteratorData
  {
    size_t lexeme_id;
    bool end;
    Storage::index_t::const_iterator iter;
  };
  typedef std::vector<iteratorData> iters_t;
  const Storage &m_storage;

  iters_t m_iters;
  results_t m_results;
  size_t m_lexemes_counter;

  void m_init(const std::string &);
};

bool operator < (const Searcher::resultKey &, const Searcher::resultKey &);

#endif //__searcher_h__
