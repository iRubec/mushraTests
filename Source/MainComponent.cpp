#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
    : keyboardComponent(keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard),
    audioSetupComp(deviceManager, 0, 24, 0,  24, false, false, false, true),
    thumbnailCache(5), thumbnail(512, formatManager, thumbnailCache)
    //startTime(juce::Time::getMillisecondCounterHiRes() * 0.001)
{
    // Make sure you set the size of the component after
    // you add any child components.
    setSize(1400, 600);

    alert->showMessageBox(juce::AlertWindow::AlertIconType::InfoIcon, "Hola!", "Vas a hacer un test mushra", "Adelante!");

    readJSON();
    
    int width = getWidth() - 400;
    float sliderWidth = (width - 20) / 10;

    for (int i = 0; i < 8; i++)
    {
        juce::Slider* s = new juce::Slider;
        s->setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
        s->setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxBelow, false, 60, 20);
        s->setBounds(40 + sliderWidth * (i+1), 200, sliderWidth, getHeight() - 280);
        s->setColour(0x1001400, juce::Colours::black);
        s->setColour(0x1001300, colours[i]);
        s->setRange(0, 100);
        s->setNumDecimalPlacesToDisplay(0);
        s->setEnabled(false);
        if (i <= stimulis) { s->setAlpha(0.6f); }
        else { s->setAlpha(0.4f); s->setColour(0x1001300, juce::Colours::grey); };
        arraySliders.add(s);
        addAndMakeVisible(s);

        juce::TextButton* newBut = new juce::TextButton;
        newBut->setButtonText(buttonText[i]);
        newBut->setName(buttonText[i]);
        newBut->addListener(this);
        newBut->setBounds(70 + sliderWidth * (i + 1), 150, sliderWidth - 60, 40);
        newBut->setColour(0x1000100, juce::Colours::black);
        if (i >= stimulis) { newBut->setAlpha(0.4f); newBut->setEnabled(false); };
        arrayButtons.add(newBut);
        addAndMakeVisible(newBut);

    };

    stop.setButtonText("STOP");
    stop.setName("stop");
    stop.addListener(this);
    addAndMakeVisible(stop);

    bRef.setButtonText("REF");
    bRef.setName("ref");
    bRef.addListener(this);
    addAndMakeVisible(bRef);

    nextButton.setButtonText("Siguiente >");
    nextButton.setName("next");
    nextButton.addListener(this);
    addAndMakeVisible(nextButton);

    // Reading the first group of audios
    formatManager.registerBasicFormats();
	juce::WavAudioFormat wavFormat;
    
    // Create the random array to load the files

    for (int i = 0; i < stimulis; i++) {
        auto randomInt = juce::Random::getSystemRandom().nextInt(6);
        int tmp = random[i];
        random[i] = random[randomInt];
        random[randomInt] = tmp;
    };

    // Reference audio
    juce::File file(juce::File::getCurrentWorkingDirectory().getChildFile("../../audios/g1/" + files[0][0]).getFullPathName());
    auto is = new juce::FileInputStream(file);

    juce::AudioSampleBuffer newBuffer;
    std::unique_ptr<juce::AudioFormatReader> reader(wavFormat.createReaderFor(is, true));
    newBuffer.setSize((int)reader->numChannels, (int)reader->lengthInSamples);
    reader->read(&newBuffer, 0, (int)reader->lengthInSamples, 0, true, true);

    buffersArray->add(newBuffer);
    duration = reader->lengthInSamples / reader->sampleRate;
    
    for (int i = 0; i < stimulis; i++) {
       
        juce::File file(juce::File::getCurrentWorkingDirectory().getChildFile("../../audios/g1/" + files[0][random[i]]).getFullPathName());
        auto is = new juce::FileInputStream(file);

        juce::AudioSampleBuffer newBuffer;
        std::unique_ptr<juce::AudioFormatReader> reader(wavFormat.createReaderFor(is, true));
		newBuffer.setSize((int)reader->numChannels, (int)reader->lengthInSamples);
		reader->read(&newBuffer, 0, (int)reader->lengthInSamples, 0, true, true);
		
		buffersArray->add(newBuffer);
	};
    
    //==============================================================================
    //================================= MIDI =======================================
    //==============================================================================
    
    // Miraremos si está conectado el MIDI Mix, y mandaremos la información al header
    // para que saque por patalla si está cnectado o no
    auto midiInputs = juce::MidiInput::getAvailableDevices();
    for (int i = 0; i < midiInputs.size(); i++)
    {
        auto newInput = midiInputs[i];

        if (newInput.name == "MIDI Mix")
        {
            deviceManager.setMidiInputDeviceEnabled(newInput.identifier, true);
            deviceManager.addMidiInputDeviceCallback(newInput.identifier, this);
            midiText = "MIDI connected";
            break;
        }

    }

    //==============================================================================
    //=========================== TARJETA DE SONIDO ================================
    //==============================================================================
    addAndMakeVisible(audioSetupComp);

    auto* device = deviceManager.getCurrentDeviceTypeObject();
    //deviceManager.initialise(0, 2, nullptr, true, {}, nullptr);
    //deviceManager.initialiseWithDefaultDevices(0, 24);
    //deviceManager.setAudioDeviceSetup()

    //juce::AudioDeviceManager* dev = deviceManager.getCurrentDeviceTypeObject()->getTypeName();
    
    /*
    // Leemos la tarjeta da sonido que se está usando y se la pasamos al header
    // para que la saque por pantalla
    // deviceManager.getAudioDeviceSetup();
    auto* device = deviceManager.getCurrentAudioDevice();
    juce::String deviceName = device->getName();
    auto activeOutputChannels = device->getActiveOutputChannels();
    int avaiableOutputs = activeOutputChannels.countNumberOfSetBits();

    // Le dcimos que use los drivers ASIO para la salida de audio
    if (deviceName == "MOTU")
        deviceManager.setCurrentAudioDeviceType("ASIO", true);
    */
    //audioSetupComp.setBounds(0, 0, dW, dH);
    
    // Some platforms require permissions to open input channels so request that here
    if (juce::RuntimePermissions::isRequired (juce::RuntimePermissions::recordAudio)
        && ! juce::RuntimePermissions::isGranted (juce::RuntimePermissions::recordAudio))
    {
        juce::RuntimePermissions::request (juce::RuntimePermissions::recordAudio,
                                           [&] (bool granted) { setAudioChannels (granted ? 2 : 0, 24); });
    }
    else
    {
        // Specify the number of input and output channels that we want to open
        setAudioChannels (0, 24);
    }
}

