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
    10/10/2005 15:15 - Fixes in allocImg to read palette for images
    2008-06-27 14:57 - Fixes by Mario Emmenlauer to support large files
      
  ver: 4
        
*******************************************************************************/

#include "dim_img_format_utils.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

// Disables Visual Studio 2005 warnings for deprecated code
#if ( defined(_MSC_VER) && (_MSC_VER >= 1400) )
  #pragma warning(disable:4996)
#endif 

const char *dimNames[6] = { "none", "X", "Y", "C", "Z", "T" };

extern void* DimMalloc(DIM_ULONG size);
extern void* DimFree(void *p);
extern TDimImageInfo initTDimImageInfo();

//------------------------------------------------------------------------------
// tests for provided callbacks
//------------------------------------------------------------------------------

bool isCustomReading ( TDimFormatHandle *fmtHndl ) {
  if ( ( fmtHndl->stream != NULL ) && 
       ( fmtHndl->readProc != NULL ) && 
       ( fmtHndl->seekProc != NULL ) 
     ) return TRUE;
  return FALSE;
}

bool isCustomWriting ( TDimFormatHandle *fmtHndl ) {
  if ( ( fmtHndl->stream != NULL ) &&
       ( fmtHndl->writeProc != NULL ) && 
       ( fmtHndl->seekProc  != NULL ) &&
       ( fmtHndl->flushProc != NULL )
     ) return TRUE;
  return FALSE;
}

//------------------------------------------------------------------------------
// Safe calls for callbacks
//------------------------------------------------------------------------------

void dimProgress ( TDimFormatHandle *fmtHndl, long done, long total, char *descr) {
  if ( fmtHndl->showProgressProc != NULL ) 
    fmtHndl->showProgressProc ( done, total, descr );
}

void dimError ( TDimFormatHandle *fmtHndl, int val, char *descr) {
  if ( fmtHndl->showErrorProc != NULL ) 
    fmtHndl->showErrorProc ( val, descr );
}

int  dimTestAbort ( TDimFormatHandle *fmtHndl ) {
  if ( fmtHndl->testAbortProc != NULL ) 
    return fmtHndl->testAbortProc ( );
  else
    return 0;
}

//------------------------------------------------------------------------------
// Safe calls for memory/io prototypes, if they are not supplied then
// standard functions are used
//------------------------------------------------------------------------------

void* dimMalloc( TDimFormatHandle *fmtHndl, D_SIZE_T size ) {
  if ( fmtHndl->mallocProc != NULL ) 
    return fmtHndl->mallocProc ( size );
  else {
    void *p = (void *) new char[size];
    return p;
  }
}

void* dimFree( TDimFormatHandle *fmtHndl, void *p ) {
  if ( fmtHndl->freeProc != NULL ) 
    return fmtHndl->freeProc( p );
  else {
    unsigned char *pu = (unsigned char*) p;
    if (p != NULL) delete pu;  
    return NULL;
  }
}

D_SIZE_T dimRead ( TDimFormatHandle *fmtHndl, void *buffer, D_SIZE_T size, D_SIZE_T count ) {
  if ( fmtHndl == NULL ) return 0;
  if ( fmtHndl->stream == NULL ) return 0;
  if ( fmtHndl->readProc != NULL ) 
    return fmtHndl->readProc( buffer, size, count, fmtHndl->stream );
  else
    return fread( buffer, size, count, (FILE *) fmtHndl->stream );
}

D_SIZE_T dimWrite ( TDimFormatHandle *fmtHndl, void *buffer, D_SIZE_T size, D_SIZE_T count ) {
  if ( fmtHndl == NULL ) return 0;
  if ( fmtHndl->stream == NULL ) return 0;    
  if ( fmtHndl->writeProc != NULL ) 
    return fmtHndl->writeProc( buffer, size, count, fmtHndl->stream );
  else
    return fwrite( buffer, size, count, (FILE *) fmtHndl->stream );
}

