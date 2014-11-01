/*****************************************************************************
  NANOSCOPE format support 
  UCSB/BioITR property
  Copyright (c) 2004 by Dmitry V. Fedorov <www.dimin.net> <dima@dimin.net>

  IMPLEMENTATION
  
  Programmer: Dima V. Fedorov <mailto:dima@dimin.net> <http://www.dimin.net/>

  History:
    01/10/2005 12:17 - First creation
    02/08/2005 22:30 - Support for incomplete image sections
        
  Ver : 2
*****************************************************************************/

#include <cstdio>
#include <cstdlib>
#include <cstring>


#include "dim_nanoscope_format.h"
#include "dim_nanoscope_format_io.cpp"



//****************************************************************************
// MISC
//****************************************************************************


//****************************************************************************
//
// INTERNAL STRUCTURES
//
//****************************************************************************

void addDefaultKeyTag(const char *token, DNanoscopeParams *nanoPar)
{
  nanoPar->metaText.append("[");
  nanoPar->metaText.append( token+2 );
  nanoPar->metaText.append("]\n");
}

void addDefaultTag(const char *token, DNanoscopeParams *nanoPar)
{
  nanoPar->metaText.append( token+1 );
  nanoPar->metaText.append("\n");
}

void addImgKeyTag(const char *token, DNanoscopeParams *nanoPar)
{
  nanoPar->imgs.rbegin()->metaText.append("[");
  nanoPar->imgs.rbegin()->metaText.append( token+2 );
  nanoPar->imgs.rbegin()->metaText.append("]\n");
}

void addImgTag(const char *token, DNanoscopeParams *nanoPar)
{
  nanoPar->imgs.rbegin()->metaText.append( token+1 );
  nanoPar->imgs.rbegin()->metaText.append("\n");
}

void nanoscopeGetImageInfo( TDimFormatHandle *fmtHndl )
{
  bool inimage = false;

  if (fmtHndl == NULL) return;
  if (fmtHndl->internalParams == NULL) return;
  DNanoscopeParams *nanoPar = (DNanoscopeParams *) fmtHndl->internalParams;
  TDimImageInfo *info = &nanoPar->i; 
  
  info->ver = sizeof(TDimImageInfo);
  info->imageMode = DIM_GRAYSCALE;
  info->tileWidth = 0;
  info->tileHeight = 0; 
  info->transparentIndex = 0;
  info->transparencyMatting = 0;
  info->lut.count = 0;
  info->samples = 1;
  info->depth = 16;

  if (fmtHndl->stream == NULL) return;

  TDimTokenReader tr( fmtHndl );
  tr.readNextLine();

  while (!tr.isEOLines())
  {
    if ( tr.isImageTag() )
    {
      // now add new image
      nanoPar->imgs.push_back( DNanoscopeImg() );
      addImgKeyTag( tr.line_c_str(), nanoPar );
      inimage=true;
      tr.readNextLine();
      continue;
    }
    
    if (!inimage)
    { // if capturing default parameters
      if ( tr.isKeyTag() )
        addDefaultKeyTag(tr.line_c_str(), nanoPar); // add key meta data tag
      else
        addDefaultTag(tr.line_c_str(), nanoPar);    // add simple meta data tag
    }
    else
    { // if capturing image parameters
      if ( tr.isTag( "\\Data offset:" ) )
        nanoPar->imgs.rbegin()->data_offset = tr.readParamInt ( "\\Data offset:" );
      else
      if ( tr.isTag( "\\Samps/line:" ) )
        nanoPar->imgs.rbegin()->width = tr.readParamInt ( "\\Samps/line:" );
      else
      if ( tr.isTag( "\\Number of lines:" ) )
        nanoPar->imgs.rbegin()->height = tr.readParamInt ( "\\Number of lines:" );
      else
        addImgTag(tr.line_c_str(), nanoPar);


      // parse metadata
      //\Scan size: 15 15 ~m - microns
      //\Scan size: 353.381 353.381 nm - nanometers
      if ( tr.isTag( "\\Scan size:" ) ) {
        //tr.readTwoParamDouble ( "\\Scan size:", &nanoPar->imgs.rbegin()->xR, &nanoPar->imgs.rbegin()->yR );
        
        std::string str;
        tr.readTwoDoubleRemString( "\\Scan size:", &nanoPar->imgs.rbegin()->xR, &nanoPar->imgs.rbegin()->yR, &str );
        
        // if size is in nm then convert to um
        if ( strncmp( str.c_str(), "nm", 2 ) == 0 ) {
          nanoPar->imgs.rbegin()->xR /= 1000.0;
          nanoPar->imgs.rbegin()->yR /= 1000.0;
        }
      }

      //\@2:Image Data: S [Height] "Height"
      if ( tr.isTag( "\\@2:Image Data:" ) ) {
        nanoPar->imgs.rbegin()->data_type = tr.readImageDataString ( "\\@2:Image Data:" );
      }
    }

    tr.readNextLine();
  } // while
  
  // walk trough image list and verify it's consistency
  DNanoImgVector::iterator imgit = nanoPar->imgs.begin();

  while (imgit < nanoPar->imgs.end() )
  {
    if ( (imgit->width <= 0) || (imgit->height <= 0) || (imgit->data_offset <= 0) )
    {
      // before removing add metadata to image pool
      nanoPar->metaText.append( imgit->metaText.c_str() );
      nanoPar->imgs.erase(imgit);
    }
    else
      ++imgit;
  }

  // now finalize info
  DNanoscopeImg nimg = nanoPar->imgs.at(0);
  if (fmtHndl->pageNumber < nanoPar->imgs.size())
    nimg = nanoPar->imgs.at( fmtHndl->pageNumber );

  info->width  = nimg.width;
  info->height = nimg.height;
  info->samples = 1;
  nanoPar->channels = 1;
  info->number_pages = nanoPar->imgs.size();
  //info->number_z = info->number_pages;

  // decide reading as pages or channels
  /*
  bool same_size = true;
  for (int i=0; i<nanoPar->imgs.size(); ++i) {
    DNanoscopeImg nimgt = nanoPar->imgs.at(i);
    if (info->width != nimgt.width) { same_size = false; break; }
    if (info->height != nimgt.height) { same_size = false; break; }
  }

  if ( same_size && nanoPar->imgs.size()<=3 ) {
    info->number_pages = 1;
    info->samples = nanoPar->imgs.size();
  } else {
    info->number_pages = nanoPar->imgs.size();
    info->samples = 1;
  }
  nanoPar->channels = info->samples;
  */

  info->resUnits = DIM_RES_um;
  info->xRes = nimg.xR / nimg.width;
  info->yRes = nimg.yR / nimg.height;
}

