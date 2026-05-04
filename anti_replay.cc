#include "anti_replay.h"

int CheckAntiReplay(uint8_t droneId, uint32_t seq)
{
    AntiReplayWindow& window = g_antiReplay[droneId];

    if (seq > window.maxSeq)
    {
        uint32_t shift = seq - window.maxSeq;

        if (shift >= 64)
            window.bitmap = 0;
        else
            window.bitmap <<= shift;

        window.bitmap |= 1ULL;
        window.maxSeq = seq;

        return 0;
    }

    uint32_t diff = window.maxSeq - seq;

    if (diff >= 64)
        return 2;

    if (window.bitmap & (1ULL << diff))
        return 1;

    window.bitmap |= (1ULL << diff);

    return 0;
}
