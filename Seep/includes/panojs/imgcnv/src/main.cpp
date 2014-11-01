/*******************************************************************************
 Command line imgcnv utility

 Author: Dima V. Fedorov <mailto:dima@dimin.net> <http://www.dimin.net/>

 Run arguments: [[-i | -o] FILE_NAME | -t FORMAT_NAME ]

  -i  - output file name, multiple -i are allowed, but in multiple case each will be interpreted as a 1 page image.
  -o  - output file name
  -t  - format to use

  -page    - pages to extract, should be followed by page numbers separated by comma, ex: -page 1,2,5
             page enumeration starts at 1 and ends at number_of_pages
             page number can be a dash where dash will be substituted by a range of values, ex: -page 1,-,5
             if dash is not followed by any number, maximum will be used, ex: '-page 1,-' means '-page 1,-,number_of_pages'
             if dash is a first caracter, 1 will be used, ex: '-page -,5' means '-page 1,-,5'

  [-roi x1,y1,x2,y2]
  -roi     - region of interest, should be followed by: x1,y1,x2,y2 that defines ROI rectangle, ex: -roi 10,10,100,100
             if x1 or y1 are ommited they will be set to 0, ex: -roi ,,100,100 means 0,0,100,100
             if x2 or y2 are ommited they will be set to image size, ex: -roi 10,10,, means 10,10,width-1,height-1

  [-resize w,h[,NN|BL|BC]]
  -resize - should be followed by: width and height of the new image
           if followed by commma and [NN|BL|BC] allowes to choose interpolation method
           NN - Nearest neighbor (default)
           BL - Bilinear
           BC - Bicubic
           Note: resize now is a smarter method that employs image pyramid for faster processing of large images

  [-resample w,h[,NN|BL|BC]]
  -resample - should be followed by: width and height of the new image
           if followed by commma and [NN|BL|BC] allowes to choose interpolation method
           NN - Nearest neighbor (default)
           BL - Bilinear
           BC - Bicubic

  [-depth integer[,F|D|T|E][,U|S|F]]
  -depth - output depth (in bits) per channel, allowed values now are: 8,16,32,64
           if followed by commma and [F|D|T|E] allowes to choose LUT method
           F - Linear full range
           D - Linear data range (default)
           T - Linear data range with tolerance ignoring very low values
           E - equalized
           if followed by commma and U|S|F] the type of output image can be defined
           U - Unsigned integer (with depths: 8,16,32)
           S - Signed integer (with depths: 8,16,32)
           F - Float (with depths: 32,64,80)

  [-remap int[,int]]
  -remap - set of integers separated by comma specifying output channel order (0 means empty channel), ex: 1,2,3

  [-create w,h,z,t,c,d]
  -create - creates a new image with w-width, h-height, z-num z, t-num t, c - channels, d-bits per channel

  [-rotate deg]
  -rotate - rotates the image by deg degrees, only accepted valueas now are: 90, -90 and 180

  [-sampleframes n]
  -sampleframes - samples for reading every Nth frame (useful for videos), ex: -sampleframes 5

  [-tile n]
  -tile - tilte the image and store tiles in output directory, ex: -tile 256

  [-options "xxxxx"]
  -options  - specify encoder specific options, ex: -options "fps 15 bitrate 1000"

  -norm    - normalize input into 8 bits output
  -stretch - stretch data in it's original range
  -meta    - print image's parsed meta-data
  -rawmeta - print image's raw meta-data in one huge pile
  -fmt     - print supported formats
  -multi   - create a multi-paged image if possible, valid for TIFF, GIF...
  -info    - print image info
  -supported - prints yes/no if the file can be decoded
  -display - creates 3 channel image with preferred channel mapping
  -project - combines by MAX all inout frames into one
  -projectmax - combines by MAX all inout frames into one
  -projectmin - combines by MIN all inout frames into one
  -negative - returns negative of input image

  ------------------------------------------------------------------------------
  Encoder specific options

  All video files AVI, SWF, MPEG, etc:
    fps N - specify Frames per Second, where N is a float number, if empty or 0 uses default, ex: fps 29.9
    bitrate N - specify bitrate, where N is an integer number, if empty or 0 uses default, ex: bitrate 1000

  JPEG:
    quality N - specify encoding quality 0-100, where 100 is best

  TIFF:
    compression N - where N can be: none, packbits, lzw, fax, ex: compression none

  ------------------------------------------------------------------------------
  Ex: imgcnv -i 1.jpg -o 2.tif -t TIFF


 History:
   08/08/2001 21:53:31 - First creation
   12/01/2005 20:54:00 - multipage support
   12/02/2005 14:27:00 - print image info
   02/07/2006 19:29:00 - ROI support
   01/29/2007 15:23:00 - support pixel formats different from power of two
                         done by converting incoming format into supported one,
                         now support only for 12 bit -> 16 bit conversion
   2010-01-25 18:55:54 - support for floating point images throughout the app
   2010-01-29 11:25:38 - preserve all metadata and correctly transform it
                         

 Ver : 51
*******************************************************************************/

#define IMGCNV_VER "1.52"

#include <cmath>
#include <cstdio>
#include <cstring>

#include <string>
#include <vector>
#include <list>
#include <algorithm>
#include <iostream>
#include <fstream>

#include <BioImageCore>
#include <BioImage>
#include <BioImageFormats>

#include "reg/registration.h"

//------------------------------------------------------------------------------
// return codes
//------------------------------------------------------------------------------

#define IMGCNV_ERROR_NONE                   0
#define IMGCNV_ERROR_NO_INPUT_FILE          1
#define IMGCNV_ERROR_NO_OUTPUT_FILE         2
#define IMGCNV_ERROR_READING_FILE           3
#define IMGCNV_ERROR_READING_FILE_RAW       4
#define IMGCNV_ERROR_WRITING_FILE           5
#define IMGCNV_ERROR_WRITING_NOT_SUPPORTED  6
#define IMGCNV_ERROR_CREATING_IMAGE         7



//------------------------------------------------------------------------------
// Command line arguments processing
//------------------------------------------------------------------------------
class DConf: public XConf {

public:
  std::vector<xstring> i_names;
  std::vector<xstring> c_names;
  xstring o_name;
  xstring o_fmt;
  bool normalize;
  bool print_meta;
  bool print_meta_parsed;
  bool print_meta_custom;
  std::string print_tag;
  bool print_formats;
  bool print_formats_xml;
  bool print_formats_html;
  bool multipage;
  bool print_info;
  bool supported;
  bool roi;
  std::vector<int> page;
  bool raw_meta;
  int roi_x1, roi_y1, roi_x2, roi_y2;

  bool remap_channels;
  std::vector<int> out_channels;

  bool fuse_channels;
  bool fuse_to_grey;
  std::vector< std::set<int> > out_fuse_channels;
  //std::vector< std::vector< std::pair<unsigned int,float> > > out_weighted_fuse_channels;

  int out_depth;
  D_DataFormat out_pixel_format;
  DimLut::LutType lut_method;
  
  bool version;

