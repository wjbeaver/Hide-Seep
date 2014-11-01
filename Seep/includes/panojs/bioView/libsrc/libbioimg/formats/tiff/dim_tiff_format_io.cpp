/*****************************************************************************
  TIFF IO 
  Copyright (c) 2004 by Dmitry V. Fedorov <www.dimin.net> <dima@dimin.net>

  IMPLEMENTATION
  
  Programmer: Dima V. Fedorov <mailto:dima@dimin.net> <http://www.dimin.net/>
    
  TODO:
    4) read preview image in RGB 8bit

  History:
    03/29/2004 22:23 - First creation
        
  Ver : 1
*****************************************************************************/

#include <cstdio>
#include <cstdlib>
#include <cmath>

#include <limits>

#include "memio.h"
#include "dim_tiny_tiff.h"
#include "dim_tiff_format.h"

#ifndef MAX
#  define MIN(a,b)      ((a<b) ? a : b)
#  define MAX(a,b)      ((a>b) ? a : b)
#endif

// Disables Visual Studio 2005 warnings for deprecated code
#if ( defined(_MSC_VER) && (_MSC_VER >= 1400) )
  #pragma warning(disable:4996)
#endif 

int invertImg(TDimImageBitmap *img);
int invertSample(TDimImageBitmap *img, const int &sample);

// must include these guys here if not no access to internal TIFF structs
#include "dim_tiny_tiff.cpp"
#include "dim_stk_format_io.cpp"
#include "dim_psia_format_io.cpp"
#include "dim_fluoview_format_io.cpp"
#include "dim_cz_lsm_format_io.cpp"
#include "dim_ometiff_format_io.cpp"

//****************************************************************************
// util procs
//****************************************************************************

template< typename T >
void invert_buffer(void *buf, const unsigned int &size) {
  T maxval = std::numeric_limits<T>::max();
  T *p = (T *) buf;  
  for (unsigned int i=0; i<size; i++)
    p[i] = maxval - p[i];
}

void invert_buffer_1bit(void *buf, const unsigned int &size) {
  int maxval = 1;
  int rest = size%8;
  unsigned int w = floor( size/8.0 );
  unsigned char *p = (unsigned char *) buf; 
  if (rest>0) ++w;

  for (unsigned int x=0; x<w; ++x) {
    unsigned char b[8];
    b[0] = maxval - (p[x] >> 7);
    b[1] = maxval - ((p[x] & 0x40) >> 6);
    b[2] = maxval - ((p[x] & 0x20) >> 5);
    b[3] = maxval - ((p[x] & 0x10) >> 4);
    b[4] = maxval - ((p[x] & 0x08) >> 3);
    b[5] = maxval - ((p[x] & 0x04) >> 2);
    b[6] = maxval - ((p[x] & 0x02) >> 1);
    b[7] = maxval - (p[x] & 0x01);
    p[x] = (b[0]<<7) + (b[1]<<6) + (b[2]<<5) + (b[3]<<4) + (b[4]<<3) + (b[5]<<2) + (b[6]<<1) + b[7];
  } // for x
}

void invert_buffer_4bit(void *buf, const unsigned int &size) {
  int maxval = 15;
  bool even = ( size%2 == 0 );
  unsigned int w = floor( size/2.0 );
  unsigned char *p = (unsigned char *) buf; 

  for (unsigned int x=0; x<w; ++x) {
    unsigned char b1 = maxval - (p[x] >> 4);
    unsigned char b2 = maxval - (p[x] & 0x0F);
    p[x] = (b1 << 4) + b2;
  } // for x

  // do the last pixel if the size is not even
  if (!even) {
    unsigned char b1 = maxval - (p[w] >> 4);
    p[w] = (b1 << 4);
  }
}

int invertSample(TDimImageBitmap *img, const int &sample) {
  unsigned int size = img->i.width * img->i.height; 

  // all typed will fall here
  if (img->i.depth==8 && img->i.pixelType==D_FMT_UNSIGNED)
    invert_buffer<DIM_UINT8>(img->bits[sample], size);
  else
  if (img->i.depth==8 && img->i.pixelType==D_FMT_SIGNED)
    invert_buffer<DIM_INT8>(img->bits[sample], size);
  else
  if (img->i.depth==16 && img->i.pixelType==D_FMT_UNSIGNED)
    invert_buffer<DIM_UINT16>(img->bits[sample], size);
  else
  if (img->i.depth==16 && img->i.pixelType==D_FMT_SIGNED)
    invert_buffer<DIM_INT16>(img->bits[sample], size);
  else
  if (img->i.depth==32 && img->i.pixelType==D_FMT_UNSIGNED)
    invert_buffer<DIM_UINT32>(img->bits[sample], size);
  else
  if (img->i.depth==32 && img->i.pixelType==D_FMT_SIGNED)
    invert_buffer<DIM_INT32>(img->bits[sample], size);
  else
  if (img->i.depth==32 && img->i.pixelType==D_FMT_FLOAT)
    invert_buffer<DIM_FLOAT32>(img->bits[sample], size);
  else
  if (img->i.depth==64 && img->i.pixelType==D_FMT_FLOAT)
    invert_buffer<DIM_FLOAT64>(img->bits[sample], size);
  else
  // we still have 1 and 4 bits
  if (img->i.depth==4 && img->i.pixelType==D_FMT_UNSIGNED)
    invert_buffer_4bit(img->bits[sample], size);
  else
  if (img->i.depth==1 && img->i.pixelType==D_FMT_UNSIGNED)
    invert_buffer_1bit(img->bits[sample], size);

  return 0;
}

