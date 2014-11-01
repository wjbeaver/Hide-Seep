/* ZVI - Portable C++ library to read Zeiss ZVI image files

   Authors:

     Dmitry Fedorov 
     Copyright 2010 <www.bioimage.ucsb.edu> <www.dimin.net> 

     Michel Boudinot
     Copyright 2010 <Michel.Boudinot@inaf.cnrs-gif.fr>
  

   Version: 0.3

   Redistribution and use in source and binary forms, with or without 
   modification, are permitted provided that the following conditions 
   are met:
   * Redistributions of source code must retain the above copyright notice, 
     this list of conditions and the following disclaimer.
   * Redistributions in binary form must reproduce the above copyright notice, 
     this list of conditions and the following disclaimer in the documentation 
     and/or other materials provided with the distribution.
   * Neither the name of the authors nor the names of its contributors may be 
     used to endorse or promote products derived from this software without 
     specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
   ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE 
   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF 
   THE POSSIBILITY OF SUCH DAMAGE.

   ------------------------------------------------------------------------------
   classes for general usage:

   class Directory;
   Directory is probably the only class one should need, it parses the OLE storage 
   and identifies ZVI streams, reads them and parses, also provides access to pixels
   using CZT coordinates

   class Image;
   Image - parses Image_Item(X)_Contents and produces a map of image info 
   also allows dumping pixel data into a buffer

   class Metadata;
   Metadata - parses Image_Item(X)_Tags_Contents and produces a map of metadata

   class Scaling;
   Scaling - parses Image_Scaling_Contents and produces a map of metadata

   ===============================================================================

   Example using Directory class:

    POLE::Storage* storage = new POLE::Storage( filename );
    storage->open();
    if( storage->result() != POLE::Storage::Ok ) return;

    zvi::Directory zvi_dir(storage);
    if (!zvi_dir.isValid()) return;

    //read pixel data for C=0, Z=0 and T=0
    unsigned int w = zvi_dir.imageWidth();
    unsigned int h = zvi_dir.imageHeight();
    zvi::PIXEL_TYPES pf = zvi_dir.pixelFormat();
    unsigned int bpp = 8;
    if (pf == PT_UINT16) bpp = 16;
    if (pf == PT_UINT32) bpp = 32;
    // use all other pixel types here

    std::vector<unsigned char> img(w*h*(bpp/8));
    unsigned int c=0, z=0, t=0;
    zvi_dir.readImagePixels( c, z, t, img.size(), &img[0] );

    // access some metadata
    std::string date = zvi_dir.meta()->get_value("Acquisition Date");
    int wave         = zvi_dir.meta()->get_value_int("Emission Wavelength");
    double mag       = zvi_dir.meta()->get_value_double("Objective Magnification");
*/

#ifndef BIO_IMAGE_ZVI_H
#define BIO_IMAGE_ZVI_H

#include <string>
#include <list>
#include <vector>
#include <map>

#include <tag_map.h>

#include <pole.h>

namespace bim {

  typedef char int8;
  typedef short int16;
  typedef unsigned char uint8;
  typedef unsigned short uint16;

#if defined WIN32 || defined WIN64 || defined _WIN32 || defined _WIN64
  typedef __int32 int32;
  typedef __int64 int64;
  typedef unsigned __int32 uint32;
  typedef unsigned __int64 uint64;
#else
  typedef int int32;
  typedef long long int64;
  typedef unsigned int uint32;
  typedef unsigned long long uint64;
#endif

} // namespace bim


