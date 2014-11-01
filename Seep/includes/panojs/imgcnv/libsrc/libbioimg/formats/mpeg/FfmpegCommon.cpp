// $Date: 2007-06-12 23:51:57 -0400 (Tue, 12 Jun 2007) $
// $Revision: 408 $

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

#include "FfmpegCommon.h"

#include <stdio.h>
#include <stdlib.h>

// FFMPEG Includes
extern "C" {
#ifdef FFMPEG_2007_VERSION
#include <ffmpeg/avcodec.h>
#include <ffmpeg/avformat.h>
#else
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#endif
};

#if (LIBAVUTIL_VERSION_MAJOR < 49) && (LIBAVUTIL_VERSION_INT < ((49<<16)+(2<<8)+0))
// At svn revision 7614, the expanded log levels were introduced.  This 
// approximately corresponds to libavutil version 49.2.0.
#  define AV_LOG_WARNING AV_LOG_ERROR // upgrade warnings to errors
#  define AV_LOG_VERBOSE (AV_LOG_DEBUG-1)
#endif

using namespace std;

namespace VideoIO 
{

  char const *averror(int errCode) 
  {
#define SELFCASE(C) case C: return #C
    if (errCode >= 0) return NULL;
    switch (errCode) {
      SELFCASE(AVERROR_UNKNOWN);
      SELFCASE(AVERROR_IO);
      SELFCASE(AVERROR_NUMEXPECTED);
#if (AVERROR_UNKNOWN != AVERROR_INVALIDDATA)
      SELFCASE(AVERROR_INVALIDDATA);
#endif
      SELFCASE(AVERROR_NOMEM);
      SELFCASE(AVERROR_NOFMT);
      SELFCASE(AVERROR_NOTSUPP);
    default:
      return "Unrecognized ffmpeg error code.";
    }
  }


  // Accumulator for log messages
  static char realBuffer[16384];
  char * const ffBuffer = realBuffer;

  // ffmpeg logging callback (see <ffmpeg/log.h>)
  static void ffLog(void *avcl, int level, const char *fmt, va_list ap) {
    static int printPrefix = 1;
    if (level < AV_LOG_VERBOSE) {
      AVClass* avc = avcl ? *(AVClass**)avcl : NULL;
      
      char tmp[sizeof(realBuffer)];
      tmp[sizeof(tmp)-1] = '\0';
      
      if (printPrefix && avc) {
        strncat(ffBuffer, "                       ", sizeof(realBuffer)-1);
        sprintf(tmp, "[%s @ %p] ", avc->item_name(avcl), avc);
      }
      
      printPrefix = (strstr(fmt, "\n") != NULL);
      
      const size_t len = strlen(tmp);
      vsnprintf(tmp+len, sizeof(tmp)-1-len, fmt, ap);
      if (level <= AV_LOG_ERROR) {
        PRINTERROR(tmp);
      } else if (level <= AV_LOG_WARNING) {
        PRINTWARN(tmp);
      } else if (level <= AV_LOG_INFO) {
        PRINTINFO(tmp);
      } else if (level <= AV_LOG_VERBOSE) {
        VERBOSE(tmp);
      }
      
      strncat(ffBuffer, tmp, sizeof(realBuffer)-1); 
    }
  }
  
  static void hijackLog() {
    memset(ffBuffer, 0, sizeof(realBuffer));
#if (LIBAVUTIL_VERSION_MAJOR >= 49) || (LIBAVUTIL_VERSION_INT > (50<<16))
    av_log_set_callback(ffLog);
#else
    av_vlog = ffLog;
#endif
  }

  bool operator< (FfmpegFormatName const &a, FfmpegFormatName const &b)
  {
    if (a.name < b.name) return true;
    if (a.name == b.name) {
      return (a.longName < b.longName);
    } else {
      return false;
    }
  }

  static set<FfmpegFormatName> iformats, oformats;
  static set<string>           ishort,   oshort;
  static set<string>           icodecs,  ocodecs;
  static set<string>           protocols;

