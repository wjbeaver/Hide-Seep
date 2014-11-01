/*******************************************************************************

  Memory IO
 
  Programmer: Dima Fedorov Levit <dimin@dimin.net> <http://www.dimin.net/>
  
  Based on memio used by GDAL and libtiff by:
      Mike Johnson - Banctec AB
      Frank Warmerdam, warmerdam@pobox.com

  History:
    08/05/2004 13:57 - First creation
      
  ver: 1
        
*******************************************************************************/

#ifndef DIM_MEMIO_H
#define DIM_MEMIO_H

#include <dim_img_format_interface.h>

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct DMemIO {
    unsigned char *data;         // allocated data buffer
    unsigned long size;          // "file" size in bytes
    unsigned long data_buf_size; // in bytes - may be larger than used
    unsigned long offset;        // current file offset from start of file
    int           own_buffer;    // non-zero if 'data' is owned by MemIOBuf
} DMemIO;


void  dMemIO_Init( DMemIO *, int size, unsigned char *data );
void  dMemIO_Destroy( DMemIO * );

DIM_ULONG dMemIO_Read  ( void *buffer, DIM_ULONG size, DIM_ULONG count, DIM_STREAM_CLASS *stream );
DIM_ULONG dMemIO_Write ( void *buffer, DIM_ULONG size, DIM_ULONG count, DIM_STREAM_CLASS *stream );
DIM_INT   dMemIO_Seek  ( DIM_STREAM_CLASS *stream, DIM_LONG offset, DIM_INT origin );
DIM_INT   dMemIO_Flush ( DIM_STREAM_CLASS *stream );
DIM_ULONG dMemIO_Size  ( DIM_STREAM_CLASS *stream );
DIM_LONG  dMemIO_Tell  ( DIM_STREAM_CLASS *stream );
DIM_INT   dMemIO_Eof   ( DIM_STREAM_CLASS *stream );
DIM_INT   dMemIO_Close ( DIM_STREAM_CLASS *stream );

#if defined(__cplusplus)
}
#endif

#endif // DIM_MEMIO_H
