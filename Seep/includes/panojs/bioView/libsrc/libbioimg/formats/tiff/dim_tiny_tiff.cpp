/*****************************************************************************
  TINY TIFF READER
  Copyright (c) 2004 by Dmitry V. Fedorov <www.dimin.net> <dima@dimin.net>

  IMPLEMENTATION
  
  Programmer: Dima V. Fedorov <mailto:dima@dimin.net> <http://www.dimin.net/>
    
  History:
    03/29/2004 22:23 - First creation
    09/28/2005 23:10 - fixed bug in swabData        

  Ver : 2
*****************************************************************************/

#include "dim_tiny_tiff.h"

TDimTiffIFD initTDimTiffIFD ()
{
  TDimTiffIFD ifd;
  ifd.entries = NULL;
  ifd.count = 0;	
  ifd.next = 0;
  return ifd;
}

TDimTiffIFDs initTDimTiffIFDs ()
{
  TDimTiffIFDs ifds;
  ifds.ifds = NULL;
  ifds.count = 0;	
  return ifds;
}

void clearTiffIFD (TDimTiffIFD *ifd)
{
  //#ifndef _TIFF_ 
  //delete ifd->entries;
  //#endif
  //#ifdef _TIFF_ 
  _TIFFfree( ifd->entries );
  //#endif

  *ifd = initTDimTiffIFD();
}

void clearTiffIFDs (TDimTiffIFDs *ifds)
{
  if (ifds == NULL) return;
  if (ifds->count <= 0) return;

  int i;
  for (i=0; i<ifds->count; i++)
    clearTiffIFD( &ifds->ifds[i] );

  _TIFFfree( ifds->ifds );
}

void swabData(int type, long size, void* data)
{
  if (!swabflag) return;
  
  if ( (type == TIFF_SHORT) || (type == TIFF_SSHORT) )
    TIFFSwabArrayOfShort( (uint16*) data, size/2 );

  if ( (type == TIFF_LONG) || (type == TIFF_SLONG) || (type == TIFF_FLOAT) )
    TIFFSwabArrayOfLong( (uint32*) data, size/4 );

  if (type == TIFF_RATIONAL)
    TIFFSwabArrayOfLong( (uint32*) data, size/4 );

  if (type == TIFF_DOUBLE)
    TIFFSwabArrayOfDouble( (double*) data, size/8 );

}


//----------------------------------------------------------------------------
// independent functions, not used if library is present
//----------------------------------------------------------------------------
/*
#ifndef _TIFF_ 
TDimTiffIFD readFirstTiffIFD (FILE *stream, int offset)
{
  TDimTiffIFD ifd = initTDimTiffIFD();
  int i;

  fseek(stream, offset, SEEK_SET);
  if ( fread(&ifd.count, sizeof(unsigned char), 2, stream) < 2) return ifd;

  ifd.entries = new TDimTiffIFDEntry [ifd.count];

  for (i=0; i<ifd.count; i++)
    if ( fread(&ifd.entries[i], sizeof(unsigned char), 12, stream) < 12) return ifd;

  if ( fread(&ifd.next, sizeof(unsigned char), 4, stream) < 4) return ifd;

  return ifd;
}

void readTiffTag (TDimTiffIFD *ifd, FILE *stream, uint32 tag, uint32 &size, void **buf)
{
  if (ifd == NULL) return;
  if (stream == NULL) return;

  int i;
  TDimTiffIFDEntry *ifd_entry = NULL;

  for (i=0; i<ifd->count; i++)
  {
    if (tag == ifd->entries[i].tag) {
      ifd_entry = &ifd->entries[i];
      break;
    }
  }

  if (ifd_entry != NULL) // if found tag
  {
    size = ifd_entry->count * tag_size_bytes[ifd_entry->type]; 
    if (*buf != NULL) delete *buf;
    *buf = (void *) new unsigned char [size];
    
    fseek(stream, ifd_entry->offset, SEEK_SET);
    if ( fread(*buf, sizeof(unsigned char), size, stream) < size) size = 0;
  }

}
#endif // !_TIFF_
*/

