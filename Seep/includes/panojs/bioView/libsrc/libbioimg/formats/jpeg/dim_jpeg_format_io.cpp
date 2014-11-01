/*****************************************************************************
  JPEG IO 
  Copyright (c) 2004 by Dmitry V. Fedorov <www.dimin.net> <dima@dimin.net>

  IMPLEMENTATION
  
  Programmer: Dima V. Fedorov <mailto:dima@dimin.net> <http://www.dimin.net/>
    

  History:
    03/29/2004 22:23 - First creation
    08/04/2004 22:25 - Update to FMT_IFS 1.2, support for io protorypes
        
  Ver : 2
*****************************************************************************/

#include "dim_jpeg_format.h"


struct my_error_mgr : public jpeg_error_mgr {
    jmp_buf setjmp_buffer;
};


extern "C" {
static void my_error_exit (j_common_ptr cinfo)
{
    my_error_mgr* myerr = (my_error_mgr*) cinfo->err;
    char buffer[JMSG_LENGTH_MAX];
    (*cinfo->err->format_message)(cinfo, buffer);
    longjmp(myerr->setjmp_buffer, 1);
}
}

static const int max_buf = 4096;

//****************************************************************************
// READ STUFF
//****************************************************************************
/*
struct my_jpeg_source_mgr : public jpeg_source_mgr {
    // Nothing dynamic - cannot rely on destruction over longjump
    FILE* stream;
    JOCTET buffer[max_buf];

public:
    my_jpeg_source_mgr(FILE* new_stream);
};

extern "C" {

static void dimjpeg_init_source(j_decompress_ptr)
{
}

static boolean dimjpeg_fill_input_buffer(j_decompress_ptr cinfo)
{
  int num_read;
  my_jpeg_source_mgr* src = (my_jpeg_source_mgr*)cinfo->src;
  src->next_input_byte = src->buffer;
  
  num_read = fread( src->buffer, 1, max_buf, src->stream);
  if ( num_read <= 0 ) 
  {
    // Insert a fake EOI marker - as per jpeglib recommendation
    src->buffer[0] = (JOCTET) 0xFF;
    src->buffer[1] = (JOCTET) JPEG_EOI;
    src->bytes_in_buffer = 2;
  } 
  else 
    src->bytes_in_buffer = num_read;

  return TRUE;
}

static void dimjpeg_skip_input_data(j_decompress_ptr cinfo, long num_bytes)
{
  my_jpeg_source_mgr* src = (my_jpeg_source_mgr*)cinfo->src;

  // `dumb' implementation from jpeglib

  // Just a dumb implementation for now.  Could use fseek() except
  // it doesn't work on pipes.  Not clear that being smart is worth
  // any trouble anyway --- large skips are infrequent.
  if (num_bytes > 0) 
  {
    while (num_bytes > (long) src->bytes_in_buffer) 
    {
      num_bytes -= (long) src->bytes_in_buffer;
      (void) dimjpeg_fill_input_buffer(cinfo);
      // note we assume that qt_fill_input_buffer will never return FALSE,
      // so suspension need not be handled.
    }
    src->next_input_byte += (size_t) num_bytes;
    src->bytes_in_buffer -= (size_t) num_bytes;
  }
}

static void dimjpeg_term_source(j_decompress_ptr)
{
}

} // extern C


inline my_jpeg_source_mgr::my_jpeg_source_mgr(FILE* new_stream)
{
  jpeg_source_mgr::init_source       = dimjpeg_init_source;
  jpeg_source_mgr::fill_input_buffer = dimjpeg_fill_input_buffer;
  jpeg_source_mgr::skip_input_data   = dimjpeg_skip_input_data;
  jpeg_source_mgr::resync_to_restart = jpeg_resync_to_restart;
  jpeg_source_mgr::term_source       = dimjpeg_term_source;
  stream = new_stream;
  bytes_in_buffer = 0;
  next_input_byte = buffer;
}
*/


struct my_jpeg_source_mgr : public jpeg_source_mgr {
    // Nothing dynamic - cannot rely on destruction over longjump
    TDimFormatHandle *fmtHndl;
    JOCTET buffer[max_buf];

public:
    my_jpeg_source_mgr(TDimFormatHandle *new_hndl);
};

