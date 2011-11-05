#ifndef INSTRUM_H_
#define INSTRUM_H_

#include "tracer.h"

// use this to enable/disable tracing. as this is designed
// to be built into your application and so deployed to production,
// tracing is disabled by default
#define INSTRUM_TRACE_ENABLE(enable)               \
Instrum::Tracer::enableTracing(enable);            \


// use this at the start of a method (or any scope) to start a trace
// for that scope. uses constructor & destructor so relies on
// the stack unwinding (not jump/goto safe).
#define INSTRUM_AUTO_TRACE(name)                   \
Instrum::InstrumAutoTrace instrumAutoTrace(name);  \


// use this while a trace is active to instrument an event which is
// part of the trace. uses constructor & destructor so relies on
// the stack unwinding (not jump/goto safe)
#define INSTRUM_AUTO_PROBE(name)                   \
Instrum::InstrumAutoProbe instrumAutoProbe(name);  \


// explicitly marks the end of a trace
#define INSTRUM_TRACE_FINISH()                     \
Instrum::Tracer::finishTrace();                    \


// explicitly marks the end of a probe
#define INSTRUM_PROBE_FINISH()                     \
Instrum::Tracer::finishProbe();                    \


// changes resolution of trace timing
// valid values:
// Instrum::NANO_SECONDS
// Instrum::MICRO_SECONDS
// Instrum::MILLI_SECONDS
// Instrum::SECONDS
#define INSTRUM_TRACE_RESOLUTION(res)              \
Instrum::Tracer::setResolution(res);               \


// gets trace output
// eg: std::cout << INSTRUM_TRACE_OUTPUT()
#define INSTRUM_TRACE_OUTPUT()                     \
Instrum::Tracer::toString();                       \

#endif /* INSTRUM_H_ */
