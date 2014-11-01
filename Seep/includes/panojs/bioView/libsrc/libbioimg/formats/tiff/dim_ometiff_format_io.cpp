/*****************************************************************************
  OME-TIFF definitions 
  Copyright (c) 2009, Center for Bio-Image Informatics, UCSB
 
  Author: Dima V. Fedorov <mailto:dima@dimin.net> <http://www.dimin.net/>
    
  History:
    2009-07-09 12:01 - First creation

  Ver : 1
*****************************************************************************/

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <algorithm>

#include <iostream>
#include <fstream>

#include <xstring.h>
#include <tag_map.h>

#include "dim_tiff_format.h"

//----------------------------------------------------------------------------
// OME-TIFF MISC FUNCTIONS
//----------------------------------------------------------------------------

bool omeTiffIsValid(DTiffParams *par) {
  if (!par) return false;
  if (par->dimTiff->tif_flags&TIFF_BIGTIFF) return false;        
  TDimTiffIFD *ifd = &par->ifds.ifds[0];
  TIFF *tif = par->dimTiff;
  if (!tif) return false;
  if (!ifd) return false;

  if (!isTagPresentInFirstIFD( &par->ifds, TIFFTAG_IMAGEDESCRIPTION )) return false;
  xstring tag_270 = read_tag_as_string(tif, ifd, TIFFTAG_IMAGEDESCRIPTION );
  if ( tag_270.contains("<OME") && tag_270.contains("<Image") && tag_270.contains("<Pixels") ) return true;

  return false;
}

xstring ometiff_normalize_xml_spaces( const xstring &s ) {
  xstring o = s;
  xstring::size_type b=0;

  while (b != std::string::npos) {
    b = o.find ( "=" , b );
    if (b != std::string::npos ) {
      while (o[b+1] == ' ')
        o = o.erase(b+1, 1);

      while (b>0 && o[b-1] == ' ') {
        o = o.erase(b-1, 1);
        --b;
      }

      ++b;
    }
  }

  return o;
}

