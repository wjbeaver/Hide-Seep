/*******************************************************************************

  Manager for Image Formats

  Uses DimFiSDK version: 1.2
  
  Programmer: Dima Fedorov Levit <dimin@dimin.net> <http://www.dimin.net/>

  Notes:
    Session: during session any session wide operation will be performed
    with current session!!!
    If you want to start simultaneous sessions then create new manager
    using = operator. It will copy all necessary initialization data
    to the new manager.

    Non-session wide operation might be performed simultaneously with 
    an open session.

  History:
    03/23/2004 18:03 - First creation
    08/04/2004 18:22 - custom stream managment compliant
    
  ver: 2
        
*******************************************************************************/

#ifndef DIM_FORMAT_MANAGER_H
#define DIM_FORMAT_MANAGER_H

#include <string>
#include <vector>

#include <dim_img_format_interface.h>
#include <dim_image.h>

class TDimFormatManager
{
public:
  TDimFormatManager();
  ~TDimFormatManager();
  TDimFormatManager &operator=( TDimFormatManager fm );

  void              addNewFormatHeader (TDimFormatHeader *nfh);
  TDimFormatHeader *getFormatHeader(int i);
  unsigned int      countInstalledFormats();
  //bool              canWriteMulti (const char *formatName);
  bool              isFormatSupported (const char *formatName);
  bool              isFormatSupportsR (const char *formatName);
  bool              isFormatSupportsW (const char *formatName);  
  bool              isFormatSupportsWMP (const char *formatName);

  bool              isFormatSupportsBpcW (const char *formatName, int bpc); 

  void              printAllFormats();
  void              printAllFormatsXML();
  void              printAllFormatsHTML();
  std::string       getAllFormatsHTML();

  std::string       getAllExtensions();
  std::vector<std::string> getReadFormats();
  std::vector<std::string> getWriteFormats();
  std::vector<std::string> getWriteMPFormats();
  std::string       getFormatFilter( const char *formatName );
  std::string       getFilterExtensions( const char *formatName );
  std::string       getFilterExtensionFirst( const char *formatName );

  std::string       getQtFilters();
  std::string       getQtReadFilters();
  std::string       getQtWriteFilters();
  std::string       getQtWriteMPFilters();

  // Simple Read/Write
  void loadImage  (const char *fileName, TDimImageBitmap *bmp);
  void loadImage  (const char *fileName, TDimImage &img) { loadImage(fileName, img.imageBitmap()); }
  void loadImage  (const char *fileName, TDimImageBitmap *bmp, int page);
  void loadImage  (const char *fileName, TDimImage &img, int page) { loadImage(fileName, img.imageBitmap(), page); }


  void loadImage  ( DIM_STREAM_CLASS *stream,
                    TDimReadProc readProc, TDimSeekProc seekProc, TDimSizeProc sizeProc, 
                    TDimTellProc  tellProc,  TDimEofProc eofProc, TDimCloseProc closeProc,   
                    const char *fileName, TDimImageBitmap *bmp, int page );
  void loadBuffer (void *p, int buf_size, TDimImageBitmap *bmp);
  
  void readImagePreview (const char *fileName, TDimImageBitmap *bmp, 
                         DIM_UINT roiX, DIM_UINT roiY, DIM_UINT roiW, DIM_UINT roiH, 
                         DIM_UINT w, DIM_UINT h);
  void readImageThumb   (const char *fileName, TDimImageBitmap *bmp, DIM_UINT w, DIM_UINT h);


  void writeImage ( DIM_STREAM_CLASS *stream, TDimReadProc readProc, TDimWriteProc writeProc, TDimFlushProc flushProc, 
                    TDimSeekProc seekProc, TDimSizeProc sizeProc, TDimTellProc  tellProc,  
                    TDimEofProc eofProc, TDimCloseProc closeProc, const char *fileName, 
                    TDimImageBitmap *bmp, const char *formatName, int quality, TDimTagList *meta, const char *options = NULL );  
  void writeImage (const char *fileName, TDimImageBitmap *bmp, const char *formatName, 
                   int quality, TDimTagList *meta );
  void writeImage (const char *fileName, TDimImageBitmap *bmp, const char *formatName, int quality);
  void writeImage (const char *fileName, TDimImageBitmap *bmp, const char *formatName, const char *options = NULL);
  void writeImage (const char *fileName, TDimImage &img, const char *formatName, const char *options = NULL) {
    writeImage (fileName, img.imageBitmap(), formatName, options);
  }