DIM_INT dimFlush ( TDimFormatHandle *fmtHndl ) {
  if ( fmtHndl == NULL ) return EOF;
  if ( fmtHndl->stream == NULL ) return EOF;  
  if ( fmtHndl->flushProc != NULL ) 
    return fmtHndl->flushProc( fmtHndl->stream );
  else
    return fflush( (FILE *) fmtHndl->stream );
}

DIM_INT dimSeek ( TDimFormatHandle *fmtHndl, D_OFFSET_T offset, DIM_INT origin ) {
  if ( fmtHndl == NULL ) return 1;
  if ( fmtHndl->stream == NULL ) return 1;

	off_t off_io = (off_t) offset;
	if ((D_OFFSET_T) off_io != offset) return -1;

  if ( fmtHndl->seekProc != NULL ) {
    return fmtHndl->seekProc( fmtHndl->stream, off_io, origin );
  } else {
    return fseek( (FILE *) fmtHndl->stream, off_io, origin );
  }
}

D_SIZE_T dimSize ( TDimFormatHandle *fmtHndl ) {
  if ( fmtHndl == NULL ) return 0;
  if ( fmtHndl->stream == NULL ) return 0;   
  D_SIZE_T fsize = 0; 
  if ( fmtHndl->sizeProc != NULL ) 
    return fmtHndl->sizeProc( fmtHndl->stream );
  else {
    D_SIZE_T p = dimTell(fmtHndl);
    fseek( (FILE *) fmtHndl->stream, 0, SEEK_END );
    D_SIZE_T end_p = dimTell(fmtHndl);
    fseek( (FILE *) fmtHndl->stream, p, SEEK_SET );
    return end_p;
  }
}

D_OFFSET_T dimTell ( TDimFormatHandle *fmtHndl ) {
  if ( fmtHndl->stream == NULL ) return 0;  
  if ( fmtHndl->tellProc != NULL ) 
    return fmtHndl->tellProc( fmtHndl->stream );
  else
    return ftell( (FILE *) fmtHndl->stream );
}

DIM_INT dimEof ( TDimFormatHandle *fmtHndl ) {
  if ( fmtHndl->stream == NULL ) return 1;
  if ( fmtHndl->eofProc != NULL ) 
    return fmtHndl->eofProc( fmtHndl->stream );
  else
    return feof( (FILE *) fmtHndl->stream );
}

DIM_INT dimClose ( TDimFormatHandle *fmtHndl ) {
  if ( fmtHndl == NULL ) return EOF;
  if ( fmtHndl->stream == NULL ) return EOF;
  DIM_INT res;
  if ( fmtHndl->closeProc != NULL ) 
    res = fmtHndl->closeProc( fmtHndl->stream );
  else
    res = fclose( (FILE *) fmtHndl->stream );

  fmtHndl->stream = NULL;
  return res;
}
   
//------------------------------------------------------------------------------
// MISC
//------------------------------------------------------------------------------

DIM_UCHAR iTrimUC (int num) {
  if (num < 0) return 0;
  if (num > 255) return 255;
  return (DIM_UCHAR) num;
}

int trimInt(int i, int Min, int Max) {
  if (i>Max) return(Max);
  if (i<Min) return(Min);
  return(i);
}

//------------------------------------------------------------------------------
// SWAP TYPES
//------------------------------------------------------------------------------

void dimSwapShort(DIM_UINT16* wp) {
  register DIM_UCHAR* cp = (DIM_UCHAR*) wp;
  DIM_UCHAR t;

  t = cp[1]; cp[1] = cp[0]; cp[0] = t;
}

void dimSwapLong(DIM_UINT32* lp) {
  register DIM_UCHAR* cp = (DIM_UCHAR*) lp;
  DIM_UCHAR t;

  t = cp[3]; cp[3] = cp[0]; cp[0] = t;
  t = cp[2]; cp[2] = cp[1]; cp[1] = t;
}

void dimSwapArrayOfShort(DIM_UINT16* wp, register DIM_ULONG n) {
  register DIM_UCHAR* cp;
  register DIM_UCHAR t;

  while (n-- > 0) {
    cp = (DIM_UCHAR*) wp;
    t = cp[1]; cp[1] = cp[0]; cp[0] = t;
    wp++;
  }
}

