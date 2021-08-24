#include "Envelope.h"

Envelope::Envelope() {
  adjustedAttack = calculateAdjustedRate(attack);
  adjustedRelease = calculateAdjustedRate(release);
}

void Envelope::setSampleRate(uint32_t value) { sampleRate = value; }

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

void Envelope::setAttack(double value) {
  attack = value;
  adjustedAttack = calculateAdjustedRate(value);
}

void Envelope::setRelease(double value) {
  release = value;
  adjustedRelease = calculateAdjustedRate(value);
}

bool Envelope::isNoteFinished() {
  return state == State::RELEASE && level <= 0.005;
}

Envelope::State Envelope::getCurrentState() { return state; }

void Envelope::handleAttack() {
  attackAccumulator *= adjustedAttack;

  level = 1 - attackAccumulator;

  if (attackAccumulator <= 0.005) {
    level = 1.0;
    state = State::SUSTAIN;
  }
}

void Envelope::handleRelease() { level *= adjustedRelease; }

double Envelope::calculateAdjustedRate(double rate) {
  /**
   * To make exponential decay sample rate independent we solve the following
   * for sampleRateAdjust:
   * (rate * sampleRateAdjust)^(sampleRate - 1) = 1/200
   *
   * This is based on the generic solution to a geometric series.
   * Nn = (N0 * factor)^(n -1)
   *
   * This gives:
   * sampleRateAdjust = (200^(-1/(rate * sampleRate -1)))/rate
   *
   * So finally when we do the calcuation we do
   * n1 = n0 * rate * sampleRateAdjust
   *
   * Which gives n1 = n0 * adjustedRate
   *
   * where adjustedRate = rate * (200^(-1/(rate * sampleRate -1)))/rate
   *                    = 200^(-1/(rate * sampleRate -1))
   */

  return pow(200, -1 / (rate * sampleRate - 1));
}
