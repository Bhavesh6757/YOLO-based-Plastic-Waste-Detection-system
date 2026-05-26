// ============================================================
//   4-DOF Robotic Arm — YOLOv5 Plastic Waste Sorter
//   Servos : Base(3) | Shoulder(5) | Elbow(6) | Gripper(9)
//   Baud   : 115200  (matches Python)
//   Input  : "pet" | "hdpe" | "pvc" | "ldpe" | "pp" | "ps"
//   Output : "READY" (sent at boot + after every sequence)
// ============================================================

#include <Servo.h>

// ---------- Servo Objects ----------
Servo baseServo;
Servo shoulderServo;
Servo elbowServo;
Servo gripperServo;

// ---------- Pin Definitions ----------
#define BASE_PIN     3
#define SHOULDER_PIN 5
#define ELBOW_PIN    6
#define GRIPPER_PIN  9

// ---------- Angle Tracking ----------
int currentBase     = 90;
int currentShoulder = 90;
int currentElbow    = 90;
int currentGripper  = 90;

// ---------- Assigned base angle (set per waste type) ----------
int assignedBaseAngle = 90;

// ============================================================
//  PLASTIC TYPE → BASE ANGLE MAP
//  Spread 6 bins evenly across 30°–150° servo range
//  pet=30 | hdpe=54 | pvc=78 | ldpe=102 | pp=126 | ps=150
// ============================================================
int getBaseAngle(String wType) {
  if (wType == "pet")  return 30;
  if (wType == "hdpe") return 54;
  if (wType == "pvc")  return 78;
  if (wType == "ldpe") return 102;
  if (wType == "pp")   return 126;
  if (wType == "ps")   return 150;
  return -1;   // unknown
}

// ============================================================
//   HELPER — print all four angles in one line
// ============================================================
void printAngles(int b, int sh, int el, int gr) {
  Serial.print("    Angles => Base: "); Serial.print(b);
  Serial.print(" | Shoulder: ");        Serial.print(sh);
  Serial.print(" | Elbow: ");           Serial.print(el);
  Serial.print(" | Gripper: ");         Serial.println(gr);
}

// ============================================================
//   HELPER — write all four servos + update tracking vars
// ============================================================
// void smoothMove(int b, int sh, int el, int gr) {
//   baseServo.write(b);
//   shoulderServo.write(sh);
//   elbowServo.write(el);
//   gripperServo.write(gr);
//   currentBase     = b;
//   currentShoulder = sh;
//   currentElbow    = el;
//   currentGripper  = gr;
// }
void smoothMove(int targetB, int targetS, int targetE) {

  while (currentBase != targetB ||
         currentShoulder != targetS ||
         currentElbow != targetE) {

    if (currentBase < targetB) currentBase++;
    else if (currentBase > targetB) currentBase--;

    if (currentShoulder < targetS) currentShoulder++;
    else if (currentShoulder > targetS) currentShoulder--;

    if (currentElbow < targetE) currentElbow++;
    else if (currentElbow > targetE) currentElbow--;

    baseServo.write(currentBase);
    shoulderServo.write(currentShoulder);
    elbowServo.write(currentElbow);

    delay(20);
  }
}

// ============================================================
//   STEP FUNCTIONS
// ============================================================

void moveToPickup() {
  Serial.println("  ----------------------------------------");
  Serial.println("  [STEP 1] Pickup Position");
  Serial.print("  Before "); printAngles(currentBase,currentShoulder,currentElbow,currentGripper);
  smoothMove(90, 90, 90);  // gripper OPEN
  gripperServo.write(90);
  delay(800);
  Serial.print("  After  "); printAngles(currentBase,currentShoulder,currentElbow,currentGripper);
  Serial.println("  >> Step 1 Completed");
}

void grabWaste() {
  Serial.println("  ----------------------------------------");
  Serial.println("  [STEP 2] Grab Waste");
  Serial.print("  Before "); printAngles(currentBase,currentShoulder,currentElbow,currentGripper);
  smoothMove(90, 10, 100);
  for (int g = 90; g >= 20; g--) {
  gripperServo.write(160);
  currentGripper = 160;
  delay(15);
}
currentGripper = 20;
  delay(800);
  Serial.print("  After  "); printAngles(currentBase,currentShoulder,currentElbow,currentGripper);
  Serial.println("  >> Step 2 Completed");
  delay(800);
}

void liftUp() {
  Serial.println("  ----------------------------------------");
  Serial.println("  [STEP 3] Lift Up");
  Serial.print("  Before "); printAngles(currentBase,currentShoulder,currentElbow,currentGripper);
  smoothMove(90, 60, 100);
  delay(800);
  Serial.print("  After  "); printAngles(currentBase,currentShoulder,currentElbow,currentGripper);
  Serial.println("  >> Step 3 Completed");
  delay(800);
}

