/*******************************************************************************

  Manager for Image Formats

  Uses DimFiSDK version: 1.2
  
  Programmer: Dima Fedorov Levit <dimin@dimin.net> <http://www.dimin.net/>

  History:
    03/23/2004 18:03 - First creation
    08/04/2004 18:22 - custom stream managment compliant
      
  ver: 2
        
*******************************************************************************/

#include "dim_format_manager.h"
#include "xstring.h"

// Disables Visual Studio 2005 warnings for deprecated code
#if ( defined(_MSC_VER) && (_MSC_VER >= 1400) )
  #pragma warning(disable:4996)
#endif 

// formats
#include "png/dim_png_format.h"
#include "jpeg/dim_jpeg_format.h"
#include "tiff/dim_tiff_format.h"
#include "biorad_pic/dim_biorad_pic_format.h"
#include "bmp/dim_bmp_format.h"
#include "nanoscope/dim_nanoscope_format.h"
#include "ibw/dim_ibw_format.h"
#include "ome/dim_ome_format.h"
#include "raw/dim_raw_format.h"
#ifdef DIM_FFMPEG_FORMAT
#include "mpeg/dim_ffmpeg_format.h"
#endif
#include "oib/dim_oib_format.h"


#include "dmemio.h"

TDimFormatManager::TDimFormatManager()
{
  progress_proc = NULL;
  error_proc = NULL;
  test_abort_proc = NULL;

  session_active = FALSE;
  sessionCurrentPage = 0;
  max_magic_size = 0;
  magic_number = NULL;
  sessionHandle = initTDimFormatHandle();
  sessionFormatIndex = 0;
  sessionSubIndex = 0;
  
  // add static formats, jpeg first updated: dima 07/21/2005 17:11
  addNewFormatHeader ( dimJpegGetFormatHeader( ) );
  addNewFormatHeader ( dimTiffGetFormatHeader( ) );
  addNewFormatHeader ( dimBioRadPicGetFormatHeader() );
  addNewFormatHeader ( dimBmpGetFormatHeader() );
  addNewFormatHeader ( dimPngGetFormatHeader() );
  addNewFormatHeader ( dimNanoscopeGetFormatHeader() );
  addNewFormatHeader ( dimIbwGetFormatHeader() );
  addNewFormatHeader ( dimOmeGetFormatHeader() );
  addNewFormatHeader ( dimRawGetFormatHeader() );
  addNewFormatHeader ( dimOibGetFormatHeader() );
  static_formats = 10;
  #ifdef DIM_FFMPEG_FORMAT
  addNewFormatHeader ( dimFFMpegGetFormatHeader() );
  static_formats += 1;
  #endif
}


TDimFormatManager::~TDimFormatManager()
{
  sessionEnd();
  formatList.clear();
}

inline TDimFormatManager &TDimFormatManager::operator=( TDimFormatManager fm )
{ 
  unsigned int i;

  if (fm.countInstalledFormats() > static_formats)
  for (i=static_formats; i<fm.countInstalledFormats(); i++) {
    this->addNewFormatHeader ( fm.getFormatHeader(i) );
  }

  return *this; 
}

TDimFormatHeader *TDimFormatManager::getFormatHeader(int i)
{
  if (i >= (int) formatList.size()) return NULL;
  return formatList.at( i ); 
}

unsigned int TDimFormatManager::countInstalledFormats() {
  return formatList.size();
}

void TDimFormatManager::addNewFormatHeader (TDimFormatHeader *nfh)
{
  formatList.push_back( nfh );
  setMaxMagicSize();
}

void TDimFormatManager::setMaxMagicSize()
{
  unsigned int i;

  for (i=0; i<formatList.size(); i++)
  {
    if (formatList.at(i)->neededMagicSize > max_magic_size)
      max_magic_size = formatList.at(i)->neededMagicSize;
  }

  if (magic_number != NULL) delete magic_number;
  magic_number = new unsigned char [ max_magic_size ];
}

bool TDimFormatManager::loadMagic(const char *fileName)
{
  memset( magic_number, 0, max_magic_size );

  FILE *in_stream = fopen(fileName, "rb");
  if (in_stream == NULL) return FALSE;
  fread(magic_number, sizeof(unsigned char), max_magic_size, in_stream);
  fclose(in_stream);
  return TRUE;
}

bool TDimFormatManager::loadMagic( DIM_STREAM_CLASS *stream, TDimSeekProc seekProc, TDimReadProc readProc )
{
  if ( (stream == NULL) || ( seekProc == NULL ) || ( readProc == NULL ) ) return FALSE;
  if ( seekProc ( stream, 0, SEEK_SET ) != 0 ) return FALSE;
  readProc ( magic_number, sizeof(unsigned char), max_magic_size, stream );
  return TRUE;
}

int TDimFormatManager::getNeededFormatByMagic()
{
  unsigned int i;

  for (i=0; i<formatList.size(); i++)
  {
    if (formatList.at(i)->validateFormatProc( magic_number, max_magic_size ) != -1)
      return i;
  }

  return -1;
}

