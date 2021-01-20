//
// Created by yuwenyong on 2020/11/21.
//

#include "EasyEvent/Logging/LogCommon.h"

enum { Stride = 256 };

extern const char gHexCharTable[2][16] =
{
    { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' },
    { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' }
};


void EasyEvent::dumpData(const void *data, size_t size, std::ostream &strm) {
    char buf[Stride * 3u];

    const char* const charTable = gHexCharTable[(strm.flags() & std::ios_base::uppercase) != 0];
    const std::size_t strideCount = size / Stride, tailSize = size % Stride;

    const uint8_t* p = static_cast< const uint8_t* >(data);
    char* bufBeg = buf + 1u;
    char* bufEnd = buf + sizeof(buf) / sizeof(*buf);

    for (std::size_t i = 0; i < strideCount; ++i) {
        char* b = buf;
        for (unsigned int j = 0; j < Stride; ++j, b += 3u, ++p) {
            uint32_t n = *p;
            b[0] = static_cast<char>(' ');
            b[1] = static_cast<char>(charTable[n >> 4]);
            b[2] = static_cast<char>(charTable[n & 0x0F]);
        }
        strm.write(bufBeg, bufEnd - bufBeg);
        bufBeg = buf;
    }

    if (tailSize > 0) {
        char* b = buf;
        unsigned int i = 0;
        do {
            uint32_t n = *p;
            b[0] = static_cast<char>(' ');
            b[1] = static_cast<char>(charTable[n >> 4]);
            b[2] = static_cast<char>(charTable[n & 0x0F]);
            ++i;
            ++p;
            b += 3u;
        } while (i < tailSize);
        strm.write(bufBeg, b - bufBeg);
    }
}

std::ostream & EasyEvent::operator<<(std::ostream &sout, const Bin &bin) {
    if (sout.good()) {
        if (bin.getMaxSize() > 0 && bin.getMaxSize() < bin.getSize()) {
            dumpData(bin.getData(), bin.getMaxSize(), sout);
            sout << " and " << (bin.getSize() - bin.getMaxSize()) << " bytes more";
        } else {
            dumpData(bin.getData(), bin.getSize(), sout);
        }
    }
    return sout;
}