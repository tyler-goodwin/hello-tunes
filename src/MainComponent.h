#pragma once

// CMake builds don't use an AppConfig.h, so it's safe to include juce module headers
// directly. If you need to remain compatible with Projucer-generated builds, and
// have called `juce_generate_juce_header(<thisTarget>)` in your CMakeLists.txt,
// you could `#include <JuceHeader.h>` here instead, to make all your module headers visible.
#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_audio_utils/juce_audio_utils.h>

constexpr int INPUT_CHANNELS = 0;
constexpr int OUTPUT_CHANNELS = 2;
//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent   : public juce::AudioAppComponent,
                        public juce::Slider::Listener
{
public:

    //==============================================================================
    MainComponent();
    ~MainComponent();

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void releaseResources() override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;

private:
    //==============================================================================
    juce::Slider frequencySlider;
    juce::Label  frequencyLabel;
    juce::Slider amplitudeSlider;
    juce::Label  amplitudeLabel;

    double
        currentAmplitude  = 0.0,
        currentSampleRate = 0.0,
        currentAngle      = 0.0,
        angleDelta        = 0.0;

    juce::Logger* log;

    void updateAngleDelta();
    void sliderValueChanged(juce::Slider* slider) override;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};