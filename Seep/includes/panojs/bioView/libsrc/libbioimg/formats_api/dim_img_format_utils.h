/*******************************************************************************

  Defines Image Format Utilities
  rely on: DimFiSDK version: 1
  
  Programmer: Dima Fedorov Levit <dimin@dimin.net> <http://www.dimin.net/>

  Image file structure:
    
    1) Page:   each file may contain 1 or more pages, each page is independent

    2) Sample: in each page each pixel can contain 1 or more samples
               preferred number of samples: 1 (GRAY), 3(RGB) and 4(RGBA)

    3) Depth:  each sample can be of 1 or more bits of depth
                preferred depths are: 8 and 16 bits per sample

    4) Allocation: each sample is allocated having in mind:
               All lines are stored contiguasly from top to bottom where
               having each particular line is byte alligned, i.e.:
               each line is allocated using minimal necessary number of bytes
               to store given depth. Which for the image means:
               size = ceil( ( (width*depth) / 8 ) * height )

  As a result of Sample/Depth structure we can get images of different 
  Bits per Pixel as: 1, 8, 24, 32, 48 and any other if needed

  History:
    04/08/2004 11:57 - First creation
      
  ver: 2
        
*******************************************************************************/

#ifndef DIM_IMG_FMT_UTL_H
#define DIM_IMG_FMT_UTL_H

#include "dim_img_format_interface.h"

#include <string>

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE  1
#endif

//------------------------------------------------------------------------------
// Safe calls for memory/io prototypes, if they are not supplied then
// standard functions are used
//------------------------------------------------------------------------------

//#define D_SIZE_T DIM_ULONG
//#define D_OFFSET_T DIM_LONG
#define D_SIZE_T DIM_UINT64
#define D_OFFSET_T DIM_INT64

void* dimMalloc ( TDimFormatHandle *fmtHndl, D_SIZE_T size );
void* dimFree   ( TDimFormatHandle *fmtHndl, void *p );
// overload of dimin free to make a safe delete
inline void dimFree( void **p ) {
  if ( *p == NULL ) return;
  delete (unsigned char *) *p;  
  *p = NULL;
}

void dimProgress  ( TDimFormatHandle *fmtHndl, long done, long total, char *descr);
void dimError     ( TDimFormatHandle *fmtHndl, int val, char *descr);
int  dimTestAbort ( TDimFormatHandle *fmtHndl );

// the stream is specified by TDimFormatHandle
D_SIZE_T   dimRead  ( TDimFormatHandle *fmtHndl, void *buffer, D_SIZE_T size, D_SIZE_T count );
D_SIZE_T   dimWrite ( TDimFormatHandle *fmtHndl, void *buffer, D_SIZE_T size, D_SIZE_T count );
DIM_INT    dimFlush ( TDimFormatHandle *fmtHndl );
DIM_INT    dimSeek  ( TDimFormatHandle *fmtHndl, D_OFFSET_T offset, DIM_INT origin );
D_SIZE_T   dimSize  ( TDimFormatHandle *fmtHndl );
D_OFFSET_T dimTell  ( TDimFormatHandle *fmtHndl );
DIM_INT    dimEof   ( TDimFormatHandle *fmtHndl );
DIM_INT    dimClose ( TDimFormatHandle *fmtHndl );
   
//------------------------------------------------------------------------------
// tests for provided callbacks
//------------------------------------------------------------------------------
bool isCustomReading ( TDimFormatHandle *fmtHndl );
bool isCustomWriting ( TDimFormatHandle *fmtHndl );

//------------------------------------------------------------------------------------------------
// misc
//------------------------------------------------------------------------------------------------

DIM_UCHAR iTrimUC (int num);
TDimFormatHandle initTDimFormatHandle();
TDimImageInfo initTDimImageInfo();

//------------------------------------------------------------------------------------------------
// swap
//------------------------------------------------------------------------------------------------

void dimSwapShort(DIM_UINT16* wp);
void dimSwapLong(DIM_UINT32* lp);
void dimSwapArrayOfShort(DIM_UINT16* wp, register DIM_ULONG n);
void dimSwapArrayOfLong(register DIM_UINT32* lp, register DIM_ULONG n);
void dimSwapDouble(double *dp);
void dimSwapArrayOfDouble(double* dp, register DIM_ULONG n);
void dimSwapData(int type, long size, void* data);

//------------------------------------------------------------------------------------------------
// TDimImageBitmap
//------------------------------------------------------------------------------------------------

// you must call this function once declared image var
void initImagePlanes(TDimImageBitmap *bmp);

int allocImg( TDimImageBitmap *img, DIM_UINT w, DIM_UINT h, DIM_UINT samples, DIM_UINT depth);
// alloc image using w,h,s,d
int allocImg( TDimFormatHandle *fmtHndl, TDimImageBitmap *img, DIM_UINT w, DIM_UINT h, DIM_UINT samples, DIM_UINT depth);
// alloc image using info
int allocImg( TDimFormatHandle *fmtHndl, TDimImageInfo *info, TDimImageBitmap *img);
// alloc handle image using info
int allocImg( TDimFormatHandle *fmtHndl, TDimImageInfo *info );

void deleteImg( TDimImageBitmap *img);
void deleteImg( TDimFormatHandle *fmtHndl, TDimImageBitmap *img);

long getLineSizeInBytes(TDimImageBitmap *img);
long getImgSizeInBytes(TDimImageBitmap *img);
long getImgNumColors(TDimImageBitmap *img);

int getSampleHistogram(TDimImageBitmap *img, long *hist, int sample);
int normalizeImg(TDimImageBitmap *img, TDimImageBitmap *img8);
int resizeImgNearNeighbor( TDimImageBitmap *img, unsigned int newWidth, unsigned int newHeight);
int retreiveImgROI( TDimImageBitmap *img, DIM_ULONG x, DIM_ULONG y, DIM_ULONG w, DIM_ULONG h );

std::string getImageInfoText( TDimImageBitmap *img );

//------------------------------------------------------------------------------------------------
// metadata
//------------------------------------------------------------------------------------------------

void clearMetaTag(TDimTagItem *tagItem);
void clearMetaTags(TDimTagList *tagList);
//bool isTagPresent(TDimTagList *tagList, int tag);
bool isTagPresent(TDimTagList *tagList, int group, int tag);
//int tagPos(TDimTagList *tagList, int tag);
int tagPos(TDimTagList *tagList, int group, int tag);

int addMetaTag(TDimTagList *tagList, TDimTagItem item);

#endif //DIM_IMG_FMT_UTL_H


