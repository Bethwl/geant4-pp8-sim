#ifndef MyGunSharedState_h
#define MyGunSharedState_h 1

// Kho luu tru CHIA SE GIUA CAC THREAD (bien static file-scope, dung chung
// cho toan bo process) chua thong tin ve hat/nang luong ma nguoi dung da
// set qua cac lenh macro /MyGun/particleType, /MyGun/energy,
// /MyGun/particleList, /MyGun/energyListKeV.
//
// LY DO CAN FILE NAY:
//   Trong Geant4 MT, G4VUserPrimaryGeneratorAction CHI duoc dang ky o
//   Worker thread (qua Build()), TUYET DOI KHONG duoc dang ky o Master
//   thread (BuildForMaster()) - vi pham se nem G4Exception "Run0123".
//   Vi vay Master thread KHONG co instance MyPrimaryGeneratorAction, va
//   G4RunManager::GetUserPrimaryGeneratorAction() tren Master luon tra ve
//   nullptr.
//
//   Nhung MyRunAction::EndOfRunAction() (chay tren Master khi IsMaster())
//   lai can biet dung particle/energy/scan-list de tinh MSP va ghi CSV.
//
//   Giai phap: moi Worker thread, khi nhan lenh macro (UI broadcast CUNG
//   MOT lenh toi TAT CA Worker thread), se ghi gia tri do vao day (bien
//   static, dung chung ca process). Master doc truc tiep qua cac ham
//   nay, KHONG can instance rieng.

#include "G4String.hh"
#include "globals.hh"
#include <vector>

namespace MyGunShared {

    // ----- Setters: goi tu MyPrimaryGeneratorAction moi khi nhan lenh macro -----
    void SetParticleName(const G4String& name);
    void SetParticleEnergy(G4double energy);
    void SetParticleScanList(const std::vector<G4String>& list);
    void SetEnergyScanListMeV(const std::vector<G4double>& list);

    // ----- Getters: goi tu MyRunAction (Master thread) -----
    G4String GetParticleName();
    G4double GetParticleEnergy();
    const std::vector<G4String>& GetParticleScanList();
    const std::vector<G4double>& GetEnergyScanListMeV();

}

#endif
