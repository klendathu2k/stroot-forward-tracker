
#define LOGURU_IMPLEMENTATION 1
#include "Tracker/FwdTracker.h"
#include "Tracker/FwdHit.h"
#include "Tracker/TrackFitter.h"

#include "TMath.h"

#include "StgMaker.h"
#include <map>
#include <string>

#include "StEvent.h"
#include "StRnDHitCollection.h"
#include "StRnDHit.h"
#include "StTrack.h"
#include "StTrackNode.h"
#include "StGlobalTrack.h"
#include "StPrimaryTrack.h"
#include "StTrackGeometry.h"
#include "StHelixModel.h"

#include "tables/St_g2t_track_Table.h"
#include "tables/St_g2t_fts_hit_Table.h"


#include "StarMagField.h"
#include "TRandom.h"

#include "StEventUtilities/StEventHelper.h"

#include "StTrackDetectorInfo.h"
#include "StEvent/StEnumerations.h"

#include "StarClassLibrary/StPhysicalHelix.hh"

//_______________________________________________________________________________________
// For now, accept anything we are passed, no matter what it is or how bad it is
template<typename T> bool accept( T ){ return true; }

template<> bool accept( genfit::Track* track ) {
  if (track->getNumPoints() <= 0 ) return false; // fit may have failed
  return true;
};


