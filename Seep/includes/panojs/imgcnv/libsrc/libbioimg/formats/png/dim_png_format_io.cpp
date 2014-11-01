/*****************************************************************************
  PNG IO 
  Copyright (c) 2004 by Dmitry V. Fedorov <www.dimin.net> <dima@dimin.net>

  IMPLEMENTATION
  
  Programmer: Dima V. Fedorov <mailto:dima@dimin.net> <http://www.dimin.net/>

  History:
    07/29/2004 16:31 - First creation
    08/04/2004 22:25 - Update to FMT_IFS 1.2, support for io protorypes
    02/22/2007 15:28 - fixed bug reading metadata
        
  Ver : 3
*****************************************************************************/

#include <string>

// Disables Visual Studio 2005 warnings for deprecated code
#ifdef WIN32
   #pragma warning(disable:4996)
#endif

#include "dim_png_format.h"

//****************************************************************************
// READ PROC
//****************************************************************************

static int read_png_image(TDimFormatHandle *fmtHndl)
{
  if (fmtHndl == NULL) return 1;
  if (fmtHndl->internalParams == NULL) return 1;
  TDimPngParams *pngPar = (TDimPngParams *) fmtHndl->internalParams;
  TDimImageInfo *info = &pngPar->i;  
  TDimImageBitmap *img = fmtHndl->image;
  
  //-------------------------------------------------
  // init the image
  //-------------------------------------------------
  if ( allocImg( fmtHndl, info, img) != 0 ) return 1;

  //-------------------------------------------------
  // read the image
  //-------------------------------------------------

  if (setjmp( png_jmpbuf(pngPar->png_ptr) ))
  {
    png_destroy_read_struct( &pngPar->png_ptr, &pngPar->info_ptr, &pngPar->end_info );
    return 1;
  }

  int num_passes = png_set_interlace_handling( pngPar->png_ptr );
  int pass;

  unsigned int bpl = getLineSizeInBytes( img );  
  unsigned long h = info->height;
  unsigned long y = 0;
 
  if (img->i.samples == 1)
  {
    for ( pass=0; pass<num_passes; pass++ )
    {
      while ( y < h ) 
      {
        png_bytep p = ((unsigned char *) img->bits[0]) + ( y * bpl );
        png_read_row( pngPar->png_ptr, p, NULL );
        y++;
      }
      y = 0;
    } // interlace passes
  }
  else
  { // multi samples (channels)
    unsigned char *buf = new unsigned char [ bpl * img->i.samples ];
    
    if (num_passes == 0)
    { // faster code in the case of non interlaced image

      while ( y < h ) 
      {
        png_bytep pbuf = buf;
        png_read_row( pngPar->png_ptr, pbuf, NULL ); 

        if ( img->i.samples == 3 )
        {
          unsigned long x = 0;
          DIM_UCHAR *p0 = ((unsigned char *) img->bits[0]) + ( y * bpl );
          DIM_UCHAR *p1 = ((unsigned char *) img->bits[1]) + ( y * bpl );
          DIM_UCHAR *p2 = ((unsigned char *) img->bits[2]) + ( y * bpl );
          for ( x=0; x<bpl*3; x+=3 )
          {
            *p0 = buf[x+0]; p0++; // R
            *p1 = buf[x+1]; p1++; // G
            *p2 = buf[x+2]; p2++; // B
          }
        }

        if ( img->i.samples == 4 )
        {
          unsigned long x = 0;
          DIM_UCHAR *p0 = ((unsigned char *) img->bits[0]) + ( y * bpl );
          DIM_UCHAR *p1 = ((unsigned char *) img->bits[1]) + ( y * bpl );
          DIM_UCHAR *p2 = ((unsigned char *) img->bits[2]) + ( y * bpl );
          DIM_UCHAR *p3 = ((unsigned char *) img->bits[3]) + ( y * bpl );
          for ( x=0; x<bpl*4; x+=4 )
          {
            *p0 = buf[x+0]; p0++; // R          
            *p1 = buf[x+1]; p1++; // G
            *p2 = buf[x+2]; p2++; // B
            *p3 = buf[x+3]; p3++; // A
          }
        }

        y++;
      } // while
    }
    else
    {  // slower code which handles interlaced images
      for ( unsigned int sample=0; sample<img->i.samples; ++sample )
        memset( img->bits[sample], 0, bpl * h );

      for ( pass=0; pass<num_passes; pass++ ) {    
        while ( y < h ) {
          png_bytep pbuf = buf;
          memset( buf, 0, bpl * img->i.samples );
          png_read_row( pngPar->png_ptr, pbuf, NULL ); 

          if ( img->i.samples == 3 ) {
            unsigned long x = 0;
            DIM_UCHAR *p0 = ((unsigned char *) img->bits[0]) + ( y * bpl );
            DIM_UCHAR *p1 = ((unsigned char *) img->bits[1]) + ( y * bpl );
            DIM_UCHAR *p2 = ((unsigned char *) img->bits[2]) + ( y * bpl );
            for ( x=0; x<bpl*3; x+=3 ) {
              if (buf[x+0] != 0) *p0 = buf[x+0]; p0++; // R
              if (buf[x+1] != 0) *p1 = buf[x+1]; p1++; // G
              if (buf[x+2] != 0) *p2 = buf[x+2]; p2++; // B
            }
          }

          if ( img->i.samples == 4 ) {
            unsigned long x = 0;
            DIM_UCHAR *p0 = ((unsigned char *) img->bits[0]) + ( y * bpl );
            DIM_UCHAR *p1 = ((unsigned char *) img->bits[1]) + ( y * bpl );
            DIM_UCHAR *p2 = ((unsigned char *) img->bits[2]) + ( y * bpl );
            DIM_UCHAR *p3 = ((unsigned char *) img->bits[3]) + ( y * bpl );
            for ( x=0; x<bpl*4; x+=4 ) {
              *p0 = buf[x+0]; p0++; // R          
              *p1 = buf[x+1]; p1++; // G
              *p2 = buf[x+2]; p2++; // B
              *p3 = buf[x+3]; p3++; // A
            }
          }

          y++;
        } // while
        y = 0;
      } // interlace passes
    } // interlaced code
    delete [] buf;
  }

  png_read_end( pngPar->png_ptr, pngPar->end_info );

  return 0;
}


