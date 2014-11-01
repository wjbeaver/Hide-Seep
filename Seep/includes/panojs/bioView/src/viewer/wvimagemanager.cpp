/*******************************************************************************

  Manager for Image Formats with MetaData parsing

  Uses DimFiSDK version: 1
  
  Programmer: Dima Fedorov Levit <dimin@dimin.net> <http://www.dimin.net/>

  Notes:
    Session: during session any session wide operation will be performed
    with current session!!!
    If you want to start simultaneous sessions then create new manager
    using = operator. It will copy all necessary initialization data
    to the new manager.

    Non-session wide operation might be performed simultaneously with 
    an open session.

  History:
    03/23/2004 18:03 - First creation
    01/23/2007 22:02 - interpolation - speedup by removing rounding and trimming
      
  ver: 2
        
*******************************************************************************/

#include "wvimagemanager.h"

#include <QtCore>
#include <QtGui>

#include <QApplication>
#include <QByteArray>
#include <QLabel>
#include <QMessageBox>

#include <BioImageCore>
#include <BioImage>
#include <BioImageFormats>
#include <formats_api/dim_qt_utils.h>

#include <appconfig.h>


const int channel_lut_def [WVImageManager::NumDisplayChannels] = {0, 1,  2, -1,-1,-1,-1};
const int channel_lut_gray[WVImageManager::NumDisplayChannels] = {0, 0,  0, -1,-1,-1,-1};
const int channel_lut_2ch [WVImageManager::NumDisplayChannels] = {0, 1, -1, -1,-1,-1,-1};

inline unsigned char trim_to_uc (int num) {
  if (num < 0) return 0;
  if (num > 255) return 255;
  return (unsigned char) num;
}

//------------------------------------------------------------------------------
// WVRenderParams
//------------------------------------------------------------------------------

WVRenderParams::WVRenderParams(): channel_lut(0), image_hist(0) { 
  for (int i=0; i<WVImageManager::NumDisplayChannels; ++i) scale[i]=1.0;
  for (int i=0; i<WVImageManager::NumDisplayChannels; ++i) shift[i]=0.0;

  fast_copy = false;
  lut_map = 0; 
  in_img = 0;

  lut_map = 0;
  intencity_luts = 0;

  need_fusion = false;
}

WVRenderParams::~WVRenderParams() {
}

void WVRenderParams::compute_shift_scale() {
  if (image_hist==0) return;
  if (channel_lut==0) return;

  for (int i=0; i<WVImageManager::NumDisplayChannels; ++i) {
    if (channel_lut[i]>=0) {
      scale[i] = (*image_hist)[channel_lut[i]].get_scale();
      shift[i] = (*image_hist)[channel_lut[i]].get_shift();
    } else {
      scale[i] = 1.0;
      shift[i] = 0.0;
    }
  }
}

void WVRenderParams::init() {
  compute_shift_scale();
}

//------------------------------------------------------------------------------
// WVImageManager
//------------------------------------------------------------------------------

WVImageManager::WVImageManager() {
  memcpy( channel_lut, channel_lut_def, sizeof(int)*NumDisplayChannels );
  zoom_method = dzmNN;
  enhance_area = deaFullImage;

  enhance_type = wv_lut_generator_default;

  lut_generators << &lut_gen_full_range;
  lut_generators << &lut_gen_data_range;
  lut_generators << &lut_gen_data_toler;
  lut_generators << &lut_gen_equalize;

  lut_modifiers << &lut_mod_negative;
  lut_modifiers << &lut_mod_bright_contrast;
  lut_modifiers << &lut_mod_levels;

  connect(&lut_mod_negative, SIGNAL( changed() ), this, SLOT( onLutsChanged() ));
  connect(&lut_mod_bright_contrast, SIGNAL( changed() ), this, SLOT( onLutsChanged() ));
  connect(&lut_mod_levels, SIGNAL( changed() ), this, SLOT( onLutsChanged() ));

  zoom = 0;
  pages_in_image = 0;
  progress_proc = NULL;
  error_proc = NULL;
  test_abort_proc = NULL;
  pixel_size = QVector<double>(3,0.0);

  // initialize screen buffer
  screen_buffer = QImage( DSysConfig::virtualScreenMaxSize(), QImage::Format_RGB32 );
  num_cpus = DSysConfig::numberCPUs();

  for (int i=0; i<num_cpus; ++i) {
    render_thread_pool.append( new DRenderThread() );
    render_interp_thread_pool.append( new DRenderThreadInterp() );
  }
}

WVImageManager::~WVImageManager() {
  for (int i=0; i<render_thread_pool.size(); ++i)
    delete render_thread_pool[i];

  for (int i=0; i<render_interp_thread_pool.size(); ++i)
    delete render_interp_thread_pool[i];
}

void WVImageManager::generateMetaDataText( ) {
  meta_text = "";

  // everything parsed
  QStringList ls;
  QString line;
  QHash<QString, QVariant>::const_iterator it = tags.begin();
  while ( it != tags.end() ) {
    if (it.key() != "heap" && !it.key().startsWith("custom/") && !it.key().startsWith("raw/") ) {
      line = it.key();
      line += ": <b>";
      line += it.value().toString();
      line += "</b>";
      ls << line;
    }
    ++it;
  }
  ls.sort();
  meta_text += ls.join("<br />");

  // include custom tags
  meta_text += "<h3>Custom</h3>";
  it = tags.begin();
  while ( it != tags.end() ) {
    if (it.key().startsWith("custom/")) {
      line = it.key();
      line += ": <b>";
      line += it.value().toString();
      line += "</b><br>";
      meta_text += line;
    }
    ++it;
  }


  // include raw tags


  // include heap tag
  it = tags.find( "heap" ); 
  if (!it.value().toString().isEmpty()) {
    meta_text += "<h3>";
    meta_text += it.key();
    meta_text += "</h3><pre>";
    meta_text += it.value().toString();
    meta_text += "</pre>";
  }
}

bool WVImageManager::startImageReadSession( const QString &fileName ) {
  
  if (format_manager_filename == fileName) return true;
  format_manager.sessionEnd();
  format_manager.setCallbacks( progress_proc, error_proc, test_abort_proc );
  QByteArray ba = fileName.toLatin1();
  if (format_manager.sessionStartRead( ba.data() ) == 0) {
    format_manager_filename = fileName;
    return true;
  } else {
    format_manager.sessionEnd();
    format_manager_filename = "";
    return false;
  }
}

bool WVImageManager::loadImage( const QString &fileName, int page ) {

  bool res = true;
  int page_to_load = (page < 0) ? 0 : page;

  TDimImage image;
  if (startImageReadSession( fileName ) == true) {

    format_manager.sessionReadImage( image.imageBitmap(), page_to_load );
    image = image.ensureTypedDepth(); // make sure image is power of two
    pyramid.createFrom( image );
    zoomSet( zoom, true );
    pages_in_image = format_manager.sessionGetNumberOfPages();

    // getting metadata fields
    format_manager.sessionParseMetaData( 0 );
    generateHash(fileName, &format_manager, &image);
    generateMetaDataText();

    pixel_size[0] = format_manager.getPixelSizeX();
    pixel_size[1] = format_manager.getPixelSizeY();
    pixel_size[2] = format_manager.getPixelSizeZ();

  } else res = false;

  if (image.depth()>8 && enhance_type == wv_lut_generator_default) 
    enhance_type = wv_lut_generator_data;

  // compute image stats: min, max, hist, LUTs, etc...
  computeImageStats();
  
  // initialize channel to Display mapping e.g. 123 -> RGB
  if (page < 0) {
    // DEFAULT LUT
    switch ( image.samples() ) {
      case 1:
        memcpy( channel_lut, channel_lut_gray, sizeof(int)*WVImageManager::NumDisplayChannels );
        break;
      case 2:
        memcpy( channel_lut, channel_lut_2ch, sizeof(int)*WVImageManager::NumDisplayChannels );
        break;
      default:
        memcpy( channel_lut, channel_lut_def, sizeof(int)*WVImageManager::NumDisplayChannels );
    }

    // lut saved in the file
    if ( format_manager.get_display_lut().size() == 3 ) {
      for (int i=0; i<3; ++i)
        channel_lut[i] = format_manager.get_display_lut()[i];
    }
  }

  // create enhancement mappings for 8/16 bit images
  recomputeLuts();

  // set textual names for channels of the image
  if (page < 0)
    getChannelNames( &format_manager );

  return res;
}

void WVImageManager::onLutsChanged() {
  recomputeLuts();
  emit repaintRequired();
}

void WVImageManager::clearImage( ) {
  pyramid.clear();
  pages_in_image = 0;
  tags.clear();
  meta_text = "";
}

