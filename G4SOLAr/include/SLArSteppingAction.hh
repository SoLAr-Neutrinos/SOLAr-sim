/**
 * @author      Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        SLArSteppingAction.hh
 * @created     Sat Apr 15, 2023 15:27:09 CEST
 * @brief       Definition of the SLArSteppingAction class
 */


#ifndef SLArSteppingAction_h
#define SLArSteppingAction_h 1

#include "SLArTrackingAction.hh"
#include "SLArEventAction.hh"

#include "G4UserSteppingAction.hh"
#include "event/SLArEventTrajectory.hh"
//#include "globals.hh"

#include "G4OpBoundaryProcess.hh"


class SLArSteppingAction : public G4UserSteppingAction
{
  public:
    SLArSteppingAction(SLArEventAction*, SLArTrackingAction*);
    virtual ~SLArSteppingAction();

    virtual void UserSteppingAction(const G4Step*);

    double ph_count = 0;
    int count = 0;
    double ek_count = 0;
    double LAr_energy_count = 0;
    double ek = 0;
    int current_iev = 0;
    int event_counter = 0;

  private:
    trj_point set_evtrj_point(const G4StepPoint* point, const int nel = 0, const int nph = 0); 
    G4OpBoundaryProcessStatus fExpectedNextStatus;
    SLArEventAction*          fEventAction;
    SLArTrackingAction*       fTrackinAction;
    G4int fEventNumber;
};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif
