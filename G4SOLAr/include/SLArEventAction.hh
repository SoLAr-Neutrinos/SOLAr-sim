//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
// $Id: SLArEventAction.hh 94486 2015-11-19 08:33:37Z gcosmo $
//
/// \file SLArEventAction.hh
/// \brief Definition of the SLArEventAction class

#ifndef SLArEventAction_h
#define SLArEventAction_h 1


#include "G4UserEventAction.hh"
#include "G4HCofThisEvent.hh"
#include "globals.hh"

#include <cassert>
#include <map>
#include <vector>

/// Event action

class SLArEventAction : public G4UserEventAction
{
public:
    SLArEventAction();
    virtual ~SLArEventAction();
    
    virtual void BeginOfEventAction(const G4Event*);
    virtual void EndOfEventAction(const G4Event*);

    inline void IncPhotonCount_Scnt()  {fPhotonCount_Scnt++;}
    inline void IncPhotonCount_Cher()  {fPhotonCount_Cher++;}
    inline void IncPhotonCount_WLS ()  {fPhotonCount_WLS ++;}
    inline void IncAbsorption()        {fAbsorptionCount++;}
    inline void IncBoundaryAbsorption(){fBoundaryAbsorptionCount++;}
    inline void IncHitCount(G4int i=1) {fHitCount+=i;}
    inline void IncReadoutTileHitCount() {fReadoutTileHits++;}
    inline void IncSuperCellHitCount() {fSuperCellHits++;}

    inline G4int GetPhotonCount_Scnt()const    {return fPhotonCount_Scnt;}
    inline G4int GetPhotonCount_Cher()const    {return fPhotonCount_Cher;}
    inline G4int GetPhotonCount_WLS ()const    {return fPhotonCount_WLS ;}
    inline G4int GetReadoutTileHits ()const    {return fReadoutTileHits ;}
    inline G4int GetSuperCellHits   ()const    {return fSuperCellHits   ;}
    inline G4int GetHitCount()const            {return fHitCount;}
    inline G4double GetEDep()const             {return fTotEdep;}
    inline G4int GetAbsorptionCount()const     {return fAbsorptionCount;}
    inline G4int GetBoundaryAbsorptionCount()const {return fBoundaryAbsorptionCount;}
    int   FindAncestorID(int); 
    void  RegisterNewTrackPID(int, int); 

    struct TrackIdHelpInfo_t {
      int parent;
      int pdg;
      double quadrimomentum[4];

      inline TrackIdHelpInfo_t() : parent(-1), pdg(-1), quadrimomentum{0} {}
      inline TrackIdHelpInfo_t(const int& _pid, const int& _pdg, const double* p) 
        : parent(_pid), pdg(_pdg) {
          for (size_t i = 0; i < 4; i++) {
            quadrimomentum[i] = p[i];
          }
        }
      bool operator<(const TrackIdHelpInfo_t& other) const {
        if (parent != other.parent) {
          return parent < other.parent;
        } else if (pdg != other.pdg) {
          return pdg < other.pdg;
        } else {
          if (quadrimomentum[3] != other.quadrimomentum[3]) {
            return quadrimomentum[3] < other.quadrimomentum[3];
          }
          for (int i = 0; i < 3; ++i) {
            if (quadrimomentum[i] != other.quadrimomentum[i]) {
              return quadrimomentum[i] < other.quadrimomentum[i];
            }
          }
        }
        return false; // Gli oggetti sono uguali
      }
      inline bool operator==(const TrackIdHelpInfo_t& other) const {
        return parent == other.parent &&
               pdg == other.pdg &&
               std::equal(std::begin(quadrimomentum), std::end(quadrimomentum), std::begin(other.quadrimomentum));
      }
      inline bool operator!=(const TrackIdHelpInfo_t& other) const {
        return !(*this == other); 
      }
    };
    void  RegisterNewProcessExtraInfo(const TrackIdHelpInfo_t&, G4String&);
    inline std::map<TrackIdHelpInfo_t, G4String>& GetProcessExtraInfo() {return fExtraProcessInfo;}
    inline const std::map<TrackIdHelpInfo_t, G4String>& GetProcessExtraInfo() const {return fExtraProcessInfo;}
    
private:
    G4int fTileHCollID;
    G4int fSuperCellHCollID;
    std::vector<G4int> fLArHCollID;
    std::vector<G4int> fExtScorerHCollID;

    G4int fHitCount;
    G4int fPhotonCount_Scnt;
    G4int fPhotonCount_Cher;
    G4int fPhotonCount_WLS ;
    G4int fReadoutTileHits ; 
    G4int fSuperCellHits   ; 
    G4int fAbsorptionCount ;
    G4int fBoundaryAbsorptionCount;
    G4double fTotEdep;

    std::map<int, int> fParentIDMap;
    std::map<TrackIdHelpInfo_t, G4String> fExtraProcessInfo;

    void RecordEventReadoutTile (const G4Event* ev, const G4int& verbose = 0);
    void RecordEventSuperCell( const G4Event* ev, const G4int& verbose = 0); 
    void RecordEventLAr(const G4Event* ev, const G4int& verbose = 0);
    void RecordEventExtScorer(const G4Event* ev, const G4int& verbose = 0); 
};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif
