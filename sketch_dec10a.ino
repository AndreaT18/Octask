#include <Adafruit_NeoPixel.h>

/*
 * OCTASK - Versione Finale (Speriamo)
 * Progetto Interaction Design
 * * Note: Finalmente l'animazione di errore non blinka a caso ma respira.
 * Se tocchi i delay esplode tutto, occhio.
 */

// =================================================================================
//  SETTINGS & COSTANTI
// =================================================================================

// --- Luci ---
#define MASTER_BRIGHTNESS 100   
#define PIGI_BRIGHTNESS   100   
#define EDIT_BRIGHTNESS   220   
#define LOGIN_MAX_BRIGHT  100   

// --- Colori ---
#define COL_EDIT_R   255
#define COL_EDIT_G   255
#define COL_EDIT_B   255

#define COL_TASK_R   255
#define COL_TASK_G   255
#define COL_TASK_B   255

#define COL_ERR_R    200
#define COL_ERR_G    0
#define COL_ERR_B    0

#define COL_WIPE_R   255
#define COL_WIPE_G   255
#define COL_WIPE_B   255

#define COL_LOGIN_R  255
#define COL_LOGIN_G  255
#define COL_LOGIN_B  255

// --- Palette Pigi (Tentacoli Giù = Caldo / Su = Freddo) ---
#define WARM_A_R     148 
#define WARM_A_G     0
#define WARM_A_B     211 // Viola
#define WARM_B_R     255 
#define WARM_B_G     0
#define WARM_B_B     0   // Rosso
#define WARM_C_R     255 
#define WARM_C_G     215
#define WARM_C_B     0   // Oro

#define COOL_A_R     0   
#define COOL_A_G     0
#define COOL_A_B     139 // Blu
#define COOL_B_R     0   
#define COOL_B_G     255
#define COOL_B_B     255 // Ciano
#define COOL_C_R     0   
#define COOL_C_G     255
#define COOL_C_B     127 // Verde

// --- Velocità Animazioni (Non toccare se non sai cosa fai) ---
#define SPEED_FADE    8    
#define SPEED_BULLET  60   
#define SPEED_ERROR   40   
#define SPEED_LAVA    30   
#define SPEED_EDIT    150.0 
#define SPEED_EDIT_WAVE 150.0 
#define SPEED_LOGIN   600.0 
#define SPEED_SOFT_OFF 3     
#define SPEED_SOFT_IN  5 
#define SPEED_SPARKLE 200.0   
#define SMOOTH_START_MS 1500.0

#define SPEED_WARNING_BREATH 300.0 // Velocità respiro errore

// Parametri animazione "Liquida" (Intro Focus)
#define SCAN_SPEED_STEP 0.25  
#define SCAN_TAIL_LEN   4.0   

// Parametri animazione "Task Done" (Risucchio)
#define DRAIN_SPEED_STEP 0.06 // 0.06 è perfetto, non cambiare

// =================================================================================
//  HARDWARE & PIN
// =================================================================================

#define NUM_TENTACOLI  4
#define NUM_LEDS       8   
#define DEBOUNCE_MS    50  

enum SystemMode { MODE_IDLE, MODE_WORK, MODE_PIGI, MODE_LOGIN, MODE_ERROR_FIX };
SystemMode currentMode = MODE_IDLE; 

int editModeTarget = 0; 
int errorTargetIdx = -1; 
unsigned long animStartTime = 0; 

// Pinout
const int ledPins[NUM_TENTACOLI]    = {7, 5, 6, 4};
const int switchPins[NUM_TENTACOLI] = {11, 9, 10, 8};

Adafruit_NeoPixel strisce[NUM_TENTACOLI] = {
  Adafruit_NeoPixel(NUM_LEDS, ledPins[0], NEO_GRB + NEO_KHZ800),
  Adafruit_NeoPixel(NUM_LEDS, ledPins[1], NEO_GRB + NEO_KHZ800),
  Adafruit_NeoPixel(NUM_LEDS, ledPins[2], NEO_GRB + NEO_KHZ800),
  Adafruit_NeoPixel(NUM_LEDS, ledPins[3], NEO_GRB + NEO_KHZ800)
};

