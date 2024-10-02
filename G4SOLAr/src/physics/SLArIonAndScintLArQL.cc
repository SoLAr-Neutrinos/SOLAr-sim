#include "SLArIonAndScintLArQL.h"
#include "G4MaterialPropertiesTable.hh"
#include <iostream>


SLArIonAndScintLArQL::SLArIonAndScintLArQL() : 
  SLArIonAndScintModel(), 
  fElectronIonRatio(0.29), fLArQLAlpha(0.0372), fLArQLBeta(0.0124), 
  fLArQLChiPars( {0.00338427, -6.57037, 1.88418, 0.000129379} )
{}

SLArIonAndScintLArQL::SLArIonAndScintLArQL(const G4MaterialPropertiesTable* mpt) : 
  SLArIonAndScintModel(mpt), 
  fElectronIonRatio( mpt->GetConstProperty("LARQL_ELECTRONIONRATIO") ), 
  fLArQLAlpha( mpt->GetConstProperty("LARQL_ALPHA") ), 
  fLArQLBeta( mpt->GetConstProperty("LARQL_BETA") ), 
  fLArQLChiPars( {
      mpt->GetConstProperty("LARQL_CHIPAR0"), 
      mpt->GetConstProperty("LARQL_CHIPAR1"), 
      mpt->GetConstProperty("LARQL_CHIPAR2"), 
      mpt->GetConstProperty("LARQL_CHIPAR3") } )
{}
  
SLArIonAndScintLArQL::~SLArIonAndScintLArQL()
{
  // Default destructor
}

double SLArIonAndScintLArQL::QBirks(double& dEdx, const double& E) const {
    if(dEdx<0.1) dEdx = 0.1;
    double nom = fBirksA/fWion;
    double denom = 1+ fBirksK*dEdx /(fLArDensity * E);
    return nom/denom;
}

double SLArIonAndScintLArQL::QBirks(const double& dE, const double& dx, const double& E) const {
    double dEdx = dE/dx;
    return QBirks(dEdx, E);
}

double SLArIonAndScintLArQL::Corr(double& dEdx, const double& E) const {
    if(dEdx<1) dEdx = 1.;
    return exp(-E/(fLArQLAlpha *log(dEdx) + fLArQLBeta));
}

double SLArIonAndScintLArQL::Corr(const double& dE, const double& dx, const double& E) const {
    double dEdx = dE/dx;
    return Corr(dEdx,E);
}

double SLArIonAndScintLArQL::QChi(double& dEdx, const double& E) const {
    if(dEdx<1) dEdx = 1.;
    return fLArQLChiPars[0] / 
      (fLArQLChiPars[1]+exp(fLArQLChiPars[2] + fLArQLChiPars[3]*dEdx));
}


double SLArIonAndScintLArQL::QChi(const double& dE, const double& dx, const double& E) const {
    double dEdx = dE/dx;
    if(dEdx<1) dEdx = 1.;
    return QChi(dEdx, E);
}

double SLArIonAndScintLArQL::Qinf() const {
    return fIonizationDensity;
}

Ion_and_Scint_t SLArIonAndScintLArQL::ComputeIonAndScintYield(const double& energy_deposit, const double& hit_distance, const double& electric_field) const {
  // compute Ionization yield
    const double ion_yield = ComputeIonYield(energy_deposit, hit_distance, electric_field);
    const double scn_yield = fElectronIonRatio * fIonizationDensity + fIonizationDensity - ion_yield;
    return Ion_and_Scint_t(ion_yield, scn_yield);
}

Ion_and_Scint_t SLArIonAndScintLArQL::ComputeIonAndScintYield(double& dEdx, const double& electric_field) const {
  // compute Ionization yield
    const double ion_yield = ComputeIonYield(dEdx, electric_field);
    const double scn_yield = fElectronIonRatio * fIonizationDensity + fIonizationDensity - ion_yield;
    return Ion_and_Scint_t(ion_yield, scn_yield);
}

double SLArIonAndScintLArQL::ComputeIonYield(const double& energy_deposit, const double& hit_distance, const double& electric_field) const {
  double dEdx = energy_deposit / hit_distance;
  return ComputeIonYield(dEdx, electric_field);
}

double SLArIonAndScintLArQL::ComputeIonYield(double& dEdx, const double& electric_field) const {
  double QB = QBirks(dEdx, electric_field); 
  double QX = Corr(dEdx, electric_field)*QChi(dEdx, electric_field)*Qinf();
  return (QB + QX); 
}


double SLArIonAndScintLArQL::Flat() const {
    return FlatLightYieldPerMeV;
}