MainComponent::~MainComponent()
{
    // This shuts down the audio device and clears the audio source.
    shutdownAudio();
}

//==============================================================================
void MainComponent::prepareToPlay (int samplesPerBlockExpected, double newSampleRate)
{
    // This function will be called when the audio device is started, or when
    // its settings (i.e. sample rate, block size, etc) are changed.

    // You can use this function to initialise any resources you might need,
    // but be careful - it will be called on the audio thread, not the GUI thread.

    // For more details, see the help for AudioProcessor::prepareToPlay()
    sampleRate = newSampleRate;
    expectedSamplesPerBlock = samplesPerBlockExpected;

}

void MainComponent::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{
    // Your audio-processing code goes here!

    // For more details, see the help for AudioProcessor::getNextAudioBlock()

    // Right now we are not producing any data, in which case we need to clear the buffer
    // (to prevent the output of random noise)

    bufferToFill.clearActiveBufferRegion();

    if (isPlaying)
    {
        juce::AudioSampleBuffer stimuli = buffersArray->getUnchecked(buffer);
        auto outputSamplesOffset = bufferToFill.startSample;
        auto outputSamplesRemaining = bufferToFill.numSamples;
        auto bufferSamplesRemaining = stimuli.getNumSamples() - position;
        auto samplesThisTime = juce::jmin(outputSamplesRemaining, bufferSamplesRemaining);

        for (int ch = 0; ch < 24; ++ch)
            bufferToFill.buffer->addFrom(ch, outputSamplesOffset, stimuli, ch, position, samplesThisTime, 0.5);

        outputSamplesRemaining -= samplesThisTime;
        outputSamplesOffset += samplesThisTime;
        position += samplesThisTime;

        if (position >= stimuli.getNumSamples())
            position = 0;
        
    };
    
}