int omeTiffGetInfo (DTiffParams *par) {
  if (!par) return 1;
  if (!par->dimTiff) return 1;
  if (par->ifds.count <= 0) return 1;

  TDimTiffIFD *ifd = &par->ifds.ifds[0];
  TIFF *tif = par->dimTiff;
  if (!tif) return false;
  if (!ifd) return false;

  TDimImageInfo *info = &par->info;
  DOMETiffInfo *ome = &par->omeTiffInfo;

  // Read OME-XML from image description tag
  if (!isTagPresentInFirstIFD( &par->ifds, TIFFTAG_IMAGEDESCRIPTION )) return false;
  xstring tag_270 = read_tag_as_string(tif, ifd, TIFFTAG_IMAGEDESCRIPTION );
  if (tag_270.size()<=0) return false;

  //---------------------------------------------------------------
  // image geometry
  //---------------------------------------------------------------
  xstring tag_pixels = tag_270.section("<Pixels", ">");
  if (tag_pixels.size()<=0) return false;
  tag_pixels = ometiff_normalize_xml_spaces( tag_pixels );

  unsigned int real_tiff_pages = info->number_pages;
  info->samples  = tag_pixels.section(" SizeC=\"", "\"").toInt(1);
  info->number_t = tag_pixels.section(" SizeT=\"", "\"").toInt(1);
  info->number_z = tag_pixels.section(" SizeZ=\"", "\"").toInt(1);
  info->width    = tag_pixels.section(" SizeX=\"", "\"").toInt(1);
  info->height   = tag_pixels.section(" SizeY=\"", "\"").toInt(1);
  ome->dim_order = tag_pixels.section(" DimensionOrder=\"", "\"");
  ome->pages     = info->number_t*info->number_z;
  info->number_pages = ome->pages;
  ome->channels = info->samples;

  if (info->samples > 1) 
    info->imageMode = DIM_RGB;
  else
    info->imageMode = DIM_GRAYSCALE;    

  uint16 bitspersample = 1;  
  TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &bitspersample);  
  info->depth = bitspersample;

  //---------------------------------------------------------------
  // fix for the case of multiple files, with many channels defined 
  // but not provided in the particular TIFF
  //---------------------------------------------------------------
  uint16 samplesperpixel = 1;
  TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &samplesperpixel);
  if (samplesperpixel == 1) {
    // ok, test if number of channels is correct given the number of pages
    if (real_tiff_pages < ome->pages * ome->channels) {
      info->samples = 1;
      ome->channels = 1;
    }

    if (real_tiff_pages < ome->pages) {
      ome->pages = real_tiff_pages;
      info->number_pages = real_tiff_pages;
    }

    if (info->number_t*info->number_z < ome->pages) {
      if (info->number_z > info->number_t) 
        info->number_z = ome->pages;
      else
        info->number_t = ome->pages;
    }
  }



  //---------------------------------------------------------------
  // define dims
  //---------------------------------------------------------------
  if (info->number_z > 1) {
    info->number_dims = 4;
    info->dimensions[3].dim = DIM_DIM_Z;
  }
  if (info->number_t > 1) {
    info->number_dims = 4;
    info->dimensions[3].dim = DIM_DIM_T;
  }
  if (info->number_z>1 && info->number_t>1) {
    info->number_dims = 5;
    info->dimensions[3].dim = DIM_DIM_Z;        
    info->dimensions[4].dim = DIM_DIM_T;
  }

  //--------------------------------------------------------------------  
  // pixel resolution
  //--------------------------------------------------------------------

  ome->pixel_resolution[0] = tag_pixels.section(" PhysicalSizeX=\"", "\"").toDouble(0.0);
  ome->pixel_resolution[1] = tag_pixels.section(" PhysicalSizeY=\"", "\"").toDouble(0.0);
  ome->pixel_resolution[2] = tag_pixels.section(" PhysicalSizeZ=\"", "\"").toDouble(0.0);
  ome->pixel_resolution[3] = tag_pixels.section(" TimeIncrement=\"", "\"").toDouble(0.0);

  // Fix for old style OME-TIFF images
  xstring tag_image = tag_270.section("<Image", ">");
  if (tag_image.size()>0) {
    tag_image = ometiff_normalize_xml_spaces( tag_image );
    
    if (ome->pixel_resolution[0]==0.0)
      ome->pixel_resolution[0] = tag_image.section(" PixelSizeX=\"", "\"").toDouble(0.0);
    if (ome->pixel_resolution[1]==0.0)
      ome->pixel_resolution[1] = tag_image.section(" PixelSizeY=\"", "\"").toDouble(0.0);
    if (ome->pixel_resolution[2]==0.0)
      ome->pixel_resolution[2] = tag_image.section(" PixelSizeZ=\"", "\"").toDouble(0.0);
    if (ome->pixel_resolution[3]==0.0)
      ome->pixel_resolution[3] = tag_image.section(" PixelSizeT=\"", "\"").toDouble(0.0);
  }

  //std::ofstream myfile;
  //myfile.open ("D:\\dima_media\\images\\_BIO\\_dima_ome_tiff\\oib.xml");
  //myfile << tag_270;
  //myfile.close();  

  return 0;
}

void omeTiffGetCurrentPageInfo(DTiffParams *par) {
  if (!par) return;
  TDimImageInfo *info = &par->info;
  DOMETiffInfo *ome = &par->omeTiffInfo;

  info->samples = ome->channels;
  info->number_pages = ome->pages;

  info->resUnits = DIM_RES_um;
  info->xRes = ome->pixel_resolution[0];
  info->yRes = ome->pixel_resolution[1];
}


//----------------------------------------------------------------------------
// READ/WRITE FUNCTIONS
//----------------------------------------------------------------------------

int computeTiffDirectory( TDimFormatHandle *fmtHndl, int page, int sample ) {
  DTiffParams *par = (DTiffParams *) fmtHndl->internalParams;
  TDimImageInfo *info = &par->info;
  DOMETiffInfo *ome = &par->omeTiffInfo;

  int nz = std::max<unsigned int>(info->number_z, 1);
  int nt = std::max<unsigned int>(info->number_t, 1);
  int nc = std::max<unsigned int>(info->samples, 1);

  //XYLZT file will be named as “xxx_C00mL00qZ00nT00p.tif”
  int c = sample;
  int l = (page/nz)/nt;
  int t = (page - l*nt*nz) / nz;
  int z = page - nt*l - nz*t;

  // compute directory position based on order: XYCZT
  int dirNum = (t*nz + z)*nc + c;

  // Possible orders: XYCZT XYCTZ XYZCT XYZTC XYTCZ XYTZC
  if (ome->dim_order.startsWith("XYCZ") ) dirNum = (t*nz + z)*nc + c;
  if (ome->dim_order.startsWith("XYCT") ) dirNum = (z*nt + t)*nc + c;
  if (ome->dim_order.startsWith("XYZC") ) dirNum = (t*nc + c)*nz + z;
  if (ome->dim_order.startsWith("XYZT") ) dirNum = (c*nt + t)*nz + z;
  if (ome->dim_order.startsWith("XYTC") ) dirNum = (z*nc + c)*nt + t;
  if (ome->dim_order.startsWith("XYTZ") ) dirNum = (t*nz + z)*nt + t;

  return dirNum;
}

