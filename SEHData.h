#ifndef SEHDATA_H
#define SEHDATA_H

#include <stddef.h>

namespace DotNetPELib {

struct SEHData
{
    enum {
        Exception = 0,
        Filter = 1,
        Finally = 2,
        Fault = 4
    } flags;
    size_t tryOffset;
    size_t tryLength;
    size_t handlerOffset;
    size_t handlerLength;
    union
    {
        size_t filterOffset;
        size_t classToken;
    };
};
}

#endif // SEHDATA_H