#ifdef WIN32
#define strncmp strnicmp
#define strcmp stricmp
#else
#define strncmp strncasecmp
#define strcmp strcasecmp
#endif
void TDimFormatManager::getNeededFormatByName(const char *formatName, int &format_index, int &sub_index) {
  unsigned int i, s;
  format_index = -1;
  sub_index = -1;

  for (i=0; i<formatList.size(); i++)
    for (s=0; s<formatList.at(i)->supportedFormats.count; s++) {
      char *fmt_short_name = formatList.at(i)->supportedFormats.item[s].formatNameShort;
      if (strcmp(fmt_short_name, formatName) == 0) {
        format_index = i;
        sub_index = s;
        break;
      } // if strcmp
    } // for s

  return;
}
#undef strncmp
#undef strcmp

TDimFormatItem *TDimFormatManager::getFormatItem (const char *formatName)
{
  int format_index, sub_index;
  TDimFormatHeader *selectedFmt;
  TDimFormatItem   *subItem;
  
  getNeededFormatByName(formatName, format_index, sub_index);  
  if (format_index < 0) return NULL;
  if (sub_index < 0) return NULL;

  selectedFmt = formatList.at( format_index );
  if (sub_index >= (int)formatList.at(format_index)->supportedFormats.count) return NULL;
  subItem = &formatList.at(format_index)->supportedFormats.item[sub_index];
  return subItem;
}

bool TDimFormatManager::isFormatSupported (const char *formatName)
{
  TDimFormatItem *fi = getFormatItem (formatName);
  if (fi == NULL) return false;
  return true;
}

bool TDimFormatManager::isFormatSupportsWMP (const char *formatName)
{
  TDimFormatItem *fi = getFormatItem (formatName);
  if (fi == NULL) return false;
  if (fi->canWriteMultiPage == 1) return true;
  return false;
}

bool TDimFormatManager::isFormatSupportsR (const char *formatName)
{
  TDimFormatItem *fi = getFormatItem (formatName);
  if (fi == NULL) return false;
  if (fi->canRead == 1) return true;
  return false;
}

bool TDimFormatManager::isFormatSupportsW (const char *formatName)
{
  TDimFormatItem *fi = getFormatItem (formatName);
  if (fi == NULL) return false;
  if (fi->canWrite == 1) return true;
  return false;
}

bool TDimFormatManager::isFormatSupportsBpcW (const char *formatName, int bpc) {
  TDimFormatItem *fi = getFormatItem (formatName);
  if (fi == NULL) return false;
  if (fi->canWrite == 1) {
    if (fi->constrains.maxBitsPerSample == 0) return true;
    if ((int)fi->constrains.maxBitsPerSample >= bpc) return true;
  }
  return false;
}

void TDimFormatManager::getNeededFormatByFileExt(const char *fileName, int &format_index, int &sub_index)
{
  format_index = 0;
  sub_index = 0;
  fileName;
}

void TDimFormatManager::printAllFormats()
{
  unsigned int i, s;

  for (i=0; i<formatList.size(); i++)
  {
    printf("Format %d: ""%s"" ver: %s\n", i, formatList.at(i)->name, formatList.at(i)->version );
    for (s=0; s<formatList.at(i)->supportedFormats.count; s++)
    {
      printf("  %d: %s [", s, formatList.at(i)->supportedFormats.item[s].formatNameShort);

      if ( formatList.at(i)->supportedFormats.item[s].canRead )  printf("R ");
      if ( formatList.at(i)->supportedFormats.item[s].canWrite ) printf("W ");
      if ( formatList.at(i)->supportedFormats.item[s].canReadMeta ) printf("RM ");
      if ( formatList.at(i)->supportedFormats.item[s].canWriteMeta ) printf("WM ");
      if ( formatList.at(i)->supportedFormats.item[s].canWriteMultiPage ) printf("WMP ");

      printf("] <%s>\n", formatList.at(i)->supportedFormats.item[s].extensions);
    }
    printf("\n");
  }
}

void TDimFormatManager::printAllFormatsXML() {
  unsigned int i, s;

  for (i=0; i<formatList.size(); i++) {
    printf("<format index=\"%d\" name=\"%s\" version=\"%s\" >\n", i, formatList.at(i)->name, formatList.at(i)->version );

    for (s=0; s<formatList.at(i)->supportedFormats.count; s++) {
      printf("  <codec index=\"%d\" name=\"%s\" >\n", s, formatList.at(i)->supportedFormats.item[s].formatNameShort);
      
      if ( formatList.at(i)->supportedFormats.item[s].canRead )  
        printf("    <tag name=\"support\" value=\"reading\" />\n");

      if ( formatList.at(i)->supportedFormats.item[s].canWrite )
        printf("    <tag name=\"support\" value=\"writing\" />\n");

      if ( formatList.at(i)->supportedFormats.item[s].canReadMeta )
        printf("    <tag name=\"support\" value=\"reading metadata\" />\n");

      if ( formatList.at(i)->supportedFormats.item[s].canWriteMeta )
        printf("    <tag name=\"support\" value=\"writing metadata\" />\n");

      if ( formatList.at(i)->supportedFormats.item[s].canWriteMultiPage )
        printf("    <tag name=\"support\" value=\"writing multiple pages\" />\n");

      printf("    <tag name=\"extensions\" value=\"%s\" />\n", formatList.at(i)->supportedFormats.item[s].extensions);

      printf("  </codec>\n");
    }
    printf("</format>\n");
  }
}

