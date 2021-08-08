#pragma once

// CMake builds don't use an AppConfig.h, so it's safe to include juce module
// headers directly. If you need to remain compatible with Projucer-generated
// builds, and have called `juce_generate_juce_header(<thisTarget>)` in your
// CMakeLists.txt, you could `#include <JuceHeader.h>` here instead, to make all
// your module headers visible.
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_dsp/juce_dsp.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>

#include "SynthAudioSource.h"

constexpr int INPUT_CHANNELS = 0;
constexpr int OUTPUT_CHANNELS = 2;

class MainComponent : public juce::AudioAppComponent,
                      public juce::Slider::Listener {
public:
  //==============================================================================
  MainComponent();
  ~MainComponent();

  //==============================================================================
  void paint(juce::Graphics &) override;
  void resized() override;

  void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
  void releaseResources() override;

  void
  getNextAudioBlock(const juce::AudioSourceChannelInfo &bufferToFill) override;

private:
  //==============================================================================
  juce::AudioDeviceManager deviceManager;
  juce::ComboBox midiInputList;
  juce::Label midiInputListLabel;
  int lastInputIndex = 0;
  bool isAddingFromMidiInput = false;

  juce::MidiKeyboardState keyboardState;
  juce::MidiKeyboardComponent keyboardComponent;

  SynthAudioSource synthAudioSource;

  juce::Slider releaseSlider;
  juce::Label releaseLabel;

  juce::Slider attackSlider;
  juce::Label attackLabel;

  double currentAmplitude = 0.0, currentSampleRate = 0.0;

  juce::Logger *log;

  void sliderValueChanged(juce::Slider *slider) override;
  void setMidiInput(int index);

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
