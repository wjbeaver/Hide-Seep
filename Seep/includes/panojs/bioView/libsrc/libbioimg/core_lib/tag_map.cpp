/*******************************************************************************
  
  Map for tag/value pairs
    
  Author: Dima Fedorov Levit <dimin@dimin.net> <http://www.dimin.net/>

  History:
    2008-03-18 17:13 - First creation
    2009-07-08 19:24 - INI parsing, setting values, iterative removing
      
  ver: 2
        
*******************************************************************************/

#include "tag_map.h"
#include "xstring.h"

#include <string>
#include <sstream>
#include <map>

//******************************************************************************

bool DTagMap::keyExists( const std::string &key ) const {
  std::map<std::string, std::string>::const_iterator it = this->find( key ); 
  return (it != this->end());
}

//******************************************************************************

void DTagMap::append_tag( const std::string &key, const std::string &value ) {
  this->insert( make_pair( key, value ) );
}

void DTagMap::append_tag( const std::string &key, const int &value ) {
  append_tag( key, xstring::xprintf("%d", value) );
}

void DTagMap::append_tag( const std::string &key, const unsigned int &value ) {
  append_tag( key, xstring::xprintf("%u", value) );
}

void DTagMap::append_tag( const std::string &key, const double &value ) {
  append_tag( key, xstring::xprintf("%f", value) );
}

void DTagMap::append_tag( const std::string &key, const float &value ) {
  append_tag( key, xstring::xprintf("%f", value) );
}

//******************************************************************************

void DTagMap::set_value( const std::string &key, const std::string &value ) {
  (*this)[key] = value;
}

void DTagMap::set_value( const std::string &key, const int &value ) {
  set_value( key, xstring::xprintf("%d", value) );
}

void DTagMap::set_value( const std::string &key, const unsigned int &value ) {
  set_value( key, xstring::xprintf("%u", value) );
}

void DTagMap::set_value( const std::string &key, const double &value ) {
  set_value( key, xstring::xprintf("%f", value) );
}

void DTagMap::set_value( const std::string &key, const float &value ) {
  set_value( key, xstring::xprintf("%f", value) );
}

//******************************************************************************

std::string DTagMap::get_value( const std::string &key, const std::string &def ) const {
  std::string v = def;
  std::map<std::string, std::string>::const_iterator it = this->find( key ); 
  if (it != this->end() )
    v = (*it).second;
  return v;
}

int DTagMap::get_value_int( const std::string &key, const int &def ) const {
  xstring str = this->get_value( key, "" );
  return str.toInt( def );
}

double DTagMap::get_value_double( const std::string &key, const double &def ) const {
  xstring str = this->get_value( key, "" );
  return str.toDouble( def );
}

//******************************************************************************

std::string DTagMap::get_key( const std::string &val ) const {
  std::map< std::string, std::string >::const_iterator it = this->begin();
  while (it != this->end() ) {
    if ( it->second == val ) return it->first;
    ++it;
  }
  return std::string();
}

std::string DTagMap::get_key( const int &value ) const {
  return get_key( xstring::xprintf("%d", value) );
}

std::string DTagMap::get_key( const unsigned int &value ) const {
  return get_key( xstring::xprintf("%u", value) );
}

std::string DTagMap::get_key( const double &value ) const {
  return get_key( xstring::xprintf("%f", value) );
}

std::string DTagMap::get_key( const float &value ) const {
  return get_key( xstring::xprintf("%f", value) );
}

//******************************************************************************
std::string DTagMap::get_key_where_value_startsWith( const std::string &val ) const {
  std::map< std::string, std::string >::const_iterator it = this->begin();
  while (it != this->end() ) {
    xstring s = it->second;
    if ( s.startsWith(val) ) return it->first;
    ++it;
  }
  return std::string();
}

std::string DTagMap::get_key_where_value_endsWith( const std::string &val ) const {
  std::map< std::string, std::string >::const_iterator it = this->begin();
  while (it != this->end() ) {
    xstring s = it->second;
    if ( s.endsWith(val) ) return it->first;
    ++it;
  }
  return std::string();
}


//******************************************************************************

void DTagMap::set_values( const std::map<std::string, std::string> &tags, const std::string &prefix ) {
  std::map< std::string, std::string >::const_iterator it = tags.begin();
  while (it != tags.end()) {
    set_value( prefix + it->first, it->second );
    it++;
  }
}

void DTagMap::append_tags( const std::map<std::string, std::string> &tags, const std::string &prefix ) {
  std::map< std::string, std::string >::const_iterator it = tags.begin();
  while (it != tags.end()) {
    append_tag( prefix + it->first, it->second );
    it++;
  }
}

//******************************************************************************

std::string DTagMap::readline( const std::string &str, int &pos ) const {
  std::string line;
  std::string::const_iterator it = str.begin() + pos;
  while (it<str.end() && *it != 0xA ) {
    if (*it != 0xD) 
      line += *it;
    else
      ++pos;
    ++it;
  }
  pos += line.size();
  if (it<str.end() && *it == 0xA) ++pos;
  return line;
}

void DTagMap::parse_ini( const std::string &ini, const std::string &separator, const std::string &prefix, const std::string &stop_at_dir ) {
  std::string dir;
  int pos=0;
  std::string line = this->readline( ini, pos );
  while (pos<ini.size()) {
    
    // if directory
    if (line[0] == '[') {
      dir = line.substr( 1, line.size()-2);
      if (stop_at_dir.size()>0 && dir == stop_at_dir) return;
    } else {
      // if tag-value pair
      int eq_pos = line.find(separator);
      if (eq_pos != std::string::npos ) {
        std::string key = dir + "/" + line.substr( 0, eq_pos );
        std::string val = line.substr( eq_pos+1 );
        if ( *val.begin() == '"' && *(val.end()-1) == '"' ) val = val.substr( 1, val.size()-2);
        (*this)[prefix+key] = val; 
      }// found '='
    } // if tag-value

    line = this->readline( ini, pos );
  }
}

//******************************************************************************
void DTagMap::eraseKeysStaringWith( const std::string &str ) {
  std::map< std::string, std::string >::iterator it = this->begin();
  while (it != this->end()) {
    xstring s = it->first;
    std::map< std::string, std::string >::iterator toerase = it;
    ++it;
    if ( s.startsWith(str) )
      this->erase(toerase);
  }
}

//******************************************************************************
std::string DTagMap::join( const std::string &sep ) const {
  std::string s;
  std::map< std::string, std::string >::const_iterator it = this->begin();
  while (it != this->end()) {
    s += it->first;
    s += ": ";
    s += it->second;
    s += sep;
    ++it;
  }
  return s;
}