// this is here due to some OME-TIFF do not conform with the standard and come with all channels in the same IFD
int computeTiffDirectoryNoChannels( TDimFormatHandle *fmtHndl, int page, int sample ) {
  DTiffParams *par = (DTiffParams *) fmtHndl->internalParams;
  TDimImageInfo *info = &par->info;
  DOMETiffInfo *ome = &par->omeTiffInfo;

  int nz = std::max<unsigned int>(info->number_z, 1);
  int nt = std::max<unsigned int>(info->number_t, 1);
  int nc = 1;

  //XYLZT file will be named as “xxx_C00mL00qZ00nT00p.tif”
  int c = 0;
  int l = (page/nz)/nt;
  int t = (page - l*nt*nz) / nz;
  int z = page - nt*l - nz*t;

  // compute directory position based on order: XYCZT
  int dirNum = (t*nz + z)*nc + c;

  // Possible orders: XYCZT XYCTZ XYZCT XYZTC XYTCZ XYTZC
  if (ome->dim_order.startsWith("XYCZ") ) dirNum = (t*nz + z)*nc + c;
  if (ome->dim_order.startsWith("XYCT") ) dirNum = (z*nt + t)*nc + c;
  if (ome->dim_order.startsWith("XYZC") ) dirNum = (t*nc + c)*nz + z;
  if (ome->dim_order.startsWith("XYZT") ) dirNum = (c*nt + t)*nz + z;
  if (ome->dim_order.startsWith("XYTC") ) dirNum = (z*nc + c)*nt + t;
  if (ome->dim_order.startsWith("XYTZ") ) dirNum = (t*nz + z)*nt + t;

  return dirNum;
}


DIM_UINT omeTiffReadPlane( TDimFormatHandle *fmtHndl, DTiffParams *par, int plane ) {
  if (!par) return 1;
  if (!par->dimTiff) return 1;
  if (par->subType != tstOmeTiff) return 1;  
  if (par->ifds.count <= 0) return 1;

  TDimTiffIFD *ifd = &par->ifds.ifds[0];
  TIFF *tif = par->dimTiff;
  if (!tif) return false;
  if (!ifd) return false;

  TDimImageInfo *info = &par->info;
  DOMETiffInfo *ome = &par->omeTiffInfo;
  
  //--------------------------------------------------------------------  
  // read image parameters
  //--------------------------------------------------------------------
  int tiff_page = computeTiffDirectory( fmtHndl, fmtHndl->pageNumber, 0 );
  TDimImageBitmap *img = fmtHndl->image;
  TIFFSetDirectory( tif, tiff_page );

  uint16 bitspersample = 1;
  uint32 height = 0; 
  uint32 width = 0; 
  uint16 samplesperpixel = 1;
  uint16 sampleformat = 1;

  TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
  TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);
  TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &samplesperpixel);
  TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &bitspersample);
  TIFFGetField(tif, TIFFTAG_SAMPLEFORMAT, &sampleformat);

  if( TIFFIsTiled(tif) ) return 1;

  if (samplesperpixel > 1) {
    tiff_page = computeTiffDirectoryNoChannels( fmtHndl, fmtHndl->pageNumber, 0 );
    TIFFSetDirectory( tif, tiff_page );
    return 2;
  }

  if (img->i.depth != bitspersample || img->i.width != width || img->i.height != height) {
    //info->samples = ome->ch;
    info->depth = bitspersample;
    info->width = width;
    info->height = height;
    info->pixelType = D_FMT_UNSIGNED;
    if (sampleformat == SAMPLEFORMAT_INT)
      info->pixelType = D_FMT_SIGNED;
    else
    if (sampleformat == SAMPLEFORMAT_IEEEFP)
      info->pixelType = D_FMT_FLOAT;

    if ( allocImg( fmtHndl, info, img) != 0 ) return 1;
  }


  //--------------------------------------------------------------------
  // read data
  //--------------------------------------------------------------------
  DIM_UINT lineSize = getLineSizeInBytes( img );
  for (unsigned int sample=0; sample<(unsigned int)info->samples; ++sample) {

    tiff_page = computeTiffDirectory( fmtHndl, fmtHndl->pageNumber, sample );
    TIFFSetDirectory( tif, tiff_page );

    // small safeguard
    TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
    TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);
    TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &bitspersample);
    if (img->i.depth != bitspersample || img->i.width != width || img->i.height != height) continue;

    DIM_UCHAR *p = (DIM_UCHAR *) img->bits[ sample ];
    register DIM_UINT y = 0;

    for(y=0; y<img->i.height; y++) {
      dimProgress( fmtHndl, y*(sample+1), img->i.height*img->i.samples, "Reading OME-TIFF" );
      if ( dimTestAbort( fmtHndl ) == 1) break;  
      TIFFReadScanline(tif, p, y, 0);
      p += lineSize;
    } // for y
  }  // for sample

  //TIFFSetDirectory(tif, fmtHndl->pageNumber);
  return 0;
}

