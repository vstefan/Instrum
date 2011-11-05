#include "../src/instrum.h"

#include <iostream>
#include <time.h>
#include <unistd.h>

using namespace std;
using namespace Instrum;

namespace
{

    const int QUARTER_SEC_SLEEP = 250000;
    const int HALF_SEC_SLEEP    = QUARTER_SEC_SLEEP*2;
    const int ONE_SEC_SLEEP     = HALF_SEC_SLEEP*2;
}

int main()
{
    // identation used to show probe tree
    Tracer::startTrace("TRACE: Dummy-Call-Trace");

        Tracer::startProbe("METHOD: JobProcessor::start()");
        usleep(QUARTER_SEC_SLEEP);

            Tracer::startProbe("METHOD: RecordProcessor::processRecords()");
            usleep(QUARTER_SEC_SLEEP);

                Tracer::startProbe("STORED_PROC: records_pkg.fetch_records()");
                usleep(HALF_SEC_SLEEP);
                Tracer::finishProbe(false);

                Tracer::startProbe("METHOD: RecordProcessor::processRecord()");
                usleep(QUARTER_SEC_SLEEP);
                Tracer::finishProbe(false);

                Tracer::startProbe("METHOD: RecordProcessor::processRecord()");
                usleep(QUARTER_SEC_SLEEP);
                Tracer::finishProbe(false);

            Tracer::finishProbe(false);

        Tracer::finishProbe(false);

        Tracer::startProbe("METHOD: JobProcessor::finish()");
        usleep(ONE_SEC_SLEEP);
        Tracer::finishProbe(false);

    Tracer::finishTrace(false);

    cout << Tracer::toString();

    // change res
    Tracer::setResolution(MICRO_SECONDS);

    cout << Tracer::toString();

    return 0;
}
