#include "../src/instrum.h"

#include <iostream>
#include <time.h>
#include <unistd.h>

using namespace std;
using namespace Instrum;


int main()
{
    Tracer::startTrace("test-trace");

    // sleep half a second
    usleep(500000);

    Tracer::finishTrace(false);

    cout << Tracer::toString() << endl;

    return 0;
}
