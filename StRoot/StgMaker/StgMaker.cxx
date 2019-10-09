#include "StgMaker.h"
#include <map>
#include <string>

#include "StEvent.h"

#define LOGURU_IMPLEMENTATION 1
#include "Tracker/FwdTracker.h"

//  Wrapper class around the forward tracker
class ForwardTracker : public KiTrack::ForwardTrackMaker { 
  
  // Replaces original initialization.  Config file and hitloader
  // will be provided by the maker.
  void initialize(){  
    LOG_INFO << "ForwardTracker::initialize()" << endm;
    nEvents = 1; // only process single event


    // Create the forward system...
    KiTrack::gFwdSystem = new KiTrack::FwdSystem( 7 );    

    // make our quality plotter
    qPlotter = new QualityPlotter( cfg );
    qPlotter->makeHistograms( cfg.get<size_t>( "TrackFinder:nIterations", 1 ) );

    // initialize the track fitter
    trackFitter = new TrackFitter( cfg );
    trackFitter->setup( cfg.get<bool>("TrackFitter:display") );    
  }

};

// Wrapper around the hit load
class ForwardHitLoader : public IHitLoader {
public:
  unsigned long long nEvents(){ return 1; }
  std::map<int, std::vector<KiTrack::IHit*> > &load( unsigned long long ){
    return _hits;
  };
  std::map<int, shared_ptr<KiTrack::McTrack>> &getMcTrackMap() {
    return _mctracks;
  };

  // Cleanup
  void clear() {
    _hits.clear();
    _mctracks.clear();
  }
  
  // TODO, protect and add interaface for pushing hits / tracks
  std::map<int, std::vector<KiTrack::IHit*> >  _hits;
  std::map<int, shared_ptr<KiTrack::McTrack> > _mctracks;
};

//________________________________________________________________________
StgMaker::StgMaker() : StMaker("stg"), mForwardTracker(0), mForwardHitLoader(0) {

};
//________________________________________________________________________
int StgMaker::Init() {

  // Initialize configuration file
  std::string configFile = "config.xml";
  std::map<string, string> cmdLineConfig;
  _xmlconfig.loadFile( configFile, cmdLineConfig );
  // Dump configuration to screen
  LOG_INFO << _xmlconfig.dump().c_str() << endm;

  // Initialize debugging
  int argc=0;
  char* argv[]={0};
  loguru::init(argc, argv);
  loguru::add_file("everything.log", loguru::Truncate, loguru::Verbosity_2);
  loguru::g_stderr_verbosity = 1;  
  
  // Create instance of the criteria histograms  (is this optional?)
  //  CritHisto::Instance();

  mForwardTracker = new ForwardTracker();
  mForwardTracker->setConfig( _xmlconfig );

  mForwardHitLoader = new ForwardHitLoader();
  mForwardTracker->setLoader( mForwardHitLoader );

  return kStOK;
};
//________________________________________________________________________
int StgMaker::Make() {

  StEvent* event = static_cast<StEvent*>(GetInputDS("StEvent"));
  if ( 0==event ) {
    LOG_INFO << "No event, punt on forward tracking." << endm;
    return kStWarn;
  }

  // TODO: Add hits to the hit loader...

  // Process single event
  mForwardTracker -> doEvent();

  // TODO: Hang tracks on StEvent

  return kStOK;
}
//________________________________________________________________________
void StgMaker::Clear( const Option_t* opts ) {
  mForwardHitLoader->clear();
}
//________________________________________________________________________
