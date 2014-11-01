/*******************************************************************************

  DRegExp is the same as QRegExp except it allows group syntax (from Python)

  DRegExp extends QRegExp with python: (?P<name>...)

  <<< http://docs.python.org/lib/re-syntax.html
  (?P<name>...)   
  Similar to regular parentheses, but the substring matched by the group is 
  accessible via the symbolic group name name. Group names must be valid 
  Python identifiers, and each group name must be defined only once within 
  a regular expression. A symbolic group is also a numbered group, just as 
  if the group were not named. So the group named 'id' in the example above 
  can also be referenced as the numbered group 1. 
  
  For example, if the pattern is (?P<id>[a-zA-Z_]\w*), the group can be 
  referenced by its name in arguments to methods of match objects, such as 
  m.group('id') or m.end('id'), and also by name in pattern text 
  (for example, (?P=id)) and replacement text (such as \g<id>). 
  >>>
  
  Groups generated are accessible through "groups" methods

  ---------------------------------------------------------------------------

  Ex: Say you want to parse file name and get particular values with names:
  fileName: "2007-2-5.tif"
  pattern:  "(?P<year>[0-9]*)-(?P<month>[0-9]*)-(?P<day>[0-9]*)"
  output hash: { year: 2007, month: 2, day: 5 }

  Author: Dima Fedorov Levit <dimin@dimin.net> <http://www.dimin.net/>
  Copyright (C) BioImage Informatics <www.bioimage.ucsb.edu>

  History:
    2007-08-02 15:32 - First creation
      
  ver: 1
       
*******************************************************************************/

#include "d_reg_exp.h"

DRegExp::DRegExp(): QRegExp() {}

DRegExp::DRegExp(const DRegExp &rx)
: QRegExp(rx) 
{
  operator=(rx);
}

DRegExp &DRegExp::operator=(const DRegExp &rx) {

  QRegExp::operator=(rx);
  this->tag_names = rx.tag_names;
  return *this;
}

DRegExp::DRegExp(const QString &pattern, Qt::CaseSensitivity cs, PatternSyntax syntax)
: QRegExp( "", cs, syntax ) 
{
  setPattern(pattern);
}

void DRegExp::setPattern(const QString &pattern) {
  QRegExp::setPattern( parsePattern(pattern) );
}

QString DRegExp::parsePattern( const QString &_pattern ) {

  QList<int> poss, sizs;
  QString pattern = _pattern;

  // problem with these patterns - they will give eventual any character in the matched begining
  QString pts_pattern = "(^\\(|[^\\\\]\\()"; // matches only ( not \(
  QString tag_pattern = "(^\\(\\?P<\\w*>|[^\\\\]\\(\\?P<\\w*>)"; // matches only (?P<...> not \(?P<...>

  tag_names.clear();

  QRegExp rx, rx2;
  rx.setPattern( pts_pattern );
  rx2.setPattern( tag_pattern );
  int pos = 0;
  while ((pos = rx.indexIn(pattern, pos)) != -1) {
     
     int newpos = rx2.indexIn(pattern, pos);

     // if (?P<> found
     if (pos == newpos) {
       QString str = rx2.cap();
       if (!str.startsWith("(?P<")) {
         str = str.remove(0, 1); 
         pos++;
       }
       poss += pos;
       sizs += str.size();

       str = str.remove("(?P<");
       str = str.remove(">");
       tag_names += str;
       pos += rx2.matchedLength();
     } else {
       // if (?P<> was not found create unknown tag, it will go to the dictionary too
       tag_names += "__noname__";
       pos += rx.matchedLength();
     }
  }
  
 //return pattern.replace( rx, "(" );    
  for (int i=poss.size()-1; i>=0; --i)
    pattern = pattern.replace( poss[i], sizs[i], "(" );
  return pattern;
}

void DRegExp::generateGroups() {

  cap_groups.clear();
  QStringList caps = this->capturedTexts();
  int num_tags = qMin( caps.size()-1, tag_names.size() );
  for (unsigned int i=0; i<num_tags; ++i)
    if ( !caps[i+1].isEmpty() && tag_names[i] != "__noname__" )
      cap_groups.insert( tag_names[i], caps[i+1] );
}

const QHash<QString, QString> &DRegExp::groups(const QString &str, int offset, CaretMode caretMode) {
 
  this->indexIn(str, offset, caretMode);
  this->generateGroups(); 
  return cap_groups;
}