extern "C" {

static void dimjpeg_init_source(j_decompress_ptr)
{
}

static boolean dimjpeg_fill_input_buffer(j_decompress_ptr cinfo)
{
  int num_read;
  my_jpeg_source_mgr* src = (my_jpeg_source_mgr*)cinfo->src;
  src->next_input_byte = src->buffer;
  
  //num_read = fread( src->buffer, 1, max_buf, src->stream);
  num_read = dimRead( src->fmtHndl, src->buffer, 1, max_buf );

  if ( num_read <= 0 ) 
  {
    // Insert a fake EOI marker - as per jpeglib recommendation
    src->buffer[0] = (JOCTET) 0xFF;
    src->buffer[1] = (JOCTET) JPEG_EOI;
    src->bytes_in_buffer = 2;
  } 
  else 
    src->bytes_in_buffer = num_read;

  return TRUE;
}

static void dimjpeg_skip_input_data(j_decompress_ptr cinfo, long num_bytes)
{
  my_jpeg_source_mgr* src = (my_jpeg_source_mgr*)cinfo->src;

  // `dumb' implementation from jpeglib

  // Just a dumb implementation for now.  Could use fseek() except
  // it doesn't work on pipes.  Not clear that being smart is worth
  // any trouble anyway --- large skips are infrequent.
  if (num_bytes > 0) 
  {
    while (num_bytes > (long) src->bytes_in_buffer) 
    {
      num_bytes -= (long) src->bytes_in_buffer;
      (void) dimjpeg_fill_input_buffer(cinfo);
      // note we assume that qt_fill_input_buffer will never return FALSE,
      // so suspension need not be handled.
    }
    src->next_input_byte += (size_t) num_bytes;
    src->bytes_in_buffer -= (size_t) num_bytes;
  }
}

static void dimjpeg_term_source(j_decompress_ptr)
{
}

} // extern C


inline my_jpeg_source_mgr::my_jpeg_source_mgr(TDimFormatHandle *new_hndl)
{
  jpeg_source_mgr::init_source       = dimjpeg_init_source;
  jpeg_source_mgr::fill_input_buffer = dimjpeg_fill_input_buffer;
  jpeg_source_mgr::skip_input_data   = dimjpeg_skip_input_data;
  jpeg_source_mgr::resync_to_restart = jpeg_resync_to_restart;
  jpeg_source_mgr::term_source       = dimjpeg_term_source;
  fmtHndl = new_hndl;
  bytes_in_buffer = 0;
  next_input_byte = buffer;
}


//----------------------------------------------------------------------------
// READ PROC
//----------------------------------------------------------------------------