void WVImageManager::computeImageStats( const QRect &roi, int zoom_level ) {

  zoom_level = dim::trim( -1*zoom_level, 0, 10000 );
  TDimImage *image = pyramid.imageAt(zoom_level);
  image_hist.resize( image->samples() );

  if (roi.width()==0 || roi.height()==0 ) {
    unsigned int num_pix = image->width() * image->height();
    for (int sample=0; sample<image->samples(); ++sample)
      image_hist[sample].newData( image->depth(), image->bits(sample), num_pix, image->pixelType() );    
  } else {
    unsigned int bottom = dim::trim<unsigned int>( roi.bottom(), 0, image->height()-1 );
    unsigned int width = dim::trim<unsigned int>( roi.width(), 0, image->width()-1-roi.x() );
    if (width>0 && bottom>0)
    for (int sample=0; sample<image->samples(); ++sample) { 
      image_hist[sample].clear();
      image_hist[sample].init(image->depth(), image->pixelType());
      for (int y=roi.y(); y<=bottom; ++y) {
        DIM_UCHAR *p = image->scanLine(sample, y) + image->bytesInPixels( roi.x() );
        image_hist[sample].updateStats( p, width );    
      }
      for (int y=roi.y(); y<=bottom; ++y) {
        DIM_UCHAR *p = image->scanLine(sample, y) + image->bytesInPixels( roi.x() );
        image_hist[sample].addData( p, width );    
      }
    } // for sample
  }
}

void WVImageManager::getChannelNames(TMetaFormatManager *fm) {
  // obtain names for channels
  channel_names.clear();

  // get default names
  switch ( pyramid.imageAt(0)->samples() ) {
    case 1:
      channel_names << "Hide";
      channel_names << "ch 1 (Luminance)";
      break;
    case 3:
      channel_names << "Hide";
      channel_names << "ch 1 (Red)";
      channel_names << "ch 2 (Green)";
      channel_names << "ch 3 (Blue)";
      break;
    default:
      channel_names << "Hide";
      for (int c=0; c<pyramid.imageAt(0)->samples(); ++c)
        channel_names << QObject::tr("ch %1").arg(c+1);
  }

  // if metadata is present
  if (fm == NULL) return;

  // check if we have channel names in metadata
  for (int c=0; c<fm->get_channel_names().size(); ++c)
    channel_names[c+1] = QObject::tr("ch %1 (%2)").arg(c+1).arg( fm->get_channel_names()[c].c_str() );  

  // now set channel names for live enhancements, it's a static pointer to QStrilgList, 
  // so the change will reflect all the enhancements
  lut_gen_full_range.setChannelNames( &channel_names );
}

bool WVImageManager::metaTextEmpty() const {
  return meta_text.isEmpty();
}
  
bool WVImageManager::imageEmpty() const {
  return pyramid.isEmpty();
}

bool WVImageManager::pixelSizeEmpty() const {
  return (pixel_size[0] == 0.0 && pixel_size[1] == 0.0); 
}

QVector<double> WVImageManager::pixelSize() const {
  return pixel_size;
}

QStringList WVImageManager::formatFilters() {

  TDimFormatManager fm; 
  std::string str = fm.getAllExtensions();
  QString qstr( str.c_str() );
  qstr = QString("*.") + qstr;
  qstr.chop(1);
  qstr.replace(QString("|"), QString("|*."));
  return qstr.split("|");
}

QString WVImageManager::dialogFilters() {

  TDimFormatManager fm; 
  std::string allext = fm.getAllExtensions();
  QString qall( "All images (*." );
  qall += allext.c_str();
  qall.replace(QString("|"), QString(" *."));
  qall += ");;";
  
  std::string str = fm.getQtFilters();
  QString qstr( str.c_str() );
  qstr.replace(QString("|"), QString(" *."));
  qstr += "All files (*.*)";
  return qall+qstr;
}

QString WVImageManager::getShortImageInfo() const {
  TDimImage *image = pyramid.imageAt(0);  
  
  QStringList pixel_type_str;
  pixel_type_str << "Undefined" << "Unsigned" << "Signed" << "Float";

  QString s;
  s.sprintf( "[x:%d y:%d z:%d t:%d] chan: %d/%dbit ", 
    image->width(), image->height(), image->numZ(), image->numT(), image->samples(), image->depth() );
  s += pixel_type_str[image->pixelType()];

  return s;
}

int WVImageManager::channelMapping( DisplayChannels dc ) { 
  return channel_lut[dc]+1;
}

void WVImageManager::setMapping( DisplayChannels dc, int ic ) { 
  
  channel_lut[dc] = ic-1;
}

unsigned int WVImageManager::viewWidth() const { 
  if (zoom <= 0) return pyramid.image()->width(); 
  double ratio = dim::power<double>(2, zoom);
  return dim::round<unsigned int>( (double)pyramid.imageAt(0)->width() * ratio );
}  

unsigned int WVImageManager::viewHeight() const { 
  if (zoom <= 0) return pyramid.image()->height();
  double ratio = dim::power<double>(2, zoom);
  return dim::round<unsigned int>( (double)pyramid.imageAt(0)->height() * ratio );
}


//------------------------------------------------------------------------------
// Rendering utils
//------------------------------------------------------------------------------

bool WVImageManager::allowFastDraw() {
  bool res = true;
  if (pyramid.image()->depth() != 8) res = false;
  if (enhance_type != wv_lut_generator_default) res = false;
  return res;
}

template <typename T>
inline unsigned char chvalDRT( T **src, const unsigned int &x, const unsigned int &c,  WVRenderParams *par ) {
  return (*par->intencity_luts)[ par->lut_map[c] ][ ((T*)src[c])[x] ];
}

template <typename T>
inline unsigned char chvalSCL( T **src, const unsigned int &x, const unsigned int &c,  WVRenderParams *par ) {
  return (*par->intencity_luts)[ par->lut_map[c] ][ dim::trim<unsigned char, float>( ((float)((T*)src[c])[x]-par->shift[c])*par->scale[c], 0, 255 ) ];
}

//------------------------------------------------------------------------------
// Rendering direct
//------------------------------------------------------------------------------

template <typename T, typename F>
void fill_pixels_to8bits ( QRgb *dest, T **src, unsigned int  &w, WVRenderParams *par, F func ) {
  if (!par->need_fusion) {
    for (unsigned int x=0; x<w; ++x) 
      dest[x] = qRgb ( func( src, x, WVImageManager::dcRed, par ), 
                       func( src, x, WVImageManager::dcGreen, par ), 
                       func( src, x, WVImageManager::dcBlue, par ) );
  } else {
    for (unsigned int x=0; x<w; ++x) {
      unsigned char r = func( src, x, WVImageManager::dcRed, par );
      unsigned char g = func( src, x, WVImageManager::dcGreen, par );
      unsigned char b = func( src, x, WVImageManager::dcBlue, par );
      unsigned char y = func( src, x, WVImageManager::dcYellow, par ); // R+G
      unsigned char p = func( src, x, WVImageManager::dcPurple, par ); // R+B
      unsigned char c = func( src, x, WVImageManager::dcCyan, par ); // B+G
      r = std::max( r, std::max( y, p ) );
      g = std::max( g, std::max( y, c ) );
      b = std::max( b, std::max( c, p ) );
      dest[x] = qRgb ( r, g, b );
    }
  } // with fusion
}

void fill_range( unsigned int y1, unsigned int y2, QImage *out_img, 
                 DIM_UCHAR *srcp[WVImageManager::NumDisplayChannels], unsigned int src_bytes_per_line[WVImageManager::NumDisplayChannels], 
                 unsigned int  &w, WVRenderParams *par ) {

  DIM_UCHAR *src[WVImageManager::NumDisplayChannels];
  for (int dc=0; dc<WVImageManager::NumDisplayChannels; ++dc)
    src[dc] = srcp[dc] + y1*src_bytes_per_line[dc];

  // now copy data
  for (unsigned int y=y1; y<y2; ++y) {
    QRgb *dest = (QRgb *) out_img->scanLine(y);

    if (par->in_img->depth()==8  && par->in_img->pixelType()==D_FMT_UNSIGNED)
      fill_pixels_to8bits ( dest, (DIM_UINT8 **)src, w, par, chvalDRT<DIM_UINT8> );
    else
    if (par->in_img->depth()==16 && par->in_img->pixelType()==D_FMT_UNSIGNED)
      fill_pixels_to8bits ( dest, (DIM_UINT16 **)src, w, par, chvalDRT<DIM_UINT16> );
    else
    if (par->in_img->depth()==32 && par->in_img->pixelType()==D_FMT_UNSIGNED)
      fill_pixels_to8bits ( dest, (DIM_UINT32 **)src, w, par, chvalSCL<DIM_UINT32> );
    else
    if (par->in_img->depth()==8 && par->in_img->pixelType()==D_FMT_SIGNED)
      fill_pixels_to8bits ( dest, (DIM_INT8 **)src, w, par, chvalSCL<DIM_INT8> );
    else
    if (par->in_img->depth()==16 && par->in_img->pixelType()==D_FMT_SIGNED)
      fill_pixels_to8bits ( dest, (DIM_INT16 **)src, w, par, chvalSCL<DIM_INT16> );
    else
    if (par->in_img->depth()==32 && par->in_img->pixelType()==D_FMT_SIGNED)
      fill_pixels_to8bits ( dest, (DIM_INT32 **)src, w, par, chvalSCL<DIM_INT32> );
    else
    if (par->in_img->depth()==32 && par->in_img->pixelType()==D_FMT_FLOAT)
      fill_pixels_to8bits ( dest, (DIM_FLOAT32 **)src, w, par, chvalSCL<DIM_FLOAT32> );
    else
    if (par->in_img->depth()==64 && par->in_img->pixelType()==D_FMT_FLOAT)
      fill_pixels_to8bits ( dest, (DIM_FLOAT64 **)src, w, par, chvalSCL<DIM_FLOAT64> );


    // advance the line in the source
    for (int dc=0; dc<WVImageManager::NumDisplayChannels; ++dc)
      src[dc] += src_bytes_per_line[dc];

  } // for y  
}

