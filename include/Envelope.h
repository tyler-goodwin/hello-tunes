#pragma once

#include <juce_dsp/juce_dsp.h>

class Envelope {
public:
  enum State { PENDING, ATTACK, SUSTAIN, RELEASE };

  Envelope();

  void setSampleRate(uint32_t value);
  void noteOn();
  void noteOff();
  void reset();

  double getNextLevel();

  void setAttack(double value);
  void setRelease(double value);

  bool isNoteFinished();

  State getCurrentState();

private:
  State state = State::PENDING;

  uint32_t sampleRate = 0;
  double level = 0.0, attack = 0.025, release = 0.025, attackAccumulator = 0.0;

  // Rates that have been adjusted for input rate
  double adjustedAttack = 0.0, adjustedRelease = 0.0;

  void handleAttack();
  void handleRelease();

  double calculateAdjustedRate(double rate);
};