// Variabili globali
int switchState[NUM_TENTACOLI];             
int lastSwitchState[NUM_TENTACOLI];         
unsigned long lastDebounceTime[NUM_TENTACOLI]; 
int statoLedAttuali[NUM_TENTACOLI] = {0, 0, 0, 0}; 

uint32_t colorActive, colorError, colorWipe, colorEdit, colorLogin;

// Funzioni
void enterIdleMode();
void enterWorkMode();
void enterPigiMode();
void enterLoginMode(); 
void enterErrorFixMode(int idx); 
void gestioneSerialInput();
void gestioneSwitch();
void runLavaLamp(); 
void runEditWaveAnimation(int targetIdx); 
void runLoginAnimation(); 
void runFocusSparkle(); 
void runErrorFixAnimation(); 
void aggiornaTransizione(int idx, int target, uint32_t col);
void animaErrore(int idx);
void animaReset(int idx);
void animaTaskDone(int idx);
void cambiaLuminosita(uint8_t val);
void globalFadeOut(); 
void stripFadeOut(int idx); 
uint32_t applicaLuminosita(uint32_t c, uint8_t l);
float getFadeInFactor();

// =================================================================================
//  SETUP
// =================================================================================
void setup() {
  Serial.begin(9600);
  Serial.println("--- OCTASK START ---");

  colorActive = strisce[0].Color(COL_TASK_R, COL_TASK_G, COL_TASK_B);
  colorError  = strisce[0].Color(COL_ERR_R,  COL_ERR_G,  COL_ERR_B);
  colorWipe   = strisce[0].Color(COL_WIPE_R, COL_WIPE_G, COL_WIPE_B);
  colorEdit   = strisce[0].Color(COL_EDIT_R, COL_EDIT_G, COL_EDIT_B); 
  colorLogin  = strisce[0].Color(COL_LOGIN_R, COL_LOGIN_G, COL_LOGIN_B);

  for (int i = 0; i < NUM_TENTACOLI; i++) {
    strisce[i].begin();
    strisce[i].clear();
    strisce[i].show(); 

    pinMode(switchPins[i], INPUT_PULLUP);
    switchState[i] = digitalRead(switchPins[i]);
    lastSwitchState[i] = switchState[i];
    lastDebounceTime[i] = 0;
  }
  
  // Parte subito in Login così sembra vivo
  enterLoginMode();
}

// =================================================================================
//  LOOP
// =================================================================================
void loop() {
  gestioneSerialInput();

  switch (currentMode) {
    case MODE_IDLE:
      if (editModeTarget > 0) {
        runEditWaveAnimation(editModeTarget - 1);
      }
      break;

    case MODE_WORK:
      gestioneSwitch(); 
      runFocusSparkle(); // Effetto sbrilluccico se finisci il task
      break;

    case MODE_PIGI:
      runLavaLamp();
      break;

    case MODE_LOGIN:
      runLoginAnimation();
      break;

    case MODE_ERROR_FIX:
      runErrorFixAnimation(); // Bloccato finché non tiri su la levetta
      break;
  }
}

// =================================================================================
//  GESTIONE STATI
// =================================================================================

void enterIdleMode() {
  if (currentMode != MODE_IDLE) {
    globalFadeOut(); 
  }
  
  currentMode = MODE_IDLE;
  editModeTarget = 0; 
  errorTargetIdx = -1;
  cambiaLuminosita(0); 
  for(int i=0; i<NUM_TENTACOLI; i++) { strisce[i].clear(); strisce[i].show(); }
  Serial.println("STATE: IDLE.");
}

