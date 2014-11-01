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
      
  ver: 1
        
*******************************************************************************/

#ifndef WV_IMAGE_MANAGER_H
#define WV_IMAGE_MANAGER_H

#include <QtCore>
#include <QtGui>

#include <QPixmap>
#include <QString>
#include <QImage>
#include <QDateTime>
#include <QStringList>
#include <QVector>
#include <QThread>
#include <QHash>
#include <QVariant>

#include <vector>

#include <BioImage>
#include <BioImageFormats>

#include "d_histogram_widget.h"
#include "d_range_slider.h"

//class TMetaFormatManager;

//------------------------------------------------------------------------------
// WVLiveEnhancement - provides interface to plug methods into the UI
// has a name and configuration widget, should not not be used directly
// specific processing classes should be used: WVLutGenerator, WVLutModifier
//------------------------------------------------------------------------------

class WVLiveEnhancement: public QObject {
  Q_OBJECT

public:
  WVLiveEnhancement();

  inline const QString &name( ) const { return enhancement_name; }
  inline bool isEnabled() { return enabled; }

  virtual QWidget *widget( ) { return NULL; }

signals:
  void changed();

public slots:
  void onParamsChanged();
  void setEnabled( bool v );
  void setChannelNames(const QStringList *ls) { channel_names = (QStringList *) ls; }

protected:
  unsigned int display_bpp;
  bool enabled;
  QString enhancement_name;
  static QStringList *channel_names;
};

//------------------------------------------------------------------------------
// WVLutGenerator - specification of WVLiveEnhancement to provide LUT generation
// ex: 16bit -> display using data range
// ex: 8bit -> display using equalization
//------------------------------------------------------------------------------
class WVLutGenerator: public WVLiveEnhancement {
  Q_OBJECT
public:
  WVLutGenerator(): WVLiveEnhancement() {}

  virtual void lutsGenerate( const std::vector<DimHistogram> &image_histograms, 
                             std::vector< std::vector<unsigned char> > &image_luts );
};

//------------------------------------------------------------------------------
// WVLutModifier - specification of WVLiveEnhancement to provide LUT modification
// ex: apply brightness and contrast to display
//------------------------------------------------------------------------------
class WVLutModifier: public WVLiveEnhancement {
  Q_OBJECT
public:
  WVLutModifier(): WVLiveEnhancement() {}

  virtual void lutsModify( const std::vector<DimHistogram> &image_histograms, 
                           std::vector< std::vector<unsigned char> > &image_luts ) = 0;
};

//------------------------------------------------------------------------------
// WVLiveEnhancement
//------------------------------------------------------------------------------

class WVLinearFullRangeLutGenerator: public WVLutGenerator {
  Q_OBJECT
public:
  WVLinearFullRangeLutGenerator(): WVLutGenerator() { enhancement_name = "None"; }
  virtual void lutsGenerate( const std::vector<DimHistogram> &image_histograms, 
                             std::vector< std::vector<unsigned char> > &image_luts );
};

class WVLinearDataRangeLutGenerator: public WVLutGenerator {
  Q_OBJECT
public:
  WVLinearDataRangeLutGenerator(): WVLutGenerator() { enhancement_name = "Stretch by Min/Max"; }
  virtual void lutsGenerate( const std::vector<DimHistogram> &image_histograms, 
                             std::vector< std::vector<unsigned char> > &image_luts );
};

class WVLinearDataToleranceLutGenerator: public WVLutGenerator {
  Q_OBJECT
public:
  WVLinearDataToleranceLutGenerator(): WVLutGenerator() { enhancement_name = "Stretch with tolerance"; }
  virtual void lutsGenerate( const std::vector<DimHistogram> &image_histograms, 
                             std::vector< std::vector<unsigned char> > &image_luts );
};

class WVLinearEqualizedLutGenerator: public WVLutGenerator {
  Q_OBJECT
public:
  WVLinearEqualizedLutGenerator(): WVLutGenerator() { enhancement_name = "Equalize"; }
  virtual void lutsGenerate( const std::vector<DimHistogram> &image_histograms, 
                             std::vector< std::vector<unsigned char> > &image_luts );
};

