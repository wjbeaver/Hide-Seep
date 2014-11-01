/*******************************************************************************

  Defines Image Format Interface
  DimFiSDK version: 1.6
  
  Programmer: Dima Fedorov Levit <dimin@dimin.net> <http://www.dimin.net/>

  Image file structure:
    
    1) Page:   each file may contain 1 or more pages, each page is independent

    2) Sample: in each page each pixel can contain 1 or more samples
               preferred number of samples: 1 (GRAY), 3(RGB) and 4(RGBA)

    3) Depth:  each sample can be of 1 or more bits of depth
               preferred depths are: 8 and 16 bits per sample

    4) Allocation: each sample is allocated having in mind:
               All lines are stored contiguasly from top to bottom where
               having each particular line is byte alligned, i.e.:
               each line is allocated using minimal necessary number of bytes
               to store given depth. Which for the image means:
               size = ceil( ( (width*depth) / 8 ) * height )

  As a result of Sample/Depth structure we can get images of different 
  Bits per Pixel as: 1, 8, 24, 32, 48 and any other if needed

  TODO:
    Add: readMetaDataAsText

  History:
    03/23/2004 18:03 - First creation
    09/12/2005 17:06 - updated API to v1.3
    12/01/2005 16:24 - updated API to v1.4
    2008-04-04 14:11 - updated API to v1.6
    2009-06-29 16:21 - updated API to v1.7
    2010-01-25 16:45 - updated API to v1.8

  ver: 17
        
*******************************************************************************/

#ifndef DIM_IMG_FMT_IFC_H
#define DIM_IMG_FMT_IFC_H

#include <stdio.h>
   
// here's work around if we're on bigendian machine
// or use and DEFINE THIS IF USE SPARC or Motorola CPU!!!
//#define DIM_BIG_ENDIAN

// or rely on dimBigendian
static int dimOne = 1;
static int dimBigendian = (*(char *)&dimOne == 0);

//------------------------------------------------------------------------------
// first define type macros, you can define them as needed, OS specific
//------------------------------------------------------------------------------

/*
Datatype  LP64  ILP64 LLP64 ILP32 LP32
char       8     8     8      8      8
short     16    16    16     16     16
_int32          32      
int       32    64    32     32     16
long      64    64    32     32     32
long long       64    
pointer   64    64    64     32     32
*/ 

//#ifndef DIM_CHAR
#define DIM_CHAR char
//#endif

//#ifndef DIM_UCHAR
#define DIM_UCHAR unsigned char
//#endif

//#ifndef DIM_INT
#define DIM_INT int
#define DIM_INT8 char
#define DIM_INT16 short

#if defined WIN32 || defined WIN64 || defined _WIN32 || defined _WIN64
#define DIM_INT32 __int32
#else
#define DIM_INT32 int
#endif

#if defined WIN32 || defined WIN64 || defined _WIN32 || defined _WIN64
#define DIM_INT64 __int64
#else
#define DIM_INT64 long long
#endif
//#endif

//#ifndef DIM_UINT
#define DIM_UINT unsigned int
#define DIM_UINT8  unsigned char
#define DIM_UINT16 unsigned short

#if defined WIN32 || defined WIN64 || defined _WIN32 || defined _WIN64
#define DIM_UINT32 unsigned __int32
#else
#define DIM_UINT32 unsigned int
#endif

#if defined WIN32 || defined WIN64 || defined _WIN32 || defined _WIN64
#define DIM_UINT64 unsigned __int64
#else
#define DIM_UINT64 unsigned long long
#endif
//#endif

//#ifndef DIM_LONG
#define DIM_LONG long
//#endif

//#ifndef DIM_ULONG
#define DIM_ULONG unsigned long
//#endif

//#ifndef DIM_DOUBLE
#define DIM_DOUBLE double
//#endif

//#ifndef DIM_UDOUBLE
#define DIM_UDOUBLE unsigned double
//#endif

