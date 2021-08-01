#include "SynthAudioSource.h"

bool SineWaveVoice::canPlaySound(juce::SynthesiserSound *sound) {
  return dynamic_cast<SineWaveSound *>(sound) != nullptr;
}

void SineWaveVoice::startNote(int midiNoteNumber, float velocity,
                              juce::SynthesiserSound *,
                              int /*currentPitchWheelPosition*/) {
  currentAngle = 0.0;
  level = velocity * 0.15;
  tailOff = 0.0;

  auto cyclesPerSecond = juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber);
  auto cyclesPerSample = cyclesPerSecond / getSampleRate();

  angleDelta = cyclesPerSample * 2.0 * juce::MathConstants<double>::pi;
}

void SineWaveVoice::stopNote(float /*velocity*/, bool allowTailOff) {
  if (allowTailOff) {
    if (tailOff == 0.0)
      tailOff = 1.0;
  } else {
    clearCurrentNote();
    angleDelta = 0.0;
  }
}

void SineWaveVoice::renderNextBlock(juce::AudioSampleBuffer &outputBuffer,
                                    int startSample, int numSamples) {
  if (angleDelta == 0.0)
    return;

  if (tailOff > 0.0) {
    while (--numSamples >= 0) {
      auto currentSample = (float)(std::sin(currentAngle) * level * tailOff);

      for (auto i = outputBuffer.getNumChannels(); --i >= 0;)
        outputBuffer.addSample(i, startSample, currentSample);

      currentAngle += angleDelta;
      ++startSample;

      // Handle tailOff
      tailOff *= juce::dsp::FastMathApproximations::exp(-1 / decayLength);
      if (tailOff <= 0.005) {
        clearCurrentNote();
        angleDelta = 0.0;
        break;
      }
    }
  } else {
    while (--numSamples >= 0) {
      auto currentSample = (float)(std::sin(currentAngle) * level);

      for (auto i = outputBuffer.getNumChannels(); --i >= 0;)
        outputBuffer.addSample(i, startSample, currentSample);

      currentAngle += angleDelta;
      ++startSample;
    }
  }
}

void SineWaveVoice::setDecayLength(double newLength) {
  decayLength = newLength * 10000;
}

SynthAudioSource::SynthAudioSource(juce::MidiKeyboardState &keyState)
    : keyboardState(keyState) {

  for (auto i = 0; i < NUM_VOICES; ++i)
    synth.addVoice(new SineWaveVoice());

  synth.addSound(new SineWaveSound());
}

void SynthAudioSource::setUsingSineWaveSound() { synth.clearSounds(); }

void SynthAudioSource::prepareToPlay(int /*samplesPerBlockExpected*/,
                                     double sampleRate) {
  synth.setCurrentPlaybackSampleRate(sampleRate);
  midiCollector.reset(sampleRate);
}

void SynthAudioSource::getNextAudioBlock(
    const juce::AudioSourceChannelInfo &bufferToFill) {
  bufferToFill.clearActiveBufferRegion();

  juce::MidiBuffer incomingMidi;
  midiCollector.removeNextBlockOfMessages(incomingMidi,
                                          bufferToFill.numSamples);

  keyboardState.processNextMidiBuffer(incomingMidi, bufferToFill.startSample,
                                      bufferToFill.numSamples,
                                      /*injectIndirectEvents*/ true);

  synth.renderNextBlock(*bufferToFill.buffer, incomingMidi,
                        bufferToFill.startSample, bufferToFill.numSamples);
}

juce::MidiMessageCollector *SynthAudioSource::getMidiCollector() {
  return &midiCollector;
}

void SynthAudioSource::setDecayDuration(float newLength) {
  for (auto i = 0; i < NUM_VOICES; ++i) {
    auto voice = static_cast<SineWaveVoice *>(synth.getVoice(i));
    voice->setDecayLength(newLength);
  }
}