std::string TDimFormatManager::getAllFormatsHTML() {
  unsigned int i, s;
  std::string fmts;
  xstring fmt;

  fmts += "<table>\n";
  fmts += "  <tr><th>Codec</th><th>Name</th><th>Features</th><th>Extensions</th></tr>\n";
  for (i=0; i<formatList.size(); i++) {

    for (s=0; s<formatList.at(i)->supportedFormats.count; s++) {
      std::string codec = formatList.at(i)->supportedFormats.item[s].formatNameShort;
      std::string name  = formatList.at(i)->supportedFormats.item[s].formatNameLong;
      xstring ext       = formatList.at(i)->supportedFormats.item[s].extensions;
      xstring features;

      if ( formatList.at(i)->supportedFormats.item[s].canRead ) features += " R"; 
      if ( formatList.at(i)->supportedFormats.item[s].canWrite ) features += " W";
      if ( formatList.at(i)->supportedFormats.item[s].canReadMeta ) features += " RM";
      if ( formatList.at(i)->supportedFormats.item[s].canWriteMeta ) features += " WM";
      if ( formatList.at(i)->supportedFormats.item[s].canWriteMultiPage ) features += " WMP";
      fmt.sprintf("  <tr><td>%s</td><td>%s</td><td>%s</td><td>%s</td></tr>\n", codec.c_str(), name.c_str(), features.c_str(), ext.c_str() );
      fmts += fmt;
    }
  }
  fmts += "</table>\n";
  return fmts;
}

void TDimFormatManager::printAllFormatsHTML() {
  std::string str = getAllFormatsHTML();
  printf( str.c_str() );
}

std::string TDimFormatManager::getAllExtensions() {
  
  unsigned int i, s;
  std::string str;

  for (i=0; i<formatList.size(); i++) {
    for (s=0; s<formatList.at(i)->supportedFormats.count; s++) {
      str += formatList.at(i)->supportedFormats.item[s].extensions;
      str += "|";
    }
  }
  return str;
}

std::string filterizeExtensions( const std::string &_exts ) {
  std::string exts = _exts;
  std::string::size_type pos;
  pos = exts.find ( "|" );
  while (pos != std::string::npos) {
    exts.replace( pos, 1, " *.");
    pos = exts.find ( "|" );
  }
  return exts;
}

std::string TDimFormatManager::getQtFilters() {
  
  unsigned int i, s;
  std::string str;
  std::string t;

  for (i=0; i<formatList.size(); i++) {
    for (s=0; s<formatList.at(i)->supportedFormats.count; s++) {
      str += formatList.at(i)->supportedFormats.item[s].formatNameLong;
      str += " (*.";
      std::string exts = formatList.at(i)->supportedFormats.item[s].extensions;
      str += filterizeExtensions( exts );

      str += ");;";
    }
  }
  return str;
}

std::string TDimFormatManager::getQtReadFilters() {
  unsigned int i, s;
  std::string str;
  std::string t;

  str += "All images (*.";
  std::string exts = getAllExtensions();
  str += filterizeExtensions( exts );
  str += ");;";

  for (i=0; i<formatList.size(); i++) {
    for (s=0; s<formatList.at(i)->supportedFormats.count; s++) 
      if (formatList.at(i)->supportedFormats.item[s].canRead) {
        str += formatList.at(i)->supportedFormats.item[s].formatNameLong;
        str += " (*.";
        std::string exts = formatList.at(i)->supportedFormats.item[s].extensions;
        str += filterizeExtensions( exts );
        str += ");;";
      }
  }
  str += "All files (*.*)";
  return str;
}

std::string TDimFormatManager::getQtWriteFilters() {
  unsigned int i, s;
  std::string str;
  std::string t;

  for (i=0; i<formatList.size(); i++) {
    for (s=0; s<formatList.at(i)->supportedFormats.count; s++) 
      if (formatList.at(i)->supportedFormats.item[s].canWrite) {
        str += formatList.at(i)->supportedFormats.item[s].formatNameLong;
        str += " (*.";
        std::string exts = formatList.at(i)->supportedFormats.item[s].extensions;
        str += filterizeExtensions( exts );
        str += ");;";
      }
  }
  return str;
}

std::string TDimFormatManager::getQtWriteMPFilters() {
  unsigned int i, s;
  std::string str;
  std::string t;

  for (i=0; i<formatList.size(); i++) {
    for (s=0; s<formatList.at(i)->supportedFormats.count; s++) 
      if (formatList.at(i)->supportedFormats.item[s].canWriteMultiPage) {
        str += formatList.at(i)->supportedFormats.item[s].formatNameLong;
        str += " (*.";
        std::string exts = formatList.at(i)->supportedFormats.item[s].extensions;
        str += filterizeExtensions( exts );
        str += ");;";
      }
  }
  return str;
}

