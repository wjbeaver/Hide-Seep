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

#ifndef DREGEXP_H
#define DREGEXP_H

#include <QtCore>

class DRegExp : public QRegExp {
public:
  DRegExp();
  DRegExp(const QString &pattern, Qt::CaseSensitivity cs = Qt::CaseSensitive, PatternSyntax syntax = QRegExp::RegExp);
  DRegExp(const DRegExp &rx);
  DRegExp &operator=(const DRegExp &rx);

  void setPattern(const QString &pattern);

  const QHash<QString, QString> &groups() { generateGroups(); return cap_groups; }
  const QHash<QString, QString> &groups(const QString &str, int offset = 0, CaretMode caretMode = CaretAtZero);

protected:
  QString parsePattern( const QString &pattern );
  void generateGroups();

private:
  QStringList tag_names;
  QHash<QString, QString> cap_groups;
};

#endif // DREGEXP_H

