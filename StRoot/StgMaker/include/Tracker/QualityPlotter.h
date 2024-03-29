#ifndef QUALITY_PLOTTER_H
#define QUALITY_PLOTTER_H

#include "XmlConfig.h"
#include "HistoBins.h"
#include <string>
#include <map>

// ROOT
#include "TH1.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TVector3.h"

#include "loguru.h"

#include "FwdHit.h"

#include <FitStatus.h>

class QualityPlotter {
public:
	QualityPlotter( XmlConfig &_cfg ) : cfg(_cfg){

	}

	void makeHistograms( size_t maxI ){
		using namespace std;
		maxIterations = maxI;
		hist["FinalEff"] = new TH1F( "FinalEff", ";Eff", 100, 0, 2.0 );
		hist["FinalN7Eff"] = new TH1F( "FinalN7Eff", ";Eff", 100, 0, 2.0 );
		hist["FinalN6Eff"] = new TH1F( "FinalN6Eff", ";Eff", 100, 0, 2.0 );
		hist["FinalN5Eff"] = new TH1F( "FinalN5Eff", ";Eff", 100, 0, 2.0 );
		hist["FinalN4Eff"] = new TH1F( "FinalN4Eff", ";Eff", 100, 0, 2.0 );
		hist["FinalN7Quality"] = new TH1F( "FinalN7Quality", ";Quality", 150, 0, 1.5 );
		hist["AllQuality"] = new TH1F( "AllQuality", ";Quality", 150, 0, 1.5 );
		hist["DurationPerEvent"] = new TH1F( "DurationPerEvent", ";Duration(ms) per Event", 1e5, 0, 1e5 );

		hist["nHitsOnTrack"] = new TH1F( "nHitsOnTrack", ";nHit", 10, 0, 10 );
		hist["nHitsOnTrackMc"] = new TH1F( "nHitsOnTrackMc", ";nHit", 10, 0, 10 );

		HistoBins hb_InvPtRes( -5, 5, 0.01 ); // default
		if ( cfg.exists( "QualityPlotter.Bins.InvPtRes" ) ){
			hb_InvPtRes.load( cfg, "QualityPlotter.Bins.InvPtRes" );
		}
		HistoBins hb_InvPtRes2D( -2, 2, 0.01 ); // default
		if ( cfg.exists( "QualityPlotter.Bins.InvPtRes2D" ) ){
			hb_InvPtRes2D.load( cfg, "QualityPlotter.Bins.InvPtRes2D" );
		} 
		
		hist["InvPtRes"]      = new TH1F( "InvPtRes", ";(p_{T}^{RC} - p_{T}^{MC}) / p_{T}^{MC}", hb_InvPtRes.nBins(), hb_InvPtRes.bins.data() );
		hist["InvPtResVsNHits"]      = new TH2F( "InvPtResVsNHits", ";(p_{T}^{RC} - p_{T}^{MC}) / p_{T}^{MC}", 10, 0, 10, hb_InvPtRes.nBins(), hb_InvPtRes.bins.data() );
		hist["PtRes"]      = new TH1F( "PtRes", ";(p_{T}^{RC} - p_{T}^{MC}) / p_{T}^{MC}", hb_InvPtRes.nBins(), hb_InvPtRes.bins.data() );
		hist["PtResVsTrue"]   = new TH2F( "PtResVsTrue", ";q^{MC} #times p_{T}^{MC};(p_{T}^{RC} - p_{T}^{MC}) / p_{T}^{MC}", 100, -5, 5, hb_InvPtRes2D.nBins(), hb_InvPtRes2D.bins.data() );
		hist["InvPtResVsTrue"]   = new TH2F( "InvPtResVsTrue", ";q^{MC} #times p_{T}^{MC}; #sigma_{p_{T}^{-1}}", 100, -5, 5, hb_InvPtRes2D.nBins(), hb_InvPtRes2D.bins.data() );
		hist["DeltaPt"]      = new TH1F( "DeltaPt", ";p_{T}^{RC} - p_{T}^{MC} (GeV/c)", hb_InvPtRes.nBins(), hb_InvPtRes.bins.data() );
		hist["InvPtResVsEta"]   = new TH2F( "InvPtResVsEta", ";#eta^{MC}; #sigma_{p_{T}^{-1}}", 300, 2, 5, hb_InvPtRes2D.nBins(), hb_InvPtRes2D.bins.data() );
		
		hist["RecoPtVsMcPt"]   = new TH2F( "RecoPtVsMcPt", "; p_{T}^{MC}; p_{T}^{RC}", 100, 0, 5, 100, 0, 5 );
		hist["QMatrix"]   = new TH2F( "QMatrix", ";GEN;RECO", 6, -3, 3, 6, -3, 3);
		hist[ "FitPValue" ]  = new TH1F( "FitPValue", ";", 1000, 0, 10 );
		hist[ "FitChi2" ]  = new TH1F( "FitChi2", ";", 2000, 0, 200 );
		hist[ "FitChi2Ndf" ]  = new TH1F( "FitChi2Ndf", ";", 1500, 0, 150 );
		hist[ "FitNFailedHits" ]  = new TH1F( "FitNFailedHits", ";", 10, 0, 10 );


		hist["RightQVsMcPt"] = new TH1F( "RightQVsMcPt", ";p_{T}^{GEN}; N", 100, -5, 5 );
		hist["WrongQVsMcPt"] = new TH1F( "WrongQVsMcPt", ";p_{T}^{GEN}; N", 100, -5, 5 );
		hist["AllQVsMcPt"]   = new TH1F( "AllQVsMcPt", ";p_{T}^{GEN}; N", 100, -5, 5 );

		hist["McPt"]      = new TH1F( "McPt", ";p_{T}^{MC} (GeV/c)", 100, 0, 10 );
		hist["McPt_4hits"] = new TH1F( "McPt_4hits", ";p_{T}^{MC} (GeV/c)", 100, 0, 10 );
		hist["McPt_5hits"] = new TH1F( "McPt_5hits", ";p_{T}^{MC} (GeV/c)", 100, 0, 10 );
		hist["McPt_6hits"] = new TH1F( "McPt_6hits", ";p_{T}^{MC} (GeV/c)", 100, 0, 10 );
		hist["McPt_7hits"] = new TH1F( "McPt_7hits", ";p_{T}^{MC} (GeV/c)", 100, 0, 10 );
		hist["McPtFound"] = new TH1F( "McPtFound", ";p_{T}^{MC} (GeV/c)", 100, 0, 10 );
		hist["McPtFoundAllQ"] = new TH1F( "McPtFoundAllQ", ";p_{T}^{MC} (GeV/c)", 100, 0, 10 );

		hist["McEta"]      = new TH1F( "McEta", ";#eta^{MC} (GeV/c)", 100, 2, 5 );
		hist["McEta_4hits"]      = new TH1F( "McEta_4hits", ";#eta^{MC} (GeV/c)", 100, 2, 5 );
		hist["McEta_5hits"]      = new TH1F( "McEta_5hits", ";#eta^{MC} (GeV/c)", 100, 2, 5 );
		hist["McEta_6hits"]      = new TH1F( "McEta_6hits", ";#eta^{MC} (GeV/c)", 100, 2, 5 );
		hist["McEta_7hits"]      = new TH1F( "McEta_7hits", ";#eta^{MC} (GeV/c)", 100, 2, 5 );
		hist["McEtaFound"] = new TH1F( "McEtaFound", ";#eta^{MC} (GeV/c)", 100, 2, 5 );
		hist["McEtaFoundAllQ"] = new TH1F( "McEtaFoundAllQ", ";#eta^{MC} (GeV/c)", 100, 2, 5 );

		hist["McPhi"]      = new TH1F( "McPhi", ";#phi^{MC} (GeV/c)", 128, -3.2, 3.2 );
		hist["McPhi_4hits"]      = new TH1F( "McPhi_4hits", ";#phi^{MC} (GeV/c)", 128, -3.2, 3.2 );
		hist["McPhi_5hits"]      = new TH1F( "McPhi_5hits", ";#phi^{MC} (GeV/c)", 128, -3.2, 3.2 );
		hist["McPhi_6hits"]      = new TH1F( "McPhi_6hits", ";#phi^{MC} (GeV/c)", 128, -3.2, 3.2 );
		hist["McPhi_7hits"]      = new TH1F( "McPhi_7hits", ";#phi^{MC} (GeV/c)", 128, -3.2, 3.2 );
		hist["McPhiFound"] = new TH1F( "McPhiFound", ";#phi^{MC} (GeV/c)", 128, -3.2, 3.2 );
		hist["McPhiFoundAllQ"] = new TH1F( "McPhiFoundAllQ", ";#phi^{MC} (GeV/c)", 128, -3.2, 3.2 );
		hist["nFailedFits"] = new TH1F( "nFailedFits", ";;nFailedFits", 5, 0, 5 );


		for ( size_t track_len : { 4, 5, 6, 7 } ){
			string n = TString::Format( "McPtFound%lu", track_len ).Data();
			hist[n] = new TH1F( n.c_str(), ";p_{T}^{MC} (GeV/c)", 100, 0, 10 );
			n = TString::Format( "McEtaFound%lu", track_len ).Data();
			hist[n] = new TH1F( n.c_str(), ";#eta^{MC} (GeV/c)", 100, 2, 5 );
			n = TString::Format( "McPhiFound%lu", track_len ).Data();
			hist[n] = new TH1F( n.c_str(), ";#phi^{MC} (GeV/c)", 128, -3.2, 3.2 );
		}


		hist["McHitMap"] = new TH1F( "McHitMap", ";VID", 15, 0, 15 );
		for ( size_t i = 0; i < 15; i++ ){// hack to prevent crash...
			string n = "McHitMapLayer" + to_string( i );
			hist[n] = new TH2F( n.c_str(), ("Layer " + to_string(i) + ";x;y").c_str(), 200, 100, 100, 200, 100, 100 );
		}
		

		for ( size_t i = 0 ; i < maxIterations; i++ ){
			string n = "NTracksAfter" + to_string( i );
			hist[ n ] = new TH1F( n.c_str(), ("N Tracks After Iteration" + to_string( i )).c_str(), 500, 0, 500 );

			n = "RunningFractionFoundVsIt" + to_string( i );
			hist[n] = new TH1F( n.c_str(), (";Found ( All It <= " + to_string( i ) + " ) / Total Found").c_str(), 110, 0, 1.1 );
			
			n = "FractionFoundVsIt" + to_string( i );
			hist[n] = new TH1F( n.c_str(), (";Found ( It = " + to_string( i ) + " ) / Total Found").c_str(), 110, 0, 1.1 );

			n = "DurationIt" + to_string( i );
			hist[n] = new TH1F( n.c_str(), (";Duration(ms) for Iteration " +to_string(i)).c_str(), 1e5, 0, 1e5 );
		}
	}