void MainComponent::releaseResources()
{
    // This will be called when the audio device stops, or when it is being
    // restarted due to a setting change.

    // For more details, see the help for AudioProcessor::releaseResources()

}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (juce::Colours::lightgrey);
    g.setColour(juce::Colours::grey);
    g.fillRect(1000, 0, 400, getHeight());
    
    audioSetupComp.setBounds(1000, 0, 400, getHeight());

    // You can add your drawing code here!
    float w = 1000;
    float h = getHeight();
    float width = (w - 20) / 10;

    juce::Rectangle<int> thumbnailBounds(150, 100, w - 250, 30);
    g.setColour(juce::Colours::white);
    g.fillRect(thumbnailBounds);
    g.setColour(juce::Colours::aqua);
    thumbnail.drawChannel(g, thumbnailBounds, 0.0, thumbnail.getTotalLength(), 2, 1.0f);
    //thumbnail.drawChannels(g, thumbnailBounds, 0.0, thumbnail.getTotalLength(), 1.0f);
    g.setColour(juce::Colours::black);
    g.drawLine(150 + posPlayer, 100, 150 + posPlayer, 130, 1.0f);
    
    g.setColour(juce::Colours::black);
    g.setFont(18.0f);
    g.drawText(testText, 10, 10, 60, 20, juce::Justification::left, false);
    g.setFont(10.0f);
    g.drawText(midiText, w - 150, 5, 140, 20, juce::Justification::right, false);
    
    g.setFont(18.0f);
    for (int i = 0; i < 6; i++)
    {
        g.drawText(juce::String(100 - i*20), 20, 205 + 56 * i, 40, 10, juce::Justification::right, false);
        if (i < 5) { g.drawText(perceptions[i], 70, 210 + 56 * i, width - 20, 56, juce::Justification::centred, false); }
        g.drawLine(70 , 210 + 56 * i, w - 80, 210 + 56 * i);
    }

    stop.setBounds(1030 - width, 100, 60, 30);
    bRef.setBounds(1030 - width, 145, width - 40, 50);
    nextButton.setBounds(w - 110, h - 60, 100, 50);
}

void MainComponent::resized()
{
    // This is called when the MainContentComponent is resized.
    // If you add any child components, this is where you should
    // update their positions.
    
}

void MainComponent::buttonClicked(juce::Button* button)
{
  
    juce::String name = button->getName();

    if (name == "stop") { isPlaying = false; position = 0; posPlayer = 0; stopTimer(); }
    else { isPlaying = true; startTimer(20); };

    if (name == "next")
    {
        stopTimer();
        isPlaying = false;
        handleTests();
        testNumber = testNumber + 1;
        testText = "Test " + std::to_string(testNumber);
        for (auto s : arraySliders) { s->setValue(0); }
        repaint();
    }
    else if (name == "ref"){ buffer = 0; }
    else if (name == "A") { buffer = 1; }
    else if (name == "B") { buffer = 2; }
    else if (name == "C"){ buffer = 3; }
    else if (name == "D"){ buffer = 4; }
    else if (name == "E"){ buffer = 5;}
    else if (name == "F"){ buffer = 6; }
    else if (name == "G") { buffer = 7; }
    else if (name == "H") { buffer = 8; }

    for (int i = 0; i < stimulis; i++)
    {
        arraySliders[i]->setEnabled(false);
        arraySliders[i]->setAlpha(0.5f);
        arrayButtons[i]->setColour(0x1000100, juce::Colours::black);
        bRef.setColour(0x1000100, juce::Colours::black);
    }

    if (buffer != 0 && isPlaying)
    {
        arraySliders[buffer - 1]->setEnabled(true);
        arraySliders[buffer - 1]->setAlpha(1.0f);
        arrayButtons[buffer - 1]->setColour(0x1000100, juce::Colours::lightgreen);
        bRef.setColour(0x1000100, juce::Colours::black);
    }
    
    if(buffer == 0 && isPlaying)
    {
        bRef.setColour(0x1000100, juce::Colours::lightgreen);
    }

};

