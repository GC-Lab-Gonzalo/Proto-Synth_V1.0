/*
  Secuenciador de Trance Electrónico para ESP32 - VERSIÓN MEJORADA
  
  Hardware necesario:
  - 4 Potenciómetros en pines 13, 14, 12, 27
  - 4 Botones en pines 32, 33, 26, 35 (con pull-up interno)
  - 4 LEDs en pines 2, 5, 22, 23
  - Altavoz en pin 25 (DAC)
  
  Controles:
  - Pot 1 (Pin 13): Frecuencia de corte del filtro
  - Pot 2 (Pin 14): Resonancia del filtro
  - Pot 3 (Pin 12): Tempo (40-300 BPM) - RANGO SÚPER AMPLIO
  - Pot 4 (Pin 27): Decay/Release - CONTROL DE ENVELOPE EXTENDIDO
  
  - Botón 1 (Pin 32): Cambiar escala
  - Botón 2 (Pin 33): Cambiar patrón
  - Botón 3 (Pin 26): Play/Stop
  - Botón 4 (Pin 35): Cambiar longitud de secuencia
  
  - LEDs: Indican el paso actual de la secuencia
*/

#include "driver/dac.h"
#include "math.h"

// Pines de hardware
const int CUTOFF_POT_PIN = 13;
const int RESONANCE_POT_PIN = 14;
const int TEMPO_POT_PIN = 12;
const int DECAY_POT_PIN = 27;  // Cambiado de PITCH a DECAY

const int SCALE_BTN_PIN = 32;
const int PATTERN_BTN_PIN = 33;
const int PLAY_BTN_PIN = 26;
const int LENGTH_BTN_PIN = 35;

const int LED_PINS[4] = {23, 22, 5, 2};

// Configuración de audio
const int SAMPLE_RATE = 8000;
const int AMPLITUDE = 90;

// Variables del filtro
float filter_x1 = 0, filter_x2 = 0;
float filter_y1 = 0, filter_y2 = 0;
float filter_a0, filter_a1, filter_a2;
float filter_b1, filter_b2;

// Variables del secuenciador
bool isPlaying = false;
unsigned long lastStepTime = 0;
int currentStep = 0;
int sequenceLength = 4;
int currentPattern = 0;
int currentScale = 0;
float baseFreq = 65.41; // C2 - 1 octava abajo de C3

// Variables para control de decay
float decayFactor = 0.5; // Factor de decay por defecto

// Variables para eliminar chasquidos entre notas
float lastSample = 128.0; // Último sample reproducido para continuidad
bool noteActive = false;  // Estado de si hay una nota sonando

// Escalas musicales (intervalos en semitonos)
const int scales[][8] = {
  {0, 2, 4, 5, 7, 9, 11, 12},  // Mayor
  {0, 2, 3, 5, 7, 8, 10, 12},  // Menor natural
  {0, 2, 3, 5, 7, 8, 11, 12},  // Menor armónica
  {0, 1, 3, 4, 6, 8, 10, 12},  // Árabe
  {0, 2, 4, 6, 7, 9, 11, 12},  // Lidia
  {0, 1, 4, 5, 7, 8, 11, 12}   // Frigia
};
const char* scaleNames[] = {"Mayor", "Menor", "Armonica", "Arabe", "Lidia", "Frigia"};
const int numScales = 6;

