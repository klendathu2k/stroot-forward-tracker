#include "StgMaker.h"
#include <map>
#include <string>

//________________________________________________________________________-
StgMaker::StgMaker() : StMaker("stg") {

};
//________________________________________________________________________-
int StgMaker::Init() {

  std::string configFile = "config.xml";
  std::map<string, string> cmdLineConfig;
  _xmlconfig.loadFile( configFile, cmdLineConfig );

  LOG_INFO << _xmlconfig.dump().c_str() << endm;

  return kStOK;
};
//________________________________________________________________________-
