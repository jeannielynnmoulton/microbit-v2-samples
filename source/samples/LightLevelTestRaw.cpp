#include "MicroBitCompat.h"
#include "Tests.h"

void 
light_level_test_raw()
{
    int t;

    uBit.io.row1.getDigitalValue();
    uBit.io.row2.getDigitalValue();
    uBit.io.row3.getDigitalValue();
    uBit.io.row4.getDigitalValue();
    uBit.io.row5.getDigitalValue();

    uBit.io.col1.setDigitalValue(1);
    uBit.io.col2.setDigitalValue(1);
    uBit.io.col3.setDigitalValue(1);
    uBit.io.col4.setDigitalValue(1);
    uBit.io.col5.setDigitalValue(1);

    while(1)
    {
        t = 0;
        uBit.io.row1.setDigitalValue(0);
        while(t < 1000000 && uBit.io.row1.getDigitalValue(PullMode::None) == 0)
            t++;
        
        DMESG("DECAY: %d\n", t);

        uBit.sleep(500);
    }
}

void light_sensing_event_test()
{
    // very similar to AccelerometerTest.shake_test()
    uBit.messageBus.listen(DEVICE_ID_LIGHT_SENSOR, MICROBIT_DISPLAY_EVT_LIGHTSENSE_DARK, [](MicroBitEvent e) {
        uBit.display.print("D");
    });
    uBit.messageBus.listen(DEVICE_ID_LIGHT_SENSOR, MICROBIT_DISPLAY_EVT_LIGHTSENSE_LIGHT, [](MicroBitEvent e) {
        uBit.display.print("L");
    });


    while(1)
    {
        uBit.sleep(3000);
        uBit.serial.send(uBit.display.getLastLightLevel());
    }
}