  unsigned char *writeBuffer ( TDimImageBitmap *bmp, const char *formatName, int quality, TDimTagList *meta, int &buf_size );
  unsigned char *writeBuffer ( TDimImageBitmap *bmp, const char *formatName, int quality, int &buf_size );
  unsigned char *writeBuffer ( TDimImageBitmap *bmp, const char *formatName, int &buf_size );

  //--------------------------------------------------------------------------------------
  // Callbacks
  //--------------------------------------------------------------------------------------

  TDimProgressProc progress_proc;
  TDimErrorProc error_proc;
  TDimTestAbortProc test_abort_proc;
  void setCallbacks( TDimProgressProc _progress_proc, 
                     TDimErrorProc _error_proc, 
                     TDimTestAbortProc _test_abort_proc ) {
    progress_proc = _progress_proc;
    error_proc = _error_proc;
    test_abort_proc = _test_abort_proc;
  }

  //--------------------------------------------------------------------------------------
  // begin: session-wide operations
  //--------------------------------------------------------------------------------------
  int   sessionStartRead  ( DIM_STREAM_CLASS *stream, TDimReadProc readProc, TDimSeekProc seekProc, 
                            TDimSizeProc sizeProc, TDimTellProc tellProc, TDimEofProc eofProc, 
                            TDimCloseProc closeProc, const char *fileName, const char *formatName=NULL);
  
  int   sessionStartWrite ( DIM_STREAM_CLASS *stream, TDimWriteProc writeProc, TDimFlushProc flushProc, 
          TDimSeekProc seekProc, TDimSizeProc sizeProc, TDimTellProc  tellProc, TDimEofProc eofProc,
          TDimCloseProc closeProc, const char *fileName, const char *formatName, const char *options = NULL );

  // if formatName spesified then forces specific format, used for RAW
  int   sessionStartRead    (const char *fileName, const char *formatName=NULL);
  int   sessionStartReadRAW (const char *fileName, unsigned int header_offset=0, bool big_endian=false);
  int   sessionStartWrite   (const char *fileName, const char *formatName, const char *options = NULL);

  int   sessionGetFormat ();
  int   sessionGetSubFormat ();
  char* sessionGetCodecName ();
  char* sessionGetFormatName ();
  bool  sessionIsCurrentCodec ( const char *name );
  bool  sessionIsCurrentFormat ( const char *name );
  int   sessionGetNumberOfPages ();
  int   sessionGetCurrentPage ();
  char* sessionGetTextMetaData  ();
  TDimTagList* sessionReadMetaData ( DIM_UINT page, int group, int tag, int type);
  int   sessionReadImage  ( TDimImageBitmap *bmp, DIM_UINT page );
  int   sessionReadImage  ( TDimImage &img, DIM_UINT page ) { return sessionReadImage(img.imageBitmap(), page); }

  void  sessionReadImagePreview ( TDimImageBitmap *bmp, 
                                  DIM_UINT roiX, DIM_UINT roiY, DIM_UINT roiW, DIM_UINT roiH, 
                                  DIM_UINT w, DIM_UINT h);
  void  sessionReadImageThumb   ( TDimImageBitmap *bmp, DIM_UINT w, DIM_UINT h);


  void  sessionSetQuality ( int quality );
  int   sessionWriteImage ( TDimImageBitmap *bmp, DIM_UINT page );
  int   sessionWriteImage ( TDimImage &img, DIM_UINT page ) 
  { return sessionWriteImage(img.imageBitmap(), page); }

  void  sessionEnd();
  //--------------------------------------------------------------------------------------
  // end: session-wide operations
  //--------------------------------------------------------------------------------------


protected:
  unsigned char *magic_number; // magic number loaded to suit correct format search
  unsigned int max_magic_size; // maximum size needed to establish format
  unsigned int static_formats;

  // session vars
  bool session_active;
  TDimFormatHandle sessionHandle;
  int sessionFormatIndex, sessionSubIndex;
  int sessionCurrentPage;
  std::string sessionFileName;

  std::vector<TDimFormatHeader *> formatList;

  void setMaxMagicSize();
  bool loadMagic( const char *fileName );
  bool loadMagic( DIM_STREAM_CLASS *stream, TDimSeekProc seekProc, TDimReadProc readProc );
  int  getNeededFormatByMagic();
  void getNeededFormatByName    (const char *formatName, int &format_index, int &sub_index);
  void getNeededFormatByFileExt (const char *fileName, int &format_index, int &sub_index);
  
  TDimFormatItem *getFormatItem (const char *formatName);
};

#endif // DIM_FORMAT_MANAGER_H

