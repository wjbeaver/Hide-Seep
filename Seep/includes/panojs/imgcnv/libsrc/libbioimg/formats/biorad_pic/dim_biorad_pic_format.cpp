/*****************************************************************************
  BIORAD PIC support
  UCSB/BioITR property  
  Copyright (c) 2004 by Dmitry V. Fedorov <www.dimin.net> <dima@dimin.net>

  IMPLEMENTATION
  
  Programmer: Dima V. Fedorov <mailto:dima@dimin.net> <http://www.dimin.net/>

  Note:
    Now supports Big-Endian and Small-Endian arcitectures

  History:
    04/22/2004 13:06 - First creation
    05/10/2004 14:55 - Big endian support
    08/04/2004 22:25 - Update to FMT_IFS 1.2, support for io protorypes
        
  Ver : 3
*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string>

#include "dim_biorad_pic_format.h"
#include "dim_biorad_pic_format_io.cpp"

// windows: use secure C libraries with VS2005 or higher
#if ( defined(_MSC_VER) && (_MSC_VER >= 1400) )
  #pragma warning(disable:4996)
  #pragma message(">>>>> dim_biorad_pic_format: ignoring secure libraries")
#endif 


//****************************************************************************
// Misc
//****************************************************************************

const char months[13][4] = {"", "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

int biorad_getMonthNum(const char *buf) {
  for (int i=1; i<=12; i++)
    if (strncmp( months[i], buf, 3 ) == 0) return i;
  return 0;
}

DBioRadPicParams::DBioRadPicParams(): num_images(0), page_size_bytes(0), data_offset(76), notes_offset(0), has_notes(0), magnification(0)
{
  i = initTDimImageInfo(); 
  i.depth  = 8;
}


//****************************************************************************
//
// INTERNAL STRUCTURES
//
//****************************************************************************

void bioradReadNotes( TDimFormatHandle *fmtHndl ) {
  if (fmtHndl == NULL) return;
  if (fmtHndl->internalParams == NULL) return;
  DBioRadPicParams *par = (DBioRadPicParams *) fmtHndl->internalParams; 
  
  par->note01 = ""; 
  par->note20 = ""; 
  par->note21 = ""; 

  char *buf = NULL;
  std::string str = ""; 
  unsigned char note[96];

  // print biorad notes into the file
  dimSeek(fmtHndl, par->notes_offset, SEEK_SET);
  while (dimRead( fmtHndl, note, 1, 96 ) == 96) {
     short note_type  = * (DIM_INT16 *) (note + 10);
     if (dimBigendian) dimSwapShort ( (DIM_UINT16*) &note_type );
     char *text     = (char *) (note + 16);
     if (note_type == 1)  { if (text[0] == '\0') continue; par->note01 += text; par->note01 += '\n'; }
     if (note_type == 20) { if (text[0] == '\0') continue; par->note20 += text; par->note20 += '\n'; }
     if (note_type == 21) { if (text[0] == '\0') continue; par->note21 += text; par->note21 += '\n'; }
  }
}

void bioradParseNote01( TDimFormatHandle *fmtHndl )
{
  if (fmtHndl == NULL) return;
  if (fmtHndl->internalParams == NULL) return;
  DBioRadPicParams *par = (DBioRadPicParams *) fmtHndl->internalParams;
  if (par->note01.size()<=0) return;


  char t[1024], month[3];
  int m=0, y, d, h, mi, s;

  //Live Thu Aug  7 13:42:13 2003  Zoom 2.0 Kalman 3 Mixer A PMT 
  sscanf( par->note01.c_str(), "Live %s %3s %d %d:%d:%d %d", t, month, &d, &h, &mi, &s, &y ); 
  m = biorad_getMonthNum( month );
  xstring xs;
  xs.sprintf("%.4d-%.2d-%.2d %.2d:%.2d:%.2d", y, m, d, h, mi, s);
  par->datetime = xs;
}

void bioradParseNote20( TDimFormatHandle *fmtHndl )
{
  if (fmtHndl == NULL) return;
  if (fmtHndl->internalParams == NULL) return;
  DBioRadPicParams *par = (DBioRadPicParams *) fmtHndl->internalParams;
  TDimImageInfo *info = &par->i;    
  
  double pixel_size_x=0.0, pixel_size_y=0.0, pixel_size_z=0.0; //in microns

  // now parse it
  if (par->note20.size()<=0) return;
  std::string str = par->note20; 

  //int i=0;
  char *line;
  int d=0;
  float f=0.0, res=0.0;
  char t[1024];
  
  // Common:
  //AXIS_2 = 257 0.000000e+00 2.689307e-01 microns 
  // Uncommon way
  //AXIS_2 257 0.000000e+00 2.286561e-01 microns


  // X
  line = (char*) strstr( str.c_str(), "AXIS_2 =" );
  if (line != NULL)
    sscanf( line, "AXIS_2 = %d %e %e microns ", &d, &f, &res );
  else
  {
    line = (char*) strstr( str.c_str(), "AXIS_2 " );
    if (line != NULL)
      sscanf( line, "%s %d %e %e microns ", t, &d, &f, &res );
  }
  pixel_size_x = res;
    

  // Y
  line = (char*) strstr( str.c_str(), "AXIS_3 =" );
  if (line != NULL)
    sscanf( line, "AXIS_3 = %d %e %e microns ", &d, &f, &res );
  else  
  {
    line = (char*) strstr( str.c_str(), "AXIS_3 " );
    if (line != NULL)
      sscanf( line, "%s %d %e %e microns ", t, &d, &f, &res );
  }
  pixel_size_y = res;


  // Z
  line = (char*) strstr( str.c_str(), "AXIS_4 =" );
  if (line != NULL)
    sscanf( line, "AXIS_4 = %d %e %e microns ", &d, &f, &res );
  else  
  {
    line = (char*) strstr( str.c_str(), "AXIS_4 " );
    if (line != NULL)
      sscanf( line, "%s %d %e %e microns ", t, &d, &f, &res );
  }
  pixel_size_z = res;

  info->resUnits = DIM_RES_um;
  info->xRes = pixel_size_x;
  info->yRes = pixel_size_y;

  par->pixel_size[0] = pixel_size_x;
  par->pixel_size[1] = pixel_size_y;
  par->pixel_size[2] = pixel_size_z;
}


void bioradGetImageInfo( TDimFormatHandle *fmtHndl )
{
  unsigned char header[76];
  DIM_INT16 val;

  if (fmtHndl == NULL) return;
  if (fmtHndl->internalParams == NULL) return;
  DBioRadPicParams *picPar = (DBioRadPicParams *) fmtHndl->internalParams;
  TDimImageInfo *info = &picPar->i;  

  *info = initTDimImageInfo();
  info->imageMode = DIM_GRAYSCALE;
  info->tileWidth = 0;
  info->tileHeight = 0; 
  info->transparentIndex = 0;
  info->transparencyMatting = 0;
  info->lut.count = 0;
  info->samples = 1;

  if (fmtHndl->stream == NULL) return;
  if ( dimSeek(fmtHndl, 0, SEEK_SET) != 0) return;
  if ( dimRead( fmtHndl, header, 1, 76 ) != 76) return;

  info->number_pages = *(header+4) + *(header+5) * 256; // multiplatform approach, no swapping needed
  info->width  = *(header + 0) + *(header + 1) * 256;
  info->height = *(header + 2) + *(header + 3) * 256;

  picPar->has_notes = * (DIM_INT32 *) (header + 10);
  if (dimBigendian) dimSwapLong ( (DIM_UINT32*) &picPar->has_notes );
  picPar->num_images = info->number_pages;


  val = (short)( *(header + 14) + *(header + 15) * 256 ); 
  if (val == 1)   {
    info->depth = 8;
    info->pixelType = D_FMT_UNSIGNED;
  }  else   {
    info->depth = 16;
    info->pixelType = D_FMT_UNSIGNED;
  }

  picPar->data_offset = 76;
  picPar->page_size_bytes = info->width * info->height * (info->depth / 8);
  picPar->notes_offset = picPar->data_offset + picPar->page_size_bytes * info->number_pages;

  if ( (info->number_pages == 3) || (info->number_pages == 2) )   {
    info->samples = 3;    
    info->number_pages = 1;
    info->imageMode = DIM_RGB;
  }

  bioradReadNotes( fmtHndl );
  bioradParseNote01( fmtHndl );
  bioradParseNote20( fmtHndl );

  dimSeek(fmtHndl, 76, SEEK_SET);

  // if more than 1 page it's a z series
  if (info->number_pages > 1) {
    info->number_dims = 4;
    info->dimensions[3].dim = DIM_DIM_Z;
    info->number_z = info->number_pages;
  }
}

//****************************************************************************
//
// FORMAT DEMANDED FUNTIONS
//
//****************************************************************************


//----------------------------------------------------------------------------
// PARAMETERS, INITS
//----------------------------------------------------------------------------

DIM_INT dimBioRadPicValidateFormatProc (DIM_MAGIC_STREAM *magic, DIM_UINT length)
{
  if (length < 56) return -1;
  unsigned char *mag_num = (unsigned char *) magic;
  if ( (mag_num[54] == 0x39) && (mag_num[55] == 0x30) ) return 0;
  return -1;
}

TDimFormatHandle dimBioRadPicAquireFormatProc( void )
{
  TDimFormatHandle fp = initTDimFormatHandle();
  return fp;
}

void dimBioRadPicReleaseFormatProc (TDimFormatHandle *fmtHndl)
{
  if (fmtHndl == NULL) return;
  dimBioRadPicCloseImageProc ( fmtHndl );  
}


//----------------------------------------------------------------------------
// OPEN/CLOSE
//----------------------------------------------------------------------------
void dimBioRadPicCloseImageProc (TDimFormatHandle *fmtHndl)
{
  if (fmtHndl == NULL) return;
  dimClose ( fmtHndl );
  DBioRadPicParams *par = (DBioRadPicParams *) fmtHndl->internalParams;
  fmtHndl->internalParams = 0;
  delete par;
}

DIM_UINT dimBioRadPicOpenImageProc  (TDimFormatHandle *fmtHndl, DIM_ImageIOModes io_mode)
{
  if (fmtHndl == NULL) return 1;
  if (fmtHndl->internalParams != NULL) dimBioRadPicCloseImageProc (fmtHndl);  
  fmtHndl->internalParams = (void *) new DBioRadPicParams();

  if (io_mode == DIM_IO_READ)
  {
    if ( isCustomReading ( fmtHndl ) != TRUE )
      fmtHndl->stream = fopen( fmtHndl->fileName, "rb" );

    if (fmtHndl->stream == NULL) return 1;
    bioradGetImageInfo( fmtHndl );
  }
  else return 1;

  return 0;
}


//----------------------------------------------------------------------------
// INFO for OPEN image
//----------------------------------------------------------------------------

DIM_UINT dimBioRadPicGetNumPagesProc ( TDimFormatHandle *fmtHndl )
{
  if (fmtHndl == NULL) return 0;
  if (fmtHndl->internalParams == NULL) return 0;
  DBioRadPicParams *picPar = (DBioRadPicParams *) fmtHndl->internalParams;

  return picPar->i.number_pages;
}


TDimImageInfo dimBioRadPicGetImageInfoProc ( TDimFormatHandle *fmtHndl, DIM_UINT page_num )
{
  TDimImageInfo ii = initTDimImageInfo();
  page_num;

  if (fmtHndl == NULL) return ii;
  DBioRadPicParams *picPar = (DBioRadPicParams *) fmtHndl->internalParams;

  return picPar->i;
}

//----------------------------------------------------------------------------
// METADATA
//----------------------------------------------------------------------------

DIM_UINT dimBioRadPicAddMetaDataProc (TDimFormatHandle *fmtHndl)
{
  fmtHndl=fmtHndl;
  return 1;
}


DIM_UINT dimBioRadPicReadMetaDataProc (TDimFormatHandle *fmtHndl, DIM_UINT page, int group, int tag, int type)
{
  if (fmtHndl == NULL) return 1;
  page;
  return read_biorad_metadata (fmtHndl, group, tag, type);
}

char* dimBioRadPicReadMetaDataAsTextProc ( TDimFormatHandle *fmtHndl )
{
  return NULL;
}


//----------------------------------------------------------------------------
// READ/WRITE
//----------------------------------------------------------------------------

DIM_UINT dimBioRadPicReadImageProc  ( TDimFormatHandle *fmtHndl, DIM_UINT page )
{
  if (fmtHndl == NULL) return 1;
  if (fmtHndl->stream == NULL) return 1;

  fmtHndl->pageNumber = page;
  return read_biorad_image( fmtHndl );
}

DIM_UINT dimBioRadPicWriteImageProc ( TDimFormatHandle *fmtHndl )
{
  return 1;
  fmtHndl;
}

/*
// at the moment w and h make no effect,the image retreived is the same size as original
DIM_UINT dimJpegReadImagePreviewProc (TDimFormatHandle *fmtHndl, DIM_UINT w, DIM_UINT h)
{
  TDimImageBitmap bmp8, *bmp;
  initImagePlanes( &bmp8 );

  w=w; h=h;
  
  if ( dimTiffReadImageProc( fmtHndl, 0 ) != 0) return 1;
  bmp = fmtHndl->image;
  
  if (bmp->i.samples > 3) bmp->i.samples = 3;

  if (bmp->i.depth == 16) {
    allocImg(&bmp8, bmp->i.width, bmp->i.height, bmp->i.samples, 8);
    normalizeImg(bmp, &bmp8);
    deleteImg( bmp );

    bmp->bits[0] = bmp8.bits[0]; 
    bmp->bits[1] = bmp8.bits[1];    
    bmp->bits[2] = bmp8.bits[2]; 
  }

  bmp->i.depth = 8;
  
  if (bmp->i.samples == 1) {
    bmp->bits[1] = bmp->bits[0];    
    bmp->bits[2] = bmp->bits[0]; 
  }

  if (bmp->i.samples == 2) {
    bmp->bits[2] = bmp->bits[0]; 
  }
  bmp->i.samples = 3;

  return 0;
}

*/




