///
/// Utilities
///

const int MAX_DELAY = 9999; // Use this to indicate nothing to do.

int initialStarter = 0; // Keep track when first starting up, waiting for signal to battle.
// 0 = waiting for sensor to be blocked.
// 1 = waiting for sensor to be re-opened.
// -1 = battle!


// Note: Arduino's min() is a #define, so when given a function might run it twice :(
long safeMin(long x, long y) {
  return (x < y) ? x : y;
}

/// LEDS

// Used for debugging.
void setOnboardLED(boolean on) {
  if (on) {
    digitalWrite(LED_BUILTIN, HIGH);
  } else {
    digitalWrite(LED_BUILTIN, LOW);
  }
}


///
/// AUDIO
///

#include "Volume.h" // Include the Volume library
Volume vol;

void XDELAY(int ms) {
  // When using Volume.h, delay needs to go through this:
  vol.delay(ms);
  //delay(ms);
}

void XDELAY_MICRO(int microsec) {
  vol.delayMicroseconds(microsec);
  //delayMicroseconds(microsec);
}

long XMILLIS() {
  return vol.millis();
  //return millis();
}


///
/// DISTANCE SENSORS
///
const int WAY_TOO_FAR = 999;

const int TRIGGER_PIN = 2;
const int ECHO_PIN = 3;
const int MICROSEC_TO_CM = (29 * 2); // Speed of sound ~= 29 microsec/cm
const int MAX_CM = 50; // Only can read distances up to this far.
const int MAX_MICROSEC = (MICROSEC_TO_CM * MAX_CM); // Timeout if not reply in this long.

unsigned long distanceCm() {
  unsigned long durationMicroSec;

  // Trigger the read by pinging the pin with HIGH for >2 microseconds
  digitalWrite(TRIGGER_PIN, LOW);
  XDELAY_MICRO(2);
  digitalWrite(TRIGGER_PIN, HIGH);
  XDELAY_MICRO(5);
  digitalWrite(TRIGGER_PIN, LOW);

  // Read back
  durationMicroSec = pulseIn(ECHO_PIN, HIGH, MAX_MICROSEC);
  if (durationMicroSec == 0) { // 0 = timeout.
    durationMicroSec = MAX_MICROSEC;
  }
  return durationMicroSec / MICROSEC_TO_CM;
}

unsigned long distLast1 = WAY_TOO_FAR;
unsigned long distLast2 = WAY_TOO_FAR;

// Normalize the left sensor by taking the lowest of the last three readings.
unsigned long distanceCmFixed() {
  unsigned long now = distanceCm();
  if (now == 0) {
    Serial.println("OOPS! dist 0?");
    now = WAY_TOO_FAR;
  }
  unsigned long dist = now;
  dist = safeMin(dist, distLast1);
  dist = safeMin(dist, distLast2);
  distLast2 = distLast1;
  distLast1 = now;
  return dist;
}

///
/// WHEEL motor code
///
#define L_DIR 12  // IA1 (orange)
#define L_PWM 11  // IB1 (blue)
#define R_DIR 10  // IB2 (black)
#define R_PWM 9   // IA2 (purple)

const int FORWARD_POWER = 255;
const int TURN_POWER = 255;

// Stop moving and pause for some time.
void stopMoving() {
  digitalWrite(L_PWM, LOW);
  digitalWrite(R_PWM, LOW);
  digitalWrite(L_DIR, LOW);
  digitalWrite(R_DIR, LOW);
}

// Move forwards at a given strength.
void forwardPulse() {
  int power = FORWARD_POWER;
  digitalWrite(L_DIR, HIGH);
  analogWrite(L_PWM, 255-power);
  digitalWrite(R_DIR, HIGH);
  analogWrite(R_PWM, 255-power);
}

// Turn left at a given strength, for some time.
void turn() {
  int power = TURN_POWER;
  digitalWrite(L_DIR, LOW);
  analogWrite(L_PWM, power);
  digitalWrite(R_DIR, HIGH);
  analogWrite(R_PWM, 255-power);
}


///
/// SCOOP motor code
///