void rotateBase() {
  Serial.println("  ----------------------------------------");
  Serial.println("  [STEP 4] Rotate Base");
  Serial.print("  Target base angle: "); Serial.println(assignedBaseAngle);
  Serial.print("  Before "); printAngles(currentBase,currentShoulder,currentElbow,currentGripper);
  smoothMove(assignedBaseAngle, currentShoulder, currentElbow);
  delay(1000);
  Serial.print("  After  "); printAngles(currentBase,currentShoulder,currentElbow,currentGripper);
  Serial.println("  >> Step 4 Completed");
  delay(800);
}

void dropWaste() {
  Serial.println("  ----------------------------------------");
  Serial.println("  [STEP 5] Drop Position");
  Serial.print("  Before "); printAngles(currentBase,currentShoulder,currentElbow,currentGripper);
  smoothMove(assignedBaseAngle, 30, 70);
  for (int g = 20; g <= 90; g++) {
  gripperServo.write(90);
  currentGripper = 90;
  delay(15);
}
currentGripper = 90;
  delay(800);
  Serial.print("  After  "); printAngles(currentBase,currentShoulder,currentElbow,currentGripper);
  Serial.println("  >> Step 5 Completed");
  delay(800);
}

void liftAfterDrop() {
  Serial.println("  ----------------------------------------");
  Serial.println("  [STEP 6] Lift After Drop");
  Serial.print("  Before "); printAngles(currentBase,currentShoulder,currentElbow,currentGripper);
  smoothMove(assignedBaseAngle, 60, 100);
  delay(800);
  Serial.print("  After  "); printAngles(currentBase,currentShoulder,currentElbow,currentGripper);
  Serial.println("  >> Step 6 Completed");
  delay(800);
}

void returnToPickup() {
  Serial.println("  ----------------------------------------");
  Serial.println("  [STEP 7] Return to Pickup");
  Serial.print("  Before "); printAngles(currentBase,currentShoulder,currentElbow,currentGripper);
  smoothMove(90, 90, 90);  // gripper OPEN
  gripperServo.write(90);
  delay(800);
  Serial.print("  After  "); printAngles(currentBase,currentShoulder,currentElbow,currentGripper);
  Serial.println("  >> Step 7 Completed");
  delay(800);
}

// ============================================================
//   SETUP
// ============================================================
void setup() {
  // *** MUST match Python: serial.Serial('COM6', 115200) ***
  Serial.begin(115200);

  baseServo.attach(BASE_PIN);
  shoulderServo.attach(SHOULDER_PIN);
  elbowServo.attach(ELBOW_PIN);
  gripperServo.attach(GRIPPER_PIN);

  Serial.println("========================================");
  Serial.println("  4-DOF Robotic Arm  |  Plastic Sorter ");
  Serial.println("========================================");
  Serial.println("  System Initialized");
  Serial.println("  Moving to Pickup Position...");

  smoothMove(90, 90, 90);  // gripper OPEN
  gripperServo.write(90); 
  delay(1000);

  printAngles(currentBase, currentShoulder, currentElbow, currentGripper);
  Serial.println("========================================");

  // *** Handshake: tell Python the arm is ready ***
  Serial.println("READY");
}

// ============================================================
//   LOOP
// ============================================================
void loop() {
  if (Serial.available() > 0) {

    String wasteType = Serial.readStringUntil('\n');
    wasteType.trim();
    wasteType.toLowerCase();

    Serial.println("========================================");
    Serial.print("  Received Waste Type: ");
    Serial.println(wasteType);

    // Validate and assign base angle
    assignedBaseAngle = getBaseAngle(wasteType);

    if (assignedBaseAngle == -1) {
      Serial.println("  [ERROR] Unknown type. Expected: pet|hdpe|pvc|ldpe|pp|ps");
      Serial.println("========================================");
      Serial.println("READY");   // still signal ready so Python isn't stuck
      return;
    }

    Serial.print("  Assigned Base Angle: ");
    Serial.print(assignedBaseAngle);
    Serial.println(" deg");
    Serial.println("  Starting Motion Sequence...");
    Serial.println("========================================");

    // ---- Full 7-step sequence ----
    moveToPickup();
    grabWaste();
    liftUp();
    rotateBase();
    dropWaste();
    liftAfterDrop();
    returnToPickup();

    // ---- Done ----
    Serial.println("========================================");
    Serial.println("  SEQUENCE COMPLETE");
    Serial.println("  Arm at Pickup. Awaiting next object.");
    Serial.println("========================================");

    // *** Handshake: signal Python it can send the next detection ***
    Serial.println("READY");
  }
}
