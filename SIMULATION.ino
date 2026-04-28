#define PIR_PIN     13
#define IR_PIN       4
#define BUZZER_PIN  14
#define RED_LED      2
#define GREEN_LED    5

bool personDetected     = false;
bool violationTriggered = false;
unsigned long entryTime = 0;
int totalEntries        = 0;
int compliantCount      = 0;

void setup() {
  Serial.begin(115200);

  pinMode(PIR_PIN,    INPUT);
  pinMode(IR_PIN,     INPUT_PULLUP);  // ← PULLUP for Wokwi IR receiver
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(RED_LED,    OUTPUT);
  pinMode(GREEN_LED,  OUTPUT);

  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(RED_LED,    LOW);
  digitalWrite(GREEN_LED,  LOW);

  Serial.println("===========================================");
  Serial.println("  HAI Tracker — Wokwi Simulation Ready");
  Serial.println("  HOW TO TEST:");
  Serial.println("  1. Click PIR sensor to trigger entry");
  Serial.println("  2. Press IR remote within 30s = COMPLIANT");
  Serial.println("  3. Don't press = VIOLATION (buzzer fires)");
  Serial.println("  4. Press IR remote after violation = CLEARED");
  Serial.println("===========================================");
}

void loop() {

  // ── Step 1: PIR triggered (click the PIR in Wokwi) ──────
  if (digitalRead(PIR_PIN) == HIGH
      && !personDetected
      && !violationTriggered) {

    personDetected = true;
    entryTime = millis();
    totalEntries++;

    Serial.println("");
    Serial.println("⚠️  Person entered patient zone!");
    Serial.println("    Sanitize within 30 seconds...");
  }

  // ── Step 2: Compliance window active ────────────────────
  if (personDetected && !violationTriggered) {

    unsigned long elapsed = millis() - entryTime;

    // Show countdown every 5 seconds
    if (elapsed % 5000 < 50) {
      int remaining = (30000 - elapsed) / 1000;
      if (remaining >= 0) {
        Serial.print("    ⏱ Time remaining: ");
        Serial.print(remaining);
        Serial.println("s");
      }
    }

    // IR remote pressed within 30s → COMPLIANT ✅
    // In Wokwi: press any button on the IR remote
    if (digitalRead(IR_PIN) == LOW) {

      compliantCount++;
      digitalWrite(GREEN_LED,  HIGH);
      digitalWrite(RED_LED,    LOW);
      digitalWrite(BUZZER_PIN, LOW);

      Serial.println("✅ Sanitizer used — COMPLIANT!");
      printStats();

      delay(2000);
      digitalWrite(GREEN_LED, LOW);
      personDetected = false;   // cycle complete
    }

    // 30 seconds passed → VIOLATION ❌
    else if (elapsed >= 30000) {

      violationTriggered = true;
      digitalWrite(RED_LED,    HIGH);
      digitalWrite(BUZZER_PIN, HIGH);

      Serial.println("❌ TIME UP — VIOLATION!");
      Serial.println("   Press IR remote to clear alert...");
      printStats();
    }
  }

  // ── Step 3: Violation — waiting for IR to clear ─────────
  // In Wokwi: press any button on IR remote to simulate
  // sanitizer use and clear the alert
  if (violationTriggered) {

    if (digitalRead(IR_PIN) == LOW) {

      digitalWrite(BUZZER_PIN, LOW);   // stop buzzer
      digitalWrite(RED_LED,    LOW);   // turn off red
      digitalWrite(GREEN_LED,  HIGH);  // turn on green

      Serial.println("✅ Sanitizer used after violation!");
      Serial.println("   Alert cleared — ready for next cycle.");

      delay(2000);
      digitalWrite(GREEN_LED, LOW);

      // Only NOW reset for next cycle
      violationTriggered = false;
      personDetected     = false;
    }
  }
}

// ── Print live stats to Serial Monitor ──────────────────────
void printStats() {
  float rate = (totalEntries > 0)
               ? (compliantCount * 100.0 / totalEntries)
               : 0.0;

  Serial.println("-------------------------------------------");
  Serial.print("   Total Entries  : "); Serial.println(totalEntries);
  Serial.print("   Compliant      : "); Serial.println(compliantCount);
  Serial.print("   Violations     : "); Serial.println(totalEntries - compliantCount);
  Serial.print("   Compliance %   : "); Serial.print(rate, 1); Serial.println("%");
  Serial.println("-------------------------------------------");
}