namespace zvi {

//---------------------------------------------------------------------------------
// ZVI Entry Types
//---------------------------------------------------------------------------------

enum VAR_TYPES {
  VT_EMPTY= 0,			  // (*) nothing
  VT_NULL	= 1,	    	// (*) nothing
  VT_I2	= 2,		     	// short
  VT_I4	= 3,		     	// int
  VT_R4	= 4,	      	// float
  VT_R8	= 5,	      	// double
  VT_CY	= 6,	      	// currency value
  VT_DATE	= 7,		  	// (*) date value (OLEDate)
  VT_BSTR	= 8,		  	// (*) bstring (unsigned long len_in_bytes, short[len/2])
  VT_DISPATCH	= 9,	  // (*) 16 bytes
  VT_ERROR	= 10,
  VT_BOOL	    = 11,		// 2 bytes
  VT_VARIANT	= 12,
  VT_UNKNOWN	= 13,		// 16 bytes
  VT_DECIMAL	= 14,
  VT_I1	= 16,	    		// signed char
  VT_UI1	= 17,		  	// unsigned char
  VT_UI2	= 18,		  	// unsigned short
  VT_UI4	= 19,		  	// unsigned long
  VT_I8	= 20,		    	// 8 bytes
  VT_UI8	= 21,		   	// unsigned 8 bytes
  VT_INT	= 22,  
  VT_UINT	= 23,		   	// unsigned int
  VT_VOID	= 24,
  VT_HRESULT	= 25,
  VT_PTR	    = 26,
  VT_SAFEARRAY	= 27,
  VT_CARRAY	    = 28,
  VT_USERDEFINED	= 29,
  VT_LPSTR	= 30,
  VT_LPWSTR	= 31,
  VT_RECORD	= 36,
  VT_FILETIME	= 64,
  VT_BLOB	    = 65,			    // (*) bstring (unsigned long len_in_bytes, short[len/2])
  VT_STREAM	= 66,
  VT_STORAGE	= 67,		
  VT_STREAMED_OBJECT	= 68, // (*) bstring (unsigned long len_in_bytes, short[len/2])
  VT_STORED_OBJECT	= 69,	  // (*) bstring (unsigned long len_in_bytes, short[len/2])
  VT_BLOB_OBJECT	    = 70,	// (*) bstring (unsigned long len_in_bytes, short[len/2])
  VT_CF	    = 71,
  VT_CLSID	= 72,
  VT_BSTR_BLOB	= 0xfff,
  VT_VECTOR	= 0x1000,
  VT_ARRAY	= 0x2000,       // (*) (unsigned short len_in_bytes, long[len])
  VT_BYREF	= 0x4000,
  VT_RESERVED	= 0x8000,
  VT_ILLEGAL	= 0xffff,
  VT_ILLEGALMASKED	= 0xfff,
  VT_TYPEMASK	= 0xfff
};

enum PIXEL_TYPES {
  PT_RGB8    = 1, // 8-bit RGB Triple (B, G, R)
  PT_RGBA8   = 2, // 8-bit RGB Quad (B, G, R, A)
  PT_GRAY8   = 3, // 8-bit grayscale
  PT_UINT16  = 4, // 16-bit unsigned integer
  PT_UINT32  = 5, // 32-bit unsigned integer
  PT_FLOAT32 = 6, // 32-bit IEEE float
  PT_FLOAT64 = 7, // 64-bit IEEE float
  PT_RGB16   = 8, // 16-bit unsigned RGB Triple (B, G, R)
  PT_RGB32   = 9  // 32-bit RGB Triple (B, G, R)
}; 

//---------------------------------------------------------------------------------
// BaseAccessor - provides ways of accessing ZVI variables in streams
// the BaseAccessor must be inherited by actual types, can be found in cpp
//---------------------------------------------------------------------------------

class BaseAccessor {
public:
  BaseAccessor(): type(VT_ILLEGAL), size(0) {} 