//----------------------------------------------------------------------------
// EXTENSION TO libtiff is here, it needs the library
//----------------------------------------------------------------------------
#ifdef _TIFF_ 

int32 getTiffTagOffset(TIFF *tif, TDimTiffIFD *ifd, uint32 tag) {
  if (tif == NULL) return -1;
  if (ifd == NULL) return -1;

  TDimTiffIFDEntry *ifd_entry = NULL;
  int i = getTagPositionInIFD ( ifd, tag );
  if (i != -1) ifd_entry = &ifd->entries[i];
  if (ifd_entry == NULL) return -1;
  return ifd_entry->offset;
}

int32 getTiffTagCount(TIFF *tif, TDimTiffIFD *ifd, uint32 tag) {
  if (tif == NULL) return -1;
  if (ifd == NULL) return -1;
  TDimTiffIFDEntry *ifd_entry = NULL;
  int i = getTagPositionInIFD ( ifd, tag );
  if (i != -1) ifd_entry = &ifd->entries[i];
  if (ifd_entry == NULL) return -1;
  return ifd_entry->count;
}

TDimTiffIFD readTiffIFD (TIFF *tif, uint32 ifd_offset) {
  TDimTiffIFD ifd = initTDimTiffIFD();
  if (tif == NULL) return ifd;

  tif->tif_seekproc((thandle_t) tif->tif_fd, ifd_offset, SEEK_SET);
  if (tif->tif_readproc((thandle_t) tif->tif_fd, &ifd.count, 2) < 2) return ifd;
  if (swabflag) { TIFFSwabShort(&ifd.count); }

  ifd.entries = (TDimTiffIFDEntry *) _TIFFmalloc( sizeof(TDimTiffIFDEntry) * ifd.count );

  for (int i=0; i<ifd.count; i++) {
    if (tif->tif_readproc((thandle_t) tif->tif_fd, &ifd.entries[i], 12) < 12) return ifd;
    if (swabflag) {
		  TIFFSwabShort( &ifd.entries[i].tag );
		  TIFFSwabShort( &ifd.entries[i].type );
		  TIFFSwabLong ( (uint32*) &ifd.entries[i].count );
		  TIFFSwabLong ( (uint32*) &ifd.entries[i].offset );
    }
  }

  if (tif->tif_readproc((thandle_t) tif->tif_fd, &ifd.next, 4) < 4) return ifd;
  if (swabflag) { TIFFSwabLong((uint32*) &ifd.next); }

  return ifd;
}

TDimTiffIFD readFirstTiffIFD (TIFF *tif) {
  TDimTiffHeader hdr;
  TDimTiffIFD ifd = initTDimTiffIFD();
  if (tif == NULL) return ifd;

  tif->tif_seekproc((thandle_t) tif->tif_fd, 0, SEEK_SET);
  if (tif->tif_readproc((thandle_t) tif->tif_fd, &hdr, sizeof(TDimTiffHeader) ) < (int) sizeof(TDimTiffHeader)) return ifd;  

	if (hdr.tiff_magic == TIFF_BIGENDIAN) 
		swabflag = !bigendian;
	else
		swabflag = bigendian;

  if (swabflag) {
		TIFFSwabShort(&hdr.tiff_version);
		TIFFSwabLong((uint32*) &hdr.tiff_diroff);
	}

  if (hdr.tiff_version >= 43) return ifd; 

  ifd = readTiffIFD (tif, hdr.tiff_diroff);
  return ifd;
}

TDimTiffIFDs readAllTiffIFDs (TIFF *tif) {
  TDimTiffIFDs ifds;
  TDimTiffIFD  newIFD;

  ifds.count = 1;
  ifds.ifds = (TDimTiffIFD *) _TIFFmalloc( sizeof(TDimTiffIFD) );
  
  ifds.ifds[0] = readFirstTiffIFD ( tif );
  newIFD = ifds.ifds[0];

  while (newIFD.next != 0) {
    ifds.count++;
    newIFD = readTiffIFD ( tif, newIFD.next);
    ifds.ifds = (TDimTiffIFD *) _TIFFrealloc(ifds.ifds, sizeof(TDimTiffIFD) * ifds.count );
    if (ifds.ifds == NULL) return ifds;
    ifds.ifds[ifds.count-1] = newIFD;
  }

  return ifds;
}

