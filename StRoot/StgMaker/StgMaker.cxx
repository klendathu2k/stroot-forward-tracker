#include "StgMaker.h"
#include <map>
#include <string>

#include "StEvent.h"
#include "StRnDHitCollection.h"
#include "StRnDHit.h"

#include "tables/St_g2t_track_Table.h"
#include "tables/St_g2t_fts_hit_Table.h"

#define LOGURU_IMPLEMENTATION 1
#include "Tracker/FwdTracker.h"
#include "Tracker/FwdHit.h"

//  Wrapper class around the forward tracker
class ForwardTracker : public KiTrack::ForwardTrackMaker { 
public:
  // Replaces original initialization.  Config file and hitloader
  // will be provided by the maker.
  void initialize(){  
    LOG_INFO << "ForwardTracker::initialize()" << endm;
    nEvents = 1; // only process single event

    // Create the forward system...
    KiTrack::gFwdSystem = new KiTrack::FwdSystem( 7 );    

    // make our quality plotter
    qPlotter = new QualityPlotter( cfg );
    LOG_INFO << "Booking histograms for nIterations=" << cfg.get<size_t>( "TrackFinder:nIterations", 1 ) << endm;
    qPlotter->makeHistograms( cfg.get<size_t>( "TrackFinder:nIterations", 1 ) );

    // initialize the track fitter
    trackFitter = new TrackFitter( cfg );
    trackFitter->setup( cfg.get<bool>("TrackFitter:display") );    
  }

};

// Wrapper around the hit load.
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
  // int argc=1;
  // char* argv[]={"StgMaker",""}; // deprecated, g++ warns...
  // loguru::init(argc, argv );
  loguru::add_file("everything.log", loguru::Truncate, loguru::Verbosity_2);
  loguru::g_stderr_verbosity = 1;  
  
  // Create instance of the criteria histograms  (is this optional?)
  //  CritHisto::Instance();

  mForwardTracker = new ForwardTracker();
  mForwardTracker->setConfig( _xmlconfig );

  mForwardHitLoader = new ForwardHitLoader();
  mForwardTracker->setLoader( mForwardHitLoader );


  mForwardTracker->initialize();

  return kStOK;
};
//________________________________________________________________________
int StgMaker::Make() {

  StEvent* event = static_cast<StEvent*>(GetInputDS("StEvent"));
  if ( 0==event ) {
    LOG_INFO << "No event, punt on forward tracking." << endm;
    return kStWarn;
  }

  // I am a horrible person for doing this by reference, but at least
  // I don't use "goto" anywhere.
  std::map<int, shared_ptr<KiTrack::McTrack> >& mcTrackMap = mForwardHitLoader->_mctracks;
  std::map<int, std::vector<KiTrack::IHit*> >&  hitMap     = mForwardHitLoader->_hits;

  // Get geant tracks
  St_g2t_track *g2t_track = (St_g2t_track *) GetDataSet("geant/g2t_track"); //  if (!g2t_track)    return kStWarn;
  for ( int irow=0; irow<g2t_track->GetNRows();irow++ ) {
    g2t_track_st* track = (g2t_track_st *)g2t_track->At(irow);
    if ( 0==track ) continue;
    int track_id = track->id;
    float pt2 = track->p[0]*track->p[0] + track->p[1]*track->p[1];
    float pt = sqrt(pt2);
    float eta = track->eta;
    float phi = atan2(track->p[1], track->p[0]); //track->phi;
    int   q   = track->charge;
    if ( 0 == mcTrackMap[ track_id ] ) // should always happen
      mcTrackMap[ track_id ] = shared_ptr< KiTrack::McTrack >( new KiTrack::McTrack(pt, eta, phi, q) );
    
  }

  // Add hits onto the hit loader (from rndHitCollection)
  int count = 0;

#if 0
  for ( auto h : event->rndHitCollection()->hits() ) { // TODO: exend RnD hit collection w/ begin/end

    int volume_id = h->layer(); // MC volume ID [not positive about this mapping]
    int track_id = h->idTruth();  // MC truth

    const StThreeVectorF& xyz = h->position();
    const float& x = xyz[0];       // Position of the hit
    const float& y = xyz[1];       // Position of the hit
    const float& z = xyz[2];       // Position of the hit

    // Create the hit
    KiTrack::FwdHit* hit = new KiTrack::FwdHit(count++, x, y, z, volume_id, track_id, mcTrackMap[track_id] );

    // Add the hit to the hit map
    hitMap[ hit->getSector() ].push_back(hit);

    // Add hit pointer to the track
    if ( mcTrackMap[ track_id ] )    mcTrackMap[ track_id ]->addHit( hit );
    
  }
#else
  //
  // Use geant hits directly
  //
  St_g2t_fts_hit* g2t_stg_hits = (St_g2t_fts_hit*) GetDataSet("geant/g2t_stg_hit");
  St_g2t_fts_hit* g2t_fsi_hits = (St_g2t_fts_hit*) GetDataSet("geant/g2t_fsi_hit");
  int nstg = g2t_stg_hits->GetNRows();
  int nfsi = g2t_fsi_hits->GetNRows();
  LOG_INFO << "nstg = " << nstg << endm;
  for ( int i=0;i<nstg;i++ ) {  

    g2t_fts_hit_st* git = (g2t_fts_hit_st*)g2t_stg_hits->At(i); if (0==git) continue; // geant hit
    int   track_id  = git->track_p;
    int   volume_id = git->volume_id;
    int   plane_id  = volume_id / 4 ;//+ 9; // four chambers/station, offset by 9 for reasons
    float x         = git->x[0];
    float y         = git->x[1];
    float z         = git->x[2];

    LOG_INFO << "track_id=" << track_id << " volume_id=" << volume_id << " plane_id=" << plane_id << " x/y/z " << x << "/" << y << "/" << z << endm;

    KiTrack::FwdHit* hit = new KiTrack::FwdHit(count++, x, y, z, -plane_id, track_id, mcTrackMap[track_id] );

    // Add the hit to the hit map
    hitMap[ hit->getSector() ].push_back(hit);

    // Add hit pointer to the track
    if ( mcTrackMap[ track_id ] )    mcTrackMap[ track_id ]->addHit( hit );
   
  }
  for ( int i=0;i< -nfsi;i++ ) {   // yes, negative... because are skipping Si in initial tests

    g2t_fts_hit_st* git = (g2t_fts_hit_st*)g2t_fsi_hits->At(i); if (0==git) continue; // geant hit
    int   track_id  = git->track_p;
    int   volume_id = git->volume_id;
    int   plane_id  = volume_id;
    float x         = git->x[0];
    float y         = git->x[1];
    float z         = git->x[2];

    LOG_INFO << "track_id=" << track_id << " volume_id=" << volume_id << " x/y/z " << x << "/" << y << "/" << z << endm;

    KiTrack::FwdHit* hit = new KiTrack::FwdHit(count++, x, y, z, volume_id, track_id, mcTrackMap[track_id] );

    // Add the hit to the hit map
    hitMap[ hit->getSector() ].push_back(hit);

    // Add hit pointer to the track
    if ( mcTrackMap[ track_id ] )    mcTrackMap[ track_id ]->addHit( hit );
  
  }

#endif  

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