//#ifndef DIM_BOOL
#define DIM_BOOL bool
//#endif

#define DIM_FLOAT float
#define DIM_FLOAT32 float
#define DIM_FLOAT64 double
#define DIM_FLOAT80 long double

// HERE DEFINED DATA TYPES IN CONFORMANCE WITH TIFF STRUCTURE
// This types are used in tiff tags andmany independent structures
// like EXIF, let's stick to the it too
typedef enum {
  DIM_TAG_NOTYPE    = 0,  // placeholder
  DIM_TAG_BYTE      = 1,  // 8-bit unsigned integer
  DIM_TAG_ASCII     = 2,  // 8-bit bytes w/ last byte null
  DIM_TAG_SHORT     = 3,  // 16-bit unsigned integer
  DIM_TAG_LONG      = 4,  // 32-bit unsigned integer
  DIM_TAG_RATIONAL  = 5,  // 64-bit unsigned fraction
  DIM_TAG_SBYTE     = 6,  // 8-bit signed integer
  DIM_TAG_UNDEFINED = 7,  // 8-bit untyped data
  DIM_TAG_SSHORT    = 8,  // 16-bit signed integer
  DIM_TAG_SLONG     = 9,  // 32-bit signed integer
  DIM_TAG_SRATIONAL = 10, // 64-bit signed fraction
  DIM_TAG_FLOAT     = 11, // 32-bit IEEE floating point
  DIM_TAG_DOUBLE    = 12  // 64-bit IEEE floating point
} DIM_DataType;

// Data storage format used along with pixel depth (BPP)
typedef enum {
  D_FMT_UNDEFINED   = 0,  // placeholder type
  D_FMT_UNSIGNED    = 1,  // unsigned integer
  D_FMT_SIGNED      = 2,  // signed integer
  D_FMT_FLOAT       = 3   // floating point
} D_DataFormat;


// modes are declared similarry to Adobe Photoshop, but there are differences
typedef enum {
  DIM_BITMAP    = 0,  // 1-bit
  DIM_GRAYSCALE = 1,  // 8-bit
  DIM_INDEXED   = 2,  // 8-bit
  DIM_RGB       = 3,  // 24-bit
  DIM_BGR       = 4,  // 24-bit
  DIM_HSL       = 5,  // 24-bit
  DIM_HSB       = 6,  // 24-bit
  DIM_RGBA      = 7,  // 32-bit
  DIM_ABGR      = 8,  // 32-bit
  DIM_CMYK      = 9,  // 32-bit
  DIM_GRAY16    = 10, // 16-bit
  DIM_RGB48     = 11, // 48-bit
  DIM_MULTI     = 12, // undefined
} DIM_ImageModes;

typedef enum {
  DIM_IO_READ   = 0,
  DIM_IO_WRITE  = 1
} DIM_ImageIOModes;

// this strange numbering is due to be equal to TIFF metric units
typedef enum {
  DIM_RES_m   = 0, // Meters = 0.0254 of inch
  DIM_RES_NONE= 1, // inches
  DIM_RES_IN  = 2, // inches
  DIM_RES_cm  = 3, // Centi Meters 10^^-2 = 2.54 of inch
  DIM_RES_mm  = 4, // Mili  Meters 10^^-3
  DIM_RES_um  = 5, // Micro Meters 10^^-6
  DIM_RES_nm  = 6, // Nano  Meters 10^^-9
  DIM_RES_Gm  = 7, // Giga  Meters 10^^9  
  DIM_RES_Mm  = 8, // Mega  Meters 10^^6
  DIM_RES_km  = 9, // Kilo  Meters 10^^3
  DIM_RES_hm  = 10,// Hecto Meters 10^^2

} DIM_ResolutionUnits;

//------------------------------------------------------------------------------
// exemplar structures, you might use anything different
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// PALETTE
//------------------------------------------------------------------------------