void enterWorkMode() {
  currentMode = MODE_WORK;
  editModeTarget = 0; 
  cambiaLuminosita(MASTER_BRIGHTNESS);
  for(int i=0; i<NUM_TENTACOLI; i++) statoLedAttuali[i] = 0; 
  
  Serial.println("STATE: FOCUS WORK.");

  // Animazione KITT (quella fluida figa)
  // Salita
  for(float pos = -SCAN_TAIL_LEN; pos < NUM_LEDS + SCAN_TAIL_LEN; pos += SCAN_SPEED_STEP) {
    for(int s = 0; s < NUM_TENTACOLI; s++) {
      strisce[s].clear();
      for(int i = 0; i < NUM_LEDS; i++) {
        float dist = fabs((float)i - pos);
        if(dist < SCAN_TAIL_LEN) {
          float factor = 1.0 - (dist / SCAN_TAIL_LEN); 
          factor = factor * factor; 
          uint8_t brightness = (uint8_t)(255.0 * factor);
          strisce[s].setPixelColor(i, applicaLuminosita(colorWipe, brightness));
        }
      }
      strisce[s].show();
    }
    delay(5); 
  }

  // Discesa
  for(float pos = NUM_LEDS + SCAN_TAIL_LEN; pos > -SCAN_TAIL_LEN; pos -= SCAN_SPEED_STEP) {
    for(int s = 0; s < NUM_TENTACOLI; s++) {
      strisce[s].clear();
      for(int i = 0; i < NUM_LEDS; i++) {
        float dist = fabs((float)i - pos);
        if(dist < SCAN_TAIL_LEN) {
          float factor = 1.0 - (dist / SCAN_TAIL_LEN); 
          factor = factor * factor; 
          uint8_t brightness = (uint8_t)(255.0 * factor);
          strisce[s].setPixelColor(i, applicaLuminosita(colorWipe, brightness));
        }
      }
      strisce[s].show();
    }
    delay(5);
  }

  for(int s = 0; s < NUM_TENTACOLI; s++) { strisce[s].clear(); strisce[s].show(); }
  Serial.println("SYSTEM READY.");
}

void enterPigiMode() {
  currentMode = MODE_PIGI;
  editModeTarget = 0;
  cambiaLuminosita(PIGI_BRIGHTNESS);
  animStartTime = millis(); 
  Serial.println("STATE: PIGI RELAX.");
}

void enterLoginMode() {
  currentMode = MODE_LOGIN;
  editModeTarget = 0;
  Serial.println("STATE: LOGIN.");

  // Fade-in manuale
  cambiaLuminosita(0);
  for(int b = 0; b <= 255; b += 4) {
    cambiaLuminosita(b);
    runLoginAnimation(); 
    delay(SPEED_SOFT_IN);
  }
}

// Modalità panico se lasci il tentacolo giù
void enterErrorFixMode(int idx) {
  currentMode = MODE_ERROR_FIX;
  errorTargetIdx = idx;
  
  // Spegne tutto tranne quello incriminato
  for(int i=0; i<NUM_TENTACOLI; i++) {
    if(i != idx) { strisce[i].clear(); strisce[i].show(); }
  }
  
  strisce[idx].setBrightness(255);
  Serial.println("WARNING");
}

// =================================================================================
//  SERIALE E COMANDI
// =================================================================================
void gestioneSerialInput() {
  if (Serial.available() > 0) {
    String comando = Serial.readStringUntil('\n');
    comando.trim(); 
    comando.toUpperCase(); 

    // Se sono bloccato in errore, esco solo se l'app manda comandi forti
    if (currentMode == MODE_ERROR_FIX) {
        if (comando == "FOCUSOFF" || comando == "LOGINOFF" || comando == "PIGIOFF") {
             Serial.println("WARNING_OFF"); 
             enterIdleMode();
             return;
        }
    }

    if (comando == "FOCUS") { enterWorkMode(); return; }
    if (comando == "FOCUSOFF") { enterIdleMode(); return; } 
    if (comando == "PIGI") { enterPigiMode(); return; }
    if (comando == "PIGIOFF") { enterIdleMode(); return; }  
    if (comando == "LOGIN") { enterLoginMode(); return; }
    if (comando == "LOGINOFF") { enterIdleMode(); return; } 

    // Comandi Edit (solo in IDLE)
    if (currentMode == MODE_IDLE) {
      if (comando.startsWith("EDIT:")) {
        String param = comando.substring(5); 
        
        if (param == "OFF") {
          if (editModeTarget > 0) {
            int idx = editModeTarget - 1;
            // Check fisico della levetta
            bool isTentacoloAlzato = (digitalRead(switchPins[idx]) == HIGH);

            if (!isTentacoloAlzato) {
              enterErrorFixMode(idx); // Ops, l'hai lasciato giù
            } else {
              stripFadeOut(idx);
              editModeTarget = 0;
              Serial.println("EDIT MODE: OFF");
            }
          } else {
             Serial.println("EDIT MODE: OFF");
          }
        } 
        else {
          int target = param.toInt();
          if (target >= 1 && target <= 4) {
            if (editModeTarget > 0) stripFadeOut(editModeTarget - 1);
            else { for(int i=0; i<NUM_TENTACOLI; i++) { strisce[i].clear(); strisce[i].show(); } }

            editModeTarget = target;
            animStartTime = millis(); 
            strisce[target-1].setBrightness(EDIT_BRIGHTNESS);
            Serial.print("EDIT MODE: T"); Serial.println(target);
          }
        }
      }
      return;
    }

    // Comandi da Protopie
    if (currentMode == MODE_WORK) {
      if (comando.startsWith("T") && comando.indexOf(':') > 0) {
        int separatore = comando.indexOf(':');
        int tentIndex = comando.substring(1, separatore).toInt(); 
        int numLedAccesi = comando.substring(separatore + 1).toInt(); 

        if (tentIndex >= 1 && tentIndex <= 4 && numLedAccesi >= 0 && numLedAccesi <= 8) {
          int idx = tentIndex - 1; 
          bool isTentacoloAlzato = (digitalRead(switchPins[idx]) == HIGH);

          if (isTentacoloAlzato) {
            aggiornaTransizione(idx, numLedAccesi, colorActive);
            Serial.print("TASK T"); Serial.print(idx + 1); Serial.print(" -> "); Serial.println(numLedAccesi);
          } else {
            Serial.print("ERROR T"); Serial.print(tentIndex); Serial.println(" DOWN.");
            animaErrore(idx);
          }
        } 
      }
    }
  }
}