// Patrones de secuencia (16 patrones diferentes)
const int patterns[][16] = {
  {1, 0, 3, 0, 5, 0, 3, 0, 1, 0, 3, 0, 5, 0, 7, 0}, // Trance básico
  {1, 0, 0, 0, 5, 0, 0, 0, 3, 0, 0, 0, 7, 0, 0, 0}, // Espaciado
  {1, 3, 5, 7, 5, 3, 1, 0, 1, 3, 5, 7, 5, 3, 1, 0}, // Ascendente
  {7, 5, 3, 1, 3, 5, 7, 0, 7, 5, 3, 1, 3, 5, 7, 0}, // Descendente
  {1, 1, 5, 5, 3, 3, 7, 0, 1, 1, 5, 5, 3, 3, 7, 0}, // Repetitivo
  {1, 0, 5, 3, 0, 7, 1, 0, 5, 0, 3, 7, 0, 1, 5, 0}, // Sincopado
  {1, 3, 0, 5, 7, 0, 3, 1, 5, 7, 0, 1, 3, 0, 5, 7}, // Complejo
  {1, 0, 0, 5, 0, 0, 1, 5, 0, 3, 0, 7, 0, 1, 0, 0}, // Breakbeat
  {7, 7, 5, 5, 3, 3, 1, 1, 7, 7, 5, 5, 3, 3, 1, 1}, // Dobles
  {1, 5, 1, 7, 1, 3, 1, 5, 1, 7, 1, 3, 1, 5, 1, 0}, // Bajo constante
  {0, 1, 0, 3, 0, 5, 0, 7, 0, 5, 0, 3, 0, 1, 0, 0}, // Off-beat
  {1, 3, 5, 1, 7, 5, 3, 7, 1, 3, 5, 1, 7, 5, 3, 0}, // Arpegio
  {5, 0, 5, 0, 1, 0, 1, 0, 3, 0, 3, 0, 7, 0, 7, 0}, // Alternado
  {1, 7, 1, 5, 1, 3, 1, 7, 5, 7, 5, 3, 5, 1, 5, 0}, // Saltos
  {3, 3, 3, 5, 5, 5, 1, 0, 7, 7, 7, 5, 5, 5, 3, 0}, // Grupos
  {1, 0, 7, 0, 5, 0, 3, 0, 5, 0, 7, 0, 1, 0, 0, 0}  // Clásico
};
const int numPatterns = 16;

// Variables simples para botones
bool playButtonPressed = false;
bool lastPlayState = HIGH;
bool lastScaleState = HIGH;
bool lastPatternState = HIGH;
bool lastLengthState = HIGH;

// Variables para anti-rebote sin delays
unsigned long lastScalePress = 0;
unsigned long lastPatternPress = 0;
unsigned long lastLengthPress = 0;
const unsigned long debounceDelay = 200; // 200ms anti-rebote

// FUNCIÓN PARA ACTUALIZAR DECAY
void updateDecay() {
  int decayPot = analogRead(DECAY_POT_PIN);
  // Rango de decay EXTENDIDO: de 0.05 (súper corto) a 5.0 (súper largo)
  decayFactor = 0.05 + (decayPot / 4095.0) * 4.95;
}

// Función para calcular coeficientes del filtro
void updateFilterCoefficients() {
  int cutoff_raw = analogRead(CUTOFF_POT_PIN);
  int resonance_raw = analogRead(RESONANCE_POT_PIN);
  
  float cutoff_freq = 200.0 + (cutoff_raw / 4095.0) * 2800.0;
  float Q = 0.7 + (resonance_raw / 4095.0) * 15.0;
  
  float omega = 2.0 * PI * cutoff_freq / SAMPLE_RATE;
  float sin_omega = sin(omega);
  float cos_omega = cos(omega);
  float alpha = sin_omega / (2.0 * Q);
  
  float b0 = (1.0 - cos_omega) / 2.0;
  float b1 = 1.0 - cos_omega;
  float b2 = (1.0 - cos_omega) / 2.0;
  float a0_temp = 1.0 + alpha;
  float a1_temp = -2.0 * cos_omega;
  float a2_temp = 1.0 - alpha;
  
  filter_a0 = b0 / a0_temp;
  filter_a1 = b1 / a0_temp;
  filter_a2 = b2 / a0_temp;
  filter_b1 = a1_temp / a0_temp;
  filter_b2 = a2_temp / a0_temp;
}

// Función para aplicar el filtro
float applyFilter(float input) {
  float output = filter_a0 * input + filter_a1 * filter_x1 + filter_a2 * filter_x2
                 - filter_b1 * filter_y1 - filter_b2 * filter_y2;
  
  filter_x2 = filter_x1;
  filter_x1 = input;
  filter_y2 = filter_y1;
  filter_y1 = output;
  
  return output;
}

