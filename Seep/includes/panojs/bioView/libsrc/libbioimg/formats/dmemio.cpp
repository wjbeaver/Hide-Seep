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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "dmemio.h"

static void dMemIO_ExtendFile( DMemIO *miobuf, DIM_ULONG size );

//------------------------------------------------------------------------
// Initialize a passed in DMemIO structure.                      
//------------------------------------------------------------------------

void  dMemIO_Init( DMemIO *miobuf, int size, unsigned char *data ) {
  miobuf->data = NULL;
  miobuf->size = 0;
  miobuf->offset = 0;
  miobuf->data_buf_size = 0;
  miobuf->own_buffer = 1;

  if( size > 0 ) {
      miobuf->data = data;
      miobuf->size = size;
      miobuf->data_buf_size = size;
      miobuf->own_buffer = 0;
  }
}

//------------------------------------------------------------------------
// Clear and free memory buffer
//------------------------------------------------------------------------

void  dMemIO_Destroy( DMemIO *miobuf ) {
  if( miobuf->own_buffer && miobuf->data != NULL )
      free( miobuf->data );
  memset( miobuf, 0, sizeof(DMemIO) );
}

//------------------------------------------------------------------------
// seek within the buffer
//------------------------------------------------------------------------
DIM_INT dMemIO_Seek  ( DIM_STREAM_CLASS *stream, DIM_LONG offset, DIM_INT origin ) {
  DMemIO *miobuf = (DMemIO *) stream;
  long new_off;

  if( origin == SEEK_SET )
    new_off = offset;
  else if( origin == SEEK_CUR )
    new_off = miobuf->offset + offset;
  else if( origin == SEEK_END )
    new_off = miobuf->size + offset;
  else
    return -1;

  if( new_off < 0 ) return -1;

  if( new_off > (long) miobuf->size ) {
    dMemIO_ExtendFile( miobuf, new_off );
    if ( new_off > (long) miobuf->size ) return -1;
  }
  
  miobuf->offset = new_off;
  return miobuf->offset;
}

//------------------------------------------------------------------------
// read from the buffer
//------------------------------------------------------------------------
DIM_ULONG dMemIO_Read  ( void *buffer, DIM_ULONG size, DIM_ULONG count, DIM_STREAM_CLASS *stream ) {
  DMemIO *miobuf = (DMemIO *) stream;
  int result = 0;
  DIM_ULONG nsize = size * count;

  if( miobuf->offset + nsize > miobuf->size )
    result = miobuf->size - miobuf->offset;
  else
    result = nsize;

  memcpy( buffer, miobuf->data + miobuf->offset, result );
  miobuf->offset += result;

  return (DIM_ULONG) floor( (double) result / (double) size );
}

//------------------------------------------------------------------------
// write into buffer
//------------------------------------------------------------------------

DIM_ULONG dMemIO_Write ( void *buffer, DIM_ULONG size, DIM_ULONG count, DIM_STREAM_CLASS *stream ) {
  DMemIO *miobuf = (DMemIO *) stream;
  int result = 0;
  DIM_ULONG nsize = size * count;

  if( miobuf->offset + nsize > miobuf->size )
      dMemIO_ExtendFile( miobuf, miobuf->offset + nsize );

  if( miobuf->offset + nsize > miobuf->size )
    result = miobuf->size - miobuf->offset;
  else
    result = nsize;

  memcpy( miobuf->data + miobuf->offset, buffer, result );
  miobuf->offset += result;

  return (DIM_ULONG) floor( (double)result / (double)size );
}

//------------------------------------------------------------------------
// access buffer size
//------------------------------------------------------------------------

DIM_ULONG dMemIO_Size( DIM_STREAM_CLASS *stream ) {
  DMemIO *miobuf = (DMemIO *) stream;
  return miobuf->size;
}

//------------------------------------------------------------------------
// flush the buffer - no such thing
//------------------------------------------------------------------------

DIM_INT dMemIO_Flush ( DIM_STREAM_CLASS *stream ) {
  return 0;
}

//------------------------------------------------------------------------
// return current position
//------------------------------------------------------------------------

DIM_LONG dMemIO_Tell ( DIM_STREAM_CLASS *stream ) {
  DMemIO *miobuf = (DMemIO *) stream;
  return miobuf->offset;
}

//------------------------------------------------------------------------
// test if current position is in the end of the file
//------------------------------------------------------------------------

DIM_INT dMemIO_Eof ( DIM_STREAM_CLASS *stream ) {
  DMemIO *miobuf = (DMemIO *) stream;
  if ( miobuf->offset == miobuf->size ) 
    return 1;
  else
    return 0;
}

//------------------------------------------------------------------------
// prototype for close stream funct, does nothing
//------------------------------------------------------------------------

DIM_INT dMemIO_Close ( DIM_STREAM_CLASS *stream ) {
  return 0;
}


//------------------------------------------------------------------------
// increase buffer size
//------------------------------------------------------------------------

static void dMemIO_ExtendFile( DMemIO *miobuf, DIM_ULONG size ) {
  DIM_ULONG new_buf_size;
  DIM_UINT8 *new_buffer;

  if( size < miobuf->size ) return;

  if( size < miobuf->data_buf_size ) {
      miobuf->size = size;
      return;
  }

  new_buf_size = (int) (size * 1.25);
  
  if( miobuf->own_buffer )
    new_buffer = (DIM_UINT8 *) realloc( miobuf->data, new_buf_size );
  else
  {
    new_buffer = (DIM_UINT8 *) malloc( new_buf_size );
    if( new_buffer != NULL )
      memcpy( new_buffer, miobuf->data, miobuf->size );
  }
  
  if( new_buffer == NULL ) return;

  miobuf->data = new_buffer;
  miobuf->data_buf_size = new_buf_size;
  miobuf->size = size;
}


