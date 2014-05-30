#include <stdlib.h>
#include <iomanip>

#include "logger.h"
#include "searcher.h"

using namespace std;

bool operator < (const Searcher::resultKey &left, const Searcher::resultKey &right)
{
  return (left.cnt < right.cnt) || (left.cnt == right.cnt && left.raiting < right.raiting)  || (left.cnt == right.cnt && left.raiting == right.raiting && left.mark < right.mark);
}

size_t Searcher::lexemes_total() const
{
  return m_lexemes_counter;
}

size_t Searcher::lexemes_found() const
{
  return m_iters.size();
}

const Searcher::results_t &Searcher::results() const
{
  return m_results;
}

void Searcher::dump_results(ostream &os) const
{
  for (results_t::const_reverse_iterator iter = m_results.rbegin(), iter_end = m_results.rend(); iter != iter_end; iter ++)
  {
    const resultKey &key = iter->first;
    const resultData &value = iter->second;
    os << setw(3) << key.cnt << setw(6) << key.raiting << ' ' << value.raw << " (" << value.line << "/" << value.offset << ", [";
    bool delimiter = false;
    for (coords_t::const_iterator iter_coord = value.coords.begin(), iter_coord_end = value.coords.end(); iter_coord != iter_coord_end; iter_coord ++)
    {
      if (delimiter)
      {
        os << ", ";
      }
      os << static_cast<uint16_t>(*iter_coord);
      delimiter = true;
    }
    os
      << "])" << endl
    ;
  }
}

void Searcher::search()
{
  LOG_DEBUG("Iterators " << m_iters.size());

  if (!m_iters.size())
  {
    return;
  }

  size_t rcs[m_iters.size()];

  while (true)
  {
    bool min_inited = false;
    size_t
        min
      , rcs_cnt = 0
      , iter_num = 0
    ;

    for (iters_t::const_iterator iter = m_iters.begin(), iter_end = m_iters.end(); iter != iter_end; iter ++)
    {
      const iteratorData &itm = *iter;
      if (!itm.end)
      {
        if (min_inited)
        {
          if (min > itm.iter->first.offset)
          {
            min = itm.iter->first.offset;
            rcs_cnt = 0;
          }
        }
        else
        {
          min = itm.iter->first.offset;
          min_inited = true;
        }
        rcs[rcs_cnt ++] = iter_num;
      }
      iter_num ++;
    }

    /*
    Найдено rcs_cnt итераторов, указывающих на одну и ту же строку, номер которой минимален из всех возможных
    Индексы этих операторов сложены в массив rcs
    */

    const Storage::indexKey &idx = m_iters[rcs[0]].iter->first;
    const Storage::dataItem &data = m_storage.data()[idx.offset];

    resultData rd = {data.line, idx.offset, data.raw};
    resultKey rk = {rcs_cnt, data.rating, 0};

    /*
    Текущие значения итераторов слиты в m_results
    Приращение всех "отставших" итераторов
    */

    for (size_t pos = 0; pos != rcs_cnt; pos ++)
    {
      iteratorData &itm = m_iters[rcs[pos]];
      Storage::index_t::const_iterator &iter = itm.iter;
      const Storage::indexKey key_prev = iter->first;
      while (true)
      {
        rd.coords.push_back(iter->second);
        if (++ iter == m_storage.index().end() || iter->first.lexeme_id != key_prev.lexeme_id)
        {
          itm.end = true;
          break;
        }
        else if (iter->first.offset != key_prev.offset)
        {
          break;
        }
      }
    }

    m_results.insert(results_t::value_type(rk, rd));

    /*
    Итераторы сместились, проверить их состояние. Если все достигли конца, завершить цикл
    */

    bool all_iterators_finished = true;
    for (iters_t::const_iterator iter = m_iters.begin(), iter_end = m_iters.end(); iter != iter_end; iter ++)
    {
      if (!iter->end)
      {
        all_iterators_finished = false;
        break;
      }
    }

    if (all_iterators_finished)
    {
      break;
    }
  }
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
          const size_t lexeme_id = iter_dict->second.lexeme_id;
          Storage::indexKey key = {lexeme_id, Storage::first_key};
          iteratorData itm = {lexeme_id, false, m_storage.index().find(key)};
          itm.iter ++;
          m_iters.push_back(itm);
          LOG_DEBUG("Lexeme `" << lexeme << "`: id = " << lexeme_id << ", freq = " << iter_dict->second.frequency);
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
