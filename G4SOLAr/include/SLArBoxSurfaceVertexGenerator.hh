/**
 * @author      Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        SLArBoxSurfaceVertexGenerator
 * @created     Tue Apr 11, 2023 09:58:25 CEST
 */

#ifndef SLARBOXSURFACEVERTEXGENERATOR_HH

#define SLARBOXSURFACEVERTEXGENERATOR_HH

#include <random>

#include "bxdecay0_g4/vertex_generator_interface.hh"

#include "G4LogicalVolume.hh"
#include "G4Box.hh"
#include "G4ThreeVector.hh"
#include "G4RotationMatrix.hh"
#include "detector/SLArGeoUtils.hh"

class SLArBoxSurfaceVertexGenerator : 
  public bxdecay0_g4::VertexGeneratorInterface 
{
  public: 
    SLArBoxSurfaceVertexGenerator(); 
    SLArBoxSurfaceVertexGenerator(const SLArBoxSurfaceVertexGenerator&); 
    ~SLArBoxSurfaceVertexGenerator() override; 

    void FixVertexFace(const bool isFaceFixed) {fFixFace = isFaceFixed;}
    void SetVertexFace(const slargeo::EBoxFace face) {fVtxFace = face;}
    slargeo::EBoxFace GetVertexFace() const {return fVtxFace;}
    const G4LogicalVolume* GetBoxLogicalVolume() const; 
    void SetBoxLogicalVolume(const G4LogicalVolume*); 
    const G4VSolid* GetSolid() const; 
    const G4ThreeVector & GetSolidTranslation() const;
    void SetSolidTranslation(const G4ThreeVector &);
    const G4RotationMatrix & GetSolidRotation() const;
    const G4RotationMatrix & GetSolidInverseRotation() const;
    void SetSolidRotation(G4RotationMatrix* );
    double GetTolerance() const;
    void SetTolerance(double tolerance_);
    void SetRandomSeed(unsigned int seed_);
    void SetNoDaughters(bool no_daughters_);
    G4double GetSurfaceGenerator() const; 

    // From the VertexGeneratorInterface abstract class:
    void ShootVertex(G4ThreeVector & vertex_) override;

  private:
    // Configuration:
    const G4LogicalVolume * fLogVol = nullptr; ///< Reference to the logical volume
    G4ThreeVector fBulkTranslation; ///< The box position in world coordinate frame
    G4RotationMatrix fBulkRotation; ///< The box rotation in world coordinate frame
    double fTolerance{1.0 * CLHEP::um}; ///< Geometrical tolerance (length)
    unsigned int fRandomSeed{0}; ///< Seed for the random number generator
    bool fNoDaughters = false; ///< Flag to reject vertexes generated from daughter volumes
    bool fFixFace = false; ///
    slargeo::EBoxFace fVtxFace = slargeo::kXplus; 

    // Working internals:
    const G4VSolid * fSolid = nullptr; ///< Reference to the solid volume from which are generated vertexes
    G4RotationMatrix fBulkInverseRotation; ///< The inverse box rotation
    unsigned int fCounter = 0.0; // Internal vertex counter
}; 

#endif /* end of include guard SLARBOXSURFACEVERTEXGENERATOR_HH */

