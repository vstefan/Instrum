#include "tracer.h"
#include <sstream>
#include <list>
#include <iomanip>
#include <iostream>

namespace Instrum
{

// represents a probe (1 trace can have many probes).
// each probe can have children (one method calling another..)
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
                  std::ostringstream&  outStr) const;

    std::string          name;
    timespec             start;
    timespec             end;
    bool                 sentinel;
    ProbeData*           parentPtr;
    std::list<ProbeData> children;
};

// holds the trace data and importantly, the head of the probe tree
class TraceData
{
public:
    TraceData();

    void reset();

    Resolution   res;
    bool         active;
    bool         enabled;
    unsigned int probeCount;
    ProbeData    head;

private:
    TraceData(const TraceData& copy);
    const TraceData& operator=(const TraceData& rhs);
};


// Resolution is a direct index into this array
static const char* resStrings[] = {"ns", "us", "ms", "sec"};


static TraceData  m_traceData;
static ProbeData* m_currProbePtr = 0;
bool Tracer::enabled             = false;


bool isValidTime(const timespec& tp)
{
    return (tp.tv_sec > 0 || tp.tv_nsec > 0);
}


timespec timeDiff(const timespec& start, const timespec& end)
{
    timespec temp;
    if((end.tv_nsec - start.tv_nsec) < 0)
    {
        temp.tv_sec  = end.tv_sec - start.tv_sec - 1;
        temp.tv_nsec = 1000000000 + end.tv_nsec - start.tv_nsec;
    }
    else
    {
        temp.tv_sec  = end.tv_sec  - start.tv_sec;
        temp.tv_nsec = end.tv_nsec - start.tv_nsec;
    }

    return temp;
}


unsigned long elapsedTime(const timespec& start, const timespec& end)
{
    unsigned long elapsed = 0;

    timespec diff = timeDiff(start, end);

    if(NANO_SECONDS == m_traceData.res)
    {
        elapsed = (diff.tv_sec * 1000000000) + diff.tv_nsec;
    }
    else if(MICRO_SECONDS == m_traceData.res)
    {
        elapsed = (diff.tv_sec * 1000000) + (diff.tv_nsec / 1000);
    }
    else if(MILLI_SECONDS == m_traceData.res)
    {
        elapsed = (diff.tv_sec * 1000) + (diff.tv_nsec / 1000000);
    }
    else if(SECONDS == m_traceData.res)
    {
        elapsed = diff.tv_sec + (diff.tv_nsec / 1000000000);
    }

    return elapsed;
}


unsigned int numDigits(unsigned int val)
{
    unsigned int numDigits = 0;

    while(val > 0)
    {
        numDigits++;
        val /= 10;
    }

    return numDigits;
}


ProbeData::ProbeData(const std::string& n)
    : name     (n)
     ,sentinel (false)
     ,parentPtr(0)
{
    start.tv_sec  = 0;
    start.tv_nsec = 0;
    end.tv_sec    = 0;
    end.tv_nsec   = 0;
}


ProbeData::ProbeData(const std::string& n,
                     bool               sen)
    : name     (n)
     ,sentinel (sen)
     ,parentPtr(0)
{
    start.tv_sec  = 0;
    start.tv_nsec = 0;
    end.tv_sec    = 0;
    end.tv_nsec   = 0;
}


ProbeData::ProbeData()
    : name     ()
     ,sentinel (false)
     ,parentPtr(0)
{
    start.tv_sec  = 0;
    start.tv_nsec = 0;
    end.tv_sec    = 0;
    end.tv_nsec   = 0;
}


void ProbeData::reset()
{
    name          = "";
    start.tv_sec  = 0;
    start.tv_nsec = 0;
    end.tv_sec    = 0;
    end.tv_nsec   = 0;
    sentinel      = false;
    parentPtr     = 0;
    children.clear();
}