#define S_DIR 6 // yellow
#define S_PWM 7 // green
#define SCOOP_PWM_UP 255
#define SCOOP_PWM_DN 10

void driveScoopUp() {
  digitalWrite(S_DIR, LOW);
  digitalWrite(S_PWM, HIGH);
}

void driveScoopDown() {
  digitalWrite(S_DIR, HIGH);
  analogWrite(S_PWM, SCOOP_PWM_DN);
}

void stopScoop() {
  digitalWrite(S_DIR, LOW);
  digitalWrite(S_PWM, LOW);
}


/// SETUP

void setup() {
  Serial.begin(9600);
  // wheels
  pinMode(L_DIR, OUTPUT);
  pinMode(L_PWM, OUTPUT);
  pinMode(R_DIR, OUTPUT);
  pinMode(R_PWM, OUTPUT);
  // scoop
  pinMode(S_DIR, OUTPUT);
  pinMode(S_PWM, OUTPUT);
  // distance sensor.
  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  // debug LED.
  pinMode(LED_BUILTIN, OUTPUT);
  // sound
  vol.begin();
  vol.setMasterVolume(1.0);
  XDELAY(100);

  Serial.println("");
  Serial.println("Set up complete!");
  Serial.println("");
}

///
/// Scoop state (can be used by music code).
///
boolean scoopRunning = false;
boolean scoopForward = false;
boolean initialScoop = false;
boolean postScoop = false;
int scoopMsLeft = -1;

const int FORWARD_SCOOP_MS = 400; // how long to raise the scoop up.
const int BACKWARD_SCOOP_MS = 300; // how long to lower the scoop for.
const int POST_SCOOP_MS = 1000; // how long to wait after a scoop before scooping again, so the scoop doesn't trigger the sensor itself.


///
/// Sensor logic.
///

// Note: Don't query sensor state outside of here, as that takes time.
// Just read these values instead.
int SENSOR_distance = WAY_TOO_FAR;
boolean SENSOR_startMusic = false; // Note: fake sensor, but it works ok. Set to true to start music (when enabled)

void updateSensors() {
  SENSOR_distance = safeMin(distanceCmFixed(), WAY_TOO_FAR);
}

///
/// Music behaviour.
///

int noteMsLeft = -1;

// hz, vol, ms
const int HAHA_NOTE_COUNT = 11;
int hahaNotes[HAHA_NOTE_COUNT][3] = {
  {0, 0, 10},
  {220, 240, 200},
  {0, 0, 10},
  {185, 180, 150},
  {0, 0, 10},
  {247, 255, 100},
  {0, 0, 10},
  {220, 250, 250},
  {0, 0, 10},
  {185, 180, 200},
  {0, 0, 10},
};

int noteToPlay = -1;

const int MARCH_NOTE_COUNT = 38;
const int G = 196 * 2;
const int Gb = 185 * 2;
const int Eb = 156 * 2;
const int Bb = 233 * 2;
const int D = 293 * 2;
const int Eb2 = Eb * 2;
const int SEMIQ = 120;
const int V1 = 200;
const int V2 = 255;

// This is the droid you're looking for.
int marchNotes[MARCH_NOTE_COUNT][3] = {
  // G G G Eb Bb G Eb Bb G
  {0, 0, 20},
  { G, V1, 3 * SEMIQ},
  {0, 0, 20},
  { G, V1, 3 * SEMIQ},
  {0, 0, 20},
  { G, V1, 3 * SEMIQ},
  {0, 0, 20},
  {Eb, V1, 2 * SEMIQ},
  {0, 0, 20},
  {Bb, V1, 1 * SEMIQ},
  {0, 0, 20},
  { G, V2, 3 * SEMIQ},
  {0, 0, 20},
  {Eb, V1, 2 * SEMIQ},
  {0, 0, 20},
  {Bb, V1, 1 * SEMIQ},
  {0, 0, 20},
  { G, V2, 6 * SEMIQ},

  // D D D Eb Bb Gb Eb Bb G
  {  D, V2, 3 * SEMIQ},
  {0, 0, 20},
  {  D, V2, 3 * SEMIQ},
  {0, 0, 20},
  {  D, V2, 3 * SEMIQ},
  {0, 0, 20},
  {Eb2, V2, 2 * SEMIQ},
  {0, 0, 20},
  {  D, V2, 1 * SEMIQ},
  {0, 0, 20},
  { Gb, V2, 3 * SEMIQ},
  {0, 0, 20},
  { Eb, V1, 2 * SEMIQ},
  {0, 0, 20},
  { Bb, V1, 1 * SEMIQ},
  {0, 0, 20},
  {  G, V2, 6 * SEMIQ},
  {0, 0, 20},
};

