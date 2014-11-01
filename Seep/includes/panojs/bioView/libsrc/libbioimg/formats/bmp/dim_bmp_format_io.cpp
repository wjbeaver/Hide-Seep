/*****************************************************************************
  BMP IO 
  Copyright (c) 2004 by Dmitry V. Fedorov <www.dimin.net> <dima@dimin.net>

  IMPLEMENTATION
  
  Programmer: Dima V. Fedorov <mailto:dima@dimin.net> <http://www.dimin.net/>

  CURRENT STATE: NO RLE COMPRESSION SUPPORTED RIGHT NOW!!!  

  History:
    03/29/2004 22:23 - First creation
    05/10/2004 14:55 - Big endian support
    08/04/2004 22:25 - Update to FMT_IFS 1.2, support for io protorypes
    01/23/2007 22:14 - fixed 4 channel ordering
        
  Ver : 4
*****************************************************************************/

#include <string>

#include "dim_bmp_format.h"

//****************************************************************************
// READ PROC
//****************************************************************************

static int read_bmp_image(TDimFormatHandle *fmtHndl)
{
  if (fmtHndl == NULL) return 1;
  if (fmtHndl->internalParams == NULL) return 1;
  TDimBmpParams *bmpPar = (TDimBmpParams *) fmtHndl->internalParams;
  TDimImageInfo *info = &bmpPar->i;  
  TDimImageBitmap *img = fmtHndl->image;
  
  //-------------------------------------------------
  // init the image
  //-------------------------------------------------
  if ( allocImg( fmtHndl, info, img) != 0 ) return 1;

  //-------------------------------------------------
  // read the image
  //-------------------------------------------------

  // offset can be bogus, verify
  if ( bmpPar->bf.bfOffBits <= 0 ) return FALSE;
  if ( dimSeek( fmtHndl, bmpPar->bf.bfOffBits, SEEK_SET ) != 0 ) return FALSE;
  
  int bpl = getLineSizeInBytes( img );
  unsigned long bplF = (( bmpPar->bi.biWidth * bmpPar->bi.biBitCount + 31)/32)*4;

    
  //-------------------------------------------------
  // 1,4,8 bit uncompressed BMP image
  //-------------------------------------------------
  if ( (bmpPar->bi.biBitCount == 1) || ( (bmpPar->bi.biBitCount <= 8) && (bmpPar->bi.biCompression == DIM_BMP_RGB) ) ) 
  {       
    int h = bmpPar->bi.biHeight;
    unsigned char *buf = new unsigned char [bplF];      
    
    while ( --h >= 0 ) {
      dimProgress( fmtHndl, bmpPar->bi.biHeight-h, bmpPar->bi.biHeight, "Reading BMP" );

      DIM_UCHAR *p = ((unsigned char *) img->bits[0]) + ( h * bpl );
      if ( dimRead( fmtHndl, buf, 1, bplF ) != bplF) return 1;
      memcpy( p, buf, bpl );
    }
    delete [] buf;
  }

  //-------------------------------------------------
  // 4 bit BMP image
  //-------------------------------------------------  
  if ( ( bmpPar->bi.biBitCount == 4 ) && (bmpPar->bi.biCompression == DIM_BMP_RLE4) )
  {      
    /*
    int    buflen = ((w+7)/8)*4;
    uchar *buf    = new uchar[buflen];
    Q_CHECK_PTR( buf );

    int x=0, y=0, b, c, i;
    register uchar *p = line[h-1];
    uchar *endp = line[h-1]+w;
    while ( y < h ) {
      if ( (b=d->getch()) == EOF )
        break;
      if ( b == 0 ) {     // escape code
        switch ( (b=d->getch()) ) {
        case 0:     // end of line
          x = 0;
          y++;
          p = line[h-y-1];
          break;
        case 1:     // end of image
        case EOF:   // end of file
          y = h;    // exit loop
          break;
        case 2:     // delta (jump)
          x += d->getch();
          y += d->getch();
          
          // Protection
          if ( (uint)x >= (uint)w )
            x = w-1;
          if ( (uint)y >= (uint)h )
            y = h-1;
          
          p = line[h-y-1] + x;
          break;
        default:    // absolute mode
          // Protection
          if ( p + b > endp )
            b = endp-p;
          
          i = (c = b)/2;
          while ( i-- ) {
            b = d->getch();
            *p++ = b >> 4;
            *p++ = b & 0x0f;
          }
          if ( c & 1 )
            *p++ = d->getch() >> 4;
          if ( (((c & 3) + 1) & 2) == 2 )
            d->getch(); // align on word boundary
          x += c;
        }
      } else {      // encoded mode
        // Protection
        if ( p + b > endp )
          b = endp-p;
        
        i = (c = b)/2;
        b = d->getch();   // 2 pixels to be repeated
        while ( i-- ) {
          *p++ = b >> 4;
          *p++ = b & 0x0f;
        }
        if ( c & 1 )
          *p++ = b >> 4;
        x += c;
      }
    }
    */
  } // 4 bits

  //-------------------------------------------------
  // 8 bit BMP image RLE
  //-------------------------------------------------  
  if ( ( bmpPar->bi.biBitCount == 8 ) && (bmpPar->bi.biCompression == DIM_BMP_RLE8) )
  {    
    /*
    int x=0, y=0, b;
    register uchar *p = line[h-1];
    while ( y < h ) {
      if ( (b=d->getch()) == EOF )
        break;
      if ( b == 0 ) {     // escape code
        switch ( (b=d->getch()) ) {
        case 0:     // end of line
          x = 0;
          y++;
          p = line[h-y-1];
          break;
        case 1:     // end of image
        case EOF:   // end of file
          y = h;    // exit loop
          break;
        case 2:     // delta (jump)
          x += d->getch();
          y += d->getch();
          p = line[h-y-1] + x;
          break;
        default:    // absolute mode
          if ( d->readBlock( (char *)p, b ) != b )
            return FALSE;
          if ( (b & 1) == 1 )
            d->getch(); // align on word boundary
          x += b;
          p += b;
        }
      } else {      // encoded mode
        memset( p, d->getch(), b ); // repeat pixel
        x += b;
        p += b;
      }
    }
    */
  }
  
  //-------------------------------------------------
  // RGB BMP image
  //-------------------------------------------------  
  if ( bmpPar->bi.biBitCount == 16 || bmpPar->bi.biBitCount == 24 || bmpPar->bi.biBitCount == 32 )
  {  
    int h = bmpPar->bi.biHeight;
    unsigned char *bufF = new unsigned char [bplF];

    while ( --h >= 0 ) {
      dimProgress( fmtHndl, bmpPar->bi.biHeight-h, bmpPar->bi.biHeight, "Reading BMP" );
      if ( dimTestAbort( fmtHndl ) == 1) break;  

      if ( dimRead( fmtHndl, bufF, bplF, 1 ) != 1 ) return 1;

      if ( bmpPar->bi.biBitCount == 16 ) {
        // nothing yet
        // 565 - format
      }
      
      if ( bmpPar->bi.biBitCount == 24 ) {
        unsigned long x = 0;
        DIM_UCHAR *p0 = ((unsigned char *) img->bits[0]) + ( h * bpl );
        DIM_UCHAR *p1 = ((unsigned char *) img->bits[1]) + ( h * bpl );
        DIM_UCHAR *p2 = ((unsigned char *) img->bits[2]) + ( h * bpl );
        for ( x=0; x<(unsigned long)bpl*3; x+=3 ) {
          *p0 = bufF[x+2]; p0++; // R
          *p1 = bufF[x+1]; p1++; // G
          *p2 = bufF[x+0]; p2++; // B
        }
      }

      if ( bmpPar->bi.biBitCount == 32 ) {
        unsigned long x = 0;
        DIM_UCHAR *p0 = ((unsigned char *) img->bits[0]) + ( h * bpl );
        DIM_UCHAR *p1 = ((unsigned char *) img->bits[1]) + ( h * bpl );
        DIM_UCHAR *p2 = ((unsigned char *) img->bits[2]) + ( h * bpl );
        DIM_UCHAR *p3 = ((unsigned char *) img->bits[3]) + ( h * bpl );
        for ( x=0; x<(unsigned long)bpl*4; x+=4 ) {
          *p0 = bufF[x+2]; p0++; // R          
          *p1 = bufF[x+1]; p1++; // G
          *p2 = bufF[x+0]; p2++; // B
          *p3 = bufF[x+3]; p3++; // A
        }
      }

    }
    delete[] bufF;
  }

  return 0;
}


