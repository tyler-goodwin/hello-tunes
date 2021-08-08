#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
    : synthAudioSource(keyboardState),
      keyboardComponent(keyboardState,
                        juce::MidiKeyboardComponent::horizontalKeyboard) {
  setOpaque(true);
  setSize(600, 400);
  setAudioChannels(INPUT_CHANNELS, OUTPUT_CHANNELS);

  addAndMakeVisible(midiInputListLabel);
  midiInputListLabel.setText("MIDI Input:", juce::dontSendNotification);
  midiInputListLabel.attachToComponent(&midiInputList, true);

  addAndMakeVisible(midiInputList);
  midiInputList.setTextWhenNoChoicesAvailable("No Midi Inputs Enabled");
  auto midiInputs = juce::MidiInput::getAvailableDevices();

  juce::StringArray midiInputNames;

  for (auto input : midiInputs)
    midiInputNames.add(input.name);

  midiInputList.addItemList(midiInputNames, 1);
  midiInputList.onChange = [this] {
    setMidiInput(midiInputList.getSelectedItemIndex());
  };

  for (auto input : midiInputs) {
    if (deviceManager.isMidiInputDeviceEnabled(input.identifier)) {
      setMidiInput(midiInputs.indexOf(input));
      break;
    }
  }

  if (midiInputList.getSelectedId() == 0) {
    setMidiInput(0);
  }

  addAndMakeVisible(attackSlider);
  attackSlider.setRange(MIN_DECAY, MAX_DECAY);
  attackSlider.addListener(this);

  addAndMakeVisible(attackLabel);
  attackLabel.setText("Attack", juce::dontSendNotification);
  attackLabel.attachToComponent(&attackSlider, true);

  addAndMakeVisible(releaseSlider);
  releaseSlider.setRange(MIN_DECAY, MAX_DECAY);
  releaseSlider.addListener(this);

  addAndMakeVisible(releaseLabel);
  releaseLabel.setText("Release", juce::dontSendNotification);
  releaseLabel.attachToComponent(&releaseSlider, true);

  addAndMakeVisible(keyboardComponent);

  log = juce::Logger::getCurrentLogger();
}

MainComponent::~MainComponent() { shutdownAudio(); }

//==============================================================================
void MainComponent::paint(juce::Graphics &g) {
  g.fillAll(
      getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void MainComponent::resized() {
  auto area = getLocalBounds();

  auto sliderLeft = 120;
  attackSlider.setBounds(area.removeFromTop(30)
                             .removeFromRight(getWidth() - sliderLeft - 10)
                             .reduced(8));

  releaseSlider.setBounds(area.removeFromTop(30)
                              .removeFromRight(getWidth() - sliderLeft - 10)
                              .reduced(8));

  midiInputList.setBounds(
      area.removeFromTop(60).removeFromRight(getWidth() - 150).reduced(8));

  keyboardComponent.setBounds(area.removeFromTop(90).reduced(8));
}

void MainComponent::prepareToPlay(int samplesPerBlockExpected,
                                  double sampleRate) {
  synthAudioSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
}

void MainComponent::releaseResources() { synthAudioSource.releaseResources(); }

void MainComponent::getNextAudioBlock(
    const juce::AudioSourceChannelInfo &bufferToFill) {
  synthAudioSource.getNextAudioBlock(bufferToFill);
}

void MainComponent::sliderValueChanged(juce::Slider *slider) {
  if (slider == &releaseSlider) {
    synthAudioSource.setReleaseDuration(releaseSlider.getValue());
  } else if (slider == &attackSlider) {
    synthAudioSource.setAttackDuration(attackSlider.getValue());
  }
}

void MainComponent::setMidiInput(int index) {
  auto list = juce::MidiInput::getAvailableDevices();

  auto newInput = list[index];

  if (!deviceManager.isMidiInputDeviceEnabled(newInput.identifier))
    deviceManager.setMidiInputDeviceEnabled(newInput.identifier, this);

  deviceManager.addMidiInputCallback(newInput.identifier,
                                     synthAudioSource.getMidiCollector());
  midiInputList.setSelectedId(index + 1, juce::dontSendNotification);

  lastInputIndex = index;
}
