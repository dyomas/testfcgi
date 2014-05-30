#ifndef __searcher_h__
#define __searcher_h__

#include <stdint.h>
#include <string>
#include <vector>

#include "storage.h"

class Searcher
{
public:
  size_t lexemes_total() const;
  size_t lexemes_found() const;
  void search();

  Searcher(const Storage &, const std::string &);

private:
  typedef std::vector<Storage::index_t::const_iterator> iters_t;
  const Storage &m_storage;
  iters_t m_iters;
  size_t m_lexemes_counter;

  void m_init(const std::string &);
};

#endif //__searcher_h__
