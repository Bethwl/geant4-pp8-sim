// ============================================================================
// MySteppingAction.hh
// Records:
//   (1) First interaction process of primary GAMMA in Sample → for R = N_compton/N_total
//   (2) Energy loss (dE) and step length (dx) of primary CHARGED PARTICLE in Sample
//       → for Mass Stopping Power
// ============================================================================

#ifndef MYSTEPPINGACTION_HH
#define MYSTEPPINGACTION_HH

#include "G4UserSteppingAction.hh"
#include "globals.hh"

class MyRunAction;
class MyEventAction;

class MySteppingAction : public G4UserSteppingAction {
public:
    MySteppingAction(MyRunAction* runAction, MyEventAction* eventAction);
    ~MySteppingAction() override = default;

    void UserSteppingAction(const G4Step* step) override;

private:
    MyRunAction*   fRunAction;
    MyEventAction* fEventAction;
};

#endif // MYSTEPPINGACTION_HH