void MainComponent::handleTests()
{
    isPlaying = false;
    dataExport();
    test = test + 1;

    // Randomization
    for (int i = 0; i < stimulis; i++) {
        auto randomInt = juce::Random::getSystemRandom().nextInt(6);
        int tmp = random[i];
        random[i] = random[randomInt];
        random[randomInt] = tmp;
    };

    buffersArray->clear();
    juce::WavAudioFormat wavFormat;

    juce::File file(juce::File::getCurrentWorkingDirectory().getChildFile("../../audios/g" + juce::String(test + 1) + "/" + files[test][0]).getFullPathName());
    auto is = new juce::FileInputStream(file);

    juce::AudioSampleBuffer newBuffer;
    std::unique_ptr<juce::AudioFormatReader> reader(wavFormat.createReaderFor(is, true));
    newBuffer.setSize((int)reader->numChannels, (int)reader->lengthInSamples);
    reader->read(&newBuffer, 0, (int)reader->lengthInSamples, 0, true, true);

    buffersArray->add(newBuffer);
    duration = reader->lengthInSamples / reader->sampleRate;

    for (int i = 0; i < stimulis; i++) {

        juce::File file(juce::File::getCurrentWorkingDirectory().getChildFile("../../audios/g" + juce::String(test+1) + "/" + files[test][random[i]]).getFullPathName());
        auto is = new juce::FileInputStream(file);

        juce::AudioSampleBuffer newBuffer;
        std::unique_ptr<juce::AudioFormatReader> reader(wavFormat.createReaderFor(is, true));
        newBuffer.setSize((int)reader->numChannels, (int)reader->lengthInSamples);
        reader->read(&newBuffer, 0, (int)reader->lengthInSamples, 0, true, true);

        buffersArray->add(newBuffer);
    };

}

