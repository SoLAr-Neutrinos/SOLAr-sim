/**
 * @author      Daniele Guffanti (University and INFN Milano-Bicocca)
 * @file        SLArIonAndScintModel.h
 * @created     Wed Mar 01, 2023 09:53:04 CET
 */

#ifndef SLARIONANDSCINTMODEL_H

#define SLARIONANDSCINTMODEL_H

class G4MaterialPropertiesTable;

struct Ion_and_Scint_t {
  double ion;
  double scint;

  Ion_and_Scint_t();
  Ion_and_Scint_t(const double& ne, const double& nph);

};

inline Ion_and_Scint_t::Ion_and_Scint_t() : ion(0.), scint(0.) {};
inline Ion_and_Scint_t::Ion_and_Scint_t(const double& ne, const double& nph) : ion(ne), scint(nph) {}

class SLArIonAndScintModel {
  public: 
    enum EISModel {kSeparate = 0, kLArQL = 1};
    SLArIonAndScintModel();
    SLArIonAndScintModel(const G4MaterialPropertiesTable* mpt); 
    virtual ~SLArIonAndScintModel() {}; 

    virtual Ion_and_Scint_t ComputeIonAndScintYield(const double& energy_deposit, const double& step_length, const double& electric_field) const = 0; 
    virtual Ion_and_Scint_t ComputeIonAndScintYield(double& dEdx, const double& electric_field) const = 0; 
    virtual Ion_and_Scint_t ComputeIonAndScint(const double& energy_deposit, const double& step_length, const double& electric_field) const;

  protected: 
    const double fWion; 
    const double fBirksK; 
    const double fBirksA; 
    const double fLArDensity; 
    double fIonizationDensity; 

}; 

#endif /* end of include guard SLARIONANDSCINTMODEL_H */