static int read_jpeg_image(TDimFormatHandle *fmtHndl) {
  JSAMPROW row_pointer[1];
  TDimImageBitmap *image = fmtHndl->image;
  struct jpeg_decompress_struct cinfo;

  struct my_jpeg_source_mgr *iod_src = new my_jpeg_source_mgr( fmtHndl );
  struct my_error_mgr jerr;

  jpeg_create_decompress(&cinfo);

  cinfo.src = iod_src;

  cinfo.err = jpeg_std_error(&jerr);
  jerr.error_exit = my_error_exit;

  if (!setjmp(jerr.setjmp_buffer)) {

    (void) jpeg_read_header(&cinfo, TRUE);

    (void) jpeg_start_decompress(&cinfo);

    if (allocImg( image, cinfo.output_width, cinfo.output_height, cinfo.output_components, 8 ) != 0) {
      jpeg_destroy_decompress(&cinfo);      
      return 1;
    }

    if (cinfo.output_components == 1) {
      while (cinfo.output_scanline < cinfo.output_height) {
        dimProgress( fmtHndl, cinfo.output_scanline, cinfo.output_height, "Reading JPEG" );
        if ( dimTestAbort( fmtHndl ) == 1) break;  

        row_pointer[0] =  ((uchar *) image->bits[0]) + (cinfo.output_width*cinfo.output_scanline);
        (void) jpeg_read_scanlines( &cinfo, row_pointer, 1 );
      }
    } // if 1 component
    

    if ( cinfo.output_components == 3 ) {
      row_pointer[0] = new uchar[cinfo.output_width*cinfo.output_components];

      while (cinfo.output_scanline < cinfo.output_height) {
        dimProgress( fmtHndl, cinfo.output_scanline, cinfo.output_height, "Reading JPEG" );
        if ( dimTestAbort( fmtHndl ) == 1) break; 

        register unsigned int i;        
        (void) jpeg_read_scanlines( &cinfo, row_pointer, 1 );
        uchar *row = row_pointer[0];        
        uchar* pix1 = ((uchar *) image->bits[0]) + cinfo.output_width * (cinfo.output_scanline-1);
        uchar* pix2 = ((uchar *) image->bits[1]) + cinfo.output_width * (cinfo.output_scanline-1);
        uchar* pix3 = ((uchar *) image->bits[2]) + cinfo.output_width * (cinfo.output_scanline-1);

        for (i=0; i<cinfo.output_width; i++) {
          *pix1++ = *row++;
          *pix2++ = *row++;
          *pix3++ = *row++;
        }

      } // while scanlines
      delete row_pointer[0];
    } // if 3 components

    if ( cinfo.output_components == 4 ) {
      row_pointer[0] = new uchar[cinfo.output_width*cinfo.output_components];

      while (cinfo.output_scanline < cinfo.output_height) {
        dimProgress( fmtHndl, cinfo.output_scanline, cinfo.output_height, "Reading JPEG" );
        if ( dimTestAbort( fmtHndl ) == 1) break; 

        register unsigned int i;        
        (void) jpeg_read_scanlines( &cinfo, row_pointer, 1 );
        uchar *row = row_pointer[0];        
        uchar* pix1 = ((uchar *) image->bits[0]) + cinfo.output_width * (cinfo.output_scanline-1);
        uchar* pix2 = ((uchar *) image->bits[1]) + cinfo.output_width * (cinfo.output_scanline-1);
        uchar* pix3 = ((uchar *) image->bits[2]) + cinfo.output_width * (cinfo.output_scanline-1);
        uchar* pix4 = ((uchar *) image->bits[3]) + cinfo.output_width * (cinfo.output_scanline-1);

        for (i=0; i<cinfo.output_width; i++) {
          *pix1++ = *row++;
          *pix2++ = *row++;
          *pix3++ = *row++;
          *pix4++ = *row++;
        }

      } // while scanlines
      delete row_pointer[0];
    } // if 4 components

    (void) jpeg_finish_decompress(&cinfo);
  }

  jpeg_destroy_decompress(&cinfo);
  delete iod_src;

  // set some image parameters
  image->i.number_pages = 1;

  return 0;
}


//****************************************************************************
// WRITE STUFF
//****************************************************************************
/*
struct my_jpeg_destination_mgr : public jpeg_destination_mgr {
    // Nothing dynamic - cannot rely on destruction over longjump
    FILE* stream;
    JOCTET buffer[max_buf];

public:
    my_jpeg_destination_mgr(FILE* stream);
};

extern "C" {

static void dimjpeg_init_destination(j_compress_ptr)
{
}

static void dimjpeg_exit_on_error(j_compress_ptr cinfo, FILE* stream)
{
  // cinfo->err->msg_code = JERR_FILE_WRITE;
  fflush( stream );
  (*cinfo->err->error_exit)((j_common_ptr)cinfo);
}

static boolean dimjpeg_empty_output_buffer(j_compress_ptr cinfo)
{
  my_jpeg_destination_mgr* dest = (my_jpeg_destination_mgr*)cinfo->dest;
  
  if ( fwrite((char*)dest->buffer, 1, max_buf, dest->stream) != max_buf )
    dimjpeg_exit_on_error(cinfo, dest->stream);

  fflush( dest->stream );
  dest->next_output_byte = dest->buffer;
  dest->free_in_buffer = max_buf;

  return TRUE;
}

static void dimjpeg_term_destination(j_compress_ptr cinfo)
{
  my_jpeg_destination_mgr* dest = (my_jpeg_destination_mgr*)cinfo->dest;
  unsigned int n = max_buf - dest->free_in_buffer;
  
  if ( fwrite((char*)dest->buffer, 1, n, dest->stream) != n )
    dimjpeg_exit_on_error(cinfo, dest->stream);

  fflush( dest->stream );
  dimjpeg_exit_on_error( cinfo, dest->stream );
}

} // extern C

inline my_jpeg_destination_mgr::my_jpeg_destination_mgr(FILE* new_stream)
{
  jpeg_destination_mgr::init_destination    = dimjpeg_init_destination;
  jpeg_destination_mgr::empty_output_buffer = dimjpeg_empty_output_buffer;
  jpeg_destination_mgr::term_destination    = dimjpeg_term_destination;
  stream = new_stream;
  next_output_byte = buffer;
  free_in_buffer = max_buf;
}
*/