std::vector<std::string> TDimFormatManager::getReadFormats() {
  unsigned int i, s;
  std::string str;
  std::vector<std::string> fmts;

  for (i=0; i<formatList.size(); i++) {
    for (s=0; s<formatList.at(i)->supportedFormats.count; s++) 
      if (formatList.at(i)->supportedFormats.item[s].canRead) {
        str = formatList.at(i)->supportedFormats.item[s].formatNameShort;
        fmts.push_back( str );
      }
  }
  return fmts;
}

std::vector<std::string> TDimFormatManager::getWriteFormats() {
  unsigned int i, s;
  std::string str;
  std::vector<std::string> fmts;

  for (i=0; i<formatList.size(); i++) {
    for (s=0; s<formatList.at(i)->supportedFormats.count; s++) 
      if (formatList.at(i)->supportedFormats.item[s].canWrite) {
        str = formatList.at(i)->supportedFormats.item[s].formatNameShort;
        fmts.push_back( str );
      }
  }
  return fmts;
}

std::vector<std::string> TDimFormatManager::getWriteMPFormats() {
  unsigned int i, s;
  std::string str;
  std::vector<std::string> fmts;

  for (i=0; i<formatList.size(); i++) {
    for (s=0; s<formatList.at(i)->supportedFormats.count; s++) 
      if (formatList.at(i)->supportedFormats.item[s].canWriteMultiPage) {
        str = formatList.at(i)->supportedFormats.item[s].formatNameShort;
        fmts.push_back( str );
      }
  }
  return fmts;
}

std::string TDimFormatManager::getFormatFilter( const char *formatName ) {
  TDimFormatItem *fi = getFormatItem (formatName);
  std::string str;
  if (fi == NULL) return str;

  //tr("Images (*.png *.xpm *.jpg)"));
  str += fi->formatNameLong;
  str += " (*.";
  std::string exts = fi->extensions;
  str += filterizeExtensions( exts );
  str += ")";
  return str;
}

std::string TDimFormatManager::getFilterExtensions( const char *formatName ) {
  TDimFormatItem *fi = getFormatItem (formatName);
  std::string str;
  if (fi == NULL) return str;
  str += fi->extensions;
  return str;
}

std::string TDimFormatManager::getFilterExtensionFirst( const char *formatName ) {
  std::string str = getFilterExtensions( formatName );
  
  std::string::size_type pos;
  pos = str.find ( "|" );
  if (pos != std::string::npos)
    str.resize(pos);
  return str;
}

//--------------------------------------------------------------------------------------
// simple read/write operations
//--------------------------------------------------------------------------------------

void TDimFormatManager::loadImage ( DIM_STREAM_CLASS *stream,
                  TDimReadProc readProc, TDimSeekProc seekProc, TDimSizeProc sizeProc, 
                  TDimTellProc  tellProc,  TDimEofProc eofProc, TDimCloseProc closeProc,   
                  const char *fileName, TDimImageBitmap *bmp, int page  )
{
  int format_index;
  TDimFormatHeader *selectedFmt;
  
  if ( ( stream != NULL ) && (seekProc != NULL) && (readProc != NULL) ) { 
    if (!loadMagic( stream, seekProc, readProc )) return;
  }
  else {
    if (!loadMagic(fileName)) return;
  }
  
  format_index = getNeededFormatByMagic();
  if (format_index == -1) return;
  selectedFmt = formatList.at( format_index );

  TDimFormatHandle fmtParams = selectedFmt->aquireFormatProc();

  if ( selectedFmt->validateFormatProc(magic_number, max_magic_size) == -1 ) return;

  fmtParams.showProgressProc = progress_proc;
  fmtParams.showErrorProc    = error_proc;
  fmtParams.testAbortProc    = test_abort_proc;

  fmtParams.stream    = stream;
  fmtParams.readProc  = readProc;
  fmtParams.writeProc = NULL;
  fmtParams.flushProc = NULL;
  fmtParams.seekProc  = seekProc;
  fmtParams.sizeProc  = sizeProc;
  fmtParams.tellProc  = tellProc;
  fmtParams.eofProc   = eofProc;
  fmtParams.closeProc = closeProc;

  fmtParams.fileName = (char *) fileName;
  fmtParams.image = bmp;
  fmtParams.magic = magic_number;
  fmtParams.io_mode = DIM_IO_READ;
  fmtParams.pageNumber = page;

  if ( selectedFmt->openImageProc ( &fmtParams, DIM_IO_READ ) != 0) return;

  //bmpInfo = selectedFmt->getImageInfoProc ( &fmtParams, 0 );
  //selectedFmt->readMetaDataProc ( &fmtParams, 0, -1, -1, -1);
  selectedFmt->readImageProc ( &fmtParams, page );

  selectedFmt->closeImageProc ( &fmtParams );

  // RELEASE FORMAT
  selectedFmt->releaseFormatProc ( &fmtParams );
}

