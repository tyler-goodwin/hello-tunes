#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
    : keyboardComponent(keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard),
      startTime(juce::Time::getMillisecondCounterHiRes() * 0.001)
{
    setOpaque (true);
    setSize (600, 400);
    setAudioChannels(INPUT_CHANNELS, OUTPUT_CHANNELS);

    addAndMakeVisible(midiInputListLabel);
    midiInputListLabel.setText ("MIDI Input:", juce::dontSendNotification);
    midiInputListLabel.attachToComponent (&midiInputList, true);

    addAndMakeVisible(midiInputList);
    midiInputList.setTextWhenNoChoicesAvailable("No Midi Inputs Enabled");
    auto midiInputs = juce::MidiInput::getAvailableDevices();

    juce::StringArray midiInputNames;

    for(auto input : midiInputs)
        midiInputNames.add(input.name);

    midiInputList.addItemList(midiInputNames, 1);
    midiInputList.onChange = [this] { setMidiInput(midiInputList.getSelectedItemIndex()); };

    for(auto input: midiInputs) {
        if(deviceManager.isMidiInputDeviceEnabled(input.identifier)) {
            setMidiInput(midiInputs.indexOf(input));
            break;
        }
    }

    if(midiInputList.getSelectedId() == 0) {
        setMidiInput(0);
    }

    addAndMakeVisible(amplitudeSlider);
    amplitudeSlider.setRange(0.0, 1.0);
    amplitudeSlider.addListener(this);

    addAndMakeVisible(amplitudeLabel);
    amplitudeLabel.setText("Amplitude", juce::dontSendNotification);
    amplitudeLabel.attachToComponent(&amplitudeSlider, true);

    addAndMakeVisible(midiMessagesBox);
    midiMessagesBox.setMultiLine (true);
    midiMessagesBox.setReturnKeyStartsNewLine (true);
    midiMessagesBox.setReadOnly (true);
    midiMessagesBox.setScrollbarsShown (true);
    midiMessagesBox.setCaretVisible (false);
    midiMessagesBox.setPopupMenuEnabled (true);
    midiMessagesBox.setColour (juce::TextEditor::backgroundColourId, juce::Colour (0x32ffffff));
    midiMessagesBox.setColour (juce::TextEditor::outlineColourId, juce::Colour (0x1c000000));
    midiMessagesBox.setColour (juce::TextEditor::shadowColourId, juce::Colour (0x16000000));

    addAndMakeVisible(keyboardComponent);
    keyboardState.addListener(this);


    log = juce::Logger::getCurrentLogger();
}

MainComponent::~MainComponent()
{
    keyboardState.removeListener(this);
    deviceManager.removeMidiInputDeviceCallback(
        juce::MidiInput::getAvailableDevices()[midiInputList.getSelectedItemIndex()].identifier,
        this
    );
    shutdownAudio();
}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void MainComponent::resized()
{
    auto area = getLocalBounds();

    auto sliderLeft = 120;
    amplitudeSlider.setBounds(area.removeFromTop(30).removeFromRight(getWidth() - sliderLeft - 10).reduced(8));

    midiInputList    .setBounds (area.removeFromTop(60).removeFromRight (getWidth() - 150).reduced (8));
    keyboardComponent.setBounds (area.removeFromTop(90).reduced(8));
    midiMessagesBox  .setBounds (area.reduced (8));
}

void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate) {
    currentSampleRate = sampleRate;

    juce::String message;
    message << "Preparing to play audio...\n"
        << "    samplesPerBlockExpected: " << samplesPerBlockExpected << "\n"
        << "    sampleRate: " << sampleRate;
    log->writeToLog(message);
}

void MainComponent::releaseResources() {
    log->writeToLog("Releasing audio resources");
}

void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    float* buffers[OUTPUT_CHANNELS];

    auto numChannels = bufferToFill.buffer->getNumChannels();

    for(auto channel = 0; channel < numChannels; ++channel) {
        buffers[channel] = bufferToFill.buffer->getWritePointer(channel, bufferToFill.startSample);
    }

    for(auto sample = 0; sample < bufferToFill.numSamples; ++sample) {
        auto currentSample = 0.0;

        float leveledSample = currentSample * (float) currentAmplitude;

        for(auto channel = 0; channel < numChannels; ++channel) {
            buffers[channel][sample] = leveledSample;
        }
    }
}

void MainComponent::handleIncomingMidiMessage(juce::MidiInput* source, const juce::MidiMessage& message)
{
    const juce::ScopedValueSetter<bool> scopedInputFlag(isAddingFromMidiInput, true);
    keyboardState.processNextMidiEvent(message);
    postMessageToList(message, source->getName());
}

void MainComponent::handleNoteOn(juce::MidiKeyboardState*, int midiChannel, int midiNoteNumber, float velocity)
{
    if(isAddingFromMidiInput) return;

    auto m = juce::MidiMessage::noteOn(midiChannel, midiNoteNumber, velocity);
    m.setTimeStamp(juce::Time::getMillisecondCounterHiRes() * 0.001);
    postMessageToList(m, "On-Screen Keyboard");
}

void MainComponent::handleNoteOff(juce::MidiKeyboardState*, int midiChannel, int midiNoteNumber, float /*velocity*/)
{
    if(isAddingFromMidiInput) return;

    auto m = juce::MidiMessage::noteOff(midiChannel, midiNoteNumber);
    m.setTimeStamp(juce::Time::getMillisecondCounterHiRes() * 0.001);
    postMessageToList(m, "On-Screen Keyboard");
}


void MainComponent::sliderValueChanged(juce::Slider* slider)
{
    if (slider == &amplitudeSlider) {
        currentAmplitude = amplitudeSlider.getValue();
    }
}

void MainComponent::setMidiInput(int index) {
    auto list = juce::MidiInput::getAvailableDevices();

    deviceManager.removeMidiInputDeviceCallback(list[lastInputIndex].identifier, this);

    auto newInput = list[index];

    if(!deviceManager.isMidiInputDeviceEnabled(newInput.identifier))
        deviceManager.setMidiInputDeviceEnabled(newInput.identifier, this);

    deviceManager.addMidiInputCallback(newInput.identifier, this);
    midiInputList.setSelectedId(index + 1, juce::dontSendNotification);

    lastInputIndex = index;
}

void MainComponent::postMessageToList(const juce::MidiMessage& message, const juce::String& source)
{
    (new IncomingMessageCallback(this, message, source))->post();
}