int invertImg(TDimImageBitmap *img) {
  if (!img) return -1;
  for (unsigned int sample=0; sample<img->i.samples; sample++)
    invertSample(img, sample);
  return 0;
}

//****************************************************************************
// MISC
//****************************************************************************

bool areValidParams(TDimFormatHandle *fmtHndl, DTiffParams *tifParams)
{
  if (fmtHndl == NULL) return FALSE;
  if (tifParams == NULL) return FALSE;
  if (tifParams->dimTiff == NULL) return FALSE;
  if (fmtHndl->image == NULL) return FALSE;

  return TRUE;
}

void init_image_palette( TIFF *tif, TDimImageInfo *info ) {
  if (tif == NULL) return;
  if (info == NULL) return;
  uint16 photometric = PHOTOMETRIC_MINISWHITE;
  uint16 bitspersample = 1;  
  TIFFGetField(tif, TIFFTAG_PHOTOMETRIC, &photometric);
  TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &bitspersample);

  info->lut.count = 0;
  for (DIM_UINT i=0; i<256; i++) 
    info->lut.rgba[i] = dimRGB( i, i, i );
  
  if (photometric == PHOTOMETRIC_PALETTE) { // palette
    uint16 *red, *green, *blue;
    DIM_UINT num_colors = ( 1L << bitspersample );
    if (num_colors > 256) num_colors = 256;    

    TIFFGetField(tif, TIFFTAG_COLORMAP, &red, &green, &blue);
    for (DIM_UINT i=0; i<num_colors; i++)
      info->lut.rgba[i] = dimRGB( red[i]/256, green[i]/256, blue[i]/256 );

    info->lut.count = num_colors;
  } // if paletted
}

//****************************************************************************
// META DATA
//****************************************************************************

DIM_UINT read_one_tag (TDimFormatHandle *fmtHndl, DTiffParams *tifParams, int tag)
{
  if (!areValidParams(fmtHndl, tifParams)) return 1;
  if (tifParams->ifds.count == 0) return 1;

  if (fmtHndl->pageNumber >= tifParams->ifds.count) fmtHndl->pageNumber = 0;
  TIFF *tif = tifParams->dimTiff;
  TDimTiffIFDs *ifds = &tifParams->ifds;

  DIM_UCHAR *buf = NULL;
  uint32 buf_size;
  uint32 buf_type;

  if ( (tifParams->subType == tstStk) && (tag == 33629) ) // stk 33629 got custom size 6*N
  {
    buf_type = DIM_TAG_LONG;
    DIM_INT32 count = getTiffTagCount(tif, &ifds->ifds[fmtHndl->pageNumber], tag);
    buf_size = ( count * 6 ) * tag_size_bytes[buf_type];
    readTiffCustomTag (tif, &ifds->ifds[fmtHndl->pageNumber], tag, buf_size, buf_type, (DIM_UCHAR **) &buf);
  }
  else
    readTiffTag (tif, &ifds->ifds[fmtHndl->pageNumber], tag, buf_size, buf_type, (DIM_UCHAR **) &buf);



  if ( (buf_size == 0) || (buf == NULL) ) return 1;
  else
  {
    // now add tag into structure
    TDimTagItem item;

    item.tagGroup  = DIM_META_TIFF_TAG;
    item.tagId     = tag;
    item.tagType   = buf_type;
    item.tagLength = buf_size / tag_size_bytes[buf_type];
    item.tagData   = buf;

    addMetaTag( &fmtHndl->metaData, item);
  }

  return 0;
}

DIM_UINT read_tiff_metadata (TDimFormatHandle *fmtHndl, DTiffParams *tifParams, int group, int tag, int type)
{
  if (!areValidParams(fmtHndl, tifParams)) return 1;

  if (group == DIM_META_BIORAD) return 1;
  
  if (fmtHndl->pageNumber >= tifParams->ifds.count) fmtHndl->pageNumber = 0;

  // first read custom formatted tags
  if (tifParams->subType == tstStk)
    stkReadMetaMeta (fmtHndl, group, tag, type);

  if (tag != -1)
    return read_one_tag ( fmtHndl, tifParams, tag );

  if ( (group == -1) || (group == DIM_META_TIFF_TAG) )
  {
    DIM_UINT i;
    TDimTiffIFD *ifd = &tifParams->ifds.ifds[fmtHndl->pageNumber];

    for (i=0; i<ifd->count; i++ )
    {
      if (type == -1)
      {
        if ( (ifd->entries[i].tag > 532) &&
             (ifd->entries[i].tag != 50434)

           ) read_one_tag ( fmtHndl, tifParams, ifd->entries[i].tag );    
    
        switch( ifd->entries[i].tag ) 
        {
          case 269: //DocumentName
          case 270: //ImageDescription
          case 271: //Make
          case 272: //Model
          case 285: //PageName
          case 305: //Software
          case 306: //DateTime
          case 315: //Artist
          case 316: //HostComputer
            read_one_tag ( fmtHndl, tifParams, ifd->entries[i].tag );
            break;
        } // switch
      } // type == -1
      else
      {
        if (ifd->entries[i].type == type)
          read_one_tag ( fmtHndl, tifParams, ifd->entries[i].tag );

      } // type != -1

    } // for i<ifd count
  } // if no group

  return 0;
}