  // adapted from show_formats in ffmpeg.c
  static void enumerateFormats()
  {
    iformats.clear(); oformats.clear();
    ishort.clear();   oshort.clear();
    icodecs.clear();  ocodecs.clear();

    AVInputFormat *ifmt=NULL;
    AVOutputFormat *ofmt=NULL;
    URLProtocol *up=NULL;
    AVCodec *p=NULL, *p2;
    const char **pp, *last_name;
    
    // formats
    last_name= "000";
    while (true) {
      bool        decode    = false;
      bool        encode    = false;
      const char *name      = NULL;
      const char *long_name = NULL;

      while((ofmt= av_oformat_next(ofmt))) {
          if((name == NULL || strcmp(ofmt->name, name)<0) &&
              strcmp(ofmt->name, last_name)>0){
              name= ofmt->name;
              long_name= ofmt->long_name;
              encode=1;
          }
      }

      while((ifmt= av_iformat_next(ifmt))) {
          if((name == NULL || strcmp(ifmt->name, name)<0) &&
              strcmp(ifmt->name, last_name)>0){
              name= ifmt->name;
              long_name= ifmt->long_name;
              encode=0;
          }
          if(name && strcmp(ifmt->name, name)==0)
              decode=1;
      }

      if(name==NULL) break;
      last_name= name;
            
      if (encode) {
        oformats.insert(FfmpegFormatName(name, long_name));
        oshort.insert(name);
      }
      if (decode) {
        iformats.insert(FfmpegFormatName(name, long_name));
        ishort.insert(name);
      }
    }

    // codecs 
    last_name= "000";
    p=NULL;
    while (true) {
      int decode = 0;
      int encode = 0;
      int cap    = 0; // ignored for now
      const char *type_str;

      p2=NULL;
      while((p= av_codec_next(p))) {
          if((p2==NULL || strcmp(p->name, p2->name)<0) &&
              strcmp(p->name, last_name)>0){
              p2= p;
              decode= encode= cap=0;
          }
          if(p2 && strcmp(p->name, p2->name)==0){
              if(p->decode) decode=1;
              if(p->encode) encode=1;
              cap |= p->capabilities;
          }
      }
      if(p2==NULL) break;
      last_name= p2->name;

      if (p2->type == CODEC_TYPE_VIDEO) {
        if (encode) ocodecs.insert(p2->name);
        if (decode) icodecs.insert(p2->name);
      }
    }

    // file protocols
    while((up = av_protocol_next(up)))
      protocols.insert(up->name);
  }

  set<FfmpegFormatName> const &getFfmpegInputFileFormats() {
    ffmpegInitIfNeeded();
    return iformats;
  }

  set<string> const &getFfmpegInputFileFormatShortNames() {
    ffmpegInitIfNeeded();
    return ishort;
  }

  set<string> const &getFfmpegInputCodecNames() {
    ffmpegInitIfNeeded();
    return icodecs;
  }

  set<FfmpegFormatName>  const &getFfmpegOutputFileFormats() {
    ffmpegInitIfNeeded();
    return oformats;
  }

  set<string> const &getFfmpegOutputFileFormatShortNames() {
    ffmpegInitIfNeeded();
    return oshort;
  }

  set<string> const &getFfmpegOutputCodecNames() {
    ffmpegInitIfNeeded();
    return ocodecs;
  }

  void ffmpegInitIfNeeded()
  {
    static bool registered = false;
    if (!registered) {
      av_register_all();
      hijackLog();
      enumerateFormats();
      registered = true;
    }
  }

  AVCodecContext *getCodecFromStream(AVStream *s) { 
#if (LIBAVCODEC_VERSION_MAJOR < 52) && (LIBAVCODEC_VERSION_INT < ((51<<16)+(7<<8)+0))
    return &s->codec;
#else
    return s->codec;
#endif
  }
  
#if (!((LIBAVCODEC_VERSION_MAJOR >= 52) || (LIBAVCODEC_VERSION_INT > 0x000409) || (LIBAVCODEC_VERSION_INT == 0x000409 && LIBAVCODEC_BUILD >= 4754)))
  // av_rescale_q was introduced in svn revision 4168
  int64_t av_rescale_q(int64_t a, AVRational bq, AVRational cq) {
    int64_t b= bq.num * (int64_t)cq.den;
    int64_t c= cq.num * (int64_t)bq.den;
    return av_rescale_rnd(a, b, c, AV_ROUND_NEAR_INF);
  }
#endif

};