//_______________________________________________________________________________________
// Adaptor for STAR magnetic field
class StarFieldAdaptor : public genfit::AbsBField { 
public:
  StarFieldAdaptor() {
    _gField = this; // global pointer in TrackFitter.h
  };
  virtual TVector3 get(const TVector3& position) const {
    double x[] = { position[0], position[1], position[2] };
    double B[] = { 0, 0, 0 };
    if (  StarMagField::Instance() )    StarMagField::Instance()->Field( x, B );
    return TVector3( B );
  };  
  virtual void     get(const double& _x, const double& _y, const double& _z, double& Bx, double& By, double& Bz ) const {
    double x[] = { _x, _y, _z };
    double B[] = { 0, 0, 0 };
    if (  StarMagField::Instance() ) StarMagField::Instance()->Field( x, B );
    Bx = B[0];
    By = B[1];
    Bz = B[2];
    return;
  };
};




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
StgMaker::StgMaker() : StMaker("stg"), mForwardTracker(0), mForwardHitLoader(0), mFieldAdaptor(new StarFieldAdaptor()) {

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

  const double z_fst[]  = { 93.3, 140.0, 186.6 };
  const double z_stgc[] = { 280.9, 303.7, 326.6, 349.4 };
  const double z_wcal[] = { 711.0 };
  const double z_hcal[] = { 740.0 }; // TODO: get correct value
  
  
  StEvent* event = static_cast<StEvent*>(GetInputDS("StEvent"));
  if ( 0==event ) {
    LOG_INFO << "No event, punt on forward tracking." << endm;
    return kStWarn;
  }

  // I am a horrible person for doing this by reference, but at least
  // I don't use "goto" anywhere.
  std::map<int, shared_ptr<KiTrack::McTrack> >& mcTrackMap = mForwardHitLoader->_mctracks;
  std::map<int, std::vector<KiTrack::IHit*> >&  hitMap = mForwardHitLoader->_hits;

  // Get geant tracks
  St_g2t_track *g2t_track = (St_g2t_track *) GetDataSet("geant/g2t_track"); //  if (!g2t_track)    return kStWarn;
  if ( g2t_track ) for ( int irow=0; irow<g2t_track->GetNRows();irow++ ) {
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
  if ( g2t_stg_hits == nullptr ){
    LOG_INFO << "g2t_stg_hits is null" << endm;
    return kStErr;
  }
  int nstg = g2t_stg_hits->GetNRows();
  LOG_INFO << "nstg = " << nstg << endm;
  for ( int i=0;i<nstg;i++ ) {  

    g2t_fts_hit_st* git = (g2t_fts_hit_st*)g2t_stg_hits->At(i); if (0==git) continue; // geant hit
    int   track_id  = git->track_p;
    int   volume_id = git->volume_id;
    int   plane_id  = volume_id / 4 ;//+ 9; // four chambers/station, offset by 9 for reasons
    float x         = git->x[0] + 0.001 * gRandom->Gaus();
    float y         = git->x[1];
    float z         = git->x[2];

    LOG_INFO << "track_id=" << track_id << " volume_id=" << volume_id << " plane_id=" << plane_id << " x/y/z " << x << "/" << y << "/" << z << endm;

    KiTrack::FwdHit* hit = new KiTrack::FwdHit(count++, x, y, z, -plane_id, track_id, mcTrackMap[track_id] );

    // Add the hit to the hit map
    hitMap[ hit->getSector() ].push_back(hit);

    // Add hit pointer to the track
    if ( mcTrackMap[ track_id ] )    mcTrackMap[ track_id ]->addHit( hit );
   
  }


  St_g2t_fts_hit* g2t_fsi_hits = (St_g2t_fts_hit*) GetDataSet("geant/g2t_fsi_hit");
  if ( g2t_fsi_hits == nullptr){
    LOG_INFO << "g2t_fsi_hits is null" << endm;
    return kStErr;
  }

  int nfsi = g2t_fsi_hits->GetNRows();
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

  // Now fill StEvent
  FillEvent();

#if 0
  const auto& reco_tracks = mForwardTracker -> getRecoTracks();
  const auto& fit_momenta = mForwardTracker -> getFitMomenta();
  const auto& fit_status  = mForwardTracker -> getFitStatus();
  const auto& global_reps = mForwardTracker -> globalTrackReps();
  const auto& global_tracks = mForwardTracker -> globalTracks();

  assert ( reco_tracks.size() == fit_momenta.size() );
  assert ( reco_tracks.size() == fit_status.size() );

  int tcount = 0;
  for ( auto seed : reco_tracks ) {

    const TVector3&          fitmom  = fit_momenta[tcount];
    const genfit::FitStatus& fitstat = fit_status[tcount];
    
    LOG_INFO << "------------------------------------------------------------" << endm;
    LOG_INFO << "Reconstructed track " << tcount << endm;
    LOG_INFO << "N hits = " << seed.size() << endm;
    int hcount = 0;
    for ( auto hit : seed ) {
      KiTrack::FwdHit* fwdhit = (KiTrack::FwdHit*)hit;
      LOG_INFO << "  hit " << hcount++ 
	       << " track_id=" << fwdhit->_tid
	       << " volume_id=" << fwdhit->_vid
	       << endm;
    }
    LOG_INFO << "Momentum: " << fitmom[0] << " " << fitmom[1] << " " << fitmom[2] << " | pT=" << fitmom.Perp() << endm;
    LOG_INFO << "Status: " << endm;
    fitstat.Print();

    // Get the global track rep
    const auto* rep   = global_reps[tcount];
    const auto* track = global_tracks[tcount];

   
    tcount++;
  }
#endif

  
  // Get reco tracks, their momenta and fit status

  

  // TODO: Hang tracks on StEvent

  return kStOK;
}
//________________________________________________________________________
void StgMaker::Clear( const Option_t* opts ) {
  // mForwardHitLoader->clear();
}
//________________________________________________________________________
void StgMaker::FillEvent() {

  StEvent* event = static_cast<StEvent*>(GetInputDS("StEvent"));
  assert(event); // we warned ya

  LOG_INFO << "Filling StEvent w/ results from genfit tracker" << endm;

  // Track seeds
  const auto& seed_tracks = mForwardTracker -> getRecoTracks();
  // Reconstructed globals
  const auto& glob_tracks = mForwardTracker -> globalTracks();

  // Clear up somethings... (but does this interfere w/ Sti and/or Stv?)
  StEventHelper::Remove(event,"StSPtrVecTrackNode");
  StEventHelper::Remove(event,"StSPtrVecPrimaryVertex");

  LOG_INFO << "  number of tracks      = " << glob_tracks.size() << endm;
  LOG_INFO << "  number of track seeds = " << seed_tracks.size() << endm;

  auto& trackNodes         = event->trackNodes();
  auto& trackDetectorInfos = event->trackDetectorInfo();

  int track_count_total  = 0;
  int track_count_accept = 0;
  for ( auto* track : mForwardTracker->globalTracks() ) {

    // Get the track seed
    const auto& seed = seed_tracks[track_count_total];

    // Increment total track count
    track_count_total++;

    // Check to see if the track passes cuts (it should, for now)
    if ( 0==accept(track) ) continue; 
    track_count_accept++;

    // Create a detector info object to be filled
    StTrackDetectorInfo* detectorInfo = new StTrackDetectorInfo;
    FillDetectorInfo( detectorInfo, track, true );
    
    // Create a new track node (on which we hang a global and, maybe, primary track)
    StTrackNode* trackNode = new StTrackNode;

    // This is our global track, to be filled from the genfit::Track object "track"
    StGlobalTrack* globalTrack = new StGlobalTrack;

    // Fill the track with the good stuff
    FillTrack( globalTrack, track, seed, detectorInfo );   

    // On successful fill (and I don't see why we wouldn't be) add detector info to the list
    trackDetectorInfos.push_back( detectorInfo );    

    // // Set relationships w/ tracker object and MC truth
    // // globalTrack->setKey( key );
    // // globalTrack->setIdTruth( idtruth, qatruth ); // StTrack is dominant contributor model

    // // Add the track to its track node
    // trackNode->addTrack( globalTrack );
    // trackNodes.push_back( trackNode );

    // NOTE: could we qcall here mForwardTracker->fitTrack( seed, vertex ) ?

  } // end of loop over tracks

  LOG_INFO << "  number accepted = " <<   track_count_accept << endm;
  
}
//________________________________________________________________________
void StgMaker::FillTrack( StTrack*             otrack, genfit::Track* itrack, const Seed_t& iseed, StTrackDetectorInfo* info )
{
  const double z_fst[]  = { 93.3, 140.0, 186.6 };
  const double z_stgc[] = { 280.9, 303.7, 326.6, 349.4 };

  // otrack == output track
  // itrack == input track (genfit)

  otrack->setEncodedMethod(kUndefinedFitterId);
  
  // Track length and TOF between first and last point on the track
  // TODO: is this same definition used in StEvent?
  double track_len = itrack->getTrackLen();
  //  double track_tof = itrack->getTrackTOF();
  otrack->setLength( track_len );

  // Get the so called track seed quality... the number of hits in the seed
  int seed_qual = iseed.size();
  otrack->setSeedQuality( seed_qual );

  // Set number of possible points in each detector
  // TODO: calcuate the number of possible points in each detector, for now set = 4
  otrack->setNumberOfPossiblePoints( 4, kUnknownId );

  // Fill the inner and outer geometries of the track.  For now,
  // always propagate the track to the first layer of the silicon
  // to fill the inner geometry.
  //
  // TODO: We may need to extend our "geometry" classes for RK parameters
  FillTrackGeometry( otrack, itrack, z_fst [0], kInnerGeometry );
  FillTrackGeometry( otrack, itrack, z_stgc[3], kOuterGeometry );

  // Next fill the fit traits
  FillTrackFitTraits( otrack, itrack );
  
}
//________________________________________________________________________
void StgMaker::FillTrackFitTraits( StTrack*             otrack, genfit::Track* itrack ) {

  unsigned short g3id_pid_hypothesis = 6; // TODO: do not hard code this

  // TODO: extract chi2 and covariance from the GF track
  float chi2[] = {0,0};
  float covM[15] = { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0 };

  // ... odd that we make this determination based on the output track's type ...
  if ( primary == otrack->type() ) {
    // TODO: chi2[1] should hold the incremental chi2 of adding the vertex for the primary track
  }

  StTrackFitTraits fit_traits(g3id_pid_hypothesis,0,chi2,covM);

  // Get number of hits in all detectors
  int nhits[kMaxDetectorId] = {};
  for ( const auto* point : itrack->getPoints() ) {

    const auto* measurement = point->getRawMeasurement();
    int detId = measurement->getDetId();
    nhits[detId]++;

  }

  for ( int i=0;i<kMaxDetectorId;i++ ) {
    if ( 0 == nhits[i] ) continue; // not sure why, but Sti skips setting zero hits
    fit_traits.setNumberOfFitPoints( (unsigned char)nhits[i], (StDetectorId)i );
  }

  if ( primary == otrack->type() ) {
    fit_traits.setPrimaryVertexUsedInFit( true );
  }
  
  otrack -> setFitTraits( fit_traits );

  
 
}
//________________________________________________________________________
void StgMaker::FillTrackGeometry( StTrack*             otrack, genfit::Track* itrack, double zplane, int io ) {

  
  // Obtain fitted state 
  genfit::MeasuredStateOnPlane measuredState = itrack->getFittedState(1);

  // Obtain the cardinal representation
  genfit::AbsTrackRep* cardinal =  itrack->getCardinalRep();

  // We really don't want the overhead in the TVector3 ctor/dtor here
  static TVector3 xhat(1,0,0), yhat(0,1,0), Z(0,0,0);

  // Assign the z position
  Z[2] = zplane;

  // This is the plane for which we are evaluating the fit
  const auto detectorPlane = genfit::SharedPlanePtr( new genfit::DetPlane(Z, xhat, yhat) );

  // Update the state to the given plane
  cardinal->extrapolateToPlane( measuredState, detectorPlane, false, true );

  //  measuredState.Print();

  static StThreeVector<double> momentum;
  static StThreeVector<double> origin;  

  static TVector3 pos;
  static TVector3 mom;
  static TMatrixDSym cov;

  measuredState.getPosMomCov(pos, mom, cov);

  for ( int i=0;i<3;i++ )  momentum[i] = mom[i];
  for ( int i=0;i<3; i++ ) origin[i]   = pos[i];

  double charge = measuredState.getCharge();

  // Get magnetic field
  double X[] = { pos[0], pos[1], pos[2] };
  double B[] = { 0, 0, 0 };
  StarMagField::Instance()->Field( X, B );

  // This is really an approximation, should be good enough for the inner
  // geometry (in the Silicon) but terrible in the outer geometry ( sTGC)
  double Bz = B[2];

  // Temporary helix to get the helix parameters
  StPhysicalHelix helix( momentum, origin, Bz, charge );
  // StiStEventFiller has this as |curv|. 
  double curv = TMath::Abs(helix.curvature());
  double h    = -TMath::Sign(charge*Bz, 1.0); // helicity
  if ( charge==0 ) h = 1;
  //
  // From StHelix::helix()
  //
  // phase = mPsi - h*pi/2
  // so...
  // psi = phase + h*pi/2
  //
  double psi = helix.phase() + h * TMath::Pi() / 2;
  double dip  = helix.dipAngle();
  short  q    = charge; assert( q==1 || q==-1 || q==0 );

  // Create the track geometry
  StTrackGeometry* geometry = new StHelixModel (q, psi, curv, dip, origin, momentum, h);


  // TODO: check helix parameters... geometry->helix() should return an StPhysicalHelix... 
  //       double check that we converted everthing correctly.


  if ( kInnerGeometry == io ) otrack->setGeometry( geometry );
  else                        otrack->setOuterGeometry( geometry );
  

}
// //________________________________________________________________________
void StgMaker::FillDetectorInfo(  StTrackDetectorInfo* info, genfit::Track* track, bool increment ) {

//   // here is where we would fill in
//   // 1) total number of hits
//   // 2) number of sTGC hits
//   // 3) number of silicon hits
//   // 4) an StHit for each hit fit to the track
//   // 5) The position of the first and last hits on the track
  
  int ntotal   = track->getNumPoints(); // vs getNumPointsWithMeasurement() ?
  int nstgc    = ntotal;

  float zmin =  9E9;
  float zmax = -9E9;

  StThreeVectorF firstPoint(0,0,9E9);
  StThreeVectorF lastPoint(0,0,-9E9);

  int count = 0;
  for ( const auto* point : track->getPoints() ) {

    const auto* measurement = point->getRawMeasurement();
    const TVectorD& xyz = measurement->getRawHitCoords();
    float x = xyz[0];
    float y = xyz[1];
    float z = 0; // We get this from the detector plane...
    
    // Get fitter info for the cardinal representation
    const auto* fitinfo = point->getFitterInfo();

    const auto& plane = fitinfo->getPlane();
    TVector3 normal = plane->getNormal();
    const TVector3& origin = plane->getO();

    z = origin[2]; 
    if ( z > lastPoint[2] ) {
      lastPoint.setX(x);      lastPoint.setY(y);      lastPoint.setZ(z);
    }
    if ( z < firstPoint[2] ) {
      firstPoint.setX(x);     firstPoint.setY(y);     firstPoint.setZ(z);
    }
    
    int detId = measurement->getDetId();
    int hitId = measurement->getHitId();

    ++count;

    //TODO: Convert (or access) StHit and add to the track detector info

  }


  assert(count);

  info -> setFirstPoint( firstPoint );
  info -> setLastPoint( lastPoint );
  info -> setNumberOfPoints( ntotal, kUnknownId ); // TODO: Assign 

    
}
//________________________________________________________________________