//----------------------------------------------------------------------------
// Metadata hash
//----------------------------------------------------------------------------

DIM_UINT append_metadata_omeTiff (TDimFormatHandle *fmtHndl, DTagMap *hash ) {

  if (!fmtHndl) return 1;
  if (!fmtHndl->internalParams) return 1;
  if (!hash) return 1;

  DTiffParams *par = (DTiffParams *) fmtHndl->internalParams;
  TDimImageInfo *info = &par->info;
  DOMETiffInfo *ome = &par->omeTiffInfo;


  hash->append_tag( "image_num_z", info->number_z );
  hash->append_tag( "image_num_t", info->number_t );
  hash->append_tag( "image_num_c", ome->channels );

  //----------------------------------------------------------------------------
  // DIMENSIONS
  //----------------------------------------------------------------------------
  hash->append_tag( "pixel_resolution_x", ome->pixel_resolution[0] );
  hash->append_tag( "pixel_resolution_unit_x", "microns" );
  hash->append_tag( "pixel_resolution_y", ome->pixel_resolution[1] );
  hash->append_tag( "pixel_resolution_unit_y", "microns" );
  hash->append_tag( "pixel_resolution_z", ome->pixel_resolution[2] );
  hash->append_tag( "pixel_resolution_unit_z", "microns" );
  hash->append_tag( "pixel_resolution_t", ome->pixel_resolution[3] );
  hash->append_tag( "pixel_resolution_unit_t", "seconds" );

  //----------------------------------------------------------------------------
  // Reading OME-TIFF tag
  //----------------------------------------------------------------------------

  TDimTiffIFD *ifd = &par->ifds.ifds[0];
  TIFF *tif = par->dimTiff;
  xstring tag_270 = read_tag_as_string(tif, ifd, TIFFTAG_IMAGEDESCRIPTION);
  if (tag_270.size()<=0) return 0;
  hash->append_tag( "raw/ome-tiff", tag_270 );

  //----------------------------------------------------------------------------
  // Channel names and preferred mapping
  //----------------------------------------------------------------------------
  for (unsigned int i=0; i<ome->channels; ++i) {
    xstring tag = xstring::xprintf("<LightSource ID=\"LightSource:%d\">", i);
    std::string::size_type p = tag_270.find(tag);
    if (p == std::string::npos) continue;
    xstring tag_laser = tag_270.section("<Laser", ">", p);
    if (tag_laser.size()<=0) continue;
    xstring medium = tag_laser.section(" LaserMedium=\"", "\"");
    if (medium.size()<=0) continue;
    xstring wavelength = tag_laser.section(" Wavelength=\"", "\"");
    if (wavelength.size()>0)
      hash->append_tag( xstring::xprintf("channel_%d_name", i), medium+" - "+wavelength+"nm" );
    else
      hash->append_tag( xstring::xprintf("channel_%d_name", i), medium );
  }

  // channel names may also be stored in Logical channel in the Image
  std::string::size_type p = tag_270.find("<LogicalChannel ");
  if (p != std::string::npos)
  for (unsigned int i=0; i<ome->channels; ++i) {

    //if (p == std::string::npos) continue;
    xstring tag = tag_270.section("<LogicalChannel", ">", p);
    if (tag.size()<=0) continue;
    tag = ometiff_normalize_xml_spaces( tag );
    xstring medium = tag.section(" Name=\"", "\"");
    if (medium.size()<=0) continue;
    xstring wavelength = tag.section(" ExWave=\"", "\"");
    int chan = i;
    p = tag_270.find("<ChannelComponent", p);
    tag = tag_270.section("<ChannelComponent", ">", p);
    if (tag.size()>0) {
      tag = ometiff_normalize_xml_spaces( tag );
      xstring index = tag.section(" Index=\"", "\"");
      chan = index.toInt(i);
    }

    if (wavelength.size()>0)
      hash->append_tag( xstring::xprintf("channel_%d_name", chan), medium+" - "+wavelength+"nm" );
    else
      hash->append_tag( xstring::xprintf("channel_%d_name", chan), medium );

    p += 15;
  }



  // the preferred mapping seems to be the default order
  /*
  if ( fvi->display_lut.size() == 3 ) {
    hash->append_tag( "display_channel_red",   fvi->display_lut[0] );
    hash->append_tag( "display_channel_green", fvi->display_lut[1] );
    hash->append_tag( "display_channel_blue",  fvi->display_lut[2] );
  }
  */

  //----------------------------------------------------------------------------
  // stage position
  //----------------------------------------------------------------------------
  p = tag_270.find("<Plane ");
  while (p != std::string::npos) {
    xstring tag = tag_270.section("<Plane", ">", p); 
    if (tag.size()>0) {
      tag = ometiff_normalize_xml_spaces( tag );
      int c = tag.section(" TheC=\"", "\"").toInt(0);
      int t = tag.section(" TheT=\"", "\"").toInt(0);
      int z = tag.section(" TheZ=\"", "\"").toInt(0);
      
      tag = tag_270.section("<StagePosition", ">", p); 
      if (tag.size()>0) {
        tag = ometiff_normalize_xml_spaces( tag );
        double sx = tag.section(" PositionX=\"", "\"").toDouble(0);
        double sy = tag.section(" PositionY=\"", "\"").toDouble(0);
        double sz = tag.section(" PositionZ=\"", "\"").toDouble(0);
        int page = t*info->number_z + z;
        hash->append_tag( xstring::xprintf( "stage_position_x/%d", page), sx );
        hash->append_tag( xstring::xprintf( "stage_position_y/%d", page), sy );
        hash->append_tag( xstring::xprintf( "stage_position_z/%d", page), sz );
      }

    }
    p = tag_270.find("<Plane ", p+5);
  }


  //----------------------------------------------------------------------------
  // more stuff
  //----------------------------------------------------------------------------
  xstring tag = tag_270.section("<CreationDate>", "</CreationDate>"); 
  if (tag.size()>=19) {
    tag[10] = ' ';
    hash->append_tag( "date_time", tag );
  }

  p = tag_270.find("<Instrument ID=\"Instrument:0\">");
  
  xstring tag_objective = tag_270.section("<Objective", ">", p);
  tag_objective = ometiff_normalize_xml_spaces( tag_objective );
  if (tag_objective.size()>0) {
    xstring model = tag_objective.section(" Model=\"", "\"");
    if (model.size()>0)
      hash->append_tag( "objective", model );
  }

  xstring tag_magnification = tag_270.section("<NominalMagnification>", "</NominalMagnification>", p);
  if (tag_magnification.size()>0)
    hash->append_tag( "magnification", tag_magnification + "X" );


  //----------------------------------------------------------------------------
  // read all custom attributes
  //----------------------------------------------------------------------------

  p = tag_270.find("<CustomAttributes");
  p = tag_270.find("<OriginalMetadata", p);
  xstring tag_original_meta = tag_270.section("<OriginalMetadata", ">", p);  
  while (tag_original_meta.size()>0) {
    tag_original_meta = ometiff_normalize_xml_spaces( tag_original_meta );
    xstring name = tag_original_meta.section(" Name=\"", "\"");
    xstring val = tag_original_meta.section(" Value=\"", "\"");
    if (name.size()>0 && val.size()>0) {
      // replace all / here with some other character
      hash->append_tag( "custom/"+name, val );
    }
    p += tag_original_meta.size();
    tag_original_meta = tag_270.section("<OriginalMetadata", ">", p);
  }



  return 0;
}


