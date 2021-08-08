#include "SynthAudioSource.h"

bool SineWaveVoice::canPlaySound(juce::SynthesiserSound *sound) {
  return dynamic_cast<SineWaveSound *>(sound) != nullptr;
}

void SineWaveVoice::startNote(int midiNoteNumber, float velocity,
                              juce::SynthesiserSound *,
                              int /*currentPitchWheelPosition*/) {
  currentAngle = 0.0;
  level = velocity * 0.15;

  auto cyclesPerSecond = juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber);
  auto cyclesPerSample = cyclesPerSecond / getSampleRate();

  angleDelta = cyclesPerSample * 2.0 * juce::MathConstants<double>::pi;
  envelope.noteOn();
}

void SineWaveVoice::stopNote(float /*velocity*/, bool allowTailOff) {
  if (allowTailOff) {
    envelope.noteOff();
  } else {
    envelope.reset();
    clearCurrentNote();
    angleDelta = 0.0;
  }
}

void SineWaveVoice::renderNextBlock(juce::AudioSampleBuffer &outputBuffer,
                                    int startSample, int numSamples) {
  if (angleDelta == 0.0)
    return;

  while (--numSamples >= 0) {
    auto currentSample =
        (float)(std::sin(currentAngle) * level * envelope.getNextLevel());

    for (auto i = outputBuffer.getNumChannels(); --i >= 0;)
      outputBuffer.addSample(i, startSample, currentSample);

    currentAngle += angleDelta;
    ++startSample;

    if (envelope.isNoteFinished()) {
      envelope.reset();
      clearCurrentNote();
      angleDelta = 0.0;
      break;
    }
  }
}

void SineWaveVoice::setAttack(double value) {
  envelope.setAttack(value * 10000);
}

void SineWaveVoice::setRelease(double value) {
  envelope.setRelease(value * 10000);
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

void SynthAudioSource::setAttackDuration(double value) {
  for (auto i = 0; i < NUM_VOICES; ++i) {
    auto voice = static_cast<SineWaveVoice *>(synth.getVoice(i));
    voice->setAttack(value);
  }
}

void SynthAudioSource::setReleaseDuration(double value) {
  for (auto i = 0; i < NUM_VOICES; ++i) {
    auto voice = static_cast<SineWaveVoice *>(synth.getVoice(i));
    voice->setRelease(value);
  }
}
