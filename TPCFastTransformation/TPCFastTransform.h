// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

/// \file  TPCFastTransform.h
/// \brief Definition of TPCFastTransform class
///
/// \author  Sergey Gorbunov <sergey.gorbunov@cern.ch>


#ifndef ALICE_ALITPCOMMON_TPCFASTTRANSFORMATION_TPCFASTTRANSFORM_H
#define ALICE_ALITPCOMMON_TPCFASTTRANSFORMATION_TPCFASTTRANSFORM_H

#include "FlatObject.h"
#include "TPCDistortionIRS.h"
#include <cmath>


namespace ali_tpc_common {
namespace tpc_fast_transformation {

///
/// The TPCFastTransform class represents transformation of raw TPC coordinates to XYZ
///
/// (TPC Row number, Pad, Drift Time) ->  (X,Y,Z)
///
/// The following coordinate systems are used:
///
/// 1. raw coordinate system: TPC row number [int], readout pad number [float], drift time [float]
///
/// 2. drift volume coordinate system (x,u,v)[cm]. These are cartesian coordinates:
///    x = local x,  
///    u = along the local y axis but towards to the pad increase direction, 
///    v = along the global z axis but towards the drift length increase derection.
///
///    u and v are mirrored for A/C sides of the TPC
///
/// 3. local coordinate system: x,y,z, where global x,y are rotated such that x goes through the middle of the TPC sector
///
/// 4. global coordinate system: x,y,z in ALICE coordinate system
///
///
/// The transformation is pefformed as the following:
///
/// First, the class transforms input raw coordinates to the drift volume coordinates applying the drift velocity calibration.
/// Then it aplies TPCCorrectionIRS to the drift coordinates. 
/// At the end it transforms the drift coordinates to the output local coordinates.
/// 
/// The class is flat C structure. No virtual methods, no ROOT types are used.


class TPCFastTransform :public FlatObject
{
 public:   
  
  /// The struct contains necessary info for TPC slice 
  struct SliceInfo{
    float sinAlpha;
    float cosAlpha;
  };

  /// The struct contains necessary info for TPC padrow
  struct RowInfo{
    float x;        ///< x coordinate of the row [cm]
    int maxPad;     ///< maximal pad number = n pads - 1
    float padWidth; ///< width of pads [cm] 
  };
  
  /// _____________  Constructors / destructors __________________________
 
  /// Default constructor: creates an empty uninitialized object
  TPCFastTransform();

  /// Copy constructor: disabled to avoid ambiguity. Use cloneFromObject() instead
  TPCFastTransform(const TPCFastTransform& ) CON_DELETE;
 
  /// Assignment operator: disabled to avoid ambiguity. Use cloneFromObject() instead
  TPCFastTransform &operator=(const TPCFastTransform &)  CON_DELETE;
   
  /// Destructor
  ~TPCFastTransform() CON_DEFAULT;

  
  /// _____________  FlatObject functionality, see FlatObject class for description  ____________

  /// Memory alignment 
 

  /// Gives minimal alignment in bytes required for the class object
  static constexpr size_t getClassAlignmentBytes() {return TPCDistortionIRS::getClassAlignmentBytes(); }
  
  /// Gives minimal alignment in bytes required for the flat buffer
  static constexpr size_t getBufferAlignmentBytes() {return TPCDistortionIRS::getBufferAlignmentBytes(); }
  

  /// Construction interface
  
  void cloneFromObject( const TPCFastTransform &obj, char *newFlatBufferPtr );
  
  /// Making the data buffer external
  
  using FlatObject::releaseInternalBuffer;  
  void moveBufferTo( char *newBufferPtr );
    
  /// Moving the class with its external buffer to another location 
  
  void setActualBufferAddress( char* actualFlatBufferPtr );
  void setFutureBufferAddress( char* futureFlatBufferPtr );


  /// _______________  Construction interface  ________________________


  /// Starts the initialization procedure, reserves temporary memory
  void startConstruction( int numberOfRows );

  /// Initializes a TPC row
  void setTPCrow( int iRow, float x, int nPads, float padWidth );  

  /// Sets TPC geometry
  ///
  /// It must be called once during initialization
  void setTPCgeometry( float tpcZlengthSideA, float tpcZlengthSideC );
  
  /// Sets all drift calibration parameters and the time stamp
  ///
  /// It must be called once during construction, 
  /// but also may be called afterwards to reset these parameters.
  void setCalibration( long int timeStamp, float t0, float vDrift, float vDriftCorrY, float lDriftCorr, float tofCorr, float primVtxZ, float tpcAlignmentZ );

  /// Sets the time stamp of the current calibaration 
  void setTimeStamp( long int v)  { mTimeStamp = v; }

  /// Gives a reference for external initialization of TPC distortions
  TPCDistortionIRS& getDistortionNonConst() { return mDistortion; }