//----------------------------------------------------------------------------
// Textual METADATA
//----------------------------------------------------------------------------

void change_0_to_n (char *str, long size) {
  for (long i=0; i<size; i++)
    if (str[i] == '\0') str[i] = '\n'; 
}

void write_title_text(const char *text, MemIOBuf *outIOBuf)
{
  char title[1024];
  sprintf(title, "\n[%s]\n\n", text);
  MemIO_WriteProc( (thandle_t) outIOBuf, title, strlen(title)-1 );
}

void read_text_tag(TIFF *tif, TDimTiffIFD *ifd, DIM_UINT tag, MemIOBuf *outIOBuf, const char *text)
{
  uint32 buf_size;
  uint32 buf_type;  
  DIM_UCHAR *buf = NULL;
  
  if (isTagPresentInIFD ( ifd, tag ) == TRUE ) {
    write_title_text(text, outIOBuf);
    readTiffTag (tif, ifd, tag, buf_size, buf_type, &buf);
    change_0_to_n ((char *) buf, buf_size);
    MemIO_WriteProc( (thandle_t) outIOBuf, buf, buf_size );
    freeTiffTagBuf( (DIM_UCHAR **) &buf );
  }
}

void read_text_tag(TIFF *tif, TDimTiffIFD *ifd, DIM_UINT tag, MemIOBuf *outIOBuf)
{
  uint32 buf_size;
  uint32 buf_type;  
  DIM_UCHAR *buf = NULL;
  
  if (isTagPresentInIFD ( ifd, tag ) == TRUE ) {
    readTiffTag (tif, ifd, tag, buf_size, buf_type, &buf);
    change_0_to_n ((char *) buf, buf_size);
    MemIO_WriteProc( (thandle_t) outIOBuf, buf, buf_size );
    freeTiffTagBuf( (DIM_UCHAR **) &buf );
  }
}

char* read_text_tiff_metadata ( TDimFormatHandle *fmtHndl, DTiffParams *tifParams ) {
  return NULL;
}

//----------------------------------------------------------------------------
// New METADATA
//----------------------------------------------------------------------------

DIM_UINT append_metadata_generic_tiff (TDimFormatHandle *fmtHndl, DTagMap *hash ) {
  if (fmtHndl == NULL) return 1;
  if (fmtHndl->internalParams == NULL) return 1;
  if (!hash) return 1;

  DTiffParams *tifParams = (DTiffParams *) fmtHndl->internalParams;
  TIFF *tif = tifParams->dimTiff;
  TDimTiffIFD *ifd = &tifParams->ifds.ifds[0];

  std::map< int, std::string > hash_tiff_tags;
  hash_tiff_tags[269] = "Document Name";
  hash_tiff_tags[270] = "Image Description";
  hash_tiff_tags[285] = "Page Name";
  hash_tiff_tags[271] = "Make";
  hash_tiff_tags[272] = "Model";
  hash_tiff_tags[305] = "Software";
  hash_tiff_tags[306] = "Date Time";
  hash_tiff_tags[315] = "Artist";
  hash_tiff_tags[316] = "Host Computer";

  std::map< int, std::string >::const_iterator it = hash_tiff_tags.begin();
  while (it != hash_tiff_tags.end()) {
    xstring tag_str = read_tag_as_string(tif, ifd, it->first);
    if (tag_str.size()>0) hash->append_tag( xstring("custom/") + it->second, tag_str );
    it++;
  }
  return 0;
}

std::string read_tag_as_string(TIFF *tif, TDimTiffIFD *ifd, DIM_UINT tag) {
  uint32 buf_size, buf_type;  
  DIM_UCHAR *buf = NULL;
  std::string s;

  if (isTagPresentInIFD ( ifd, tag ) == TRUE ) {
    readTiffTag (tif, ifd, tag, buf_size, buf_type, &buf);
    s.resize( buf_size );
    memcpy( &s[0], buf, buf_size );
    freeTiffTagBuf( (DIM_UCHAR **) &buf );
  }
  return s;
}

