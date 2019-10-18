#ifndef __StgMaker_h__
#define __StgMaker_h__

#include "StMaker.h"

#ifndef __CINT__
#include "StgUtil/XmlConfig/XmlConfig.h"
#include "Track.h"
#endif

class ForwardTracker;
class ForwardHitLoader;
class StarFieldAdaptor;

class StRnDHitCollection;
class StTrack;
class StTrackDetectorInfo;

class StgMaker : public StMaker {

  ClassDef(StgMaker,0);

#ifndef __CINT__
  XmlConfig _xmlconfig;
#endif

public:

  StgMaker();
  ~StgMaker(){ /* nada */ };

  int  Init();
  int  Make();
  void Clear( const Option_t* opts="" );

private:
protected:

#ifndef __CINT__
  ForwardTracker*        mForwardTracker;
  ForwardHitLoader*      mForwardHitLoader;
  StarFieldAdaptor*      mFieldAdaptor;

  // Fill StEvent
  void FillEvent();
  void FillDetectorInfo( StTrackDetectorInfo* info, genfit::Track* track, bool increment );
  // void FillTrack       ( StTrack*             otrack, genfit::Track* itrack, StTrackDetectorInfo* info );

#endif
  
};

#endif