  /// Finishes initialization: puts everything to the flat buffer, releases temporary memory
  void finishConstruction();



  /// _______________ The main method: cluster transformation _______________________  
  ///
  /// Transforms raw TPC coordinates to local XYZ withing a slice
  /// taking calibration + alignment into account.  
  ///
  int Transform( int slice, int row, float pad, float time, float &x, float &y, float &z );

  int convPadTimeToUV(int slice, int row, float pad, float time, float &u, float &v );
  int convUVtoYZ(int slice, int row, float x, float u, float v, float &y, float &z );
  int getTOFcorrection(int slice, int row, float x, float y, float z, float &dz );

  int convYZtoUV(int slice, int row, float x, float y, float z, float &u, float &v );
  int convUVtoPadTime(int slice, int row, float u, float v, float &pad, float &time );

  /// _______________  Utilities  _______________________________________________
 
  /// Gives number of TPC slices
  static int getNumberOfSlices(){ return NumberOfSlices; }

  /// Gives number of TPC rows
  int getNumberOfRows() const { return mNumberOfRows; }

  /// Gives the time stamp of the current calibaration parameters
  long int getTimeStamp() const { return mTimeStamp; }
 
  /// Gives slice info
  const SliceInfo& getSliceInfo( int slice ) const { return mSliceInfos[slice]; }

  /// Gives TPC row info
  const RowInfo& getRowInfo( int row ) const { return mRowInfoPtr[row]; }
 
 
 private:   


  /// Enumeration of possible initialization states
  enum ConstructionExtraState : unsigned int { 
    GeometryIsSet = 0x4,         ///< the TPC geometry is set
    CalibrationIsSet = 0x8       ///< the drift calibration is set
  };

  /// _______________  Utilities  _______________________________________________


  void relocateBufferPointers( const char* oldBuffer, char *actualBuffer );


  /// _______________  Data members  _______________________________________________

  
  static constexpr int NumberOfSlices = 36; ///< Number of TPC slices ( slice = inner + outer sector )
  

  /// _______________  Construction control  _______________________________________________
    
  unsigned int mConstructionCounter; ///< counter for initialized parameters
  std::unique_ptr<RowInfo[]> mConstructionRowInfoBuffer; ///< Temporary container of the row infos during initialization

  /// _______________  Geometry  _______________________________________________

  SliceInfo mSliceInfos[NumberOfSlices]; ///< array of slice information [fixed size]

  int mNumberOfRows = 0; ///< Number of TPC rows. It is different for the Run2 and the Run3 setups
 
  const RowInfo  *mRowInfoPtr; ///< pointer to RowInfo array inside the mFlatBufferPtr buffer

  float mTPCzLengthA; ///< Z length of the TPC, side A
  float mTPCzLengthC; ///< Z length of the TPC, side C
 
  /// _______________  Calibration data. See Transform() method  ________________________________
 

  long int mTimeStamp; ///< time stamp of the current calibration


  /// Correction of (x,u,v) with irregular splines.
  ///
  /// After the initialization, mDistortion.getFlatBufferPtr()  
  /// is pointed to the corresponding part of this->mFlatBufferPtr 
  ///
  TPCDistortionIRS mDistortion; 
                                
 
  /// _____ Parameters for drift length calculation ____
  ///
  /// t = (float) time bin, y = global y
  ///
  /// L(t,y) = (t-mT0)*(mVdrift + mVdriftCorrY*y ) + mLdriftCorr  ____
  /// 
  float mT0; ///< T0 in [time bin]
  float mVdrift; ///< VDrift in  [cm/time bin]
  float mVdriftCorrY; ///< VDrift correction for global Y[cm] in [1/time bin]
  float mLdriftCorr; ///< drift length correction in [cm]

  /// A coefficient for Time-Of-Flight correction: drift length -= EstimatedDistanceToVtx[cm]*mTOFcorr
  ///
  /// Since this correction requires a knowledge of the spatial position, it is appied after mDistortion,
  /// not on the drift length but directly on V coordinate.
  ///
  /// mTOFcorr == mVdrift/(speed of light)
  ///
  float mTOFcorr; 

