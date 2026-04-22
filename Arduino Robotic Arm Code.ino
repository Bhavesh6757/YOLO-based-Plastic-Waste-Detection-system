
/*
 * ============================================================
 *  WASTE SORTING ROBOTIC ARM — ARDUINO UNO
 *  4 Servos: Base + Shoulder + Elbow + Gripper
 *
 *  BOOT SEQUENCE:
 *    1. All motors go to 0° first
 *    2. Then move to PICKUP position (90°)
 *    3. Wait for waste
 *
 *  SORT SEQUENCE:
 *    1. Grip waste at pickup position (90°)
 *    2. Rotate base to correct bin
 *    3. Drop waste
 *    4. Return to pickup position (90°)
 *
 *  Base Bin Angles:
 *    PET  →   0°
 *    HDPE →  36°
 *    PVC  →  72°
 *    PICKUP→  90° (always picks from here)
 *    LDPE → 108°
 *    PP   → 144°
 *    PS   → 180°
 *
 *  Pins:
 *    PIN 9  → Base
 *    PIN 10 → Shoulder
 *    PIN 11 → Elbow
 *    PIN 6  → Gripper
 * ============================================================
 */

#include <Servo.h>

// ─── PINS ────────────────────────────────────────────────────
#define PIN_BASE      9
#define PIN_SHOULDER  10
#define PIN_ELBOW     11
#define PIN_GRIPPER   6

// ─── SERVO OBJECTS ───────────────────────────────────────────
Servo baseServo;
Servo shoulderServo;
Servo elbowServo;
Servo gripperServo;

// ─── BIN ANGLES ──────────────────────────────────────────────
#define BIN_PET    0
#define BIN_HDPE   36
#define BIN_PVC    72
#define BIN_LDPE   108
#define BIN_PP     144
#define BIN_PS     180

// ─── PICKUP POSITION ─────────────────────────────────────────
#define PICKUP_BASE      90   // base angle for pickup
#define PICKUP_SHOULDER  0   // shoulder down at pickup
#define PICKUP_ELBOW     50   // elbow extended at pickup

// ─── TRAVEL POSITION (safe for rotation) ─────────────────────
#define TRAVEL_SHOULDER  20    // shoulder up for safe rotation
#define TRAVEL_ELBOW     0    // elbow retracted for safe rotation

// ─── GRIPPER ─────────────────────────────────────────────────
#define GRIPPER_OPEN     0
#define GRIPPER_CLOSE    92

// ─── SPEED CONTROL ───────────────────────────────────────────
#define SPEED_BASE       25 //30
#define SPEED_SHOULDER   25 //15
#define SPEED_ELBOW      25 //15
#define SPEED_GRIPPER    25 //20

// ─── DELAY BETWEEN STEPS ─────────────────────────────────────
#define STEP_DELAY       800  // 5 seconds

// ─── TRACKED POSITIONS ───────────────────────────────────────
int posBase     = 0;
int posShoulder = 0;
int posElbow    = 0;
int posGripper  = 0;

// ─── SLOW MOVE ───────────────────────────────────────────────
void moveSlowly(Servo &srv, int &currentPos, int targetPos, int stepDelay = 15) {
  if (currentPos < targetPos) {
    for (int pos = currentPos; pos <= targetPos; pos++) {
      srv.write(pos);
      delay(stepDelay);
    }
  } else {
    for (int pos = currentPos; pos >= targetPos; pos--) {
      srv.write(pos);
      delay(stepDelay);
    }
  }
  currentPos = targetPos;
}

// ─── GO TO PICKUP POSITION ───────────────────────────────────
void goToPickup() {
  Serial.println("STATUS: Moving to pickup position");

  // Rotate base to 90°
  moveSlowly(baseServo, posBase, PICKUP_BASE, SPEED_BASE);
  delay(500);

  // Extend elbow
  moveSlowly(elbowServo, posElbow, PICKUP_ELBOW, SPEED_ELBOW);  //20 -> posShoulder
  delay(1000);

  moveSlowly(gripperServo, posGripper, GRIPPER_OPEN, SPEED_GRIPPER);

  moveSlowly(shoulderServo, posShoulder, PICKUP_SHOULDER, SPEED_SHOULDER+15);

  // Lower shoulder to pickup height
  //moveSlowly(shoulderServo, posShoulder, PICKUP_SHOULDER, SPEED_SHOULDER);
  //delay(1000);

  // Open gripper — ready to receive waste
  //moveSlowly(gripperServo, posGripper, GRIPPER_OPEN, SPEED_GRIPPER);

  Serial.println("STATUS: At pickup position — waiting for waste");
  Serial.println("READY");
}