	void writeHistograms(){
		for (auto nh : hist ){
			nh.second->Write();
		}
	}

	TH1 * get( std::string hn ){
		if ( hist.count( hn ) > 0 )
			return hist[hn];
		LOG_F( ERROR, "histogram name=%s does not exist, returning NULL", hn.c_str() );
		return nullptr; //careful
	}

	void startIteration() {
		// start the timer
		itStart = loguru::now_ns();
	}
	void afterIteration( size_t iteration, std::vector<KiTrack::Seed_t> acceptedTracks ){

		size_t nTracks = acceptedTracks.size();
		nTracksAfterIteration.push_back( nTracks ); // assume that we call the iterations in order

		long long itEnd = loguru::now_ns();
		long long duration = (itEnd - itStart) * 1e-6; // milliseconds
		this->get( "DurationIt" + to_string(iteration) )->Fill( duration );
		LOG_F( INFO, "Duration( It=%lu ) = %lld", iteration, duration );

	}

	void statEvent(){
		eventStart = loguru::now_ns();
	}
	void summarizeEvent( std::vector<KiTrack::Seed_t> foundTracks,  std::map<int, shared_ptr<KiTrack::McTrack>> &mcTrackMap, std::vector<TVector3> fitMoms, std::vector<genfit::FitStatus> fitStatus ){
		using namespace std;

		long long duration = (loguru::now_ns() - eventStart) * 1e-6; // milliseconds
		this->get("DurationPerEvent")->Fill( duration );

		// make a map of the number of tracks found for each # of hits
		map<size_t, size_t> tracks_found_by_nHits;
		for ( auto t : foundTracks ){
			tracks_found_by_nHits[ t.size() ]++;
		}

		for ( size_t i = 0; i  < 9; i++ ){
			if ( tracks_found_by_nHits.count( i ) > 0 )
				this->get( "nHitsOnTrack" )->Fill( i, tracks_found_by_nHits[i] );
		}

		// if ( tracks_found_by_nHits.size() > 1 ){
		// 	LOG_F( INFO, "FOUND ACCEPTANCE" );
		// }

		// the total number of tracks found (all nHits)
		size_t nTotal = foundTracks.size();


		// Now compute fraction of tracks found vs. iterations
		float runningFrac = 0;
		for ( size_t i = 0; i < maxIterations; i++ ){
			if ( nTracksAfterIteration.size() < i+1 ) {
				break;
			}
			float frac = (float)nTracksAfterIteration[i] / (float)nTotal;
			this->get( "FractionFoundVsIt" + to_string(i) )->Fill( frac );
			runningFrac += frac;
			this->get( "RunningFractionFoundVsIt" + to_string(i) )->Fill( runningFrac );
		}

		// fill McInfo
		for ( auto kv : mcTrackMap ){
			
			if ( kv.second == nullptr ) continue;
			this->get( "nHitsOnTrackMc" )->Fill( kv.second->hits.size() );
			this->get( "McPt" )->Fill( kv.second->_pt );
			this->get( "McEta" )->Fill( kv.second->_eta );
			this->get( "McPhi" )->Fill( kv.second->_phi );

			if ( kv.second->hits.size() >= 4 ){
				this->get( "McPt_4hits" )->Fill( kv.second->_pt );
				this->get( "McEta_4hits" )->Fill( kv.second->_eta );
				this->get( "McPhi_4hits" )->Fill( kv.second->_phi );
			}
			if ( kv.second->hits.size() >= 5 ){
				this->get( "McPt_5hits" )->Fill( kv.second->_pt );
				this->get( "McEta_5hits" )->Fill( kv.second->_eta );
				this->get( "McPhi_5hits" )->Fill( kv.second->_phi );
			}
			if ( kv.second->hits.size() >= 6 ){
				this->get( "McPt_6hits" )->Fill( kv.second->_pt );
				this->get( "McEta_6hits" )->Fill( kv.second->_eta );
				this->get( "McPhi_6hits" )->Fill( kv.second->_phi );
			}
			if ( kv.second->hits.size() >= 7 ){
				this->get( "McPt_7hits" )->Fill( kv.second->_pt );
				this->get( "McEta_7hits" )->Fill( kv.second->_eta );
				this->get( "McPhi_7hits" )->Fill( kv.second->_phi );
			}

			for ( auto h : kv.second->hits ){
				auto fh = static_cast<KiTrack::FwdHit*>(h);
				this->get( "McHitMap" )->Fill( abs(fh->_vid) );
				std::string n = "McHitMapLayer" + std::to_string( fh->getLayer() );
				this->get( n ) -> Fill( fh->getX(), fh->getY() );
			}
		}

		float avgQuality = 0;
		// calculate quality of tracks
		size_t track_index = 0;
		for ( auto t : foundTracks ){

			if ( t.size() > 7 ){
				LOG_F( INFO, "too many hits!" );
				
			}

			map<int, float> qual_map;
			for ( auto hit : t ){
				qual_map[ static_cast<KiTrack::FwdHit*>(hit)->_tid ]++;
			}
			for ( auto &kv : qual_map ){
				kv.second = kv.second / t.size();
			}

			// now get the quality corresponding to most hits on single track
			// we need to remeber the key also, so we can lookup a hit which corresponds to maxq - to retrieve track info
			float quality = 0;
			int mctid = -1;
			for ( auto kv : qual_map ){
				if ( kv.second >= quality ){
					quality = kv.second;
					mctid = kv.first;
				}
			}
			avgQuality += quality;
			this->get( "AllQuality" )->Fill( quality );

			if ( mctid > 0 ){
				this->get( "McPtFoundAllQ" )->Fill( mcTrackMap[ mctid ]->_pt );
				this->get( "McEtaFoundAllQ" )->Fill( mcTrackMap[ mctid ]->_eta );
				this->get( "McPhiFoundAllQ" )->Fill( mcTrackMap[ mctid ]->_phi );
			}

			if ( mctid > 0 && quality > 0.9 ){
				this->get( "McPtFound" )->Fill( mcTrackMap[ mctid ]->_pt );
				this->get( "McEtaFound" )->Fill( mcTrackMap[ mctid ]->_eta );
				this->get( "McPhiFound" )->Fill( mcTrackMap[ mctid ]->_phi );

				
				

				float mcpt = mcTrackMap[ mctid ]->_pt;
				float mceta = mcTrackMap[ mctid ]->_eta;
				int mcq  = (int)mcTrackMap[ mctid ]->_q;
				float rcpt = fitMoms[track_index].Pt();
				
				int rcq  = 0;
				float pval = 999;
				float chi2 = 999;
				float rchi2 = 999;
				int nFailedHits = 0;
				
				if ( track_index < fitStatus.size() ){
					rcq = (int)fitStatus[track_index].getCharge();
					pval = fitStatus[track_index].getPVal();
					chi2 = fitStatus[track_index].getChi2();
					rchi2 = fitStatus[track_index].getChi2() / fitStatus[track_index].getNdf();
					nFailedHits = fitStatus[track_index].getNFailedPoints();

					if ( fitStatus[track_index].isFitConvergedFully() == false ){
						rcq = -10;
						pval = 999;
						chi2 = -10;
						rchi2 = -10;
						nFailedHits = 9;
					}
				}
				
				
				

				float dPt = mcpt - rcpt;
				float dInvPt = (1.0/mcpt) - (1.0/rcpt);

				if ( t.size() >= 4 && rcpt > 0.01 ){
					this->get( "DeltaPt" )->Fill( dPt );
					this->get( "PtRes" )->Fill( dPt / mcpt );
					this->get( "InvPtRes" )->Fill( dInvPt / (1.0/mcpt) );
					this->get( "InvPtResVsNHits" )->Fill( t.size(), dInvPt / (1.0/mcpt) );
					this->get( "PtResVsTrue" )->Fill( mcpt * mcq, dPt / mcpt );
					this->get( "InvPtResVsTrue" )->Fill( mcpt * mcq, dInvPt / (1.0/mcpt) );
					this->get( "InvPtResVsEta" )->Fill( mceta, dInvPt / (1.0/mcpt) );
					this->get( "RecoPtVsMcPt" )->Fill( mcpt, rcpt );
					this->get( "FitPValue" )->Fill( pval );
					this->get( "FitChi2" )->Fill( chi2 );
					this->get( "FitChi2Ndf" )->Fill( rchi2 );
					this->get( "FitNFailedHits" )->Fill( nFailedHits );

					this->get( "QMatrix" )->Fill( mcq, rcq );

					if ( mcq == rcq )
						this->get( "RightQVsMcPt" )->Fill( mcpt * mcq );
					else if ( rcq != -10 )
						this->get( "WrongQVsMcPt" )->Fill( mcpt * mcq );
					if ( rcq != -10 )
						this->get( "AllQVsMcPt" )->Fill( mcpt * mcq );
					
				}

				if ( rcpt < 0.01 ){
					this->get( "nFailedFits" )->Fill( 1 ) ;
				}
				

				for ( size_t min_track_len : { 4, 5, 6, 7 } ){
					if ( t.size() >= min_track_len ){
						this->get( TString::Format("McPtFound%lu", min_track_len).Data() )->Fill( mcTrackMap[ mctid ]->_pt );
						this->get( TString::Format("McEtaFound%lu", min_track_len).Data() )->Fill( mcTrackMap[ mctid ]->_eta );
						this->get( TString::Format("McPhiFound%lu", min_track_len).Data() )->Fill( mcTrackMap[ mctid ]->_phi );
					}
				}
				
			} else if ( mctid == 0 ){
				this->get( "McPtFound" )->Fill( 0 );
				this->get( "McEtaFound" )->Fill( 0 );
				this->get( "McPhiFound" )->Fill( -10 );
			}

			


			// fill the pT versus efficiency for found tracks
			// this->
			track_index++;
		}
		avgQuality /= (float)nTotal;
		this->get( "FinalN7Quality" )->Fill( avgQuality );

		



		nTracksAfterIteration.clear();
	} // summarize event


