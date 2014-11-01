/*****************************************************************************
 Extended String Class

 DEFINITION

 Author: Dima V. Fedorov <mailto:dima@dimin.net> <http://www.dimin.net/>

 History:
   12/05/2005 23:38 - First creation

 Ver : 6
*****************************************************************************/

#ifndef XSTRING_H
#define XSTRING_H
//#pragma message(">>>>>  xstring: included enhanced string")


// windows: use secure C libraries with VS2005 or higher
#if ( defined(_MSC_VER) && (_MSC_VER >= 1400) )
  #define HAVE_SECURE_C
  #pragma message(">>>>>  xstring: using secure c libraries")
#endif 

#include <cstdarg>
#include <string>
#include <vector>

class xstring: public std::string {
public:
  // constructors
  explicit xstring(): std::string() {}
  xstring( const char *cstr ): std::string() { *this += cstr; }

  /*
  xstring( const value_type* _Ptr, 
           size_type _Count = npos ) : std::string( _Ptr, _Count ) {}
  */
  xstring( const std::string& _Right, 
           size_type _Roff = 0, 
           size_type _Count = npos ) : std::string( _Right, _Roff, _Count ) {}

  xstring( size_type _Count,
           value_type _Ch ) : std::string( _Count, _Ch ) {}

  template <class InputIterator >
  xstring( InputIterator _First, 
           InputIterator _Last ) : std::string( _First, _Last ) {}

  // members
  xstring &sprintf(const char *fmt, ...);
  xstring &saddf(const char *fmt, ...);
  xstring &insertAfterLast(const char *p, const xstring &s);

  static xstring xprintf(const char *fmt, ...);

  xstring &removeSpacesLeft();
  xstring &removeSpacesRight();
  xstring &removeSpacesBoth(); 

  int    toInt( int def = 0 );
  double toDouble( double def = 0.0 );

  std::string toLowerCase();
  std::string toUpperCase();

  bool operator==(const xstring &s) const;
  bool operator==(const char *s) const;
  bool operator!=(const xstring &s) const { return !(*this == s); }
  bool operator!=(const char *s) const { return !(*this == s); }

  bool startsWith(const xstring &s) const;
  bool endsWith(const xstring &s) const;

  int compare(const xstring &s) const;
  static inline int compare(const xstring &s1, const xstring &s2) { return s1.compare(s2); }

  std::vector<xstring> split( const xstring &separator ) const;

  xstring left(std::string::size_type pos) const;
  xstring left(const xstring &sub) const;
  xstring right(std::string::size_type pos) const;
  xstring right(const xstring &sub) const;
  xstring section(std::string::size_type start, std::string::size_type end) const;
  xstring section(const xstring &start, const xstring &end, std::string::size_type pos=0) const;

  bool contains ( const xstring &str ) const;

  using std::string::replace;
  xstring replace( const xstring &what, const xstring &with ) const;

private:
  xstring &sprintf( const char *format, va_list ap );  
};


#endif // XSTRING_H
