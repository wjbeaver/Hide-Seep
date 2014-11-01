/*****************************************************************************
  TINY TIFF READER
  Copyright (c) 2004 by Dmitry V. Fedorov <www.dimin.net> <dima@dimin.net>

  IMPLEMENTATION
  
  Programmer: Dima V. Fedorov <mailto:dima@dimin.net> <http://www.dimin.net/>
    
  History:
    03/29/2004 22:23 - First creation
        
  Ver : 1
*****************************************************************************/

#ifndef DIM_TINY_TIFF_H
#define DIM_TINY_TIFF_H

#include <tiffio.h>
#include <tif_dir.h>
#include <tiffiop.h>

#ifndef uchar
#define uchar unsigned char
#endif  

static int tag_size_bytes[13] = { 1, 1, 1, 2, 4, 8, 1, 1, 2, 4, 8, 4, 8 };

static int one = 1;
static int bigendian = (*(char *)&one == 0);
static int swabflag = 0; // we can only set this one opening file, 
                         // it depends on internal endian

typedef struct {
  uint16 tiff_magic;  
  uint16 tiff_version;
  uint32 tiff_diroff;
} TDimTiffHeader;

/*
typedef struct {
	uint16 tiff_magic;      // magic number (defines byte order) 
	uint16 tiff_version;    // TIFF version number 
} TIFFHeaderCommon;
typedef struct {
	uint16 tiff_magic;      // magic number (defines byte order) 
	uint16 tiff_version;    // TIFF version number 
	uint32 tiff_diroff;     // byte offset to first directory 
} TIFFHeaderClassic;
typedef struct {
	uint16 tiff_magic;      // magic number (defines byte order) 
	uint16 tiff_version;    // TIFF version number 
	uint16 tiff_offsetsize; // size of offsets, should be 8
	uint16 tiff_unused;     // unused word, should be 0
	uint64 tiff_diroff;     // byte offset to first directory 
} TIFFHeaderBig;
*/


typedef struct TDimTiffIFDEntry {
  uint16 tag; 
  uint16 type;
  uint32 count;
  uint32 offset;
} TDimTiffIFDEntry;

typedef struct TDimTiffIFD {
  uint16 count; 
  TDimTiffIFDEntry *entries;
  uint32 next;
} TDimTiffIFD;

typedef struct TDimTiffIFDs {
  uint16 count; 
  TDimTiffIFD *ifds;
} TDimTiffIFDs;

  TDimTiffIFDs initTDimTiffIFDs ();
  void clearTiffIFD (TDimTiffIFD *ifd);  

  bool isTagPresentInIFD ( TDimTiffIFD *ifd, uint32 tag );
  bool isTagPresentInFirstIFD ( TDimTiffIFDs *ifds, uint32 tag );
  int getTagPositionInIFD ( TDimTiffIFD *ifd, uint32 tag );


//  TDimTiffIFD readFirstTiffIFD (FILE *stream, int offset);
//  void readTiffTag (TDimTiffIFD *ifd, FILE *stream, uint32 tag, uint32 &size, void **buf);

#ifdef _TIFF_ 

  void clearTiffIFDs (TDimTiffIFDs *ifds);
  
  TDimTiffIFD readFirstTiffIFD (TIFF *tif);
  TDimTiffIFDs readAllTiffIFDs (TIFF *tif);
  void freeTiffTagBuf( uchar **buf );

  int32 getTiffTagOffset(TIFF *tif, TDimTiffIFD *ifd, uint32 tag);
  int32 getTiffTagCount(TIFF *tif, TDimTiffIFD *ifd, uint32 tag);

  // reads to buffer data of size in bytes from determant offset and do necessary convertion
  void readTiffBuf (TIFF *tif, uint32 offset, uint32 size, uint32 type, uchar **buf);
  int readTiffBufNoAlloc (TIFF *tif, uint32 offset, uint32 size, uint32 type, uchar *buf);
  void readTiffTag (TIFF *tif, TDimTiffIFD *ifd, uint32 tag, uint32 &size, uint32 &type, uchar **buf);
  // this function reads tif tag using provided size and type instead of IFD values
  void readTiffCustomTag (TIFF *tif, TDimTiffIFD *ifd, uint32 tag, uint32 size, uint32 type, uchar **buf);

#endif

#endif //DIM_TINY_TIFF_H
