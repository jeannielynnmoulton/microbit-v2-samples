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

// Copied from AudioTest


class AudioController
{
public:
    const ManagedString song = ManagedString("010232279000001440226608881023012800000000240000000000000000000000000000,000000440000000440044008880000012800000000240000000000000000000000000000,310232226070801440162408881023012800000100240000000000000000000000000000,310231623093602440093908880000012800000100240000000000000000000000000000");
    bool shouldBePlaying = false;

    void runAudio()
    {
        if (shouldBePlaying && !uBit.audio.isPlaying()) // checking isPlaying keeps from setting up endless async which we cannot interrupt
        {
            playaudio();
        }
        if (!shouldBePlaying)
        {
            stopaudio();
        }
    }

private:
    void playaudio()
    {
        uBit.audio.soundExpressions.playAsync(song);
    }

    void stopaudio()
    {
        uBit.audio.soundExpressions.stop();
    }

};

// TODO:  This current does not track where the music started or stopped, so isn't totally like a music box
AudioController audio_controller;

void light_sensing_event_test()
{
    // very similar to AccelerometerTest.shake_test()
    // Only the first event seems to be working, isn't filtering by values, so I've implemented this
    // to extract the value from the event.

    uBit.messageBus.listen(DEVICE_ID_LIGHT_SENSOR, MICROBIT_DISPLAY_EVT_LIGHTSENSE_LIGHT, [](MicroBitEvent e) {
        if (e.value == MICROBIT_DISPLAY_EVT_LIGHTSENSE_LIGHT)
        {
            uBit.display.print("L");
            audio_controller.shouldBePlaying = true;
        } else {
            uBit.display.print("D");
            audio_controller.shouldBePlaying = false;
        }
    });

    while(1)
    {
        uBit.sleep(100);
        uBit.display.readLightLevel();
        audio_controller.runAudio();
        // debugging
        uBit.serial.send(uBit.display.getLastLightLevel());
    }
}