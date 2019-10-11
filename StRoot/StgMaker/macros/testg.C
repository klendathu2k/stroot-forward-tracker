void testg() {

  gROOT->LoadMacro("bfc.C");
  bfc(0,"fzin agml makeevent sdt20181215","testg.fzd");
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

  StgMaker *gmk = new StgMaker();
  gmk->Init();

  for ( int i=0;i<10; i++ ) {
    chain->Clear();
    chain->Make(); 
  }
}