// Función para tocar una nota con decay controlable y sin chasquidos
void playNote(float frequency, int duration_ms) {
  if (frequency == 0) {
    // Silencio con fade-out suave para evitar chasquidos
    int silence_samples = (duration_ms * SAMPLE_RATE) / 1000;
    int fadeout_samples = min(200, silence_samples); // Fade-out de hasta 200 samples
    
    for (int i = 0; i < silence_samples && isPlaying; i++) {
      float sample_value = 128.0;
      
      // Fade-out suave al inicio del silencio
      if (i < fadeout_samples && noteActive) {
        float fade_factor = 1.0 - ((float)i / fadeout_samples);
        sample_value = 128.0 + (lastSample - 128.0) * fade_factor;
      }
      
      dac_output_voltage(DAC_CHANNEL_1, (int)sample_value);
      lastSample = sample_value;
      delayMicroseconds(125);
    }
    noteActive = false;
    return;
  }
  
  // Actualizar decay al inicio de cada nota
  updateDecay();
  
  float samples_per_cycle = SAMPLE_RATE / frequency;
  int total_samples = (duration_ms * SAMPLE_RATE) / 1000;
  int filter_update_rate = SAMPLE_RATE / 200;
  int fadein_samples = min(100, total_samples / 4); // Fade-in suave
  
  for (int i = 0; i < total_samples && isPlaying; i++) {
    if (i % filter_update_rate == 0) {
      updateFilterCoefficients();
    }
    
    // Onda diente de sierra
    float cycle_position = fmod(i, samples_per_cycle) / samples_per_cycle;
    float raw_sample = 2 * cycle_position - 1;
    
    // ENVELOPE MEJORADO CON FADE-IN/OUT PARA ELIMINAR CHASQUIDOS
    float envelope = 1.0;
    float progress = (float)i / total_samples;
    
    // Fade-in suave al inicio (previene chasquidos)
    if (i < fadein_samples) {
      float fadein_factor = (float)i / fadein_samples;
      envelope *= fadein_factor;
      
      // Si hay una nota anterior, hacer crossfade
      if (noteActive && i < 50) {
        float crossfade = (float)i / 50.0;
        float prev_contribution = (lastSample - 128.0) / (float)AMPLITUDE * (1.0 - crossfade);
        raw_sample = raw_sample * crossfade + prev_contribution * (1.0 - crossfade);
      }
    }
    
    // Decay/Release controlable por potenciómetro
    if (i >= fadein_samples) {
      float decay_progress = (float)(i - fadein_samples) / (total_samples - fadein_samples);
      envelope *= exp(-decay_progress * (6.0 / decayFactor)); // Decay exponencial
    }
    
    raw_sample *= envelope;
    
    float filtered_sample = applyFilter(raw_sample);
    int sample = 128 + (int)(AMPLITUDE * filtered_sample);
    
    if (sample > 255) sample = 255;
    if (sample < 0) sample = 0;
    
    dac_output_voltage(DAC_CHANNEL_1, sample);
    lastSample = (float)sample;
    delayMicroseconds(125);
  }
  noteActive = true;
}

// Obtener frecuencia de la nota (pitch fijo en C3)
float getNoteFrequency(int scaleStep) {
  if (scaleStep == 0) return 0; // Silencio
  
  // Calcular la frecuencia de la nota en la escala desde C3 fijo
  int semitone = scales[currentScale][scaleStep - 1];
  float frequency = baseFreq * pow(2.0, semitone / 12.0);
  
  return frequency;
}

void setup() {
  
  // Configurar pines
  pinMode(CUTOFF_POT_PIN, INPUT);
  pinMode(RESONANCE_POT_PIN, INPUT);
  pinMode(TEMPO_POT_PIN, INPUT);
  pinMode(DECAY_POT_PIN, INPUT);
  
  // Configurar botones con pull-up interno
  pinMode(SCALE_BTN_PIN, INPUT_PULLUP);    
  pinMode(PATTERN_BTN_PIN, INPUT_PULLUP);  
  pinMode(PLAY_BTN_PIN, INPUT_PULLUP);     
  pinMode(LENGTH_BTN_PIN, INPUT_PULLUP);   
  
  for (int i = 0; i < 4; i++) {
    pinMode(LED_PINS[i], OUTPUT);
    digitalWrite(LED_PINS[i], LOW);
  }
  
  // Configurar DAC
  dac_output_enable(DAC_CHANNEL_1);
  updateFilterCoefficients();
  
  // Secuencia de inicio con LEDs
  for (int i = 0; i < 4; i++) {
    digitalWrite(LED_PINS[i], HIGH);
    delay(100);
    digitalWrite(LED_PINS[i], LOW);
  }
}