//------------------------------------------------------------------------------
// WVNegativeLutModifier
//------------------------------------------------------------------------------

class WVNegativeLutModifier: public WVLutModifier {
  Q_OBJECT
public:
  WVNegativeLutModifier(): WVLutModifier() { enhancement_name = "Negative"; }
  virtual void lutsModify( const std::vector<DimHistogram> &image_histograms, 
                           std::vector< std::vector<unsigned char> > &image_luts );
};

//------------------------------------------------------------------------------
// WVBrightnessContrastEnhancement
//------------------------------------------------------------------------------

class WVBrightnessContrastWidget : public QWidget {
  Q_OBJECT

public:
  WVBrightnessContrastWidget(QWidget *parent = 0, Qt::WindowFlags f = 0);

public:
  QGridLayout *gridLayout;
  QLabel *label_4;
  QSlider *contrastSlider;
  QLabel *label_3;
  QLabel *contrastLabel;
  QLabel *label;
  QSlider *brightnessSlider;
  QLabel *label_2;
  QLabel *brightnessLabel;

public:
  void setValues(int bright, int contr) { brightnessSlider->setValue(bright); contrastSlider->setValue(contr); }
  int brightness() { return brightnessSlider->value(); }
  int contrast() { return contrastSlider->value(); }

signals:
  void paramsChanged();

public slots:
  void onSliderChanged(int); 

};

class WVBrightnessContrastLutModifier: public WVLutModifier {
  Q_OBJECT
public:
  WVBrightnessContrastLutModifier(): WVLutModifier() { enhancement_name = "Brightness/Contrast"; conf_widget = NULL; }
  virtual void lutsModify( const std::vector<DimHistogram> &image_histograms, 
                           std::vector< std::vector<unsigned char> > &image_luts );
  virtual QWidget *widget( ) { 
    if (!conf_widget) conf_widget = new WVBrightnessContrastWidget();
    connect(conf_widget, SIGNAL( paramsChanged() ), this, SLOT( onParamsChanged() ));
    return conf_widget;
  }
protected:
  WVBrightnessContrastWidget *conf_widget;
};

//------------------------------------------------------------------------------
// WVLevelsLutModifier
//------------------------------------------------------------------------------

class WVLevelsWidget : public QWidget {
  Q_OBJECT

public:
  WVLevelsWidget(QWidget *parent = 0, Qt::WindowFlags f = 0);

public:
  QGridLayout *gridLayout;
  QSpacerItem *spacerItem;
  DHistogramWidget *histogramView;
  DRangeSlider *slider;
  QLabel *channelsLabel;
  QComboBox *channelsCombo;

public:
  std::vector<DimHistogram> hist_in;
  std::vector<DimHistogram> hist_out;
  void plot();

  void setChannels(const QStringList &);

  void  setValues(const std::vector<double> &minvals, const std::vector<double> &maxvals);
  const std::vector<double> &minValues() { return mins; }
  const std::vector<double> &maxValues() { return maxs; }
  void  initValues( int num_channels );
  int   currentChannel() { return channelsCombo->currentIndex(); }
  double minValue( int channel ) { if (channelsCombo->currentIndex()==0 || channel>=mins.size()) return mins[0]; else return mins[channel+1]; }
  double maxValue( int channel ) { if (channelsCombo->currentIndex()==0 || channel>=maxs.size()) return maxs[0]; else return maxs[channel+1]; }

signals:
  void paramsChanged();

public slots:
  void onSliderChanged(); 
  void onComboChanged(int); 

protected:
  std::vector<double> mins;
  std::vector<double> maxs;
};