//----------------------------------------------------------------------------
// Write METADATA
//----------------------------------------------------------------------------

std::string omeTiffPixelType( TDimImageBitmap *img ) {
  std::string pt = "Uint8";
  if (img->i.depth==16 && img->i.pixelType==D_FMT_UNSIGNED) pt = "Uint16";
  if (img->i.depth==32 && img->i.pixelType==D_FMT_UNSIGNED) pt = "Uint32";
  if (img->i.depth==8  && img->i.pixelType==D_FMT_SIGNED) pt = "int8";
  if (img->i.depth==16 && img->i.pixelType==D_FMT_SIGNED) pt = "int16";
  if (img->i.depth==32 && img->i.pixelType==D_FMT_SIGNED) pt = "int32";
  if (img->i.depth==32 && img->i.pixelType==D_FMT_FLOAT)  pt = "float";
  if (img->i.depth==64 && img->i.pixelType==D_FMT_FLOAT)  pt = "double"; 
  return pt;
}

std::string constructOMEXML( TDimFormatHandle *fmtHndl, DTagMap *hash ) {
  TDimImageBitmap *img = fmtHndl->image; 
  
  // Header
  std::string str = "<?xml version=\"1.0\" encoding=\"UTF-8\"?><!-- Warning: this comment is an OME-XML metadata block, which contains crucial dimensional parameters and other important metadata. Please edit cautiously (if at all), and back up the original data before doing so. For more information, see the OME-TIFF web site: http://loci.wisc.edu/ome/ome-tiff.html. --><OME xmlns=\"http://www.openmicroscopy.org/XMLschemas/OME/FC/ome.xsd\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:schemaLocation=\"http://www.openmicroscopy.org/XMLschemas/OME/FC/ome.xsd http://www.openmicroscopy.org/XMLschemas/OME/FC/ome.xsd\">";

  // Image tag
  str += xstring::xprintf("<Image ID=\"openmicroscopy.org:Image:1\" Name=\"%s\" DefaultPixels=\"openmicroscopy.org:Pixels:1-1\">", "libbioimage" );
  //str += "<CreationDate>2007-11-08T14:52:40</CreationDate>";
  
  str += "<Pixels ID=\"openmicroscopy.org:Pixels:1-1\"";
  str += " DimensionOrder=\"XYCZT\"";
  str += xstring::xprintf(" PixelType=\"%s\"", omeTiffPixelType(img).c_str() );
  str += " BigEndian=\"false\""; 
  str += xstring::xprintf(" SizeX=\"%d\"", img->i.width );
  str += xstring::xprintf(" SizeY=\"%d\"", img->i.height ); 
  str += xstring::xprintf(" SizeC=\"%d\"", img->i.samples ); 
  str += xstring::xprintf(" SizeZ=\"%d\"", img->i.number_z ); 
  str += xstring::xprintf(" SizeT=\"%d\"", img->i.number_t );

  // writing physical sizes
  if (hash && hash->size()>0) {
    if ( hash->hasKey("pixel_resolution_x") )
      str += xstring::xprintf(" PhysicalSizeX=\"%s\"", hash->get_value("pixel_resolution_x").c_str() );
    if ( hash->hasKey("pixel_resolution_y") )
      str += xstring::xprintf(" PhysicalSizeY=\"%s\"", hash->get_value("pixel_resolution_y").c_str() );
    if ( hash->hasKey("pixel_resolution_z") )
      str += xstring::xprintf(" PhysicalSizeZ=\"%s\"", hash->get_value("pixel_resolution_z").c_str() );
    if ( hash->hasKey("pixel_resolution_t") )
      str += xstring::xprintf(" TimeIncrement=\"%s\"", hash->get_value("pixel_resolution_t").c_str() );
  }

  str += " >";
  str += "<TiffData/></Pixels>";

  // channel names
  if (hash && hash->size()>0 && hash->hasKey("channel_0_name")) {
    for (int i=0; i<img->i.samples; ++i) {
      xstring key = xstring::xprintf("channel_%d_name", i);
      if (hash->hasKey(key)) {
        str += xstring::xprintf("<LogicalChannel ID=\"LogicalChannel:%d\" Name=\"%s\" SamplesPerPixel=\"1\">", 
          i, hash->get_value(key).c_str() );
        str += xstring::xprintf("<ChannelComponent Index=\"%d\" Pixels=\"Pixels:0\"/>", i);
        str += "</LogicalChannel>";
      }
    }
  }


  // custom attributes
  if (hash && hash->size()>0) {
    str += "<CustomAttributes>";
    int i=0;
    std::map<std::string, std::string>::const_iterator it;
    for(it = hash->begin(); it != hash->end(); ++it) {
      xstring key = (*it).first;
      if (key.startsWith("custom/")) {
        str += xstring::xprintf("<OriginalMetadata ID=\"OriginalMetadata:%d\" Name=\"%s\" Value=\"%s\"/>", 
          i, key.right(7).c_str(), (*it).second.c_str() );
        i++;
      }
    }
    str += "</CustomAttributes>";
  }

  str += "</Image>";
  if (hash && hash->size()>0)
    str += "<SemanticTypeDefinitions xmlns=\"http://www.openmicroscopy.org/XMLschemas/STD/RC2/STD.xsd\"><SemanticType AppliesTo=\"I\" Name=\"OriginalMetadata\"><Element DBLocation=\"ORIGINAL_METADATA.NAME\" DataType=\"string\" Name=\"Name\"/><Element DBLocation=\"ORIGINAL_METADATA.VALUE\" DataType=\"string\" Name=\"Value\"/></SemanticType></SemanticTypeDefinitions>";

  str += "</OME>";
  return str;
}