void dimSwapArrayOfLong(register DIM_UINT32* lp, register DIM_ULONG n) {
  register unsigned char *cp;
  register unsigned char t;

  while (n-- > 0) 
  {
    cp = (unsigned char *)lp;
    t = cp[3]; cp[3] = cp[0]; cp[0] = t;
    t = cp[2]; cp[2] = cp[1]; cp[1] = t;
    lp++;
  }
}

void dimSwapDouble(double *dp) {
  register DIM_UINT32* lp = (DIM_UINT32*) dp;
  DIM_UINT32 t;

  dimSwapArrayOfLong(lp, 2);
  t = lp[0]; lp[0] = lp[1]; lp[1] = t;
}

void dimSwapArrayOfDouble(double* dp, register DIM_ULONG n) {
  register DIM_UINT32* lp = (DIM_UINT32*) dp;
  register DIM_UINT32 t;

  dimSwapArrayOfLong(lp, n + n);
  while (n-- > 0) 
  {
    t = lp[0]; lp[0] = lp[1]; lp[1] = t;
    lp += 2;
  }
}

void dimSwapData(int type, long size, void* data) {
  if ( (type == DIM_TAG_SHORT) || (type == DIM_TAG_SSHORT) )
    dimSwapArrayOfShort( (DIM_UINT16*) data, size );

  if ( (type == DIM_TAG_LONG) || (type == DIM_TAG_SLONG) || (type == DIM_TAG_FLOAT) )
    dimSwapArrayOfLong( (DIM_UINT32*) data, size );

  if (type == DIM_TAG_RATIONAL)
    dimSwapArrayOfLong( (DIM_UINT32*) data, size );

  if (type == DIM_TAG_DOUBLE)
    dimSwapArrayOfDouble( (double*) data, size );
}

//------------------------------------------------------------------------------
// Init parameters
//------------------------------------------------------------------------------

TDimFormatHandle initTDimFormatHandle()
{
  TDimFormatHandle tp;
  tp.ver = sizeof(TDimFormatHandle);
  
  tp.showProgressProc = NULL;
  tp.showErrorProc = NULL;
  tp.testAbortProc = NULL;
  tp.mallocProc = NULL;
  tp.freeProc = NULL;  

  tp.subFormat = 0;
  tp.pageNumber = 0;
  tp.resolutionLevel = 0;
  tp.quality = 0;
  tp.compression = 0;
  tp.order = 0;
  tp.metaData.count = 0;
  tp.metaData.tags = NULL;
  tp.roiX = 0;
  tp.roiY = 0;
  tp.roiW = 0;
  tp.roiH = 0;

  tp.imageServicesProcs = NULL;
  tp.internalParams = NULL;
  tp.fileName = NULL;
  tp.parent = NULL;

  tp.stream  = NULL;
  tp.io_mode = DIM_IO_READ;

  tp.image = NULL;
  tp.options = NULL;
  
  tp.readProc  = NULL;
  tp.writeProc = NULL;
  tp.flushProc = NULL;
  tp.seekProc  = NULL;
  tp.sizeProc  = NULL;
  tp.tellProc  = NULL;
  tp.eofProc   = NULL;
  tp.closeProc = NULL;

  
  return tp;
}


