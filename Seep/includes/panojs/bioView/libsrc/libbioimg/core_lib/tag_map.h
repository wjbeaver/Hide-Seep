/*******************************************************************************
  
  Map for tag/value pairs
    
  Author: Dima Fedorov Levit <dimin@dimin.net> <http://www.dimin.net/>

  History:
    2008-03-18 17:13 - First creation
    2009-07-08 19:24 - INI parsing, setting values, iterative removing
      
  ver: 2
        
*******************************************************************************/

#ifndef D_TAG_MAP_H
#define D_TAG_MAP_H

#include <string>
#include <map>

class DTagMap : public std::map<std::string, std::string>
{
public:
  // constructors
  explicit DTagMap(): std::map<std::string, std::string>() {}
  DTagMap( const DTagMap& _Right ): std::map<std::string, std::string>(_Right) {}
  DTagMap( const std::map<std::string, std::string>& _Right ): std::map<std::string, std::string>(_Right) {}

  template<class InputIterator>
  DTagMap( InputIterator _First,
           InputIterator _Last ): std::map<std::string, std::string>(_First, _Last) {}

  bool keyExists( const std::string &key ) const;
  bool hasKey( const std::string &key ) const { return keyExists(key); }


  // additional methods
  std::string get_value( const std::string &key, const std::string &def="" ) const;
  int         get_value_int( const std::string &key, const int &def ) const;
  double      get_value_double( const std::string &key, const double &def ) const;

  std::string get_key( const std::string &val ) const;
  std::string get_key( const int &value ) const;
  std::string get_key( const unsigned int &value ) const;
  std::string get_key( const double &value ) const;
  std::string get_key( const float &value ) const;

  std::string get_key_where_value_startsWith( const std::string &val ) const;
  std::string get_key_where_value_endsWith( const std::string &val ) const;

  // Sets the value of a tag, if the key exists its value is modified
  void set_value( const std::string &key, const std::string &value );
  void set_value( const std::string &key, const int &value );
  void set_value( const std::string &key, const unsigned int &value );
  void set_value( const std::string &key, const double &value );
  void set_value( const std::string &key, const float &value );

  // Appends a tag if the key does not exist yet
  void append_tag( const std::string &key, const std::string &value );
  void append_tag( const std::string &key, const int &value );
  void append_tag( const std::string &key, const unsigned int &value );
  void append_tag( const std::string &key, const double &value );
  void append_tag( const std::string &key, const float &value );

  void eraseKeysStaringWith( const std::string &str );

  // set tags from another hashtable using a prefix for a tag name
  void set_values( const std::map<std::string, std::string> &tags, const std::string &prefix="" );

  // append tags from another hashtable, using a prefix for a tag name: prefix+name
  void append_tags( const std::map<std::string, std::string> &tags, const std::string &prefix="" );

  // parse a string containing an INI formatted variables, using a prefix for a tag name: prefix+name
  void parse_ini( const std::string &ini, const std::string &separator="=", const std::string &prefix="", const std::string &stop_at_dir="" );

  std::string join( const std::string &sep ) const;
  std::string toString() const { return join("\n"); }

private:
  std::string readline( const std::string &str, int &pos ) const;
};

#endif // D_TAG_MAP_H


