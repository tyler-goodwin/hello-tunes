#pragma once

#include <juce_dsp/juce_dsp.h>

class Envelope {
public:
  enum State { PENDING, ATTACK, SUSTAIN, RELEASE };

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

  double level = 0.0, attack = 0.025, release = 0.025, attackAccumulator = 0.0;

  void handleAttack();
  void handleRelease();
};