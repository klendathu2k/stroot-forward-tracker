#ifndef CRIT_HISTO_H
#define CRIT_HISTO_H

// STL
#include <map>
#include <string>

// ROOT
#include "TH1.h"
#include "TH1F.h"
#include "TFile.h"
#include "TTree.h"


#include "XmlConfig/loguru.h"

// class Crit2SegmentTreeData {
// 	public:
// 	float ax, ay, az;
// 	float bx, by, bz;
// 	int atid, btid;
// 	int avid, bvid;


// };

class Crit2TreeData : public TObject {
	public:
	bool real;
	float Crit2_DeltaPhi;
	float Crit2_DeltaTheta_MV;
	float Crit2_Distance_MV;
	float Crit2_DeltaRho;
	float Crit2_RZRatio;
	float Crit2_StraightTrackRatio;
	float Crit3_2DAngle;
	float Crit3_2DAngleTimesR;
	void reset(){
		real = false;
		Crit2_DeltaPhi = -9999;
		Crit2_DeltaTheta_MV = -9999;
		Crit2_Distance_MV = -9999;
		Crit2_DeltaRho = -9999;
		Crit2_RZRatio = -9999;
		Crit2_StraightTrackRatio = -9999;
		Crit3_2DAngle = -9999;
		Crit3_2DAngleTimesR = -9999;
	}
};


class CritHisto {
public:
	
	static CritHisto * Instance(){
		if ( !instance ){
			instance = new CritHisto();
			instance->writeTree = true;
			instance->MakeHistos();
			
			

		}
		return instance;

	}

	static void Fill( std::string name, float v ){
		if ( Instance()->histograms.count( name ) > 0 && Instance()->histograms[ name ] != nullptr ){
			Instance()->histograms[name]->Fill( v );
		}
	}
	static void Record( std::string critName, float value, bool same_track = false ){
		// return;
		std::string name = critName;
		Fill( name, value );
		if ( same_track ){
			name = name + "_Real";
			Fill( name, value );
		}

		// fill the tree
		Instance()->c2data.real = same_track;
		if ("Crit2_DeltaPhi" == critName )
			Instance()->c2data.Crit2_DeltaPhi = value;
		if ("Crit2_DeltaTheta_MV" == critName )
			Instance()->c2data.Crit2_DeltaTheta_MV = value;
		if ("Crit2_Distance_MV" == critName )
			Instance()->c2data.Crit2_Distance_MV = value;
		if ("Crit2_DeltaRho" == critName )
			Instance()->c2data.Crit2_DeltaRho = value;
		if ("Crit2_RZRatio" == critName )
			Instance()->c2data.Crit2_RZRatio = value;
		if ("Crit2_StraightTrackRatio" == critName )
			Instance()->c2data.Crit2_StraightTrackRatio = value;
		if ("Crit3_2DAngle" == critName )
			Instance()->c2data.Crit3_2DAngle = value;
		if ("Crit3_2DAngleTimesR" == critName )
			Instance()->c2data.Crit3_2DAngleTimesR = value;
		
	}

	static void Save() {
		// TFile *fCritHistograms = new TFile( "fCritHistograms.root", "RECREATE" );
		Instance()->fCritHistograms->cd();
		for ( auto kv : Instance()->histograms ){
			kv.second->Write();
		}

		if ( Instance()->writeTree )
			Instance()->tree->Write();

		Instance()->fCritHistograms->Write();

	}


	static void ResetTree(){
		if ( Instance()->writeTree )
			Instance()->c2data.reset();
	}
	static void FillTree(){
		if ( Instance()->writeTree )
			Instance()->tree->Fill();
	}

	protected:
	static CritHisto * instance;
	std::map<std::string, TH1 *> histograms;
	TTree * tree;
	Crit2TreeData c2data;
	TFile *fCritHistograms;
	bool writeTree;


	private:
	CritHisto() {
	}

	~CritHisto() {
		
	}

	void setupTree(){
		

		fCritHistograms = new TFile( "fCritHistograms.root", "RECREATE" );
		fCritHistograms->cd();

		if ( Instance()->writeTree == false ) return;
		tree = new TTree( "Crit2", "Crit2" );
		
		tree->Branch( "data", &c2data.real, "real/O" );
		tree->Branch( "Crit2_DeltaPhi", &c2data.Crit2_DeltaPhi, "Crit2_DeltaPhi/F");
		tree->Branch( "Crit2_DeltaTheta_MV", &c2data.Crit2_DeltaTheta_MV, "Crit2_DeltaTheta_MV/F");
		tree->Branch( "Crit2_Distance_MV", &c2data.Crit2_Distance_MV, "Crit2_Distance_MV/F");
		tree->Branch( "Crit2_DeltaRho", &c2data.Crit2_DeltaRho, "Crit2_DeltaRho/F");
		tree->Branch( "Crit2_RZRatio", &c2data.Crit2_RZRatio, "Crit2_RZRatio/F");
		tree->Branch( "Crit2_StraightTrackRatio", &c2data.Crit2_StraightTrackRatio, "Crit2_StraightTrackRatio/F");
		tree->Branch( "Crit3_2DAngle", &c2data.Crit3_2DAngle, "Crit3_2DAngle/F");
		tree->Branch( "Crit3_2DAngleTimesR", &c2data.Crit3_2DAngleTimesR, "Crit3_2DAngleTimesR/F");
	}