void TDimFormatManager::loadImage(const char *fileName, TDimImageBitmap *bmp, int page)
{
  loadImage ( NULL, NULL, NULL, NULL, NULL, NULL, NULL, fileName, bmp, page );
}

void TDimFormatManager::loadImage(const char *fileName, TDimImageBitmap *bmp)
{
  loadImage ( NULL, NULL, NULL, NULL, NULL, NULL, NULL, fileName, bmp, 0 );
}

void TDimFormatManager::loadBuffer (void *p, int buf_size, TDimImageBitmap *bmp) {
  DMemIO memBuf;
  dMemIO_Init( &memBuf, buf_size, (unsigned char *) p );
  loadImage ( &memBuf, dMemIO_Read, dMemIO_Seek, dMemIO_Size, dMemIO_Tell, dMemIO_Eof, dMemIO_Close, "buffer.dat", bmp, 0 );
  dMemIO_Destroy( &memBuf );
}

void TDimFormatManager::writeImage ( DIM_STREAM_CLASS *stream, TDimReadProc readProc,
                  TDimWriteProc writeProc, TDimFlushProc flushProc, TDimSeekProc seekProc,
                  TDimSizeProc sizeProc, TDimTellProc  tellProc,  TDimEofProc eofProc, TDimCloseProc closeProc,
                  const char *fileName, TDimImageBitmap *bmp, const char *formatName, 
                  int quality, TDimTagList *meta, const char *options )
{
  int format_index, sub_index;
  TDimFormatHeader *selectedFmt;
  
  getNeededFormatByName(formatName, format_index, sub_index);  
  selectedFmt = formatList.at( format_index );

  TDimFormatHandle fmtParams = selectedFmt->aquireFormatProc();

  fmtParams.showProgressProc = progress_proc;
  fmtParams.showErrorProc    = error_proc;
  fmtParams.testAbortProc    = test_abort_proc;
  
  fmtParams.stream    = stream;
  fmtParams.writeProc = writeProc;
  fmtParams.readProc  = readProc;
  fmtParams.flushProc = flushProc;
  fmtParams.seekProc  = seekProc;
  fmtParams.sizeProc  = sizeProc;
  fmtParams.tellProc  = tellProc;
  fmtParams.eofProc   = eofProc;
  fmtParams.closeProc = closeProc;  

  fmtParams.subFormat = sub_index;
  fmtParams.fileName = (char *) fileName;
  fmtParams.image = bmp;
  fmtParams.quality = (unsigned char) quality;
  fmtParams.io_mode = DIM_IO_WRITE;
  if ( meta != NULL ) 
    fmtParams.metaData = *meta;

  if (options != NULL && options[0] != 0) fmtParams.options = (char *) options;

  if ( selectedFmt->openImageProc ( &fmtParams, DIM_IO_WRITE ) != 0) return;

  selectedFmt->writeImageProc ( &fmtParams );

  selectedFmt->closeImageProc ( &fmtParams );

  // RELEASE FORMAT
  selectedFmt->releaseFormatProc ( &fmtParams );
}

void TDimFormatManager::writeImage (const char *fileName, TDimImageBitmap *bmp, const char *formatName, 
                                    int quality, TDimTagList *meta )
{
  writeImage ( NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, fileName, bmp, formatName, quality, meta );
}

void TDimFormatManager::writeImage (const char *fileName, TDimImageBitmap *bmp, const char *formatName, int quality) 
{
  writeImage ( NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, fileName, bmp, formatName, quality, NULL );  
}

void TDimFormatManager::writeImage(const char *fileName, TDimImageBitmap *bmp, const char *formatName, const char *options)
{
  writeImage ( NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, fileName, bmp, formatName, 100, NULL, options );  
}

unsigned char *TDimFormatManager::writeBuffer ( TDimImageBitmap *bmp, const char *formatName, int quality, 
                                               TDimTagList *meta, int &buf_size ) {
  DMemIO memBuf;
  unsigned char *p;
  dMemIO_Init( &memBuf, 0, NULL );
  writeImage ( &memBuf, dMemIO_Read, dMemIO_Write, dMemIO_Flush, dMemIO_Seek,
               dMemIO_Size, dMemIO_Tell, dMemIO_Eof, dMemIO_Close,
               "buffer.dat", bmp, formatName, quality, meta );

  p = new unsigned char [ memBuf.size ];
  buf_size = memBuf.size;
  memcpy( p, memBuf.data, memBuf.size );
  dMemIO_Destroy( &memBuf );
  return p;
}

unsigned char *TDimFormatManager::writeBuffer ( TDimImageBitmap *bmp, const char *formatName, int quality, int &buf_size )
{
  return writeBuffer ( bmp, formatName, quality, NULL, buf_size );
}

unsigned char *TDimFormatManager::writeBuffer ( TDimImageBitmap *bmp, const char *formatName, int &buf_size )
{
  return writeBuffer ( bmp, formatName, 100, NULL, buf_size );
}

