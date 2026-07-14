/**
 * @author      Daniele Guffanti (University and INFN Milano-Bicocca)
 * @file        SLArIonAndScintSeparate.h
 * @created     Wed Mar 01, 2023 10:39:57 CET
 */

#ifndef SLARIONANDSCINTSEPARATE_H

#define SLARIONANDSCINTSEPARATE_H

#include "LiquidArgon/SLArIonAndScintModel.h"

class SLArIonAndScintSeparate : public SLArIonAndScintModel {

  private: 
    double fModBoxA; 
    double fModBoxB; 
    bool   fUseModBoxRecomb; 
    double fLightYield;

  public: 
    SLArIonAndScintSeparate(); 
    SLArIonAndScintSeparate(const G4MaterialPropertiesTable* mpt);
    ~SLArIonAndScintSeparate() {}; 

    Ion_and_Scint_t ComputeIonAndScintYield(const double& energy_deposit, const double& step_length, const double& electric_field) const override; 
    Ion_and_Scint_t ComputeIonAndScintYield(double& dEdx, const double& electric_field) const override; 
    Ion_and_Scint_t ComputeIonAndScint(const double& energy_deposit, const double& step_length, const double& electric_field) const override; 
    void SetLightYield(const double ly) {fLightYield = ly;}
}; 



#endif /* end of include guard SLARIONANDSCINTSEPARATE_H */

