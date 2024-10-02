/**
 * @author      Daniele Guffanti (daniele.guffanti@mib.infn.it), 
 *              Till Dieminger (UoM), Nicholas Lane (UoM) 
 * @file        SLArIonAndScintLArQL.h
 * @created     Tue Feb 21, 2023 17:22:25 CET
 */

#ifndef SLARIONANDSCINTLARQL_H

#define SLARIONANDSCINTLARQL_H

#include <math.h>       /* exp */
#include <vector>
#include "physics/SLArIonAndScintModel.h"
#include "G4SystemOfUnits.hh"


class SLArIonAndScintLArQL : public SLArIonAndScintModel {

  private:
    // ************************************************************************
    // LArQL Argon Scintillation Prediction PARAMETRIZATION
    // https://lss.fnal.gov/archive/2022/conf/fermilab-conf-22-088-nd.pdf
    // ************************************************************************

    // All used for LArQL/Q
    const double fElectronIonRatio;
    const double fLArQLAlpha; 
    const double fLArQLBeta; 
    std::vector<double> fLArQLChiPars; 
 
    /*
     *const double k_Birks=0.0486; //!< Birks Constant
     *const double A_Birks=0.800; //!< Birks Constant
     *const double Wion = 23.6 ; //!< W-value for ionisation in eV/e-
     *const double Nex_Ni = 0.29; //!< Ratio of number of electrons to number of ions
     *const double Ni = 1.0e6/Wion; //!< Ionization density in LAr
     *const double pLAr = 1.39;  //!< Density Liquid Argon
     *const double alpha = 0.0372; //!< Fit result for the Corr function
     *const double beta = 0.0124; //!< Fit result for the Corr function
     *const std::vector<double> chi_param = {0.00338427, -6.57037, 1.88418, 0.000129379};
     */

    //! Default Birks Law
    double QBirks(const double& dE, const double& dx, const double& E_field) const; 
    double QBirks(double& dEdx, const double& E_field) const; 
    //! Correction factor for Birks Law
    double Corr(const double& dE, const double& dx, const double& E_field) const;
    double Corr(double& dEdx, const double& E_field) const;
    //! Chi2 fit to Birks Law Correction
    double QChi(const double& dE, const double& dx, const double& E_field) const; 
    double QChi(double& dEdx, const double& E_field) const; 
    //! Charge yield at an infinite electric field -
    double Qinf() const; 

    //! Set fixed light yield for Flat-example
    double FlatLightYieldPerMeV = 24000.;


  public:
    SLArIonAndScintLArQL(); // Constructor
    SLArIonAndScintLArQL(const G4MaterialPropertiesTable* mpt); 
    ~SLArIonAndScintLArQL(); // Destructor

    // Light and charge yield functions
    Ion_and_Scint_t ComputeIonAndScintYield(const double& energyDeposit, const double& stepWidth, const double& electricField) const override;
    Ion_and_Scint_t ComputeIonAndScintYield(double& dEdx, const double& electricField) const override;
    double ComputeIonYield(const double& energy_dep, const double& hit_distance, const double& electric_field) const;
    double ComputeIonYield(double& dEdx, const double& electric_field) const;
    double Flat() const;

};

#endif /* end of include guard SLARIONANDSCINT_H */
