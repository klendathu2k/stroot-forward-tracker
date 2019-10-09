void testg() {

  gROOT->LoadMacro("bfc.C");
  bfc(0,"nodefault");
  gSystem->Load("../GenFit/lib/libgenfit2.so");
  gSystem->Load("StgUtil.so");
  gSystem->Load("StgMaker.so");
  
  StgMaker *gmk = new StgMaker();


  chain->Init();

}