void TDimFormatManager::readImagePreview (const char *fileName, TDimImageBitmap *bmp, 
                         DIM_UINT roiX, DIM_UINT roiY, DIM_UINT roiW, DIM_UINT roiH, DIM_UINT w, DIM_UINT h)
{
  int format_index;
  TDimFormatHeader *selectedFmt;
  
  if (!loadMagic(fileName)) return;
  
  format_index = getNeededFormatByMagic();
  selectedFmt = formatList.at( format_index );

  TDimFormatHandle fmtParams = selectedFmt->aquireFormatProc();

  if ( selectedFmt->validateFormatProc(magic_number, max_magic_size) == -1 ) return;

  fmtParams.roiX = roiX;
  fmtParams.roiY = roiY;
  fmtParams.roiW = roiW;
  fmtParams.roiH = roiH;

  fmtParams.fileName = (char *) fileName;
  fmtParams.image = bmp;
  fmtParams.magic = magic_number;
  fmtParams.io_mode = DIM_IO_READ;
  if ( selectedFmt->openImageProc ( &fmtParams, DIM_IO_READ ) != 0) return;

  
  //----------------------------------------------------
  // now if function is not implemented, run standard
  // implementation
  //----------------------------------------------------
  if ( selectedFmt->readImagePreviewProc != NULL )
    selectedFmt->readImagePreviewProc ( &fmtParams, w, h );
  else
  {
    // read whole image and resize if needed
    selectedFmt->readImageProc ( &fmtParams, 0 );

    // first extract ROI from full image
    //if ( roiX < 0) roiX = 0; if ( roiY < 0) roiY = 0;
    if ( roiX >= bmp->i.width) roiX = bmp->i.width-1; 
    if ( roiY >= bmp->i.height) roiY = bmp->i.height-1;
    if ( roiW >= bmp->i.width-roiX) roiW = bmp->i.width-roiX-1; 
    if ( roiY >= bmp->i.height-roiY) roiH = bmp->i.height-roiY-1; 

    if ( (roiW>0) && (roiH>0) )
      retreiveImgROI( bmp, roiX, roiY, roiW, roiH );

    // now resize to desired size
    if ( (w>0) && (h>0) )
      resizeImgNearNeighbor( bmp, w, h);


  }
  //----------------------------------------------------

  selectedFmt->closeImageProc ( &fmtParams );

  // RELEASE FORMAT
  selectedFmt->releaseFormatProc ( &fmtParams );
}
 
void TDimFormatManager::readImageThumb (const char *fileName, TDimImageBitmap *bmp, DIM_UINT w, DIM_UINT h)
{
  int format_index;
  TDimFormatHeader *selectedFmt;
  
  if (!loadMagic(fileName)) return;
  
  format_index = getNeededFormatByMagic();
  selectedFmt = formatList.at( format_index );

  TDimFormatHandle fmtParams = selectedFmt->aquireFormatProc();

  if ( selectedFmt->validateFormatProc(magic_number, max_magic_size) == -1 ) return;

  fmtParams.fileName = (char *) fileName;
  fmtParams.image = bmp;
  fmtParams.magic = magic_number;
  fmtParams.io_mode = DIM_IO_READ;
  if ( selectedFmt->openImageProc ( &fmtParams, DIM_IO_READ ) != 0) return;

  
  //----------------------------------------------------
  // now if function is not implemented, run standard
  // implementation
  //----------------------------------------------------
  if ( selectedFmt->readImagePreviewProc != NULL )
    selectedFmt->readImagePreviewProc ( &fmtParams, w, h );
  else
  {
    // read whole image and resize if needed
    selectedFmt->readImageProc ( &fmtParams, 0 );

    // now resize to desired size
    if ( (w>0) && (h>0) )
      resizeImgNearNeighbor( bmp, w, h);

  }
  //----------------------------------------------------

  selectedFmt->closeImageProc ( &fmtParams );

  // RELEASE FORMAT
  selectedFmt->releaseFormatProc ( &fmtParams );
}


//--------------------------------------------------------------------------------------
// begin: session-wide operations
//--------------------------------------------------------------------------------------
int TDimFormatManager::sessionStartRead ( DIM_STREAM_CLASS *stream, TDimReadProc readProc, TDimSeekProc seekProc, 
                         TDimSizeProc sizeProc, TDimTellProc tellProc, TDimEofProc eofProc, 
                         TDimCloseProc closeProc, const char *fileName, const char *formatName)
{
  if (session_active) sessionEnd();
  sessionCurrentPage = 0;
  
  if (formatName == NULL) {
    if ( ( stream != NULL ) && (seekProc != NULL) && (readProc != NULL) ) { 
      if (!loadMagic( stream, seekProc, readProc )) return 1;
    }
    else {
      if (!loadMagic(fileName)) return 1;
    }
    sessionFormatIndex = getNeededFormatByMagic();
  } else {
    // force specific format
    getNeededFormatByName(formatName, sessionFormatIndex, sessionSubIndex);  
  }

  if (sessionFormatIndex < 0) return 1;
  TDimFormatHeader *selectedFmt = formatList.at( sessionFormatIndex );

  sessionHandle = selectedFmt->aquireFormatProc();

  //if ( selectedFmt->validateFormatProc(magic_number, max_magic_size)==-1 ) return 1;
  
  sessionHandle.showProgressProc = progress_proc;
  sessionHandle.showErrorProc    = error_proc;
  sessionHandle.testAbortProc    = test_abort_proc;

  sessionHandle.stream    = stream;
  sessionHandle.readProc  = readProc;
  sessionHandle.writeProc = NULL;
  sessionHandle.flushProc = NULL;
  sessionHandle.seekProc  = seekProc;
  sessionHandle.sizeProc  = sizeProc;
  sessionHandle.tellProc  = tellProc;
  sessionHandle.eofProc   = eofProc;
  sessionHandle.closeProc = closeProc;

  sessionFileName = fileName; 
  sessionHandle.fileName = &sessionFileName[0];
  sessionHandle.magic    = magic_number;
  sessionHandle.io_mode  = DIM_IO_READ;
  int res = selectedFmt->openImageProc ( &sessionHandle, DIM_IO_READ );
  sessionSubIndex = sessionHandle.subFormat;
  if (res == 0) session_active = TRUE;

  return res;
}