struct my_jpeg_destination_mgr : public jpeg_destination_mgr {
    // Nothing dynamic - cannot rely on destruction over longjump
    TDimFormatHandle *fmtHndl;
    JOCTET buffer[max_buf];

public:
    my_jpeg_destination_mgr(TDimFormatHandle *new_hndl);
};

extern "C" {

static void dimjpeg_init_destination(j_compress_ptr)
{
}

static void dimjpeg_exit_on_error(j_compress_ptr cinfo, TDimFormatHandle *fmtHndl)
{
  // cinfo->err->msg_code = JERR_FILE_WRITE;
  dimFlush( fmtHndl );
  (*cinfo->err->error_exit)((j_common_ptr)cinfo);
}

static boolean dimjpeg_empty_output_buffer(j_compress_ptr cinfo)
{
  my_jpeg_destination_mgr* dest = (my_jpeg_destination_mgr*)cinfo->dest;
  
  if ( dimWrite( dest->fmtHndl, (char*)dest->buffer, 1, max_buf) != max_buf )
    dimjpeg_exit_on_error(cinfo, dest->fmtHndl);

  dimFlush( dest->fmtHndl );
  dest->next_output_byte = dest->buffer;
  dest->free_in_buffer = max_buf;

  return TRUE;
}

static void dimjpeg_term_destination(j_compress_ptr cinfo)
{
  my_jpeg_destination_mgr* dest = (my_jpeg_destination_mgr*)cinfo->dest;
  unsigned int n = max_buf - dest->free_in_buffer;
  
  if ( dimWrite( dest->fmtHndl, (char*)dest->buffer, 1, n ) != n )
    dimjpeg_exit_on_error(cinfo, dest->fmtHndl);

  dimFlush( dest->fmtHndl );
  dimjpeg_exit_on_error( cinfo, dest->fmtHndl );
}

} // extern C

inline my_jpeg_destination_mgr::my_jpeg_destination_mgr(TDimFormatHandle *new_hndl)
{
  jpeg_destination_mgr::init_destination    = dimjpeg_init_destination;
  jpeg_destination_mgr::empty_output_buffer = dimjpeg_empty_output_buffer;
  jpeg_destination_mgr::term_destination    = dimjpeg_term_destination;
  fmtHndl = new_hndl;
  next_output_byte = buffer;
  free_in_buffer = max_buf;
}

//----------------------------------------------------------------------------
// WRITE PROC
//----------------------------------------------------------------------------

