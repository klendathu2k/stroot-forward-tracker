#ifndef __StgMaker_h__
#define __StgMaker_h__

#include "StMaker.h"

#ifndef __CINT__
#include "StgUtil/XmlConfig/XmlConfig.h"
#endif

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


  
};

#endif