int TDimFormatManager::sessionStartRead  (const char *fileName, const char *formatName) {
  return sessionStartRead ( NULL, NULL, NULL, NULL, NULL, NULL, NULL, fileName, formatName );
}

int TDimFormatManager::sessionStartReadRAW  (const char *fileName, unsigned int header_offset, bool big_endian) {
  int res = sessionStartRead(fileName, "RAW");
  if (res == 0) {
    TDimRawParams *rp = (TDimRawParams *) sessionHandle.internalParams;
    rp->header_offset = header_offset;
    rp->big_endian = big_endian;
  }
  return res;
}

int TDimFormatManager::sessionStartWrite (const char *fileName, const char *formatName, const char *options ) {
  return sessionStartWrite ( NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, fileName, formatName, options );
}

int TDimFormatManager::sessionStartWrite ( DIM_STREAM_CLASS *stream,
                  TDimWriteProc writeProc, TDimFlushProc flushProc, TDimSeekProc seekProc,
                  TDimSizeProc sizeProc, TDimTellProc  tellProc,  TDimEofProc eofProc, TDimCloseProc closeProc,
                  const char *fileName, const char *formatName, const char *options ) {
  int res;
  if (session_active == TRUE) sessionEnd();
  sessionCurrentPage = 0;

  getNeededFormatByName(formatName, sessionFormatIndex, sessionSubIndex);  
  TDimFormatHeader *selectedFmt = formatList.at( sessionFormatIndex );

  sessionHandle = selectedFmt->aquireFormatProc();

  sessionHandle.showProgressProc = progress_proc;
  sessionHandle.showErrorProc    = error_proc;
  sessionHandle.testAbortProc    = test_abort_proc;

  sessionHandle.stream    = stream;
  sessionHandle.writeProc = writeProc;
  sessionHandle.readProc  = NULL;
  sessionHandle.flushProc = flushProc;
  sessionHandle.seekProc  = seekProc;
  sessionHandle.sizeProc  = sizeProc;
  sessionHandle.tellProc  = tellProc;
  sessionHandle.eofProc   = eofProc;
  sessionHandle.closeProc = closeProc;  
  
  sessionFileName = fileName; 
  sessionHandle.fileName = &sessionFileName[0];
  sessionHandle.io_mode  = DIM_IO_WRITE;  
  sessionHandle.subFormat = sessionSubIndex;
  
  if (options != NULL && options[0] != 0) sessionHandle.options = (char *) options;

  res = selectedFmt->openImageProc ( &sessionHandle, DIM_IO_WRITE );

  if (res == 0) session_active = TRUE;
  return res;
}

void TDimFormatManager::sessionEnd()
{
  if ( (sessionFormatIndex>0) && (sessionFormatIndex<(int)formatList.size()) )
  {
    TDimFormatHeader *selectedFmt = formatList.at( sessionFormatIndex );
    selectedFmt->closeImageProc    ( &sessionHandle );
    selectedFmt->releaseFormatProc ( &sessionHandle );
  }
  session_active = FALSE;
  sessionCurrentPage = 0;
}

void  TDimFormatManager::sessionSetQuality  (int quality) {
  sessionHandle.quality = (unsigned char) quality;
}

int TDimFormatManager::sessionGetFormat ()
{
  if (session_active != TRUE) return -1;
  return sessionFormatIndex;
}

int TDimFormatManager::sessionGetSubFormat ()
{
  if (session_active != TRUE) return -1;
  return sessionSubIndex;
}

char* TDimFormatManager::sessionGetCodecName () {
  if (session_active != TRUE) return NULL;
  return formatList.at(sessionFormatIndex)->name;
}

bool TDimFormatManager::sessionIsCurrentCodec ( const char *name ) {
  char* fmt = sessionGetCodecName(); 
  return ( strcmp( fmt, name ) == 0 );
}

char* TDimFormatManager::sessionGetFormatName () {
  if (session_active != TRUE) return NULL;
  return formatList.at(sessionFormatIndex)->supportedFormats.item[sessionSubIndex].formatNameShort;
}