class WVLevelsLutModifier: public WVLutModifier {
  Q_OBJECT
public:
  WVLevelsLutModifier(): WVLutModifier() { enhancement_name = "Levels (Histogram adjustment)"; conf_widget = NULL; }
  virtual void lutsModify( const std::vector<DimHistogram> &image_histograms, 
                           std::vector< std::vector<unsigned char> > &image_luts );
  virtual QWidget *widget( ) { 
    if (!conf_widget) conf_widget = new WVLevelsWidget();
    connect(conf_widget, SIGNAL( paramsChanged() ), this, SLOT( onParamsChanged() ));
    return conf_widget;
  }
protected:
  WVLevelsWidget *conf_widget;
};


//------------------------------------------------------------------------------
// WVMemoryCacheItem
//------------------------------------------------------------------------------

class WVMemoryCacheItem {
public:
  WVMemoryCacheItem(): page(0), time(0) {}
  ~WVMemoryCacheItem() {}
  
  QString fileName;
  unsigned int page;
  unsigned int time;
protected:

};


//------------------------------------------------------------------------------
// WVRenderParams
//------------------------------------------------------------------------------

#define CL_COLORS 7

class WVRenderParams {
public:
  WVRenderParams();
  ~WVRenderParams();
  
  // image parameters
  int *channel_lut;
  std::vector<DimHistogram> *image_hist;

  // support of float, signed and large integers
  double scale[CL_COLORS];
  double shift[CL_COLORS];
  void compute_shift_scale();

  // other rendering
  bool fast_copy;
  unsigned int *lut_map; 
  TDimImage *in_img;
  std::vector< std::vector<unsigned char> > *intencity_luts;

  bool need_fusion;

  void init();

protected:

};

//------------------------------------------------------------------------------
// WVImageManager
//------------------------------------------------------------------------------

class DRenderThread;
class DRenderThreadInterp;

class WVImageManager: public QObject {
  Q_OBJECT
public:
  // available channels in the display
  static const int NumDisplayChannels = CL_COLORS;
  enum DisplayChannels { dcRed=0, dcGreen=1, dcBlue=2, dcAlpha=3, dcYellow=4, dcPurple=5, dcCyan=6 };

  // zoom interpolation method
  enum DisplayZoomMethod { 
    dzmNN=0, 
    dzmBL=1
  };

  // this choise slightly affects the performace, the Visible Area processing requires
  // repainting all visible area every time, which makes it slightly slower,
  // it also might make it look different from one instance to another
  enum DisplayEnhancemetArea { 
    deaFullImage=0, 
    deaOnlyVisible=1
  };

  // if channels are connected their enhancement is exactly the same
  // that preserves the inter relationshp between channels,
  // for bio imagery it's more common to use independent channels
  enum DisplayEnhancemetChannels { 
    decIndependent=0, 
    dleConnected=1
  };

  WVImageManager();
  ~WVImageManager();

  bool     loadImage( const QString &fileName, int page=-1 );
  void     clearImage();
  void     clear() { clearImage(); }
  QPixmap  toQPixmap( ) { return pyramid.imageAt(0)->toQPixmap(); }

 
  bool     metaTextEmpty() const;
  bool     pixelSizeEmpty() const;
  bool     imageEmpty() const;
  bool     needFullViewportPaint() const { return(enhance_area == deaOnlyVisible); }
    
  static QStringList formatFilters();
  static QString     dialogFilters();
  QStringList channelNames() { return channel_names; }

  int channelMapping( DisplayChannels dc );
  void setMapping( DisplayChannels dc, int ic );

  // in order to provide good visualization, number of enhancements can be done
  // "on the fly" without processing the whole image and altering original image data.
  // All these enhancements are done using LUTs (mappings), that
  // are precomputed and do not decrese drawing performace, it's also necessary
  // to do this with non 8 bit data, such as 12 or 16 bit p/ channel
  typedef unsigned int DisplayLiveEnhancemet;
  static const unsigned int wv_lut_generator_default = 0;
  static const unsigned int wv_lut_generator_data = 1;
  QStringList enhancementTypes();
  DisplayLiveEnhancemet currentEnhancementIndex() { return enhance_type; }
  WVLiveEnhancement* currentEnhancement() { return lut_generators[(int)enhance_type]; }
  void setCurrentEnhancement(DisplayLiveEnhancemet et);