bool isTagPresentInIFD ( TDimTiffIFD *ifd, uint32 tag ) {
  if (ifd == NULL) return FALSE;
  for (int i=0; i<ifd->count; i++)
    if (tag == ifd->entries[i].tag) return TRUE;
  return FALSE;
}

bool isTagPresentInFirstIFD ( TDimTiffIFDs *ifds, uint32 tag ) {
  if (ifds == NULL) return FALSE;
  if (ifds->count < 1) return FALSE;
  return isTagPresentInIFD ( &ifds->ifds[0], tag );
}

int getTagPositionInIFD ( TDimTiffIFD *ifd, uint32 tag ) {
  if (ifd == NULL) return -1;
  for (int i=0; i<ifd->count; i++)
    if (tag == ifd->entries[i].tag) return i;
  return -1;
}

void freeTiffTagBuf( uchar **buf ) {
  if (*buf != NULL) _TIFFfree( *buf );
  *buf = NULL;
}

//----------------------------------------------------------------------------
// READING
//----------------------------------------------------------------------------

int readTiffBufNoAlloc (TIFF *tif, uint32 offset, uint32 size, uint32 type, uchar *buf) {
  if (!buf) return 1;
  tif->tif_seekproc((thandle_t) tif->tif_fd, offset, SEEK_SET);
  if (tif->tif_readproc((thandle_t) tif->tif_fd, buf, size) < (int) size) return 1;
  else
    swabData(type, size, buf);
  return 0;
}

void readTiffBuf (TIFF *tif, uint32 offset, uint32 size, uint32 type, uchar **buf) {
  if (*buf != NULL) _TIFFfree( *buf );
  *buf = (uchar*) _TIFFmalloc( size );
  if (readTiffBufNoAlloc (tif, offset, size, type, *buf) != 0) {
    _TIFFfree( *buf );
    *buf = NULL;
  }
}

// this function reads tif tag using IFD values
void readTiffTag (TIFF *tif, TDimTiffIFD *ifd, uint32 tag, uint32 &size, uint32 &type, uchar **buf) {
  TDimTiffIFDEntry *ifd_entry = NULL;
  size = 0;
  type = 0;
  if (tif == NULL) return;
  if (ifd == NULL) return;

  int i = getTagPositionInIFD ( ifd, tag );
  if (i != -1) ifd_entry = &ifd->entries[i]; else return;
  if (ifd_entry == NULL) return;

  type = ifd_entry->type;
  size = ifd_entry->count * tag_size_bytes[ifd_entry->type];

  if (size <= 4) // if tag contain data instead of offset
  {
    if (*buf != NULL) _TIFFfree( *buf );
    *buf = (uchar*) _TIFFmalloc( size );
    _TIFFmemcpy(*buf, &ifd_entry->offset, size);
    if (size == 4) TIFFSwabLong( (uint32*) *buf );
  }
  else // if tag contain offset
  {
    readTiffBuf ( tif, ifd_entry->offset, size, type, buf);
    if (*buf == NULL) size = 0;
  }
}

// this function reads tif tag using provided size and type instead of IFD values
void readTiffCustomTag (TIFF *tif, TDimTiffIFD *ifd, uint32 tag, uint32 size, uint32 type, uchar **buf) {
  if (tif == NULL) return;
  if (ifd == NULL) return;
  TDimTiffIFDEntry *ifd_entry = NULL;
  int i = getTagPositionInIFD ( ifd, tag );
  if (i != -1) ifd_entry = &ifd->entries[i];
  if (ifd_entry == NULL) return;
  
  readTiffBuf ( tif, ifd_entry->offset, size, type, buf);
}


#endif // _TIFF_
