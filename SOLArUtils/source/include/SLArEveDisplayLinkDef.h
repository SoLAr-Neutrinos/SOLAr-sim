#ifdef __CINT__
#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;

#pragma link C++ namespace display;

// Top-level orchestrator — uses ClassDef / ClassImp.
#pragma link C++ class display::SLArEveOpHitSelectorPanel+;
#pragma link C++ class display::SLArEveDisplay+;

// MCParticleSelector_t uses ROOT Connect() signals from the GUI, so its
// methods must be visible to CINT even though it is not a TObject subclass.
#pragma link C++ class display::MCParticleSelector_t+;

// IDList is trivial but referenced from the dictionary compilation unit.
#pragma link C++ class display::IDList+;

#endif