typedef DIM_UINT32 TDimRGBA;
typedef struct TDimLUT
{
  DIM_UINT count;
  TDimRGBA rgba[256]; 
} TDimLUT;

inline int dimR( TDimRGBA rgb )   
{ return (int)((rgb >> 16) & 0xff); }

inline int dimG( TDimRGBA rgb )   
{ return (int)((rgb >> 8) & 0xff); }

inline int dimB( TDimRGBA rgb )   
{ return (int)(rgb & 0xff); }

inline int dimA( TDimRGBA rgb )   
{ return (int)((rgb >> 24) & 0xff); }

inline TDimRGBA dimRGB( int r, int g, int b )
{ return (0xff << 24) | ((r & 0xff) << 16) | ((g & 0xff) << 8) | (b & 0xff); }

inline TDimRGBA dimRGBA( int r, int g, int b, int a )
{ return ((a & 0xff) << 24) | ((r & 0xff) << 16) | ((g & 0xff) << 8) | (b & 0xff); }

inline int dimGray( int r, int g, int b )
{ return (r*11+g*16+b*5)/32; }

inline int dimGray( TDimRGBA rgb )
{ return dimGray( dimR(rgb), dimG(rgb), dimB(rgb) ); }


//------------------------------------------------------------------------------
// DIMENSIONS v1.4
// The default dimension order is: XYCZT
// each image MUST have XYC dimenstions as first in the list!!!
// C MUST be third dimention in order for the paged approach to work
// image is constituted of pages with XYC dimentions
// then the sequence of pages is interpreted as Z, T, ZT
//------------------------------------------------------------------------------

#define DIM_MAX_DIMS 10

typedef enum {
  DIM_DIM_0 = 0,  
  DIM_DIM_X = 1,
  DIM_DIM_Y = 2,
  DIM_DIM_C = 3,
  DIM_DIM_Z = 4,
  DIM_DIM_T = 5
} DIM_ImageDims;

typedef struct TDimDimension
{
  DIM_ImageDims dim;
  //DIM_UINT      resUnits;     // resolution units defined by: DIM_ResolutionUnits
  //DIM_DOUBLE    res;         // pixels per unit
  char *description;
  void *ext;
} TDimDimension;

//------------------------------------------------------------------------------
// CHANNELS v1.4
// Just a description of each channel, usually: R, G, B...
//------------------------------------------------------------------------------

#define DIM_MAX_CHANNELS 512

typedef struct TDimChannels
{
  char *description;
  void *ext;
} TDimChannels;

//------------------------------------------------------------------------------
// IMAGE
//------------------------------------------------------------------------------

typedef struct TDimImageInfo // v 1.5
{
  DIM_UINT      ver;           // must be == sizeof(TDimImageInfo)
  
  DIM_ULONG     width;
  DIM_ULONG     height;
  
  DIM_UINT      number_pages;   // number of images within the file
  DIM_UINT      number_levels;  // v1.1 number of levels in resolution pyramid
  
  // interpretative parameters, not needed for actual image decoding
  DIM_UINT      number_t;       // v1.4 number of time points
  DIM_UINT      number_z;       // v1.4 number of z depth points
  DIM_UINT      number_dims;    // v1.4 number of dimensions
  TDimDimension dimensions[DIM_MAX_DIMS]; // v1.4 dimensions description
  TDimChannels  channels[DIM_MAX_CHANNELS]; // v1.4 channels description
  DIM_UINT      file_format_id; // v1.4 id for the file format used

  TDimRGBA      transparentIndex;
  TDimRGBA      transparencyMatting;  // if A == 0: no matte
    
  DIM_UINT      imageMode;    // Image mode as declared in DIM_ImageModes
  DIM_UINT      samples;      // Samples per pixel
  DIM_UINT      depth;        // Bits per sample, currently must be 8 or 16
  D_DataFormat  pixelType;    // type in which the pixels are stored, changed in v1.8
                              // related to depth but differentiate signed/unsigned/floats etc. 
  DIM_DataType  rowAlignment; // type to which each row is aligned v1.5

  TDimLUT       lut;          // LUT, only used for Indexed Color images  

  DIM_UINT      resUnits;     // resolution units defined by: DIM_ResolutionUnits
  DIM_DOUBLE    xRes;         // pixels per unit
  DIM_DOUBLE    yRes;         // pixels per unit
  
  DIM_UINT      tileWidth;    // The width of the tiles. Zero if not set
  DIM_UINT      tileHeight;   // The height of the tiles. Zero if not set  
                      
} TDimImageInfo;