DIM_UINT write_omeTiff_metadata (TDimFormatHandle *fmtHndl, DTiffParams *tifParams) {
  TIFF *tif = tifParams->dimTiff;
  TDimTagList *tagList = &fmtHndl->metaData;

  if (tagList->count>0 && tagList->tags)
  for (DIM_UINT i=0; i<tagList->count; i++) {
    TDimTagItem *tagItem = &tagList->tags[i];
    
    if (tagItem->tagGroup == DIM_META_GENERIC && tagItem->tagId == METADATA_TAGS ) {
      DTagMap *hash = (DTagMap *) tagItem->tagData;
      std::string xml = constructOMEXML( fmtHndl, hash );
      TIFFSetField( tif, TIFFTAG_IMAGEDESCRIPTION, xml.c_str() );
      return 0;
    }

    if (tagItem->tagGroup == DIM_META_GENERIC && tagItem->tagId == METADATA_OMEXML ) {
      std::string *xml = (std::string *) tagItem->tagData;
      TIFFSetField( tif, TIFFTAG_IMAGEDESCRIPTION, xml->c_str() );
      return 0;
    }
  }

  std::string xml = constructOMEXML( fmtHndl, 0 );
  TIFFSetField( tif, TIFFTAG_IMAGEDESCRIPTION, xml.c_str());

  return 0;
}

