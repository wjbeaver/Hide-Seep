/*****************************************************************************
  NANOSCOPE format support
  UCSB/BioITR property
  Copyright (c) 2005 by Dmitry V. Fedorov <www.dimin.net> <dima@dimin.net>

  IMPLEMENTATION
  
  Programmer: Dima V. Fedorov <mailto:dima@dimin.net> <http://www.dimin.net/>

  History:
    01/10/2005 12:17 - First creation
    02/08/2005 22:30 - Update for new magic
    09/12/2005 17:34 - updated to api version 1.3
            
  Ver : 3
*****************************************************************************/

#ifndef DIM_NANOSCOPE_FORMAT_H
#define DIM_NANOSCOPE_FORMAT_H

#include <dim_img_format_interface.h>
#include <dim_img_format_utils.h>

#include <stdio.h>
#include <vector>
#include <string>


// DLL EXPORT FUNCTION
extern "C" {
TDimFormatHeader* dimNanoscopeGetFormatHeader(void);
}

//----------------------------------------------------------------------------
// Internal Format Structs
//----------------------------------------------------------------------------
void             dimNanoscopeCloseImageProc     ( TDimFormatHandle *fmtHndl);

//----------------------------------------------------------------------------
// Internal Format Structs
//----------------------------------------------------------------------------

#define DIM_NANOSCOPE_MAGIC_SIZE 11
const char nanoscopeMagic[12]  = "\\*File list";
const char nanoscopeMagicF[18] = "\\*Force file list";

class DNanoscopeImg {
public:
  DNanoscopeImg(): width(0), height(0), data_offset(0), xR(0.0), yR(0.0), zR(0.0) {}

  int width, height;
  long data_offset;
  std::string metaText;
  double xR, yR, zR; // pixel resolution for XYZ
  std::string data_type;
};

typedef std::vector<DNanoscopeImg> DNanoImgVector;

class DNanoscopeParams {
public:
  DNanoscopeParams(): channels(1) { i = initTDimImageInfo(); }
  TDimImageInfo i;
  std::string metaText;
  DNanoImgVector imgs;
  int channels;
};

//----------------------------------------------------------------------------
// TDimLineReader
//----------------------------------------------------------------------------

#define DIM_LINE_BUF_SIZE 2048

class TDimLineReader
{
public:
  TDimLineReader( TDimFormatHandle *newHndl );
  ~TDimLineReader();

  std::string line(); 
  const char* line_c_str(); 

  bool readNextLine();
  bool isEOLines();

protected:
  std::string prline;

private:
  TDimFormatHandle *fmtHndl;
  char buf[DIM_LINE_BUF_SIZE];
  int  bufpos;
  int  bufsize;
  bool eolines;

  bool loadBuff();
};


class TDimTokenReader: public TDimLineReader
{
public:
  TDimTokenReader( TDimFormatHandle *newHndl );

  bool isKeyTag   ();
  bool isImageTag ();
  bool isEndTag   ();
  bool compareTag ( const char *tag ); // compare exactly the tag
  bool isTag ( const char *tag );      // only verify if this token contains tag
  int  readParamInt ( const char *tag );
  void readTwoParamDouble ( const char *tag, double *p1, double *p2 );
  void readTwoDoubleRemString ( const char *tag, double *p1, double *p2, std::string *str );
  std::string readImageDataString ( const char *tag );
};


#endif // DIM_NANOSCOPE_FORMAT_H