// here we should store samples like this: 0, 1, 2...
// sample meaning is defined by imageMode in TDimImageInfo
typedef struct TDimImageBitmap 
{
  TDimImageInfo i;
  void *bits[DIM_MAX_CHANNELS];  // pointer to RAW data by samples i.e. plane by plane, now restricted to 512 planes
} TDimImageBitmap;


//------------------------------------------------------------------------------
//  META DATA
//------------------------------------------------------------------------------
typedef enum {
  DIM_META_TIFF_TAG = 0,  // any tiff tag
  DIM_META_EXIF     = 1,  // whole EXIF buffer
  DIM_META_IPTC     = 2,  // whole IPTC buffer
  DIM_META_GEO_TIFF = 3,  // whole GeoTiff buffer
  DIM_META_BIORAD   = 4,  // any of BioRad PIC tag
  DIM_META_STK      = 5,  // text formated STK tags
  DIM_META_PNG      = 6,  // any of PNG text notes
  DIM_META_GENERIC  = 7   // OME tags: metadata hash or the whole OME XML
} DIM_MetaGroups;

typedef struct TDimTagItem {
  DIM_UINT32 tagGroup;  // group to which tag's ID pertense, e.g. TIFF, EXIF, BioPIC... value of (DIM_MetaGroups)
  DIM_UINT32 tagId;     // tag ID: e.g. TIFF tag number
  DIM_UINT32 tagType;   // type: format in which data is stored inside data buffer: value of DIM_DataType
  DIM_UINT32 tagLength; // how many records of type is stored in data buffer
  void      *tagData;
} TDimTagItem;

typedef struct TDimTagList {
  DIM_UINT32    count;
  TDimTagItem  *tags;
} TDimTagList;

typedef enum {
  METADATA_TAGS    = 0,  // points to DTagMap
  METADATA_OMEXML  = 1,  // points to the whole XML as a std::string
} GENERIC_MetaTags;

//******************************************************************************
// interface wide data type macros, define them using data types you desire
//******************************************************************************

class DTagMap; // v1.7 the class for textual metadata storage

// some macros for data types used by interface
#define DIM_IMAGE_CLASS         TDimImageBitmap
#define DIM_STRING_CLASS        const char

//#define DIM_STREAM_CLASS        FILE
#define DIM_STREAM_CLASS        void

#define DIM_PARAM_STREAM_CLASS  FILE
#define DIM_INTPARAMS_CLASS     void
#define DIM_PARENT_WIN_CLASS    void
#define DIM_DIALOG_WIN_CLASS    void
#define DIM_MAGIC_STREAM        void             // could be stream or just memory buffer...
#define DIM_IMG_SERVICES        void             // by now it's empty
#define DIM_FORMAT_ID           DIM_UINT32       // this number is a number in TDimFormatList

#define DIM_OPTIONS_CLASS       char
#define DIM_METADATA_CLASS      DTagMap          // v1.7


//******************************************************************************
// plug-in receves from the host program
//******************************************************************************

//-----------------------------------------------------
// generic functions that host provides
//-----------------------------------------------------
// host shows progress of the executon
// done  - value in a range of [0..total]
// total - range maximum
// descr - any text describing the executed procedure
typedef void (*TDimProgressProc)(long done, long total, char *descr);