DIM_UINT append_metadata_qimaging_tiff (TDimFormatHandle *fmtHndl, DTagMap *hash ) {
  if (fmtHndl == NULL) return 1;
  if (fmtHndl->internalParams == NULL) return 1;
  if (!hash) return 1;

  DTiffParams *tifParams = (DTiffParams *) fmtHndl->internalParams;
  TIFF *tif = tifParams->dimTiff;
  TDimTiffIFD *ifd = &tifParams->ifds.ifds[0];

  /*
[Image Description]
Exposure: 000 : 00 : 00 . 300 : 000
Binning: 2 x 2
Gain: 2.000000
%Accumulated%=0

[Software]
QCapture Pro

[Date Time]
08/28/2006 04:34:47.000 PM
  */

  // check if it's QImage tiff file
  // should exist private tags 50288 and 50296
  if (!isTagPresentInIFD(ifd, 50288)) return 0;
  if (!isTagPresentInIFD(ifd, 50296)) return 0;

  // tag 305 should be "QCapture Pro"
  xstring tag_software = read_tag_as_string(tif, ifd, 305);
  if ( tag_software != "QCapture Pro" ) return 0;
  
  // ok, we're sure it's QImaging
  hash->append_tag( "custom/Software", tag_software );


  xstring tag_description = read_tag_as_string(tif, ifd, TIFFTAG_IMAGEDESCRIPTION);
  if (tag_description.size()>0)
    hash->parse_ini( tag_description, ":", "custom" );

  // read tag 306 - Date/Time
  xstring tag_datetime = read_tag_as_string(tif, ifd, 306);
  if (tag_datetime.size()>0) {
    int y=0, m=0, d=0, h=0, mi=0, s=0, ms=0;
    char ampm=0;
    //08/28/2006 04:34:47.000 PM
    sscanf( (char *)tag_datetime.c_str(), "%d/%d/%d %d:%d:%d.%d %c", &m, &d, &y, &h, &mi, &s, &ms, &ampm );
    if (ampm == 'P') h += 12;
    tag_datetime.sprintf("%.4d-%.2d-%.2d %.2d:%.2d:%.2d", y, m, d, h, mi, s);
    hash->append_tag( "date_time", tag_datetime );
  }

  //hash->append_tag( "pixel_resolution_x", par->pixel_size[0] );
  //hash->append_tag( "pixel_resolution_y", par->pixel_size[1] );
  //hash->append_tag( "pixel_resolution_z", par->pixel_size[2] );

  return 0;
}

DIM_UINT tiff_append_metadata (TDimFormatHandle *fmtHndl, DTagMap *hash ) {
  if (fmtHndl == NULL) return 1;
  if (fmtHndl->internalParams == NULL) return 1;
  if (!hash) return 1;
  DTiffParams *tifParams = (DTiffParams *) fmtHndl->internalParams;

  append_metadata_qimaging_tiff (fmtHndl, hash );

  if (tifParams->subType == tstStk) 
    append_metadata_stk(fmtHndl, hash);
  else
  if (tifParams->subType == tstPsia) 
    append_metadata_psia(fmtHndl, hash);
  else
  if (tifParams->subType == tstFluoview)
    append_metadata_fluoview(fmtHndl, hash);
  else
  if (tifParams->subType == tstCzLsm)
    append_metadata_lsm(fmtHndl, hash);
  else
  if (tifParams->subType == tstOmeTiff)
    append_metadata_omeTiff (fmtHndl, hash);
  else
    append_metadata_generic_tiff(fmtHndl, hash);

  return 0;
}

//----------------------------------------------------------------------------
// Write METADATA
//----------------------------------------------------------------------------


DIM_UINT write_tiff_metadata (TDimFormatHandle *fmtHndl, DTiffParams *tifParams)
{
  if (!areValidParams(fmtHndl, tifParams)) return 1;

  DIM_UINT i;
  TDimTagList *tagList = &fmtHndl->metaData;
  void  *t_list = NULL;
  int16 t_list_count;
  TIFF *tif = tifParams->dimTiff;

  if (tagList->count == 0) return 1;
  if (tagList->tags == NULL) return 1;

  for (i=0; i<tagList->count; i++) {
    TDimTagItem *tagItem = &tagList->tags[i];
    if (tagItem->tagGroup == DIM_META_TIFF_TAG) {
      t_list = tagItem->tagData;
      t_list_count = tagItem->tagLength;

      TIFFSetField( tif, tagItem->tagId, tagItem->tagLength, tagItem->tagData ); 
    }
  }

  return 0;
}


//****************************************************************************
// WRITING LINE SEGMENT FROM BUFFER
//****************************************************************************

template< typename T >
void write_line_segment_t(void *po, void *bufo, TDimImageBitmap *img, DIM_UINT sample, DIM_ULONG w) {
  T *p   = (T *) po;
  T *buf = (T *) bufo;  
  DIM_UINT nsamples = img->i.samples;
  register DIM_UINT x, xi=0;
  for (x=sample; x<w*nsamples; x+=nsamples) {
    p[xi] = buf[x];
    xi++;
  }
}

void write_line_segment(void *po, void *bufo, TDimImageBitmap *img, DIM_UINT sample, DIM_ULONG w) {
  if (img->i.depth==8)  write_line_segment_t<DIM_UINT8>  (po, bufo, img, sample, w);
  else
  if (img->i.depth==16) write_line_segment_t<DIM_UINT16> (po, bufo, img, sample, w);
  else
  if (img->i.depth==32) write_line_segment_t<DIM_UINT32> (po, bufo, img, sample, w);
  else
  if (img->i.depth==64) write_line_segment_t<DIM_FLOAT64>(po, bufo, img, sample, w);
}


//****************************************************************************
// SCANLINE METHOD TIFF
//****************************************************************************

