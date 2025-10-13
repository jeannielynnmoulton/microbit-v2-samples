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

/**
 * To separate out audio control from the runAudio loop.
 * TODO:  Needs to track where the music stops/starts.

 */
class AudioController
{
public:
    // Copied from AudioTest
    const ManagedString song = ManagedString("010232279000001440226608881023012800000000240000000000000000000000000000,000000440000000440044008880000012800000000240000000000000000000000000000,310232226070801440162408881023012800000100240000000000000000000000000000,310231623093602440093908880000012800000100240000000000000000000000000000");
    bool shouldBePlaying = false;

    void playaudio()
    {
        uBit.audio.soundExpressions.playAsync(song);
    }

    void stopaudio()
    {
        uBit.audio.soundExpressions.stop();
    }

};


AudioController audio_controller;

/**
 * Function to call in the fiber for audio
 */
void runAudio()
{
    while (true)
    {
        if (audio_controller.shouldBePlaying && !uBit.audio.isPlaying()) // checking isPlaying keeps from setting up endless async which we cannot interrupt
        {
            audio_controller.playaudio();
        }
        if (!audio_controller.shouldBePlaying)
        {
            audio_controller.stopaudio();
        }
        fiber_sleep(100);
    }

}

/**
 * Function to call in the fiber for reading the light level. We must continuously read the light level
 * for the light sense event to be triggered.
 */
void readLight()
{
    while(1)
    {
        uBit.display.readLightLevel();
        uBit.serial.send(uBit.display.getLastLightLevel());
        fiber_sleep(100);
    }
}

/**
 * Simulates a music box and demonstrates the new MICROBIT_DISPLAY_EVT_LIGHTSENSE_LIGHT
 * and MICROBIT_DISPLAY_EVT_LIGHTSENSE_DARK DEVICE_ID_LIGHT_SENSOR values in use
 * on an event listener.
 * TODO: Music should start where it stopped, until then, it's not quite like a music box.
 * TODO: Get some better music.
 */
void music_box()
{

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

    create_fiber(runAudio);
    create_fiber(readLight);
    release_fiber();
}