// host shows error of the executon
// val   - error code, not treated at the moment
// descr - any text describing the error
typedef void (*TDimErrorProc)(int val, char *descr);

// plugin should use this to test is processing should be interrupted...
// If it returns TRUE, the operation should be aborted 
typedef int (*TDimTestAbortProc)( void );


//-----------------------------------------------------
// memory allocation prototypes, if these are null then 
// formats should use standard way of memory allocation
// using malloc...
//-----------------------------------------------------
typedef void* (*TDimMallocProc) (DIM_ULONG size);
typedef void* (*TDimFreeProc)   (void *p);

//-----------------------------------------------------
// io prototypes, if these are null then formats should 
// use standard C io functions: fwrite, fread...
//-----------------------------------------------------

// (fread)  read from stream  
typedef DIM_ULONG (*TDimReadProc)  ( void *buffer, DIM_ULONG size, DIM_ULONG count, DIM_STREAM_CLASS *stream );
              
// (fwrite) write into stream 
typedef DIM_ULONG (*TDimWriteProc) ( void *buffer, DIM_ULONG size, DIM_ULONG count, DIM_STREAM_CLASS *stream );

// (flush) flushes a stream
typedef DIM_INT   (*TDimFlushProc) ( DIM_STREAM_CLASS *stream );

// (fseek)  seek within stream
typedef DIM_INT   (*TDimSeekProc)  ( DIM_STREAM_CLASS *stream, DIM_LONG offset, DIM_INT origin );

// get file "stream" size (libtiff askes for it)
typedef DIM_ULONG (*TDimSizeProc)  ( DIM_STREAM_CLASS *stream );

// (ftell) gets the current position in stream
typedef DIM_LONG  (*TDimTellProc)  ( DIM_STREAM_CLASS *stream );

// (feof) tests for end-of-file on a stream
typedef DIM_INT   (*TDimEofProc)   ( DIM_STREAM_CLASS *stream );

// (feof) tests for end-of-file on a stream
typedef DIM_INT   (*TDimCloseProc) ( DIM_STREAM_CLASS *stream );

//-----------------------------------------------------


//******************************************************************************
// structure passed by host to a format function
//******************************************************************************