void ProbeData::toStream(
    const unsigned long& traceElapsedTime,
    unsigned int         depth,
    unsigned int&        probeNum,
    const unsigned int&  probeCount,
    std::ostringstream&  outStr) const
{
    // first output itself
    if(!sentinel)
    {
        outStr << "#";

        for(int i=0; i<numDigits(probeCount) - numDigits(probeNum); i++)
        {
            outStr << "0";
        }

        outStr << probeNum << " ";

        for(int i=0; i<depth; i++)
        {
            outStr << "-";
        }

        outStr << "> " << name;

        if(!isValidTime(start) || !isValidTime(end))
        {
            outStr << " has invalid start/end time" << std::endl;
        }
        else
        {
            unsigned long elapsed = elapsedTime(start, end);

            outStr << " took " << elapsed << " " << resStrings[m_traceData.res]
                   << " - " << std::setprecision(2) << (elapsed/static_cast<double>(traceElapsedTime))*100.0
                   << "% of total time"
                   << std::endl;
        }
    }

    // then output any children
    if(!children.empty())
    {
        depth++;

        for(std::list<ProbeData>::const_iterator it = children.begin(); it != children.end(); it++)
        {
            it->toStream(traceElapsedTime, depth, ++probeNum, probeCount, outStr);
        }
    }
}


TraceData::TraceData()
    : res       (MILLI_SECONDS)
     ,active    (false)
     ,probeCount(0)
{}


void TraceData::reset()
{
    active     = false;
    probeCount = 0;
    head.reset();
}


void Tracer::startTrace(const std::string& name)
{
    if(!enabled) return;

    reset();

    // create sentinel node as head of probe tree
    ProbeData sentinel(name, true);
    getCurrentTime(&sentinel.start);

    m_traceData.head   = sentinel;
    m_currProbePtr     = &m_traceData.head;
    m_traceData.active = true;
}


void Tracer::finishTrace()
{
    if(!enabled) return;

    if(!m_traceData.active)
    {
        std::cerr << "finishTrace() called for inactive trace" << std::endl;
        return;
    }

    getCurrentTime(&m_traceData.head.end);
}


void Tracer::startProbe(const std::string& name)
{
    if(!enabled) return;

    if(!m_traceData.active || !m_currProbePtr)
    {
        std::cerr << "startProbe() called for inactive trace" << std::endl;
        return;
    }

    ProbeData probe(name);
    getCurrentTime(&probe.start);

    probe.parentPtr = m_currProbePtr;
    m_currProbePtr->children.push_back(probe);
    m_currProbePtr  = &(m_currProbePtr->children.back());

    m_traceData.probeCount++;
}


void Tracer::finishProbe()
{
    if(!enabled) return;

    if(!m_traceData.active)
    {
        std::cerr << "finishProbe() called for inactive trace" << std::endl;
        return;
    }

    if(!m_currProbePtr)
    {
        std::cerr << "finishProbe() called for null current probe" << std::endl;
        return;
    }

    // probe complete, current probe becomes its parent
    getCurrentTime(&(m_currProbePtr->end));
    m_currProbePtr = m_currProbePtr->parentPtr;
}


void Tracer::setResolution(Resolution res)
{
    m_traceData.res = res;
}


void Tracer::enableTracing(bool enable)
{
    enabled = enable;
}


std::string Tracer::toString()
{
    if(!enabled) return "";

    if(!m_traceData.active)
    {
        // ignore toString() for inactive trace
        return "";
    }

    std::ostringstream outStr;
    outStr.setf(std::ios::fixed);
    outStr << m_traceData.head.name;

    unsigned long elapsed = 0;

    if(!isValidTime(m_traceData.head.start) || !isValidTime(m_traceData.head.end))
    {
        outStr << " has invalid start/end time" << std::endl;
    }
    else
    {
        elapsed = elapsedTime(m_traceData.head.start, m_traceData.head.end);

        outStr << " processed for " << elapsed << " " << resStrings[m_traceData.res] << std::endl;
    }

    if(m_traceData.probeCount > 0)
    {
        unsigned int probeNum = 0;

        m_traceData.head.toStream(elapsed, 0, probeNum, m_traceData.probeCount, outStr);
    }

    return outStr.str();
}


void Tracer::getCurrentTime(timespec *tpPtr)
{
    clock_gettime(CLOCK_MONOTONIC, tpPtr);
}


void Tracer::reset()
{
    m_currProbePtr = 0;
    m_traceData.reset();
}

};