void loop() {
  // Debug cada 2 segundos (menos frecuente)
  static unsigned long lastDebugTime = 0;
  if (millis() - lastDebugTime > 2000) {
    lastDebugTime = millis();
  }
  
  // Detección SUPER SIMPLE del botón de play (pin 26)
  bool currentPlayState = digitalRead(PLAY_BTN_PIN);
  if (currentPlayState == LOW && lastPlayState == HIGH) {
    // Botón presionado!
    isPlaying = !isPlaying;
    
    if (isPlaying) {
      currentStep = 0;
      lastStepTime = millis();
    } else {
      dac_output_voltage(DAC_CHANNEL_1, 128); // Silencio
      for (int i = 0; i < 4; i++) {
        digitalWrite(LED_PINS[i], LOW);
      }
    }
    delay(200); // Anti-rebote simple
  }
  lastPlayState = currentPlayState;
  
  // Detección simple de otros botones - SIN INTERRUMPIR LA SECUENCIA
  bool currentScaleState = digitalRead(SCALE_BTN_PIN);
  if (currentScaleState == LOW && lastScaleState == HIGH && 
      (millis() - lastScalePress) > debounceDelay) {
    currentScale = (currentScale + 1) % numScales;
    lastScalePress = millis();
    // Cambio instantáneo sin pausas ni parpadeos
  }
  lastScaleState = currentScaleState;
  
  bool currentPatternState = digitalRead(PATTERN_BTN_PIN);
  if (currentPatternState == LOW && lastPatternState == HIGH && 
      (millis() - lastPatternPress) > debounceDelay) {
    currentPattern = (currentPattern + 1) % numPatterns;
    lastPatternPress = millis();
    // Cambio instantáneo sin pausas ni parpadeos
  }
  lastPatternState = currentPatternState;
  
  bool currentLengthState = digitalRead(LENGTH_BTN_PIN);
  if (currentLengthState == LOW && lastLengthState == HIGH && 
      (millis() - lastLengthPress) > debounceDelay) {
    sequenceLength = (sequenceLength == 4) ? 8 : ((sequenceLength == 8) ? 16 : 4);
    // Ajustar el paso actual si es necesario para evitar saltos bruscos
    if (currentStep >= sequenceLength) {
      currentStep = 0;
    }
    lastLengthPress = millis();
    // Cambio instantáneo sin pausas ni parpadeos
  }
  lastLengthState = currentLengthState;
  
  // Actualizar LEDs
  if (isPlaying) {
    for (int i = 0; i < 4; i++) {
      if (i == (currentStep % 4)) {
        digitalWrite(LED_PINS[i], HIGH);
      } else {
        digitalWrite(LED_PINS[i], LOW);
      }
    }
  }
  
  // Secuenciador principal
  if (isPlaying) {
    int tempoPot = analogRead(TEMPO_POT_PIN);
    // NUEVO RANGO DE TEMPO: 40-300 BPM (súper amplio para hardcore extremo)
    int bpm = 40 + (tempoPot / 4095.0) * 260; // 40-300 BPM
    unsigned long stepDuration = 60000 / (bpm * 4); // 16th notes
    
    if (millis() - lastStepTime >= stepDuration) {
      // Actualizar LEDs directamente aquí
      for (int i = 0; i < 4; i++) {
        if (i == (currentStep % 4)) {
          digitalWrite(LED_PINS[i], HIGH);
        } else {
          digitalWrite(LED_PINS[i], LOW);
        }
      }
      
      int noteValue = patterns[currentPattern][currentStep];
      float frequency = getNoteFrequency(noteValue);
      
      // Duración de la nota (90% del step)
      int noteDuration = stepDuration * 0.9;
      playNote(frequency, noteDuration);
      
      currentStep = (currentStep + 1) % sequenceLength;
      lastStepTime = millis();
    }
  }
  
  delay(1); // Pequeña pausa para estabilidad
}