// =================================================================================
//  HELPER E ROBE VARIE
// =================================================================================

float getFadeInFactor() {
  unsigned long elapsed = millis() - animStartTime;
  if (elapsed >= SMOOTH_START_MS) return 1.0;
  return (float)elapsed / SMOOTH_START_MS;
}

void globalFadeOut() {
  for (int b = 200; b >= 0; b -= 4) {
    cambiaLuminosita(b);
    for(int i=0; i<NUM_TENTACOLI; i++) strisce[i].show();
    delay(SPEED_SOFT_OFF);
  }
}

void stripFadeOut(int idx) {
  int startB = strisce[idx].getBrightness(); 
  for (int b = startB; b >= 0; b -= 5) {
    strisce[idx].setBrightness(b);
    strisce[idx].show();
    delay(SPEED_SOFT_OFF);
  }
  strisce[idx].clear(); 
  strisce[idx].show();
}

// =================================================================================
//  ANIMAZIONI
// =================================================================================

// Loop finché non risolvi l'errore fisico
void runErrorFixAnimation() {
  // Check levetta
  if (digitalRead(switchPins[errorTargetIdx]) == HIGH) {
    Serial.println("WARNING_OFF");
    stripFadeOut(errorTargetIdx); // Uscita smooth
    editModeTarget = 0; 
    enterIdleMode();
    return;
  }

  // Respiro rosso ansioso
  unsigned long now = millis();
  float wave = sin(now / SPEED_WARNING_BREATH);
  int brightness = (int)((wave + 1.0) * 127.5); 
  
  uint32_t pulsedError = applicaLuminosita(colorError, brightness);
  
  for(int i=0; i<NUM_LEDS; i++) {
    strisce[errorTargetIdx].setPixelColor(i, pulsedError);
  }
  strisce[errorTargetIdx].show();
}

void runLoginAnimation() {
  unsigned long now = millis();
  for (int t = 0; t < NUM_TENTACOLI; t++) {
    for (int i = 0; i < NUM_LEDS; i++) {
      float wave = sin((now / SPEED_LOGIN) + (i * 0.5) + (t * 0.2));
      int brightness = (int)((wave + 1.0) / 2.0 * LOGIN_MAX_BRIGHT);
      int minLight = 5; 
      if (brightness < minLight) brightness = minLight;
      strisce[t].setPixelColor(i, applicaLuminosita(colorLogin, brightness));
    }
    strisce[t].show();
  }
}

