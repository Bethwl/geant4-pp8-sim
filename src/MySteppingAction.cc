#include "MySteppingAction.hh"
#include "MyRunAction.hh"
#include "MyEventAction.hh"

#include "G4Step.hh"
#include "G4Track.hh"
#include "G4VProcess.hh"
#include "G4VTouchable.hh"

MySteppingAction::MySteppingAction(MyRunAction* runAction, MyEventAction* eventAction)
    : G4UserSteppingAction(), fRunAction(runAction), fEventAction(eventAction)
{
}

void MySteppingAction::UserSteppingAction(const G4Step* step)
{
    auto track = step->GetTrack();

    if (track->GetTrackID() != 1) return;

    auto volume = step->GetPreStepPoint()->GetTouchableHandle()->GetVolume();
    if (!volume) return;
    G4String volName = volume->GetName();
    if (volName != "Sample") return;

    G4String particleName = track->GetDefinition()->GetParticleName();

    // PART 1: GAMMA → record first interaction process
    if (particleName == "gamma") {
        if (fEventAction->IsFirstInteractionRecorded()) return;

        auto process = step->GetPostStepPoint()->GetProcessDefinedStep();
        if (!process) return;

        G4String name = process->GetProcessName();
        if (name == "Transportation" || name == "CoupledTransportation") return;

        fRunAction->AddProcessCount(name);
        fEventAction->SetFirstInteractionRecorded();
        return;
    }

    // PART 2: CHARGED PARTICLE → accumulate dE and dx for Mass Stopping Power
    // (Hien khong dung cho output MSP nua - xem MyRunAction::EndOfRunAction,
    //  da chuyen sang tinh truc tiep bang G4EMCalculator. Van giu accumulate
    //  o day de co the bat lai cach tinh tracking khi can.)
    G4double charge = track->GetDefinition()->GetPDGCharge();
    if (charge != 0.0) {
        G4double dE = step->GetTotalEnergyDeposit();
        G4double dx = step->GetStepLength();

        if (dx > 0.0 && dE >= 0.0) {
            fRunAction->AddIonEnergyLoss(dE, dx);
        }
    }
}
