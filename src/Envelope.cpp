#include "Envelope.h"

void Envelope::noteOn() {
  state = State::ATTACK;
  attackAccumulator = 1.0;
}

void Envelope::noteOff() { state = State::RELEASE; }

void Envelope::reset() {
  level = 0.0;
  state = State::PENDING;
}

double Envelope::getNextLevel() {
  switch (state) {
  case State::ATTACK:
    handleAttack();
    break;
  case State::RELEASE:
    handleRelease();
    break;
  default:
    // Do nothing!
    break;
  }

  return level;
}

void Envelope::setAttack(double value) { attack = value; }

void Envelope::setRelease(double value) { release = value; }

bool Envelope::isNoteFinished() {
  return state == State::RELEASE && level <= 0.005;
}

Envelope::State Envelope::getCurrentState() { return state; }

void Envelope::handleAttack() {
  attackAccumulator *= juce::dsp::FastMathApproximations::exp(-1 / attack);

  level = 1 - attackAccumulator;

  if (attackAccumulator <= 0.005) {
    level = 1.0;
    state = State::SUSTAIN;
  }
}

void Envelope::handleRelease() {
  level *= juce::dsp::FastMathApproximations::exp(-1 / release);
}