typedef struct TDimFormatHandle {
  
  DIM_UINT32 ver; // must be == sizeof(TDimFormatHandle)
  
  // IN
  TDimProgressProc      showProgressProc;    // function provided by host to show plugin progress
  TDimErrorProc         showErrorProc;       // function provided by host to show plugin error
  TDimTestAbortProc     testAbortProc;       // function provided by host to test if plugin should interrupt processing
  
  TDimMallocProc        mallocProc;          // function provided by host to allocate memory
  TDimFreeProc          freeProc;            // function provided by host to free memory

  // some standard parameters are defined here, any specific goes inside internalParams
  DIM_MAGIC_STREAM      *magic;
  DIM_FORMAT_ID         subFormat;           // sub format used to read or write, same as TDimFormatItem
  DIM_UINT32            pageNumber;          // page number to retreive/write, if number is invalid thenpage 0 is retreived
  DIM_UINT32            resolutionLevel;     // v1.1 resolution level retreive/write, considered 0 (highest) as default
  DIM_UCHAR             quality;             // number between 0 and 100 to specify quality (read/write)
  DIM_UINT32            compression;         // 0 - specify lossless method
  DIM_UINT32            order;               // progressive, normal, interlaced...
  DIM_UINT32            roiX;                // v1.1 Region Of Interest: Top Left X (pixels), ONLY used for reading!!!
  DIM_UINT32            roiY;                // v1.1 Region Of Interest: Top Left Y (pixels), ONLY used for reading!!!
  DIM_UINT32            roiW;                // v1.1 Region Of Interest: Width (pixels), ONLY used for reading!!!
  DIM_UINT32            roiH;                // v1.1 Region Of Interest: Height (pixels), ONLY used for reading!!!
  TDimTagList           metaData;            // meta data tag list, ONLY used for writing, returned NULL while reading!!!
  
  DIM_IMG_SERVICES      *imageServicesProcs; // The suite of image processing services callbacks  
  DIM_INTPARAMS_CLASS   *internalParams;     // internal plugin parameters
  
  DIM_PARENT_WIN_CLASS  *parent;             // pointer to parent window
  void                  *hDllInstance;       // handle to loaded dynamic library instance (plug-in)

  char                  *fileName;           // file name
  DIM_STREAM_CLASS      *stream;             // pointer to a file stream that might be open by host
  DIM_ImageIOModes      io_mode;             // v1.2 io mode for opened stream 
  // i/o callbacks
  TDimReadProc          readProc;            // v1.2 (fread)  read from stream
  TDimWriteProc         writeProc;           // v1.2 (fwrite) write into stream 
  TDimFlushProc         flushProc;           // v1.2 (flush) flushes a stream
  TDimSeekProc          seekProc;            // v1.2 (fseek)  seek within stream
  TDimSizeProc          sizeProc;            // v1.2 () get file "stream" size (libtiff askes for it)
  TDimTellProc          tellProc;            // v1.2 (ftell) gets the current position in stream
  TDimEofProc           eofProc;             // v1.2 (feof) tests for end-of-file on a stream
  TDimCloseProc         closeProc;           // v1.2 (fclose) close stream

  // IN/OUT
  DIM_IMAGE_CLASS       *image;              // pointer to an image structure to read/write
  DIM_OPTIONS_CLASS     *options;            // v1.6 pointer to encoding options string
  //DIM_METADATA_CLASS    *hash;               // v1.7 pointer to DTagMap for textual metadata

  void *param1; // reserved
  void *param2; // reserved
  void *param3; // reserved  

  char reserved[52];
  
} TDimFormatHandle;
//-----------------------------------------------------


//******************************************************************************
// plug-in provides to the host program
//******************************************************************************

//------------------------------------------------------------------------------
//  FORMAT
//------------------------------------------------------------------------------

// in all restrictions 0 means no restriction
typedef struct TDimFormatConstrains
{
  DIM_UINT32  maxWidth;
  DIM_UINT32  maxHeight;
  DIM_UINT32  maxPageNumber;
  DIM_UINT32  minSamplesPerPixel; // v1.3
  DIM_UINT32  maxSamplesPerPixel; // v1.3
  DIM_UINT32  minBitsPerSample;   // v1.3
  DIM_UINT32  maxBitsPerSample;   // v1.3
  DIM_BOOL    lutNotSupported;    // v1.3
} TDimFormatConstrains;

typedef struct TDimFormatItem
{
  char *formatNameShort; // short name, no spaces
  char *formatNameLong;  // Long format name
  char *extensions;      // pipe "|" separated supported extension list
  DIM_BOOL canRead;           // 0 - NO, 1 - YES v1.3
  DIM_BOOL canWrite;          // 0 - NO, 1 - YES v1.3
  DIM_BOOL canReadMeta;       // 0 - NO, 1 - YES v1.3
  DIM_BOOL canWriteMeta;      // 0 - NO, 1 - YES v1.3
  DIM_BOOL canWriteMultiPage; // 0 - NO, 1 - YES v1.3
  TDimFormatConstrains constrains; //  v1.3
  char reserved[17];
} TDimFormatItem;

typedef struct TDimFormatList
{
  DIM_UINT32        version;
  DIM_UINT32        count;
  TDimFormatItem    *item;
} TDimFormatList;

//------------------------------------------------------------------------------
// NULL for any of this function means unavailable function 
//   ex: only reader shuld return NULL for all reader functions
//------------------------------------------------------------------------------

// this wil return if image can be opened
typedef DIM_INT (*TDimValidateFormatProc) (DIM_MAGIC_STREAM *magic, DIM_UINT length);

//------------------------------------------------------------------------------
// FORMAT PARAMS and ALLOCATION
//------------------------------------------------------------------------------

