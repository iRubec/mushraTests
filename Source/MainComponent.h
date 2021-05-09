#pragma once

#include <JuceHeader.h>
//using namespace std;
//using namespace juce;

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent : public juce::AudioAppComponent,
                      public juce::Button::Listener,
                      private juce::MidiKeyboardStateListener,
                      private juce::MidiInputCallback,
                      private juce::Timer
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent() override;

    //==============================================================================
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    //==============================================================================
    void paint(juce::Graphics& g) override;
    void resized() override;

    //==============================================================================
    void buttonClicked(juce::Button* button) override;
    void timerCallback();
    //==============================================================================
    
    void handleIncomingMidiMessage(juce::MidiInput* source, const juce::MidiMessage& message) override
    {
        const juce::ScopedValueSetter<bool> scopedInputFlag(isAddingFromMidiInput, true);
        const juce::MessageManagerLock mmLock;
        if (message.isController())
        {
            midiMove(message.getNoteNumber(), message.getControllerValue());
        }
        else if (message.isNoteOn())
        {
            midiButton(message.getNoteNumber());
        };

    };

    void handleNoteOn(juce::MidiKeyboardState*, int midiChannel, int midiNoteNumber, float velocity) override {};
    
    void handleNoteOff(juce::MidiKeyboardState*, int midiChannel, int midiNoteNumber, float /*velocity*/) override {};

    void midiMove(float noteNumber, int velocity)
    {
        //float div = 100.0 / 128.0;
        //float div = 0.78125;
        float div = 0.787402;
        if(arraySliders.getUnchecked(sliderPositions[noteNumber])->isEnabled())
            arraySliders.getUnchecked(sliderPositions[noteNumber])->setValue(round(velocity * div));
    };

    void midiButton(float noteNumber)
    {
        if (noteNumber / 3 - 1 < stimulis)
            arrayButtons[noteNumber / 3 - 1]->triggerClick();
        else if (noteNumber == 27)
            bRef.triggerClick();
        else if (noteNumber == 26)
            stop.triggerClick();
        else if (noteNumber == 25)
            nextButton.triggerClick();
    };
    
    //==============================================================================
    void handleTests();
    void dataExport();
    void readJSON();

    /*
    void audioDeviceAboutToStart(juce::AudioIODevice* device) override {};
    void audioDeviceIOCallback(const float** /*inputChannelData, int /*numInputChannels,
        float** outputChannelData, int numOutputChannels,
        int numSamples) override {};
    */

private:
    //==============================================================================
    // Your private member variables go here...
    juce::AlertWindow* alert;
    juce::TextButton bRef, nextButton, stop;
    int test = 0;

    juce::String midiText = "MIDI not found";
    juce::String testText = "Test 1";
    int testNumber = 1, stimulis = 0, groups = 0;
    float duration, posPlayer = 0.0;
    juce::String path;
    juce::Array<juce::Slider*> arraySliders;
    juce::Array<juce::TextButton*> arrayButtons;
    std::map<int, int> sliderPositions = { {19, 0,}, {23, 1,}, {27, 2,}, {31, 3,}, {49, 4,}, {53, 5,}, {57, 6,}, {61, 7,} };

    std::string perceptions[5] = {"Excelente", "Bueno", "Igual", "Pobre", "Malo"};
    std::string buttonText[8] = { "1", "2", "3", "4", "5", "6", "7", "8" };
    std::string names[8] = { "ref", "anch", "s1", "s2", "s3", "s4", "s5", "s6" };
    int random[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };
    juce::Array<juce::Colour> colours{ juce::Colours::red, juce::Colours::blue, juce::Colours::blueviolet, juce::Colours::aquamarine,
                                       juce::Colours::azure, juce::Colours::blanchedalmond, juce::Colours::indigo, juce::Colours::gold};

    juce::XmlElement* testData = new juce::XmlElement("testData");
    juce::String files[8][6];

    juce::AudioDeviceSelectorComponent audioSetupComp;
    juce::AudioFormatManager formatManager;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
	
    juce::AudioSampleBuffer buffersArray[7];
    juce::MemoryInputStream* wavs[8];

    juce::MidiKeyboardState keyboardState;
    juce::MidiKeyboardComponent keyboardComponent;
    bool isAddingFromMidiInput = false;

    // Waveform
    juce::AudioThumbnailCache thumbnailCache;
    juce::AudioThumbnail thumbnail;

    double sampleRate = 0.0;
    int expectedSamplesPerBlock = 0;
    int position = 0;
    int buffer = 0;
    bool isPlaying = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
