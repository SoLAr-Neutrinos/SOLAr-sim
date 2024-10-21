/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArReadoutTileSD
 * @created     : mercoledì ago 10, 2022 08:54:48 CEST
 */

#ifndef SLARREADOUTTILESD_HH

#define SLARREADOUTTILESD_HH

#include "G4VSensitiveDetector.hh"

#include "detector/Anode/SLArReadoutTileHit.hh"

class G4Step;
class G4HCofThisEvent;
class G4TouchableHistory;

class SLArReadoutTileSD : public G4VSensitiveDetector
{
public:
    SLArReadoutTileSD(G4String name);
    virtual ~SLArReadoutTileSD();
    
    virtual void Initialize(G4HCofThisEvent*HCE);
    virtual G4bool ProcessHits(G4Step* aStep, G4TouchableHistory* ROhist);
    G4bool ProcessHits_constStep(const G4Step* ,
                                 G4TouchableHistory* );
    inline void write_csv(std::string filename, std::vector<std::pair<std::string, std::vector<double>>> dataset);

    int ph_hits = 0;
    double E_Kin = 0;
    std::vector<double> xVec = {};
    std::vector<double> yVec = {};
    std::vector<double> zVec = {};
    std::vector<double> PosVec = {};  
    
    std::vector<double> energyVec ={}; 
    std::vector<double> wavelengthVec={}; 
    const double h = 6.626e-34;
    const double c = 2.99e8;
    const double e_charge = 1.6e-19;
    const double hc = (h*c)/e_charge;
   
private:

    SLArReadoutTileHitsCollection* fHitsCollection;
    G4int fHCID;
};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif /* end of include guard SLARREADOUTTILESD_HH */
