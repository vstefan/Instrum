#ifndef INSTRUM_H_
#define INSTRUM_H_

#include <string>
#include <sstream>
#include <list>
#include <time.h>

namespace Instrum
{

enum Resolution{
    NANO_SECONDS  = 0,
    MICRO_SECONDS = 1,
    MILLI_SECONDS = 2,
    SECONDS       = 3
};


struct ProbeData
{
    ProbeData(const std::string& n);

    ProbeData(const std::string& n,
              bool               sen);

    ProbeData();

    void reset();

    void toStream(const unsigned long& traceElapsedTime,
                  unsigned int         depth,
                  unsigned int&        probeNum,
                  const unsigned int&  probeCount,
                  const Resolution&    res,
                  std::ostringstream&  outStr) const;

    std::string          name;
    timespec             start;
    timespec             end;
    bool                 error;
    bool                 sentinel;
    ProbeData*           parentPtr;
    std::list<ProbeData> children;
};


struct TraceData
{
    TraceData();

    void reset();

    Resolution   res;
    bool         active;
    unsigned int probeCount;
    ProbeData    head;
};

class Tracer
{

public:
    static void startTrace(const std::string& name);

    static void finishTrace(bool error);

    static void startProbe(const std::string& name);

    static void finishProbe(bool error);

    static std::string toString();

    // change elapsed time resolution
    // defaults to milliseconds.
    static void setResolution(Resolution res);

private:
    inline static void getCurrentTime(timespec *tpPtr);

    inline static void reset();
};

};


#endif /* INSTRUM_H_ */