QImage* WVImageManager::getDisplayRoi( QRect r ) {

  if (pyramid.isEmpty()) {
    // here just paint black the requested region
    // TODO
    screen_buffer.fill( 0 );
    return &screen_buffer; 
  }

  if (zoom > 0) return getDisplayRoiInterpolated( r );

  TDimImage *image = pyramid.image();

  // fill in the 32bit ARGB image
  unsigned int t = dim::trim<unsigned int>( r.top(), 0, image->height() );
  unsigned int l = dim::trim<unsigned int>( r.left(), 0, image->width() );
  unsigned int w = dim::trim<unsigned int>( r.width(), 1, image->width() );
  unsigned int h = dim::trim<unsigned int>( r.height(), 1, image->height() );

  unsigned int src_byte_offset = t*image->bytesPerLine() + image->bytesInPixels(l);
  DimBuffer line_black( image->bytesPerLine(), 0 );


  DIM_UCHAR *src[NumDisplayChannels]; // R, G, B, A, R+G=Yellow, R+B=Purple, B+G=Cyan
  unsigned int src_bytes_per_line[NumDisplayChannels];
  unsigned int lut_map[NumDisplayChannels]; // R, G, B, A, R+G=Yellow, R+B=Purple, B+G=Cyan 

  // RGB output
  render_params.need_fusion = false;
  for (int dc=0; dc<NumDisplayChannels; ++dc)
  if (channel_lut[dc] >= 0) {
    src[dc] = ((DIM_UCHAR*)image->bits( channel_lut[dc] )) + src_byte_offset;
    src_bytes_per_line[dc] = image->bytesPerLine();
    lut_map[dc] = channel_lut[dc];
    if (dc==dcYellow || dc==dcPurple || dc==dcCyan) render_params.need_fusion = true;
  } else {
    src[dc] = line_black.bytes();
    src_bytes_per_line[dc] = 0;
    lut_map[dc] = image->samples();  
  }

  // precompute dynamic LUT if needed
  if (enhance_area == deaOnlyVisible) {
    computeImageStats( QRect(l,t,w,h), zoom );
    recomputeLuts();
  }


  render_params.fast_copy = allowFastDraw();
  render_params.lut_map = lut_map;
  render_params.intencity_luts = &intencity_luts;
  render_params.in_img = image;
  render_params.channel_lut = channel_lut;
  render_params.image_hist = &image_hist;
  render_params.init();

  // If we have multiple CPUs parallelize rendering, otherwise don't bother creating threads
  if (num_cpus>1 && w>64 && h>32) {
    unsigned int pos_start = 0;
    unsigned int part = ceil( (double)h/(double)num_cpus );
    for (int i=0; i<render_thread_pool.size(); ++i) {
      unsigned int pos_stop = std::min( pos_start+part, h );
      render_thread_pool[i]->render( pos_start, pos_stop, &screen_buffer, src, src_bytes_per_line, w, &render_params );
      render_thread_pool[i]->start();
      pos_start += part;
      if (pos_start>=h) break;
    }
    for (int i=0; i<render_thread_pool.size(); ++i)
      render_thread_pool[i]->wait();

  } else
   fill_range( 0, h, &screen_buffer, src, src_bytes_per_line, w, &render_params );

  return &screen_buffer;
}

//------------------------------------------------------------------------------
// Rendering interpolated
//------------------------------------------------------------------------------

template <typename T, typename F>
void fill_pixels_to8bitsInterpNN ( QRgb *dest, T **src, unsigned int  &w, unsigned int &l, float &ratio, 
                                   void **psrc1, void **psrc2, float wghty[3], WVRenderParams *par, F func ) {
  if (!par->need_fusion) {
    for (unsigned int x=0; x<w; ++x) { 
      unsigned int src_x = dim::trim<unsigned int>(dim::round<unsigned int>((x+l)/ratio), 0, par->in_img->width()-1);
      dest[x] = qRgb ( func( src, src_x, WVImageManager::dcRed, par ),
                       func( src, src_x, WVImageManager::dcGreen, par ), 
                       func( src, src_x, WVImageManager::dcBlue, par ) );
    }
  } else {
    for (unsigned int x=0; x<w; ++x) { 
      unsigned int src_x = dim::trim<unsigned int>(dim::round<unsigned int>((x+l)/ratio), 0, par->in_img->width()-1);
      unsigned char r = func( src, src_x, WVImageManager::dcRed, par );
      unsigned char g = func( src, src_x, WVImageManager::dcGreen, par );
      unsigned char b = func( src, src_x, WVImageManager::dcBlue, par );
      unsigned char y = func( src, src_x, WVImageManager::dcYellow, par ); // R+G
      unsigned char p = func( src, src_x, WVImageManager::dcPurple, par ); // R+B
      unsigned char c = func( src, src_x, WVImageManager::dcCyan, par ); // B+G
      r = std::max( r, std::max( y, p ) );
      g = std::max( g, std::max( y, c ) );
      b = std::max( b, std::max( c, p ) );
      dest[x] = qRgb ( r, g, b );
    }
  } // with fusion
}

template <typename T, typename F>
void fill_pixels_to8bitsInterpBL ( QRgb *dest, T **src, unsigned int  &w, unsigned int &l,
                                   float &ratio, T **src1, T **src2, float wghty[3], WVRenderParams *par, F func ) {

  int num_channels = 3;
  if (par->need_fusion) num_channels = WVImageManager::NumDisplayChannels;
  for (unsigned int x=0; x<w; ++x) { 
    unsigned int xi = dim::round<unsigned int>((x+l)/ratio);
    unsigned int src_x  = dim::trim<unsigned int>(xi,   0, par->in_img->width()-1);
    unsigned int src_x1 = dim::trim<unsigned int>(xi+1, 0, par->in_img->width()-1);
    unsigned int src_x2 = dim::trim<unsigned int>(xi-1, 0, par->in_img->width()-1);

    // compute weights
    float wght[3];
    float mw = 1.5*ratio;
    wght[0] = (mw - fabs( ((float)src_x2)*ratio - (float)(x+l) ));
    wght[1] = (mw - fabs( ((float)src_x) *ratio - (float)(x+l) ));
    wght[2] = (mw - fabs( ((float)src_x1)*ratio - (float)(x+l) ));
    // normalize
    float density = wght[0]+wght[1]+wght[2];
    for (int i=0; i<3; ++i) wght[i] /= density;

    unsigned char vx[3];
    unsigned char vy[3];
    unsigned char rgbaypc[WVImageManager::NumDisplayChannels];
    for (unsigned int ch=0; ch<num_channels; ++ch) {
      vy[0] = func( src,  src_x2, ch, par );
      vy[1] = func( src1, src_x2, ch, par );
      vy[2] = func( src2, src_x2, ch, par );
      vx[0] = ((float)vy[0])*wghty[0] + ((float)vy[1])*wghty[1] + ((float)vy[2])*wghty[2];

      vy[0] = func( src,  src_x, ch, par );
      vy[1] = func( src1, src_x, ch, par );
      vy[2] = func( src2, src_x, ch, par );
      vx[1] = ((float)vy[0])*wghty[0] + ((float)vy[1])*wghty[1] + ((float)vy[2])*wghty[2];

      vy[0] = func( src,  src_x1, ch, par );
      vy[1] = func( src1, src_x1, ch, par );
      vy[2] = func( src2, src_x1, ch, par );
      vx[2] = ((float)vy[0])*wghty[0] + ((float)vy[1])*wghty[1] + ((float)vy[2])*wghty[2];

      rgbaypc[ch] = (unsigned char)( ((float)vx[0])*wght[0] + ((float)vx[1])*wght[1] + ((float)vx[2])*wght[2] );
    } // for ch

    if (!par->need_fusion)
      dest[x] = qRgb ( rgbaypc[WVImageManager::dcRed], rgbaypc[WVImageManager::dcGreen], rgbaypc[WVImageManager::dcBlue] );
    else {
      unsigned char r = rgbaypc[WVImageManager::dcRed];
      unsigned char g = rgbaypc[WVImageManager::dcGreen];
      unsigned char b = rgbaypc[WVImageManager::dcBlue];
      unsigned char y = rgbaypc[WVImageManager::dcYellow]; // R+G
      unsigned char p = rgbaypc[WVImageManager::dcPurple]; // R+B
      unsigned char c = rgbaypc[WVImageManager::dcCyan]; // B+G
      r = std::max( r, std::max( y, p ) );
      g = std::max( g, std::max( y, c ) );
      b = std::max( b, std::max( c, p ) );
      dest[x] = qRgb ( r, g, b );
    } // with fusion
  } // for x
}