//****************************************************************************
// WRITE PROC
//****************************************************************************

static int write_bmp_image(TDimFormatHandle *fmtHndl)
{
  if (fmtHndl == NULL) return 1;
  if (fmtHndl->internalParams == NULL) return 1;
  TDimBmpParams *bmpPar = (TDimBmpParams *) fmtHndl->internalParams;
  TDimImageBitmap *img = fmtHndl->image;
  //TDimImageInfo *info = &img->i;   
  
  unsigned long bpl = getLineSizeInBytes( img );
  unsigned long num_cols = getImgNumColors( img );
  unsigned long nbits = img->i.depth * img->i.samples;
  unsigned long bpl_bmp = (( img->i.width * nbits + 31)/32)*4;

  //-------------------------------------------------
  // write BMP header
  //-------------------------------------------------  
  memcpy( &bmpPar->bf.bfType, "BM", 2 );
  bmpPar->bf.bfReserved1 = bmpPar->bf.bfReserved2 = 0;  // reserved, should be zero
  if (nbits <= 8)
    bmpPar->bf.bfOffBits = BMP_FILEHDR_SIZE + DIM_BMP_WIN + num_cols*4;
  else
    bmpPar->bf.bfOffBits = BMP_FILEHDR_SIZE + DIM_BMP_WIN;
  bmpPar->bf.bfSize      = bmpPar->bf.bfOffBits + bpl_bmp * img->i.height;

  // swap structure elements if running on Big endian machine...
  if (dimBigendian) {
    dimSwapLong( (DIM_UINT32*) &bmpPar->bf.bfOffBits );
    dimSwapLong( (DIM_UINT32*) &bmpPar->bf.bfSize );
  }

  if (dimWrite( fmtHndl, &bmpPar->bf, 1, sizeof(bmpPar->bf) ) != sizeof(bmpPar->bf)) return 1;

  //-------------------------------------------------
  // write image header
  //------------------------------------------------- 
  bmpPar->bi.biSize          = DIM_BMP_WIN;   // build info header
  bmpPar->bi.biWidth         = img->i.width;
  bmpPar->bi.biHeight        = img->i.height;
  bmpPar->bi.biPlanes        = 1;
  bmpPar->bi.biBitCount      = (unsigned short) nbits;
  bmpPar->bi.biCompression   = DIM_BMP_RGB;
  bmpPar->bi.biSizeImage     = bpl_bmp * img->i.height;
  bmpPar->bi.biXPelsPerMeter = 2834; // 72 dpi default
  bmpPar->bi.biYPelsPerMeter = 2834;
  if (nbits <= 8) {
    bmpPar->bi.biClrUsed       = num_cols;
    bmpPar->bi.biClrImportant  = num_cols;
  }
  else {
    bmpPar->bi.biClrUsed       = 0;
    bmpPar->bi.biClrImportant  = 0;
  }

  if (dimBigendian) {
    dimSwapLong( (DIM_UINT32*) &bmpPar->bi.biSize );    
    dimSwapLong( (DIM_UINT32*) &bmpPar->bi.biWidth );  
    dimSwapLong( (DIM_UINT32*) &bmpPar->bi.biHeight );  
    dimSwapShort( (DIM_UINT16*) &bmpPar->bi.biPlanes );
    dimSwapShort( (DIM_UINT16*) &bmpPar->bi.biBitCount );
    dimSwapLong( (DIM_UINT32*) &bmpPar->bi.biCompression );    
    dimSwapLong( (DIM_UINT32*) &bmpPar->bi.biSizeImage );    
    dimSwapLong( (DIM_UINT32*) &bmpPar->bi.biXPelsPerMeter );    
    dimSwapLong( (DIM_UINT32*) &bmpPar->bi.biYPelsPerMeter );    
    dimSwapLong( (DIM_UINT32*) &bmpPar->bi.biClrUsed );    
    dimSwapLong( (DIM_UINT32*) &bmpPar->bi.biClrImportant );    
  }

  if (dimWrite( fmtHndl, &bmpPar->bi, 1, sizeof(bmpPar->bi) ) != sizeof(bmpPar->bi)) return 1;

  //-------------------------------------------------
  // write image palette if there's any
  //------------------------------------------------- 
  unsigned int i;
  if (nbits <= 8)
  {    
    unsigned char *color_table = new unsigned char [ num_cols*4 ];
    unsigned char *rgb = color_table;

    if (img->i.lut.count > 0)
      for ( i=0; i<num_cols; i++ ) 
      {
        *rgb++ = (unsigned char) dimB ( img->i.lut.rgba[i] );
        *rgb++ = (unsigned char) dimG ( img->i.lut.rgba[i] );
        *rgb++ = (unsigned char) dimR ( img->i.lut.rgba[i] );
        *rgb++ = 0;
      }
    else
      for ( i=0; i<num_cols; i++ ) 
      {
        *rgb++ = iTrimUC(i * (256/num_cols));
        *rgb++ = iTrimUC(i * (256/num_cols));
        *rgb++ = iTrimUC(i * (256/num_cols));
        *rgb++ = 0;
      }

    if ( dimWrite( fmtHndl, color_table, 1, num_cols*4 ) != num_cols*4) return 1;
    delete [] color_table;
  }


  //-------------------------------------------------
  // write image data
  //-------------------------------------------------
  unsigned char *buf = new unsigned char[bpl_bmp];
  memset( buf, 0, bpl_bmp );
  int h = img->i.height;

  while ( --h >= 0 ) 
  {
    
    if (nbits <= 8) 
    {
      DIM_UCHAR *p = ((unsigned char *) img->bits[0]) + ( h * bpl );
      memcpy( buf, p, bpl );
    }
  
    if ( nbits == 16 )
    {
      // nothing yet
    }
    
    if ( ( img->i.depth == 8 ) && ( img->i.samples == 3 ) )
    {
      unsigned long x = 0;
      DIM_UCHAR *p0 = ((unsigned char *) img->bits[0]) + ( h * bpl );
      DIM_UCHAR *p1 = ((unsigned char *) img->bits[1]) + ( h * bpl );
      DIM_UCHAR *p2 = ((unsigned char *) img->bits[2]) + ( h * bpl );
      for ( x=0; x<bpl*3; x+=3 )
      {
        buf[x+2] = *p0++; // R
        buf[x+1] = *p1++; // G
        buf[x+0] = *p2++; // B
      }
    }

    if ( ( img->i.depth == 8 ) && ( img->i.samples == 4 ) )
    {
      unsigned long x = 0;
      DIM_UCHAR *p0 = ((unsigned char *) img->bits[0]) + ( h * bpl );
      DIM_UCHAR *p1 = ((unsigned char *) img->bits[1]) + ( h * bpl );
      DIM_UCHAR *p2 = ((unsigned char *) img->bits[2]) + ( h * bpl );
      DIM_UCHAR *p3 = ((unsigned char *) img->bits[3]) + ( h * bpl );
      for ( x=0; x<bpl*4; x+=4 )
      {
        buf[x+3] = *p0++; // R
        buf[x+2] = *p1++; // G
        buf[x+1] = *p2++; // B
        buf[x+0] = *p3++; // A
      }
    }

    if (dimWrite( fmtHndl, buf, 1, bpl_bmp ) != bpl_bmp) return 1;

  } // while --h
  delete[] buf;

  dimFlush( fmtHndl );
  return 0;

}











