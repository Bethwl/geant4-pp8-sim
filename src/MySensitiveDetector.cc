#include "MySensitiveDetector.hh"
#include "MyHits.hh"

#include "G4SDManager.hh"
#include "G4HCofThisEvent.hh"
#include "G4Step.hh"
#include "G4Track.hh"
#include "G4VTouchable.hh"

MySensitiveDetector::MySensitiveDetector(const G4String& name, const G4String& collectionName)
    : G4VSensitiveDetector(name) {
    this->collectionName.push_back(collectionName);
}

MySensitiveDetector::~MySensitiveDetector() = default;

void MySensitiveDetector::Initialize(G4HCofThisEvent* hce) {
    fHitsCollection = new MyHitsCollection(SensitiveDetectorName, collectionName[0]);
    G4int hcID = G4SDManager::GetSDMpointer()->GetCollectionID(fHitsCollection);
    hce->AddHitsCollection(hcID, fHitsCollection);
}

G4bool MySensitiveDetector::ProcessHits(G4Step* aStep, G4TouchableHistory*) {

    G4double edep = aStep->GetTotalEnergyDeposit();

    if (edep <= 0.0) return false;

    MyHit* hit = new MyHit();
    hit->SetEdep(edep);
    hit->SetVolumeName(aStep->GetPreStepPoint()->GetTouchableHandle()
        ->GetVolume()->GetName());

    hit->SetWeight(aStep->GetPreStepPoint()->GetWeight());
    hit->SetParticleName(aStep->GetTrack()->GetDefinition()->GetParticleName());
    hit->SetTrackID(aStep->GetTrack()->GetTrackID());

    fHitsCollection->insert(hit);

    return true;
}