void runLavaLamp() {
  float speedFactor = (float)SPEED_LAVA * 50.0;
  unsigned long now = millis();
  float fade = getFadeInFactor(); 

  for (int t = 0; t < NUM_TENTACOLI; t++) {
    bool isUp = (digitalRead(switchPins[t]) == HIGH);
    uint8_t tA_R, tA_G, tA_B;
    uint8_t tB_R, tB_G, tB_B;
    uint8_t tC_R, tC_G, tC_B;

    if (isUp) {
      tA_R = COOL_A_R; tA_G = COOL_A_G; tA_B = COOL_A_B;
      tB_R = COOL_B_R; tB_G = COOL_B_G; tB_B = COOL_B_B;
      tC_R = COOL_C_R; tC_G = COOL_C_G; tC_B = COOL_C_B;
    } else {
      tA_R = WARM_A_R; tA_G = WARM_A_G; tA_B = WARM_A_B;
      tB_R = WARM_B_R; tB_G = WARM_B_G; tB_B = WARM_B_B;
      tC_R = WARM_C_R; tC_G = WARM_C_G; tC_B = WARM_C_B;
    }

    for (int i = 0; i < NUM_LEDS; i++) {
      float wave = sin((now / speedFactor) + (t * 0.5) + (i * 0.2)); 
      uint8_t r, g, b;
      if (wave < 0) {
        float blend = 1.0 + wave; 
        r = tA_R + (int)((tB_R - tA_R) * blend);
        g = tA_G + (int)((tB_G - tA_G) * blend);
        b = tA_B + (int)((tB_B - tA_B) * blend);
      } else {
        float blend = wave;
        r = tB_R + (int)((tC_R - tB_R) * blend);
        g = tB_G + (int)((tC_G - tB_G) * blend);
        b = tB_B + (int)((tC_B - tB_B) * blend);
      }
      
      r = (uint8_t)(r * fade);
      g = (uint8_t)(g * fade);
      b = (uint8_t)(b * fade);

      strisce[t].setPixelColor(i, r, g, b);
    }
    strisce[t].show();
  }
  delay(20);
}

void runEditWaveAnimation(int targetIdx) {
  unsigned long now = millis();
  float fade = getFadeInFactor(); 
  
  for (int i = 0; i < NUM_LEDS; i++) {
    float wavePhase = (now / SPEED_EDIT_WAVE) - (i * 0.7);
    float wave = sin(wavePhase);
    int brightness = (int)((wave + 1.0) / 2.0 * EDIT_BRIGHTNESS);
    int minLight = 30; 
    if (brightness < minLight) brightness = minLight;
    
    brightness = (int)(brightness * fade);

    strisce[targetIdx].setPixelColor(i, applicaLuminosita(colorEdit, brightness));
  }
  strisce[targetIdx].show();
}

void runFocusSparkle() {
  unsigned long now = millis();
  bool needUpdate = false;

  for(int t=0; t<NUM_TENTACOLI; t++) {
    if(statoLedAttuali[t] == 8) {
      needUpdate = true;
      for(int i=0; i<NUM_LEDS; i++) {
        float wave = sin((now / SPEED_SPARKLE) + (i * 1.5));
        int minB = 120;
        int maxB = 255;
        int val = minB + (int)((wave + 1.0) / 2.0 * (maxB - minB));
        strisce[t].setPixelColor(i, applicaLuminosita(colorActive, val));
      }
      strisce[t].show();
    }
  }
  
  if(needUpdate) delay(15);
}

// Qui succede la magia degli switch
void gestioneSwitch() {
  for (int i = 0; i < NUM_TENTACOLI; i++) {
    int reading = digitalRead(switchPins[i]);
    if (reading != lastSwitchState[i]) lastDebounceTime[i] = millis();

    if ((millis() - lastDebounceTime[i]) > DEBOUNCE_MS) {
      if (reading != switchState[i]) {
        switchState[i] = reading;
        if (switchState[i] == LOW) {
          Serial.print("TASK_"); Serial.print(i + 1); Serial.println("_DONE");
          animaTaskDone(i);
          statoLedAttuali[i] = 0; 
        } else {
          Serial.print("TASK_"); Serial.print(i + 1); Serial.println("_RESET");
          statoLedAttuali[i] = 0;
          animaReset(i);
        }
      }
    }
    lastSwitchState[i] = reading;
  }
}

void cambiaLuminosita(uint8_t valore) {
  for(int i=0; i<NUM_TENTACOLI; i++) strisce[i].setBrightness(valore);
}