//****************************************************************************
// WRITE PROC
//****************************************************************************
template <typename T>
void write_png_buff ( TDimImageBitmap *img, T *buf, int y ) {
  
  unsigned int bpl = getLineSizeInBytes( img );  
  unsigned long x = 0;
  unsigned int w = img->i.width;

  if ( img->i.samples == 2 ) {
    T *p0 = (T*) ( ((unsigned char *) img->bits[0]) + ( y * bpl ) );
    T *p1 = (T*) ( ((unsigned char *) img->bits[1]) + ( y * bpl ) );
    for ( x=0; x<w*3; x+=3 ) {
      buf[x+0] = *p0; p0++; // 1          
      buf[x+1] = *p1; p1++; // 2
      buf[x+2] = 0; // B
    }
  } // if 2 channels

  if ( img->i.samples == 3 ) {
    T *p0 = (T*) ( ((unsigned char *) img->bits[0]) + ( y * bpl ) );
    T *p1 = (T*) ( ((unsigned char *) img->bits[1]) + ( y * bpl ) );
    T *p2 = (T*) ( ((unsigned char *) img->bits[2]) + ( y * bpl ) );
    for ( x=0; x<w*3; x+=3 ) {
      buf[x+0] = *p0; p0++; // R          
      buf[x+1] = *p1; p1++; // G
      buf[x+2] = *p2; p2++; // B
    }
  } // if 3 channels

  if ( img->i.samples == 4 ) {
    T *p0 = (T*) ( ((unsigned char *) img->bits[0]) + ( y * bpl ) );
    T *p1 = (T*) ( ((unsigned char *) img->bits[1]) + ( y * bpl ) );
    T *p2 = (T*) ( ((unsigned char *) img->bits[2]) + ( y * bpl ) );
    T *p3 = (T*) ( ((unsigned char *) img->bits[3]) + ( y * bpl ) );
    for ( x=0; x<w*4; x+=4 ) {
      buf[x+0] = *p0; p0++; // R          
      buf[x+1] = *p1; p1++; // G
      buf[x+2] = *p2; p2++; // B
      buf[x+3] = *p3; p3++; // A
    }
  } // if 4 chanels
}