	void MakeHistos(){
		// LOG_F( INFO, "CritHisto" );
		// build the histograms!
		Instance()->histograms["Crit2_DeltaPhi"]      = new TH1F( "Crit2_DeltaPhi", "Crit2_DeltaPhi (All); DeltaPhi", 3600, 0, 360 );
		Instance()->histograms["Crit2_DeltaPhi_Real"] = new TH1F( "Crit2_DeltaPhi_Real", "Crit2_DeltaPhi (Real); DeltaPhi", 3600, 0, 360 );

		Instance()->histograms["Crit2_DeltaTheta_MV"]      = new TH1F( "Crit2_DeltaTheta", "Crit2_DeltaTheta (All); DeltaTheta", 360, 0, 360 );
		Instance()->histograms["Crit2_DeltaTheta_MV_Real"] = new TH1F( "Crit2_DeltaTheta_Real", "Crit2_DeltaTheta (Real); DeltaTheta", 360, 0, 360 );

		Instance()->histograms["Crit2_Distance_MV"]      = new TH1F( "Crit2_Distance_MV", "Crit2_Distance_MV (All); Distance_MV", 500, 0, 500 );
		Instance()->histograms["Crit2_Distance_MV_Real"] = new TH1F( "Crit2_Distance_MV_Real", "Crit2_Distance_MV (Real); Distance_MV", 500, 0, 500 );

		Instance()->histograms["Crit2_DeltaRho"]      = new TH1F( "Crit2_DeltaRho", "Crit2_DeltaRho (All); DeltaRho", 500, -50, 50 );
		Instance()->histograms["Crit2_DeltaRho_Real"] = new TH1F( "Crit2_DeltaRho_Real", "Crit2_DeltaRho (Real); DeltaRho", 500, -50, 50 );

		Instance()->histograms["Crit2_RZRatio"]      = new TH1F( "Crit2_RZRatio", "Crit2_RZRatio (All); RZRatio", 5000, 0.99, 5 );
		Instance()->histograms["Crit2_RZRatio_Real"] = new TH1F( "Crit2_RZRatio_Real", "Crit2_RZRatio (Real); RZRatio", 5000, 0.99, 5 );

		Instance()->histograms["Crit2_StraightTrackRatio"]      = new TH1F( "Crit2_StraightTrackRatio", "Crit2_StraightTrackRatio (All); StraightTrackRatio", 1400, -2, 5 );
		Instance()->histograms["Crit2_StraightTrackRatio_Real"] = new TH1F( "Crit2_StraightTrackRatio_Real", "Crit2_StraightTrackRatio (Real); StraightTrackRatio", 1400, -2, 5 );


		Instance()->histograms["Crit3_2DAngle"]      = new TH1F( "Crit3_2DAngle", "Crit3_2DAngle (All); 2DAngle", 450, 0, 45 );
		Instance()->histograms["Crit3_2DAngle_Real"] = new TH1F( "Crit3_2DAngle_Real", "Crit3_2DAngle (Real); 2DAngle", 450, 0, 45 );

		Instance()->histograms["Crit3_3DAngle"]      = new TH1F( "Crit3_3DAngle", "Crit3_3DAngle (All); 3DAngle", 3600, -360, 360 );
		Instance()->histograms["Crit3_3DAngle_Real"] = new TH1F( "Crit3_3DAngle_Real", "Crit3_3DAngle (Real); 3DAngle", 3600, -360, 360 );

		Instance()->histograms["Crit3_2DAngleTimesR"]      = new TH1F( "Crit3_2DAngleTimesR", "Crit3_2DAngleTimesR (All); 2DAngle", 360, 0, 3600 );
		Instance()->histograms["Crit3_2DAngleTimesR_Real"] = new TH1F( "Crit3_2DAngleTimesR_Real", "Crit3_2DAngleTimesR (Real); 2DAngle", 360, 0, 3600 );
		

		Instance()->histograms["Crit3_ChangeRZRatio"]      = new TH1F( "Crit3_ChangeRZRatio", "Crit3_ChangeRZRatio (All); ChangeRZRatio", 500, 0.5, 1.5 );
		Instance()->histograms["Crit3_ChangeRZRatio_Real"] = new TH1F( "Crit3_ChangeRZRatio_Real", "Crit3_ChangeRZRatio (Real); ChangeRZRatio", 500, 0.5, 1.5 );

		Instance()->histograms["Crit3_PT"]      = new TH1F( "Crit3_PT", "Crit3_PT (All); PT", 1000, 0, 10 );
		Instance()->histograms["Crit3_PT_Real"] = new TH1F( "Crit3_PT_Real", "Crit3_PT (Real); PT", 1000, 0, 10 );
		

		for ( auto kv : Instance()->histograms ){
			if ( kv.first.find( "Real" ) != std::string::npos ){
				kv.second->SetLineColor(kRed);
			}
		} // loop on histograms


		Instance()->setupTree();

	}

	

};




#endif