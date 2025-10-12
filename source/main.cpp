#include "MicroBit.h"
#include "samples/Tests.h"

MicroBit uBit;

int main()
{
    uBit.init();
    light_sensing_event_test();
}

