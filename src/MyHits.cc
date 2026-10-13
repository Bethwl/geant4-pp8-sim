#include "MyHits.hh"

#include "G4VVisManager.hh"
#include "G4Circle.hh"
#include "G4ios.hh"
#include "G4SystemOfUnits.hh"

void MyHit::Draw()
{
    G4VVisManager* pVVisManager = G4VVisManager::GetConcreteInstance();
    if (pVVisManager)
    {
        // Nếu muốn, vẽ điểm hit, hiện tại để trống
    }
}

void MyHit::Print()
{
    G4cout
        << "  --- Hit ---"
        << "  Volume: " << fVolumeName
        << "  Particle: " << fParticleName
        << "  Edep: " << G4BestUnit(fEdep, "Energy")
        << "  Weight (VR): " << fWeight
        << G4endl;
}
