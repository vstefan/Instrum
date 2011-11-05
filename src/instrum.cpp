#include "instrum.h"
#include <iomanip>
#include <iostream>

namespace Instrum
{

static const char* resStrings[] = {"ns", "us", "ms", "sec"};

static TraceData  m_traceData;
static ProbeData* m_currProbePtr = 0;

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
    : name(n)
     ,error(false)
     ,sentinel(false)
     ,parentPtr(0)
{
    start.tv_sec  = 0;
    start.tv_nsec = 0;
    end.tv_sec    = 0;
    end.tv_nsec   = 0;
}

ProbeData::ProbeData(const std::string& n,
                     bool               sen)
    : name(n)
     ,error(false)
     ,sentinel(sen)
     ,parentPtr(0)
{
    start.tv_sec  = 0;
    start.tv_nsec = 0;
    end.tv_sec    = 0;
    end.tv_nsec   = 0;
}

ProbeData::ProbeData()
    : name()
     ,error(false)
     ,sentinel(false)
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
    error         = false;
    sentinel      = false;
    parentPtr     = 0;
    children.clear();
}


void ProbeData::toStream(
    const unsigned long& traceElapsedTime,
    unsigned int         depth,
    unsigned int&        probeNum,
    const unsigned int&  probeCount,
    const Resolution&    res,
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

            outStr << " took " << elapsed << " " << resStrings[res]
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
            it->toStream(traceElapsedTime, depth, ++probeNum, probeCount, res, outStr);
        }
    }
}


TraceData::TraceData()
    : res(MILLI_SECONDS)
     ,active(false)
     ,probeCount(0)
{}


void TraceData::reset()
{
    active        = false;
    probeCount    = 0;
    head.reset();
}


void Tracer::startTrace(const std::string& name)
{
    reset();

    // create sentinel node
    ProbeData sentinel(name, true);
    getCurrentTime(&sentinel.start);

    m_traceData.head   = sentinel;
    m_currProbePtr     = &m_traceData.head;
    m_traceData.active = true;
}


void Tracer::finishTrace(bool error)
{
    if(!m_traceData.active)
    {
        std::cout << "finishTrace() called for inactive trace" << std::endl;
        // todo: log to error stream that finishTrace() was called for inactive trace
        return;
    }

    getCurrentTime(&m_traceData.head.end);
    m_traceData.head.error = error;
}


void Tracer::startProbe(const std::string& name)
{
    if(!m_traceData.active || !m_currProbePtr)
    {
        std::cout << "startProbe() called for inactive trace" << std::endl;
        // todo: log to error stream that startProbe() was called for inactive trace
        return;
    }

    ProbeData probe(name);
    getCurrentTime(&probe.start);

    probe.parentPtr = m_currProbePtr;
    m_currProbePtr->children.push_back(probe);
    m_currProbePtr  = &(m_currProbePtr->children.back());

    m_traceData.probeCount++;
}


void Tracer::finishProbe(bool error)
{
    if(!m_traceData.active)
    {
        std::cout << "finishProbe() called for inactive trace" << std::endl;
        // todo: log to error stream that finishProbe() was called for inactive trace
        return;
    }

    if(!m_currProbePtr)
    {
        std::cout << "finishProbe() called for null current probe" << std::endl;

        // todo: log to error stream that finishProbe() was called for null current probe
        return;
    }

    // probe complete, current probe becomes its parent
    getCurrentTime(&(m_currProbePtr->end));
    m_currProbePtr->error = error;
    m_currProbePtr = m_currProbePtr->parentPtr;
}


void Tracer::setResolution(Resolution res)
{
    m_traceData.res = res;
}


std::string Tracer::toString()
{
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

        m_traceData.head.toStream(elapsed, 0, probeNum, m_traceData.probeCount, m_traceData.res, outStr);
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