void fill_range_interp( unsigned int y1, unsigned int y2, QImage *out_img,
                 DIM_UCHAR *srcp[WVImageManager::NumDisplayChannels], unsigned int src_bytes_per_line[WVImageManager::NumDisplayChannels],
                 unsigned int &w, unsigned int &t, unsigned int &l, float &scale, 
                 WVImageManager::DisplayZoomMethod zoom_method, WVRenderParams *par ) {

  DIM_UCHAR *src[WVImageManager::NumDisplayChannels];
  DIM_UCHAR *src1[WVImageManager::NumDisplayChannels];
  DIM_UCHAR *src2[WVImageManager::NumDisplayChannels];

  // now copy data
  for (int y=y1; y<y2; ++y) {
    QRgb *dest = (QRgb *) out_img->scanLine(y);

    // advance the line in the source
    float init_y = (t+y)/scale;

    unsigned int init_yi = dim::trim<unsigned int>(dim::round<unsigned int>(init_y), 0, par->in_img->height()-1);
    for (int dc=0; dc<WVImageManager::NumDisplayChannels; ++dc)
      src[dc] = srcp[dc] + src_bytes_per_line[dc] * init_yi;

    float wght[3];

    // compute weights
    if (zoom_method == WVImageManager::dzmBL) {

      unsigned int init_yi1 = dim::trim<unsigned int>(dim::round<unsigned int>(init_y)+1, 0, par->in_img->height()-1);
      for (int dc=0; dc<WVImageManager::NumDisplayChannels; ++dc)
        src1[dc] = srcp[dc] + src_bytes_per_line[dc] * init_yi1;

      unsigned int init_yi2 = dim::trim<unsigned int>(dim::round<unsigned int>(init_y)-1, 0, par->in_img->height()-1);
      for (int dc=0; dc<WVImageManager::NumDisplayChannels; ++dc)
        src2[dc] = srcp[dc] + src_bytes_per_line[dc] * init_yi2;

      float mw = 1.5*scale;
      wght[0] = (mw - fabs( ((float)init_yi)*scale - (t+y) ));
      wght[1] = (mw - fabs( ((float)init_yi1)*scale - (t+y) ));
      wght[2] = (mw - fabs( ((float)init_yi2)*scale - (t+y) ));

      // normalize
      float density = wght[0]+wght[1]+wght[2];
      for (int i=0; i<3; ++i)
        wght[i] /= density;
    }

    if (zoom_method == WVImageManager::dzmBL) {
      if (par->in_img->depth()==8  && par->in_img->pixelType()==D_FMT_UNSIGNED)
        fill_pixels_to8bitsInterpBL ( dest, (DIM_UINT8 **)src,   w, l, scale, (DIM_UINT8 **)src1,  (DIM_UINT8 **)src2,  wght, par, chvalDRT<DIM_UINT8> );
      else
      if (par->in_img->depth()==16 && par->in_img->pixelType()==D_FMT_UNSIGNED)
        fill_pixels_to8bitsInterpBL ( dest, (DIM_UINT16 **)src,  w, l, scale, (DIM_UINT16 **)src1, (DIM_UINT16 **)src2, wght, par, chvalDRT<DIM_UINT16> );
      else
      if (par->in_img->depth()==32 && par->in_img->pixelType()==D_FMT_UNSIGNED)
        fill_pixels_to8bitsInterpBL ( dest, (DIM_UINT32 **)src,  w, l, scale, (DIM_UINT32 **)src1, (DIM_UINT32 **)src2, wght, par, chvalSCL<DIM_UINT32> );
      else
      if (par->in_img->depth()==8  && par->in_img->pixelType()==D_FMT_SIGNED)
        fill_pixels_to8bitsInterpBL ( dest, (DIM_INT8 **)src,    w, l, scale, (DIM_INT8 **)src1, (DIM_INT8 **)src2, wght, par, chvalSCL<DIM_INT8> );
      else
      if (par->in_img->depth()==16 && par->in_img->pixelType()==D_FMT_SIGNED)
        fill_pixels_to8bitsInterpBL ( dest, (DIM_INT16 **)src,   w, l, scale, (DIM_INT16 **)src1, (DIM_INT16 **)src2, wght, par, chvalSCL<DIM_INT16> );
      else
      if (par->in_img->depth()==32 && par->in_img->pixelType()==D_FMT_SIGNED)
        fill_pixels_to8bitsInterpBL ( dest, (DIM_INT32 **)src,   w, l, scale, (DIM_INT32 **)src1, (DIM_INT32 **)src2, wght, par, chvalSCL<DIM_INT32> );
      else
      if (par->in_img->depth()==32 && par->in_img->pixelType()==D_FMT_FLOAT)
        fill_pixels_to8bitsInterpBL ( dest, (DIM_FLOAT32 **)src, w, l, scale, (DIM_FLOAT32 **)src1, (DIM_FLOAT32 **)src2, wght, par, chvalSCL<DIM_FLOAT32> );
      else
      if (par->in_img->depth()==64 && par->in_img->pixelType()==D_FMT_FLOAT)
        fill_pixels_to8bitsInterpBL ( dest, (DIM_FLOAT64 **)src, w, l, scale, (DIM_FLOAT64 **)src1, (DIM_FLOAT64 **)src2, wght, par, chvalSCL<DIM_FLOAT64> );
    } else
    if (zoom_method == WVImageManager::dzmNN) {
      if (par->in_img->depth()==8  && par->in_img->pixelType()==D_FMT_UNSIGNED)
        fill_pixels_to8bitsInterpNN ( dest, (DIM_UINT8 **)  src, w, l, scale, (void **)src1, (void **)src2, wght, par, chvalDRT<DIM_UINT8> );
      else
      if (par->in_img->depth()==16 && par->in_img->pixelType()==D_FMT_UNSIGNED)
        fill_pixels_to8bitsInterpNN ( dest, (DIM_UINT16 **) src, w, l, scale, (void **)src1, (void **)src2, wght, par, chvalDRT<DIM_UINT16> );
      else
      if (par->in_img->depth()==32 && par->in_img->pixelType()==D_FMT_UNSIGNED)
        fill_pixels_to8bitsInterpNN ( dest, (DIM_UINT32 **) src, w, l, scale, (void **)src1, (void **)src2, wght, par, chvalSCL<DIM_UINT32> );
      else
      if (par->in_img->depth()==8  && par->in_img->pixelType()==D_FMT_SIGNED)
        fill_pixels_to8bitsInterpNN ( dest, (DIM_INT8 **)   src, w, l, scale, (void **)src1, (void **)src2, wght, par, chvalSCL<DIM_INT8> );
      else
      if (par->in_img->depth()==16 && par->in_img->pixelType()==D_FMT_SIGNED)
        fill_pixels_to8bitsInterpNN ( dest, (DIM_INT16 **)  src, w, l, scale, (void **)src1, (void **)src2, wght, par, chvalSCL<DIM_INT16> );
      else
      if (par->in_img->depth()==32 && par->in_img->pixelType()==D_FMT_SIGNED)
        fill_pixels_to8bitsInterpNN ( dest, (DIM_INT32 **)  src, w, l, scale, (void **)src1, (void **)src2, wght, par, chvalSCL<DIM_INT32> );
      else
      if (par->in_img->depth()==32 && par->in_img->pixelType()==D_FMT_FLOAT)
        fill_pixels_to8bitsInterpNN ( dest, (DIM_FLOAT32 **)src, w, l, scale, (void **)src1, (void **)src2, wght, par, chvalSCL<DIM_FLOAT32> );
      else
      if (par->in_img->depth()==64 && par->in_img->pixelType()==D_FMT_FLOAT)
        fill_pixels_to8bitsInterpNN ( dest, (DIM_FLOAT64 **)src, w, l, scale, (void **)src1, (void **)src2, wght, par, chvalSCL<DIM_FLOAT64> );
    }
  } // for y  
}

