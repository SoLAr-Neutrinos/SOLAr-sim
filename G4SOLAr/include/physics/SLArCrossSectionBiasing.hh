/**
 * @author      Daniele Guffanti (University and INFN Milano-Bicocca)
 * @file        SLArCrossSectionBiasing.hh
 * @created     Mon Jun 19, 2023 10:53:05 CEST
 */

#ifndef SLARCROSSSECTIONBIASING

#define SLARCROSSSECTIONBIASING

#include "G4VBiasingOperator.hh"
class G4BOptnChangeCrossSection;
class G4ParticleDefinition;
#include <map>

/**
 * @brief Definition of the SLArCrossSectionBiasing class
 *
 * Originally taken from Geant4 v11.1.1 example GB01. 
 *
 * A G4VBiasingOperator concrete implementation example to
 * illustrate how to bias physics processes cross-section for
 * one particle type.
 * The G4VBiasingOperation G4BOptnChangeCrossSection is
 * selected by this operator, and is sent to each process
 * calling the operator.
 * A simple constant bias to the cross-section is applied,
 * but more sophisticated changes can be applied.
 *
 */
class SLArCrossSectionBiasing : public G4VBiasingOperator {
public:
  //! Constructor: takes the name of the particle type to bias:
  SLArCrossSectionBiasing(G4String particleToBias, G4String name = "ChangeXS");
  virtual ~SLArCrossSectionBiasing();
  
  inline void SetBiasingFactor(const G4double bias) {fBiasingFactor = bias;}
  //! method called at beginning of run:
  virtual void StartRun();
  
private:
  //! This method returns the biasing operation that will bias the physics process occurence.
  virtual G4VBiasingOperation*
  ProposeOccurenceBiasingOperation(const G4Track*                            track,
                                   const G4BiasingProcessInterface* callingProcess);
  //! Method not used:
  virtual G4VBiasingOperation*
  ProposeFinalStateBiasingOperation(const G4Track*, const G4BiasingProcessInterface*)
  {return 0;}
  //! Method not used:
  virtual G4VBiasingOperation*
  ProposeNonPhysicsBiasingOperation(const G4Track*, const G4BiasingProcessInterface*)
  {return 0;}

private:
  // -- ("using" is avoid compiler complaining against (false) method shadowing.)
  using G4VBiasingOperator::OperationApplied;

  //! Optional base class method implementation.
  //! This method is called to inform the operator that a proposed operation has been applied.
  //! In the present case, it means that a physical interaction occured (interaction at
  //! PostStepDoIt level):
  virtual void OperationApplied( const G4BiasingProcessInterface* callingProcess,
                                 G4BiasingAppliedCase biasingCase,
                                 G4VBiasingOperation* occurenceOperationApplied,
                                 G4double weightForOccurenceInteraction,
                                 G4VBiasingOperation* finalStateOperationApplied, 
                                 const G4VParticleChange* particleChangeProduced );
  
private:
  //! List of associations between processes and biasing operations:
  std::map< const G4BiasingProcessInterface*, G4BOptnChangeCrossSection* > 
                   fChangeCrossSectionOperations;
  G4bool                                  fSetup;
  G4double                        fBiasingFactor;
  const G4ParticleDefinition*    fParticleToBias;

};



#endif /* end of include guard SLARBIASINGOPERATION_HH */

