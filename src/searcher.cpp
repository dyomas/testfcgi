#include <stdlib.h>
#include <iomanip>
#include <sstream>

#include "logger.h"
#include "searcher.h"
#include "morph_parser.h"

using namespace std;

extern MorphParser _G_morph_parser;

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
  return m_cogwheels.size();
}

const Searcher::results_t &Searcher::results() const
{
  return m_results;
}

const string &Searcher::query_coiffured() const
{
  return m_query_coiffured;
}

void Searcher::dump_results(ostream &os) const
{
  for (results_t::const_reverse_iterator iter = m_results.rbegin(), iter_end = m_results.rend(); iter != iter_end; iter ++)
  {
    const resultKey &key = iter->first;
    const resultData &value = iter->second;
    os << setw(8) << key.cnt << setw(8) << key.raiting << setw(8) << static_cast<uint16_t>(key.mark) << ' ' << m_storage.data()[value.offset].raw << " (" << value.line << "/" << value.offset << ", [";
    bool delimiter = false;
    for (coords_t::const_iterator iter_coord = value.coords.begin(), iter_coord_end = value.coords.end(); iter_coord != iter_coord_end; iter_coord ++)
    {
      if (delimiter)
      {
        os << ", ";
      }
      os << *iter_coord;
      delimiter = true;
    }
    os
      << "])" << endl
    ;
  }
}

void Searcher::dump_results_pre(ostream &os) const
{
  const Storage::data_t &datas = m_storage.data();

  os
    << "lexemes                       |rating|relevan|rownum(-offset)|text, url" << endl
  ;

  for (results_t::const_reverse_iterator iter = m_results.rbegin(), iter_end = m_results.rend(); iter != iter_end; iter ++)
  {
    const resultKey &key = iter->first;
    const resultData &value = iter->second;
    const Storage::dataItem &data = datas[value.offset];

    ostringstream ldescr;
    bool delimiter = false;
    ldescr << key.cnt << ' ' << '{';
    for (coords_t::const_iterator iter_coord = value.coords.begin(), iter_coord_end = value.coords.end(); iter_coord != iter_coord_end; iter_coord ++)
    {
      if (delimiter)
      {
        ldescr << ' ';
      }
      ldescr << *iter_coord;
      delimiter = true;
    }
    ldescr << '}';

    ostringstream pdescr;
    pdescr << value.line;
    if (value.line > value.offset)
    {
      pdescr << " (-" << (value.line - value.offset) << ")";
    }

    os
      << setw(30) << left << ldescr.str() << right
      << setw(7) << key.raiting
      << setw(8) << static_cast<uint16_t>(key.mark)
      << setw(16) << pdescr.str()
      << ' ' << data.raw << ", " << data.url
      << endl
    ;
  }
}