QImage *WVImageManager::getDisplayRoiInterpolated( QRect r ) {

  if (pyramid.isEmpty() || zoom <= 0) {
    screen_buffer.fill( 0 );
    return &screen_buffer; 
  }
  TDimImage *image = pyramid.imageAt(0);
  
  // fill in the 32bit ARGB image
  float scale = dim::power<double>(2, zoom);  
  unsigned int t = dim::trim<unsigned int>( r.top(), 0, image->height()*scale );
  unsigned int l = dim::trim<unsigned int>( r.left(), 0, image->width()*scale );
  unsigned int w = dim::trim<unsigned int>( r.width(), 1, image->width()*scale );
  unsigned int h = dim::trim<unsigned int>( r.height(), 1, image->height()*scale );

  DimBuffer line_black( image->bytesPerLine(), 0 );

  DIM_UCHAR *src[NumDisplayChannels]; // R, G, B, A, R+G=Yellow, R+B=Purple, B+G=Cyan
  DIM_UCHAR *srcp[NumDisplayChannels]; // RGBA
  unsigned int src_bytes_per_line[NumDisplayChannels];
  unsigned int lut_map[NumDisplayChannels];

  render_params.need_fusion = false;
  for (int dc=0; dc<NumDisplayChannels; ++dc)
  if (channel_lut[dc] >= 0) {
    src[dc] = ((DIM_UCHAR*)image->bits( channel_lut[dc] ));
    src_bytes_per_line[dc] = image->bytesPerLine();
    lut_map[dc] = channel_lut[dc];
    srcp[dc] = src[dc];
    if (dc==dcYellow || dc==dcPurple || dc==dcCyan) render_params.need_fusion = true;
  } else {
    src[dc] = line_black.bytes();
    src_bytes_per_line[dc] = 0;
    lut_map[dc] = image->samples(); 
    srcp[dc] = src[dc];
  }

  // precompute dynamic LUT if needed
  if (enhance_area == deaOnlyVisible) {
    unsigned int ti = dim::trim<unsigned int>( dim::round<unsigned int>(r.top()/scale), 0, image->height()-1 );
    unsigned int li = dim::trim<unsigned int>( dim::round<unsigned int>(r.left()/scale), 0, image->width()-1 );
    unsigned int wi = dim::trim<unsigned int>( dim::round<unsigned int>(r.width()/scale), 0, image->width()-1 );
    unsigned int hi = dim::trim<unsigned int>( dim::round<unsigned int>(r.height()/scale), 0, image->height()-1 );
    computeImageStats( QRect(li,ti,wi,hi), 0 );
    recomputeLuts();
  }

  render_params.fast_copy = allowFastDraw();
  render_params.lut_map = lut_map;
  render_params.intencity_luts = &intencity_luts;
  render_params.in_img = image;
  render_params.channel_lut = channel_lut;
  render_params.image_hist = &image_hist;
  render_params.init();

  // If we have multiple CPUs parallelize rendering, otherwise don't bother creating threads
  if (num_cpus>1 && w>64 && h>32) {
    unsigned int pos_start = 0;
    unsigned int part = ceil( (double)h/(double)num_cpus );
    for (int i=0; i<render_interp_thread_pool.size(); ++i) {
      unsigned int pos_stop = std::min( pos_start+part, h );
      render_interp_thread_pool[i]->render( pos_start, pos_stop, &screen_buffer, src, src_bytes_per_line, w, t, l, scale, zoom_method, &render_params );
      render_interp_thread_pool[i]->start();
      pos_start += part;
      if (pos_start>=h) break;
    }
    for (int i=0; i<render_interp_thread_pool.size(); ++i)
      render_interp_thread_pool[i]->wait();

  } else
   fill_range_interp( 0, h, &screen_buffer, src, src_bytes_per_line, w, t, l, scale, zoom_method, &render_params );

  return &screen_buffer;
}

/*
void fill_range_to_16( unsigned int y1, unsigned int y2, TDimImage *in_img, QImage *out_img, bool fast_copy,
                 DIM_UCHAR *srcp[4], unsigned int src_bytes_per_line[4],
                 unsigned int  &w, const unsigned int lut_map[4], 
                 const std::vector< std::vector<unsigned char> > &intencity_luts, WVRenderParams *par ) {

  DIM_UCHAR *src[4];
  for (int dc=0; dc<3; ++dc)
    src[dc] = srcp[dc] + y1*src_bytes_per_line[dc];

  // now copy data
  for (unsigned int y=y1; y<y2; ++y) {
    QRgb *dest = (QRgb *) out_img->scanLine(y);

    if (in_img->depth() == 8)
      fill_pixels_to8bits<DIM_UCHAR> ( dest, (void **)src, w, lut_map, intencity_luts, par );
    else
    if (in_img->depth() == 16)
      fill_pixels_to8bits<DIM_UINT16> ( dest, (void **)src, w, lut_map, intencity_luts, par );

    // advance the line in the source
    for (int dc=0; dc<3; ++dc)
      src[dc] += src_bytes_per_line[dc];

  } // for y  
}

//kpixmapio.cpp KPixmapIO

void* WVImageManager::getDisplayRoi16bitX11( QRect r ) {


  if (pyramid.isEmpty()) {
    // here just paint black the requested region
    // TODO
    screen_buffer.fill( 0 );
    return &screen_buffer; 
  }

  if (zoom > 0) return getDisplayRoiInterpolated( r );

  TDimImage *image = pyramid.image();

  // fill in the 32bit ARGB image
  unsigned int t = dim::trim<unsigned int>( r.top(), 0, image->height() );
  unsigned int l = dim::trim<unsigned int>( r.left(), 0, image->width() );
  unsigned int w = dim::trim<unsigned int>( r.width(), 1, image->width() );
  unsigned int h = dim::trim<unsigned int>( r.height(), 1, image->height() );

  unsigned int src_byte_offset = t*image->bytesPerLine() + image->bytesInPixels(l);
  DimBuffer line_black( image->bytesPerLine(), 0 );


  DIM_UCHAR *src[4]; // RGBA
  unsigned int src_bytes_per_line[4];
  unsigned int lut_map[4];

  for (int dc=0; dc<3; ++dc)
  if (channel_lut[dc] >= 0) {
    src[dc] = ((DIM_UCHAR*)image->bits( channel_lut[dc] )) + src_byte_offset;
    src_bytes_per_line[dc] = image->bytesPerLine();
    lut_map[dc] = channel_lut[dc];
  } else {
    src[dc] = line_black.bytes();
    src_bytes_per_line[dc] = 0;
    lut_map[dc] = 0;
  }

  // precompute dynamic LUT if needed

  render_params.fast_copy = allowFastDraw();
  render_params.lut_map = lut_map;
  render_params.in_img = image;
  render_params.channel_lut = channel_lut;
  render_params.image_hist = &image_hist;
  render_params.init();

  fill_range_to_16( 0, h, image, &screen_buffer, allowFastDraw(), src, src_bytes_per_line, w, lut_map, intencity_luts, &render_params );

  return screen_buffer.bits();
}
*/

QPixmap WVImageManager::getThumbnail( const QSize &s ) {
  
  if (pyramid.isEmpty()) return QPixmap(0,0);

  // first find closest pyramid level
  int level_thumb = pyramid.levelClosestTop( s.width(), s.height() );

  // get ROI of all image of that level
  int level_orig  = zoom;
  zoomSet( -level_thumb );
  int wo = pyramid.image()->width();
  int ho = pyramid.image()->height(); 
  QImage img = getDisplayRoi( QRect(0,0,wo,ho) )->copy( 0, 0, wo, ho );
  zoomSet( level_orig );

  // interpolate into the final pixmap
  //Qt::FastTransformation Qt::SmoothTransformation
  return QPixmap::fromImage( img.scaled( s.width(), s.height(), Qt::KeepAspectRatio, Qt::SmoothTransformation ) );
}

//------------------------------------------------------------------------------
// Stats
//------------------------------------------------------------------------------

void WVImageManager::recomputeLuts() {
  if (pyramid.isEmpty()) return;
  TDimImage *image = pyramid.imageAt(0);

  // the lut for additional channel is used for hidden channels during visualization
  //int lut_size = pow( 2.0, (int) image->depth() );
  int lut_size = image_hist[0].size();
  intencity_luts.resize( image->samples()+1 );
  for (int c=0; c<image->samples()+1; ++c)
    intencity_luts[c].resize( lut_size );

  // initialize the empty channel lut
  int disp_range = 256;
  linear_full_range_generator( image_hist[0], intencity_luts[image->samples()], disp_range, 0 );

  //------------------------------------------------------
  // run current lut_generator
  //------------------------------------------------------
  WVLutGenerator *lut_generator = lut_generators[(int)enhance_type];
  lut_generator->lutsGenerate( image_hist, intencity_luts );

  //------------------------------------------------------
  // run enabled lut_modifiers
  //------------------------------------------------------
  for (unsigned int m=0; m<lut_modifiers.size(); ++m)
    if ( lut_modifiers[m]->isEnabled() )
      lut_modifiers[m]->lutsModify( image_hist, intencity_luts );
}