TDimImageInfo initTDimImageInfo()
{
  TDimImageInfo tp;
  tp.ver = sizeof(TDimImageInfo);
  
  tp.width = 0;
  tp.height = 0;
  
  tp.number_pages = 0;
  tp.number_levels = 0;
    
  tp.number_t = 1;
  tp.number_z = 1;

  tp.transparentIndex = 0;
  tp.transparencyMatting = 0;
    
  tp.imageMode = 0;
  tp.samples = 0;
  tp.depth = 0;
  tp.pixelType = D_FMT_UNSIGNED;
  tp.rowAlignment = DIM_TAG_BYTE;

  tp.resUnits = 0;  
  tp.xRes = 0;
  tp.yRes = 0;
  
  tp.tileWidth = 0;
  tp.tileHeight = 0;

  tp.lut.count = 0;

  tp.file_format_id = 0;

  DIM_UINT i;

  // dimensions
  for (i=0; i<DIM_MAX_DIMS; ++i)
  {
    tp.dimensions[i].dim = DIM_DIM_0;
    tp.dimensions[i].description = NULL;
    tp.dimensions[i].ext = NULL;
  }

  tp.number_dims = 3;
  for (i=0; i<tp.number_dims; ++i)
  {
    tp.dimensions[i].dim = (DIM_ImageDims) (DIM_DIM_0+i+1);
  }

  // channels
  for (i=0; i<DIM_MAX_CHANNELS; ++i)
  {
    tp.channels[i].description = NULL;
    tp.channels[i].ext = NULL;
  }

  return tp;
}


//------------------------------------------------------------------------------
// TDimImageBitmap
//------------------------------------------------------------------------------

long getLineSizeInBytes(TDimImageBitmap *img) {
  return (long) ceil( ((double)(img->i.width * img->i.depth)) / 8.0 );
}

long getImgSizeInBytes(TDimImageBitmap *img)
{
  long size = (long) ceil( ((double)(img->i.width * img->i.depth)) / 8.0 ) * img->i.height;
  return size;
}

long getImgNumColors(TDimImageBitmap *img)
{
  return (long) pow( 2.0f, (float)(img->i.depth * img->i.samples) );
}

void initImagePlanes(TDimImageBitmap *bmp)
{
  if (bmp == NULL) return;
  DIM_UINT i;

  bmp->i = initTDimImageInfo();

  for (i=0; i<512; i++)
    bmp->bits[i] = NULL;  
}

int allocImg( TDimImageBitmap *img, DIM_UINT w, DIM_UINT h, DIM_UINT samples, DIM_UINT depth)
{
  DIM_UINT sample;
  if (img == NULL) return 1;
  
  for (sample=0; sample<img->i.samples; sample++) 
    if (img->bits[sample] != NULL) 
      img->bits[sample] = DimFree( img->bits[sample] );

  initImagePlanes( img );

  img->i.width = w;
  img->i.height = h;
  img->i.imageMode = 0;
  img->i.samples = samples;
  img->i.depth = depth;
  long size = getImgSizeInBytes( img );

  for (sample=0; sample<img->i.samples; sample++)
  {
    //img->bits[sample] = (DIM_UCHAR *) DimMalloc( size );
	img->bits[sample] = new DIM_UCHAR [size];
    if (img->bits[sample] == NULL) return 1;
  }
  return 0;
}

void deleteImg(TDimImageBitmap *img)
{
  if (img == NULL) return;
  DIM_UINT sample=0;

  for (sample=0; sample<img->i.samples; ++sample) {
    if (img->bits[sample]) {
      void *p = img->bits[sample];
      for (unsigned int i=0; i<img->i.samples; ++i)
        if (img->bits[i] == p) img->bits[i] = NULL;
      delete (DIM_UCHAR*) p;
    } // if channel found
  } // for samples
}

int allocImg( TDimFormatHandle *fmtHndl, TDimImageBitmap *img, DIM_UINT w, DIM_UINT h, DIM_UINT samples, DIM_UINT depth)
{
  DIM_UINT sample;
  if (img == NULL) return 1;
  
  for (sample=0; sample<img->i.samples; sample++) 
    if (img->bits[sample] != NULL) 
      img->bits[sample] = dimFree( fmtHndl, img->bits[sample] );

  initImagePlanes( img );

  img->i.width = w;
  img->i.height = h;
  img->i.samples = samples;
  img->i.depth = depth;
  long size = getImgSizeInBytes( img );

  for (sample=0; sample<img->i.samples; sample++)
  {
    img->bits[sample] = (DIM_UCHAR *) dimMalloc( fmtHndl, size );
    if (img->bits[sample] == NULL) return 1;
  }
  return 0;
}

