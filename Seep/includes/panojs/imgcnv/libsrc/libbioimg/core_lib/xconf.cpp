/*******************************************************************************
 Configuration parameters from command line

 Author: Dima V. Fedorov <mailto:dima@dimin.net> <http://www.dimin.net/>

 History:
   08/08/2001 21:53:31 - First creation

 Ver : 1
*******************************************************************************/

#include <cstring>
#include <cstdio>

#include <string>
#include <vector>
#include <algorithm>
#include <iostream>

#include "xstring.h"
#include "xconf.h"

//--------------------------------------------------------------------------------------
// XConf
//--------------------------------------------------------------------------------------

void XConf::init() {
  arguments_defs.clear();
  arguments.clear();
}

void XConf::appendArgumentDefinition( const std::string &key, int number_values, const xstring &description ) {
  arguments_defs.insert( make_pair( key, number_values ) );
  arguments_descr[key] = description;
}

//--------------------------------------------------------------------------------------

// this function scans through comma separated strings and appends them to the vector
std::vector<xstring> XConf::scan_strings( char *line ) {
  std::vector<xstring> v;
  xstring s;

  while ( strstr( line, "," ) ) {
    s = line;
    int pos = s.find(",");    
    v.push_back( s.substr(0,pos) );
    line = strstr( line, "," ) + 1;
  }
  if (strlen(line)>0) v.push_back( line );

  return v;
}

int XConf::readParams( int argc, char** argv ) {
  init();
  if (argc<2) return 1;
  int i=0;
  while (i<argc-1) {
    i++;
    xstring key = argv[i];
    key = key.toLowerCase();
    std::map<xstring, int>::const_iterator it = arguments_defs.find( key ); 
    if (it == arguments_defs.end()) continue;
    std::vector<xstring> strs;

    std::map< xstring, std::vector<xstring> >::iterator in_it = arguments.find( key ); 
    if ( in_it == arguments.end() ) {
      arguments.insert( make_pair( key, strs ) );
      in_it = arguments.find( key ); 
    }

    int n = (*it).second;
    if (n < 0) {
      i++;
      if (argc-i < 1) break;
      strs = scan_strings( argv[i] );
    } else
    for (int p=0; p<n; p++) {
      i++;
      if (argc-i < 1) break;
      strs.push_back( argv[i] );
    }

    for (int x=0; x<strs.size(); x++)  
      in_it->second.push_back( strs[x] );

  } // while (i<argc-1)

  processArguments();
  cureParams();
  return 0;
}

//--------------------------------------------------------------------------------------

bool XConf::keyExists( const std::string &key ) const {
  std::map<xstring, std::vector<xstring> >::const_iterator it = arguments.find( key ); 
  return (it != arguments.end());
}

std::vector<xstring> XConf::getValues( const std::string &key ) const {
  std::vector<xstring> v;
  std::map<xstring, std::vector<xstring> >::const_iterator it = arguments.find( key ); 
  if (it == arguments.end()) return v;
  return (*it).second;
}

std::vector<int> XConf::getValuesInt( const std::string &key, int def ) const {
  std::vector<int> vi;
  std::vector<xstring> v = getValues( key );
  for (int i=0; i<v.size(); i++)
    vi.push_back( v[i].toInt(def) );
  return vi;
}

std::vector<double> XConf::getValuesDouble( const std::string &key, double def ) const {
  std::vector<double> vi;
  std::vector<xstring> v = getValues( key );
  for (int i=0; i<v.size(); i++)
    vi.push_back( v[i].toDouble(def) );
  return vi;
}

std::vector<xstring> XConf::splitValue( const std::string &key, const std::string &def, const xstring &separator ) const {
  if (!keyExists(key)) return std::vector<xstring>();
  return getValue(key, def).split(separator);
}

std::vector<int> XConf::splitValueInt( const std::string &key, int def, const xstring &separator ) const {
  if (!keyExists(key)) return std::vector<int>();
  std::vector<xstring> v = getValue(key).split(separator);
  std::vector<int> v2;
  for (int i=0; i<v.size(); ++i)
    v2.push_back( v[i].toInt(def) );
  return v2;
}

std::vector<double> XConf::splitValueDouble( const std::string &key, double def, const xstring &separator ) const {
  if (!keyExists(key)) return std::vector<double>();
  std::vector<xstring> v = getValue(key).split(separator);
  std::vector<double> v2;
  for (int i=0; i<v.size(); ++i)
    v2.push_back( v[i].toDouble(def) );
  return v2;
}

xstring XConf::getValue( const std::string &key, const std::string &def ) const {
  xstring v = def;
  std::map<xstring, std::vector<xstring> >::const_iterator it = arguments.find( key ); 
  if (it == arguments.end()) return v;
  std::vector<xstring> strs = (*it).second;
  if (strs.size()<1) return v;
  return strs[0];
}

int XConf::getValueInt( const std::string &key, int def ) const {
  xstring vs = getValue( key, "" );
  return vs.toInt(def);
}

double XConf::getValueDouble( const std::string &key, double def ) const {
  xstring vs = getValue( key, "" );
  return vs.toDouble(def);
}

//--------------------------------------------------------------------------------------

xstring XConf::usage() {
  xstring str;
  
  int max_key_size = 0;
  std::map<xstring, int>::const_iterator it = arguments_defs.begin(); 
  while (it != arguments_defs.end() ) {
    max_key_size = std::max<int>(max_key_size, it->first.size());
    ++it;
  }
  //xstring key_format = xstring::xprintf( "%%%ds - ", max_key_size);

  it = arguments_defs.begin(); 
  while (it != arguments_defs.end() ) {
    //str += xstring::xprintf( key_format.c_str(), it->first.c_str() );
    xstring k = it->first;
    k.resize(max_key_size, ' ');
    str += k;
    str += " - ";

    std::map<xstring, xstring>::const_iterator itD = arguments_descr.find( it->first ); 
    if (itD != arguments_descr.end()) 
      str += itD->second; 
    else 
      str += "(no description)";

    str += "\n\n";
    ++it;
  }
  return str;
}

//--------------------------------------------------------------------------------------
// EXConf
//--------------------------------------------------------------------------------------

void EXConf::init() {
  XConf::init();
  appendArgumentDefinition( "-i", 1 );
  appendArgumentDefinition( "-o", 1 );
  appendArgumentDefinition( "-v", 0 );
  appendArgumentDefinition( "-par", -1 );
}

void EXConf::processArguments() {
  file_input  = getValue("-i");
  file_output = getValue("-o");
  parameters  = getValuesInt("-par");
}