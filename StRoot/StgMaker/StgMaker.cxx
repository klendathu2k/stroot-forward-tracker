#include "StgMaker.h"
#include <map>
#include <string>

#define LOGURU_IMPLEMENTATION 1
#include "Tracker/FwdTracker.h"


//________________________________________________________________________
StgMaker::StgMaker() : StMaker("stg") {

};
//________________________________________________________________________
int StgMaker::Init() {

  // Initialize configuration file
  std::string configFile = "config.xml";
  std::map<string, string> cmdLineConfig;
  _xmlconfig.loadFile( configFile, cmdLineConfig );
  // Dump configuration to screen
  LOG_INFO << _xmlconfig.dump().c_str() << endm;

  return kStOK;
};
//________________________________________________________________________
int StgMaker::Make() {

  return kStOK;
}
//________________________________________________________________________
void StgMaker::Clear( const Option_t* opts ) {

}
//________________________________________________________________________
