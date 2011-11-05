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


// invokes methods directly on Tracer class
void testDirect()
{
    Tracer::enableTracing(true);

    // identation used to show probe tree
    Tracer::startTrace("TRACE: Dummy-Call-Trace");

        Tracer::startProbe("METHOD: JobProcessor::start()");
        usleep(QUARTER_SEC_SLEEP);

            Tracer::startProbe("METHOD: RecordProcessor::processRecords()");
            usleep(QUARTER_SEC_SLEEP);

                Tracer::startProbe("STORED_PROC: records_pkg.fetch_records()");
                usleep(HALF_SEC_SLEEP);
                Tracer::finishProbe();

                Tracer::startProbe("METHOD: RecordProcessor::processRecord()");
                usleep(QUARTER_SEC_SLEEP);
                Tracer::finishProbe();

                Tracer::startProbe("METHOD: RecordProcessor::processRecord()");
                usleep(QUARTER_SEC_SLEEP);
                Tracer::finishProbe();

            Tracer::finishProbe();

        Tracer::finishProbe();

        Tracer::startProbe("METHOD: JobProcessor::finish()");
        usleep(ONE_SEC_SLEEP);
        Tracer::finishProbe();

    Tracer::finishTrace();

    cout << Tracer::toString();

    // change res
    Tracer::setResolution(MICRO_SECONDS);

    cout << Tracer::toString();

    Tracer::enableTracing(false);
}


// uses macros
void testMacros()
{
    INSTRUM_TRACE_ENABLE(true);

    {
        INSTRUM_AUTO_TRACE("TRACE: Dummy-Call-Auto-Trace");

        {
            INSTRUM_AUTO_PROBE("METHOD: JobProcessor::start()");
            usleep(QUARTER_SEC_SLEEP);

            {
                INSTRUM_AUTO_PROBE("METHOD: RecordProcessor::processRecords()");
                usleep(QUARTER_SEC_SLEEP);
                {
                    {
                        INSTRUM_AUTO_PROBE("STORED_PROC: records_pkg.fetch_records()");
                        usleep(HALF_SEC_SLEEP);
                    }
                    {
                        INSTRUM_AUTO_PROBE("METHOD: RecordProcessor::processRecord()");
                        usleep(QUARTER_SEC_SLEEP);
                    }
                    {
                        INSTRUM_AUTO_PROBE("METHOD: RecordProcessor::processRecord()");
                        usleep(QUARTER_SEC_SLEEP);
                    }
                }
            }
        }
        {
            INSTRUM_AUTO_PROBE("METHOD: JobProcessor::finish()");
            usleep(ONE_SEC_SLEEP);
        }
    }

    INSTRUM_TRACE_RESOLUTION(Instrum::MILLI_SECONDS);

    cout << INSTRUM_TRACE_OUTPUT();

    INSTRUM_TRACE_ENABLE(false);
}


int main()
{
    testDirect();

    testMacros();

    return 0;
}