  // the following two methods operate on stream after reading the type of the incoming variable
  virtual bool skip_value( POLE::Stream *s ) const;
  virtual bool read_value( POLE::Stream *s, std::vector<unsigned char> *v ) const;
  virtual std::string toString(const std::vector<unsigned char> *v, unsigned int offset=0) const = 0;

public:
  // the following two methods read type and make sure it's compatible
  bool skip( POLE::Stream *s ) const;
  bool read( POLE::Stream *s, std::vector<unsigned char> *v ) const;

protected:
  bool ensureType(POLE::Stream *s) const;

protected:
  bim::uint16 type;
  unsigned int size;
};

//---------------------------------------------------------------------------------
// Accessors - gives static access to all accessors per type of ZVI data
//---------------------------------------------------------------------------------

class Accessors {

public:
  Accessors() { init(); }
  static const BaseAccessor* get(bim::uint16 type);

private: 
  static std::map< bim::uint16, BaseAccessor* > accessors;
  static void init();
};

//---------------------------------------------------------------------------------
// Entry - description of the variable, used in a list of variables to provide
// structured access to vars in the streams
// Also used for reading any value
//---------------------------------------------------------------------------------

class Entry {
public:
  Entry(const std::string &n, const bim::uint16 &t=VT_ILLEGAL, const bim::uint32 &_offset=0);

  bim::uint16 getType() const { return type; }
  std::string getName() const { return name; }
  bim::uint32 getOffset() const { return offset; }
  const std::vector<unsigned char> *getValue() const { return &value; }

  void setType(bim::uint16 t) { type=t; }
  void setName(const std::string &s) { name=s; }
  void setOffset(bim::uint32 o) { offset=o; }

public:
  bool read( POLE::Stream *s );
  bool skip( POLE::Stream *s );
  std::string toString() const;
  bim::uint16 toUInt16() const;
  bim::uint32 toUInt32() const;
  bim::uint64 toUInt64() const;

protected:
  bim::uint16 type;
  std::string name;
  bim::uint32 offset; // needed for parsing blobs of data

  std::vector<unsigned char> value;
};

//---------------------------------------------------------------------------------
// Stream - the ZVI stream provides means of reading structured variable lists
// from streams and from blocks of data red from the stream
//---------------------------------------------------------------------------------

// ZVI::Stream provides additional methods to read specific ZVI fileds from the stream
class Stream {

public:
  Stream( POLE::Stream *stream ) { stream_ = stream; local_stream=false; }
  Stream( POLE::Storage *storage, const std::string &stream_name );
  ~Stream();

  POLE::Stream *stream() { return stream_; }
  bool isValid() const;

  bim::uint16 currentType();
  bim::uint16 seekToNext();
  bool findNextType( const bim::uint16 &type );

  bool readParseValues( const std::list<Entry> &defs, DTagMap *info );
  bool readParseBlock( const bim::uint16 &vt, const std::list<Entry> &defs, DTagMap *info );

private: 
  POLE::Stream *stream_;
  bool local_stream;

  bool readNextValue( const Entry &e, DTagMap *info );
};

//---------------------------------------------------------------------------------
// Image - parses Image_Item(X)_Contents and produces a map of image info 
// also allows dumping pixel data into a buffer
//---------------------------------------------------------------------------------

class Image {
public:
  Image();
  Image(Stream *s);
  Image(POLE::Storage *s, unsigned int image_id);

  bool isValid() const;

  bool fromStream( Stream *s );
  bool fromStorage( POLE::Storage *s, unsigned int image_id );
  bool readImagePixels( Stream *s, const unsigned int &buf_size, unsigned char *buf );

public:
  const DTagMap *info() { return &image_info; }

protected:
  bim::uint32 data_offset;
  DTagMap image_info;
  void init();

  std::list<Entry> ImageItemContentsHeader;
  std::list<Entry> CoordinateBlock;
  std::list<Entry> RawImageDataHeader;
};

//---------------------------------------------------------------------------------
// Metadata - parses Image_Item(X)_Tags_Contents and produces a map of metadata
//---------------------------------------------------------------------------------

class Metadata {
public:
  Metadata();
  Metadata(Stream *s);
  Metadata(POLE::Storage *s);

  bool isValid() const;