void WVImageManager::setCurrentEnhancement(DisplayLiveEnhancemet et) {
  enhance_type = et;
  recomputeLuts();
}

bool WVImageManager::zoomOut() {
  if (zoom > 0) {
    --zoom;
    return true;
  }
  else {
    bool res = pyramid.levelDown();
    if (res) --zoom;
    return res;
  }
}

bool WVImageManager::zoomIn() {
  ++zoom;
  if (zoom > 0) return true;
  return pyramid.levelUp();
}

bool WVImageManager::zoom11() {
  if (zoom == 0) return false;
  zoom=0;
  return pyramid.level0();
}

bool WVImageManager::zoomSet( int _zoom, bool force ) {
  if (zoom == _zoom && !force) return false;  

  if (_zoom > 0) {
    pyramid.levelSet( 0 );
    zoom = _zoom;
    return true;
  }
  else {
    bool res = pyramid.levelSet( -1*_zoom );
    if (res) zoom = _zoom;
    return res;
  }

  return false;
}

int WVImageManager::zoomClosestSmaller( unsigned int w, unsigned int h) const {
  return zoomClosestSmaller( QSize(w,h) );
}

int WVImageManager::zoomClosestSmaller( const QSize &size ) const {
  if ( (size.width()>imageWidth()) && (size.height()>imageHeight()) ) {
    double scaleW = (double)size.width()/(double)imageWidth();
    double scaleH = (double)size.height()/(double)imageHeight();
    return dim::log2<int>( (int) floor( dim::min<double>( scaleW, scaleH ) ) );
  }

  return ( -1 * pyramid.levelClosestBottom( size.width(), size.height()) );
}
  
double WVImageManager::currentViewScale() const {
  return dim::power<double>(2, zoom);
}

QPointF WVImageManager::viewToImage( const QPointF &p ) const {
  QPointF p_res;
  double ratio = dim::power<double>(2, zoom);
  p_res.setX( dim::trim<double>( (double)p.x() / ratio, 0.0, (double)imageWidth()-1.0 ) );
  p_res.setY( dim::trim<double>( (double)p.y() / ratio, 0.0, (double)imageHeight()-1.0 ) );
  return p_res;
}

QPointF WVImageManager::imageToView( const QPointF &p ) const {
  QPointF p_res;
  double ratio = dim::power<double>(2, zoom);
  p_res.setX( dim::trim<double>( (double)p.x() * ratio, 0.0, (double)viewWidth()-1.0 ) );
  p_res.setY( dim::trim<double>( (double)p.y() * ratio, 0.0, (double)viewHeight()-1.0 ) );
  return p_res;
}

QRectF WVImageManager::viewToImage( const QRectF &r ) const {
  QRectF ro = r;
  ro.setTopLeft     ( viewToImage( r.topLeft() ) );
  ro.setBottomRight ( viewToImage( r.bottomRight() ) );
  return ro;
}

QRectF WVImageManager::imageToView( const QRectF &r ) const {
  QRectF ro = r;
  ro.setTopLeft     ( imageToView( r.topLeft() ) );
  ro.setBottomRight ( imageToView( r.bottomRight() ) );
  return ro;
}


QVariant WVImageManager::getTagValue( const QString &tag ) const {
  return tags.value( tag );
}

void WVImageManager::generateHash( const QString &fileName, TMetaFormatManager *fm, TDimImage *img ) {

  tags.clear();

  // All metadata
  QString meta_text = fm->get_text_metadata().c_str();
  meta_text = meta_text.replace(QRegExp("\n{3,}"), "\n\n");
  tags.insert( "heap", QVariant(meta_text) );
  
  const std::map<std::string, std::string> metadata = fm->get_metadata();
  std::map<std::string, std::string>::const_iterator it;
  for(it = metadata.begin(); it != metadata.end(); ++it)
    tags.insert( (*it).first.c_str(), (*it).second.c_str() );

  if (fileName != "") {
    // insert tags for auto fill: file_name and file_path
    QFileInfo fi( fileName );
    tags.insert( "file_path", fi.absolutePath() );
    tags.insert( "file_name", fi.fileName() );
  }
}

QStringList WVImageManager::enhancementTypes() { 
  QStringList enhancement_types;
  for (unsigned i=0; i<lut_generators.size(); ++i)
    enhancement_types << lut_generators[i]->name();
  return enhancement_types; 
}

void WVImageManager::rotateImage( double deg ) {
  int zm = zoomCurrent();
  TDimImage ri = *pyramid.imageAt(0);
  ri = ri.rotate(deg);
  pyramid.createFrom(ri);
  zoomSet( zm, true );
}

void WVImageManager::projectMax() {
  int zm = zoomCurrent();
  QString fileName = tags.value("file_path", "").toString() + "/" + tags.value("file_name", "").toString();

  TDimImage image, image_loaded;
  TMetaFormatManager fm;
  fm.setCallbacks( progress_proc, error_proc, test_abort_proc );

  if (fm.sessionStartRead( fileName.toLatin1().data() ) == 0) {
    int num_pages = fm.sessionGetNumberOfPages();

    for (int page=0; page<num_pages; ++page) {
      int res = fm.sessionReadImage( image_loaded.imageBitmap(), page );
      image_loaded = image_loaded.ensureTypedDepth(); // make sure image is power of two
      if (image.isNull()) 
        image = image_loaded.deepCopy();
      else
        image.pixelArithmeticMax( image_loaded );
    }
  }

  fm.sessionEnd();

  pyramid.createFrom(image);
  zoomSet( zm, true );

  // compute image stats: min, max, hist, LUTs, etc...
  computeImageStats();
 
  // create enhancement mappings for 8/16 bit images
  recomputeLuts();
}

void WVImageManager::projectMin() {
  int zm = zoomCurrent();
  QString fileName = tags.value("file_path", "").toString() + "/" + tags.value("file_name", "").toString();

  TDimImage image, image_loaded;
  TMetaFormatManager fm;
  fm.setCallbacks( progress_proc, error_proc, test_abort_proc );

  if (fm.sessionStartRead( fileName.toLatin1().data() ) == 0) {
    int num_pages = fm.sessionGetNumberOfPages();

    for (int page=0; page<num_pages; ++page) {
      int res = fm.sessionReadImage( image_loaded.imageBitmap(), page );
      image_loaded = image_loaded.ensureTypedDepth(); // make sure image is power of two
      if (image.isNull()) 
        image = image_loaded.deepCopy();
      else
        image.pixelArithmeticMin( image_loaded );
    }
  }

  fm.sessionEnd();

  pyramid.createFrom(image);
  zoomSet( zm, true );

  // compute image stats: min, max, hist, LUTs, etc...
  computeImageStats();
 
  // create enhancement mappings for 8/16 bit images
  recomputeLuts();
}


//------------------------------------------------------------------------------
// DRenderThread
//------------------------------------------------------------------------------


void DRenderThread::render( unsigned int _y1, unsigned int _y2, QImage *_out_img, 
                            DIM_UCHAR **srcp, unsigned int _src_bytes_per_line[4], unsigned int &_w, 
                            WVRenderParams *_render_params ) {
  src = srcp;
  y1 = _y1; 
  y2 = _y2; 
  out_img = _out_img;
  src_bytes_per_line = _src_bytes_per_line;
  w = _w;
  render_params = _render_params;
}

void DRenderThread::run() {
  fill_range( y1, y2, out_img, src, src_bytes_per_line, w, render_params );
}

//------------------------------------------------------------------------------
// DRenderThreadInterp
//------------------------------------------------------------------------------

void DRenderThreadInterp::render( unsigned int _y1, unsigned int _y2, QImage *_out_img, 
                 DIM_UCHAR *srcp[4], unsigned int _src_bytes_per_line[4],
                 unsigned int &_w, unsigned int &_t, unsigned int &_l, float &_scale, 
                 WVImageManager::DisplayZoomMethod zm, WVRenderParams *_render_params ) {
  src = srcp;
  y1 = _y1; 
  y2 = _y2; 
  out_img = _out_img;
  src_bytes_per_line = _src_bytes_per_line;
  w = _w;
  t = _t;
  l = _l;
  scale = _scale;
  zoom_method = zm;
  render_params = _render_params;
}

void DRenderThreadInterp::run() {
  fill_range_interp( y1, y2, out_img, src, src_bytes_per_line, w, t, l, scale, zoom_method, render_params );
}



//------------------------------------------------------------------------------
// WVLiveEnhancement
//------------------------------------------------------------------------------
QStringList* WVLiveEnhancement::channel_names = 0;

WVLiveEnhancement::WVLiveEnhancement() {
  enhancement_name = "noname";
  display_bpp = 8;
  enabled = false;
}