// ─── SORT FUNCTION ───────────────────────────────────────────
void sortWaste(int binAngle) {

  // ── GRIP WASTE AT PICKUP POSITION ────────────────────────

  // Step 1 — Close gripper to grip waste
  Serial.println("STATUS: Step 1 — Gripping waste");
  moveSlowly(gripperServo, posGripper, GRIPPER_CLOSE, SPEED_GRIPPER);
  delay(STEP_DELAY);

  // ── LIFT TO TRAVEL POSITION ───────────────────────────────

  // Step 2 — Raise shoulder up for safe travel
  Serial.println("STATUS: Step 2 — Raising shoulder");
  moveSlowly(shoulderServo, posShoulder, TRAVEL_SHOULDER, SPEED_SHOULDER+15);
  Serial.println(posShoulder);
  delay(1000);

  // Step 3 — Retract elbow for safe rotation
  Serial.println("STATUS: Step 3 — Retracting elbow");
  moveSlowly(elbowServo, posElbow, TRAVEL_ELBOW, SPEED_ELBOW);
  delay(1000);

  // ── ROTATE TO BIN ─────────────────────────────────────────

  // Step 4 — Rotate base to correct bin
  Serial.print("STATUS: Step 4 — Rotating to bin at ");
  Serial.print(binAngle);
  Serial.println(" degrees");
  moveSlowly(baseServo, posBase, binAngle, SPEED_BASE);
  delay(STEP_DELAY);

  // ── DROP WASTE ────────────────────────────────────────────

  // Step 5 — Extend elbow over bin
  Serial.println("STATUS: Step 5 — Extending elbow over bin");
  moveSlowly(elbowServo, posElbow, PICKUP_ELBOW, SPEED_ELBOW);
  delay(1000);

  Serial.println("STATUS: Step 7 — Releasing waste");
  moveSlowly(gripperServo, posGripper, GRIPPER_OPEN, SPEED_GRIPPER);
  delay(STEP_DELAY);

  // Step 6 — Lower shoulder to drop height
  Serial.println("STATUS: Step 6 — Lowering to drop");
  moveSlowly(shoulderServo, posShoulder, PICKUP_SHOULDER, SPEED_SHOULDER+15);   //PICKUP
  delay(1000);

  // Step 7 — Open gripper to release
  //Serial.println("STATUS: Step 7 — Releasing waste");
  //moveSlowly(gripperServo, posGripper, GRIPPER_OPEN, SPEED_GRIPPER);
  //delay(STEP_DELAY);

  // ── RETURN TO PICKUP POSITION ─────────────────────────────

  // Step 8 — Raise shoulder
  Serial.println("STATUS: Step 8 — Raising shoulder");
  moveSlowly(shoulderServo, posShoulder, TRAVEL_SHOULDER, SPEED_SHOULDER+15);   
  delay(STEP_DELAY);

  // Step 9 — Retract elbow
  Serial.println("STATUS: Step 9 — Retracting elbow");
  moveSlowly(elbowServo, posElbow, TRAVEL_ELBOW, SPEED_ELBOW);
  delay(1000);

  // Step 10 — Rotate base back to pickup (90°)
  Serial.println("STATUS: Step 10 — Returning to pickup position");
  moveSlowly(baseServo, posBase, PICKUP_BASE, SPEED_BASE);
  delay(STEP_DELAY);

  // Step 11 — Extend elbow to pickup
  moveSlowly(elbowServo, posElbow, PICKUP_ELBOW, SPEED_ELBOW);
  delay(500);
  
  moveSlowly(gripperServo, posGripper, GRIPPER_OPEN, SPEED_GRIPPER);
  // Step 12 — Lower shoulder to pickup height
  moveSlowly(shoulderServo, posShoulder, PICKUP_SHOULDER, SPEED_SHOULDER);
  delay(500);

  // Step 13 — Open gripper ready for next waste
  //moveSlowly(gripperServo, posGripper, GRIPPER_OPEN, SPEED_GRIPPER);

  Serial.println("STATUS: Back at pickup position — ready");
  Serial.println("READY");
}

// ─── SETUP ───────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);

  baseServo.attach(PIN_BASE);
  shoulderServo.attach(PIN_SHOULDER);
  elbowServo.attach(PIN_ELBOW);
  gripperServo.attach(PIN_GRIPPER);

  // ── BOOT SEQUENCE ─────────────────────────────────────────
  // Step A — All motors to 0° first
  Serial.println("STATUS: Boot — All motors going to 0°");

  moveSlowly(gripperServo,  posGripper,  0, 25);  delay(800);
  moveSlowly(elbowServo,    posElbow,    0, 25);  delay(1000);
  moveSlowly(shoulderServo, posShoulder, 0, 25);  delay(1000);
  moveSlowly(baseServo,     posBase,     0, 25);  delay(800);

  Serial.println("STATUS: All at 0°");
  delay(1000);

  // Step B — Move to pickup position
  goToPickup();
}

// ─── LOOP ────────────────────────────────────────────────────
void loop() {
  if (Serial.available()) {
    String waste = Serial.readStringUntil('\n');
    waste.trim();
    waste.toLowerCase();

    Serial.print("STATUS: Received → ");
    Serial.println(waste);

    if      (waste == "pet")  sortWaste(BIN_PET);
    else if (waste == "hdpe") sortWaste(BIN_HDPE);
    else if (waste == "pvc")  sortWaste(BIN_PVC);
    else if (waste == "ldpe") sortWaste(BIN_LDPE);
    else if (waste == "pp")   sortWaste(BIN_PP);
    else if (waste == "ps")   sortWaste(BIN_PS);
    else {
      Serial.println("STATUS: Unknown class ignored");
      Serial.println("READY");
    }
  }
}