  bool create;
  unsigned int  w,h,z,t,c,d;
  bool resample;
  bool resize;
  bool resize3d;
  TDimImage::ResizeMethod resize_method;
  bool resize_preserve_aspect_ratio;
  bool resize_no_upsample;

  bool geometry;

  bool resolution;
  double resvals[4];

  bool stretch;

  bool raw;
  unsigned int  p,e;
  D_DataFormat raw_type;

  bool display;
  double rotate_angle;

  bool project;
  bool project_min;

  bool negative;

  std::string options;

  std::string loadomexml;
  std::string omexml;

  int tile_size;

  int sample_frames;
  int skip_frames_leading;
  int skip_frames_trailing;

  bool no_overlap;
  int min_overlap;
  double overlap_frame_scale;
  TDimImage img_previous;
  int reg_numpoints;
  int reg_max_width;

  int verbose;
  
  std::string i_histogram_file;
  std::string o_histogram_file;

public:
  virtual void cureParams();
  void curePagesArray( const int &num_pages );

public:
  void print( const std::string &s, int verbose_level = 1 );

protected: 
  virtual void init();    
  virtual void processArguments();
};

void DConf::init() {
  XConf::init();

  appendArgumentDefinition( "-i", 1, 
    "input file name, multiple -i are allowed, but in multiple case each will be interpreted as a 1 page image." );

  appendArgumentDefinition( "-c", 1, 
    "additional channels input file name, multiple -c are allowed, in which case multiple channels will be added, -c image must have the same size" );

  appendArgumentDefinition( "-o", 1, "output file name" );
  appendArgumentDefinition( "-t", 1, "output format" );

  appendArgumentDefinition( "-v",          0, "prints version" );

  appendArgumentDefinition( "-meta",        0, "print image's meta-data" );
  appendArgumentDefinition( "-meta-parsed", 0, "print image's parsed meta-data, excluding custom fields" );
  appendArgumentDefinition( "-meta-custom", 0, "print image's custom meta-data fields" );
  appendArgumentDefinition( "-meta-raw",    0, "print image's raw meta-data in one huge pile" );
  appendArgumentDefinition( "-meta-tag",    1, "prints contents of a requested tag, ex: -tag pixel_resolution" );
  appendArgumentDefinition( "-rawmeta",     0, "print image's raw meta-data in one huge pile" );
  appendArgumentDefinition( "-info",        0, "print image info" );
  appendArgumentDefinition( "-supported",   0, "prints yes/no if the file can be decoded" );
  appendArgumentDefinition( "-loadomexml",  1, "reads OME-XML from a file and writes if output format is OME-TIFF" );

  appendArgumentDefinition( "-fmt",        0, "print supported formats" );
  appendArgumentDefinition( "-fmtxml",     0, "print supported formats in XML" );
  appendArgumentDefinition( "-fmthtml",    0, "print supported formats in HTML" );

  appendArgumentDefinition( "-multi",      0, "creates a multi-paged image if possible (TIFF,AVI), enabled by default" );
  appendArgumentDefinition( "-single",     0, "disables multi-page creation mode" );

  appendArgumentDefinition( "-stretch",    0, "stretch data to it's full range" );
  appendArgumentDefinition( "-norm",       0, "normalize input into 8 bits output" );
  appendArgumentDefinition( "-negative",   0, "returns negative of input image" );
  appendArgumentDefinition( "-display",    0, "creates 3 channel image with preferred channel mapping" );
  appendArgumentDefinition( "-project",    0, "combines by MAX all inout frames into one" );
  appendArgumentDefinition( "-projectmax", 0, "combines by MAX all inout frames into one" );
  appendArgumentDefinition( "-projectmin", 0, "combines by MIN all inout frames into one" );


  xstring tmp = "tilte the image and store tiles in the output directory, ex: -tile 256\n";
  tmp += "  argument defines the size of the tiles in pixels\n";
  tmp += "  tiles will be created based on the outrput file name with inserted L, X, Y, where";
  tmp += "    L - is a resolution level, L=0 is native resolution, L=1 is 2x smaller, and so on";
  tmp += "    X and Y - are tile indices in X and Y, where the first tile is 0,0, second in X is: 1,0 and so on";
  tmp += "  ex: '-o my_file.jpg' will produce files: 'my_file_LLL_XXX_YYY.jpg'\n";
  appendArgumentDefinition( "-tile",    1, tmp );

  appendArgumentDefinition( "-rotate", 1, 
    "rotates the image by deg degrees, only accepted valueas now are: 90, -90 and 180" );
  
  appendArgumentDefinition( "-remap", 1, 
    "Changes order and number of channels in the output, channel numbers are separated by comma (0 means empty channel), ex: -remap 1,2,3" );

  tmp = "Changes order and number of channels in the output additionally allowing combining channels\n";
  tmp += "Channels separated by comma specifying output channel order (0 means empty channel)\n";
  tmp += "multiple channels can be added using + sign, ex: -fuse 1+4,2+4+5,3";
  appendArgumentDefinition( "-fuse", 1, tmp );

  tmp = "Produces 3 channel image from up to 6 channels\n";
  tmp += "Channels separated by comma in the following order: Red,Green,Blue,Yellow,Magenta,Cyan,Gray\n";
  tmp += "(0 or empty value means empty channel), ex: -fuse6 1,2,3,4\n";
  appendArgumentDefinition( "-fuse6", 1, tmp );

  appendArgumentDefinition( "-fusegrey", 0, 
    "Produces 1 channel image averaging all input channels, uses RGB weights for 3 channel images and equal weights for all others, ex: -fuseGrey" );

  appendArgumentDefinition( "-create", 1, 
    "creates a new image with w-width, h-height, z-num z, t-num t, c - channels, d-bits per channel, ex: -create 100,100,1,1,3,8" );
  
  appendArgumentDefinition( "-geometry", 1, 
    "redefines geometry for any incoming image with: z-num z, t-num t, ex: -geometry 5,1" );

  appendArgumentDefinition( "-resolution", 1, 
    "redefines resolution for any incoming image with: x,y,z,t where x,y,z are in microns and t in seconds  ex: -resolution 0.012,0.012,1,0" );

  appendArgumentDefinition( "-resample", 1, 
    "Is the same as resize, the difference is resample is brute force and resize uses image pyramid for speed" );

  appendArgumentDefinition( "-sampleframes", 1, 
    "samples for reading every Nth frame (useful for videos), ex: -sampleframes 5" );

  appendArgumentDefinition( "-skip-frames-leading", 1, 
    "skip N initial frames of a sequence, ex: -skip-frames-leading 5" );

  appendArgumentDefinition( "-skip-frames-trailing", 1, 
    "skip N final frames of a sequence, ex: -skip-frames-trailing 5" );

  appendArgumentDefinition( "-ihst", 1, 
    "read image histogram from the file and use for nhancement operations" );

  appendArgumentDefinition( "-ohst", 1, 
    "write image histogram to the file" );

  tmp = "output information about the processing progress, ex: -verbose\n";
  tmp += "  verbose allows argument that defines the amount of info, currently: 1 and 2\n";
  tmp += "  where: 1 is the light info output, 2 is full output\n";
  appendArgumentDefinition( "-verbose", 1, tmp );

  tmp = "Skips frames that overlap with the previous non-overlapping frame, ex: -no-overlap 5\n";
  tmp += "  argument defines maximum allowed overlap in %, in the example it is 5%\n";
  appendArgumentDefinition( "-no-overlap", 1, tmp );

  tmp = "Defines quality for image alignment in number of starting points, ex: -reg-points 200\n";
  tmp += "  Suggested range is in between 32 and 512, more points slow down the processing\n";
  appendArgumentDefinition( "-reg-points", 1, tmp );
 
  tmp = "pages to extract, should be followed by page numbers separated by comma, ex: -page 1,2,5\n";
  tmp += "  page enumeration starts at 1 and ends at number_of_pages\n";
  tmp += "  page number can be a dash where dash will be substituted by a range of values, ex: -page 1,-,5";
  tmp += "  if dash is not followed by any number, maximum will be used, ex: '-page 1,-' means '-page 1,-,number_of_pages'\n";
  tmp += "  if dash is a first caracter, 1 will be used, ex: '-page -,5' means '-page 1,-,5'";
  appendArgumentDefinition( "-page", 1, tmp );

  tmp = "region of interest, should be followed by: x1,y1,x2,y2 that defines ROI rectangle, ex: -roi 10,10,100,100\n";
  tmp += "  if x1 or y1 are ommited they will be set to 0, ex: -roi ,,100,100 means 0,0,100,100\n";
  tmp += "  if x2 or y2 are ommited they will be set to image size, ex: -roi 10,10,, means 10,10,width-1,height-1";
  appendArgumentDefinition( "-roi", 1, tmp );

  tmp = "output depth (in bits) per channel, allowed values now are: 8,16,32,64, ex: -depth 8,D,U\n";
  tmp += "  if followed by commma and [F|D|T|E] allowes to choose LUT method\n";
  tmp += "    F - Linear full range\n";
  tmp += "    D - Linear data range (default)\n";
  tmp += "    T - Linear data range with tolerance ignoring very low values\n";
  tmp += "    E - equalized";
  tmp += "  if followed by commma and U|S|F] the type of output image can be defined";
  tmp += "    U - Unsigned integer (with depths: 8,16,32)";
  tmp += "    S - Signed integer (with depths: 8,16,32)";
  tmp += "    F - Float (with depths: 32,64,80)";
  appendArgumentDefinition( "-depth", 1, tmp );

  tmp = "should be followed by: width and height of the new image, ex: -resize 640,480\n";
  tmp += "  if one of the numbers is ommited or 0, it will be computed preserving aspect ratio, ex: -resize 640,,NN\n";
  tmp += "  if followed by commma and [NN|BL|BC] allowes to choose interpolation method, ex: -resize 640,480,NN\n";
  tmp += "    NN - Nearest neighbor (default)\n";
  tmp += "    BL - Bilinear\n";
  tmp += "    BC - Bicubic\n";
  tmp += "  if followed by commma [AR|MX|NOUP], the sizes will be limited:\n";
  tmp += "    AR - resize preserving aspect ratio, ex: 640,640,NN,AR\n";
  tmp += "    MX|NOUP - size will be used as maximum bounding box, preserving aspect ratio and not upsampling, ex: 640,640,NN,MX";
  appendArgumentDefinition( "-resize", 1, tmp );

  tmp = "performs 3D interpolation on an input image, ex: -resize3d 640,480,16\n";
  tmp += "  if one of the W/H numbers is ommited or 0, it will be computed preserving aspect ratio, ex: -resize3d 640,,16,NN\n";
  tmp += "  if followed by commma and [NN|BL|BC] allowes to choose interpolation method, ex: -resize3d 640,480,16,BC\n";
  tmp += "    NN - Nearest neighbor (default)\n";
  tmp += "    TL - Trilinear\n";
  tmp += "    TC - Tricubic\n";
  tmp += "  if followed by commma AR, the size will be used as maximum bounding box to resize preserving aspect ratio, ex: 640,640,16,BC,AR";
  appendArgumentDefinition( "-resize3d", 1, tmp );

  tmp = "reads RAW image with w,h,c,d,p,e,t ex: -raw 100,100,3,8,10,0,uint8\n";
  tmp += "  w-width, h-height, c - channels, d-bits per channel, p-pages\n";
  tmp += "  e-endianness(0-little,1-big), if in doubt choose 0\n";
  tmp += "  t-pixel type: int8|uint8|int16|uint16|int32|uint32|float|double, if in doubt choose uint8";
  appendArgumentDefinition( "-raw", 1, tmp );

  tmp = "specify encoder specific options, ex: -options \"fps 15 bitrate 1000\"\n\n";
  tmp += "Encoder specific options\n";
  tmp += "Video files AVI, SWF, MPEG, etc. encoder options:\n";
  tmp += "  fps N - specify Frames per Second, where N is a float number, if empty or 0 uses default, ex: fps 29.9\n";
  tmp += "  bitrate N - specify bitrate, where N is an integer number, if empty or 0 uses default, ex: bitrate 1000\n\n";
  tmp += "JPEG encoder options:\n";
  tmp += "  quality N - specify encoding quality 0-100, where 100 is best, ex: quality 90\n";
  tmp += "  progressive no - disables progressive JPEG encoding\n";  
  tmp += "  progressive yes - enables progressive JPEG encoding (default)\n\n";    
  tmp += "TIFF encoder options:\n";
  tmp += "  compression N - where N can be: none, packbits, lzw, fax, ex: compression none\n";
  appendArgumentDefinition( "-options", 1, tmp );

  

  // ---------------------------------------------
  // init the vars
  o_fmt = "TIFF";
  normalize     = false;
  print_meta    = false;
  print_meta_parsed = false;
  print_meta_custom = false;

  print_formats = false;
  print_formats_xml = false;
  print_formats_html = false;
  multipage     = true;
  print_info    = false;
  supported     = false;
  raw_meta      = false;
  //page          = 0; // first page is 1

  roi           = false;
  roi_x1=-1; roi_y1=-1; roi_x2=-1; roi_y2=-1;
  
  remap_channels = false;
  out_channels.resize(0);

  fuse_channels = false;
  fuse_to_grey = false; 
  out_fuse_channels.resize(0);

  out_depth = 0;
  out_pixel_format = D_FMT_UNSIGNED;
  lut_method = DimLut::ltLinearFullRange;

  version       = false;

  create        = false;
  w=0; h=0; z=0; t=0; c=0; d=0;
  resize        = false;
  resize3d      = false;
  resample      = false;
  resize_method = TDimImage::szNearestNeighbor;
  resize_preserve_aspect_ratio = false;
  resize_no_upsample = false;

  geometry = false;
  resolution = false;

  stretch = false;

  raw = false;
  raw_type = D_FMT_UNSIGNED;
  p = 0;
  e=0;

  display = false;
  rotate_angle = 0;

  sample_frames = 0;
  skip_frames_leading=0;
  skip_frames_trailing=0;
  
  project = false;
  project_min = false;

  negative = false;

  no_overlap = false;
  min_overlap = 0;
  reg_numpoints = REG_Q_GOOD_QUALITY;
  reg_max_width = 400; // 320 450 640

  #if defined(DEBUG) || defined(_DEBUG)
  verbose = 2;
  #else
  verbose = 0;
  #endif

  tile_size = 0;
}

void DConf::cureParams() {
  // if requested creating a new image
  if ( this->create ) {
    // while creating a new image we should not be extracting one page of that
    this->page.clear();
    // turn on multipage automatically
    this->multipage = true;
    // disable all printouts
    this->raw_meta = false;
    this->print_meta = false;
    this->print_info = false;
    this->resize = false;
    this->resize3d = false;
    this->raw = false;
  }

  if ( this->i_names.size()>1 ) {
    // while creating a new image from multiple we should not be extracting one page of that
    this->page.clear();
  }

  if ( this->display ) {
    this->remap_channels = false;
    this->fuse_channels = false;
  }

  if ( this->project )
    this->multipage = false;

  if ( this->print_info || this->raw_meta || this->print_meta || this->print_formats || this->print_formats_xml || this->print_formats_html || this->supported )
    this->multipage = false;
}

void DConf::curePagesArray( const int &num_pages ) {

  for (int i=page.size()-1; i>=0; --i) {
    if (page[i] > num_pages)
      page.erase(page.begin()+i);
  }

  for (int i=page.size()-1; i>=0; --i) {
    if (page[i] <= 0) {
      int first = 0;
      int last = num_pages+1;

      if (i>0) first = page[i-1];
      if (i<page.size()-1) last = page[i+1];
      std::vector<int> vals;
      for (int x=first+1; x<last; ++x)
        vals.push_back(x);

      page.insert( page.begin()+i+1, vals.begin(), vals.end() );
      page.erase(page.begin()+i);
    }
  }
}

void DConf::processArguments() {

  i_names = getValues( "-i" );
  c_names = getValues( "-c" );
  o_name  = getValue( "-o" );
  if (keyExists("-t")) o_fmt = getValue( "-t" ).toLowerCase();

  i_histogram_file = getValue( "-ihst" );
  o_histogram_file = getValue( "-ohst" );

  normalize  = keyExists( "-norm" ); 
  print_meta = keyExists( "-meta" ); 
  raw_meta   = keyExists( "-rawmeta" );
  raw_meta   = keyExists( "-meta-raw" );
  if (raw_meta) print_meta = true;  
  loadomexml = getValue( "-loadomexml" );
  
  print_meta_parsed = keyExists( "-meta-parsed" );
  print_meta_custom = keyExists( "-meta-custom" );
  if (print_meta_parsed) print_meta = true;  
  if (print_meta_custom) print_meta = true;  

  if (keyExists("-meta-tag")) {
    print_tag = getValue("-meta-tag");
    print_meta = true; 
  }

  print_formats      = keyExists( "-fmt" ); 
  print_formats_xml  = keyExists( "-fmtxml" );
  print_formats_html = keyExists( "-fmthtml" ); 
  if (print_formats_xml) print_formats = true;
  if (print_formats_html) print_formats = true;

  //multipage  = keyExists( "-multi" ); 
  if (keyExists("-single")) multipage = false;

  print_info = keyExists( "-info" ); 
  supported  = keyExists( "-supported" ); 
  stretch    = keyExists( "-stretch" ); 
  version    = keyExists( "-v" ); 
  display    = keyExists( "-display" ); 
  negative   = keyExists( "-negative" ); 

  if (keyExists( "-project" )) { project = true; project_min = false; }
  if (keyExists( "-projectmax")) { project = true; project_min = false; } 
  if (keyExists( "-projectmin")) { project = true; project_min = true; }   

  sample_frames        = getValueInt("-sampleframes", 0);
  skip_frames_leading  = getValueInt("-skip-frames-leading", 0);
  skip_frames_trailing = getValueInt("-skip-frames-trailing", 0);

  options       = getValue("-options");
  page          = splitValueInt( "-page", 0 );

  tile_size     = getValueInt("-tile", 0);

  if (keyExists( "-no-overlap" )) { 
    no_overlap = true; 
    min_overlap = getValueInt("-no-overlap", 0);
  }

  if (keyExists( "-reg-points" ))
    reg_numpoints = getValueInt("-reg-points", reg_numpoints);

  if (keyExists( "-verbose" ))
    verbose = getValueInt("-verbose", 1);

  rotate_angle = getValueDouble( "-rotate", 0 );
  if ( rotate_angle!=0 && rotate_angle!=90 && rotate_angle!=-90 && rotate_angle!=180 ) { 
    printf("This rotation angle value is not yet supported...\n");
    exit(0);
  }

  if (keyExists( "-roi" )) {
    roi_x1=-1; roi_y1=-1; roi_x2=-1; roi_y2=-1;

    std::vector<int> ints = splitValueInt( "-roi", -1 );
    if ( ints.size()>0 ) roi_x1 = ints[0];
    if ( ints.size()>1 ) roi_y1 = ints[1];
    if ( ints.size()>2 ) roi_x2 = ints[2];
    if ( ints.size()>3 ) roi_y2 = ints[3];

    roi = false;
    for (unsigned int x=0; x<ints.size(); ++x)
      if (ints[x] >= 0) { roi = true; break; }
  }

  if (keyExists( "-depth" )) {
    std::vector<xstring> strl = splitValue("-depth");
    out_depth = strl[0].toInt(0);
    if (strl.size()>1) {
      if ( strl[1].toLowerCase()=="f" ) lut_method = DimLut::ltLinearFullRange;
      if ( strl[1].toLowerCase()=="d" ) lut_method = DimLut::ltLinearDataRange;
      if ( strl[1].toLowerCase()=="t" ) lut_method = DimLut::ltLinearDataTolerance;
      if ( strl[1].toLowerCase()=="e" ) lut_method = DimLut::ltEqualize;
    }
    if (strl.size()>2) {
      if ( strl[2].toLowerCase()=="u" ) out_pixel_format = D_FMT_UNSIGNED;
      if ( strl[2].toLowerCase()=="s" ) out_pixel_format = D_FMT_SIGNED;
      if ( strl[2].toLowerCase()=="f" ) out_pixel_format = D_FMT_FLOAT;
    }
  }

  if (keyExists( "-remap" )) {
    out_channels = splitValueInt( "-remap" );
    for (unsigned int i=0; i<out_channels.size(); ++i)
      out_channels[i] = out_channels[i]-1;
    remap_channels = true;
  }

  if (keyExists( "-fuse" )) {
    std::vector<xstring> fc = splitValue( "-fuse" );
    for (int i=0; i<fc.size(); ++i) {
      std::vector<xstring> ss = fc[i].split( "+" );
      std::set<int> s;
      for (int p=0; p<ss.size(); ++p)
        s.insert( ss[p].toInt()-1 );
      out_fuse_channels.push_back(s);
    }
    if (out_fuse_channels.size()>0) fuse_channels = true;
  }

  if (keyExists( "-fuse6" )) {
    std::vector<int> c = splitValueInt( "-fuse6" );
    c.resize(7, 0);
    std::set<int> rv, gv, bv;
    rv.insert(c[0]-1);
    rv.insert(c[3]-1);
    rv.insert(c[4]-1);
    rv.insert(c[6]-1);
    gv.insert(c[1]-1);
    gv.insert(c[3]-1);
    gv.insert(c[5]-1);
    gv.insert(c[6]-1);
    bv.insert(c[2]-1);
    bv.insert(c[4]-1);
    bv.insert(c[5]-1);
    bv.insert(c[6]-1);

    out_fuse_channels.push_back(rv);
    out_fuse_channels.push_back(gv);
    out_fuse_channels.push_back(bv);
    if (out_fuse_channels.size()>0) fuse_channels = true;
  }

  if (keyExists( "-fusegrey" )) {
    fuse_to_grey = true;
    fuse_channels = true;
  }

  if (keyExists( "-create" )) {
    std::vector<int> ints = splitValueInt( "-create" );
    for (unsigned int x=0; x<ints.size(); ++x)
      if (ints[x] <= 0) { 
        printf("Unable to create an image, some parameters are invalid! Note that one image lives in 1 time and 1 z points...\n");
        exit(0);
      }

    if ( ints.size() >= 6  ) {
      this->w = ints[0]; 
      this->h = ints[1]; 
      this->z = ints[2]; 
      this->t = ints[3]; 
      this->c = ints[4]; 
      this->d = ints[5];
      this->create = true;
    }
  }

  if (keyExists( "-geometry" )) {
    std::vector<int> ints = splitValueInt( "-geometry" );
    for (unsigned int x=0; x<ints.size(); ++x)
      if (ints[x] <= 0) { 
        printf("Incorrect geometry values! Note that one image lives in 1 time and 1 z points...\n");
        exit(0);
      }

    if ( ints.size() >= 2  ) {
      this->z = ints[0]; 
      this->t = ints[1]; 
      this->geometry = true;
    }
  }

  if (keyExists( "-resolution" )) {
    std::vector<double> vals = splitValueDouble( "-resolution", -1.0 );
    for (unsigned int x=0; x<4; ++x) resvals[x] = 0.0;
    for (unsigned int x=0; x<vals.size(); ++x)
      if (vals[x]<0) { 
        printf("Incorrect resolution values!\n");
        exit(0);
      } else
        this->resvals[x] = vals[x];
    if (vals.size()>0) this->resolution = true;
  }

  if ( keyExists("-resize") || keyExists("-resample") ) {
    std::vector<xstring> strl;
    if (keyExists("-resize")) strl = splitValue( "-resize" );
    if (keyExists("-resample")) { strl = splitValue( "-resample" ); resample = true; }

    this->w = 0; this->h = 0;
    if ( strl.size() >= 2 ) {
      this->w = strl[0].toInt(0);
      this->h = strl[1].toInt(0);
    }
    if (strl.size()>2) {
      if ( strl[2].toLowerCase() == "nn") resize_method = TDimImage::szNearestNeighbor;
      if ( strl[2].toLowerCase() == "bl") resize_method = TDimImage::szBiLinear;
      if ( strl[2].toLowerCase() == "bc") resize_method = TDimImage::szBiCubic;
    }
    if (strl.size()>3) {
      if ( strl[3].toLowerCase() == "ar") resize_preserve_aspect_ratio = true;
      if ( strl[3].toLowerCase() == "mx") { resize_preserve_aspect_ratio = true; resize_no_upsample=true; }
      if ( strl[3].toLowerCase() == "noup") { resize_preserve_aspect_ratio = true; resize_no_upsample=true; }
    }

    if (this->w<=0 || this->h<=0) resize_preserve_aspect_ratio = false;
    if (this->w>0 || this->h>0) resize = true;
  }

  if (keyExists("-resize3d")) {
    std::vector<xstring> strl;
    if (keyExists("-resize3d")) strl = splitValue( "-resize3d" );

    this->w = 0; this->h = 0; this->z = 0;
    if ( strl.size() >= 3 ) {
      this->w = strl[0].toInt(0);
      this->h = strl[1].toInt(0);
      this->z = strl[2].toInt(0);
    }
    if (strl.size()>3) {
      if ( strl[3].toLowerCase() == "nn") resize_method = TDimImage::szNearestNeighbor;
      if ( strl[3].toLowerCase() == "tl") resize_method = TDimImage::szBiLinear;
      if ( strl[3].toLowerCase() == "tc") resize_method = TDimImage::szBiCubic;
    }
    if (strl.size()>4)
      if ( strl[4].toLowerCase() == "ar") resize_preserve_aspect_ratio = true;

    if (this->w<=0 || this->h<=0) resize_preserve_aspect_ratio = false;
    if ( (this->w>0 || this->h>0) && this->z>0) resize3d = true;
  }

  if (keyExists( "-raw" )) {
    std::vector<xstring> strl = splitValue( "-raw" );
    this->w = 0; this->h = 0; this->c = 0; this->d = 0; this->p = 0; this->e = 0;
    if ( strl.size() >= 6 ) {
      this->w = strl[0].toInt(0); 
      this->h = strl[1].toInt(0); 
      this->c = strl[2].toInt(0); 
      this->d = strl[3].toInt(0);
      this->p = strl[4].toInt(0);
      this->e = strl[5].toInt(0);
    }

    if (strl.size()>6) {
      if ( strl[6]=="int8")   raw_type = D_FMT_SIGNED;
      if ( strl[6]=="uint8")  raw_type = D_FMT_UNSIGNED;
      if ( strl[6]=="int16")  raw_type = D_FMT_SIGNED;
      if ( strl[6]=="uint16") raw_type = D_FMT_UNSIGNED;
      if ( strl[6]=="int32")  raw_type = D_FMT_SIGNED;
      if ( strl[6]=="uint32") raw_type = D_FMT_UNSIGNED;
      if ( strl[6]=="float")  raw_type = D_FMT_FLOAT;
      if ( strl[6]=="double") raw_type = D_FMT_FLOAT;
    }

    if (this->w>0 && this->h>0 && this->c>0 && this->d>0 && this->p>0 ) raw = true;
  }

}

