/**
 * @author      : guff (guff@guff-gssi)
 * @file        : SLArTrackingAction
 * @created     : lunedì ago 31, 2020 18:35:29 CEST
 *
 * Reimplemented from optical/LXe/include/LXeTrackingAction.hh
 */

#ifndef SLArTRACKINGACTION_HH

#define SLArTRACKINGACTION_HH

#include "G4UserTrackingAction.hh"
#include "G4TrackingManager.hh"
#include "globals.hh"

class SLArTrackingAction : public G4UserTrackingAction {

  public:

    SLArTrackingAction();
    virtual ~SLArTrackingAction();

    virtual void PreUserTrackingAction(const G4Track*);
    virtual void PostUserTrackingAction(const G4Track*);

    G4TrackingManager* GetTrackingManager() {return fpTrackingManager;}

};




#endif /* end of include guard SLArTRACKINGACTION_HH */

