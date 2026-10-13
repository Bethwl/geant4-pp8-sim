// =========================================================================
// Header Definition
// =========================================================================

// 1. Include necessary headers
#include "MyDetectorConstruction.hh" // Geometry definition
#include "MyPhysicsList.hh" // Custom Physics processes (include Biasing)
#include "MyActionInitialization.hh" // Initializes User Action classes (Run, Event, Step, PrimaryGenerator)

// 2. Include Geant4 headers
#include <G4RunManagerFactory.hh> // For creating Run Manager
#include <G4UImanager.hh> // Manages the user interface commands
#include <G4VisExecutive.hh> // Manages visualization
#include <G4UIExecutive.hh> // Manages UI sessions
#include <Randomize.hh> // For random number generation
#include <G4IonTable.hh> // De ep tao Carbon-12 truoc khi build bang physics

// =========================================================================
// MAIN FUNCTION
// =========================================================================
int main(int argc, char** argv)
{
    // --- 1. SET UP RUN MANAGER ---
#ifdef G4MULTITHREADED
    auto runManager = G4RunManagerFactory::CreateRunManager(G4RunManagerType::MT);
    runManager->SetNumberOfThreads(4);
#else
    auto runManager = G4RunManagerFactory::CreateRunManager(G4RunManagerType::Serial);
#endif

    // --- 2. SET UP MANDATORY CLASSES ---

    // Set a random engine
    CLHEP::HepRandom::setTheSeed(1234567);

    // a) Detector Construction (Geometry and VR Regions)
    auto detConstruction = new MyDetectorConstruction();
    runManager->SetUserInitialization(detConstruction);

    // b) Physics List (Physics and Biasing Activation)
    auto physicsList = new MyPhysicsList();
    runManager->SetUserInitialization(physicsList);

    // c) User Action Initialization (Primary Generator, RunAction, etc.)
    auto actionInitialization = new MyActionInitialization();
    runManager->SetUserInitialization(actionInitialization);

    // ===== EP TAO CARBON-12 TRUOC KHI runManager->Initialize() =====
    // Ly do: G4IonTable::GetIon(Z,A,E) la "lazy instantiate" - Carbon-12
    // CHI duoc Geant4 biet toi tu thoi diem ham nay duoc goi lan dau.
    // Neu lan goi dau tien xay ra SAU khi Initialize() da build xong bang
    // physics (dE/dx, CSDA range...) - vi du trong MyRunAction::EndOfRunAction()
    // luc quet particleList "carbon" - thi carbon-12 se KHONG co san trong
    // bang physics duoc build luc Initialize(), khien G4EmCalculator::GetDEDX()
    // / GetCSDARange() tra ve gia tri sai lech nghiem trong (hoac 0) cho carbon,
    // trong khi proton/alpha (particle co san trong G4ParticleTable ngay tu dau)
    // van dung binh thuong.
    // => Goi GetIon() ngay tai day de dam bao carbon-12 duoc dang ky TRUOC
    // Initialize(), bang physics cho carbon se duoc build day du tu dau.
    G4IonTable::GetIonTable()->GetIon(6, 12, 0.);

    runManager->Initialize();

    // --- 3. VISUALIZATION AND UI (User Interface) ---
    G4UIExecutive* ui = nullptr;
    if (argc == 1) {
        // Running in interactive mode (GUI)
        ui = new G4UIExecutive(argc, argv);
    }

    // a) Initialize Visualization Manager
    auto visManager = new G4VisExecutive();
    visManager->Initialize();

    // b) Get UIManager (command handler)
    G4UImanager* UImanager = G4UImanager::GetUIpointer();

    UImanager->ApplyCommand("/control/macroPath " MACROPATH); // Define macro path

    if (ui) {
        // interactive mode
        UImanager->ApplyCommand("/control/execute init_vis.mac");
        ui->SessionStart();
        delete ui;

    }
    else {
        // batch mode        
        G4String fileName = argv[1];
        UImanager->ApplyCommand("/control/execute " + fileName);
    }

    // --- 4. CLEAN UP ---
    delete visManager;
    delete runManager;

    return 0;
}