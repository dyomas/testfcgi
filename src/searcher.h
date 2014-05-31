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
  typedef std::vector<size_t> coords_t;
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
    coords_t coords;
  };

  typedef std::multimap<resultKey, resultData> results_t;

  size_t lexemes_total() const;
  size_t lexemes_found() const;
  const results_t &results() const;
  void dump_results(std::ostream &) const;
  void dump_results_pre(std::ostream &) const;

  void search();

  Searcher(const Storage &, const std::string &);

private:
  struct cogwheel
  {
    size_t
        lexeme_id
      , frequency
    ;
    bool end;
    Storage::index_t::const_iterator iter;
  };
  typedef std::vector<cogwheel> cogwheels_t;
  const Storage &m_storage;

  cogwheels_t m_cogwheels;
  results_t m_results;
  size_t m_lexemes_counter;

  void m_init(const std::string &);
};

bool operator < (const Searcher::resultKey &, const Searcher::resultKey &);

#endif //__searcher_h__