//****************************************************************************
//
// FORMAT DEMANDED FUNTIONS
//
//****************************************************************************


//----------------------------------------------------------------------------
// PARAMETERS, INITS
//----------------------------------------------------------------------------

DIM_INT dimNanoscopeValidateFormatProc (DIM_MAGIC_STREAM *magic, DIM_UINT length) {
  if (length < DIM_NANOSCOPE_MAGIC_SIZE) return -1;
  if (memcmp( magic, nanoscopeMagic, DIM_NANOSCOPE_MAGIC_SIZE ) == 0) return 0;
  if (memcmp( magic, nanoscopeMagicF, DIM_NANOSCOPE_MAGIC_SIZE ) == 0) return 0;
  return -1;
}

TDimFormatHandle dimNanoscopeAquireFormatProc( void )
{
  TDimFormatHandle fp = initTDimFormatHandle();
  return fp;
}

void dimNanoscopeReleaseFormatProc (TDimFormatHandle *fmtHndl)
{
  if (fmtHndl == NULL) return;
  dimNanoscopeCloseImageProc ( fmtHndl );  
}


//----------------------------------------------------------------------------
// OPEN/CLOSE
//----------------------------------------------------------------------------
void dimNanoscopeCloseImageProc (TDimFormatHandle *fmtHndl)
{
  if (fmtHndl == NULL) return;
  dimClose ( fmtHndl );
  if (fmtHndl->internalParams != NULL) {
    DNanoscopeParams *nanoPar = (DNanoscopeParams *) fmtHndl->internalParams;
    delete nanoPar;
  }
  fmtHndl->internalParams = NULL;
}

DIM_UINT dimNanoscopeOpenImageProc  (TDimFormatHandle *fmtHndl, DIM_ImageIOModes io_mode)
{
  if (fmtHndl == NULL) return 1;
  if (fmtHndl->internalParams != NULL) dimNanoscopeCloseImageProc (fmtHndl);  
  fmtHndl->internalParams = (void *) new DNanoscopeParams();

  if (io_mode == DIM_IO_READ)
  {
    if ( isCustomReading ( fmtHndl ) != TRUE )
      fmtHndl->stream = fopen( fmtHndl->fileName, "rb" );

    if (fmtHndl->stream == NULL) return 1;
    nanoscopeGetImageInfo( fmtHndl );
  }
  else return 1;

  return 0;
}