// allocate space for parameters stuct and return it
typedef TDimFormatHandle (*TDimAquireFormatProc) ();

// delete parameters struct
typedef void (*TDimReleaseFormatProc) (TDimFormatHandle *fmtHndl);

// return dialog window class for parameter initialization or create the window by itself
typedef DIM_DIALOG_WIN_CLASS* (*TDimAquireIntParamsProc) (TDimFormatHandle *fmtHndl);

// load/save parameters into stream
typedef void (*TDimLoadFormatParamsProc)  (TDimFormatHandle *fmtHndl, DIM_PARAM_STREAM_CLASS *stream);
typedef void (*TDimStoreFormatParamsProc) (TDimFormatHandle *fmtHndl, DIM_PARAM_STREAM_CLASS *stream);

//------------------------------------------------------------------------------
// OPEN/CLOSE IMAGE PROCs
//------------------------------------------------------------------------------

typedef DIM_UINT (*TDimOpenImageProc)  (TDimFormatHandle *fmtHndl, DIM_ImageIOModes io_mode);
typedef DIM_UINT (*TDimFOpenImageProc) (TDimFormatHandle *fmtHndl, char* fileName, DIM_ImageIOModes io_mode);
typedef DIM_UINT (*TDimIOpenImageProc) (TDimFormatHandle *fmtHndl, char* fileName, 
                                         DIM_IMAGE_CLASS *image, DIM_ImageIOModes io_mode);
//typedef DIM_UINT (*TDimDOpenImageProc)  (TDimFormatHandle *fmtHndl, DIM_ImageIOModes io_mode);
//typedef DIM_UINT (*TDimCOpenImageProc)  (TDimFormatHandle *fmtHndl, DIM_ImageIOModes io_mode);

typedef void     (*TDimCloseImageProc) (TDimFormatHandle *fmtHndl);

//------------------------------------------------------------------------------
// IMAGE INFO PROCs
//------------------------------------------------------------------------------

// image info procs
typedef DIM_UINT      (*TDimGetNumPagesProc)  (TDimFormatHandle *fmtHndl);
// image info for each page proc
typedef TDimImageInfo (*TDimGetImageInfoProc) (TDimFormatHandle *fmtHndl, DIM_UINT page_num);


//------------------------------------------------------------------------------
// READ/WRITE PROCs
//------------------------------------------------------------------------------

// these are the actual read/write procs
typedef DIM_UINT (*TDimReadImageProc)  (TDimFormatHandle *fmtHndl, DIM_UINT page);
typedef DIM_UINT (*TDimWriteImageProc) (TDimFormatHandle *fmtHndl);

typedef DIM_UINT (*TDimReadImageTileProc)    (TDimFormatHandle *fmtHndl, DIM_ULONG tileId);
typedef DIM_UINT (*TDimWriteImageTileProc)   (TDimFormatHandle *fmtHndl, DIM_ULONG tileId);

typedef DIM_UINT (*TDimReadImageLineProc)    (TDimFormatHandle *fmtHndl, DIM_ULONG y);
typedef DIM_UINT (*TDimWriteImageLineProc)   (TDimFormatHandle *fmtHndl, DIM_ULONG y);

// difference with preview is that if there's a thumbnail in the image file
// then it will be upscaled/downscaled to meet w and h...
typedef DIM_UINT (*TDimReadImageThumbProc)   (TDimFormatHandle *fmtHndl, DIM_UINT w, DIM_UINT h);
typedef DIM_UINT (*TDimWriteImageThumbProc)  (TDimFormatHandle *fmtHndl);

// difference with thumbnail is that if there's a thumbnail in the image file
// smaller then needed preview then actual image will be downscaled or to meet w and h,
// if image contains Resolution Levels they'll be used to retreive optimum data
// roi will be used as weel if defined in fmtHndl, if W and H are defined as 0 
// then ROI will be extracted
typedef DIM_UINT (*TDimReadImagePreviewProc) (TDimFormatHandle *fmtHndl, DIM_UINT w, DIM_UINT h);