  const QList<WVLutModifier*> &lutModifiers() { return lut_modifiers; }

  DisplayZoomMethod currentZoomMethod() { return zoom_method; }
  void setZoomMethod(DisplayZoomMethod zm) { zoom_method = zm; }

  DisplayEnhancemetArea currentEnhancemetArea() { return enhance_area; }
  void setEnhancemetArea(DisplayEnhancemetArea ea) { enhance_area = ea; computeImageStats(); recomputeLuts(); }

  QImage* getDisplayRoi( QRect r );

  // this is a specific function to generate 16 bit X11 compatible pixmap buffer 
  // for requested region, it uses the same memory used by the screen buffer image
  // returned by getDisplayRoi, so this buffer must not be deleted after use!!!
  //void* getDisplayRoi16bitX11( QRect r );


  QPixmap getThumbnail( const QSize &s );
  QString getMetaText() const { return meta_text; }
  QHash<QString, QVariant> getMetaData() const { return tags; }
  QString getShortImageInfo() const;
  QVector<double> pixelSize() const;
  double viewPixelSizeX( double scale_to_use = 0.0 ) const { 
    if (scale_to_use == 0.0)
      return pixel_size[0]/currentViewScale(); 
    else
      return pixel_size[0]/scale_to_use; 
  }
  double viewPixelSizeY( double scale_to_use = 0.0 ) const { 
    if (scale_to_use == 0.0)
      return pixel_size[1]/currentViewScale(); 
    else
      return pixel_size[1]/scale_to_use; 
  }
  void setPixelSize( const QVector<double> &v ) { pixel_size = v; }

  void rotateImage( double deg );
  void projectMax();
  void projectMin();

  // return true if the level exists
  bool zoomOut();
  bool zoomIn();
  bool zoom11();
  bool zoomSet( int _zoom, bool force=false );
  int  zoomClosestSmaller( unsigned int w, unsigned int h) const;
  int  zoomClosestSmaller( const QSize &size ) const;
  int  zoomCurrent() const { return zoom; }
  double currentViewScale() const;

  unsigned int imageNumPages() const { return pages_in_image; }
  unsigned int imageNumZ() const { return pyramid.imageAt(0)->numZ(); }  
  unsigned int imageNumT() const { return pyramid.imageAt(0)->numT(); }  
  unsigned int imageWidth() const { return pyramid.imageAt(0)->width(); }  
  unsigned int imageHeight() const { return pyramid.imageAt(0)->height(); }
  QSize        imageSize() const { return QSize(imageWidth(),imageHeight()); }
  unsigned int viewWidth() const;
  unsigned int viewHeight() const;
  QSize        viewSize() const { return QSize(viewWidth(),viewHeight()); }
  QPointF viewToImage( const QPointF &p ) const;
  QRectF  viewToImage( const QRectF  &r ) const;
  QPointF imageToView( const QPointF &p ) const;
  QRectF  imageToView( const QRectF  &r ) const;
  
  QVariant getTagValue( const QString &tag ) const;

signals:
  void repaintRequired();

public slots:
  void onLutsChanged();

// callbacks
public:
  TDimProgressProc progress_proc;
  TDimErrorProc error_proc;
  TDimTestAbortProc test_abort_proc;

  void setCallbacks( TDimProgressProc _progress_proc, 
                     TDimErrorProc _error_proc, 
                     TDimTestAbortProc _test_abort_proc ) {
    progress_proc = _progress_proc;
    error_proc = _error_proc;
    test_abort_proc = _test_abort_proc;
    pyramid.progress_proc = progress_proc;
  }

  void do_progress( long done, long total, char *descr ) {
    if (progress_proc != NULL) progress_proc( done, total, descr );
  }

private:
  DImagePyramid pyramid;
  int zoom;
  QImage screen_buffer;
  WVRenderParams render_params;