//----------------------------------------------------------------------------
// INFO for OPEN image
//----------------------------------------------------------------------------

DIM_UINT dimNanoscopeGetNumPagesProc ( TDimFormatHandle *fmtHndl ) {
  if (fmtHndl == NULL) return 0;
  if (fmtHndl->internalParams == NULL) return 0;
  DNanoscopeParams *nanoPar = (DNanoscopeParams *) fmtHndl->internalParams;
  return nanoPar->imgs.size();
}


TDimImageInfo dimNanoscopeGetImageInfoProc ( TDimFormatHandle *fmtHndl, DIM_UINT  ) {
  TDimImageInfo ii = initTDimImageInfo();
  if (fmtHndl == NULL) return ii;
  DNanoscopeParams *nanoPar = (DNanoscopeParams *) fmtHndl->internalParams;
  return nanoPar->i;
}

//----------------------------------------------------------------------------
// METADATA
//----------------------------------------------------------------------------

DIM_UINT dimNanoscopeReadMetaDataProc (TDimFormatHandle *fmtHndl, DIM_UINT , int group, int tag, int type) {
  if (fmtHndl == NULL) return 1;
  return read_nanoscope_metadata (fmtHndl, group, tag, type);
}

char* dimNanoscopeReadMetaDataAsTextProc ( TDimFormatHandle *fmtHndl ) {
  if (fmtHndl == NULL) return NULL;
  return read_text_nanoscope_metadata ( fmtHndl );
}

DIM_UINT dimNanoscopeAddMetaDataProc (TDimFormatHandle *) {
  return 1;
}




//----------------------------------------------------------------------------
// READ/WRITE
//----------------------------------------------------------------------------

DIM_UINT dimNanoscopeReadImageProc  ( TDimFormatHandle *fmtHndl, DIM_UINT page ) {
  if (fmtHndl == NULL) return 1;
  if (fmtHndl->stream == NULL) return 1;
  fmtHndl->pageNumber = page;
  return read_nanoscope_image( fmtHndl );
}

DIM_UINT dimNanoscopeWriteImageProc ( TDimFormatHandle * ) {
  return 1;
}





//****************************************************************************
// TDimLineReader
//****************************************************************************

TDimLineReader::TDimLineReader( TDimFormatHandle *newHndl )
{
  fmtHndl = newHndl;
  
  if (fmtHndl == NULL) return;
  if (fmtHndl->stream == NULL) return;
  if ( dimSeek(fmtHndl, 0, SEEK_SET) != 0) return;
  eolines = false;
  loadBuff();
}

TDimLineReader::~TDimLineReader()
{

}

bool TDimLineReader::loadBuff()
{
  bufsize = dimRead( fmtHndl, buf, 1, DIM_LINE_BUF_SIZE );
  bufpos = 0;
  if (bufsize <= 0) return false;
  return true;
}

bool TDimLineReader::readNextLine()
{
  if (eolines) return false;
  prline = "";

  while ( (buf[bufpos] != 0xA) && (buf[bufpos] != 0xD) && (buf[bufpos] != 0x1A) ) 
  {
    if (bufpos >= bufsize) loadBuff();  
    if (bufpos >= bufsize) { eolines=true; break; }
    prline += buf[bufpos]; 
    ++bufpos;
  }

  if (buf[bufpos] == 0xA) ++bufpos;
  if (buf[bufpos] == 0xD) bufpos+=2;
  if (bufpos >= bufsize) loadBuff();  
  if (bufpos >= bufsize) eolines=true;
  if (buf[bufpos] == 0x1A) eolines=true;

  return true;
}

bool TDimLineReader::isEOLines()
{
  return eolines;
}

std::string TDimLineReader::line()
{
  return prline;
}

const char* TDimLineReader::line_c_str()
{
  return prline.c_str();
}

//****************************************************************************
// TDimTokenReader
//****************************************************************************

bool TDimTokenReader::isKeyTag()
{
  if ( (prline[0]=='\\') && (prline[1]=='*') ) return true;
  return false;
}

bool TDimTokenReader::isImageTag()
{
  if (isKeyTag())
  {
    //if (prline.compare("\\*Ciao force image list") == 0) return false;
    if (prline.find("image list", 0) != -1) 
      return true;
  }

  return false;
}

bool TDimTokenReader::isEndTag()
{
  if (prline.compare("\\*File list end") == 0) return true;
  return false;
}


