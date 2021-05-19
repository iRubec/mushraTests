#pragma once

#include <JuceHeader.h>
//using namespace std;
//using namespace juce;

#include <locale>
#include <codecvt>

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
    void timerCallback() override;
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
        if (arraySliders.getUnchecked(sliderPositions[noteNumber])->isEnabled())
        {
            arraySliders.getUnchecked(sliderPositions[noteNumber])->setValue(round(velocity * div));
            circleWidth = round(velocity * 1.0236);
            repaint();
        };
    };

    void midiButton(float noteNumber)
    {
        if(noteNumber <= 24)
            arrayButtons[buttonPositions[noteNumber]]->triggerClick();
        else if (noteNumber == 27)
            refBut.triggerClick();
        else if (noteNumber == 26)
            stopBut.triggerClick();
        else if (noteNumber == 25)
            nextBut.triggerClick();
    };
    
    //==============================================================================
    void handleTests();
    void dataExport();
    void readJSON();

private:
    //==============================================================================
    // Your private member variables go here...
    juce::AlertWindow* alert;
    juce::TextButton refBut, anchorBut, prevBut, nextBut, stopBut;
    juce::ImageButton menuBut;
    int avaiableOutputs = 0, marginButtons = 20, channels = 2;
    juce::TextEditor impresions;
    int textForWidth = 0, circleWidth = 0.0;
    bool showMenu = true, paintCircles = false;

    juce::String midiText = "MIDI not found";
    juce::String testText = "Test 1";
    int testNumber = 0, stimuli = 0, groups = 0, widthTest = 0;
    float duration, posPlayer = 0.0;
    juce::String path;
    juce::Array<juce::Slider*> arraySliders;
    juce::Array<juce::TextButton*> arrayButtons;
   
    // Textos!
    std::string perceptions[5] = {"Excelente", "Bueno", "Igual", "Pobre", "Malo"};
    std::string perceptionsWidth[5] = { "Big", "", "", "", "Ref"};
    std::string buttonText[8] = { "1", "2", "3", "4", "5", "6", "7", "8" };
    std::string names[8] = { "ref", "anch", "s1", "s2", "s3", "s4", "s5", "s6" };
    std::string descriptionText = "Esta es la primera linea de la descripcion del test.\n\
Ahora va la segunda, que es lo que ha pedido Ricardo y yo soy un mandado.\n\
 1. Excelente: \n\
 2. Bueno: \n\
 3. Igual: \n\
 4. Peor: \n\
 5. Terrible: \n\
Finalmente tenemos una linea que es la ultima. A ver si le gusta al jefe, que seguro que alguna pega le saca";
    std::string descriptionTextWidth = "Descripcion del test para la anchura";
    std::string welcome = "Hola!";
    std::string welcomeText = "Vas a hacer un test mushra";
    std::string goodBye = "MUCHAS GRACIAS!";
    int random[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };

    juce::Image upnaImage, upfImage, jaulabImage;
 
    // XML element to create the output file
    juce::XmlElement* testData = new juce::XmlElement("testData");
    
    // String matrix to save the files paths 
    juce::String files[8][8];

    // Audio components
    juce::AudioDeviceSelectorComponent audioSetupComp;
    juce::AudioFormatManager formatManager;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
	
    juce::AudioSampleBuffer buffersArray[10];
    juce::MemoryInputStream* wavs[8];

    // MIDI components and dictionaries
    juce::MidiKeyboardState keyboardState;
    juce::MidiKeyboardComponent keyboardComponent;
    bool isAddingFromMidiInput = false;
    std::map<int, int> sliderPositions = { {19, 0,}, {23, 1,}, {27, 2,}, {31, 3,}, {49, 4,}, {53, 5,}, {57, 6,}, {61, 7,} };
    std::map<int, int> buttonPositions = { {1, 0,}, {4, 1,}, {7, 2,}, {10, 3,}, {13, 4,}, {16, 5,}, {19, 6,}, {22, 7,},
                                         {3, 0,}, {6, 1,}, {9, 2,}, {12, 3,}, {15, 4,}, {18, 5,}, {21, 6,}, {24, 7,} };

    // Playback variables
    double sampleRate = 0.0;
    int expectedSamplesPerBlock = 0;
    int position = 0;
    int buffer = 0;
    bool isPlaying = false, ch = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
