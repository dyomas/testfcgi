#include <stdlib.h>

#include "logger.h"
#include "searcher.h"

using namespace std;

size_t Searcher::lexemes_total() const
{
  return m_lexemes_counter;
}

size_t Searcher::lexemes_found() const
{
  return m_iters.size();
}

void Searcher::search()
{
  LOG_DEBUG("Iterators " << m_iters.size());
}

Searcher::Searcher(const Storage &storage, const string &query)
  : m_storage(storage)
  , m_lexemes_counter(0)
{
  m_init(query);
}

void Searcher::m_init(const std::string &query)
{
  const char
      *pch = query.c_str()
    , *pch_start = NULL
  ;
  LOG_DEBUG("Query `" << query << "`");

  do
  {
    if (Storage::is_lexeme_delimiter(*pch))
    {
      if (pch_start)
      {
        const string lexeme(pch_start, pch);
        const Storage::dict_t &dict = m_storage.dict();
        const Storage::dict_t::const_iterator iter_dict = dict.find(lexeme);
        if (iter_dict != dict.end())
        {
          Storage::indexKey key = {iter_dict->second.lexeme_id, Storage::first_key};
          m_iters.push_back(m_storage.index().find(key));
          LOG_DEBUG("Lexeme `" << lexeme << "`: id = " << iter_dict->second.lexeme_id << ", freq = " << iter_dict->second.frequency);
        }
        else
        {
          LOG_DEBUG("Lexeme `" << lexeme << "` not found");
        }
        pch_start = NULL;
        m_lexemes_counter ++;
      }
    }
    else if (!pch_start)
    {
      pch_start = pch;
    }
  }
  while (*pch ++);
}