//****************************************************************************
//
// EXPORTED FUNCTION
//
//****************************************************************************

TDimFormatItem dimBioRadPicItems[1] = {
  {
    "BIORAD-PIC",            // short name, no spaces
    "BioRad PIC file", // Long format name
    "pic",        // pipe "|" separated supported extension list
    1, //canRead;      // 0 - NO, 1 - YES
    0, //canWrite;     // 0 - NO, 1 - YES
    1, //canReadMeta;  // 0 - NO, 1 - YES
    0, //canWriteMeta; // 0 - NO, 1 - YES
    0, //canWriteMultiPage;   // 0 - NO, 1 - YES
    //TDivFormatConstrains constrains ( w, h, pages, minsampl, maxsampl, minbitsampl, maxbitsampl, noLut )
    { 0, 0, 0, 1, 1, 8, 16, 1 } 
  }
};

TDimFormatHeader dimBioRadPicHeader = {

  sizeof(TDimFormatHeader),
  "1.0.0",
  "DIMIN BIORAD PIC CODEC",
  "BIORAD PIC CODEC",
  
  56,                      // 0 or more, specify number of bytes needed to identify the file
  {1, 1, dimBioRadPicItems},   //dimJpegSupported,
  
  dimBioRadPicValidateFormatProc,
  // begin
  dimBioRadPicAquireFormatProc, //TDimAquireFormatProc
  // end
  dimBioRadPicReleaseFormatProc, //TDimReleaseFormatProc
  
  // params
  NULL, //TDimAquireIntParamsProc
  NULL, //TDimLoadFormatParamsProc
  NULL, //TDimStoreFormatParamsProc

  // image begin
  dimBioRadPicOpenImageProc, //TDimOpenImageProc
  dimBioRadPicCloseImageProc, //TDimCloseImageProc 

  // info
  dimBioRadPicGetNumPagesProc, //TDimGetNumPagesProc
  dimBioRadPicGetImageInfoProc, //TDimGetImageInfoProc


  // read/write
  dimBioRadPicReadImageProc, //TDimReadImageProc 
  NULL, //TDimWriteImageProc
  NULL, //TDimReadImageTileProc
  NULL, //TDimWriteImageTileProc
  NULL, //TDimReadImageLineProc
  NULL, //TDimWriteImageLineProc
  NULL, //TDimReadImageThumbProc
  NULL, //TDimWriteImageThumbProc
  NULL, //dimJpegReadImagePreviewProc, //TDimReadImagePreviewProc
  
  // meta data
  dimBioRadPicReadMetaDataProc, //TDimReadMetaDataProc
  dimBioRadPicAddMetaDataProc,  //TDimAddMetaDataProc
  dimBioRadPicReadMetaDataAsTextProc, //TDimReadMetaDataAsTextProc
  biorad_append_metadata, //TDimAppendMetaDataProc

  NULL,
  NULL,
  ""

};

extern "C" {

TDimFormatHeader* dimBioRadPicGetFormatHeader(void)
{
  return &dimBioRadPicHeader;
}

} // extern C