int read_scanline_tiff(TIFF *tif, TDimImageBitmap *img, TDimFormatHandle *fmtHndl)
{
  int result = -1;
  register DIM_UINT y = 0;
  uint16 planarConfig;

  if (tif == NULL) return result;
  if (img == NULL) return result;
  
  DIM_UINT sample;
  DIM_UINT lineSize = getLineSizeInBytes( img );

  TIFFGetField(tif, TIFFTAG_PLANARCONFIG, &planarConfig);

  if ( (planarConfig == PLANARCONFIG_SEPARATE) || (img->i.samples == 1) ) {
    for (sample=0; sample<img->i.samples; sample++) {
      DIM_UCHAR *p = (DIM_UCHAR *) img->bits[sample];

      for(y=0; y<img->i.height; y++) {

        dimProgress( fmtHndl, y*(sample+1), img->i.height*img->i.samples, "Reading TIFF" );
        if ( dimTestAbort( fmtHndl ) == 1) break;  

        result = TIFFReadScanline(tif, p, y, sample);
        p += lineSize;
      } // for y

    }  // for sample

  } // if planar
  else // if image contain several samples in one same plane ex: RGBRGBRGB...
  {
    DIM_UCHAR *buf = (DIM_UCHAR *) _TIFFmalloc( TIFFScanlineSize ( tif ) );
    for(y=0; y<img->i.height; y++) {

      dimProgress( fmtHndl, y, img->i.height, "Reading TIFF" );
      if ( dimTestAbort( fmtHndl ) == 1) break;  

      TIFFReadScanline(tif, buf, y, 0);

      for (sample=0; sample<img->i.samples; ++sample) {
        DIM_UCHAR *p = (DIM_UCHAR *) img->bits[sample] + (lineSize * y);
        write_line_segment(p, buf, img, sample, img->i.width);
      }  // for sample

    } // for y
    _TIFFfree( buf );
  }  

  return result;
}

//****************************************************************************
// TILED METHOD TIFF
//****************************************************************************

int read_tiled_tiff(TIFF *tif, TDimImageBitmap *img, TDimFormatHandle *fmtHndl) {
  uint16 planarConfig;
  if (tif == NULL) return 1;
  if (img == NULL) return 1;

  // if tiff is not tiled get out and never come back :-)
  if( !TIFFIsTiled(tif) ) return 1;
  
  uint32 columns, rows;
  DIM_UCHAR *tile_buf;
  register DIM_UINT x, y;

  DIM_UINT sample;
  DIM_UINT lineSize = getLineSizeInBytes( img );

  TIFFGetField(tif, TIFFTAG_PLANARCONFIG, &planarConfig);
  TIFFGetField(tif, TIFFTAG_TILEWIDTH,  &columns);
  TIFFGetField(tif, TIFFTAG_TILELENGTH, &rows);


  tile_buf = (DIM_UCHAR*) _TIFFmalloc( TIFFTileSize(tif) );
  if (tile_buf == NULL) return 1;
  uint32 tileW = columns, tileH = rows;


  for (y=0; y<img->i.height; y+=rows) {
    if (y > img->i.height) break;

    // the tile height may vary 
    if (img->i.height-y < rows) tileH = img->i.height-y; 

    dimProgress( fmtHndl, y, img->i.height, "Reading TIFF" );
    if ( dimTestAbort( fmtHndl ) == 1) break;  

    tileW = columns;
    for (x=0; x<(DIM_UINT)img->i.width; x+=columns) {
      register uint32 yi;
      uint32 tW;
  
      // the tile size is now treated by libtiff guys the
      // way that the size stay on unchanged      
      if (img->i.width-x < columns) tW = img->i.width-x; else tW = tileW;


      if ( (planarConfig == PLANARCONFIG_SEPARATE) || (img->i.samples == 1) ) {
        for (sample=0; sample<img->i.samples; sample++) {
          if (!TIFFReadTile(tif, tile_buf, x, y, 0, sample)) break;
 
          // now put tile into the image 
          for(yi = 0; yi < tileH; yi++) {
            DIM_UCHAR *p = (DIM_UCHAR *) img->bits[sample] + (lineSize * (y+yi));
            _TIFFmemcpy(p+x, tile_buf+yi*tileW, tW);
          }
        }  // for sample

      } // if planar
      else { // if image contain several samples in one same plane ex: RGBRGBRGB...
        if (!TIFFReadTile(tif, tile_buf, x, y, 0, 0)) break;
        for (sample=0; sample<img->i.samples; sample++) {
          // now put tile into the image 
          for(yi = 0; yi < tileH; yi++) {
            DIM_UCHAR *p = (DIM_UCHAR *) img->bits[sample] + (lineSize * (y+yi));
            write_line_segment(p+x, tile_buf+(yi*tileW*img->i.samples), img, sample, tW);
          }
        }  // for sample
      } // if not separate planes

    } // for x
  } // for y

  _TIFFfree(tile_buf);

  return 0;
}



//****************************************************************************
//*** TIFF READER
//****************************************************************************

// if the file is LSM then the strip size given in the file is incorrect, fix that
// by simply checking against the file size and adjusting if needed
void lsmFixStripByteCounts ( TIFF *tif, uint32 row, tsample_t sample ) {

  TIFFDirectory *td = &tif->tif_dir;
  D_TIFF_STRP_TYPE strip = sample*td->td_stripsperimage + row/td->td_rowsperstrip;
  D_TIFF_BCNT_TYPE bytecount = td->td_stripbytecount[strip];
  if (tif->tif_size <= 0) tif->tif_size = TIFFGetFileSize(tif);

  if ( td->td_stripoffset[strip] + bytecount > tif->tif_size) {
    bytecount = tif->tif_size - td->td_stripoffset[strip];
    td->td_stripbytecount[strip] = bytecount;
  }
}