// alloc image using info
int allocImg( TDimFormatHandle *fmtHndl, TDimImageInfo *info, TDimImageBitmap *img)
{
  if (fmtHndl == NULL) return 1;
  if (info == NULL) return 1;
  if (img == NULL) return 1;

  TDimImageInfo ii = *info;
  img->i = ii;
  if ( allocImg( fmtHndl, img, info->width, info->height, info->samples, info->depth ) == 0)
  {
    img->i = ii;
    return 0;
  }
  else return 1;
}

// alloc handle image using info
int allocImg( TDimFormatHandle *fmtHndl, TDimImageInfo *info )
{
  return allocImg( fmtHndl, info, fmtHndl->image ); 
}

void deleteImg( TDimFormatHandle *fmtHndl, TDimImageBitmap *img) {
  if (img == NULL) return;
  DIM_UINT sample=0;

  for (sample=0; sample<img->i.samples; ++sample) {
    if (img->bits[sample]) {
      void *p = img->bits[sample];
      for (unsigned int i=0; i<img->i.samples; ++i)
        if (img->bits[i] == p) img->bits[i] = NULL;
      dimFree( fmtHndl, p );
    } // if channel found
  } // for samples
}

int getSampleHistogram(TDimImageBitmap *img, long *hist, int sample)
{
  if (img == 0) return -1;
  DIM_UINT i;
  int num_used = 0;
  unsigned long size = img->i.width * img->i.height;
  DIM_UINT max_uint16 = (DIM_UINT16) -1;
  DIM_UINT max_uchar = (DIM_UCHAR) -1;  

  if (img->i.depth == 16) 
  {
    DIM_UINT16 *p = (DIM_UINT16 *) img->bits[sample];  
    for (i=0; i<=max_uint16; i++) hist[i] = 0;
    for (i=0; i<size; i++) {
      ++hist[*p];
      p++;
    }
    for (i=0; i<=max_uint16; i++) if (hist[i] != 0) ++num_used;
  }
  else // 8bit
  {
    DIM_UCHAR *p = (DIM_UCHAR *) img->bits[sample];  
    for (i=0; i<=max_uchar; i++) hist[i] = 0;
    for (i=0; i<size; i++) {
      ++hist[*p];
      p++;
    }
    for (i=0; i<=max_uchar; i++) if (hist[i] != 0) ++num_used;
  }

  return 0;
}

int normalizeImg(TDimImageBitmap *img, TDimImageBitmap *img8)
{
  DIM_UINT max_uint16 = (DIM_UINT16) -1;
  long hist[65536];
  DIM_UCHAR lut[65536];
  DIM_UINT min_col = 0;
  DIM_UINT max_col = max_uint16;
  unsigned int i;
  if (img->i.depth != 16) return -1;
  int disp_range = 256;

  DIM_ULONG size = img->i.width * img->i.height;
  DIM_UINT sample=0;

  for (sample=0; sample<img->i.samples; sample++)
  {
    DIM_UINT16 *p16 = (DIM_UINT16 *) img->bits[sample];
    DIM_UCHAR *buf8 = (DIM_UCHAR *) img8->bits[sample];
    DIM_UCHAR *p8 = buf8;

    getSampleHistogram(img, hist, sample);

    for (i=0; i<=max_uint16; i++) {
      if (hist[i] != 0) 
      { min_col = i; break; }
    }
    
    for (i=max_uint16; i>=0; i--) {
      if (hist[i] != 0) 
      { max_col = i; break; }
    }

    int range = max_col - min_col;
    if (range < 1) range = disp_range;

    for (i=0; i<=max_uint16; i++)  
      lut[i] = iTrimUC ( (i-min_col)*disp_range / range );

    for (i=0; i<size; i++) *(p8+i) = lut[*(p16+i)];
  }

  #if defined (DEBUG) || defined (_DEBUG)
  printf("Normalized size: %d max: %d min: %d\n", size, max_col, min_col );  
  #endif

  return 0;
}


