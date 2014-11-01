#ifndef PARSE_H
#define PARSE_H

// $Date: 2007-10-30 21:49:36 -0400 (Tue, 30 Oct 2007) $
// $Revision: 435 $

/*
videoIO: granting easy, flexible, and efficient read/write access to video 
files in Matlab on Windows and GNU/Linux platforms.

Copyright (c) 2006 Gerald Dalley

Permission is hereby granted, free of charge, to any person obtaining a copy 
of this software and associated documentation files (the "Software"), to deal 
in the Software without restriction, including without limitation the rights 
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell 
copies of the Software, and to permit persons to whom the Software is 
furnished to do so, subject to the following conditions:

Portions of this software link to code licensed under the Gnu General 
Public License (GPL).  As such, they must be licensed by the more 
restrictive GPL license rather than this MIT license.  If you compile 
those files, this library and any code of yours that uses it automatically
becomes subject to the GPL conditions.  Any source files supplied by 
this library that bear this restriction are clearly marked with internal
comments.

The above copyright notice and this permission notice shall be included in all 
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE 
SOFTWARE.
*/

#include "debug.h"

namespace VideoIO
{
  class KeyValueMap : public std::map<std::string, std::string> 
  {
  public:
    inline bool hasKey(const char *key) const {
      TRACE;
      KeyValueMap::const_iterator iter = find(key);
      return (iter != end() && iter->second.size() > 0);
    }

    template <class T>
    inline T parseInt(const char *key) const { 
      TRACE;
      char const *str = find(key)->second.c_str();
      char *errLoc = NULL;
      errno = 0;
      long int tmpVal = strtol(str, &errLoc, 0);
      VrRecoverableCheckMsg(errno != EINVAL,  
        "Could not convert " << key << "'s value of " << str << 
        " to an integer."); 
      VrRecoverableCheckMsg(errno != ERANGE,  
        "Out-of-range error when attempting to convert " << key << 
        "'s value of " << str << " to an integer.");
      VrRecoverableCheckMsg(*errLoc == '\0',  
        "Parse error at character " << errLoc - str << 
        " when attempting to convert " << key << "'s value of " << 
        str << " to an integer.");
      return (T)tmpVal;
    }

    template <class T>
    inline T parseFloat(const char *key) const {
      TRACE;
      char const *str = find(key)->second.c_str();
      char *endLoc = NULL;
      errno = 0;
      double tmpVal = strtod(str, &endLoc);
      VrRecoverableCheckMsg(strlen(str) == (endLoc-str),  
        "Invalid character at position " << (endLoc - str) << " in \"" <<
        str << "\" when attempting to extract a double value.");
      VrRecoverableCheckMsg(!((tmpVal == HUGE_VAL) && (errno == ERANGE)), 
        "Overflow error in parsing \"" << str << "\".");
      VrRecoverableCheckMsg(!((tmpVal == 0) && (errno == ERANGE)), 
        "Underflow error in parsing \"" << str << "\".");
      return (T)tmpVal;
    }

    // Sometimes we want to require that arguments come in pairs, for
    // example if "fpsNum" is specified, we also need "fpsDenom".  This
    // function does both-or-nothing parsing.
    template <class I>
    bool intPairParse(const char *n1, I &i1, const char *n2, I &i2) const {
      const int nargs = hasKey(n1) + hasKey(n2);
      VrRecoverableCheckMsg(nargs == 2 || nargs == 0, 
        "Either both or neither of \"" << n1 << "\" and \"" << n2 << 
        "\" must be supplied.");
      if (nargs == 2) {
        i1 = parseInt<I>(n1);
        i2 = parseInt<I>(n2);
        return true;
      }
      return false;
    };

    // allows NTSC (29.97) to be represented exactly
    static const int DEFAULT_FPS_DENOM = 1000000; // ffmpeg's favorite denom

    bool fpsParse(int &num, int &denom) const {
      TRACE;
      if (hasKey("framesPerSecond")) {
        denom = DEFAULT_FPS_DENOM;
        num   = (int)(parseFloat<double>("framesPerSecond") * denom);
        return true;
      }
      if (hasKey("fps")) {
        denom = DEFAULT_FPS_DENOM;
        num   = (int)(parseFloat<double>("fps") * denom);
        return true;
      }
      if (intPairParse("framesPerSecond_num", num,
                       "framesPerSecond_denom", denom)) {
        return true;
      }
      if (intPairParse("fpsNum",num, "fpsDenom",denom)) {
        return true;
      }
      return false;
    }
  }; /* namespace VideoIO */

};

#endif
