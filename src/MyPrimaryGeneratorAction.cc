#include "MyPrimaryGeneratorAction.hh"
#include "MyDetectorConstruction.hh"
#include "MyGunSharedState.hh"

#include "G4SystemOfUnits.hh"
#include <G4ParticleGun.hh>
#include <G4ParticleTable.hh>
#include <G4IonTable.hh>
#include <G4GenericIon.hh>
#include <G4Gamma.hh>
#include <Randomize.hh>

#include <sstream>

MyPrimaryGeneratorAction::MyPrimaryGeneratorAction(const MyDetectorConstruction* detector)
    : G4VUserPrimaryGeneratorAction(),
    fParticleGun(nullptr),
    fDetector(detector),
    fMessenger(nullptr),
    fParticleChoice("carbon"),   // mac dinh: carbon-12 (giu nhu cu)
    fEnergy(662 * keV)           // mac dinh: 662 keV (giu nhu cu)
{
    G4int n_particle = 1;
    fParticleGun = new G4ParticleGun(n_particle);

    fParticleGun->SetParticlePosition(G4ThreeVector(0, 0, 0));
    fParticleGun->SetParticleMomentumDirection(G4ThreeVector(0, 0, -1));

    // Dang ky cac lenh macro tuy chinh trong /MyGun/
    // (KHONG dat trung ten voi /gun/particle, /gun/energy co san cua G4ParticleGun
    //  de tranh xung dot command)
    fMessenger = new G4GenericMessenger(this, "/MyGun/", "Custom primary particle control");

    fMessenger->DeclareMethod("particleType", &MyPrimaryGeneratorAction::SetParticleType,
        "Chon loai hat ban (dung khi KHONG quet danh sach): proton | alpha | carbon");

    fMessenger->DeclareMethodWithUnit("energy", "MeV", &MyPrimaryGeneratorAction::SetEnergy,
        "Nang luong hat (dung khi KHONG quet danh sach): /MyGun/energy 5 MeV");

    // ===== Lenh QUET nhieu hat / nang luong trong 1 lan beamOn duy nhat =====
    // (dung cho MyRunAction::EndOfRunAction tinh MSP cho CA danh sach to hop,
    //  khong can chay lai /run/beamOn cho tung diem nang luong)
    fMessenger->DeclareMethod("particleList", &MyPrimaryGeneratorAction::SetParticleList,
        "Danh sach hat can quet, cach nhau boi dau phay. Vi du: proton,alpha,carbon");

    fMessenger->DeclareMethod("energyListKeV", &MyPrimaryGeneratorAction::SetEnergyListKeV,
        "Danh sach nang luong (keV) can quet, cach nhau boi dau phay. Vi du: 15,20,25,100,500");
}

MyPrimaryGeneratorAction::~MyPrimaryGeneratorAction() {
    delete fParticleGun;
}

void MyPrimaryGeneratorAction::SetParticleType(const G4String& name) {
    if (name == "proton" || name == "alpha" || name == "carbon") {
        fParticleChoice = name;
        MyGunShared::SetParticleName(name);   // dong bo cho Master thread doc
        G4cout << "[PrimaryGenerator] Particle set to: " << name << G4endl;
    } else {
        G4cout << "[PrimaryGenerator] WARNING: '" << name
               << "' khong hop le. Cho phep: proton | alpha | carbon. "
               << "Giu nguyen '" << fParticleChoice << "'.\n";
    }
}

void MyPrimaryGeneratorAction::SetEnergy(G4double energy) {
    fEnergy = energy;
    MyGunShared::SetParticleEnergy(energy);   // dong bo cho Master thread doc
    G4cout << "[PrimaryGenerator] Energy set to: " << fEnergy / MeV << " MeV" << G4endl;
}

void MyPrimaryGeneratorAction::SetParticleList(const G4String& csv) {
    fParticleScanList.clear();
    std::stringstream ss(csv);
    std::string token;
    while (std::getline(ss, token, ',')) {
        // trim khoang trang
        size_t a = token.find_first_not_of(" \t");
        size_t b = token.find_last_not_of(" \t");
        if (a == std::string::npos) continue;
        G4String p = token.substr(a, b - a + 1);

        if (p == "proton" || p == "alpha" || p == "carbon") {
            fParticleScanList.push_back(p);
        } else {
            G4cout << "[PrimaryGenerator] WARNING: bo qua hat khong hop le trong danh sach: '"
                   << p << "'\n";
        }
    }
    MyGunShared::SetParticleScanList(fParticleScanList);   // dong bo cho Master thread doc
    G4cout << "[PrimaryGenerator] Particle scan list (" << fParticleScanList.size()
           << " hat): " << csv << G4endl;
}

void MyPrimaryGeneratorAction::SetEnergyListKeV(const G4String& csv) {
    fEnergyScanListMeV.clear();
    std::stringstream ss(csv);
    std::string token;
    while (std::getline(ss, token, ',')) {
        size_t a = token.find_first_not_of(" \t");
        size_t b = token.find_last_not_of(" \t");
        if (a == std::string::npos) continue;
        std::string t = token.substr(a, b - a + 1);
        try {
            G4double eKeV = std::stod(t);
            fEnergyScanListMeV.push_back(eKeV * keV);
        } catch (...) {
            G4cout << "[PrimaryGenerator] WARNING: bo qua gia tri nang luong khong hop le: '"
                   << t << "'\n";
        }
    }
    MyGunShared::SetEnergyScanListMeV(fEnergyScanListMeV);   // dong bo cho Master thread doc
    G4cout << "[PrimaryGenerator] Energy scan list (" << fEnergyScanListMeV.size()
           << " muc, keV): " << csv << G4endl;
}

void MyPrimaryGeneratorAction::GeneratePrimaries(G4Event* anEvent) {
    G4GenericIon::GenericIonDefinition();

    G4ParticleDefinition* particle = nullptr;

    if (fParticleChoice == "proton") {
        particle = G4ParticleTable::GetParticleTable()->FindParticle("proton");
    }
    else if (fParticleChoice == "alpha") {
        particle = G4ParticleTable::GetParticleTable()->FindParticle("alpha");
    }
    else { // "carbon" (mac dinh) -> Carbon-12, trang thai co ban
        particle = G4IonTable::GetIonTable()->GetIon(6, 12, 0.);
    }

    fParticleGun->SetParticleDefinition(particle);
    fParticleGun->SetParticleEnergy(fEnergy);

    fParticleGun->GeneratePrimaryVertex(anEvent);
}

G4String MyPrimaryGeneratorAction::GetParticleName() const {
    return fParticleGun->GetParticleDefinition()->GetParticleName();
}

G4double MyPrimaryGeneratorAction::GetParticleEnergy() const {
    return fParticleGun->GetParticleEnergy();
}

const std::vector<G4String>& MyPrimaryGeneratorAction::GetParticleScanList() const {
    return fParticleScanList;
}

const std::vector<G4double>& MyPrimaryGeneratorAction::GetEnergyScanListMeV() const {
    return fEnergyScanListMeV;
}
