#include "Envelope.h"
#include "catch/catch.hpp"


TEST_CASE("Goes through ADSR loop", "[Envelope]") {
  Envelope envelope;

  envelope.setAttack(100);
  envelope.setRelease(100);

  REQUIRE(envelope.getNextLevel() == 0.0);
  REQUIRE(envelope.getCurrentState() == Envelope::State::PENDING);

  envelope.noteOn();

  REQUIRE(envelope.getCurrentState() == Envelope::State::ATTACK);

  double current = 0.0;
  double prev = envelope.getNextLevel();

  int attackLoops = 0;
  while (envelope.getCurrentState() != Envelope::State::SUSTAIN &&
         attackLoops++ < 1000000) {
    current = envelope.getNextLevel();
    REQUIRE(current > prev);
    prev = current;
  }

  REQUIRE(attackLoops > 500);
  REQUIRE(attackLoops < 550);

  REQUIRE(envelope.getCurrentState() == Envelope::State::SUSTAIN);
  REQUIRE(envelope.getNextLevel() == 1.0);

  envelope.noteOff();

  REQUIRE(envelope.getCurrentState() == Envelope::State::RELEASE);
  REQUIRE(envelope.getNextLevel() < 1.0);

  int releaseLoops = 0;

  while (!envelope.isNoteFinished() && releaseLoops++ < 1000000) {
    current = envelope.getNextLevel();
    REQUIRE(current < prev);
    prev = current;
  }

  REQUIRE(releaseLoops > 500);
  REQUIRE(releaseLoops < 550);

  envelope.reset();

  REQUIRE(envelope.getCurrentState() == Envelope::State::PENDING);
}