  float mPrimVtxZ;      ///< Z of the primary vertex, needed for the Time-Of-Flight correction   
  float mTPCalignmentZ; ///< Global Z shift of the TPC detector. It is applied at the end of the transformation.
};



// =======================================================================
//              Inline implementations of some methods
// =======================================================================


inline void TPCFastTransform::setTPCrow( int iRow, float x, int nPads, float padWidth )
{
  /// Initializes a TPC row  
  assert( mConstructionMask & ConstructionState::InProgress );
  assert( iRow>=0 && iRow < mNumberOfRows );
  RowInfo &row = mConstructionRowInfoBuffer[iRow];
  row.x = x;
  row.maxPad = nPads - 1;
  row.padWidth = padWidth; 
  mConstructionCounter++;
} 


inline int TPCFastTransform::convPadTimeToUV(int slice, int row, float pad, float time, float &u, float &v )
{
  if ( slice<0 || slice>=NumberOfSlices || row<0 || row>=mNumberOfRows ) return -1;
 
  bool sideC = ( slice >= NumberOfSlices / 2 );

  const RowInfo &rowInfo = getRowInfo( row );
  const SliceInfo &sliceInfo = getSliceInfo( slice );

  float x = rowInfo.x;
  u = (pad - 0.5*rowInfo.maxPad)*rowInfo.padWidth;

  float y = sideC ? -u :u; // pads are mirrorred on C-side  
  float yLab = y*sliceInfo.cosAlpha+x*sliceInfo.sinAlpha;  

  v = (time-mT0)*(mVdrift + mVdriftCorrY*yLab) + mLdriftCorr; // drift length cm  
  return 0;
}


inline int TPCFastTransform::convUVtoPadTime(int slice, int row, float u, float v, float &pad, float &time )
{
  if ( slice<0 || slice>=NumberOfSlices || row<0 || row>=mNumberOfRows ) return -1;
 
  bool sideC = ( slice >= NumberOfSlices / 2 );
  
  const RowInfo &rowInfo = getRowInfo( row );
  const SliceInfo &sliceInfo = getSliceInfo( slice );

  pad = u / rowInfo.padWidth + 0.5*rowInfo.maxPad;

  float x = rowInfo.x;
  float y = sideC ? -u :u; // pads are mirrorred on C-side  
  float yLab = y*sliceInfo.cosAlpha+x*sliceInfo.sinAlpha;
  time = mT0 + (v - mLdriftCorr) / (mVdrift + mVdriftCorrY*yLab);
  return 0;
}



inline int TPCFastTransform::convUVtoYZ(int slice, int row, float x, float u, float v, float &y, float &z )
{
  if ( slice<0 || slice>=NumberOfSlices || row<0 || row>=mNumberOfRows ) return -1; 

  bool sideC = ( slice >= NumberOfSlices / 2 );

  if( sideC ){
    y = -u; // pads are mirrorred on C-side  
    z = v - mTPCzLengthC ; // drift direction is mirrored on C-side
  } else {
    y = u;
    z = mTPCzLengthA - v;
  }

  // global TPC alignment
  z += mTPCalignmentZ;
  return 0;
}


inline int TPCFastTransform::convYZtoUV(int slice, int row, float x, float y, float z, float &u, float &v )
{
  if ( slice<0 || slice>=NumberOfSlices || row<0 || row>=mNumberOfRows ) return -1;
 
  bool sideC = ( slice >= NumberOfSlices / 2 );

  z = z - mTPCalignmentZ;   
  if( sideC ){
    u = -y;
    v = z + mTPCzLengthC;
  } else { 
    u = y;
    v = mTPCzLengthA - z;
  }
  return 0;
}


inline  int TPCFastTransform::getTOFcorrection(int slice, int row, float x, float y, float z, float &dz )
{
  // calculate time of flight correction for  z coordinate
  if ( slice<0 || slice>=NumberOfSlices || row<0 || row>=mNumberOfRows ) return -1; 
  bool sideC = ( slice >= NumberOfSlices / 2 );  
  float distZ = z - mPrimVtxZ;
  float dv = - sqrt( x*x + y*y + distZ*distZ )*mTOFcorr; 
  dz = sideC ?dv :-dv;
  return 0;
}


inline int TPCFastTransform::Transform( int slice, int row, float pad, float time, float &x, float &y, float &z )
{
  /// _______________ The main method: cluster transformation _______________________  
  ///
  /// Transforms raw TPC coordinates to local XYZ withing a slice
  /// taking calibration + alignment into account.  
  ///

  if ( slice<0 || slice>=NumberOfSlices || row<0 || row>=mNumberOfRows ) return -1;

  const RowInfo &rowInfo = getRowInfo( row );
  const SliceInfo &sliceInfo = getSliceInfo( slice );
  bool sideC = ( slice >= NumberOfSlices / 2 );

  x = rowInfo.x;
  float u=0, v=0;
  convPadTimeToUV( slice, row, pad, time, u, v );

  float dx, du, dv;
  mDistortion.getDistortion( slice, row, u, v, dx, du, dv );
  
  x += dx;
  u += du;
  v += dv;

  convUVtoYZ( slice, row, x, u, v, y, z );

  float dzTOF=0;
  getTOFcorrection( slice, row,  x,  y, z, dzTOF );
  z+=dzTOF;
  return 0;
}


}// namespace
}// namespace

#endif
