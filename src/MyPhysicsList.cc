#include "MyPhysicsList.hh"

#include "G4SystemOfUnits.hh"

#include "G4EmLivermorePhysics.hh"
#include "G4EmStandardPhysics.hh"
#include "G4EmStandardPhysics_option4.hh"
#include "G4DecayPhysics.hh"
#include "G4RadioactiveDecayPhysics.hh"
#include "G4EmParameters.hh" 
#include <G4IonPhysics.hh>

// KHONG can vat ly hadron/nuclear (FTFP_BERT, QGSP_BIC_HP, StoppingPhysics,
// HadronElasticPhysics) cho muc dich tinh Mass Stopping Power qua
// G4EmCalculator::GetDEDX() - ham nay CHI tra bang dE/dx duoc xay dung
// tu vat ly EM (ionisation), khong lien quan gi den hadronic physics.
// Giu lai hadron physics chi lam cham khoi dong va co nguy co xung dot
// (dang ky 2 bo hadron inelastic FTFP_BERT + QGSP_BIC_HP cung luc se loi).
//
// Neu sau nay can mo phong phan ung hat nhan / tan xa hadron thuc su,
// hay bat lai (chi chon MOT trong hai: FTFP_BERT HOAC QGSP_BIC_HP,
// khong dang ky ca hai cung luc), va them lai cac include tuong ung:
//   #include "G4HadronElasticPhysics.hh"
//   #include <G4HadronPhysicsFTFP_BERT.hh>
//   #include <G4StoppingPhysics.hh>

MyPhysicsList::MyPhysicsList()
    : G4VModularPhysicsList()
{
    G4EmParameters* emParams = G4EmParameters::Instance();
    emParams->SetBuildCSDARange(true);
    emParams->SetNumberOfBinsPerDecade(56);   // tăng độ mịn bảng dE/dx
    emParams->SetMinEnergy(10 * eV);
    emParams->SetMaxEnergy(10 * TeV);

    SetDefaultCutValue(0.7 * mm);
    SetVerboseLevel(1);

    // ===== EM Standard =====
    // DOI TU G4EmLivermorePhysics SANG G4EmStandardPhysics_option4:
    //   - Ly do: G4EmLivermorePhysics KHONG dang ky san
    //     G4IonParametrisedLossModel (bang ICRU-73) cho ion ionisation,
    //     khien G4EmCalculator::GetDEDX() cho carbon/alpha o vung nang
    //     luong thap bi sai lech rat lon (dinh Bragg peak carbon tinh ra
    //     ~16,358 keV/um, trong khi gia tri tham chieu SRIM/ICRU chi
    //     ~700-900 keV/um - sai lech ~18-20 lan).
    //   - G4EmStandardPhysics_option4 la bo physics EM chinh xac nhat cua
    //     Geant4 (dung cho medical physics/hadron therapy), co san
    //     G4IonParametrisedLossModel cho proton/alpha/ion (Bragg + ICRU-73)
    //     nen tinh MSP dung hon nhieu so voi Livermore.
    //   - DANH DOI: cac cross-section gamma nang luong thap (photoelectric/
    //     Compton/Rayleigh dung de tinh N_compton, N_photo... trong
    //     process_results.csv) se chuyen tu du lieu Livermore sang du lieu
    //     cua Standard physics - CAN CHAY LAI VA DOI CHIEU process_results.csv
    //     voi ket qua truoc do de dam bao van chap nhan duoc.
    RegisterPhysics(new G4EmStandardPhysics_option4());
    //RegisterPhysics(new G4EmLivermorePhysics());
    //RegisterPhysics(new G4EmStandardPhysics());

    // ===== Ion (can cho proton/alpha/carbon-12 di chuyen dung) =====
    RegisterPhysics(new G4IonPhysics());

    // ===== Decay =====
    RegisterPhysics(new G4DecayPhysics());
    RegisterPhysics(new G4RadioactiveDecayPhysics());
}

MyPhysicsList::~MyPhysicsList() {}