void MainComponent::dataExport()
{
    //XmlElement* testElement = new XmlElement(testString);
    juce::XmlElement* testElement = new juce::XmlElement("TEST");

    juce::XmlElement* a = new juce::XmlElement("A");
    juce::XmlElement* aTech = new juce::XmlElement("tech");
    juce::XmlElement* aAns = new juce::XmlElement("ans");
    aTech->addTextElement(names[random[0]]);
    aAns->addTextElement(std::to_string(arraySliders[0]->getValue()));
    a->addChildElement(aTech);
    a->addChildElement(aAns);
    testElement->addChildElement(a);
    
    if (stimulis >= 2) {
        juce::XmlElement* b = new juce::XmlElement("B");
        juce::XmlElement* bTech = new juce::XmlElement("tech");
        juce::XmlElement* bAns = new juce::XmlElement("ans");
        bTech->addTextElement(names[random[1]]);
        bAns->addTextElement(std::to_string(arraySliders[1]->getValue()));
        b->addChildElement(bTech);
        b->addChildElement(bAns);
        testElement->addChildElement(b);
    }
    if (stimulis >= 3) {
        juce::XmlElement* c = new juce::XmlElement("C");
        juce::XmlElement* cTech = new juce::XmlElement("tech");
        juce::XmlElement* cAns = new juce::XmlElement("ans");
        cTech->addTextElement(names[random[2]]);
        cAns->addTextElement(std::to_string(arraySliders[2]->getValue()));
        c->addChildElement(cTech);
        c->addChildElement(cAns);
        testElement->addChildElement(c);
    }
    if (stimulis >= 4) {
        juce::XmlElement* d = new juce::XmlElement("D");
        juce::XmlElement* dTech = new juce::XmlElement("tech");
        juce::XmlElement* dAns = new juce::XmlElement("ans");
        dTech->addTextElement(names[random[3]]);
        dAns->addTextElement(std::to_string(arraySliders[3]->getValue()));
        d->addChildElement(dTech);
        d->addChildElement(dAns);
        testElement->addChildElement(d);
    }
    if (stimulis >= 5) {
        juce::XmlElement* e = new juce::XmlElement("E");
        juce::XmlElement* eTech = new juce::XmlElement("tech");
        juce::XmlElement* eAns = new juce::XmlElement("ans");
        eTech->addTextElement(names[random[4]]);
        eAns->addTextElement(std::to_string(arraySliders[4]->getValue()));
        e->addChildElement(eTech);
        e->addChildElement(eAns);
        testElement->addChildElement(e);
    }
    if (stimulis >= 6) {
        juce::XmlElement* f = new juce::XmlElement("F");
        juce::XmlElement* fTech = new juce::XmlElement("tech");
        juce::XmlElement* fAns = new juce::XmlElement("ans");
        fTech->addTextElement(names[random[5]]);
        fAns->addTextElement(std::to_string(arraySliders[5]->getValue()));
        f->addChildElement(fTech);
        f->addChildElement(fAns);
        testElement->addChildElement(f);
    }
    if (stimulis >= 7) {
        juce::XmlElement* g = new juce::XmlElement("G");
        juce::XmlElement* gTech = new juce::XmlElement("tech");
        juce::XmlElement* gAns = new juce::XmlElement("ans");
        gTech->addTextElement(names[random[6]]);
        gAns->addTextElement(std::to_string(arraySliders[6]->getValue()));
        g->addChildElement(gTech);
        g->addChildElement(gAns);
        testElement->addChildElement(g);
    }
    if (stimulis >= 8) {
        juce::XmlElement* h = new juce::XmlElement("H");
        juce::XmlElement* hTech = new juce::XmlElement("tech");
        juce::XmlElement* hAns = new juce::XmlElement("ans");
        hTech->addTextElement(names[random[7]]);
        hAns->addTextElement(std::to_string(arraySliders[7]->getValue()));
        h->addChildElement(hTech);
        h->addChildElement(hAns);
        testElement->addChildElement(h);
    };

    testData->addChildElement(testElement);

    if (testNumber == groups)
    {
        auto xmlString = testData->toString();

        juce::FileChooser folderChooser("Save test", {}, "*.xml");
        folderChooser.browseForFileToSave(true);
        juce::File newFile(folderChooser.getResult());
        newFile.appendText(xmlString);

        alert->showMessageBox(juce::AlertWindow::AlertIconType::InfoIcon, "MUCHAS GRACIAS!", "", "");

        juce::JUCEApplicationBase::quit();
    };
}

void MainComponent::timerCallback()
{
    posPlayer = posPlayer + 750/(duration/0.02);
    if (posPlayer >= 750.0) { posPlayer = 0.0; };
    repaint();
}

