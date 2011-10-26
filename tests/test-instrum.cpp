#include "../src/instrum.h"

#include <iostream>
#include <time.h>
#include <unistd.h>

using namespace std;
using namespace Instrum;


int main()
{
    Tracer::startTrace("test-ms-trace");

    // sleep half a second
    usleep(500000);

    Tracer::finishTrace(false);

    cout << Tracer::toString();

    // change res
    Tracer::setResolution(MICRO_SECONDS);

    Tracer::startTrace("test-us-trace");

    // sleep half a second
    usleep(500000);

    Tracer::finishTrace(false);

    cout << Tracer::toString();

    return 0;
}
