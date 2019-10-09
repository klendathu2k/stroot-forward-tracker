#include "StgMaker.h"
#include <map>
#include <string>

//________________________________________________________________________
StgMaker::StgMaker() : StMaker("stg") {

};
//________________________________________________________________________
int StgMaker::Init() {

  std::string configFile = "config.xml";
  std::map<string, string> cmdLineConfig;
  _xmlconfig.loadFile( configFile, cmdLineConfig );

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