//------------------------------------------------------------------------------
// METADATA PROCs
//------------------------------------------------------------------------------

// if cannot read any meta then return value different form 0
// any possible group = -1, any possible tag = -1
// so to read all possible metadata pass -1 as group and tag
// if tag or group are provided but not present in the file then return 2
typedef DIM_UINT (*TDimReadMetaDataProc)    ( TDimFormatHandle *fmtHndl, DIM_UINT page, int group, int tag, int type);

// a simplistic approach to metadata retrieval, return null terminated string
// with possible new line caracters OR NULL if no data could be found
typedef char* (*TDimReadMetaDataAsTextProc) ( TDimFormatHandle *fmtHndl );

// if cannot write requested meta then return value different form 0
// this will only work on formats allowing adding meta data into the file
// usually it should writen using TDimWriteImageProc and meta data struct inside fmtHndl
typedef DIM_UINT (*TDimAddMetaDataProc)     ( TDimFormatHandle *fmtHndl);

// v1.7 New format metadata appending function, that should append fields 
// directly into provided HashTable, this allows performing all parsing on the reader side
class DTagMap;
typedef DIM_UINT (*TDimAppendMetaDataProc)  ( TDimFormatHandle *fmtHndl, DTagMap *hash);


//------------------------------------------------------------------------------
// *** HEADER
// structure that plug-in should provide for the host program
//------------------------------------------------------------------------------

typedef struct TDimFormatHeader {
  DIM_UINT  ver;          // must be == sizeof(TDimFormatHeader)
  char *version;     // plugin version, e.g. "1.0.0"
  char *name;        // plugin name
  char *description; // plugin description
  
  DIM_UINT                    neededMagicSize; // 0 or more, specify number of bytes needed to identify the file
  TDimFormatList              supportedFormats;

  TDimValidateFormatProc      validateFormatProc; // return TRUE if can handle file
  // begin
  TDimAquireFormatProc        aquireFormatProc;
  // end
  TDimReleaseFormatProc       releaseFormatProc;
  
  // params
  TDimAquireIntParamsProc     aquireParamsProc;
  TDimLoadFormatParamsProc    loadParamsProc;
  TDimStoreFormatParamsProc   storeParamsProc;

  // image begin
  TDimOpenImageProc           openImageProc; 
  TDimCloseImageProc          closeImageProc; 

  // info
  TDimGetNumPagesProc         getNumPagesProc;
  TDimGetImageInfoProc        getImageInfoProc;


  // read/write
  TDimReadImageProc           readImageProc; 
  TDimWriteImageProc          writeImageProc;
  TDimReadImageTileProc       readImageTileProc;
  TDimWriteImageTileProc      writeImageTileProc;
  TDimReadImageLineProc       readImageLineProc;
  TDimWriteImageLineProc      writeImageLineProc;
  TDimReadImageThumbProc      readImageThumbProc;
  TDimWriteImageThumbProc     writeImageThumbProc;
  TDimReadImagePreviewProc    readImagePreviewProc;
  
  // meta data
  TDimReadMetaDataProc        readMetaDataProc;
  TDimAddMetaDataProc         addMetaDataProc;
  TDimReadMetaDataAsTextProc  readMetaDataAsTextProc;
  TDimAppendMetaDataProc      appendMetaDataProc; // v1.7

  void *param1;       // reserved
  void *param2;       // reserved
  char reserved[100];
} TDimFormatHeader;


// -----------------------------------------------------------------------------
// function prototypes
// -----------------------------------------------------------------------------

// This functions must be exported so plug-in could be accepted as valid by host

// dimGetFormatHeader
typedef TDimFormatHeader* (*TDimGetFormatHeader)(void);



#endif //DIM_IMG_FMT_IFC_H