int (*currentTune)[3];
int currentTuneLength;

// Plays the tune specified above, noteToPlay = index into the array.
long driveMusicTune(long elapsedMs) {
  // Not playing, start the tune if needed.
  if (noteToPlay == -1) {
    if (SENSOR_startMusic) {
      noteToPlay = 0;
      noteMsLeft = -1;
    } else {
      return MAX_DELAY;
    }
  }

  // seed the msLeft with next note's duration, or reduce by how long it has been playing
  if (noteMsLeft == -1) {
    noteMsLeft = currentTune[noteToPlay][2];
  } else {
    noteMsLeft -= elapsedMs;
  }

  // this note has finished, start the next one.
  if (noteMsLeft <= 0) {
    noteToPlay++;
    if (noteToPlay == currentTuneLength) { // finished the tune!
      noteToPlay = -1;
      SENSOR_startMusic = false;
      return MAX_DELAY;
    }
    noteMsLeft = currentTune[noteToPlay][2];
  } else {
    return noteMsLeft;
  }

  // Play the note's hz+volumne, or silence if desired.
  if (currentTune[noteToPlay][0] == 0) { // represent silence as 0 hz.
    vol.noTone();
  } else {
    vol.tone(currentTune[noteToPlay][0], currentTune[noteToPlay][1]);
  }
  return noteMsLeft;
}

// Start playing a given array of {hz, vol, ms} notes, also needs the number of notes to play.
void playTune(int (*tune)[3], int tuneLength) {
  currentTune = tune;
  currentTuneLength = tuneLength;
  noteToPlay = -1;
  SENSOR_startMusic = true;
}


// Music for launch control: low beeping when waiting for sensor to be blocked,
// then high constant tone when waiting for sensor be unblocked,
// then finally silence once battle code has started.

const int BEEP_ON_MS = 750; // emulate Mario Kart beeping start :)
const int BEEP_OFF_MS = 250;
const int BEEP_TONE = 440;
const int WAITING_TONE = 880;

boolean noteBeepOn = false; // keeps track of beeping state
int lastSound = -1;

long driveInitialDebugMusic(long elapsedMs) {
  // battle! so stop tone first time round, then ignore.
  if (initialStarter == -1) {
    if (lastSound != -1) {
      vol.noTone();
    }
    lastSound = -1;
    return MAX_DELAY;

  }

  // check if note needs to keep playing, or be changed.
  noteMsLeft -= elapsedMs;
  if (noteMsLeft > 0) {
    return noteMsLeft;
  }

  if (initialStarter == 0) {
    // Beeping mode.
    if (noteBeepOn) {
      vol.tone(BEEP_TONE, 50);
      noteMsLeft = BEEP_ON_MS;
    } else {
      vol.noTone();
      noteMsLeft = BEEP_OFF_MS;
    }
    noteBeepOn = !noteBeepOn;
  } else if (initialStarter == 1) {
    // Sensor blocked, waiting for it to be unblocked again to start, just play constant tone.
    if (lastSound != WAITING_TONE) {
      lastSound = WAITING_TONE;
      vol.tone(WAITING_TONE, 50);
    }
    noteMsLeft = MAX_DELAY; // keep playing.
  }
  return noteMsLeft;
}

long driveMusic(long elapsedMs) {
  // New in lab 2: Switch to music playing code after the launch sequence is complete.
  if (noteToPlay != -1 || SENSOR_startMusic || scoopRunning) {
    return driveMusicTune(elapsedMs);
  } else {
    return driveInitialDebugMusic(elapsedMs);
  }
}