void DConf::print( const std::string &s, int verbose_level ) {
  if (this->verbose>=verbose_level)
    printf("%s\n", s.c_str() );  
}

//------------------------------------------------------------------------------
// Output
//------------------------------------------------------------------------------

void printAbout() {
  printf("\nimgcnv ver: %s\n\n", IMGCNV_VER);
  printf("Author: Dima V. Fedorov <http://www.dimin.net/>\n\n");
  printf("Arguments: [[-i | -o] FILE_NAME | -t FORMAT_NAME ]\n\n");
  printf("Ex: imgcnv -i 1.jpg -o 2.tif -t TIFF\n\n");
}


void printFormats() {
  TDimFormatManager fm;
  fm.printAllFormats();
}

void printFormatsXML() {
  TDimFormatManager fm;
  fm.printAllFormatsXML();
}

void printFormatsHTML() {
  TDimFormatManager fm;
  fm.printAllFormatsHTML();
}

void printMetaField( const xstring &key, const xstring &val ) {
  xstring v = val.replace( "\\", "\\\\" );
  v = v.replace( "\n", "\\" );
  v = v.replace( "\"", "'" );
  v = v.removeSpacesBoth();
  printf("%s: %s\n", key.c_str(), v.c_str() );  
}

void printMeta( TMetaFormatManager *fm, TDimImage *img ) {
  const std::map<std::string, std::string> metadata = fm->get_metadata();
  std::map<std::string, std::string>::const_iterator it;
  for(it = metadata.begin(); it != metadata.end(); ++it) {
    xstring s = (*it).first;
    if ( !s.startsWith("raw/") )
    printMetaField( (*it).first, (*it).second );  
  }
}

