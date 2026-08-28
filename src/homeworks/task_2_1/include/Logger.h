#ifndef LOGGER_H
#define LOGGER_H

#include "Config.h"

// Logger is a NAMESPACE, not a class: plain functions grouped under Logger:: .
namespace Logger {
    void init();
    void log(const char* message);
    void log(const char* message, float value);
}

#endif
