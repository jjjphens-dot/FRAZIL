#include "test_support.h"

#include <iostream>

void runParameterStateTests(TestContext&);
void runDspTests(TestContext&);
void runAudioEngineTests(TestContext&);

int main() {
    TestContext context;
    runParameterStateTests(context);
    runDspTests(context);
    runAudioEngineTests(context);

    if (context.failures != 0)
        return 1;

    std::cout << "FRAZIL unit tests passed (19 groups across parameter/state, DSP, and AudioEngine "
                 "modules)\n";
    return 0;
}