void MainComponent::readJSON()
{
    juce::File file(juce::File::getCurrentWorkingDirectory().getChildFile("../../filenames.json").getFullPathName());
    juce::var parsedJson = juce::JSON::parse(file);
    juce::String data = juce::JSON::toString(parsedJson);

    juce::var jsonToParse;
    if (juce::JSON::parse(data, jsonToParse).ok()) {

        path = parsedJson["Path"];
        groups = parsedJson["Groups"];
        stimulis = parsedJson["StimuliPerGroup"];

        files[0][0] = parsedJson["FileNames"]["g1"]["ref"];
        files[0][1] = parsedJson["FileNames"]["g1"]["anchor"];
        files[0][2] = parsedJson["FileNames"]["g1"]["ambi1"];
        files[0][3] = parsedJson["FileNames"]["g1"]["ambi3"];
        files[0][4] = parsedJson["FileNames"]["g1"]["swf0"];
        files[0][5] = parsedJson["FileNames"]["g1"]["swf1"];

        if (groups >= 2) {
            files[1][0] = parsedJson["FileNames"]["g2"]["ref"];
            files[1][1] = parsedJson["FileNames"]["g2"]["anchor"];
            files[1][2] = parsedJson["FileNames"]["g2"]["ambi1"];
            files[1][3] = parsedJson["FileNames"]["g2"]["ambi3"];
            files[1][4] = parsedJson["FileNames"]["g2"]["swf0"];
            files[1][5] = parsedJson["FileNames"]["g2"]["swf1"];
        }
        if (groups >= 3) {
            files[2][0] = parsedJson["FileNames"]["g3"]["ref"];
            files[2][1] = parsedJson["FileNames"]["g3"]["anchor"];
            files[2][2] = parsedJson["FileNames"]["g3"]["ambi1"];
            files[2][3] = parsedJson["FileNames"]["g3"]["ambi3"];
            files[2][4] = parsedJson["FileNames"]["g3"]["swf0"];
            files[2][5] = parsedJson["FileNames"]["g3"]["swf1"];
        }
        if (groups >= 4) {
            files[3][0] = parsedJson["FileNames"]["g4"]["ref"];
            files[3][1] = parsedJson["FileNames"]["g4"]["anchor"];
            files[3][2] = parsedJson["FileNames"]["g4"]["ambi1"];
            files[3][3] = parsedJson["FileNames"]["g4"]["ambi3"];
            files[3][4] = parsedJson["FileNames"]["g4"]["swf0"];
            files[3][5] = parsedJson["FileNames"]["g4"]["swf1"];
        }
        if (groups >= 5) {
            files[4][0] = parsedJson["FileNames"]["g5"]["ref"];
            files[4][1] = parsedJson["FileNames"]["g5"]["anchor"];
            files[4][2] = parsedJson["FileNames"]["g5"]["ambi1"];
            files[4][3] = parsedJson["FileNames"]["g5"]["ambi3"];
            files[4][4] = parsedJson["FileNames"]["g5"]["swf0"];
            files[4][5] = parsedJson["FileNames"]["g5"]["swf1"];
        }
        if (groups >= 6) {
            files[5][0] = parsedJson["FileNames"]["g6"]["ref"];
            files[5][1] = parsedJson["FileNames"]["g6"]["anchor"];
            files[5][2] = parsedJson["FileNames"]["g6"]["ambi1"];
            files[5][3] = parsedJson["FileNames"]["g6"]["ambi3"];
            files[5][4] = parsedJson["FileNames"]["g6"]["swf0"];
            files[5][5] = parsedJson["FileNames"]["g6"]["swf1"];
        }
        if (groups >= 7) {
            files[6][0] = parsedJson["FileNames"]["g7"]["ref"];
            files[6][1] = parsedJson["FileNames"]["g7"]["anchor"];
            files[6][2] = parsedJson["FileNames"]["g7"]["ambi1"];
            files[6][3] = parsedJson["FileNames"]["g7"]["ambi3"];
            files[6][4] = parsedJson["FileNames"]["g7"]["swf0"];
            files[6][5] = parsedJson["FileNames"]["g7"]["swf1"];
        }
        if (groups >= 8) {
            files[7][0] = parsedJson["FileNames"]["g8"]["ref"];
            files[7][1] = parsedJson["FileNames"]["g8"]["anchor"];
            files[7][2] = parsedJson["FileNames"]["g8"]["ambi1"];
            files[7][3] = parsedJson["FileNames"]["g8"]["ambi3"];
            files[7][4] = parsedJson["FileNames"]["g8"]["swf0"];
            files[7][5] = parsedJson["FileNames"]["g8"]["swf1"];
        };
    };
}

/*
juce::Slider createSlider(juce::String name, juce::Rectangle<int> pos)
{
    juce::Slider newSlider;
    newSlider.setBounds(pos);
    newSlider.setSliderStyle(juce::Slider::SliderStyle::LinearBarVertical);
    newSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxBelow, false, 80, 20);
    newSlider.setColour(0x1001400, juce::Colours::black);
    newSlider.setRange(0, 100);
    newSlider.setNumDecimalPlacesToDisplay(0);

    return newSlider;

}
*/
