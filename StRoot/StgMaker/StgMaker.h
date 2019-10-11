#ifndef __StgMaker_h__
#define __StgMaker_h__

#include "StMaker.h"

#ifndef __CINT__
#include "StgUtil/XmlConfig/XmlConfig.h"
#endif

class ForwardTracker;
class ForwardHitLoader;
class StarFieldAdaptor;

class StRnDHitCollection;

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
#endif
  
};

#endif