static int write_png_image(TDimFormatHandle *fmtHndl)
{
  if (fmtHndl == NULL) return 1;
  if (fmtHndl->internalParams == NULL) return 1;
  TDimPngParams *pngPar = (TDimPngParams *) fmtHndl->internalParams;
  TDimImageBitmap *img = fmtHndl->image;
  TDimImageInfo *info = &img->i; 

  if (setjmp( png_jmpbuf(pngPar->png_ptr) ))
  {
    png_destroy_read_struct( &pngPar->png_ptr, &pngPar->info_ptr, &pngPar->end_info );
    return 1;
  }
  
  int color_type = PNG_COLOR_TYPE_GRAY;;
  if ( info->samples == 1 ) color_type = PNG_COLOR_TYPE_GRAY;
  //if ( info->samples == 2 ) color_type = PNG_COLOR_TYPE_GRAY_ALPHA;
  if ( info->samples == 2 ) color_type = PNG_COLOR_TYPE_RGB;
  if ( info->samples == 3 ) color_type = PNG_COLOR_TYPE_RGB;
  if ( info->samples == 4 ) color_type = PNG_COLOR_TYPE_RGB_ALPHA;
  if ( info->imageMode == DIM_INDEXED ) color_type = PNG_COLOR_TYPE_PALETTE;

  png_set_IHDR( pngPar->png_ptr, pngPar->info_ptr, info->width, info->height, info->depth, color_type, 
                PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);


  //-------------------------------------------------
  // write palette if any
  //-------------------------------------------------

  if ( ( color_type == PNG_COLOR_TYPE_PALETTE ) && ( info->lut.count > 0 ) )
  {
    int i;
    int num_colors = info->lut.count;
    png_colorp palette = new png_color[ num_colors ];
    png_set_PLTE( pngPar->png_ptr, pngPar->info_ptr, palette, num_colors );

    for ( i=0; i<num_colors; i++ ) 
    {
      pngPar->info_ptr->palette[i].red   = (unsigned char) dimR( info->lut.rgba[i] );
      pngPar->info_ptr->palette[i].green = (unsigned char) dimG( info->lut.rgba[i] );
      pngPar->info_ptr->palette[i].blue  = (unsigned char) dimB( info->lut.rgba[i] );
    }  

  } // if paletted


  //-------------------------------------------------
  // write meta text if any
  //-------------------------------------------------
  if ( ( fmtHndl->metaData.count > 0 ) && ( fmtHndl->metaData.tags != NULL ) )
  {
    TDimTagList *tagList = &fmtHndl->metaData;
    png_textp text_ptr = new png_text[ tagList->count ];    
    std::string str = "", keystr = ""; 
    unsigned int i;

    for (i=0; i<tagList->count; i++)
    {
      TDimTagItem *tagItem = &tagList->tags[i];
      str = "";
      keystr = tagItem->tagId;

      if ( tagItem->tagType == DIM_TAG_ASCII ) 
        str = (char *) tagItem->tagData;
      
      if ( str.size() < 40 )
        text_ptr[i].compression = PNG_TEXT_COMPRESSION_NONE;
      else
        text_ptr[i].compression = PNG_TEXT_COMPRESSION_zTXt;

      char *keychar = new char [ keystr.size() ];
      char *strchar = new char [ str.size() ];
      strcpy ( keychar, keystr.c_str() );
      strcpy ( strchar, str.c_str() );

      text_ptr[i].key  = (png_charp) keychar;
      text_ptr[i].text = (png_charp) strchar;

    } // for i

    png_set_text( pngPar->png_ptr, pngPar->info_ptr, text_ptr, tagList->count );
    delete [] text_ptr;
  } // if there are meta tags

  //-------------------------------------------------
  // write image
  //-------------------------------------------------
  png_write_info( pngPar->png_ptr, pngPar->info_ptr );
  if ( (info->depth > 8) && (!dimBigendian) )  png_set_swap( pngPar->png_ptr );

  unsigned int bpl = getLineSizeInBytes( img );  
  unsigned long h = info->height;
  unsigned long y = 0;

  if (img->i.samples == 1) {
    while ( y < h ) {
      png_bytep p = ((unsigned char *) img->bits[0]) + ( y * bpl );
      png_write_row( pngPar->png_ptr, p );
      y++;
    }
    y = 0;
  }
  else { // multi samples (channels)
    int ch = img->i.samples;
    if (ch == 2) ch = 3;
    unsigned char *buf = new unsigned char [ bpl * ch ];

    while ( y < h ) {
      png_bytep pbuf = buf;

      if (info->depth == 8)
        write_png_buff<DIM_UCHAR> ( img, (DIM_UCHAR *) buf, y );
      
      if (info->depth == 16)
        write_png_buff<DIM_UINT16> ( img, (DIM_UINT16 *) buf, y );

      png_write_row( pngPar->png_ptr, pbuf );

      y++;
    } // while
 
    delete [] buf;
  }

  png_write_end( pngPar->png_ptr, pngPar->info_ptr );

  return 0;
}