///
/// Wheel driving behaviour
///

// State machine: starts stopped, drives forwards for a bit at the start,
// then stays in the rotate-left state.
// Can be set to DRIVE_FORWARDS_INTERRUPT mode while an enemy is nearby in front (incl. being scooped)
// in which case, drive towards them to increase chance of scoop / move them out of ring.

// TODO: include stopping while scoop is being driven.
const int DRIVE_STOP = 0;
const int DRIVE_FORWARDS = 1;
const int DRIVE_LEFT = 2;
const int DRIVE_FORWARDS_INTERRUPT = 3;

// How long to drive forwards into the centre of the ring at the start.
const int INITIAL_FORWARDS_MS = 1000;

int driveState = -1; // current state
long msWheelsLeft = -1; // time left in this state
int lastDrive = -1; // last direction

long driveWheels(long elapsedMs) {
  // Nothing happening yet, so await further command.
  if (driveState == -1) {
    return MAX_DELAY;
  }

  // Check if we need to keep going for a while in this direction.
  msWheelsLeft -= elapsedMs;
  if (msWheelsLeft > 0) {
    return msWheelsLeft;
  }

  if (driveState == DRIVE_FORWARDS) {
    // Driven forwards for as long as needed, to change to spinning.
    driveState = DRIVE_LEFT;
    turn();
    return MAX_DELAY;
  } else if (driveState == DRIVE_LEFT) {
    // do nothing, keep going left.
  } else if (driveState == DRIVE_FORWARDS_INTERRUPT) {
    // do nothing, we stop going forwards in post sensor logic.
  }

  // in turning mode, so always turn for as long as possible.
  return MAX_DELAY;
}

///
/// Scoop driving logic
///

long driveScoop(long elapsedMs) {
  // Enforce pause time after scoop before sensing again.
  if (postScoop) {
    scoopMsLeft -= elapsedMs;
    if (scoopMsLeft <= 0) {
      scoopMsLeft = -1;
      postScoop = false;
      scoopRunning = false;
      driveState = DRIVE_LEFT;
      turn();
    }
  }

  if (!scoopRunning) {
    return MAX_DELAY;
  }

  // startScoop() has been called, so set up state machine.
  if (initialScoop) {
    initialScoop = false;
    scoopForward = true;
    scoopMsLeft = FORWARD_SCOOP_MS;
    driveScoopUp();
    return scoopMsLeft;
  }

  // Continue moving the scoop in its current direction if need be..
  scoopMsLeft -= elapsedMs;
  if (scoopMsLeft > 0) {
    return scoopMsLeft;
  }

  // Otherwise, the current scoop direction needs to stop!
  if (scoopForward) {
    // in this case, change it from scoop-up to scoop-down.
    scoopForward = false;
    scoopMsLeft = BACKWARD_SCOOP_MS;
    driveScoopDown();
    return scoopMsLeft;
  } else {
    // Otherwise, finished scoop! Stop, and move to post-scoop pause.
    driveState = DRIVE_LEFT;
    turn();
    initialScoop = false;
    postScoop = true;
    scoopForward = false;
    scoopMsLeft = POST_SCOOP_MS;
    // Note: scoopRunning = true here still, to avoid another startScoop() until after cooldown.
    stopScoop();
    return scoopMsLeft;
  }
  return MAX_DELAY;
}

// Call this to run the scoop process once.
void startScoop() {
  SENSOR_startMusic = true;
  if (scoopRunning) {
    return;
  }
  scoopRunning = true;
  initialScoop = true;
}


///
/// Debug LED behaviour
///

const int LED_BLINK_MS = 1000; // blink every second, to test timing.
boolean isLEDOn = false;
long msLEDLeft = LED_BLINK_MS;

long driveDebugLED(long elapsedMs) {
  // Check if we should remain in this current state.
  msLEDLeft -= elapsedMs;
  if (msLEDLeft > 0) {
    return msLEDLeft;
  }
  // Otherwise, flip to blink the opposite state
  isLEDOn = !isLEDOn;
  setOnboardLED(isLEDOn);
  msLEDLeft = LED_BLINK_MS;
  return msLEDLeft;
}


