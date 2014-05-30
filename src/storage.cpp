#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <stdexcept>

#include "logger.h"
#include "storage.h"

using namespace std;

const ssize_t Storage::first_key = -1;

bool operator < (const Storage::indexKey &left, const Storage::indexKey &right)
{
  return (left.lexeme_id < right.lexeme_id) || (left.lexeme_id == right.lexeme_id) && (left.offset < right.offset);
}

bool Storage::is_lexeme_delimiter(const char ch)
{
  return ch == '\0' || (ch >= ' ' && ch <= '/') || (ch >= ':' && ch <= '?') || (ch >= '[' && ch <= '`') || (ch >= '{' && ch <= '~');
}

const Storage::dict_t &Storage::dict() const
{
  return m_dict;
}

const Storage::index_t &Storage::index() const
{
  return m_index;
}

const Storage::data_t &Storage::data() const
{
  return m_data;
}

void Storage::dump_dict(ostream &os) const
{
  for (dict_t::const_iterator iter = m_dict.begin(), iter_end = m_dict.end(); iter != iter_end; iter ++)
  {
    os
      << setw(6) << iter->second.lexeme_id << setw(6) << iter->second.frequency << ' ' << iter->first << endl
    ;
  }
}

void Storage::dump_index(ostream &os) const
{
  for (index_t::const_iterator iter = m_index.begin(), iter_end = m_index.end(); iter != iter_end; iter ++)
  {
    if (iter->first.offset == first_key)
    {
      os
        << setw(6) << iter->first.lexeme_id << endl
      ;
    }
    else
    {
      os
        << setw(6) << iter->first.lexeme_id << setw(6) << iter->first.offset << setw(4) << static_cast<uint16_t>(iter->second) << endl
      ;
    }
  }
}

void Storage::dump_data(ostream &os) const
{
  for (data_t::const_iterator iter = m_data.begin(), iter_end = m_data.end(); iter != iter_end; iter ++)
  {
    os
      << setw(6) <<  iter->line << setw(6) <<  iter->rating << ", `" << iter->url << "`, `" << iter->raw << "`" << endl
    ;
  }
}

Storage::Storage(const string &path)
{
  m_init(path);
}

void Storage::m_init(const string &path)
{
  ifstream myfile (path.c_str());
  if (!myfile.is_open())
  {
    throw runtime_error("Can not open file `" + path + "` for reading");
  }

  size_t line_cnt = 0;
  string line;
  while (! myfile.eof())
  {
    getline (myfile, line);
    line_cnt ++;

    if (!line.length() || line[0] == '#')
    {
      continue;
    }

    bool valid = true;
    size_t field = 0;
    const char
        *pch = line.c_str()
      , *pch_start = NULL
    ;
    dataItem itm;
    itm.line = line_cnt;

    do
    {
      if (*pch == '\0' || *pch == '\t')
      {
        if (pch_start)
        {
          switch (field)
          {
            case 0:
              itm.raw.assign(pch_start, pch);
              break;
            case 1:
              itm.url.assign(pch_start, pch);
              break;
            case 2:
              {
                char *ep;
                itm.rating = strtoul(pch_start, &ep, 10);
                const int errno_copy = errno;
                if (errno_copy)
                {
                  LOG_WARNING("Conversion failed from string `" << pch_start << "` to size_t in row " << line_cnt << ", " << errno_copy << ", " << strerror(errno_copy));
                  valid = false;
                }
                else if (ep != pch)
                {
                  LOG_WARNING("Conversion failed from string `" << pch_start << "` to size_t in row " << line_cnt);
                  valid = false;
                }
              }
              break;
            default:
              break;
          }
          pch_start = NULL;
        }
        field ++;
      }
      else if (!pch_start)
      {
        pch_start = pch;
      }
    }
    while (*pch ++);

    if (field != 3)
    {
      LOG_WARNING("Incorrect fields counter " << field << " in row " << line_cnt);
    }
    else if (!itm.url.length() || !itm.raw.length())
    {
      LOG_WARNING("One or more empty fields in row " << line_cnt);
    }
    else if (valid)
    {
      m_add_item(itm);
    }
  }
  myfile.close();

  LOG_DEBUG("Statistics:" << endl
    << "    dict: " << m_dict.size() << endl
    << "    index: " << m_index.size() << endl
    << "    data: " << m_data.size()
  );
}

void Storage::m_add_item(const dataItem &itm)
{
  uint8_t coord = 0;
  const char
      *pch = itm.raw.c_str()
    , *pch_start = NULL
  ;

  const size_t offset = m_data.size();

  do
  {
    if (is_lexeme_delimiter(*pch))
    {
      if (pch_start)
      {
        const string lexeme(pch_start, pch);
        size_t lexeme_id;
        dict_t::iterator iter_dict = m_dict.find(lexeme);

        if (iter_dict == m_dict.end())
        {
          lexeme_id = m_dict.size();
          dictItem d = {lexeme_id, 1};
          m_dict[lexeme] = d;
        }
        else
        {
          lexeme_id = iter_dict->second.lexeme_id;
          iter_dict->second.frequency ++;
        }

        indexKey key = {lexeme_id, first_key};
        if (m_index.find(key) == m_index.end())
        {
          m_index.insert(index_t::value_type(key, 0));
        }
        indexKey key2 = {lexeme_id, offset};
        m_index.insert(index_t::value_type(key2, coord));

        pch_start = NULL;
        coord ++;
      }
    }
    else if (!pch_start)
    {
      pch_start = pch;
    }
  }
  while (*pch ++);

  m_data.push_back(itm);
}