void WVLiveEnhancement::onParamsChanged() {
  emit changed();
}

void WVLiveEnhancement::setEnabled( bool v ) { 
  if (enabled == v) return;
  enabled = v; 
  emit changed();
}

//------------------------------------------------------------------------------
// WVLutGenerator
//------------------------------------------------------------------------------
void WVLutGenerator::lutsGenerate( const std::vector<DimHistogram> &image_histograms, 
                             std::vector< std::vector<unsigned char> > &image_luts ) 
{
  if (image_luts.size() < image_histograms.size() )
    image_luts.resize( image_histograms.size() );
  for (unsigned int c=0; c<image_histograms.size(); ++c)
    if (image_luts[c].size() < image_histograms[c].size() )
      image_luts[c].resize( image_histograms[c].size() );
}


//------------------------------------------------------------------------------
// WVLutGenerator specifications
//------------------------------------------------------------------------------

void WVLinearFullRangeLutGenerator::lutsGenerate( const std::vector<DimHistogram> &image_histograms, 
                             std::vector< std::vector<unsigned char> > &image_luts ) 
{
  WVLutGenerator::lutsGenerate( image_histograms, image_luts );
  unsigned int out_phys_range = (unsigned int) pow( 2.0f, (int)abs((int)display_bpp) );
  for (unsigned int c=0; c<image_histograms.size(); ++c)
    linear_full_range_generator<unsigned char>( image_histograms[c], image_luts[c], out_phys_range, 0 );
}

void WVLinearDataRangeLutGenerator::lutsGenerate( const std::vector<DimHistogram> &image_histograms, 
                             std::vector< std::vector<unsigned char> > &image_luts ) 
{
  WVLutGenerator::lutsGenerate( image_histograms, image_luts );
  unsigned int out_phys_range = (unsigned int) pow( 2.0f, (int)abs((int)display_bpp) );
  for (unsigned int c=0; c<image_histograms.size(); ++c)
    linear_data_range_generator<unsigned char>( image_histograms[c], image_luts[c], out_phys_range, 0 );
}

void WVLinearDataToleranceLutGenerator::lutsGenerate( const std::vector<DimHistogram> &image_histograms, 
                             std::vector< std::vector<unsigned char> > &image_luts ) 
{
  WVLutGenerator::lutsGenerate( image_histograms, image_luts );
  unsigned int out_phys_range = (unsigned int) pow( 2.0f, (int)abs((int)display_bpp) );
  for (unsigned int c=0; c<image_histograms.size(); ++c)
    linear_data_tolerance_generator<unsigned char>( image_histograms[c], image_luts[c], out_phys_range, 0 );
}

void WVLinearEqualizedLutGenerator::lutsGenerate( const std::vector<DimHistogram> &image_histograms, 
                             std::vector< std::vector<unsigned char> > &image_luts ) 
{
  WVLutGenerator::lutsGenerate( image_histograms, image_luts );
  unsigned int out_phys_range = (unsigned int) pow( 2.0f, (int)abs((int)display_bpp) );
  for (unsigned int c=0; c<image_histograms.size(); ++c)
    equalized_generator<unsigned char>( image_histograms[c], image_luts[c], out_phys_range, 0 );
}


//------------------------------------------------------------------------------
// WVNegativeLutModifier
//------------------------------------------------------------------------------

void WVNegativeLutModifier::lutsModify( const std::vector<DimHistogram> &image_histograms, 
                                        std::vector< std::vector<unsigned char> > &image_luts ) 
{
  // must to be inited
  unsigned int out_phys_range = (unsigned int) pow( 2.0f, (int)abs((int)display_bpp) );

  for (unsigned int c=0; c<image_histograms.size(); ++c)
    for (unsigned int p=0; p<image_luts[c].size(); ++p)
      image_luts[c][p] = out_phys_range - image_luts[c][p] - 1 ;
}

//------------------------------------------------------------------------------
// WVBrightnessContrastEnhancement
//------------------------------------------------------------------------------

void WVBrightnessContrastLutModifier::lutsModify( const std::vector<DimHistogram> &image_histograms, 
                                        std::vector< std::vector<unsigned char> > &image_luts ) {
 
  unsigned int out_phys_range = (unsigned int) pow( 2.0f, (int)abs((int)display_bpp) );
  float brightness = 0; if (conf_widget) brightness = conf_widget->brightness();
  float contrast   = 0; if (conf_widget) contrast = conf_widget->contrast();
  std::vector<float> b( image_histograms.size() );

  std::vector<DimHistogram> hist( image_histograms.size() );
  for (unsigned int c=0; c<image_histograms.size(); ++c) {
    DimLut lut;
    hist[c].init( display_bpp );
    lut.init( image_histograms[c], hist[c] );
    lut.set_lut( image_luts[c] );
    lut.apply( image_histograms[c], hist[c] );
  }

  if (brightness>0) {
    for (unsigned int c=0; c<image_histograms.size(); ++c)
      b[c] = (brightness/100.0)*(out_phys_range-1.0-(float)hist[c].first_pos());
  } else {
    for (unsigned int c=0; c<image_histograms.size(); ++c)
      b[c] = (brightness/100.0)*((float)hist[c].last_pos());
  }

  float delta, a;
  std::vector<float> bb( image_histograms.size() );
  for (unsigned int c=0; c<image_histograms.size(); ++c)
  if( contrast > 0 ) {
    delta = (out_phys_range/2.0-1.0)*contrast/100.0;
    a = (out_phys_range-1.0)/((out_phys_range-1.0) - delta*2.0);
    bb[c] = a*(b[c] - delta);
  } else {
    delta = (out_phys_range/2.0*-1.0)*contrast/100.0;
    a = (out_phys_range-delta*2.0)/(out_phys_range-1.0);
    bb[c] = a*b[c] + delta;
  }

  for (unsigned int c=0; c<image_histograms.size(); ++c)
    for (unsigned int p=0; p<image_luts[c].size(); ++p) {
      float v = image_luts[c][p]*a + bb[c];
      image_luts[c][p] = dim::trim<unsigned int, double>( v, 0, out_phys_range-1 );
    }
}

WVBrightnessContrastWidget::WVBrightnessContrastWidget(QWidget *parent, Qt::WindowFlags f) {

  QWidget *Form = this;

    Form->setMinimumSize(QSize(0, 110));
    gridLayout = new QGridLayout(Form);
    gridLayout->setSpacing(2);
    gridLayout->setMargin(4);
    gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
    label_4 = new QLabel(Form);
    label_4->setObjectName(QString::fromUtf8("label_4"));
    QSizePolicy sizePolicy(static_cast<QSizePolicy::Policy>(4), static_cast<QSizePolicy::Policy>(5));
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(label_4->sizePolicy().hasHeightForWidth());
    label_4->setSizePolicy(sizePolicy);

    gridLayout->addWidget(label_4, 3, 2, 1, 1);

    contrastSlider = new QSlider(Form);
    contrastSlider->setObjectName(QString::fromUtf8("contrastSlider"));
    contrastSlider->setMinimum(-100);
    contrastSlider->setMaximum(100);
    contrastSlider->setOrientation(Qt::Horizontal);
    contrastSlider->setTickPosition(QSlider::TicksAbove);
    contrastSlider->setTickInterval(99);

    gridLayout->addWidget(contrastSlider, 3, 1, 1, 1);

    label_3 = new QLabel(Form);
    label_3->setObjectName(QString::fromUtf8("label_3"));
    QSizePolicy sizePolicy1(static_cast<QSizePolicy::Policy>(4), static_cast<QSizePolicy::Policy>(5));
    sizePolicy1.setHorizontalStretch(0);
    sizePolicy1.setVerticalStretch(0);
    sizePolicy1.setHeightForWidth(label_3->sizePolicy().hasHeightForWidth());
    label_3->setSizePolicy(sizePolicy1);

    gridLayout->addWidget(label_3, 3, 0, 1, 1);

    contrastLabel = new QLabel(Form);
    contrastLabel->setObjectName(QString::fromUtf8("contrastLabel"));
    contrastLabel->setAlignment(Qt::AlignBottom|Qt::AlignHCenter);

    gridLayout->addWidget(contrastLabel, 2, 0, 1, 3);

    label = new QLabel(Form);
    label->setObjectName(QString::fromUtf8("label"));
    QSizePolicy sizePolicy2(static_cast<QSizePolicy::Policy>(4), static_cast<QSizePolicy::Policy>(5));
    sizePolicy2.setHorizontalStretch(0);
    sizePolicy2.setVerticalStretch(0);
    sizePolicy2.setHeightForWidth(label->sizePolicy().hasHeightForWidth());
    label->setSizePolicy(sizePolicy2);

    gridLayout->addWidget(label, 1, 0, 1, 1);

    brightnessSlider = new QSlider(Form);
    brightnessSlider->setObjectName(QString::fromUtf8("brightnessSlider"));
    brightnessSlider->setMinimum(-100);
    brightnessSlider->setMaximum(100);
    brightnessSlider->setPageStep(10);
    brightnessSlider->setOrientation(Qt::Horizontal);
    brightnessSlider->setInvertedAppearance(false);
    brightnessSlider->setInvertedControls(false);
    brightnessSlider->setTickPosition(QSlider::TicksAbove);
    brightnessSlider->setTickInterval(99);

    gridLayout->addWidget(brightnessSlider, 1, 1, 1, 1);

    label_2 = new QLabel(Form);
    label_2->setObjectName(QString::fromUtf8("label_2"));
    QSizePolicy sizePolicy3(static_cast<QSizePolicy::Policy>(4), static_cast<QSizePolicy::Policy>(5));
    sizePolicy3.setHorizontalStretch(0);
    sizePolicy3.setVerticalStretch(0);
    sizePolicy3.setHeightForWidth(label_2->sizePolicy().hasHeightForWidth());
    label_2->setSizePolicy(sizePolicy3);

    gridLayout->addWidget(label_2, 1, 2, 1, 1);

    brightnessLabel = new QLabel(Form);
    brightnessLabel->setObjectName(QString::fromUtf8("brightnessLabel"));
    brightnessLabel->setMinimumSize(QSize(0, 30));
    brightnessLabel->setAlignment(Qt::AlignBottom|Qt::AlignHCenter);

    gridLayout->addWidget(brightnessLabel, 0, 0, 1, 3);


    label_4->setText(QApplication::translate("Form", "+100", 0, QApplication::UnicodeUTF8));
    label_3->setText(QApplication::translate("Form", "-100", 0, QApplication::UnicodeUTF8));
    contrastLabel->setText(QApplication::translate("Form", "Contrast", 0, QApplication::UnicodeUTF8));
    label->setText(QApplication::translate("Form", "-100", 0, QApplication::UnicodeUTF8));
    label_2->setText(QApplication::translate("Form", "+100", 0, QApplication::UnicodeUTF8));
    brightnessLabel->setText(QApplication::translate("Form", "Brightness", 0, QApplication::UnicodeUTF8));

  connect(brightnessSlider, SIGNAL( valueChanged(int) ), this, SLOT( onSliderChanged(int) ));
  connect(contrastSlider, SIGNAL( valueChanged(int) ), this, SLOT( onSliderChanged(int) ));
}