///
/// Start-the-battle-by-waving-hand-in-front-of-sensor behaviour
///

// At the very start of battle, drive forwards for a bit.
void startPlaying(long elapsedMs) {
  Serial.println("START!");
  playTune(marchNotes, MARCH_NOTE_COUNT);
  driveState = DRIVE_FORWARDS;
  msWheelsLeft = elapsedMs + INITIAL_FORWARDS_MS;
  forwardPulse();
}

// Run some logic after sensor readings to
void postSensorLogic(long elapsedMs) {
  if (initialStarter == 0 && SENSOR_distance < 5) {
    // Sensor blocked, so now waiting for hand to be removed state instead (= 1).
    initialStarter = 1;
  } else if (initialStarter == 1 && SENSOR_distance > 10) {
    // Sensor unlocked, so start the battle! (= -1)
    startPlaying(elapsedMs);
    initialStarter = -1;
  } else if (initialStarter == -1) {
    if (!scoopRunning && SENSOR_distance < 7){
      // In battle mode, and enemy close, so scoop away!
      driveState = DRIVE_FORWARDS_INTERRUPT;
      forwardPulse();
      startScoop();

    // New logic added for lab 2!
    } else if (SENSOR_distance < 15) {
      // Enemy close, but not close enough to scoop, so move towards them.
      driveState = DRIVE_FORWARDS_INTERRUPT;
      forwardPulse();
    } else {
      // Enemy not close, so stop driving forwards as they're not still there...
      if (driveState == DRIVE_FORWARDS_INTERRUPT) {
        driveState = DRIVE_LEFT;
        turn();
      }
    }
  }
}


// Battle logic for running multiple systems simultaneously:
const int MAX_LOOP_DELAY = 50;
unsigned long lastLoopTimeMs = -1;

void battle() {
  // First, update the sensors values to what they should be:
  updateSensors();

  // Then, find out how much time has passed since last execution:
  unsigned long loopTimeMs = XMILLIS();
  long elapsedMs = loopTimeMs - lastLoopTimeMs; // Time change since here last.
  boolean firstRun = (lastLoopTimeMs == -1);
  lastLoopTimeMs = loopTimeMs;
  if (firstRun) { // First run through, do nothing.
    XDELAY(20);
    return;
  }

  // Update some stuff based off sensor readings.
  postSensorLogic(elapsedMs);
  long toDelay = MAX_DELAY;
  // Invoke each of the behaviours, finding which needs to act next.
  toDelay = safeMin(toDelay, driveMusic(elapsedMs));
  toDelay = safeMin(toDelay, driveWheels(elapsedMs));
  toDelay = safeMin(toDelay, driveScoop(elapsedMs));
  toDelay = safeMin(toDelay, driveDebugLED(elapsedMs));
  toDelay = safeMin(toDelay, MAX_LOOP_DELAY); // Make sure we pause for at most this long, to avoid missing stuff.
  // And delay as long as required...
  if (toDelay > 0) {
    XDELAY(toDelay);
  }
}



///
/// Testing code
///

// Blink the onboard LED.
void testLED() {
  setOnboardLED(true);
  XDELAY(1000);
  setOnboardLED(false);
  XDELAY(1000);
}

// Drive the scoop up & back continually, while driving the motors forwards
void testScoop() {
  forwardPulse();
  while(true) {
    driveScoopUp();
    XDELAY(FORWARD_SCOOP_MS);
    driveScoopDown();
    XDELAY(BACKWARD_SCOOP_MS);
  }
}

// Drive motors forwards and then turn, periodically.
void testMotors() {
  while(true) {
    forwardPulse();
    XDELAY(INITIAL_FORWARDS_MS);
    turn();
    XDELAY(INITIAL_FORWARDS_MS);
  }
}

///
/// LOOP CODE HERE!
///
void loop() {
  battle();
  //testScoop();
  //testMotors();
  //testLED();
}
