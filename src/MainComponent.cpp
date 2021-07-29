#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
    setSize (600, 400);
    setAudioChannels(INPUT_CHANNELS, OUTPUT_CHANNELS);

    addAndMakeVisible(frequencySlider);
    frequencySlider.setRange(50, 15000);
    frequencySlider.setTextValueSuffix("Hz");
    frequencySlider.addListener(this);

    addAndMakeVisible(frequencyLabel);
    frequencyLabel.setText("Frequency", juce::dontSendNotification);
    frequencyLabel.attachToComponent(&frequencySlider, true);

    addAndMakeVisible(amplitudeSlider);
    amplitudeSlider.setRange(0.0, 1.0);
    amplitudeSlider.addListener(this);

    addAndMakeVisible(amplitudeLabel);
    amplitudeLabel.setText("Amplitude", juce::dontSendNotification);
    amplitudeLabel.attachToComponent(&amplitudeSlider, true);

    log = juce::Logger::getCurrentLogger();
}

MainComponent::~MainComponent()
{
    shutdownAudio();
}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setFont (juce::Font (16.0f));
    g.setColour (juce::Colours::white);
    g.drawText ("Hello Tunes!", getLocalBounds(), juce::Justification::centred, true);

}

void MainComponent::resized()
{
    auto sliderLeft = 120;
    frequencySlider.setBounds(sliderLeft, 20, getWidth() - sliderLeft - 10, 20);
    amplitudeSlider.setBounds(sliderLeft, 50, getWidth() - sliderLeft - 10, 20);
}

void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate) {
    currentSampleRate = sampleRate;
    updateAngleDelta();

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
        auto currentSample = (float) std::sin(currentAngle);
        currentAngle += angleDelta;

        float leveledSample = currentSample * (float) currentAmplitude;

        for(auto channel = 0; channel < numChannels; ++channel) {
            buffers[channel][sample] = leveledSample;
        }
    }
}

void MainComponent::updateAngleDelta()
{
    auto cyclesPerSample = frequencySlider.getValue() / currentSampleRate;
    angleDelta = cyclesPerSample * 2.0 * juce::MathConstants<double>::pi;
}

void MainComponent::sliderValueChanged(juce::Slider* slider)
{
    if(slider == &frequencySlider) {
        if(currentSampleRate > 0.0)
            updateAngleDelta();
    } else if (slider == &amplitudeSlider) {
        currentAmplitude = amplitudeSlider.getValue();
    }
}