int read_tiff_image(TDimFormatHandle *fmtHndl, DTiffParams *tifParams) {
  if (!areValidParams(fmtHndl, tifParams)) return 1;

  TIFF *tif = tifParams->dimTiff;
  TDimImageBitmap *img = fmtHndl->image;

  uint32 height = 0; 
  uint32 width = 0; 
  uint16 bitspersample = 1;
  uint16 samplesperpixel = 1;
  uint32 rowsperstrip;  
  uint16 photometric = PHOTOMETRIC_MINISWHITE;
  uint16 compression = COMPRESSION_NONE;
  uint16 PlanarConfig;
  unsigned int currentDir = 0;

  currentDir = TIFFCurrentDirectory(tif);
  int needed_page_num = fmtHndl->pageNumber;
  if (tifParams->subType == tstCzLsm)
    needed_page_num = fmtHndl->pageNumber*2;

  // now must read correct page and set image parameters
  if (currentDir != needed_page_num)
  if (tifParams->subType != tstStk) {
    TIFFSetDirectory(tif, needed_page_num);

    currentDir = TIFFCurrentDirectory(tif);
    if (currentDir != needed_page_num) return 1;

    getCurrentPageInfo( tifParams );
  }
  
  if (tifParams->subType != tstOmeTiff) img->i = tifParams->info;

  TIFFGetField(tif, TIFFTAG_COMPRESSION, &compression);
  TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
  TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);
  TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &samplesperpixel);
  TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &bitspersample);
  TIFFGetField(tif, TIFFTAG_ROWSPERSTRIP, &rowsperstrip);   
  TIFFGetField(tif, TIFFTAG_PHOTOMETRIC, &photometric);
  TIFFGetField(tif, TIFFTAG_PLANARCONFIG, &PlanarConfig);	// single image plane 

  // this is here due to some OME-TIFF do not conform with the standard and come with all channels in the same IFD
  if (tifParams->subType == tstOmeTiff) {
    int r = omeTiffReadPlane( fmtHndl, tifParams, fmtHndl->pageNumber );
    if (r != 2) return r;
    img->i = tifParams->info;
  }

  // if image is PSIA then read and init it here
  if (tifParams->subType == tstPsia)
    return psiaReadPlane(fmtHndl, tifParams, fmtHndl->pageNumber, img);

  // if image is Fluoview and contains 1..4 channels
  if ( (tifParams->subType == tstFluoview) && (tifParams->fluoviewInfo.ch > 1) )
    return fluoviewReadPlane( fmtHndl, tifParams, fmtHndl->pageNumber );

  // if the file is LSM then the strip size given in the file is incorrect, fix that
  if (tifParams->subType == tstCzLsm)
    for (unsigned int sample=0; sample < samplesperpixel; ++sample)
      for (unsigned int y=0; y < height; ++y)
        lsmFixStripByteCounts( tif, y, sample );

  if ( allocImg( fmtHndl, &img->i, img) != 0 ) return 1;

  // if image is STK
  if (tifParams->subType == tstStk)
    return stkReadPlane(tifParams, fmtHndl->pageNumber, img, fmtHndl);


  if( !TIFFIsTiled(tif) )
    read_scanline_tiff(tif, img, fmtHndl);
  else
    read_tiled_tiff(tif, img, fmtHndl);

  // invert each pixel if PHOTOMETRIC_MINISWHITE
  if (photometric == PHOTOMETRIC_MINISWHITE)
    invertImg( img );


  return 0;
}

//****************************************************************************
// TIFF WRITER
//****************************************************************************