int resizeImgNearNeighbor( TDimImageBitmap *img, unsigned int newWidth, unsigned int newHeight)
{
  float hRatio, vRatio;

  if (img == NULL) return 1;
  if ((img->i.width == newWidth) && (img->i.height == newHeight)) return 1;
  if ((newWidth == 0) || (newHeight == 0)) return 1;

  TDimImageBitmap oBmp;
  initImagePlanes( &oBmp );
  oBmp.i = img->i;
  allocImg( &oBmp, newWidth, newHeight, oBmp.i.samples, oBmp.i.depth );
  int newLineSize = getLineSizeInBytes( &oBmp );
  int oldLineSize = getLineSizeInBytes( img );

  hRatio = ((float) img->i.width)  / ((float) newWidth);
  vRatio = ((float) img->i.height) / ((float) newHeight);

  DIM_UINT sample=0;
  for (sample=0; sample<img->i.samples; sample++)
  {
    register unsigned int x, y;
    unsigned char *p, *po;
    unsigned int xNew, yNew;

    p = (unsigned char *) oBmp.bits[sample];
    for (y=0; y<newHeight; y++)
    {
      for (x=0; x<newWidth; x++)
      {
        xNew = trimInt((int) (x*hRatio), 0, img->i.width-1);
        yNew = trimInt((int) (y*vRatio), 0, img->i.height-1);
        po = ( (unsigned char *) img->bits[sample] ) + (oldLineSize * yNew);
        
        if (oBmp.i.depth == 8)
          p[x] = po[xNew];
        else
        if (oBmp.i.depth == 16)
        {
          DIM_UINT16 *p16  = (DIM_UINT16 *) p;
          DIM_UINT16 *po16 = (DIM_UINT16 *) po;
          p16[x] = po16[xNew];
        }
      }
      p += newLineSize;
    }

  } // sample 

  deleteImg( img );
  *img = oBmp;
  return(0);
}

int retreiveImgROI( TDimImageBitmap *img, DIM_ULONG x, DIM_ULONG y, DIM_ULONG w, DIM_ULONG h )
{
  if (img == NULL) return 1;
  if ((w == 0) || (h == 0)) return 1;
  if ((img->i.width-x <= w) || (img->i.height-y <= h)) return 1;

  TDimImageBitmap nBmp;
  initImagePlanes( &nBmp );
  nBmp.i = img->i;
  allocImg( &nBmp, w, h, nBmp.i.samples, nBmp.i.depth );
  int newLineSize = getLineSizeInBytes( &nBmp );
  int oldLineSize = getLineSizeInBytes( img );
  int Bpp = (long) ceil( ((double)img->i.depth) / 8.0 );

  DIM_UINT sample=0;
  for (sample=0; sample<img->i.samples; sample++)
  {
    register unsigned int yi;
    unsigned char *pl  = (unsigned char *) nBmp.bits[sample];
    unsigned char *plo = ( (unsigned char *) img->bits[sample] ) + y*oldLineSize + x*Bpp;

    for (yi=0; yi<h; yi++)
    {
      memcpy( pl, plo, w*Bpp );      
      pl  += newLineSize;
      plo += oldLineSize;
    } // for yi

  } // sample 

  deleteImg( img );
  *img = nBmp;
  return(0);
}

