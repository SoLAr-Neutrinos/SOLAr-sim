/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArLArProperties.hh
 * @created     : Thursday May 16, 2024 12:36:24 CEST
 */

#ifndef SLARLARPROPERTIES_HH

#define SLARLARPROPERTIES_HH

#include <array>
#include <G4ThreeVector.hh>

class SLArLArProperties {
  public: 
    SLArLArProperties();
    SLArLArProperties(const SLArLArProperties& p);
    ~SLArLArProperties() {} 
    SLArLArProperties& operator=(const SLArLArProperties& p); 

    inline G4double GetElectricField() const {return fElectricField;}
    inline G4double GetElectronLifetime() const {return fElectronLifetime;}

    void SetElectricField(double _E) {fElectricField = _E;}
    inline void SetElectronLifetime(double lt) { fElectronLifetime = lt; fElectronLifetimeInverse = 1.0/lt; }
    void ComputeProperties(); 
    void PrintProperties() const; 

  private:
    double fElectricField;       //!< TPC Electric Field
    double fLArTemperature;      //!< Liquid Argon Temperature
    double fMuElectron;          //!< Electron mobility
    double fDiffCoefficientL;    //!< Longitudinal Diffusion Coefficient
    double fDiffCoefficientT;    //!< Transverse Diffusion Coefficient
    double fvDrift;              //!< Electron drift velocity
    double fvDriftInverse;       //!< Inverse of drift velocity, for fast computation of drift time
    double fElectronLifetime;    //!< Electron lifetime 
    double fElectronLifetimeInverse; //!< Inverse of electron lifetime, for fast computation of survival fraction

    double ComputeMobility(double E, double larT);
    double ComputeMobility(std::array<double, 2> par); 
    std::array<double,2>   ComputeDiffusion(double E, double larT); 
    double ComputeDriftVelocity(double E); 
    double FastMuDerivative(std::array<double, 2>, double, int); 

    const double a0 = 551.6; 
    const double a1 = 7158.3;
    const double a2 = 4440.43;
    const double a3 = 4.29;
    const double a4 = 43.63;
    const double a5 = 0.2053; 
    const double larT0 = 89.0; 
    const double larT1 = 87.0; 
    const double b0 = 0.0075; 
    const double b1 = 742.9; 
    const double b2 = 3269.6; 
    const double b3 = 31687.2;
 
    friend class SLArElectronDrift;
}; 


#endif /* end of include guard SLARLARPROPERTIES_HH */

