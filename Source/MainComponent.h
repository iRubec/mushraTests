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
                      public juce::Slider::Listener,
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
    void sliderValueChanged(juce::Slider* slider) override;
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
            //circleWidth = round(velocity * 1.0236);
            circleWidth = round(velocity * 0.976923);
        };
    };

    void midiButton(float noteNumber)
    {
        if(noteNumber <= 21 || noteNumber == 24)
            arrayButtons[buttonPositions[noteNumber]]->triggerClick();
        else if (noteNumber == 27)
            refBut.triggerClick();
        else if (noteNumber == 26)
            stopBut.triggerClick();
        else if (noteNumber == 25)
            nextBut.triggerClick();
        else if (noteNumber == 22 && testNumber + 1 >= widthTest)
            anchorBut.triggerClick();
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
    bool canPass[8] = { false };

    juce::String midiText = "MIDI not found";
    juce::String testText = "Test 1";
    int testNumber = 0, stimuli = 0, groups = 0, widthTest = 0;
    float duration, posPlayer = 0.0;
    juce::String path;
    juce::Array<juce::Slider*> arraySliders;
    juce::Array<juce::TextButton*> arrayButtons;
   
    // Textos!
    std::string perceptions[5] = {"Excelente", "Bueno", "Igual", "Pobre", "Malo"};
    std::string perceptionsWidth[5] = { "Extra Large", "Large", "Medium", "Small", "Extra Small"};
    std::string buttonText[8] = { "1", "2", "3", "4", "5", "6", "7", "8" };
    std::string names[8] = { "ref", "anch", "s1", "s2", "s3", "s4", "s5", "s6" };
    int random[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };
    
    std::string descriptionText = "Califica, siguiendo las siguientes pautas, los diferentes estimulos en una escala de 0 a 100 comparandolos con el sonido de referencia (REF).\n\n\
 81-100 Excellent: no percibo diferencia alguna en su posicion\n\
 61-80 Good: ligero cambio en la posicion del estimulos\n\
 41-60 Fair: el estimulo se desvia de su posicion de referencia\n\
 21-40 Poor: el estimulo sufre un desvio sustancial y/o hay dificultad para localizarlo\n\
 0-20 Bad: el estimulo esta completamente fuera de su posicion de referencia y/o imposibilidad de localizarlo\n\n\
Puedes alternar la escucha entre los estimulos y la referencia tantas veces como veas necesario. Pulsando STOP pararas la reproduccion y con > pasaras al siguiente test";
    
    std::string descriptionTextWidth = "Califica la anchura de los diferentes estimulos en una escala de 0 a 100, siendo XS el estimulo menos ancho (0) y XL el mas ancho posible (100).\n\n\
En este caso, XS y XL son las referencias a seguir \n\n\
Puedes alternar la escucha entre los estimulos y las referencias tantas veces como veas necesario. Pulsando STOP pararas la reproduccion y con > pasaras al siguiente test";
    
    std::string welcome = "Hola!";
    std::string welcomeText = "Vas a hacer un test mushra";
    std::string secondTestText = "Comienzo del test de anchura de fuente. Ahora, tendras que fijarte en la anchura que tenga cada estimulo";
    std::string goodBye = "MUCHAS GRACIAS!";

    juce::Image upnaImage, upfImage, jaulabImage;
 
    // XML element to create the output file
    juce::XmlElement* testData = new juce::XmlElement("testData");
    
    // String matrix to save the files paths 
    juce::String files[15][8];

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
