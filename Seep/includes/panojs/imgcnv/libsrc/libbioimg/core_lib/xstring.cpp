/*****************************************************************************
 Extended String Class

 IMPLEMENTATION

 Author: Dima V. Fedorov <mailto:dima@dimin.net> <http://www.dimin.net/>

 History:
   12/05/2005 23:38 - First creation

 Ver : 7
*****************************************************************************/

#include <cstdarg>
#include <cstdio>
#include <cstring>

#include <sstream>
#include <algorithm>
#include <iostream>
#include <fstream>

#include "xstring.h"

const int MAX_STR_SIZE = 1024;

//******************************************************************************

xstring &xstring::sprintf(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  sprintf( fmt, ap );
  va_end(ap);
  return *this;
}

xstring &xstring::saddf(const char *fmt, ...) {
  xstring result;
  va_list ap;
  va_start(ap, fmt);
  result.sprintf( fmt, ap );
  va_end(ap);
  *this += result;
  return ( *this );
}

xstring xstring::xprintf(const char *fmt, ...) {
  xstring result;
  va_list ap;
  va_start(ap, fmt);
  result.sprintf( fmt, ap );
  va_end(ap);
  return result;
}

xstring &xstring::sprintf( const char *format, va_list ap ) {
  xstring result;
  char cbuf[MAX_STR_SIZE];

#ifdef HAVE_SECURE_C  
  vsprintf_s((char *) cbuf, MAX_STR_SIZE, format, ap);
#else
  vsprintf((char *) cbuf, format, ap);
#endif
  result += cbuf;

  *this = result;
  return *this;
}

//******************************************************************************

xstring &xstring::insertAfterLast(const char *p, const xstring &s) {
  unsigned int sp = (unsigned int) this->rfind(p);
  if (sp != std::string::npos)
    this->insert(sp, s);
  return *this;
}

//******************************************************************************

// strips trailing chars
xstring &xstring::rstrip(const xstring &chars) {
  int p = this->find_last_not_of(chars);
  if (p != std::string::npos) {
    this->resize(p+1, ' ');
  } else { // if the string is only composed of those chars
    for (unsigned int i=0; i<chars.size(); ++i)
      if ((*this)[0]==chars[i]) { this->resize(0); break; }
  }
  return *this;
}

// strips leading chars
xstring &xstring::lstrip(const xstring &chars) {
  int p = this->find_first_not_of(chars);
  if (p != std::string::npos) {
    *this = this->substr( p, std::string::npos );
  } else { // if the string is only composed of those chars
    for (unsigned int i=0; i<chars.size(); ++i)
      if ((*this)[0]==chars[i]) { this->resize(0); break; }
  }
  return *this;
}

// strips leading and trailing chars
xstring &xstring::strip(const xstring &chars) {
  rstrip(chars);
  lstrip(chars);
  return *this;
}

//******************************************************************************

template<typename RT, typename T, typename Trait, typename Alloc>
RT ss_atoi( const std::basic_string<T, Trait, Alloc>& the_string, RT def ) {
  std::basic_istringstream< T, Trait, Alloc> temp_ss(the_string);
  RT num;
  try {
    temp_ss.exceptions(std::ios::badbit | std::ios::failbit);
    temp_ss >> num;
  } 
  catch(std::ios_base::failure e) {
    return def;
  }
  return num;
}

int xstring::toInt( int def ) {
  return ss_atoi<int>(*this, def);
}

double xstring::toDouble( double def ) {
  return ss_atoi<double>(*this, def);
}

//******************************************************************************

bool xstring::operator==(const xstring &other) const {
  return ( size() == other.size()) && (memcmp( this->c_str(), other.c_str(), size() )==0);
}

bool xstring::operator==(const char *s) const {
  xstring other = s;
  return ( size() == other.size()) && (memcmp( this->c_str(), other.c_str(), size() )==0);
}

int xstring::compare(const xstring &s) const {
  return strncmp( this->c_str(), s.c_str(), std::min(this->size(), s.size()) );
}

//******************************************************************************

bool xstring::startsWith(const xstring &s) const {
  //if (s.size()>this->size()) return false;
  //return (this->substr( 0, s.size() ) == s);
  return ( strncmp( this->c_str(), s.c_str(), std::min(this->size(), s.size()) ) == 0);
}

bool xstring::endsWith(const xstring &s) const {
  if (s.size()>this->size()) return false;
  return (this->substr( this->size()-s.size(), s.size() ) == s);
}


//******************************************************************************
std::vector<xstring> xstring::split( const xstring &s ) const {

  std::vector<xstring> list;
  xstring part;

  std::string::size_type start = 0;
  std::string::size_type end;

  while (1) {
    end = this->find ( s, start );
    if (end == std::string::npos) {
      part = this->substr( start );
      list.push_back( part );
      break;
    } else {
      part = this->substr( start, end-start );
      start = end + s.size();
      list.push_back( part );
    }
  }
 
  return list;
}

//******************************************************************************
std::string xstring::toLowerCase() {
  std::string s = *this;
  std::transform(s.begin(), s.end(), s.begin(), tolower);
  return s;
}

std::string xstring::toUpperCase() {
  std::string s = *this;
  std::transform(s.begin(), s.end(), s.begin(), toupper);
  return s;
}

//******************************************************************************
xstring xstring::left(std::string::size_type pos) const {
  return this->substr( 0, pos );
}

xstring xstring::left(const xstring &sub) const {
  std::string::size_type p = this->find(sub);
  if (p == std::string::npos) return xstring();
  return this->left(p);
}

xstring xstring::right(std::string::size_type pos) const {
  return this->substr( pos, std::string::npos );
}

xstring xstring::right(const xstring &sub) const {
  std::string::size_type p = this->rfind(sub);
  if (p == std::string::npos) return xstring();
  return this->right(p+sub.size());
}

xstring xstring::section(std::string::size_type start, std::string::size_type end) const {
  return this->substr( start, end-start );
}

xstring xstring::section(const xstring &start, const xstring &end, std::string::size_type pos) const {
  std::string::size_type s = this->find(start, pos);
  if (s == std::string::npos) return xstring(); else s+=start.size();
  std::string::size_type e = this->find(end, s);  
  if (e == std::string::npos) return xstring(); else e;
  return this->section(s, e);
}

//******************************************************************************
bool xstring::contains ( const xstring &str ) const {
  std::string::size_type loc = this->find( str, 0 );
  return loc != std::string::npos;
}

//******************************************************************************
xstring xstring::replace( const xstring &what, const xstring &with ) const {
  std::string::size_type p = this->find( what, 0 );
  if (p == std::string::npos) return *this;
  xstring r = *this;
  while (p != std::string::npos) {
    r = r.replace( p, what.size(), with.c_str() );
    p = r.find( what, p+with.size() );
  }
  return r;
}

//******************************************************************************
// I/O
//******************************************************************************

bool xstring::toFile( const xstring &file_name ) const {
  std::ofstream f;
  f.open(file_name.c_str());
  if (!f.is_open()) return false;
  f << *this;
  f.close();
  return true;
}

bool xstring::fromFile( const xstring &file_name ) {
  std::ifstream f(file_name.c_str());
  if (!f.is_open()) return false;
  while (!f.eof()) {
    std::string line;
    std::getline(f, line);
    this->append(line);
    this->append("\n");
  }
  f.close();
  return true;
}

