#include "MyActionInitialization.hh"
#include "MyDetectorConstruction.hh"
#include "MyPrimaryGeneratorAction.hh"
#include "MyRunAction.hh"
#include "MyEventAction.hh"
#include "MySteppingAction.hh"

#include "G4RunManager.hh"

MyActionInitialization::MyActionInitialization() : G4VUserActionInitialization() {}
MyActionInitialization::~MyActionInitialization() = default;

void MyActionInitialization::Build() const {

    const MyDetectorConstruction* detector =
        static_cast<const MyDetectorConstruction*>(
            G4RunManager::GetRunManager()->GetUserDetectorConstruction());

    SetUserAction(new MyPrimaryGeneratorAction(detector));

    auto runAction = new MyRunAction();
    SetUserAction(runAction);

    auto eventAction = new MyEventAction(runAction);
    SetUserAction(eventAction);

    // Register stepping action (gamma R + ion MSP)
    SetUserAction(new MySteppingAction(runAction, eventAction));
}

void MyActionInitialization::BuildForMaster() const {
    // LUU Y: Geant4 MT KHONG cho phep dang ky G4VUserPrimaryGeneratorAction
    // trong BuildForMaster() (chi duoc dung trong Build() cho Worker thread) -
    // se nem G4Exception "Run0123" neu vi pham. Vi vay o day CHI tao RunAction.
    //
    // De MyRunAction::EndOfRunAction() (chay tren Master) van lay dung duoc
    // particle/energy/scan-list, xem MyPrimaryGeneratorAction: cac gia tri do
    // duoc luu vao BIEN STATIC (dung chung cho moi thread trong cung process),
    // Master doc truc tiep qua static getter, KHONG can instance rieng.
    auto runAction = new MyRunAction();
    SetUserAction(runAction);
}