bool TDimFormatManager::sessionIsCurrentFormat ( const char *name ) {
  char* fmt = sessionGetFormatName(); 
  return ( strcmp( fmt, name ) == 0 );
}

int TDimFormatManager::sessionGetNumberOfPages ()
{
  if (session_active != TRUE) return 0;
  TDimFormatHeader *selectedFmt = formatList.at( sessionFormatIndex );  
  return selectedFmt->getNumPagesProc ( &sessionHandle );
}

int TDimFormatManager::sessionGetCurrentPage ()
{
  if (session_active != TRUE) return 0;
  return sessionCurrentPage;
}

char* TDimFormatManager::sessionGetTextMetaData ()
{
  if (session_active != TRUE) return NULL;
  TDimFormatHeader *selectedFmt = formatList.at( sessionFormatIndex );  
  return selectedFmt->readMetaDataAsTextProc ( &sessionHandle );
}

TDimTagList* TDimFormatManager::sessionReadMetaData ( DIM_UINT page, int group, int tag, int type)
{
  if (session_active != TRUE) return NULL;
  sessionCurrentPage = page;
  TDimFormatHeader *selectedFmt = formatList.at( sessionFormatIndex );    
  selectedFmt->readMetaDataProc ( &sessionHandle, page, group, tag, type);
  return &sessionHandle.metaData;
}

int TDimFormatManager::sessionReadImage ( TDimImageBitmap *bmp, DIM_UINT page ) {
  if (session_active != TRUE) return 1;
  TDimFormatHeader *selectedFmt = formatList.at( sessionFormatIndex );
  sessionCurrentPage = page;  
  sessionHandle.image = bmp;
  sessionHandle.pageNumber = page;
  int r = selectedFmt->readImageProc ( &sessionHandle, page );
  sessionHandle.image = NULL;
  return r;
}

int TDimFormatManager::sessionWriteImage ( TDimImageBitmap *bmp, DIM_UINT page) {
  if (session_active != TRUE) return 1;
  TDimFormatHeader *selectedFmt = formatList.at( sessionFormatIndex );
  sessionCurrentPage = page;  
  sessionHandle.image = bmp;
  sessionHandle.pageNumber = page;
  return selectedFmt->writeImageProc ( &sessionHandle );
}


void  TDimFormatManager::sessionReadImagePreview ( TDimImageBitmap *bmp, 
                                DIM_UINT roiX, DIM_UINT roiY, DIM_UINT roiW, DIM_UINT roiH, 
                                DIM_UINT w, DIM_UINT h)
{
  if (session_active != TRUE) return;
  TDimFormatHeader *selectedFmt = formatList.at( sessionFormatIndex );
  sessionCurrentPage = 0;  
  sessionHandle.image = bmp;
  sessionHandle.pageNumber = 0;
  sessionHandle.roiX = roiX;
  sessionHandle.roiY = roiY;
  sessionHandle.roiW = roiW;
  sessionHandle.roiH = roiH;  

  //----------------------------------------------------
  // now if function is not implemented, run standard
  // implementation
  //----------------------------------------------------
  if ( selectedFmt->readImagePreviewProc != NULL )
    selectedFmt->readImagePreviewProc ( &sessionHandle, w, h );
  else
  {
    // read whole image and resize if needed
    selectedFmt->readImageProc ( &sessionHandle, 0 );

    // first extract ROI from full image
    //if ( roiX < 0) roiX = 0; if ( roiY < 0) roiY = 0;
    if ( roiX >= bmp->i.width) roiX = bmp->i.width-1; 
    if ( roiY >= bmp->i.height) roiY = bmp->i.height-1;
    if ( roiW >= bmp->i.width-roiX) roiW = bmp->i.width-roiX-1; 
    if ( roiY >= bmp->i.height-roiY) roiH = bmp->i.height-roiY-1; 

    if ( (roiW>0) && (roiH>0) )
      retreiveImgROI( bmp, roiX, roiY, roiW, roiH );

    // now resize to desired size
    if ( (w>0) && (h>0) )
      resizeImgNearNeighbor( bmp, w, h);
  }

}

void  TDimFormatManager::sessionReadImageThumb   ( TDimImageBitmap *bmp, DIM_UINT w, DIM_UINT h)
{
  if (session_active != TRUE) return;
  TDimFormatHeader *selectedFmt = formatList.at( sessionFormatIndex );
  sessionCurrentPage = 0;  
  sessionHandle.image = bmp;
  sessionHandle.pageNumber = 0;

  //----------------------------------------------------
  // now if function is not implemented, run standard
  // implementation
  //----------------------------------------------------
  if ( selectedFmt->readImagePreviewProc != NULL )
    selectedFmt->readImagePreviewProc ( &sessionHandle, w, h );
  else
  {
    // read whole image and resize if needed
    selectedFmt->readImageProc ( &sessionHandle, 0 );

    // now resize to desired size
    if ( (w>0) && (h>0) )
      resizeImgNearNeighbor( bmp, w, h);

  }

}

//--------------------------------------------------------------------------------------
// end: session-wide operations
//--------------------------------------------------------------------------------------















