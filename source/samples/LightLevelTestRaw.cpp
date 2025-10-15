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

// song encoding, this is largely taken from speaker_test2()
// but I asked ChatGPT to generate periods for "Ode to Joy"
static constexpr int odeToJoyNoteLength = 500;
static constexpr int odeToJoy[16] = {
    3030, 3030, 2860, 2550,
    2550, 2860, 3030, 3400,
    3820, 3820, 3400, 3030,
    3030, 3400, 3400, 3400
};

/**
 * To separate out audio control from the runAudio loop.
 */
class AudioController
{
public:
    volatile bool shouldBePlaying = false; // controlled by the event

private:
    // control for which note to play when the music box closes
    int lastIndexPlayed = 0; // track current last index
    int resumeIndex = 0; // track last index played on when music stops, i.e., where to resume from
    static constexpr int timeBetweenNotes = 50;
    static constexpr int songLength = 16;

public:
    /**
     * Plays a 16 note song encoded as an array of periods (microseconds) of the tones.
     * All tones are played with the same tone length.
     */
    void playaudio(const int (&song)[songLength], const int &toneLength)
    {
        // again, this is largely taken from speaker_test2()
        uBit.io.runmic.setDigitalValue(0);
        for (int i=0; i < songLength; i++)
        {
            if (shouldBePlaying)
            {
                // debugging
                // uBit.serial.send(" playing index: ");
                // uBit.serial.send((i+resumeIndex)%songLength);

                // save last index
                lastIndexPlayed = (i+resumeIndex)%songLength;

                // tone
                uBit.io.speaker.setAnalogValue(512);
                uBit.io.speaker.setAnalogPeriodUs(song[lastIndexPlayed]);
                uBit.sleep(toneLength);

                // small gap between tones
                uBit.io.speaker.setAnalogValue(0);
                uBit.sleep(timeBetweenNotes);

                // increment last index
                lastIndexPlayed++;

            } else
            {
                break;
            }
        }
    }

    void stopaudio()
    {
        uBit.io.speaker.setAnalogValue(0); // no tone
        resumeIndex = lastIndexPlayed%songLength; // sets the index to start playing on again to next one

        // debugging
        // uBit.serial.send(" saving index: ");
        // uBit.serial.send(resumeIndex);
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
        if (audio_controller.shouldBePlaying)
        {
            audio_controller.playaudio(odeToJoy, odeToJoyNoteLength);
        }
        if (!audio_controller.shouldBePlaying)
        {
            audio_controller.stopaudio();
        }
        fiber_sleep(10);
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
        fiber_sleep(100);
    }
}

/**
 * Simulates a music box and demonstrates the new MICROBIT_DISPLAY_EVT_LIGHTSENSE_LIGHT
 * and MICROBIT_DISPLAY_EVT_LIGHTSENSE_DARK DEVICE_ID_LIGHT_SENSOR values in use
 * on an event listener.
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