	void finish(){
		this->get( "QMatrix" )->Scale( 1.0 / this->get( "QMatrix" )->GetEntries() );

		this->hist[ "EffVsMcPt" ] = (TH1*)this->get( "McPtFound" )->Clone( "EffVsMcPt" );
		this->get( "EffVsMcPt" ) -> Divide( this->get( "McPt" ) );

		this->hist[ "EffVsMcEta" ] = (TH1*)this->get( "McEtaFound" )->Clone( "EffVsMcEta" );
		this->get( "EffVsMcEta" ) -> Divide( this->get( "McEta" ) );

		this->hist[ "EffVsMcPhi" ] = (TH1*)this->get( "McPhiFound" )->Clone( "EffVsMcPhi" );
		this->get( "EffVsMcPhi" ) -> Divide( this->get( "McPhi" ) );
		for ( size_t i : { 4, 5, 6, 7 } ){
			string n = TString::Format( "McPt_%luhits", i ).Data();
			this->hist[ "EffVs" + n ] = (TH1*)this->get( TString::Format("McPtFound%lu", i).Data() )->Clone( ("EffVs" + n).c_str() );
			this->get( "EffVs" + n ) -> Divide( this->get( n ) );

			n = TString::Format( "McEta_%luhits", i ).Data();
			this->hist[ "EffVs" + n ] = (TH1*)this->get( TString::Format("McEtaFound%lu", i).Data() )->Clone( ("EffVs" + n).c_str() );
			this->get( "EffVs" + n ) -> Divide( this->get( n ) );

			n = TString::Format( "McPhi_%luhits", i ).Data();
			this->hist[ "EffVs" + n ] = (TH1*)this->get( TString::Format("McPhiFound%lu", i).Data() )->Clone( ("EffVs" + n).c_str() );
			this->get( "EffVs" + n ) -> Divide( this->get( n ) );
		}



		this->hist[ "EffVsMcPtAllQ" ] = (TH1*)this->get( "McPtFoundAllQ" )->Clone( "EffVsMcPtAllQ" );
		this->get( "EffVsMcPtAllQ" ) -> Divide( this->get( "McPt" ) );

		this->hist[ "EffVsMcEtaAllQ" ] = (TH1*)this->get( "McEtaFoundAllQ" )->Clone( "EffVsMcEtaAllQ" );
		this->get( "EffVsMcEtaAllQ" ) -> Divide( this->get( "McEta" ) );

		this->hist[ "EffVsMcPhiAllQ" ] = (TH1*)this->get( "McPhiFoundAllQ" )->Clone( "EffVsMcPhiAllQ" );
		this->get( "EffVsMcPhiAllQ" ) -> Divide( this->get( "McPhi" ) );


		// make the charge misid hists
		auto hRQ = this->get( "RightQVsMcPt" );
		auto hWQ = this->get( "WrongQVsMcPt" );
		TH1 * hAllQSum = (TH1*)hRQ->Clone("hAllQSum");
		hAllQSum->Add( hWQ );

		this->hist[ "ChargeMisIdVsMcPt" ] =  (TH1*)hWQ->Clone("ChargeMisIdVsMcPt");
		this->hist[ "ChargeMisIdVsMcPt" ] -> Divide( hAllQSum );

	}

	private:
	XmlConfig &cfg;
	std::map<std::string, TH1* > hist;

	vector<size_t> nTracksAfterIteration;
	size_t maxIterations;
	long long itStart;
	long long eventStart;
};

#endif
