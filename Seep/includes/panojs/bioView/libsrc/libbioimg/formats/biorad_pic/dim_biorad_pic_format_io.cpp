/*****************************************************************************
  BIORAD PIC IO
  UCSB/BioITR property
  Copyright (c) 2004 by Dmitry V. Fedorov <www.dimin.net> <dima@dimin.net>

  IMPLEMENTATION
  
  Programmer: Dima V. Fedorov <mailto:dima@dimin.net> <http://www.dimin.net/>
    

  History:
    03/29/2004 22:23 - First creation
    05/10/2004 14:55 - Big endian support
    08/04/2004 22:25 - Update to FMT_IFS 1.2, support for io protorypes
        
  Ver : 3
*****************************************************************************/


//#include <cstring>
#include <string>

#include <xstring.h>
#include <tag_map.h>

// Disables Visual Studio 2005 warnings for deprecated code
#if ( defined(_MSC_VER) && (_MSC_VER >= 1400) )
  #pragma warning(disable:4996)
#endif 

#include "dim_biorad_pic_format.h"

//----------------------------------------------------------------------------
// READ PROC
//----------------------------------------------------------------------------

static int read_biorad_image(TDimFormatHandle *fmtHndl)
{
  if (fmtHndl == NULL) return 1;
  if (fmtHndl->internalParams == NULL) return 1;
  DBioRadPicParams *picPar = (DBioRadPicParams *) fmtHndl->internalParams;
  TDimImageInfo *info = &picPar->i;  
  TDimImageBitmap *img = fmtHndl->image;

  // init the image
  if ( allocImg( fmtHndl, info, img) != 0 ) return 1;
  long offset = picPar->data_offset + fmtHndl->pageNumber * picPar->page_size_bytes;

  if (fmtHndl->stream == NULL) return 1;
  if ( dimSeek(fmtHndl, offset, SEEK_SET) != 0) return 1;
  if (dimRead( fmtHndl, img->bits[0], picPar->page_size_bytes, 1 ) != 1) return 1;
  if ( (img->i.depth == 16) && (dimBigendian) ) 
    dimSwapArrayOfShort((DIM_UINT16*) img->bits[0], picPar->page_size_bytes/2);
  
  dimProgress( fmtHndl, 0, 10, "Reading BIORAD" );

  if (info->imageMode == DIM_RGB)
  { // rgb mode, three pages are considered RGB channels

    offset += picPar->page_size_bytes;
    if (dimSeek( fmtHndl, offset, SEEK_SET) != 0) return 1;
    if (dimRead( fmtHndl, img->bits[1], picPar->page_size_bytes, 1 ) != 1) return 1;

    if ( (img->i.depth == 16) && (dimBigendian) ) 
      dimSwapArrayOfShort((DIM_UINT16*) img->bits[1], picPar->page_size_bytes/2);

    if (picPar->num_images > 2)
    {
      offset += picPar->page_size_bytes;
      if (dimSeek( fmtHndl, offset, SEEK_SET) != 0) return 1;
      if (dimRead( fmtHndl, img->bits[2], picPar->page_size_bytes, 1 ) != 1) return 1;

      if ( (img->i.depth == 16) && (dimBigendian) ) 
        dimSwapArrayOfShort((DIM_UINT16*) img->bits[2], picPar->page_size_bytes/2);
    }
    else
    {
      memset( img->bits[2], 0, picPar->page_size_bytes );
    }
  }

  return 0;
}


//----------------------------------------------------------------------------
// META DATA PROC
//----------------------------------------------------------------------------

DIM_UINT add_one_tag (TDimFormatHandle *fmtHndl, int tag, const char* str)
{
  DIM_UCHAR *buf = NULL;
  DIM_UINT32 buf_size = strlen(str);
  DIM_UINT32 buf_type = DIM_TAG_ASCII;

  if ( (buf_size == 0) || (str == NULL) ) return 1;
  else
  {
    // now add tag into structure
    TDimTagItem item;

    buf = (unsigned char *) dimMalloc(fmtHndl, buf_size + 1);
    strncpy((char *) buf, str, buf_size);
    buf[buf_size] = '\0';

    item.tagGroup  = DIM_META_BIORAD;
    item.tagId     = tag;
    item.tagType   = buf_type;
    item.tagLength = buf_size;
    item.tagData   = buf;

    addMetaTag( &fmtHndl->metaData, item);
  }

  return 0;
}

DIM_UINT read_biorad_metadata (TDimFormatHandle *fmtHndl, int group, int , int )
{
  if (fmtHndl == NULL) return 1;
  if (fmtHndl->internalParams == NULL) return 1;
  DBioRadPicParams *par = (DBioRadPicParams *) fmtHndl->internalParams;
  
  if ( (group != DIM_META_BIORAD) && (group != -1) ) return 1;

  if (par->note01.size() > 0) 
    add_one_tag ( fmtHndl, 01, par->note01.c_str() );
  if (par->note20.size() > 0) 
    add_one_tag ( fmtHndl, 20, par->note20.c_str() );
  if (par->note21.size() > 0) 
    add_one_tag ( fmtHndl, 21, par->note21.c_str() );

  return 0;
}


//----------------------------------------------------------------------------
// Metadata hash
//----------------------------------------------------------------------------

DIM_UINT biorad_append_metadata (TDimFormatHandle *fmtHndl, DTagMap *hash ) {
  if (fmtHndl == NULL) return 1;
  if (fmtHndl->internalParams == NULL) return 1;
  if (!hash) return 1;
  DBioRadPicParams *par = (DBioRadPicParams *) fmtHndl->internalParams; 

  hash->append_tag( "pixel_resolution_x", par->pixel_size[0] );
  hash->append_tag( "pixel_resolution_y", par->pixel_size[1] );
  hash->append_tag( "pixel_resolution_z", par->pixel_size[2] );

  hash->append_tag( "pixel_resolution_unit_x", "microns" );
  hash->append_tag( "pixel_resolution_unit_y", "microns" );
  hash->append_tag( "pixel_resolution_unit_z", "microns" );

  //date
  if (par->datetime.size()>0)
    hash->append_tag( "date_time", par->datetime );

  if (par->note01.size() > 0)
    hash->append_tag( "raw/Note01", par->note01 );

  if (par->note20.size() > 0)
    hash->append_tag( "raw/Note20", par->note20 );

  if (par->note21.size() > 0)
    hash->append_tag( "raw/Note21", par->note21 );

  // Parse some of the notes
  hash->parse_ini( par->note20, "=", "custom" );

  return 0;
}
