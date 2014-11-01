/*****************************************************************************
  PSIA definitions
  
  Writen by Dmitry V. Fedorov <www.dimin.net> <dima@dimin.net>
  
  History:
    04/20/2004 23:33 - First creation
    10/12/2005 15:03 - updates for 64 bit machines     
        
  Ver : 2
*****************************************************************************/


#ifndef DIM_PSIA_FORMAT_H
#define DIM_PSIA_FORMAT_H

#include <dim_img_format_interface.h>

#define DIM_PSIA_OFFSET_SOURCENAME   4
#define DIM_PSIA_OFFSET_IMAGEMODE    68
#define DIM_PSIA_OFFSET_LPFSSTRENGTH 84
#define DIM_PSIA_OFFSET_AUTOFLATTEN  92
#define DIM_PSIA_OFFSET_ACTRACK      96
#define DIM_PSIA_OFFSET_WIDTH        100
#define DIM_PSIA_OFFSET_HEIGHT       104
#define DIM_PSIA_OFFSET_ANGLE        108
#define DIM_PSIA_OFFSET_SINESCAN     116
#define DIM_PSIA_OFFSET_OVERSCAN     120
#define DIM_PSIA_OFFSET_FASTSCANDIR  128
#define DIM_PSIA_OFFSET_SLOWSCANDIR  132
#define DIM_PSIA_OFFSET_XYSWAP       136
#define DIM_PSIA_OFFSET_XSCANSIZE    140
#define DIM_PSIA_OFFSET_YSCANSIZE    148
#define DIM_PSIA_OFFSET_XOFFSET      156
#define DIM_PSIA_OFFSET_YOFFSET      164
#define DIM_PSIA_OFFSET_SCANRATE     172
#define DIM_PSIA_OFFSET_SETPOINT     180
#define DIM_PSIA_OFFSET_SETPOINTUNIT 188
#define DIM_PSIA_OFFSET_TIPBIAS      204
#define DIM_PSIA_OFFSET_SAMPLEBIAS   212
#define DIM_PSIA_OFFSET_DATAGAIN     220
#define DIM_PSIA_OFFSET_ZSCALE       228
#define DIM_PSIA_OFFSET_ZOFFSET      236
#define DIM_PSIA_OFFSET_UNIT         244
#define DIM_PSIA_OFFSET_DATAMIN      260
#define DIM_PSIA_OFFSET_DATAMAX      264
#define DIM_PSIA_OFFSET_DATAAVG      268
#define DIM_PSIA_OFFSET_NCOMPRESSION 272

// Here is how it's declared in manual
/*
#pragma pack(push, 1)
typedef struct psiaImageHeader
{
  int nImageType; // 0 = 2d mapped image, 1 = line profile image
  wchar_t szSourceNameW[32]; // Topography, ZDetector etc.
  wchar_t szImageModeW[8]; //AFM, NCM etc.
  double dfLPFStrength;//double dfLPFStrength; // LowPass filter strength.
  int bAutoFlatten;// Automatic flatten after imaging.
  int bACTrack;// AC Track
  int nWidth; // Image size: nWidth: Number of Columns
  int nHeight;//Image size: nHeight: Number of Rows.
  double dfAngle; //Angle of Fast direction about positive x-axis.
  int bSineScan; // Sine Scan
  double dfOverScan; // Overscan rate
  int bFastScanDir;//non-zero when scan up, 0 for scan down.
  int bSlowScanDir;//non-zero for forward, 0 for backward.
  int bXYSwap;//Swap fast-slow scanning dirctions.
  double dfXScanSize; //X, Y Scan size in um.
  double dfYScanSize;
  double dfXOffset;// X,Y offset in microns.
  double dfYOffset;
  double dfScanRate;// Scan speed in rows per second.
  double dfSetPoint;// Error signal set point.
  wchar_t szSetPointUnitW[8];
  double dtTipBias;// Tip Bias Voltage
  double dfSampleBias;// Sample Bias Voltage
  // Data=DataGain*( dfScale*nData+dfOffset )
  double dfDataGain;// Data Gain
  double dfZScale;// Scale , now it is always 1.
  double dfZOffset;// Z Offset in step, now it is always 0.
  
  wchar_t szUnitW[8];
  int nDataMin;
  int nDataMax;
  int nDataAvg;
  int ncompression;
  int ReservedForlmage[8*10-4];
} psiaImageHeader;
#pragma pack(pop)
*/

typedef struct psiaInfoHeader {
  DIM_CHAR    szSourceNameW[32]; // Topography, ZDetector etc.
  DIM_CHAR    szImageModeW[8];   //AFM, NCM etc.
  DIM_FLOAT64 dfLPFStrength;     //DIM_FLOAT64 dfLPFStrength; // LowPass filter strength.
  DIM_INT32   bAutoFlatten;      // Automatic flatten after imaging.
  DIM_INT32   bACTrack;          // AC Track
  DIM_INT32   nWidth;            // Image size: nWidth: Number of Columns
  DIM_INT32   nHeight;           //Image size: nHeight: Number of Rows.
  DIM_FLOAT64 dfAngle;           //Angle of Fast direction about positive x-axis.
  DIM_INT32   bSineScan;         // Sine Scan
  DIM_FLOAT64 dfOverScan;        // Overscan rate
  DIM_INT32   bFastScanDir;      //non-zero when scan up, 0 for scan down.
  DIM_INT32   bSlowScanDir;      //non-zero for forward, 0 for backward.
  DIM_INT32   bXYSwap;           //Swap fast-slow scanning dirctions.
  DIM_FLOAT64 dfXScanSize;       //X, Y Scan size in um.
  DIM_FLOAT64 dfYScanSize;
  DIM_FLOAT64 dfXOffset;         // X,Y offset in microns.
  DIM_FLOAT64 dfYOffset;
  DIM_FLOAT64 dfScanRate;        // Scan speed in rows per second.
  DIM_FLOAT64 dfSetPoint;        // Error signal set point.
  DIM_CHAR    szSetPointUnitW[8];
  DIM_FLOAT64 dtTipBias;         // Tip Bias Voltage
  DIM_FLOAT64 dfSampleBias;      // Sample Bias Voltage
  DIM_FLOAT64 dfDataGain;        // Data Gain
  DIM_FLOAT64 dfZScale;          // Scale , now it is always 1.
  DIM_FLOAT64 dfZOffset;         // Z Offset in step, now it is always 0.
  DIM_CHAR    szUnitW[8];
  DIM_INT32   nDataMin;
  DIM_INT32   nDataMax;
  DIM_INT32   nDataAvg;
  DIM_INT32   ncompression;
} psiaInfoHeader;

#endif // DIM_PSIA_FORMAT_H
