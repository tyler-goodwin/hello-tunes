#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_devices/juce_audio_devices.h>

#include "Envelope.h"

constexpr int NUM_VOICES = 8;
constexpr double MIN_DECAY = 0.01;
constexpr double MAX_DECAY = 5.0;

struct SineWaveSound : public juce::SynthesiserSound {
  SineWaveSound() {}

  bool appliesToNote(int) override { return true; }
  bool appliesToChannel(int) override { return true; }
};

struct SineWaveVoice : public juce::SynthesiserVoice {

  SineWaveVoice() {
    setAttack(MIN_DECAY);
    setRelease(MIN_DECAY);
  }

  bool canPlaySound(juce::SynthesiserSound *sound) override;

  void startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound *,
                 int /*currentPitchWheelPosition*/
                 ) override;

  void stopNote(float /*velocity*/, bool allowTailOff) override;

  void pitchWheelMoved(int) override{};
  void controllerMoved(int, int) override{};

  void renderNextBlock(juce::AudioSampleBuffer &outputBuffer, int startSample,
                       int numSamples) override;

  void setAttack(double value);
  void setRelease(double value);

private:
  double currentAngle = 0.0, angleDelta = 0.0, level = 0.0;

  Envelope envelope;
};

class SynthAudioSource : public juce::AudioSource {
public:
  SynthAudioSource(juce::MidiKeyboardState &keyState);

  void setUsingSineWaveSound();

  void prepareToPlay(int /*samplesPerBlockExpected*/,
                     double sampleRate) override;

  void releaseResources() override {}

  void
  getNextAudioBlock(const juce::AudioSourceChannelInfo &bufferToFill) override;

  juce::MidiMessageCollector *getMidiCollector();

  void setAttackDuration(double value);
  void setReleaseDuration(double value);

private:
  juce::MidiMessageCollector midiCollector;
  juce::MidiKeyboardState &keyboardState;
  juce::Synthesiser synth;
};