void WVBrightnessContrastWidget::onSliderChanged(int) {
  emit paramsChanged();
}


//------------------------------------------------------------------------------
// WVLevelsLutModifier
//------------------------------------------------------------------------------

void WVLevelsLutModifier::lutsModify( const std::vector<DimHistogram> &image_histograms, 
                                        std::vector< std::vector<unsigned char> > &image_luts ) {
  
  conf_widget->setChannels(*channel_names);
  unsigned int out_phys_range = (unsigned int) pow( 2.0f, (int)abs((int)display_bpp) );
  conf_widget->initValues( image_histograms.size() );
  DimLut lut;

  // generate input preview LUT
  for (unsigned int c=0; c<image_histograms.size(); ++c) {
    conf_widget->hist_in[c].init( display_bpp );
    lut.init( image_histograms[c], conf_widget->hist_in[c] );
    lut.set_lut( image_luts[c] );
    lut.apply( image_histograms[c], conf_widget->hist_in[c] );
  }

  // modify LUT
  for (unsigned int c=0; c<image_histograms.size(); ++c) {
    int b = conf_widget->minValue(c);
    int e = conf_widget->maxValue(c);

    int range = e - b;
    if (range < 1) range = out_phys_range;

    for (unsigned int p=0; p<image_luts[c].size(); ++p) {
      float v = (((double)image_luts[c][p])-b)*out_phys_range/range;
      image_luts[c][p] = dim::trim<unsigned int, double>( v, 0, out_phys_range-1 );
    } // for lut size
  } // for channels

  // generate output preview LUT
  /*
  for (unsigned int c=0; c<conf_widget->hist_in.size(); ++c) {
    conf_widget->hist_out[c].init( display_bpp );
    lut.init( conf_widget->hist_in[c], conf_widget->hist_out[c] );
    lut.set_lut( image_luts[c] );
    lut.apply( conf_widget->hist_in[c], conf_widget->hist_out[c] );
  }
  */
  for (unsigned int c=0; c<image_histograms.size(); ++c) {
    conf_widget->hist_out[c].init( display_bpp );
    lut.init( image_histograms[c], conf_widget->hist_out[c] );
    lut.set_lut( image_luts[c] );
    lut.apply( image_histograms[c], conf_widget->hist_out[c] );
  }

  conf_widget->plot();
}

WVLevelsWidget::WVLevelsWidget(QWidget *parent, Qt::WindowFlags f) {

  QWidget *Form = this;

  Form->setMinimumSize(QSize(0, 180));
  gridLayout = new QGridLayout(Form);
  gridLayout->setSpacing(0);
  gridLayout->setMargin(6);
  gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
  spacerItem = new QSpacerItem(20, 4, QSizePolicy::Minimum, QSizePolicy::Fixed);

  gridLayout->addItem(spacerItem, 1, 0, 1, 1);

  histogramView = new DHistogramWidget(Form);
  histogramView->setObjectName(QString::fromUtf8("histogramView"));
  histogramView->setMinimumSize(QSize(0, 100));

  gridLayout->addWidget(histogramView, 2, 0, 1, 2);

  slider = new DRangeSlider(Form);
  slider->setObjectName(QString::fromUtf8("slider"));
  slider->setMinimumSize(QSize(0, 30));
  slider->setMaximumSize(QSize(16777215, 30));

  gridLayout->addWidget(slider, 3, 0, 1, 2);

  channelsLabel = new QLabel(Form);
  channelsLabel->setObjectName(QString::fromUtf8("channelsLabel"));

  gridLayout->addWidget(channelsLabel, 0, 0, 1, 1);

  channelsCombo = new QComboBox(Form);
  channelsCombo->setObjectName(QString::fromUtf8("channelsCombo"));

  gridLayout->addWidget(channelsCombo, 0, 1, 1, 1);


  channelsLabel->setText(QApplication::translate("Form", "Channels:", 0, QApplication::UnicodeUTF8));

  connect(slider, SIGNAL( valuesChanged() ), this, SLOT( onSliderChanged() ));
  slider->setRange( 0, 255 );
  //connect(channelsCombo, SIGNAL( currentIndexChanged(int) ), this, SLOT( onComboChanged(int) ));
  connect(channelsCombo, SIGNAL( activated (int) ), this, SLOT( onComboChanged(int) ));
}

void WVLevelsWidget::plot() {
  if (hist_in.size()==0 || hist_out.size()==0) return;
  if (channelsCombo->currentIndex() > 0)
    // individual channels selection -> show only the input and output  
    histogramView->plot( hist_in[channelsCombo->currentIndex()-1], hist_out[channelsCombo->currentIndex()-1] );
  else {
    // all channels -> show all inputs and outputs
    histogramView->plot( hist_in, hist_out );
  }
}

void WVLevelsWidget::onComboChanged(int) {
  if (mins.size()>channelsCombo->currentIndex() && maxs.size()>channelsCombo->currentIndex())
    slider->setSub( mins[channelsCombo->currentIndex()], maxs[channelsCombo->currentIndex()] );
  plot();
}

void WVLevelsWidget::onSliderChanged() {
  mins[channelsCombo->currentIndex()] = slider->subMin();
  maxs[channelsCombo->currentIndex()] = slider->subMax();
  emit paramsChanged();
}

void WVLevelsWidget::setValues(const std::vector<double> &minvals, const std::vector<double> &maxvals) {
  mins.assign( minvals.begin(), minvals.end() );
  maxs.assign( maxvals.begin(), maxvals.end() );
  slider->setSub( mins[channelsCombo->currentIndex()], maxs[channelsCombo->currentIndex()] );
}

void WVLevelsWidget::initValues( int num_channels ) {
  hist_in.resize( num_channels );
  hist_out.resize( num_channels );

  if (mins.size() < num_channels+1) {
    mins.resize(num_channels+1);
    for (int c=0; c<mins.size(); ++c) mins[c] = 0;
  }
  if (maxs.size() < num_channels+1) {
    maxs.resize(num_channels+1);
    for (int c=0; c<maxs.size(); ++c) maxs[c] = 255;
  }

}

void WVLevelsWidget::setChannels(const QStringList &lc) {
  int si = channelsCombo->currentIndex();
  channelsCombo->clear();
  channelsCombo->addItem( "All" );
  channelsCombo->addItems( lc );
  channelsCombo->removeItem( 1 );
  if (si>=0 && si<channelsCombo->count()) 
    channelsCombo->setCurrentIndex(si);
  else
    channelsCombo->setCurrentIndex(0);
}