void Searcher::search()
{
  LOG_DEBUG("Iterators " << m_cogwheels.size());

  if (!m_cogwheels.size())
  {
    return;
  }

  size_t rcs[m_cogwheels.size()];

  while (true)
  {
    bool min_inited = false;
    size_t
        min
      , rcs_cnt = 0
      , iter_num = 0
    ;

    /*
    Из всех незакрытых итераторов найти такие, что указывают на ближайшую к началу файла строку. При идеальном совпадении всех лексем все итераторы будут смотреть в одну строку. В наихудшем случае, только один "отстающий" итератор будет указывать на какую то строку
    */
    for (cogwheels_t::const_iterator cogwheel = m_cogwheels.begin(), cogwheel_end = m_cogwheels.end(); cogwheel != cogwheel_end; cogwheel ++)
    {
      if (!cogwheel->end)
      {
        const ssize_t offset = cogwheel->iter->first.offset;
        if (min_inited)
        {
          if (min > offset)
          {
            min = offset;
            rcs_cnt = 0;
            rcs[rcs_cnt ++] = iter_num;
          }
          else if (min == offset)
          {
            rcs[rcs_cnt ++] = iter_num;
          }
        }
        else
        {
          min = offset;
          min_inited = true;
          rcs[rcs_cnt ++] = iter_num;
        }
      }
      iter_num ++;
    }

    if (_G_verbose)
    {
      ostringstream ostr;
      bool delimiter = false;
      for (cogwheels_t::const_iterator cogwheel = m_cogwheels.begin(), cogwheel_end = m_cogwheels.end(); cogwheel != cogwheel_end; cogwheel ++)
      {
        if (!cogwheel->end)
        {
          if (delimiter)
          {
            ostr << ", ";
          }
          ostr << cogwheel->iter->first.offset;
          delimiter = true;
        }
      }
      LOG_DEBUG("Iters: " << ostr.str() << "; " << rcs_cnt << " points to line " << min);
    }
    /*
    Найдено rcs_cnt итераторов, указывающих на одну и ту же строку, номер которой минимален из всех возможных. Индексы этих итераторов сложены в массив rcs
    */


    const Storage::indexKey &idx = m_cogwheels[rcs[0]].iter->first;
    const Storage::dataItem &data = m_storage.data()[idx.offset];
    resultData rd = {data.line, idx.offset};


    /*
    Приращение всех "отставших" итераторов
    */
    for (size_t pos = 0; pos != rcs_cnt; pos ++)
    {
      cogwheel &itm = m_cogwheels[rcs[pos]];
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


    /*
    Оценить порядок следования лексем в запросе и начислить баллы за наилучшие условия
    */
    bool
        lexemes_sequential = true //в порядке возрастания
      , lexemes_successive = true //друг за другом
    ;
    coords_t::const_iterator iter = rd.coords.begin();
    size_t coord_prev = *iter;
    while (++ iter != rd.coords.end())
    {
      if (*iter <= coord_prev)
      {
        lexemes_sequential = lexemes_successive = false;
        break;
      }
      else if (*iter - coord_prev != 1)
      {
        lexemes_successive = false;
      }
      coord_prev = *iter;
    }

    uint8_t mark = 0;
    if (lexemes_successive)
    {
      mark += 4;
    }
    else if (lexemes_sequential)
    {
      mark += 2;
    }
    if (data.lexemes == rd.coords.size())
    {
      mark += 8;
    }


    /*
    Сохранить результат
    */
    const resultKey rk = {rcs_cnt, data.rating, mark};
    m_results.insert(results_t::value_type(rk, rd));


    /*
    Проверить состояние итераторов. Если все достигли конца, завершить цикл
    */
    bool all_iterators_finished = true;
    for (cogwheels_t::const_iterator cogwheel = m_cogwheels.begin(), cogwheel_last = m_cogwheels.end(); cogwheel != cogwheel_last; cogwheel ++)
    {
      if (!cogwheel->end)
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

  LOG_DEBUG("Found records: " << m_results.size());
}

Searcher::Searcher(const Storage &storage, const string &query)
  : m_storage(storage)
  , m_lexemes_counter(0)
{
  m_init(query);
}

void Searcher::m_init(const string &query)
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
        size_t
            base_len
          , suffix_len
        ;
        _G_morph_parser.split(base_len, suffix_len, pch_start, pch);
        if (m_query_coiffured.length())
        {
          m_query_coiffured += ' ';
        }
        m_query_coiffured.append(pch_start, base_len);
        if (suffix_len)
        {
          m_query_coiffured += '|';
          m_query_coiffured.append(&pch_start[base_len], suffix_len);
        }

        const string lexeme(pch_start, pch - suffix_len);
        const Storage::dict_t &dict = m_storage.dict();
        const Storage::dict_t::const_iterator iter_dict = dict.find(lexeme);
        if (iter_dict != dict.end())
        {
          const size_t
              lexeme_id = iter_dict->second.lexeme_id
            , frequency = iter_dict->second.frequency
          ;
          Storage::indexKey key = {lexeme_id, Storage::first_key};
          cogwheel itm = {lexeme_id, frequency, false, m_storage.index().find(key)};
          itm.iter ++;
          m_cogwheels.push_back(itm);
          LOG_DEBUG("Lexeme `" << lexeme << "`: id = " << lexeme_id << ", freq = " << frequency);
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
