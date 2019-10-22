void testg() {

  gROOT->LoadMacro("bfc.C");
  bfc(0,"fzin agml makeevent stu sdt20181215 cmudst","testg.fzd");
  gSystem->Load("/star/simu/simu/jwebb/2019/10-09-2019-forward-tracker-integration/GenFit/lib/libgenfit2.so"); 
  gSystem->Load("StgUtil.so");
  gSystem->Load("StgMaker.so");

  // Force build of the geometry
  TFile* geom = TFile::Open("fGeom.root");
  if ( 0==geom ) {
    AgModule::SetStacker( new StarTGeoStacker() );
    AgPosition::SetDebug(2); 
    StarGeometry::Construct("dev2021");
  
    // Believe that genfit requires the geometry is cached in a ROOT file
    gGeoManager->Export("fGeom.root");
  }
  else {
    cout << "WARNING:  Using CACHED geometry as a convienence!" << endl;
    delete geom;
  }

  // Create genfit forward track maker and add it to the chain before the MuDst maker
  StgMaker *gmk = new StgMaker();
  chain->AddAfter( "0Event", gmk );

  // And initialize it, since we have already initialized the chain
  gmk->Init();


  // Do an ls to be sure
  chain->ls(3);


  // Loop over all events in the file...
  int stat = 0; 
  while (stat==0) {
    chain->Clear();
    stat =    chain->Make(); 
    if (stat) break;

    // Get StEvent
    StEvent* event = (StEvent* )chain->GetDataSet("StEvent");
    assert(event);

    //    event->statistics();

    int nnodes = event->trackNodes().size();

    cout << "EVENT EVENT EVENT EVENT EVENT EVENT EVENT EVENT EVENT EVENT EVENT " << endl;
    cout << "nnodes = " << nnodes << endl;
    for ( int i=0;i<nnodes; i++ ) {

      const StTrackNode* node = event->trackNodes()[i];
      StGlobalTrack* track = node->track(global);
      StTrackGeometry* geometry = track->geometry();

      StThreeVectorF origin = geometry->origin();
      StThreeVectorF momentum = geometry->momentum();
      
      cout << "Track origin: " << origin << " momentum: " << momentum << " pt=" << momentum.perp() << " eta=" << momentum.pseudoRapidity() << endl;


    }


  }

}
