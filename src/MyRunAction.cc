#include "MyRunAction.hh"
#include "MyPrimaryGeneratorAction.hh"
#include "MyDetectorConstruction.hh"
#include "MyGunSharedState.hh"

#include "G4RunManager.hh"
#include "G4Run.hh"
#include "G4AccumulableManager.hh"
#include "G4Material.hh"
#include "G4SystemOfUnits.hh"
#include <fstream>
#include <iomanip>
#include <vector>

MyRunAction::MyRunAction()
    : G4UserRunAction(),
    fEdepNaIDet(0.0),
    fPulseNaIDet(0.0),
    fNCompton(0.0),
    fNPhoto(0.0),
    fNPair(0.0),
    fNRayleigh(0.0),
    fNOther(0.0),
    fIonEnergyLoss(0.0),
    fIonPathLength(0.0)
{
    auto* accMan = G4AccumulableManager::Instance();
    accMan->RegisterAccumulable(fEdepNaIDet);
    accMan->RegisterAccumulable(fPulseNaIDet);
    accMan->RegisterAccumulable(fNCompton);
    accMan->RegisterAccumulable(fNPhoto);
    accMan->RegisterAccumulable(fNPair);
    accMan->RegisterAccumulable(fNRayleigh);
    accMan->RegisterAccumulable(fNOther);
    accMan->RegisterAccumulable(fIonEnergyLoss);
    accMan->RegisterAccumulable(fIonPathLength);
}

MyRunAction::~MyRunAction() = default;

void MyRunAction::BeginOfRunAction(const G4Run*) {
    G4AccumulableManager::Instance()->Reset();
}

void MyRunAction::AddProcessCount(const G4String& name) {
    if (name == "compt")      fNCompton += 1.0;
    else if (name == "phot")  fNPhoto += 1.0;
    else if (name == "conv")  fNPair += 1.0;
    else if (name == "Rayl")  fNRayleigh += 1.0;
    else                      fNOther += 1.0;
}

