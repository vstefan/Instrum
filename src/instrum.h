#ifndef INSTRUM_H_
#define INSTRUM_H_

#include <string>
#include <list>
#include <time.h>

namespace Instrum
{

struct ProbeData
{
    ProbeData(const std::string& n);

    ProbeData();

    void reset();

    std::string          name;
    timespec             start;
    timespec             end;
    bool                 error;
    ProbeData*           parentPtr;
    std::list<ProbeData> children;
};

enum Resolution{
    NANO_SECONDS  = 0,
    MICRO_SECONDS = 1,
    MILLI_SECONDS = 2,
    SECONDS       = 3
};

struct TraceData
{
    TraceData();

    void reset();

    Resolution   res;
    std::string  name;
    timespec     start;
    timespec     end;
    bool         active;
    bool         error;
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

    inline static bool isValidTime(const timespec& tp);

    static unsigned long elapsedTime(const timespec& start, const timespec& end);

    static timespec timeDiff(const timespec& start, const timespec& end);

    inline static unsigned int numDigits(unsigned int val);

    inline static void reset();
};

};


#endif /* INSTRUM_H_ */