//****************************************************************************
// OME-TIFF WRITER
//****************************************************************************

int omeTiffWritePlane(TDimFormatHandle *fmtHndl, DTiffParams *tifParams) {
  TIFF *out = tifParams->dimTiff;
  TDimImageBitmap *img = fmtHndl->image;
 
  uint32 height;
  uint32 width;
  uint32 rowsperstrip = (uint32) -1;
  uint16 bitspersample;
  uint16 samplesperpixel = 1;
  uint16 photometric = PHOTOMETRIC_MINISBLACK;
  uint16 compression;
  uint16 planarConfig = PLANARCONFIG_SEPARATE;	// separated planes 


  // samples in OME-TIFF are stored in separate IFDs 
  for (DIM_UINT sample=0; sample<img->i.samples; sample++) {

    width = img->i.width;
    height = img->i.height;
    bitspersample = img->i.depth;
    //samplesperpixel = img->i.samples;

    if ( bitspersample==1 && samplesperpixel==1 ) photometric = PHOTOMETRIC_MINISWHITE;

    // handle standard width/height/bpp stuff
    TIFFSetField(out, TIFFTAG_IMAGEWIDTH, width);
    TIFFSetField(out, TIFFTAG_IMAGELENGTH, height);
    TIFFSetField(out, TIFFTAG_SAMPLESPERPIXEL, samplesperpixel);
    TIFFSetField(out, TIFFTAG_BITSPERSAMPLE, bitspersample);
    TIFFSetField(out, TIFFTAG_PHOTOMETRIC, photometric);
    TIFFSetField(out, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
    TIFFSetField(out, TIFFTAG_PLANARCONFIG, planarConfig);	// separated planes
    TIFFSetField(out, TIFFTAG_SOFTWARE, "DIMIN TIFF WRAPPER <www.dimin.net>");

    // set pixel format
    uint16 sampleformat = SAMPLEFORMAT_UINT;
    if (img->i.pixelType == D_FMT_SIGNED) sampleformat = SAMPLEFORMAT_INT;
    if (img->i.pixelType == D_FMT_FLOAT)  sampleformat = SAMPLEFORMAT_IEEEFP;
    TIFFSetField(out, TIFFTAG_SAMPLEFORMAT, sampleformat);

    //if( TIFFGetField( out, TIFFTAG_DOCUMENTNAME, &pszText ) )
    //if( TIFFGetField( out, TIFFTAG_DATETIME, &pszText ) )

    //------------------------------------------------------------------------------  
    // compression
    //------------------------------------------------------------------------------  

    compression = fmtHndl->compression;
    if (compression == 0) compression = COMPRESSION_NONE; 

    switch(bitspersample) {
    case 1  :
      if (compression != COMPRESSION_CCITTFAX4) compression = COMPRESSION_NONE;
      break;

    case 8  :
    case 16 :
    case 32 :
    case 64 :
      if ( (compression != COMPRESSION_LZW) && (compression != COMPRESSION_PACKBITS) )
        compression = COMPRESSION_NONE;  
      break;
    
    default :
      compression = COMPRESSION_NONE;
      break;
    }

    TIFFSetField(out, TIFFTAG_COMPRESSION, compression);

    unsigned long strip_size = (unsigned long) MAX( TIFFDefaultStripSize(out,-1), 1 );
    switch ( compression ) {
      case COMPRESSION_JPEG: {
        TIFFSetField( out, TIFFTAG_ROWSPERSTRIP, strip_size+(16-(strip_size % 16)) );
        break;
      }
      case COMPRESSION_ADOBE_DEFLATE: {
        TIFFSetField( out, TIFFTAG_ROWSPERSTRIP, height );
        if ( (photometric == PHOTOMETRIC_RGB) ||
             ((photometric == PHOTOMETRIC_MINISBLACK) && (bitspersample >= 8)) )
          TIFFSetField( out, TIFFTAG_PREDICTOR, 2 );
        TIFFSetField( out, TIFFTAG_ZIPQUALITY, 9 );
        break;
      }
      case COMPRESSION_CCITTFAX4: {
        TIFFSetField( out, TIFFTAG_ROWSPERSTRIP, height );
        break;
      }
      case COMPRESSION_LZW: {
        TIFFSetField( out, TIFFTAG_ROWSPERSTRIP, strip_size );
        if (planarConfig == PLANARCONFIG_SEPARATE)
           TIFFSetField( out, TIFFTAG_PREDICTOR, PREDICTOR_NONE );
        else
           TIFFSetField( out, TIFFTAG_PREDICTOR, PREDICTOR_HORIZONTAL );
        break;
      }
      default: {
        TIFFSetField( out, TIFFTAG_ROWSPERSTRIP, strip_size );
        break;
      }
    }

    //------------------------------------------------------------------------------
    // writing meta data
    //------------------------------------------------------------------------------
    if (fmtHndl->pageNumber == 0 && sample == 0 ) {
      write_omeTiff_metadata (fmtHndl, tifParams);
      //TIFFFlush(out);
    }


    //------------------------------------------------------------------------------
    // writing image
    //------------------------------------------------------------------------------
    DIM_UCHAR *bits = (DIM_UCHAR *) img->bits[sample];
    DIM_UINT line_size = getLineSizeInBytes( img );
    register uint32 y;

    for (y=0; y<height; y++) {
      dimProgress( fmtHndl, y*(sample+1), height*img->i.samples, "Writing OME-TIFF" );
      if ( dimTestAbort( fmtHndl ) == 1) break;  
      TIFFWriteScanline(out, bits, y, sample);
      bits += line_size;
    } // for y

    TIFFWriteDirectory( out );
    TIFFFlushData(out);
    TIFFFlush(out);
  } // for sample

  return 0;
}