  QList<DRenderThread *> render_thread_pool;
  QList<DRenderThreadInterp *> render_interp_thread_pool;

  QString format_manager_filename;
  TMetaFormatManager format_manager;
  bool startImageReadSession( const QString &fileName );

  // channel to RGBA mapping, used to indicate which image channel maps to which display color
  // -1 means nothing is mapped to this channel and it should be filled with 0
  // after A channel follow fusion channels, that should go on two channels at the same time
  // 0:R, 1:G, 2:B, 3:A, 4:R+G=Yellow, 5:R+B=Purple, 6:B+G=Cyan 
  int channel_lut[NumDisplayChannels];
  std::vector<DimHistogram> image_hist;
  std::vector< std::vector<unsigned char> > intencity_luts;
  
  WVLinearFullRangeLutGenerator     lut_gen_full_range;
  WVLinearDataRangeLutGenerator     lut_gen_data_range;
  WVLinearDataToleranceLutGenerator lut_gen_data_toler;
  WVLinearEqualizedLutGenerator     lut_gen_equalize;
  DisplayLiveEnhancemet enhance_type;
  QList<WVLutGenerator*> lut_generators;

  WVBrightnessContrastLutModifier   lut_mod_bright_contrast;
  WVNegativeLutModifier             lut_mod_negative;
  WVLevelsLutModifier               lut_mod_levels;
  QList<WVLutModifier*> lut_modifiers;


  DisplayZoomMethod     zoom_method;
  DisplayEnhancemetArea enhance_area;

  QStringList channel_names;

  unsigned int pages_in_image;
  QString   meta_text;
  QDateTime date_time;
  QVector<double> pixel_size;

  QHash<QString, QVariant> tags;
  void generateHash( const QString &, TMetaFormatManager *, TDimImage * );

  void recomputeLuts();
  void getChannelNames( TMetaFormatManager *fm=NULL );
  void computeImageStats( const QRect &roi = QRect(0,0,0,0), int zoom_level = 0 );

  bool allowFastDraw();
  int num_cpus;
  QImage* getDisplayRoiInterpolated( QRect r );
  void generateMetaDataText();
};


//------------------------------------------------------------------------------
// DRenderThread
//------------------------------------------------------------------------------

class DRenderThread : public QThread {
  Q_OBJECT

public:
    void render( unsigned int _y1, unsigned int _y2, QImage *_out_img, 
                 DIM_UCHAR *srcp[WVImageManager::NumDisplayChannels], unsigned int _src_bytes_per_line[WVImageManager::NumDisplayChannels], unsigned int &_w, 
                 WVRenderParams *_render_params );

protected:
    void run();

private:
  DIM_UCHAR **src;
  unsigned int y1; 
  unsigned int y2; 
  QImage *out_img;
  unsigned int *src_bytes_per_line;
  unsigned int w;
  WVRenderParams *render_params;
};

//------------------------------------------------------------------------------
// DRenderThread
//------------------------------------------------------------------------------

class DRenderThreadInterp : public QThread {
  Q_OBJECT

public:
    void render( unsigned int _y1, unsigned int _y2, QImage *_out_img,
                 DIM_UCHAR *srcp[WVImageManager::NumDisplayChannels], unsigned int _src_bytes_per_line[WVImageManager::NumDisplayChannels],
                 unsigned int &_w, unsigned int &_t, unsigned int &_l, float &_scale, 
                 WVImageManager::DisplayZoomMethod zm, WVRenderParams *_render_params );

protected:
    void run();

private:
  DIM_UCHAR **src;
  unsigned int y1; 
  unsigned int y2; 
  QImage *out_img;
  unsigned int *src_bytes_per_line;
  unsigned int w;
  unsigned int t;
  unsigned int l;
  float scale;
  WVImageManager::DisplayZoomMethod zoom_method;
  WVRenderParams *render_params;
};

#endif // WV_IMAGE_MANAGER_H

