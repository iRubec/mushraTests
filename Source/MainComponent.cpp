#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
    : keyboardComponent(keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard),
    audioSetupComp(deviceManager, 0, 24, 0,  24, false, false, false, true),
    thumbnailCache(5), thumbnail(512, formatManager, thumbnailCache)
{
    // Taking the size of the screen
    juce::Rectangle<int> r = juce::Desktop::getInstance().getDisplays().getMainDisplay().userArea;
    //juce::Rectangle<int> r(0, 0, 1280, 1024);
    
    int marginY = 100, marginX = 0;
    if (r.getHeight() >= 980) {
        marginY = 250; marginButtons = 25; marginX = 300;
    };
    
    // Setting the size of the window
    centreWithSize(r.getWidth() - 500 + marginX, r.getHeight()- marginY);

    // Alert window with the welcome message
    alert->showMessageBox(juce::AlertWindow::AlertIconType::InfoIcon, welcome, welcomeText, "Adelante!");

    // Reading the info of the test to do
    readJSON();
  
    //==============================================================================
    //=========================== SLIDERS AND BUTTONS ==============================
    //==============================================================================
    // Variables to draw set the sliders and buttons size
    int width = getWidth();
    float sliderWidth = (width - 20) / 10;

    // We will create 8 sliders with their buttons in the top. Only the number of stimuli will be enabled and more coloured
    for (int i = 0; i < 8; i++)
    {
        // Slider
        juce::Slider* s = new juce::Slider;
        s->setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
        s->setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxBelow, false, 60, 20);
        s->setBounds(40 + sliderWidth * (i+1), 300, sliderWidth, 322);
        s->setColour(0x1001400, juce::Colours::black);
        s->setColour(0x1001300, juce::Colours::grey);
        s->setRange(0, 100);
        s->setNumDecimalPlacesToDisplay(0);
        s->setEnabled(false);
        if (i < stimuli) { s->setAlpha(0.6f); }
        else { s->setAlpha(0.3f); };
        arraySliders.add(s);
        addAndMakeVisible(s);

        // Button
        juce::TextButton* newBut = new juce::TextButton;
        newBut->setButtonText(buttonText[i]);
        newBut->setName(buttonText[i]);
        newBut->addListener(this);
        newBut->setBounds(70 + sliderWidth * (i + 1) - 10, 250, sliderWidth - 40, 40);
        newBut->setColour(0x1000100, juce::Colours::black);
        if (i >= stimuli) { newBut->setAlpha(0.4f); newBut->setEnabled(false); };
        arrayButtons.add(newBut);
        addAndMakeVisible(newBut);

    };

    // Stop button
    stopBut.setButtonText("STOP");
    stopBut.setName("stop");
    stopBut.addListener(this);
    addAndMakeVisible(stopBut);

    // Ref button
    refBut.setButtonText("REF");
    refBut.setName("ref");
    refBut.addListener(this);
    addAndMakeVisible(refBut);

    // Anchor button
    anchorBut.setButtonText("BIG");
    anchorBut.setName("anchor");
    anchorBut.addListener(this);
    addAndMakeVisible(anchorBut);
    anchorBut.setVisible(false);

    // Prev button
    prevBut.setButtonText("<");
    prevBut.setName("prev");
    //prevBut.addListener(this);
    addAndMakeVisible(prevBut);

    // Next button
    nextBut.setButtonText(">");
    nextBut.setName("next");
    nextBut.addListener(this);
    addAndMakeVisible(nextBut);
    
    // Impresions text box
    impresions.setTextToShowWhenEmpty("Observaciones", juce::Colours::lightgrey);
    impresions.setReturnKeyStartsNewLine(true);
    impresions.setMultiLine(true);
    addAndMakeVisible(impresions);

    // Menu button
    int menuSize;
    juce::String menuName = "menu_png";
    auto* playData = BinaryData::getNamedResource(menuName.toUTF8(), menuSize);
    juce::Image menuIcon = juce::ImageFileFormat::loadFrom(playData, menuSize);
    menuBut.setImages(false, true, true, menuIcon, 0.6f, {}, menuIcon, 1.0f, {}, menuIcon, 1.0f, {}, 0);
    menuBut.setName("menu");
    menuBut.setBounds(width - 35, 5, 30, 30);
    menuBut.setTooltip("Audio controls");
    menuBut.setToggleState(true, juce::sendNotification);
    menuBut.onClick = [this] {
        menuBut.setToggleState(!menuBut.getToggleState(), juce::NotificationType::dontSendNotification);
        showMenu = menuBut.getToggleState(); 
        repaint(); };
    addAndMakeVisible(menuBut);

    // For width tests
    if (widthTest == 1)
    {
        textForWidth = 200;
        paintCircles = true;
        anchorBut.setVisible(true);
    };

    //==============================================================================
    //================================== IMAGES ====================================
    //==============================================================================
    // Loading the images of the botttom part of the window
    int upnaSize, upfSize, jaulabSize;
    juce::String upna = "upna_png", upf = "upf_png", jaulab = "jaulab_png";
    // UPNA
    auto* upnaData = BinaryData::getNamedResource(upna.toUTF8(), upnaSize);
    upnaImage = juce::ImageFileFormat::loadFrom(upnaData, upnaSize);
    // UPF
    auto* upfData = BinaryData::getNamedResource(upf.toUTF8(), upfSize);
    upfImage = juce::ImageFileFormat::loadFrom(upfData, upfSize);
    // JAULAB
    auto* jaulabData = BinaryData::getNamedResource(jaulab.toUTF8(), jaulabSize);
    jaulabImage = juce::ImageFileFormat::loadFrom(jaulabData, jaulabSize);

    //==============================================================================
    //================================ RANDOMIZER ==================================
    //==============================================================================
    // Create the random array to load the files
    for (int i = 0; i < stimuli; i++) {
        auto randomInt = juce::Random::getSystemRandom().nextInt(stimuli);
        int tmp = random[i];
        random[i] = random[randomInt];
        random[randomInt] = tmp;
    };

    //==============================================================================
    //================================== AUDIOS ====================================
    //==============================================================================
    // Reading the first group of audios
    formatManager.registerBasicFormats();
    juce::WavAudioFormat wavFormat;

    // Reference audio
    juce::File file(juce::File::getCurrentWorkingDirectory().getChildFile("../../audios/g1/" + files[0][0]).getFullPathName());
    auto is = new juce::FileInputStream(file);

    juce::AudioSampleBuffer newBuffer;
    std::unique_ptr<juce::AudioFormatReader> reader(wavFormat.createReaderFor(is, true));
    newBuffer.setSize((int)reader->numChannels, (int)reader->lengthInSamples);
    reader->read(&newBuffer, 0, (int)reader->lengthInSamples, 0, true, true);

    buffersArray[0] = newBuffer;
    duration = reader->lengthInSamples / reader->sampleRate;
    
    // Anchor audio
    juce::File fileAnchor(juce::File::getCurrentWorkingDirectory().getChildFile("../../audios/g1/" + files[0][1]).getFullPathName());
    auto isAnchor = new juce::FileInputStream(fileAnchor);
    juce::AudioSampleBuffer newBufferAnchor;
    std::unique_ptr<juce::AudioFormatReader> readerAnchor(wavFormat.createReaderFor(isAnchor, true));
    newBufferAnchor.setSize((int)readerAnchor->numChannels, (int)readerAnchor->lengthInSamples);
    readerAnchor->read(&newBufferAnchor, 0, (int)readerAnchor->lengthInSamples, 0, true, true);
    buffersArray[9] = newBufferAnchor;

    // Now reading the stimuli audios
    for (int i = 0; i < stimuli; i++) {
       
        juce::File file(juce::File::getCurrentWorkingDirectory().getChildFile("../../audios/g1/" + files[0][random[i]]).getFullPathName());
        auto is = new juce::FileInputStream(file);

        juce::AudioSampleBuffer newBuffer;
        std::unique_ptr<juce::AudioFormatReader> reader(wavFormat.createReaderFor(is, true));
		newBuffer.setSize((int)reader->numChannels, (int)reader->lengthInSamples);
		reader->read(&newBuffer, 0, (int)reader->lengthInSamples, 0, true, true);
		
		buffersArray[i+1] = newBuffer;
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

    // Some platforms require permissions to open input channels so request that here
    if (juce::RuntimePermissions::isRequired (juce::RuntimePermissions::recordAudio)
        && ! juce::RuntimePermissions::isGranted (juce::RuntimePermissions::recordAudio))
    {
        juce::RuntimePermissions::request (juce::RuntimePermissions::recordAudio,
                                           [&] (bool granted) { setAudioChannels (granted ? 2 : 0, channels); });
    }
    else
    {
        // Specify the number of input and output channels that we want to open
        setAudioChannels (0, channels);
    }
       
    //==============================================================================
    //============================== audioSetupComp ================================
    //==============================================================================
    addAndMakeVisible(audioSetupComp);
    //startTimer(1000);

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

    if (isPlaying && ch)
    {
        auto outputSamplesOffset = bufferToFill.startSample;
        auto outputSamplesRemaining = bufferToFill.numSamples;
        auto bufferSamplesRemaining = buffersArray[buffer].getNumSamples() - position;
        auto samplesThisTime = juce::jmin(outputSamplesRemaining, bufferSamplesRemaining);

        for (int ch = 0; ch < channels; ++ch)
            bufferToFill.buffer->addFrom(ch, outputSamplesOffset, buffersArray[buffer], ch, position, samplesThisTime, 0.5);
        
        outputSamplesRemaining -= samplesThisTime;
        outputSamplesOffset += samplesThisTime;
        position += samplesThisTime;

        if (position == buffersArray[buffer].getNumSamples())
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

    // You can add your drawing code here!
    float w = getWidth();
    float h = getHeight();
    float width = (w - 20) / 10;
    /*
    juce::Rectangle<int> thumbnailBounds(150, 100, w - 250, 30);
    g.setColour(juce::Colours::white);
    g.fillRect(thumbnailBounds);
    g.setColour(juce::Colours::aqua);
    thumbnail.drawChannel(g, thumbnailBounds, 0.0, thumbnail.getTotalLength(), 2, 1.0f);
    //thumbnail.drawChannels(g, thumbnailBounds, 0.0, thumbnail.getTotalLength(), 1.0f);
    g.setColour(juce::Colours::black);
    g.drawLine(150 + posPlayer, 100, 150 + posPlayer, 130, 1.0f);
    */
    audioSetupComp.setBounds(w - 400, 50, 400, 350);
    g.setColour(juce::Colours::black);
    g.setFont(12.0f);
    g.drawText(testText, 10, 2, 80, 20, juce::Justification::left, false);
    g.setFont(12.0f);
    g.drawText(midiText, 10, 20, 80, 20, juce::Justification::left, false);
    
    juce::Rectangle<int> textBounds(10, 40, w - 105 - textForWidth, 200);
    g.setColour(juce::Colours::grey);
    g.fillRect(textBounds);
    g.setColour(juce::Colours::white);
    g.setFont(18.0f);
    g.drawMultiLineText(paintCircles ? descriptionTextWidth : descriptionText, 15, 60, w - 110, juce::Justification::centredLeft);

    g.setFont(18.0f);
    g.setColour(juce::Colours::black);
    for (int i = 0; i < 6; i++)
    {
        g.drawText(juce::String(100 - i*20), 20, 305 + 56 * i, 40, 10, juce::Justification::right, false);
        if (i < 5) { g.drawText(paintCircles ? perceptionsWidth[i] : perceptions[i], 70, 310 + 56 * i, width - 20, 56, juce::Justification::centred, false); }
        g.drawLine(70 , 310 + 56 * i, w - 80, 310 + 56 * i);
    }
    
    prevBut.setBounds(w + marginButtons - width, 80, 28, 40);
    nextBut.setBounds(w + marginButtons - width + 34, 80, 28, 40);
    stopBut.setBounds(w + marginButtons - width, 125, 60, 30);
    refBut.setBounds(w + marginButtons - width, 160, 60, 40);
    anchorBut.setBounds(w + marginButtons - width, 205, 60, 40);
    impresions.setBounds(330, h-70, w - 490, 60);

    g.drawImageWithin(upnaImage, 10, h - 70, 113, 60, juce::RectanglePlacement::stretchToFit);
    g.drawImageWithin(upfImage, 140, h - 70, 173, 60, juce::RectanglePlacement::stretchToFit);
    g.drawImageWithin(jaulabImage, w - 157, h - 70, 137, 60, juce::RectanglePlacement::stretchToFit);

    if (showMenu)
    {
        juce::Rectangle<int> audioComp(w - 400, 0, 400, 350);
        g.setColour(juce::Colours::grey);
        g.fillRect(audioComp);
        g.setColour(juce::Colours::white);
        g.drawRect(audioComp, 2.0f);
        stopBut.setVisible(false);
        refBut.setVisible(false);
        nextBut.setVisible(false);
        prevBut.setVisible(false);
        for (int i = 5; i < 8; i++)
        {
            arrayButtons[i]->setVisible(false);
            arraySliders[i]->setVisible(false);
        };
        audioSetupComp.setVisible(true);
    }
    else
    {
        stopBut.setVisible(true);
        refBut.setVisible(true);
        nextBut.setVisible(true);
        prevBut.setVisible(true);
        for (int i = 5; i < 8; i++)
        {
            arrayButtons[i]->setVisible(true);
            arraySliders[i]->setVisible(true);
        };
        audioSetupComp.setVisible(false);
    };

    if (paintCircles && !showMenu)
    {
        //juce::Rectangle<int> textBounds(10, 40, w - 105 - textForWidth, 200);
        if (buffer != 0 && buffer != 9)
        {
            g.setColour(juce::Colours::forestgreen);
            g.fillEllipse((w - 95 - textForWidth) + (textForWidth / 2) - 25 - circleWidth/2, 115- circleWidth/2, 50 + circleWidth, 50 + circleWidth);
            g.setColour(juce::Colours::black);
            g.drawEllipse((w - 95 - textForWidth) + (textForWidth / 2) - 90, 50, 180, 180, 2.0f);
            g.drawEllipse((w - 95 - textForWidth) + (textForWidth / 2) - 25, 115, 50, 50, 2.0f);
        }
        else if(buffer == 0)
        {
            g.setColour(juce::Colours::forestgreen);
            g.fillEllipse((w - 95 - textForWidth) + (textForWidth / 2) - 25, 115, 50, 50);
            g.setColour(juce::Colours::black);
            g.drawEllipse((w - 95 - textForWidth) + (textForWidth / 2) - 25, 115, 50, 50, 2.0f);
        }
        else if (buffer == 9)
        {
            g.setColour(juce::Colours::forestgreen);
            g.fillEllipse((w - 95 - textForWidth) + (textForWidth / 2) - 90, 50, 180, 180);
            g.setColour(juce::Colours::black);
            g.drawEllipse((w - 95 - textForWidth) + (textForWidth / 2) - 90, 50, 180, 180, 2.0f);
        }
    };
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
    else { isPlaying = true; /*startTimer(20);*/ };

    if (name == "next")
    {
        stopTimer();
        isPlaying = false;
        handleTests();
        testText = "Test " + std::to_string(testNumber + 1);
        impresions.clear();
        for (auto s : arraySliders) { s->setValue(0); }
        repaint();
    }
    else if (name == "ref"){ buffer = 0; }
    else if (name == "1") { buffer = 1; }
    else if (name == "2") { buffer = 2; }
    else if (name == "3"){ buffer = 3; }
    else if (name == "4"){ buffer = 4; }
    else if (name == "5"){ buffer = 5;}
    else if (name == "6"){ buffer = 6; }
    else if (name == "7") { buffer = 7; }
    else if (name == "8") { buffer = 8; }
    else if (name == "anchor") { buffer = 9; }
        
    for (int i = 0; i < stimuli; i++)
    {
        arraySliders[i]->setEnabled(false);
        arraySliders[i]->setColour(0x1001300, juce::Colours::grey);
        arraySliders[i]->setAlpha(0.5f);
        arrayButtons[i]->setColour(0x1000100, juce::Colours::black); 
    }
    refBut.setColour(0x1000100, juce::Colours::black);
    anchorBut.setColour(0x1000100, juce::Colours::black);

    if (buffer != 0 && buffer != 9 && isPlaying)
    {
        arraySliders[buffer - 1]->setEnabled(true);
        arraySliders[buffer - 1]->setColour(0x1001300, juce::Colours::lightgreen);
        arraySliders[buffer - 1]->setAlpha(1.0f);
        arrayButtons[buffer - 1]->setColour(0x1000100, juce::Colours::lightgreen);
        circleWidth = arraySliders[buffer - 1]->getValue() * 1.3;
    }
    
    if(buffer == 0 && isPlaying)
    {
        refBut.setColour(0x1000100, juce::Colours::lightgreen);
    }

    if (buffer == 9 && isPlaying)
    {
        anchorBut.setColour(0x1000100, juce::Colours::lightgreen);
    }

    repaint();
};

void MainComponent::handleTests()
{
    isPlaying = false;
    dataExport();
    testNumber = testNumber + 1;

    // Randomization
    for (int i = 0; i < stimuli; i++) {
        auto randomInt = juce::Random::getSystemRandom().nextInt(stimuli);
        int tmp = random[i];
        random[i] = random[randomInt];
        random[randomInt] = tmp;
    };

    juce::WavAudioFormat wavFormat;

    // Reference audio
    juce::File file(juce::File::getCurrentWorkingDirectory().getChildFile("../../audios/g" + juce::String(testNumber + 1) + "/" + files[testNumber][0]).getFullPathName());
    auto is = new juce::FileInputStream(file);
    juce::AudioSampleBuffer newBuffer;
    std::unique_ptr<juce::AudioFormatReader> reader(wavFormat.createReaderFor(is, true));
    newBuffer.setSize((int)reader->numChannels, (int)reader->lengthInSamples);
    reader->read(&newBuffer, 0, (int)reader->lengthInSamples, 0, true, true);
    buffersArray[0] = newBuffer;
    
    // Anchor audio
    juce::File fileAnchor(juce::File::getCurrentWorkingDirectory().getChildFile("../../audios/g" + juce::String(testNumber + 1) + "/" + files[testNumber][1]).getFullPathName());
    auto isAnchor = new juce::FileInputStream(fileAnchor);
    juce::AudioSampleBuffer newBufferAnchor;
    std::unique_ptr<juce::AudioFormatReader> readerAnchor(wavFormat.createReaderFor(isAnchor, true));
    newBufferAnchor.setSize((int)readerAnchor->numChannels, (int)readerAnchor->lengthInSamples);
    readerAnchor->read(&newBufferAnchor, 0, (int)readerAnchor->lengthInSamples, 0, true, true);
    buffersArray[9] = newBufferAnchor;

    for (int i = 0; i < stimuli; i++) {

        juce::File file(juce::File::getCurrentWorkingDirectory().getChildFile("../../audios/g" + juce::String(testNumber+1) + "/" + files[testNumber][random[i]]).getFullPathName());
        auto is = new juce::FileInputStream(file);

        juce::AudioSampleBuffer newBuffer;
        std::unique_ptr<juce::AudioFormatReader> reader(wavFormat.createReaderFor(is, true));
        newBuffer.setSize((int)reader->numChannels, (int)reader->lengthInSamples);
        reader->read(&newBuffer, 0, (int)reader->lengthInSamples, 0, true, true);

        buffersArray[i + 1] = newBuffer;
    };

    if (testNumber+1 == widthTest)
    {
        textForWidth = 200;
        paintCircles = true;
        anchorBut.setVisible(true);
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
    
    if (stimuli >= 2) {
        juce::XmlElement* b = new juce::XmlElement("B");
        juce::XmlElement* bTech = new juce::XmlElement("tech");
        juce::XmlElement* bAns = new juce::XmlElement("ans");
        bTech->addTextElement(names[random[1]]);
        bAns->addTextElement(std::to_string(arraySliders[1]->getValue()));
        b->addChildElement(bTech);
        b->addChildElement(bAns);
        testElement->addChildElement(b);
    }
    if (stimuli >= 3) {
        juce::XmlElement* c = new juce::XmlElement("C");
        juce::XmlElement* cTech = new juce::XmlElement("tech");
        juce::XmlElement* cAns = new juce::XmlElement("ans");
        cTech->addTextElement(names[random[2]]);
        cAns->addTextElement(std::to_string(arraySliders[2]->getValue()));
        c->addChildElement(cTech);
        c->addChildElement(cAns);
        testElement->addChildElement(c);
    }
    if (stimuli >= 4) {
        juce::XmlElement* d = new juce::XmlElement("D");
        juce::XmlElement* dTech = new juce::XmlElement("tech");
        juce::XmlElement* dAns = new juce::XmlElement("ans");
        dTech->addTextElement(names[random[3]]);
        dAns->addTextElement(std::to_string(arraySliders[3]->getValue()));
        d->addChildElement(dTech);
        d->addChildElement(dAns);
        testElement->addChildElement(d);
    }
    if (stimuli >= 5) {
        juce::XmlElement* e = new juce::XmlElement("E");
        juce::XmlElement* eTech = new juce::XmlElement("tech");
        juce::XmlElement* eAns = new juce::XmlElement("ans");
        eTech->addTextElement(names[random[4]]);
        eAns->addTextElement(std::to_string(arraySliders[4]->getValue()));
        e->addChildElement(eTech);
        e->addChildElement(eAns);
        testElement->addChildElement(e);
    }
    if (stimuli >= 6) {
        juce::XmlElement* f = new juce::XmlElement("F");
        juce::XmlElement* fTech = new juce::XmlElement("tech");
        juce::XmlElement* fAns = new juce::XmlElement("ans");
        fTech->addTextElement(names[random[5]]);
        fAns->addTextElement(std::to_string(arraySliders[5]->getValue()));
        f->addChildElement(fTech);
        f->addChildElement(fAns);
        testElement->addChildElement(f);
    }
    if (stimuli >= 7) {
        juce::XmlElement* g = new juce::XmlElement("G");
        juce::XmlElement* gTech = new juce::XmlElement("tech");
        juce::XmlElement* gAns = new juce::XmlElement("ans");
        gTech->addTextElement(names[random[6]]);
        gAns->addTextElement(std::to_string(arraySliders[6]->getValue()));
        g->addChildElement(gTech);
        g->addChildElement(gAns);
        testElement->addChildElement(g);
    }
    if (stimuli >= 8) {
        juce::XmlElement* h = new juce::XmlElement("H");
        juce::XmlElement* hTech = new juce::XmlElement("tech");
        juce::XmlElement* hAns = new juce::XmlElement("ans");
        hTech->addTextElement(names[random[7]]);
        hAns->addTextElement(std::to_string(arraySliders[7]->getValue()));
        h->addChildElement(hTech);
        h->addChildElement(hAns);
        testElement->addChildElement(h);
    };

    // Add the observations
    juce::XmlElement* text = new juce::XmlElement("observations");
    text->addTextElement(impresions.getText());
    testElement->addChildElement(text);

    testData->addChildElement(testElement);

    if (testNumber + 1 == groups)
    {
        auto xmlString = testData->toString();

        juce::FileChooser folderChooser("Save test", {}, "*.xml");
        folderChooser.browseForFileToSave(true);
        juce::File newFile(folderChooser.getResult());
        newFile.appendText(xmlString);

        alert->showMessageBox(juce::AlertWindow::AlertIconType::InfoIcon, goodBye, "", "Salir");

        juce::JUCEApplicationBase::quit();
    };
}

void MainComponent::readJSON()
{
    juce::File file(juce::File::getCurrentWorkingDirectory().getChildFile("../../testDescription.json").getFullPathName());
    juce::var parsedJson = juce::JSON::parse(file);
    juce::String data = juce::JSON::toString(parsedJson);

    juce::var jsonToParse;
    if (juce::JSON::parse(data, jsonToParse).ok()) {

        path = parsedJson["Path"];
        channels = (int)parsedJson["Channels"];
        widthTest = (int)parsedJson["WidthTest"];
        groups = (int)parsedJson["Groups"];
        stimuli = (int)parsedJson["StimuliPerGroup"];

        files[0][0] = parsedJson["FileNames"]["g1"]["ref"];
        files[0][1] = parsedJson["FileNames"]["g1"]["anchor"];
        files[0][2] = parsedJson["FileNames"]["g1"]["s1"];
        files[0][3] = parsedJson["FileNames"]["g1"]["s2"];
        files[0][4] = parsedJson["FileNames"]["g1"]["s3"];
        files[0][5] = parsedJson["FileNames"]["g1"]["s4"];
        files[0][6] = parsedJson["FileNames"]["g1"]["s5"];
        files[0][7] = parsedJson["FileNames"]["g1"]["s6"];

        if (groups >= 2) {
            files[1][0] = parsedJson["FileNames"]["g2"]["ref"];
            files[1][1] = parsedJson["FileNames"]["g2"]["anchor"];
            files[1][2] = parsedJson["FileNames"]["g2"]["s1"];
            files[1][3] = parsedJson["FileNames"]["g2"]["s2"];
            files[1][4] = parsedJson["FileNames"]["g2"]["s3"];
            files[1][5] = parsedJson["FileNames"]["g2"]["s4"];
            files[1][6] = parsedJson["FileNames"]["g2"]["s5"];
            files[1][7] = parsedJson["FileNames"]["g2"]["s6"];
        }
        if (groups >= 3) {
            files[2][0] = parsedJson["FileNames"]["g3"]["ref"];
            files[2][1] = parsedJson["FileNames"]["g3"]["anchor"];
            files[2][2] = parsedJson["FileNames"]["g3"]["s1"];
            files[2][3] = parsedJson["FileNames"]["g3"]["s2"];
            files[2][4] = parsedJson["FileNames"]["g3"]["s3"];
            files[2][5] = parsedJson["FileNames"]["g3"]["s4"];
            files[2][6] = parsedJson["FileNames"]["g3"]["s5"];
            files[2][7] = parsedJson["FileNames"]["g3"]["s6"];
        }
        if (groups >= 4) {
            files[3][0] = parsedJson["FileNames"]["g4"]["ref"];
            files[3][1] = parsedJson["FileNames"]["g4"]["anchor"];
            files[3][2] = parsedJson["FileNames"]["g4"]["s1"];
            files[3][3] = parsedJson["FileNames"]["g4"]["s2"];
            files[3][4] = parsedJson["FileNames"]["g4"]["s3"];
            files[3][5] = parsedJson["FileNames"]["g4"]["s4"];
            files[3][6] = parsedJson["FileNames"]["g4"]["s5"];
            files[3][7] = parsedJson["FileNames"]["g4"]["s6"];
        }
        if (groups >= 5) {
            files[4][0] = parsedJson["FileNames"]["g5"]["ref"];
            files[4][1] = parsedJson["FileNames"]["g5"]["anchor"];
            files[4][2] = parsedJson["FileNames"]["g5"]["s1"];
            files[4][3] = parsedJson["FileNames"]["g5"]["s2"];
            files[4][4] = parsedJson["FileNames"]["g5"]["s3"];
            files[4][5] = parsedJson["FileNames"]["g5"]["s4"];
            files[4][6] = parsedJson["FileNames"]["g5"]["s5"];
            files[4][7] = parsedJson["FileNames"]["g5"]["s6"];
        }
        if (groups >= 6) {
            files[5][0] = parsedJson["FileNames"]["g6"]["ref"];
            files[5][1] = parsedJson["FileNames"]["g6"]["anchor"];
            files[5][2] = parsedJson["FileNames"]["g6"]["s1"];
            files[5][3] = parsedJson["FileNames"]["g6"]["s2"];
            files[5][4] = parsedJson["FileNames"]["g6"]["s3"];
            files[5][5] = parsedJson["FileNames"]["g6"]["s4"];
            files[5][6] = parsedJson["FileNames"]["g6"]["s5"];
            files[5][7] = parsedJson["FileNames"]["g6"]["s6"];
        }
        if (groups >= 7) {
            files[6][0] = parsedJson["FileNames"]["g7"]["ref"];
            files[6][1] = parsedJson["FileNames"]["g7"]["anchor"];
            files[6][2] = parsedJson["FileNames"]["g7"]["s1"];
            files[6][3] = parsedJson["FileNames"]["g7"]["s2"];
            files[6][4] = parsedJson["FileNames"]["g7"]["s3"];
            files[6][5] = parsedJson["FileNames"]["g7"]["s4"];
            files[6][6] = parsedJson["FileNames"]["g7"]["s5"];
            files[6][7] = parsedJson["FileNames"]["g7"]["s6"];
        }
        if (groups >= 8) {
            files[7][0] = parsedJson["FileNames"]["g8"]["ref"];
            files[7][1] = parsedJson["FileNames"]["g8"]["anchor"];
            files[7][2] = parsedJson["FileNames"]["g8"]["s1"];
            files[7][3] = parsedJson["FileNames"]["g8"]["s2"];
            files[7][4] = parsedJson["FileNames"]["g8"]["s3"];
            files[7][5] = parsedJson["FileNames"]["g8"]["s4"];
            files[7][6] = parsedJson["FileNames"]["g8"]["s5"];
            files[7][7] = parsedJson["FileNames"]["g8"]["s6"];
        };
    };
}

//==============================================================================
//=================================== TIMER ====================================
//==============================================================================
// Esta función leera el estado de carga de la cpu y también el dispositivo que
// se está usando para la salida de audio. Se lellama cada medio segundo con el
// Timer
void MainComponent::timerCallback()
{
    // Leemos el dispositivo que se está usando y le pasamos la info al header
    deviceManager.getAudioDeviceSetup();
    auto* device = deviceManager.getCurrentAudioDevice();
    auto activeOutputChannels = device->getActiveOutputChannels();
    avaiableOutputs = activeOutputChannels.countNumberOfSetBits();

    if (channels > avaiableOutputs)
    {
        ch = false;
        std::string str = "Estás intentando reproducir archivos con más canales de los disponibles";
        alert->showMessageBox(juce::AlertWindow::AlertIconType::WarningIcon, "Error", str, "OK");
    }
    else
    {
        ch = true;
        setAudioChannels(0, channels);
    };

    /*
    posPlayer = posPlayer + 750/(duration/0.02);
    if (posPlayer >= 750.0) { posPlayer = 0.0; };
    repaint();
    */
}