//****************************************************************************
// READ METADATA TEXT PROC
//****************************************************************************

static char *png_read_meta_as_text(TDimFormatHandle *fmtHndl)
{
  if (fmtHndl == NULL) return NULL;
  if (fmtHndl->internalParams == NULL) return NULL;
  TDimPngParams *pngPar = (TDimPngParams *) fmtHndl->internalParams;

  char *buf = NULL;
  std::string str = ""; 
  
  if (setjmp( png_jmpbuf(pngPar->png_ptr) ))
  {
    png_destroy_read_struct( &pngPar->png_ptr, &pngPar->info_ptr, &pngPar->end_info );
    return NULL;
  }

  png_textp text_ptr;
  int num_text = 0;
  png_get_text( pngPar->png_ptr, pngPar->info_ptr, &text_ptr, &num_text );

  while (num_text--) 
  {
    str += "[ Key: ";   
    str += text_ptr->key;
    str += " ]\n";
    str += text_ptr->text;
    str += "\n\n";
    text_ptr++;
  }

  buf = new char [str.size()+1];
  buf[str.size()] = '\0';
  memcpy( buf, str.c_str(), str.size() );
  
  return buf;
}

//----------------------------------------------------------------------------
// read meta data tag by tag
//----------------------------------------------------------------------------

static DIM_UINT add_one_png_tag (TDimFormatHandle *fmtHndl, int tag, const char* str)
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

    item.tagGroup  = DIM_META_PNG;
    item.tagId     = tag;
    item.tagType   = buf_type;
    item.tagLength = buf_size;
    item.tagData   = buf;

    addMetaTag( &fmtHndl->metaData, item);
  }

  return 0;
}

static DIM_UINT read_png_metadata (TDimFormatHandle *fmtHndl, int group, int tag, int type)
{
  if (fmtHndl == NULL) return 1;
  if (fmtHndl->internalParams == NULL) return 1;
  TDimPngParams *pngPar = (TDimPngParams *) fmtHndl->internalParams;
  
  if ( (group != DIM_META_PNG) && (group != -1) ) return 1;

  if (setjmp( png_jmpbuf(pngPar->png_ptr) ))
  {
    png_destroy_read_struct( &pngPar->png_ptr, &pngPar->info_ptr, &pngPar->end_info );
    return 1;
  }

  png_textp text_ptr;
  int num_text = 0;
  png_get_text( pngPar->png_ptr, pngPar->info_ptr, &text_ptr, &num_text );

  int i = 0;
  while ( i < num_text) {
    std::string str = "";
    str += "[Key: ";   
    str += text_ptr->key;
    str += "]\n";
    str += text_ptr->text;
    
    add_one_png_tag ( fmtHndl, i, str.c_str() );

    ++text_ptr;
    ++i;
  }

  tag; type;
  return 0;
}