void MyRunAction::EndOfRunAction(const G4Run* run) {
    G4AccumulableManager::Instance()->Merge();

    if (run->GetNumberOfEvent() == 0) return;

    G4double edepNaIDet = fEdepNaIDet.GetValue();
    G4double PulseNaIDet = fPulseNaIDet.GetValue();

    G4double nCompton = fNCompton.GetValue();
    G4double nPhoto = fNPhoto.GetValue();
    G4double nPair = fNPair.GetValue();
    G4double nRayleigh = fNRayleigh.GetValue();
    G4double nOther = fNOther.GetValue();
    G4double nTotal = nCompton + nPhoto + nPair + nRayleigh + nOther;

    G4double totalEloss = fIonEnergyLoss.GetValue();
    G4double totalPath = fIonPathLength.GetValue();

    if (IsMaster()) {
        // LUU Y: Master thread KHONG co instance MyPrimaryGeneratorAction
        // (Geant4 MT cam dang ky PrimaryGeneratorAction o BuildForMaster()),
        // nen GetUserPrimaryGeneratorAction() tren Master luon la nullptr.
        // Doc thay tu MyGunShared - noi Worker thread da dong bo gia tri
        // moi khi nhan lenh macro /MyGun/... (xem MyGunSharedState.hh).
        G4String particleName = MyGunShared::GetParticleName();
        G4double beamEnergy = MyGunShared::GetParticleEnergy();

        // Lay TEN vat lieu Sample tu chinh object MyDetectorConstruction (CHI
        // CO 1 instance dung chung moi thread, KHONG phai thread-local), roi
        // tra G4Material::GetMaterial() (vat lieu la GLOBAL trong Geant4,
        // khong thread-local). Cach nay dung/an toan cho ca thread MASTER,
        // khac voi tra cuu qua G4LogicalVolumeStore (THREAD-LOCAL) - vi
        // EndOfRunAction(IsMaster()==true) chay tren thread MASTER, trong
        // khi Sample_LV chu yeu duoc (re)build o cac thread WORKER, nen tra
        // cuu qua LogicalVolumeStore tu Master co the KHONG tim thay / thay
        // gia tri cu -> day la nguyen nhan msp_results.csv bi rong truoc do.
        auto* detConstruction = static_cast<const MyDetectorConstruction*>(
            G4RunManager::GetRunManager()->GetUserDetectorConstruction());
        G4String sampleMatName = detConstruction ? detConstruction->GetSampleMaterialName() : "S5";
        G4Material* sampleMat = G4Material::GetMaterial(sampleMatName);
        G4double density = sampleMat ? sampleMat->GetDensity() / (g / cm3) : 0.0;
        G4String matName = sampleMat ? sampleMat->GetName() : "Unknown";

        // Be day Sample hien tai (dieu chinh qua /DetectorConstruction/SampleThickness),
        // dung de ghi vao msp_results.csv - giup phan biet cac dong ung voi
        // cung mot vat lieu/nang luong nhung khac be day khi quet nhieu be day.
        G4double sampleThickness_um = detConstruction ? detConstruction->GetSampleThickness() / um : 0.0;

        G4cout << "\n================ ANALYSIS RESULTS ================\n";
        G4cout << "Number of events: " << run->GetNumberOfEvent() << G4endl;
        G4cout << "Particle: " << particleName << "  Energy: " << beamEnergy / keV << " keV\n";
        G4cout << "Sample:   " << matName << "  Density: " << density << " g/cm3\n";

        G4cout << "\n[NaI Detector]\n";
        G4cout << "  Edep  = " << edepNaIDet / MeV << " MeV\n";
        G4cout << "  Pulse = " << PulseNaIDet << "\n";

        if (particleName == "gamma" && nTotal > 0) {
            G4double R_compton = nCompton / nTotal;
            G4double R_photo = nPhoto / nTotal;
            G4double R_pair = nPair / nTotal;
            G4double R_rayl = nRayleigh / nTotal;
            G4double denomNoRayl = nTotal - nRayleigh;
            G4double R_compton_noRayl = (denomNoRayl > 0) ? nCompton / denomNoRayl : 0.0;

            G4cout << "\n[First Interaction Counts in Sample]\n";
            G4cout << "  N_compton  = " << nCompton << "\n";
            G4cout << "  N_photo    = " << nPhoto << "\n";
            G4cout << "  N_pair     = " << nPair << "\n";
            G4cout << "  N_rayleigh = " << nRayleigh << "\n";
            G4cout << "  N_other    = " << nOther << "\n";
            G4cout << "  N_total    = " << nTotal << "\n";
            G4cout << "  R_compton (incl Rayl) = " << R_compton << "\n";
            G4cout << "  R_compton (excl Rayl) = " << R_compton_noRayl
                << "  ← use this for Zeq (ANSI)\n";

            std::ofstream csv("process_results.csv", std::ios::app);
            csv << beamEnergy / keV << ","
                << run->GetNumberOfEvent() << ","
                << nCompton << "," << nPhoto << "," << nPair << ","
                << nRayleigh << "," << nOther << "," << nTotal << ","
                << R_compton << "," << R_photo << "," << R_pair << ","
                << R_rayl << "," << R_compton_noRayl << ","
                << PulseNaIDet << "," << edepNaIDet / MeV << "\n";
            csv.close();
        }

        // ===== Mass Stopping Power (REAL - tu tracking thuc te) =====
        // KHONG con dung G4EmCalculator (tra bang noi suy san). Thay vao do,
        // dE va dx duoc CONG DON qua TUNG STEP that su cua hat so cap
        // (TrackID==1) khi no di qua the tich "Sample" (xem
        // MySteppingAction::UserSteppingAction -> fRunAction->AddIonEnergyLoss).
        // Gia tri nay phan anh dung vat ly stochastic (energy straggling,
        // delta-ray, tan xa...) cua CHINH lan chay (beamOn) hien tai, thay vi
        // mot con so noi suy khong phu thuoc so lieu da chay.
        //
        // LUU Y QUAN TRONG: vi day la ket qua TRACKING THAT (khong phai tra
        // bang), moi diem (hat, nang luong) BAT BUOC phai duoc chay rieng
        // bang mot lenh /run/beamOn that su voi thong ke (so event) du lon -
        // danh sach quet /MyGun/particleList va /MyGun/energyListKeV (trong
        // MyGunSharedState) KHONG con duoc dung de tinh MSP nua (chi con
        // giu lai trong code de tuong thich nguoc / co the dung cho muc dich
        // khac). Muon quet nhieu diem, hay chay nhieu lenh beamOn (moi lenh
        // dat lai /MyGun/particleType va /MyGun/energy roi beamOn), moi lan
        // se ghi them 1 dong vao msp_results.csv.
        if (particleName != "gamma" && totalPath > 0 && density > 0) {
            G4double dEdx_MeVmm = (totalEloss / MeV) / (totalPath / mm);
            G4double dEdx_MeVcm = dEdx_MeVmm * 10.0;
            G4double MSP = dEdx_MeVcm / density;

            G4cout << "\n[Mass Stopping Power - REAL tracking]\n";
            G4cout << "  Particle   = " << particleName << "\n";
            G4cout << "  Energy     = " << beamEnergy / keV << " keV\n";
            G4cout << "  Material   = " << matName << "  Density: " << density << " g/cm3\n";
            G4cout << "  Thickness  = " << sampleThickness_um << " um\n";
            G4cout << "  Total dE   = " << totalEloss / MeV << " MeV\n";
            G4cout << "  Total dx   = " << totalPath / mm << " mm\n";
            G4cout << "  dE/dx      = " << dEdx_MeVcm << " MeV/cm\n";
            G4cout << "  MSP        = " << MSP << " MeV cm2/g\n";

            // Neu file chua ton tai, ghi dong header truoc de biet thu tu cot
            bool needHeader = false;
            {
                std::ifstream test("msp_results.csv");
                needHeader = !test.good() || test.peek() == std::ifstream::traits_type::eof();
            }

            std::ofstream csv("msp_results.csv", std::ios::app);
            if (needHeader) {
                csv << "particle,energy_MeV,material,density_g_cm3,thickness_um,"
                    << "totalEloss_MeV,totalPath_mm,dEdx_MeV_cm,MSP_MeV_cm2_g\n";
            }
            csv << particleName << ","
                << beamEnergy / MeV << ","
                << matName << ","
                << density << ","
                << sampleThickness_um << ","
                << totalEloss / MeV << ","
                << totalPath / mm << ","
                << dEdx_MeVcm << ","
                << MSP << "\n";
            csv.close();
        }
        else if (particleName != "gamma") {
            G4cout << "\n[Mass Stopping Power] KHONG co du lieu tracking hop le "
                << "(totalPath=" << totalPath / mm << " mm, density=" << density
                << " g/cm3) - bo qua. Kiem tra: hat co thuc su di qua the tich "
                << "'Sample' khong, va so event co du lon khong.\n";
        }

        G4cout << "=================================================\n";
    }
}

void MyRunAction::AddEdepNaIDet(G4double edep) {
    fEdepNaIDet += edep;
}

void MyRunAction::AddCountedNaIDet() {
    fPulseNaIDet += 1.0;
}