uint32_t applicaLuminosita(uint32_t colore, uint8_t fattore) {
  uint8_t r = (uint8_t)(colore >> 16);
  uint8_t g = (uint8_t)(colore >>  8);
  uint8_t b = (uint8_t)colore;
  r = (r * fattore) / 255;
  g = (g * fattore) / 255;
  b = (b * fattore) / 255;
  return strisce[0].Color(r, g, b);
}

// Fade in/out dei led task (barra di caricamento)
void aggiornaTransizione(int idx, int targetLed, uint32_t coloreBase) {
  int currentLed = statoLedAttuali[idx]; 
  if (targetLed > currentLed) { 
    for (int i = 0; i < currentLed; i++) strisce[idx].setPixelColor(i, coloreBase);
    for (int b = 0; b <= 255; b += 10) { 
      uint32_t coloreFade = applicaLuminosita(coloreBase, b);
      for (int i = currentLed; i < targetLed; i++) strisce[idx].setPixelColor(i, coloreFade);
      strisce[idx].show();
      delay(SPEED_FADE);
    }
  } else if (targetLed < currentLed) { 
    for (int i = 0; i < targetLed; i++) strisce[idx].setPixelColor(i, coloreBase);
    for (int b = 255; b >= 0; b -= 10) {
      uint32_t coloreFade = applicaLuminosita(coloreBase, b);
      for (int i = targetLed; i < currentLed; i++) strisce[idx].setPixelColor(i, coloreFade);
      strisce[idx].show();
      delay(SPEED_FADE);
    }
    for (int i = targetLed; i < currentLed; i++) strisce[idx].setPixelColor(i, 0, 0, 0);
    strisce[idx].show();
  }
  statoLedAttuali[idx] = targetLed;
}

// Errore generico (doppio flash rosso)
void animaErrore(int indexStr) {
  for (int k = 0; k < 2; k++) {
    for(int b=0; b<=200; b+=15) {
      uint32_t c = applicaLuminosita(colorError, b);
      for(int i=0; i<NUM_LEDS; i++) strisce[indexStr].setPixelColor(i, c);
      strisce[indexStr].show();
      delay(SPEED_ERROR); 
    }
    for(int b=200; b>=0; b-=15) {
      uint32_t c = applicaLuminosita(colorError, b);
      for(int i=0; i<NUM_LEDS; i++) strisce[indexStr].setPixelColor(i, c);
      strisce[indexStr].show();
      delay(SPEED_ERROR); 
    }
  }
  strisce[indexStr].clear(); strisce[indexStr].show();
}

// Reset task
void animaReset(int indexStr) {
  for(int i = 0; i < NUM_LEDS + 3; i++) {
    strisce[indexStr].clear();
    if(i < NUM_LEDS) strisce[indexStr].setPixelColor(i, colorWipe);
    if(i - 1 >= 0 && i - 1 < NUM_LEDS) strisce[indexStr].setPixelColor(i - 1, applicaLuminosita(colorWipe, 80));
    if(i - 2 >= 0 && i - 2 < NUM_LEDS) strisce[indexStr].setPixelColor(i - 2, applicaLuminosita(colorWipe, 20));
    strisce[indexStr].show();
    delay(SPEED_BULLET);
  }
  strisce[indexStr].clear(); strisce[indexStr].show();
}

// Animazione Task Done (Effetto liquido calibrato)
void animaTaskDone(int indexStr) {
  int ledStart = statoLedAttuali[indexStr];
  if (ledStart == 0) return;

  for(float cutOff = (float)ledStart; cutOff >= -1.0; cutOff -= DRAIN_SPEED_STEP) {
    strisce[indexStr].clear();
    
    for(int i=0; i<NUM_LEDS; i++) {
      if(i < (int)cutOff) {
        strisce[indexStr].setPixelColor(i, colorActive);
      }
      else if (i == (int)cutOff) {
        float fraction = cutOff - (int)cutOff;
        uint8_t b = (uint8_t)(255.0 * fraction);
        strisce[indexStr].setPixelColor(i, applicaLuminosita(colorWipe, b));
      }
    }
    strisce[indexStr].show();
    
    delay(3); 
  }
  
  strisce[indexStr].clear(); 
  strisce[indexStr].show();
}