  bool fromStream( Stream *s, bool rootStorage=false );
  bool fromStorage( POLE::Storage *s );

public:
  const DTagMap *meta() { return &tags; }

protected:
  DTagMap tags;
  std::string parseTagValue( bim::int32 tag, const Entry &val );

  std::list<Entry> TagsContentsHeader;
  //std::list<Entry> TagBlock; // used Entry directly, so not needed now
  void init();
};


//---------------------------------------------------------------------------------
// Scaling - parses Image_Scaling_Contents and produces a map of metadata
//---------------------------------------------------------------------------------

class Scaling {
public:
  Scaling();
  Scaling(Stream *s);
  Scaling(POLE::Storage *s);

  bool isValid() const;

  bool fromStream( Stream *s );
  bool fromStorage( POLE::Storage *s );

public:
  const DTagMap *meta() { return &tags; }

protected:
  DTagMap tags;

  std::list<Entry> ScalingContentsHeader;
  void init();
};


//---------------------------------------------------------------------------------
// PlaneStreamMap - provides a fast retreival of a stream name for a 5D plane: CZT
// this is amisc class for Directory
//---------------------------------------------------------------------------------

class PlaneStreamMap : public DTagMap {
public:
  // constructors
  explicit PlaneStreamMap(): DTagMap() {}
  PlaneStreamMap( const std::map<std::string, std::string>& _Right ): DTagMap(_Right) {}

  template<class InputIterator>
  PlaneStreamMap( InputIterator _First, InputIterator _Last ): DTagMap(_First, _Last) {}

public:

  std::string get_stream_name( const unsigned int &c, const unsigned int &z, const unsigned int &t ) const;

  void set_stream_name( const unsigned int &c, const unsigned int &z, const unsigned int &t, const std::string &stream_name );

protected:
  inline std::string create_key( const unsigned int &c, const unsigned int &z, const unsigned int &t ) const;
};


//---------------------------------------------------------------------------------
// Directory - parses the OLE storage and identifies all ZVI streams, ready to 
// read actual data and identify if the storage actually contains ZVI
//---------------------------------------------------------------------------------

class Directory {
public:
  Directory();
  Directory(POLE::Storage *s);

  static bool isValid(POLE::Storage *s); // tests if the supplied storage actually contains a ZVI file
  bool isValid() const; // after loading tests if the ZVI was properly loaded
  bool fromStorage( POLE::Storage *s );

public:
  inline const DTagMap *info()       const { return &image_info; }
  inline const DTagMap *meta()       const { return &tags; }
  inline const DTagMap *meta_scale() const { return &scaling; }

  inline unsigned int channels()     const { return number_c; }
  inline unsigned int zPlanes()      const { return number_z; }
  inline unsigned int timePoints()   const { return number_t; }
  inline unsigned int pages()        const { return number_p; }

  inline unsigned int imageWidth()   const { return width; }
  inline unsigned int imageHeight()  const { return height; }
  inline PIXEL_TYPES  pixelFormat()  const { return (PIXEL_TYPES) pixel_format; }
  unsigned int pixelBitDepth() const;

  bool readImagePixels( const unsigned int &c, const unsigned int &z, const unsigned int &t, const unsigned int &buf_size, unsigned char *buf );

protected:
  unsigned int number_c, number_z, number_t, number_p;
  unsigned int width, height, pixel_format;

  POLE::Storage* storage;
  PlaneStreamMap planes;
  std::map<int, int> sequenced_to_zvi_channel;

  DTagMap image_info; // some parsed fileds from different places containing required info for reading
  DTagMap tags; // contents of /Tags
  DTagMap scaling; // contents of /Image/Scaling/Contents
  DTagMap scalingTags; // contents of /Image/Scaling/Tags/Contents

protected:
  bool parseEmbeddedMetadata();
  bool parseImageGeometry();
  bool parseCreateMetadata();
  void init();
};

} // namespace ZVI

#endif // BIO_IMAGE_ZVI_H
