#ifndef VOLUME_STRUCT_HH
#define VOLUME_STRUCT_HH

#include "G4LogicalVolume.hh"
#include "G4VPhysicalVolume.hh"
#include "G4ThreeVector.hh"
#include "G4Box.hh"
#include "G4Tubs.hh"

#include "geo/SLArGeoUtils.hh"

struct VolumeStruct {
    G4LogicalVolume* logical_volume;
    G4VPhysicalVolume* physical_volume;
    //G4ThreeVector* position;
    G4ThreeVector* dimension;
    int counter;

    VolumeStruct(G4LogicalVolume* lv, G4VPhysicalVolume* pv)
        : logical_volume(lv), physical_volume(pv), counter(0) {
            if (lv) {
                auto solid = lv->GetSolid();
                dimension = new G4ThreeVector(0, 0, 0);
                if (dynamic_cast<const G4Box*>(solid)) {
                    const G4Box* box = static_cast<const G4Box*>(solid);
                    dimension->setX(box->GetXHalfLength() * 2);
                    dimension->setY(box->GetYHalfLength() * 2);
                    dimension->setZ(box->GetZHalfLength() * 2);
                } else if (dynamic_cast<const G4Tubs*>(solid)) {
                    const G4Tubs* tubs = static_cast<const G4Tubs*>(solid);
                    dimension->setX(tubs->GetInnerRadius() + tubs->GetOuterRadius());
                    dimension->setY(tubs->GetZHalfLength() * 2);
                    dimension->setZ(0);
                }
                else {
                    dimension->setX(0);
                    dimension->setY(0);
                    dimension->setZ(0);
                }
            }
            //Counter gives the number of repetitions of the logical volume in the mother volume
            if (pv && pv->GetLogicalVolume()) {
                G4LogicalVolume* mother_lv = pv->GetLogicalVolume();
                for (int i = 0; i < mother_lv->GetNoDaughters(); ++i) {
                    G4VPhysicalVolume* daughter_pv = mother_lv->GetDaughter(i);
                    if (daughter_pv && daughter_pv->GetLogicalVolume() == lv) {
                        ++counter;
                    }
                }
            }
            //position = pv ? new G4ThreeVector(pv->GetObjectTranslation()) : new G4ThreeVector(0, 0, 0);
        }    
};

#endif // VOLUME_STRUCT_HH