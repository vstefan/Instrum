#include "instrum.h"
#include <iostream>
#include <sstream>

namespace Instrum
{

static const char* resStrings[] = {"ns", "us", "ms", "s"};

static TraceData  m_traceData;
static ProbeData* m_currProbePtr = 0;

ProbeData::ProbeData(const std::string& n)
    : name(n)
     ,error(false)
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
    parentPtr     = 0;
    children.clear();
}


TraceData::TraceData()
    : res(MILLI_SECONDS)
     ,name()
     ,active(false)
     ,error(false)
     ,probeCount(0)
{
    start.tv_sec  = 0;
    start.tv_nsec = 0;
    end.tv_sec    = 0;
    end.tv_nsec   = 0;
}


void TraceData::reset()
{
    name          = "";
    start.tv_sec  = 0;
    start.tv_nsec = 0;
    end.tv_sec    = 0;
    end.tv_nsec   = 0;
    active        = false;
    error         = false;
    probeCount    = 0;
    head.reset();
}


void Tracer::startTrace(const std::string& name)
{
    reset();

    m_traceData.name   = name;
    m_traceData.active = true;
    getCurrentTime(&m_traceData.start);
}


void Tracer::finishTrace(bool error)
{
    if(!m_traceData.active)
    {
        // todo: log to error stream that finishTrace() was called for inactive trace
        return;
    }

    getCurrentTime(&m_traceData.end);
    m_traceData.error = error;
}


void Tracer::startProbe(const std::string& name)
{
    if(!m_traceData.active)
    {
        // todo: log to error stream that startProbe() was called for inactive trace
        return;
    }

    ProbeData probe(name);
    getCurrentTime(&probe.start);

    // first probe becomes head of the tree, has null parent
    if(!m_currProbePtr)
    {
        m_traceData.head = probe;
        m_currProbePtr   = &m_traceData.head;
    }
    // subsequent probes get appended onto the end of the list of children of the current probe,
    // then the newly added probe becomes the current one
    else
    {
        probe.parentPtr = m_currProbePtr;
        m_currProbePtr->children.push_back(probe);
        m_currProbePtr  = &(m_currProbePtr->children.back());
    }

    m_traceData.probeCount++;
}


void Tracer::finishProbe(bool error)
{
    if(!m_traceData.active)
    {
        // todo: log to error stream that finishProbe() was called for inactive trace
        return;
    }

    if(!m_currProbePtr)
    {
        // todo: log to error stream that finishProbe() was called for null current probe
        return;
    }

    // probe complete, current probe becomes its parent
    getCurrentTime(&(m_currProbePtr->end));
    m_currProbePtr->error = error;
    m_currProbePtr = m_currProbePtr->parentPtr;
}


std::string Tracer::toString()
{
    if(!m_traceData.active)
    {
        // ignore toString() for inactive trace
        return "";
    }

    std::ostringstream outStr;
    outStr << m_traceData.name;

    if(!isValidTime(m_traceData.start) || !isValidTime(m_traceData.end))
    {
        outStr << " has invalid start/end time" << std::endl;
    }
    else
    {
        unsigned long elapsed = elapsedTime(m_traceData.start, m_traceData.end);

        outStr << " processed for " << elapsed << " " << resStrings[m_traceData.res] << std::endl;
    }

    // todo: output probe tree
//    if(m_traceData.probeCount > 0)
//    {
//    	outStr << "Probes Execution Tree:" << std::endl;
//    }


    return outStr.str();
}


void Tracer::getCurrentTime(timespec *tpPtr)
{
    clock_gettime(CLOCK_MONOTONIC, tpPtr);
}


bool Tracer::isValidTime(const timespec& tp)
{
    return (tp.tv_sec > 0 || tp.tv_nsec > 0);
}


unsigned long Tracer::elapsedTime(const timespec& start, const timespec& end)
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


timespec Tracer::timeDiff(const timespec& start, const timespec& end)
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


unsigned int Tracer::numDigits(unsigned int val)
{
    unsigned int numDigits = 0;

    while(val > 0)
    {
        numDigits++;
        val /= 10;
    }

    return numDigits;
}


void Tracer::reset()
{
    m_currProbePtr = 0;
    m_traceData.reset();
}

};