bool TDimTokenReader::compareTag( const char *tag )
{
  if (prline.compare(tag) == 0) return true;
  return false;
}

bool TDimTokenReader::isTag ( const char *tag )
{
  if (prline.find(tag) != -1) return true;
  return false;
}

int TDimTokenReader::readParamInt ( const char *tag )
{
  int res=0;
  char ss[1024];
  sprintf(ss, "%s %%d", tag);

  sscanf( prline.c_str(), ss, &res );
  return res;
}

void TDimTokenReader::readTwoParamDouble ( const char *tag, double *p1, double *p2 ) {
  char ss[1024];

  float pA=-1, pB=-1;
  sprintf(ss, "%s %%f %%f", tag);
  sscanf( prline.c_str(), ss, &pA, &pB );

  *p1 = pA; *p2 = pB;
}

void TDimTokenReader::readTwoDoubleRemString ( const char *tag, double *p1, double *p2, std::string *str ) {
  char ss[1024], st[1024];

  float pA=-1, pB=-1;
  sprintf(ss, "%s %%f %%f %%s", tag);
  sscanf( prline.c_str(), ss, &pA, &pB, st );

  *p1 = pA; *p2 = pB;
  *str = st;
}

std::string TDimTokenReader::readImageDataString ( const char *tag ) {
  xstring s = prline;
  //std::string str = s.section("[", "]");
  return s.section("\"", "\"");
}


TDimTokenReader::TDimTokenReader( TDimFormatHandle *newHndl )
                : TDimLineReader( newHndl )
{


}


//****************************************************************************
//
// EXPORTED FUNCTION
//
//****************************************************************************

TDimFormatItem dimNanoscopeItems[1] = {
  {
    "NANOSCOPE",            // short name, no spaces
    "NanoScope II/III file", // Long format name
    "nan",        // pipe "|" separated supported extension list
    1, //canRead;      // 0 - NO, 1 - YES
    0, //canWrite;     // 0 - NO, 1 - YES
    1, //canReadMeta;  // 0 - NO, 1 - YES
    0, //canWriteMeta; // 0 - NO, 1 - YES
    0, //canWriteMultiPage;   // 0 - NO, 1 - YES
    //TDivFormatConstrains constrains ( w, h, pages, minsampl, maxsampl, minbitsampl, maxbitsampl, noLut )
    { 0, 0, 0, 1, 1, 16, 16, 1 } 
  }
};

TDimFormatHeader dimNanoscopeHeader = {

  sizeof(TDimFormatHeader),
  "1.0.1",
  "NANOSCOPE CODEC",
  "NANOSCOPE CODEC",
  
  12,                      // 0 or more, specify number of bytes needed to identify the file
  {1, 1, dimNanoscopeItems},   //dimJpegSupported,
  
  dimNanoscopeValidateFormatProc,
  // begin
  dimNanoscopeAquireFormatProc, //TDimAquireFormatProc
  // end
  dimNanoscopeReleaseFormatProc, //TDimReleaseFormatProc
  
  // params
  NULL, //TDimAquireIntParamsProc
  NULL, //TDimLoadFormatParamsProc
  NULL, //TDimStoreFormatParamsProc

  // image begin
  dimNanoscopeOpenImageProc, //TDimOpenImageProc
  dimNanoscopeCloseImageProc, //TDimCloseImageProc 

  // info
  dimNanoscopeGetNumPagesProc, //TDimGetNumPagesProc
  dimNanoscopeGetImageInfoProc, //TDimGetImageInfoProc


  // read/write
  dimNanoscopeReadImageProc, //TDimReadImageProc 
  NULL, //TDimWriteImageProc
  NULL, //TDimReadImageTileProc
  NULL, //TDimWriteImageTileProc
  NULL, //TDimReadImageLineProc
  NULL, //TDimWriteImageLineProc
  NULL, //TDimReadImageThumbProc
  NULL, //TDimWriteImageThumbProc
  NULL, //dimJpegReadImagePreviewProc, //TDimReadImagePreviewProc
  
  // meta data
  dimNanoscopeReadMetaDataProc, //TDimReadMetaDataProc
  dimNanoscopeAddMetaDataProc,  //TDimAddMetaDataProc
  dimNanoscopeReadMetaDataAsTextProc, //TDimReadMetaDataAsTextProc
  nanoscope_append_metadata, //TDimAppendMetaDataProc

  NULL,
  NULL,
  ""

};

extern "C" {

TDimFormatHeader* dimNanoscopeGetFormatHeader(void)
{
  return &dimNanoscopeHeader;
}

} // extern C