static int write_jpeg_image( TDimFormatHandle *fmtHndl )
{
  TDimImageBitmap *image = fmtHndl->image;

  struct jpeg_compress_struct cinfo;
  JSAMPROW row_pointer[1];
  row_pointer[0] = 0;

  //struct my_jpeg_destination_mgr *iod_dest = new my_jpeg_destination_mgr( fmtHndl->stream );
  struct my_jpeg_destination_mgr *iod_dest = new my_jpeg_destination_mgr( fmtHndl );
  struct my_error_mgr jerr;

  cinfo.err = jpeg_std_error(&jerr);

  jerr.error_exit = my_error_exit;

  if (!setjmp(jerr.setjmp_buffer)) 
  {
    jpeg_create_compress(&cinfo);
    cinfo.dest = iod_dest;

    cinfo.image_width  = image->i.width;
    cinfo.image_height = image->i.height;


    TDimLUT* cmap=0;
    bool gray=TRUE;

    if ( image->i.samples == 1 )
    {
      cinfo.input_components = 1;
      cinfo.in_color_space = JCS_GRAYSCALE;
      gray = TRUE;
    }
    else
    {
      cinfo.input_components = 3;
      cinfo.in_color_space = JCS_RGB;
      gray = FALSE;
    }

    if ( image->i.depth < 8 )
    {
      cmap = &image->i.lut;
      gray = TRUE;
      if (cmap->count > 0) gray = FALSE;
      cinfo.input_components = gray ? 1 : 3;
      cinfo.in_color_space = gray ? JCS_GRAYSCALE : JCS_RGB;
    }

    jpeg_set_defaults(&cinfo);
    int quality = fmtHndl->quality;
    if (quality < 1) quality = 1;
    if (quality > 100) quality = 100;    
    jpeg_set_quality(&cinfo, quality, TRUE ); // limit to baseline-JPEG values );

    jpeg_start_compress(&cinfo, TRUE);

    row_pointer[0] = new uchar[cinfo.image_width*cinfo.input_components];
    int w = cinfo.image_width;
    long lineSizeBytes = getLineSizeInBytes( image );

    while (cinfo.next_scanline < cinfo.image_height) 
    {
      uchar *row = row_pointer[0];

      /*
      switch ( image.depth() ) 
      {
        case 1:
          if (gray) 
          {
            uchar* data = image.scanLine(cinfo.next_scanline);
            if ( image.bitOrder() == QImage::LittleEndian ) 
            {
              for (int i=0; i<w; i++) 
              {
                bool bit = !!(*(data + (i >> 3)) & (1 << (i & 7)));
                row[i] = qRed(cmap[bit]);
              }
            } 
            else 
            {
              for (int i=0; i<w; i++) 
              {
                bool bit = !!(*(data + (i >> 3)) & (1 << (7 -(i & 7))));
                row[i] = qRed(cmap[bit]);
              }
            }
          } 
          else 
          {
            uchar* data = image.scanLine(cinfo.next_scanline);
            if ( image.bitOrder() == QImage::LittleEndian ) 
            {
              for (int i=0; i<w; i++) 
              {
                bool bit = !!(*(data + (i >> 3)) & (1 << (i & 7)));
                *row++ = qRed(cmap[bit]);
                *row++ = qGreen(cmap[bit]);
                *row++ = qBlue(cmap[bit]);
              }
            } 
            else 
            {
              for (int i=0; i<w; i++) 
              {
                bool bit = !!(*(data + (i >> 3)) & (1 << (7 -(i & 7))));
                *row++ = qRed(cmap[bit]);
                *row++ = qGreen(cmap[bit]);
                *row++ = qBlue(cmap[bit]);
              }
            }
          }
          
          break;
        */


      //if 4 bits per sample, there should be only one sample
      if (image->i.depth == 4)
      {
        if (gray)
        {
          uchar* pix = ((uchar *) image->bits[0]) + lineSizeBytes * cinfo.next_scanline;
          uchar pixH, pixL;
          for (int i=0; i<lineSizeBytes; i++) 
          {
            pixH = (unsigned char) ((*pix) << 4);
            pixL = (unsigned char) ((*pix) >> 4);
            *row++ = pixH;
            if (i+1<w) *row++ = pixL;
            pix++;
          }
        } // if one sample with 8 bits
        else
        {
          uchar pixH, pixL;
          uchar* pix = ((uchar *) image->bits[0]) + lineSizeBytes * cinfo.next_scanline;

          for (int i=0; i<lineSizeBytes; i++) 
          {
            pixH = (unsigned char) ((*pix) << 4);
            pixL = (unsigned char) ((*pix) >> 4);
            *row++ = (unsigned char) dimR( cmap->rgba[pixH] );
            *row++ = (unsigned char) dimG( cmap->rgba[pixH] );
            *row++ = (unsigned char) dimB( cmap->rgba[pixH] );
            if (i+1<w) 
            {
              *row++ = (unsigned char) dimR( cmap->rgba[pixL] );
              *row++ = (unsigned char) dimG( cmap->rgba[pixL] );
              *row++ = (unsigned char) dimB( cmap->rgba[pixL] );
            }
            pix++;
          }
        } // if paletted image
      } // 4 bits per sample

      
      //if 8 bits per sample
      if (image->i.depth == 8)
      {
        if (gray)
        {
          uchar* pix = ((uchar *) image->bits[0]) + lineSizeBytes * cinfo.next_scanline;

          memcpy( row, pix, w );
          row += w;
        } // if one sample with 8 bits
        else
        {
          if (image->i.samples == 1)
          {
            uchar* pix = ((uchar *) image->bits[0]) + lineSizeBytes * cinfo.next_scanline;
            for (int i=0; i<w; i++) 
            {
              *row++ = (unsigned char) dimR( cmap->rgba[*pix] );
              *row++ = (unsigned char) dimG( cmap->rgba[*pix] );
              *row++ = (unsigned char) dimB( cmap->rgba[*pix] );
              pix++;
            }
          } // if paletted image

          if (image->i.samples == 2)
          {
            uchar* pix1 = ((uchar *) image->bits[0]) + lineSizeBytes * cinfo.next_scanline;
            uchar* pix2 = ((uchar *) image->bits[1]) + lineSizeBytes * cinfo.next_scanline;
            for (int i=0; i<w; i++) 
            {
              *row++ = *pix1;
              *row++ = *pix2;
              *row++ = 0;
              pix1++; pix2++;
            }
          } // if 2 samples

          if (image->i.samples >= 3)
          {
            uchar* pix1 = ((uchar *) image->bits[0]) + lineSizeBytes * cinfo.next_scanline;
            uchar* pix2 = ((uchar *) image->bits[1]) + lineSizeBytes * cinfo.next_scanline;
            uchar* pix3 = ((uchar *) image->bits[2]) + lineSizeBytes * cinfo.next_scanline;
            for (int i=0; i<w; i++) 
            {
              *row++ = *pix1;
              *row++ = *pix2;
              *row++ = *pix3;
              pix1++; pix2++; pix3++;
            }
          } // if 3 or more samples
        } // if not gray
      } // 8 bits per sample


      //if 16 bits per sample
      if (image->i.depth == 16)
      {
        if (image->i.samples == 1)
        {
          DIM_UINT16* pix = (DIM_UINT16*) (((uchar *) image->bits[0]) + lineSizeBytes * cinfo.next_scanline);

          for (int i=0; i<w; i++) 
          {
            *row++ = (uchar) (*pix / 256);
            ++pix;
          }
        } // if paletted image

        if (image->i.samples == 2)
        {
          DIM_UINT16* pix1 = (DIM_UINT16*) (((uchar *) image->bits[0]) + lineSizeBytes * cinfo.next_scanline);
          DIM_UINT16* pix2 = (DIM_UINT16*) (((uchar *) image->bits[1]) + lineSizeBytes * cinfo.next_scanline);
          for (int i=0; i<w; i++) 
          {
            *row++ = (uchar) (*pix1 / 256);
            *row++ = (uchar) (*pix2 / 256);
            *row++ = 0;
            ++pix1; ++pix2;
          }
        } // if 2 samples

        if (image->i.samples >= 3)
        {
          DIM_UINT16* pix1 = (DIM_UINT16*) (((uchar *) image->bits[0]) + lineSizeBytes * cinfo.next_scanline);
          DIM_UINT16* pix2 = (DIM_UINT16*) (((uchar *) image->bits[1]) + lineSizeBytes * cinfo.next_scanline);
          DIM_UINT16* pix3 = (DIM_UINT16*) (((uchar *) image->bits[2]) + lineSizeBytes * cinfo.next_scanline);
          for (int i=0; i<w; i++) 
          {
            *row++ = (uchar) (*pix1 / 256);
            *row++ = (uchar) (*pix2 / 256);
            *row++ = (uchar) (*pix3 / 256);
            ++pix1; ++pix2; ++pix3;
          }
        } // if 3 or more samples

      } // 16 bits per sample

      jpeg_write_scanlines(&cinfo, row_pointer, 1);
    }

    jpeg_finish_compress(&cinfo);
    jpeg_destroy_compress(&cinfo);
  }

  delete iod_dest;
  delete row_pointer[0];

  return 0;
}