void printTag( TMetaFormatManager *fm, TDimImage *img, const std::string &key ) {
  const std::map<std::string, std::string> metadata = fm->get_metadata();
  std::map<std::string, std::string>::const_iterator it = metadata.find(key);
  if (it != metadata.end())
    printf( (*it).second.c_str() );  
}

void printMetaParsed( TMetaFormatManager *fm, TDimImage *img ) {
  const std::map<std::string, std::string> metadata = fm->get_metadata();
  std::map<std::string, std::string>::const_iterator it;
  for(it = metadata.begin(); it != metadata.end(); ++it) {
    xstring s = (*it).first;
    if ( !s.startsWith("custom/") && !s.startsWith("raw/") )
      printMetaField( (*it).first, (*it).second ); 
  }
}

void printMetaCustom( TMetaFormatManager *fm, TDimImage *img ) {
  const std::map<std::string, std::string> metadata = fm->get_metadata();
  std::map<std::string, std::string>::const_iterator it;
  for(it = metadata.begin(); it != metadata.end(); ++it) {
    xstring s = (*it).first;
    if ( s.startsWith("custom/") )
      printMetaField( (*it).first, (*it).second ); 
  }
}

void printMetaRaw( TMetaFormatManager *fm, TDimImage *img ) {
  const std::map<std::string, std::string> metadata = fm->get_metadata();
  std::map<std::string, std::string>::const_iterator it;
  for(it = metadata.begin(); it != metadata.end(); ++it) {
    xstring s = (*it).first;
    if ( s.startsWith("raw/") )
      printMetaField( (*it).first, (*it).second ); 
  }
}


//------------------------------------------------------------------------------
// Tiles
//------------------------------------------------------------------------------

int extractTiles(DConf *c) {
  xstring input_filename = c->i_names[0];
  xstring output_path = c->o_name;
  int tile_size = c->tile_size;

  if (output_path.size() < 1) {
    printf("You must provide output path for tile storage!\n");
    return IMGCNV_ERROR_NO_OUTPUT_FILE; 
  }
 
  DImagePyramid ip;
  ip.setMinImageSize( tile_size );
  ip.fromFile( input_filename, 0 ); // load only page 0 !!!!!!!!!!!!!

  unsigned int i=0, j=0;
  for (int l=0; l<ip.numberLevels(); ++l) {
    TDimImage *level_img = ip.image();
    i=0;
    c->print( xstring::xprintf("Level: %d", l), 2 );
    for (int y=0; y<level_img->height(); y+=tile_size) {
      j=0;
      for (int x=0; x<level_img->width(); x+=tile_size) {
        TDimImage tile = level_img->ROI( x, y, tile_size, tile_size );
        xstring ofname = output_path;
        ofname.insertAfterLast( ".", xstring::xprintf("_%.3d_%.3d_%.3d", l, j, i) );
        if (!tile.toFile(ofname, c->o_fmt, c->options )) return IMGCNV_ERROR_WRITING_FILE;
        ++j;
      } // j
      ++i;
    } // i
    ip.levelDown();
  } // l

  if (c->o_histogram_file.size()>0) {
    DImageHistogram h( *ip.imageAt(0) );
    h.to( c->o_histogram_file );
  }
}

//------------------------------------------------------------------------------
// 3D interpolation
//------------------------------------------------------------------------------

int resize_3d(DConf *c) {
  c->print( "About to run resize3D", 2 );
  DImageStack stack;
  stack.fromFile( c->i_names[0] );
  stack.ensureTypedDepth();
  stack.resize( c->w, c->h, c->z, c->resize_method, c->resize_preserve_aspect_ratio );  
  stack.toFile(c->o_name, c->o_fmt);//, c->options );
  return IMGCNV_ERROR_NONE;
}

//------------------------------------------------------------------------------
// overlap detection
//------------------------------------------------------------------------------
bool is_overlapping_previous( const TDimImage &img, DConf *c ) {

  if (c->img_previous.isEmpty()) {
    c->overlap_frame_scale = 1.0;
    if (img.width()<=c->reg_max_width && img.height()<=c->reg_max_width)
      c->img_previous = img.fuseToGrayscale();
    else {
      c->img_previous = img.resample(c->reg_max_width, c->reg_max_width, TDimImage::szBiLinear, true).fuseToGrayscale();
      c->overlap_frame_scale = (double) img.width() / (double) c->img_previous.width();
    }
    return false;
  }
  
  reg::Params rp;
  rp.numpoints = c->reg_numpoints;
  rp.transformation = reg::Affine; //enum Transformation { RST, Affine, Translation, ST, ProjectiveNS  };

  // convert images if needed
  TDimImage image2;
  if (c->overlap_frame_scale>1)
    image2 = img.resample( (double)img.width()/c->overlap_frame_scale, (double)img.height()/c->overlap_frame_scale, TDimImage::szBiLinear, true).fuseToGrayscale();
  else
    image2 = img.fuseToGrayscale();

  //c->img_previous.toFile( "G:\\_florida_video_transects\\image1.png", "png" );
  //image2.toFile( "G:\\_florida_video_transects\\image2.png", "png" );

  // register
  int res = register_image_pair(&c->img_previous, &image2, &rp);

  bool overlapping = false;

  // verify 
  if ( res==REG_OK && (rp.goodbad==reg::Good || rp.goodbad==reg::Excellent) && rp.tiePoints1.size()>4 ) {
    overlapping = true;
  } else 
  if ( res==REG_OK && rp.goodbad==reg::Uncertain && rp.rmse<3 && rp.tiePoints1.size()>4 ) {
    overlapping = true;
  }
  //int min_overlap;

  if (!overlapping) c->img_previous = image2;

  return overlapping;
}

//------------------------------------------------------------------------------
// MAIN
//------------------------------------------------------------------------------