std::string getImageInfoText(TDimImageBitmap *img)
{
  std::string inftext="";
  if (img == NULL) return inftext;
  char line[1024];

  sprintf(line, "pages: %d\n",     img->i.number_pages ); inftext+=line;
  sprintf(line, "channels: %d\n",  img->i.samples );  inftext+=line;
  sprintf(line, "width: %d\n",     img->i.width);  inftext+=line;
  sprintf(line, "height: %d\n",    img->i.height);    inftext+=line;
  sprintf(line, "zsize: %d\n",     img->i.number_z );  inftext+=line;
  sprintf(line, "tsize: %d\n",     img->i.number_t );  inftext+=line;
  sprintf(line, "depth: %d\n",     img->i.depth );  inftext+=line;
  sprintf(line, "pixelType: %d\n", img->i.pixelType );  inftext+=line;
  if (dimBigendian) inftext+="endian: big\n"; else inftext+="endian: little\n";
  sprintf(line, "resUnits: %d\n",  img->i.resUnits ); inftext+=line;
  sprintf(line, "xRes: %f\n",      img->i.xRes );  inftext+=line;
  sprintf(line, "yRes: %f\n",      img->i.yRes );  inftext+=line;
  //sprintf(line, "format: %s\n",      img->i.yRes );  inftext+=line;

  unsigned int i;

  inftext += "dimensions:";
  try {
    if (img->i.number_dims >= DIM_MAX_DIMS) img->i.number_dims = DIM_MAX_DIMS-1;
    for (i=0; i<img->i.number_dims; ++i) {
      //sprintf(line, ": %d-%s\n", img->i.dimensions[i].dim, img->i.dimensions[i].description );
      sprintf(line, " %s", dimNames[img->i.dimensions[i].dim] );
      inftext+=line;
    } 
  } catch (...) {
    inftext += "unknown";     
  }
  inftext += "\n";  


  inftext += "channelsDescription:";  
  try {
    for (i=0; i<img->i.samples; ++i)
      if (img->i.channels[i].description != NULL) 
      {
        inftext += " ";
        inftext += img->i.channels[i].description;
      } 
  } catch (...) {
    inftext += "unknown";     
  }
  inftext += "\n";  

  return inftext;
}


//------------------------------------------------------------------------------
// Metadata
//------------------------------------------------------------------------------

void initMetaTag(TDimTagItem *tagItem)
{
  tagItem->tagGroup = 0;  
  tagItem->tagId = 0;     
  tagItem->tagType = 0;  
  tagItem->tagLength = 0; 
  tagItem->tagData = NULL;
}

void clearMetaTag(TDimTagItem *tagItem)
{
  if (tagItem == NULL) return;
  if (tagItem->tagLength == 0) return;
  if (tagItem->tagData == NULL) return;

  DimFree( tagItem->tagData );
  initMetaTag(tagItem);
}

void clearMetaTags(TDimTagList *tagList)
{
  if (tagList == NULL) return;
  if (tagList->count == 0) return;
  if (tagList->tags == NULL) return;

  tagList->tags = (TDimTagItem *) DimFree( tagList->tags );
  tagList->count = 0;
}

bool isTagPresent(TDimTagList *tagList, int group, int tag)
{
  if (tagList == NULL) return false;
  if (tagList->count == 0) return false;
  if (tagList->tags == NULL) return false;
  
  DIM_UINT i;
  for (i=0; i<tagList->count; i++)
    if ( (tagList->tags[i].tagId == (DIM_UINT) tag) && 
         (tagList->tags[i].tagGroup == (DIM_UINT) group) ) return true;
  
  return false;
}

int tagPos(TDimTagList *tagList, int group, int tag)
{
  if (tagList == NULL) return false;
  if (tagList->count == 0) return false;
  if (tagList->tags == NULL) return false;
  
  DIM_UINT i;
  for (i=0; i<tagList->count; i++)
    if  ( (tagList->tags[i].tagId == (DIM_UINT) tag) &&
          (tagList->tags[i].tagGroup == (DIM_UINT) group) )
    return i;
  
  return -1;
}

int addMetaTag(TDimTagList *tagList, TDimTagItem item)
{
  DIM_UINT i;
  if (tagList == NULL) return 1;

  if (tagList->tags == NULL) tagList->count = 0;

  if (tagList->count != 0)
  for (i=0; i<tagList->count; i++)
    if ( (tagList->tags[i].tagId == item.tagId) &&
         (tagList->tags[i].tagGroup == item.tagGroup) )
    {
      tagList->tags[i] = item;  
      return 0;
    }
  
  // tag was not found - add
  {
    TDimTagItem *newTags = (TDimTagItem *) DimMalloc( (tagList->count+1) * sizeof(TDimTagItem) );
    if (newTags == NULL) return 1;

    for (i=0; i<tagList->count; i++) newTags[i] = tagList->tags[i];
    newTags[tagList->count] = item;
    DimFree( tagList->tags );
    tagList->tags = newTags;
    tagList->count++;
  }

  return 0;
}





