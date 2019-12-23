#include "vex.h"
#include "uilib.h"

using namespace vex;
using namespace std;
using namespace uilib;

int main() {
  // Initializing Robot Configuration. DO NOT REMOVE!
  vexcodeInit();
  
  Brain.Screen.render(true, false);
  Brain.Screen.released(Display::handlePress);
  thread renderTask(Display::render);

  while(1) {
    // Allow other tasks to run
    this_thread::sleep_for(10);
  }
}