int main( int argc, char** argv ) {
  TMetaFormatManager fm; // input format manager
  TMetaFormatManager ofm; // output format manager
  TDimImage img;
  TDimImage img_projected;

  DConf conf;
  if ( conf.readParams( argc, argv ) != 0)  { 
    printAbout(); 
    printf( conf.usage().c_str() ); 
    return IMGCNV_ERROR_NONE; 
  }

  if (conf.version) { 
    printf("%s\n", IMGCNV_VER);
    return IMGCNV_ERROR_NONE; 
  }

  if (conf.print_formats) { 
    if (conf.print_formats_xml) printFormatsXML(); 
    else
    if (conf.print_formats_html) printFormatsHTML(); 
    else printFormats(); 
    return IMGCNV_ERROR_NONE; 
  }

  if (conf.supported) { 
    if (fm.sessionStartRead(conf.i_names[0].c_str()) != 0) printf("no\n"); else printf("yes\n");
    return IMGCNV_ERROR_NONE; 
  }

  if (conf.i_names.size() <= 0) { 
    printf("You must provide at least one input file!\n");
    return IMGCNV_ERROR_NO_INPUT_FILE; 
  }

  // check if format supported for writing
  if (ofm.isFormatSupportsW(conf.o_fmt.c_str()) == false) {
    printf("\"%s\" Format is not supported for writing!\n", conf.o_fmt.c_str());  
    return IMGCNV_ERROR_WRITING_NOT_SUPPORTED;
  }  

  if (conf.tile_size > 0) { 
    return extractTiles(&conf);
  }

  if (conf.resize3d) { 
    return resize_3d(&conf);
  }

  // reading OME-XML from std-in
  if (conf.loadomexml.size()>0) {
    std::string line;
    std::ifstream myfile(conf.loadomexml.c_str());
    if (myfile.is_open()) {
      while (!myfile.eof() ) {
        getline (myfile,line);
        conf.omexml += line;
      }
    }
    myfile.close();  

    conf.print( xstring::xprintf("Red OME-XML with %d characters", conf.omexml.size()), 2 );
  }


  // Load histogram if requested, it will be used later if operation needs it
  DImageHistogram hist;
  if (conf.i_histogram_file.size()>0)
    hist.from( conf.i_histogram_file );


  //----------------------------------------------------------------------
  // start conversion process
  //----------------------------------------------------------------------
  DIM_UINT num_pages=0;
  DIM_UINT page=0;

  if (conf.create) { 
    // create image from defeined params
    img.create( conf.w, conf.h, conf.d, conf.c );
    img.fill(0);
    if (img.isNull()) { printf("Error creating new image\n"); return IMGCNV_ERROR_CREATING_IMAGE; }
    num_pages = conf.t * conf.z;
  } else
  if (conf.i_names.size()==1) { 
    
    // load image from the file, in the normal way
    if (!conf.raw) {
      if (fm.sessionStartRead(conf.i_names[0].c_str()) != 0)  {
        printf("Input format is not supported\n");    
        return IMGCNV_ERROR_READING_FILE;
      }
      num_pages = fm.sessionGetNumberOfPages();
    } else {
      // if reading RAW
      if (fm.sessionStartReadRAW(conf.i_names[0].c_str(), 0, (bool)conf.e) != 0)  {
        printf("Error opening RAW file\n");    
        return IMGCNV_ERROR_READING_FILE_RAW;
      }
      num_pages = conf.p;
    }


    if (conf.page.size()>0) {
      // cure pages array first, removing invalid pages and dashes
      //curePagesArray( &conf, num_pages );
      conf.curePagesArray( num_pages );
      num_pages = conf.page.size();
    }

  } else
  if (conf.i_names.size() > 1) { 
    // multiple input files, interpret each one as pages
    num_pages = conf.i_names.size();
  }

  conf.print( xstring::xprintf("Number of pages: %d", num_pages) );

  // check if format supports writing multipage
  if (ofm.isFormatSupportsWMP(conf.o_fmt.c_str()) == false) conf.multipage = false;

  // start writing session if multipage 
  if (conf.multipage == true && conf.o_name.size()>0) {
    if (ofm.sessionStartWrite(conf.o_name.c_str(), conf.o_fmt.c_str(), conf.options.c_str()) != 0) {
      printf("Cannot write into: %s\n", conf.o_name.c_str());        
      return IMGCNV_ERROR_WRITING_FILE;
    }
  }
  int sampling_frame = 0;


  // WRITE IMAGES
  for (page=0; page<num_pages; ++page) {

    if (conf.skip_frames_leading>0 && conf.skip_frames_leading>page)
      continue;

    if (conf.skip_frames_trailing>0 && conf.skip_frames_trailing>num_pages-page)
      continue;

    if (conf.sample_frames>0 && sampling_frame>0) {
      ++sampling_frame;
      if (sampling_frame == conf.sample_frames) sampling_frame = 0;
      continue;
    }
    ++sampling_frame;

    // if it's raw reading we have to init raw input image
    if (conf.raw) {
      img.alloc(conf.w, conf.h, conf.c, conf.d);
      img.imageBitmap()->i.number_pages = conf.p;
      img.imageBitmap()->i.pixelType    = conf.raw_type;
    }
    
    // if normal reading
    unsigned int real_frame = page;
    if (conf.page.size()>0) real_frame = conf.page[page]-1;
    if (!conf.create && conf.i_names.size()==1)
      fm.sessionReadImage  ( img.imageBitmap(), real_frame );

    // if multiple file reading    
    if (!conf.create && conf.i_names.size()>1) {
      int res=0;
      
      if (!conf.raw)
        res = fm.sessionStartRead(conf.i_names[page].c_str());
      else
        res = fm.sessionStartReadRAW(conf.i_names[0].c_str(), 0, (bool)conf.e);

      if (res != 0)  {
        printf("Input format is not supported for: %s\n", conf.i_names[page].c_str());    
        return IMGCNV_ERROR_READING_FILE;
      }
      fm.sessionReadImage  ( img.imageBitmap(), 0 );
    }

    if (img.isNull()) continue;

    conf.print( xstring::xprintf("Got image for frame: %d", real_frame) );


    // ------------------------------------------------------------------
    // metadata
    if (page==0) {
      fm.sessionParseMetaData(0);
      img.set_metadata(fm.get_metadata());
    }
    // ------------------------------------------------------------------

    // update image's geometry
    if (conf.geometry)
      img.updateGeometry( conf.z, conf.t );

    if (conf.resolution)
      img.updateResolution( conf.resvals );

    if ( (conf.print_info == true) && (page == 0) ) {
      printf( "format: %s\n", fm.sessionGetFormatName() );
      printf( img.getTextInfo().c_str() );
    }

    // print out meta-data
    if (conf.print_meta && (page == 0) ) {
      //fm.sessionParseMetaData(0);
      if (conf.print_meta_parsed)
        printMetaParsed( &fm, &img );
      else
      if (conf.print_meta_custom)
        printMetaCustom( &fm, &img );
      else
      if (conf.raw_meta)
        printMetaRaw( &fm, &img );
      else
      if (conf.print_tag.size()>0) 
        printTag( &fm, &img, conf.print_tag );
      else
        printMeta( &fm, &img );
    }

    xstring ofname = conf.o_name;
    if (ofname.size() < 1) return IMGCNV_ERROR_NO_OUTPUT_FILE;

    // make sure red image is in supported pixel format, e.g. will convert 12 bit to 16 bit
    img = img.ensureTypedDepth();

    // ------------------------------------    
    // if asked to append channels
    if (conf.c_names.size()>0) {
      conf.print( "About to append channels", 2 );
      for (int ccc=0; ccc<conf.c_names.size(); ++ccc) {
        TDimImage ccc_img;
        ccc_img.fromFile( conf.c_names[ccc], page );
        img = img.appendChannels( ccc_img );
      }
      hist.clear();
    }

    if (conf.stretch) {
      img = img.convertToDepth( img.depth(), DimLut::ltLinearDataRange, D_FMT_UNDEFINED, &hist );
    }

    // ------------------------------------    
    // if asked convert image depth
    if (conf.out_depth>0) {
      conf.print( "About to run Depth", 2 );
      if (conf.out_depth!=8 && conf.out_depth!=16 && conf.out_depth!=32 && conf.out_depth!=64)
        printf( "Output depth (%s bpp) is not supported! Ignored!\n", conf.out_depth );
      else
        img = img.convertToDepth( conf.out_depth, conf.lut_method, conf.out_pixel_format, &hist );
    }

    // if asked, normalize, also, has to be normalized if format does not support intput depth
    if ( (conf.normalize == true) || (!fm.isFormatSupportsBpcW( conf.o_fmt.c_str(), img.depth() )) ) {
      conf.print( "About to run normalize", 2 );
      img = img.normalize(8, &hist);
    }

    // ------------------------------------
    // ROI
    if ( conf.roi ) {
      conf.print( "About to run ROI", 2 );
      // it's allowed to specify only one of the sizes, the other one will be computed
      if (conf.roi_x1 == -1) conf.roi_x1 = 0;
      if (conf.roi_y1 == -1) conf.roi_y1 = 0;
      if (conf.roi_x2 == -1) conf.roi_x2 = img.width()-1;
      if (conf.roi_y2 == -1) conf.roi_y2 = img.height()-1;

      if (conf.roi_x1>=conf.roi_x2 || conf.roi_y1>=conf.roi_y2)
        printf( "ROI parameters are invalid, ignored!\n" );
      else
        img = img.ROI( conf.roi_x1, conf.roi_y1, conf.roi_x2-conf.roi_x1+1, conf.roi_y2-conf.roi_y1+1 );
    }

    // ------------------------------------
    // Resize
    if (conf.resize && (!conf.resize_no_upsample || (conf.resize_no_upsample && img.width()>conf.w && img.height()>conf.h)) ) {
      conf.print( "About to run resize", 2 );
      if (conf.resize_preserve_aspect_ratio)
        if ( (img.width()/(float)conf.w) >= (img.height()/(float)conf.h) ) conf.h = 0; else conf.w = 0;

      // it's allowed to specify only one of the sizes, the other one will be computed
      if (conf.w == 0)
        conf.w = dim::round<unsigned int>( img.width() / (img.height()/(float)conf.h) );
      if (conf.h == 0)
        conf.h = dim::round<unsigned int>( img.height() / (img.width()/(float)conf.w) );

      if (!conf.resample)
        img = img.resize( conf.w, conf.h, conf.resize_method );
      else
        img = img.resample( conf.w, conf.h, conf.resize_method );

      if (conf.out_depth>0)
        img = img.convertToDepth( conf.out_depth, conf.lut_method, conf.out_pixel_format );
    }

    // ------------------------------------    
    // Rotate
    if (conf.rotate_angle != 0) {
      conf.print( "About to run rotate", 2 );
      img = img.rotate( conf.rotate_angle );
    }

    // ------------------------------------    
    // Negative
    if (conf.negative) {
      conf.print( "About to run negative", 2 );
      img = img.negative();
    }

    // ------------------------------------    
    // Project
    if (conf.project) {
      conf.print( "About to run project", 2 );
      if (img_projected.isNull()) 
        img_projected = img.deepCopy();
      else
        if (!conf.project_min)
          img_projected.pixelArithmeticMax( img );
        else
          img_projected.pixelArithmeticMin( img );
      hist.clear();
    }

    // ------------------------------------    
    // Channel fusion
    if (conf.fuse_channels) {
      conf.print( "About to run channel fusion", 2 );
      if (!conf.fuse_to_grey)
        img = img.fuse( conf.out_fuse_channels );
      else
        img = img.fuseToGrayscale();

//    TDimImage fuse( const std::vector< std::vector< std::pair<unsigned int,float> > > &mapping ) const;
      hist.clear();
    }


    // ------------------------------------    
    // Overlapping frames detection
    if (conf.no_overlap) {
      if (is_overlapping_previous(img, &conf)) {
        conf.print( xstring::xprintf("Overlap detected, skipping frame %d", real_frame) );
        continue;
      }
    }


    // ------------------------------------    
    // remap according to the preferred mapping, this will generate 3 channel output
    // run this at the very end, it's fast but channels can point to the same place
    if (!conf.project) {
      if ( conf.display ) {
        conf.print( "Getting parsing session metadata", 2 );
        fm.sessionParseMetaData(0);
        conf.print( "About to run display", 2 );
        // performance optimization, when no fusion is necessary simply run channel mapping
        if (!fm.display_lut_needs_fusion())
          img.remapChannels( fm.get_display_lut() );
        else {
          std::vector< int > lut = fm.get_display_lut();
          img = img.fuse( lut[bim::Red], lut[bim::Green], lut[bim::Blue], lut[bim::Yellow], lut[bim::Magenta], lut[bim::Cyan], lut[bim::Gray] );
        }
      } else
      if ( conf.remap_channels ) {
        conf.print( "About to run remapChannels", 2 );
        img.remapChannels( conf.out_channels );
      }
      hist.clear();
    } // if not projecting

    // prepare metadada for writing



    // write into a file
    conf.print( "About to write", 2 );
    if (!conf.project) {

      if ( conf.omexml.size()>0 ) 
        ofm.sessionWriteSetOMEXML( conf.omexml ); 
      else
      if ( img.get_metadata().size()>0 ) 
        ofm.sessionWriteSetMetadata( img.get_metadata() );

      if (conf.multipage == true) {
         ofm.sessionWriteImage( img.imageBitmap(), page );
      } else { // if not multipage
        if (num_pages > 1)
          ofname.insertAfterLast( ".", xstring::xprintf("_%.6d", real_frame+1) );

        fm.writeImage (ofname.c_str(), img.imageBitmap(), conf.o_fmt.c_str(), conf.options.c_str() );
      } // if not multipage
    } // if not projecting

    // if the image was remapped then kill the data repos, it'll have to be reinited
    if (!conf.project && (conf.display || conf.remap_channels) ) {
      conf.print( "About to clear image", 2 );
      img.clear();
    }

  } // for pages


  // if we were projecting an image then create correct mapping here and save
  if (conf.project) {

    if ( conf.display ) {
      fm.sessionParseMetaData(0);
      img_projected.remapChannels( fm.get_display_lut() );
    } else
      // run this at the very end, it's fast but channels can point to the same place
      if ( conf.remap_channels ) 
        img_projected.remapChannels( conf.out_channels );

    fm.writeImage ( conf.o_name.c_str(), img_projected, conf.o_fmt.c_str(), conf.options.c_str() );
    hist.clear();
  } // if storing projected file

  fm.sessionEnd(); 
  ofm.sessionEnd();

  // Store histogram if requested
  if (conf.o_histogram_file.size()>0) {
    if (!hist.isValid()) hist.fromImage( img );
    hist.to( conf.o_histogram_file );
  }

  return IMGCNV_ERROR_NONE;
}

