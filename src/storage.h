#ifndef __storage_h__
#define __storage_h__

#include <stdint.h>
#include <string>
#include <vector>
#include <map>
#include <ostream>

class Storage
{
public:
  static bool is_lexeme_delimiter(const char);

  static const ssize_t first_key;
  struct indexKey
  {
    const size_t lexeme_id;
    const ssize_t offset;
  };
  struct dictItem
  {
    size_t
        lexeme_id
      , frequency
    ;
  };
  struct dataItem
  {
    std::string
        raw
      , url
    ;
    size_t
        rating
      , line
      , lexemes
    ;
  };

  typedef std::map<std::string, dictItem> dict_t;
  typedef std::multimap<indexKey, size_t> index_t;
  typedef std::vector<dataItem> data_t;

  const dict_t &dict() const;
  const index_t &index() const;
  const data_t &data() const;

  void dump_dict(std::ostream &) const;
  void dump_index(std::ostream &) const;
  void dump_data(std::ostream &) const;

  Storage(const std::string &);

private:
  dict_t m_dict;
  index_t m_index;
  data_t m_data;

  void m_init(const std::string &);
  size_t m_split_and_indexate_lexeme(const char *);
};

bool operator < (const Storage::indexKey &, const Storage::indexKey &);

#endif //__storage_h__
