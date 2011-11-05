#ifndef TRACER_H_
#define TRACER_H_

#include <string>
#include <time.h>

namespace Instrum
{

enum Resolution{
    NANO_SECONDS  = 0,
    MICRO_SECONDS = 1,
    MILLI_SECONDS = 2,
    SECONDS       = 3
};


// does all the work - interact with this class through the
// macros in instrum.h
class Tracer
{

public:
    static void startTrace(const std::string& name);

    static void finishTrace();

    static void startProbe(const std::string& name);

    static void finishProbe();

    // returns a string representation of the trace
    // and probe tree
    static std::string toString();

    // change elapsed time resolution
    // defaults to milliseconds.
    static void setResolution(Resolution res);

    // Tracer will do nothing if tracing is not enabled
    // Tracer is disabled by default
    static void enableTracing(bool enable);

private:
    inline static void getCurrentTime(timespec *tpPtr);

    inline static void reset();

    static bool enabled;
};


// the magic behind the INSTRUM_AUTO_TRACE() macro
class InstrumAutoTrace
{
public:
    InstrumAutoTrace(const std::string& name)
    {
        Tracer::startTrace(name);
    }

    ~InstrumAutoTrace()
    {
        Tracer::finishTrace();
    }

private:
    InstrumAutoTrace();
    InstrumAutoTrace(const InstrumAutoTrace& copy);
    const InstrumAutoTrace& operator=(const InstrumAutoTrace& rhs);
};


// the magic behind the INSTRUM_AUTO_PROBE() macro
class InstrumAutoProbe
{
public:
    InstrumAutoProbe(const std::string& name)
    {
        Tracer::startProbe(name);
    }

    ~InstrumAutoProbe()
    {
        Tracer::finishProbe();
    }

private:
    InstrumAutoProbe();
    InstrumAutoProbe(const InstrumAutoProbe& copy);
    const InstrumAutoProbe& operator=(const InstrumAutoProbe& rhs);
};

};


#endif /* TRACER_H_ */