int write_tiff_image(TDimFormatHandle *fmtHndl, DTiffParams *tifParams) {
  if (!areValidParams(fmtHndl, tifParams)) return 1;

  if (tifParams->subType == tstOmeTiff)
    return omeTiffWritePlane( fmtHndl, tifParams);

  TIFF *out = tifParams->dimTiff;
  TDimImageBitmap *img = fmtHndl->image;
 
  uint32 height;
  uint32 width;
  uint32 rowsperstrip = (uint32) -1;
  uint16 bitspersample;
  uint16 samplesperpixel;
  uint16 photometric = PHOTOMETRIC_MINISBLACK;
  uint16 compression;
  uint16 planarConfig;

  width = img->i.width;
  height = img->i.height;
  bitspersample = img->i.depth;
  samplesperpixel = img->i.samples;
  if (img->i.imageMode == DIM_RGB)   photometric = PHOTOMETRIC_RGB;
  if (img->i.imageMode == DIM_MULTI) photometric = PHOTOMETRIC_RGB;
  if (samplesperpixel >= 2)          photometric = PHOTOMETRIC_RGB;
  if ( (img->i.imageMode == DIM_INDEXED) && (img->i.lut.count > 0) && (samplesperpixel==1) && (bitspersample<=8) )
    photometric = PHOTOMETRIC_PALETTE;

  if ( (bitspersample == 1) && (samplesperpixel == 1) ) photometric = PHOTOMETRIC_MINISWHITE;
  // a failed attempt to force photoshop to load 2 channel image
  //if (samplesperpixel == 2) photometric = PHOTOMETRIC_SEPARATED;

  // handle standard width/height/bpp stuff
  TIFFSetField(out, TIFFTAG_IMAGEWIDTH, width);
  TIFFSetField(out, TIFFTAG_IMAGELENGTH, height);
  TIFFSetField(out, TIFFTAG_SAMPLESPERPIXEL, samplesperpixel);
  TIFFSetField(out, TIFFTAG_BITSPERSAMPLE, bitspersample);
  TIFFSetField(out, TIFFTAG_PHOTOMETRIC, photometric);
  TIFFSetField(out, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);

 
  // set pixel format
  uint16 sampleformat = SAMPLEFORMAT_UINT;
  if (img->i.pixelType == D_FMT_SIGNED) sampleformat = SAMPLEFORMAT_INT;
  if (img->i.pixelType == D_FMT_FLOAT)  sampleformat = SAMPLEFORMAT_IEEEFP;
  TIFFSetField(out, TIFFTAG_SAMPLEFORMAT, sampleformat);


  // set planar config
  planarConfig = PLANARCONFIG_SEPARATE;	// separated planes 
  if (samplesperpixel==3 && bitspersample==8)
    planarConfig = PLANARCONFIG_CONTIG;

  /*
  if (img->i.imageMode == DIM_MULTI)
    planarConfig = PLANARCONFIG_SEPARATE;	// separated planes 
  else
    planarConfig = PLANARCONFIG_CONTIG;	// mixed planes

  // now more tests for plane configuration
  if (samplesperpixel > 3) planarConfig = PLANARCONFIG_SEPARATE;
  if ( (samplesperpixel == 1) || (samplesperpixel == 3) ) 
    planarConfig = PLANARCONFIG_CONTIG;
  */
 
  TIFFSetField(out, TIFFTAG_PLANARCONFIG, planarConfig);	// separated planes


  TIFFSetField(out, TIFFTAG_SOFTWARE, "DIMIN TIFF WRAPPER <www.dimin.net>");

  //if( TIFFGetField( out, TIFFTAG_DOCUMENTNAME, &pszText ) )
  //if( TIFFGetField( out, TIFFTAG_IMAGEDESCRIPTION, &pszText ) )
  //if( TIFFGetField( out, TIFFTAG_DATETIME, &pszText ) )



  //------------------------------------------------------------------------------  
  // compression
  //------------------------------------------------------------------------------  

  compression = fmtHndl->compression;
  if (compression == 0) compression = COMPRESSION_NONE; 

  switch(bitspersample) {
  case 1  :
    if (compression != COMPRESSION_CCITTFAX4) compression = COMPRESSION_NONE;
    break;

  case 8  :
  case 16 :
  case 32 :
  case 64 :
    if ( (compression != COMPRESSION_LZW) && (compression != COMPRESSION_PACKBITS) )
      compression = COMPRESSION_NONE;  
    break;
  
  default :
    compression = COMPRESSION_NONE;
    break;
  }

  TIFFSetField(out, TIFFTAG_COMPRESSION, compression);

  unsigned long strip_size = (unsigned long) MAX( TIFFDefaultStripSize(out,-1), 1 );
  switch ( compression ) {
    case COMPRESSION_JPEG:
    {
      TIFFSetField( out, TIFFTAG_ROWSPERSTRIP, strip_size+(16-(strip_size % 16)) );
      break;
    }

    case COMPRESSION_ADOBE_DEFLATE:
    {
      TIFFSetField( out, TIFFTAG_ROWSPERSTRIP, height );
      if ( (photometric == PHOTOMETRIC_RGB) ||
           ((photometric == PHOTOMETRIC_MINISBLACK) && (bitspersample >= 8)) )
        TIFFSetField( out, TIFFTAG_PREDICTOR, 2 );
      TIFFSetField( out, TIFFTAG_ZIPQUALITY, 9 );
      break;
    }

    case COMPRESSION_CCITTFAX4:
    {
      TIFFSetField( out, TIFFTAG_ROWSPERSTRIP, height );
      break;
    }

    case COMPRESSION_LZW:
    {
      TIFFSetField( out, TIFFTAG_ROWSPERSTRIP, strip_size );
      if (planarConfig == PLANARCONFIG_SEPARATE)
         TIFFSetField( out, TIFFTAG_PREDICTOR, PREDICTOR_NONE );
      else
         TIFFSetField( out, TIFFTAG_PREDICTOR, PREDICTOR_HORIZONTAL );
      break;
    }
    default:
    {
      TIFFSetField( out, TIFFTAG_ROWSPERSTRIP, strip_size );
      break;
    }
  }

  //------------------------------------------------------------------------------  
  // Save resolution
  //------------------------------------------------------------------------------

  {
    double rx = img->i.xRes, ry = img->i.yRes;  
    uint16 units = img->i.resUnits;
    
    if ( (img->i.xRes == 0) && (img->i.yRes == 0) || (img->i.resUnits == 1) ) {
      // Standard resolution some claim to be 72ppi... why not?
      units = RESUNIT_INCH;
      rx = 72.0; 
      ry = 72.0;
    }
    else
    if (img->i.resUnits != 2) {
      if (img->i.resUnits == 0)  { rx = pow(rx, -2); ry = pow(ry, -2); }
      if (img->i.resUnits == 4)  { rx = pow(rx, -1); ry = pow(ry, -1); }
      if (img->i.resUnits == 5)  { rx = pow(rx, -4); ry = pow(ry, -4); }
      if (img->i.resUnits == 6)  { rx = pow(rx, -7); ry = pow(ry, -7); }
      if (img->i.resUnits == 7)  { rx = pow(rx, 11); ry = pow(ry, 11); }
      if (img->i.resUnits == 8)  { rx = pow(rx, 8); ry = pow(ry, 8); }
      if (img->i.resUnits == 9)  { rx = pow(rx, 5); ry = pow(ry, 5); }
      if (img->i.resUnits == 10) { rx = pow(rx, 0); ry = pow(ry, 0); }
    }

    TIFFSetField(out, TIFFTAG_RESOLUTIONUNIT, units);
    TIFFSetField(out, TIFFTAG_XRESOLUTION, rx);
    TIFFSetField(out, TIFFTAG_YRESOLUTION, ry);
  }
 
  //------------------------------------------------------------------------------  
  // palettes (image colormaps are automatically scaled to 16-bits)
  //------------------------------------------------------------------------------ 
  uint16 palr[256], palg[256], palb[256];
  if ( (photometric == PHOTOMETRIC_PALETTE) && (img->i.lut.count > 0) ) {
    uint16 nColors = img->i.lut.count;
    for (int i=0; i<nColors; i++) {
      palr[i] = (uint16) dimR( img->i.lut.rgba[i] ) * 256;
      palg[i] = (uint16) dimG( img->i.lut.rgba[i] ) * 256;
      palb[i] = (uint16) dimB( img->i.lut.rgba[i] ) * 256;
    }
    TIFFSetField(out, TIFFTAG_COLORMAP, palr, palg, palb);
  }


  //------------------------------------------------------------------------------
  // writing meta data
  //------------------------------------------------------------------------------

  write_tiff_metadata (fmtHndl, tifParams);
  //TIFFFlush(out); // error in doing this, due to additional checks to the libtiff 4.0.0


  //------------------------------------------------------------------------------
  // writing image
  //------------------------------------------------------------------------------

  // if separate palnes or only one sample
  if ( (planarConfig == PLANARCONFIG_SEPARATE) || (samplesperpixel == 1) ) {
    DIM_UINT sample;
    DIM_UINT line_size = getLineSizeInBytes( img );

    for (sample=0; sample<img->i.samples; sample++) {
      DIM_UCHAR *bits = (DIM_UCHAR *) img->bits[sample];
      register uint32 y;

      for (y = 0; y <height; y++) {
        dimProgress( fmtHndl, y*(sample+1), height*img->i.samples, "Writing TIFF" );
        if ( dimTestAbort( fmtHndl ) == 1) break;  

        TIFFWriteScanline(out, bits, y, sample);
        bits += line_size;
      } // for y
    } // for samples

  } // if separate planes
  else
  { // if RGB image
    DIM_UINT Bpp = (unsigned int) ceil( ((double) bitspersample) / 8.0 );
    DIM_UCHAR *buffer = (DIM_UCHAR *) _TIFFmalloc(width * 3 * Bpp);
    register DIM_UINT x, y;
    DIM_UCHAR *black_line = (DIM_UCHAR *) _TIFFmalloc(width * Bpp);
    memset( black_line, 0, width * Bpp );
    
    for (y = 0; y < height; y++) {
      dimProgress( fmtHndl, y, height, "Writing TIFF" );
      if ( dimTestAbort( fmtHndl ) == 1) break;  

      DIM_UCHAR *bufIn0 = ((DIM_UCHAR *) img->bits[0]) + y*width*Bpp;
      DIM_UCHAR *bufIn1 = ((DIM_UCHAR *) img->bits[1]) + y*width*Bpp;
      DIM_UCHAR *bufIn2 = NULL;
      if (samplesperpixel > 2)
        bufIn2 = ((DIM_UCHAR *) img->bits[2]) + y*width*Bpp;
      else
        bufIn2 = black_line;

      if (img->i.depth <= 8) { // 8 bits
        DIM_UCHAR *p  = (DIM_UCHAR *) buffer;
        
        for (x=0; x<width; x++) {
          p[0] = *(bufIn0 + x);
          p[1] = *(bufIn1 + x);
          p[2] = *(bufIn2 + x);
          p += 3;
        }

      } // if 8 bit
      else  { // 16 bits
        uint16 *p  = (uint16 *) buffer;
        uint16 *p0 = (uint16 *) bufIn0;   
        uint16 *p1 = (uint16 *) bufIn1; 
        uint16 *p2 = (uint16 *) bufIn2;
      
        for (x=0; x<width; x++) {
          p[0] = *(p0 + x);
          p[1] = *(p1 + x);
          p[2] = *(p2 + x);
          p += 3;
        }
      } // if 16 bit

      // write the scanline to disc
      TIFFWriteScanline(out, buffer, y, 0);
    }

    _TIFFfree(buffer);
    _TIFFfree(black_line);
  }

  TIFFWriteDirectory( out );
  TIFFFlushData(out);
  TIFFFlush(out);

  return 0;
}

#ifdef MAX 
#undef MAX
#endif

#ifdef MIN 
#undef MIN
#endif

