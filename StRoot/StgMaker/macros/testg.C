void testg() {

  gROOT->LoadMacro("bfc.C");
  bfc(0,"fzin agml makeevent sdt20181215","testg.fzd");
  gSystem->Load("/star/simu/simu/jwebb/2019/10-09-2019-forward-tracker-integration/GenFit/lib/libgenfit2.so"); 
  gSystem->Load("StgUtil.so");
  gSystem->Load("StgMaker.so");
  
  StgMaker *gmk = new StgMaker();
  gmk->Init();

  for ( int i=0;i<10; i++ ) {
    chain->Clear();
    chain->Make(); 
  }
}
