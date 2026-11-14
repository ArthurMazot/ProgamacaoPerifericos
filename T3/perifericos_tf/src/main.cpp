#include <Arduino.h>

// ---------- Pinos ----------
const int J1_X = 32, J1_Y = 33, J1_SW = 25;
const int J2_X = 34, J2_Y = 35, J2_SW = 26;

// ---------- Ajustes do joystick (ADC 12 bits: 0..4095) ----------
const int ADC_CENTER   = 2048;
const int ADC_DEADZONE = 700;   // distancia do centro para considerar movimento
const int SAMPLE_MS    = 10;    // periodo de leitura

// ---------- Tipos ----------
struct JoyRaw {
  int  x;
  int  y;
  bool pressed;
};

struct InputState {
  JoyRaw j[2];
};

enum Direction : uint8_t { DIR_NONE, DIR_UP, DIR_DOWN, DIR_LEFT, DIR_RIGHT, DIR_PRESS };

struct DirState {
  Direction d[2];
};

// ---------- Filas ----------
QueueHandle_t qInput;  // taskInput -> taskGame  (estado mais recente)
QueueHandle_t qDir;    // taskGame  -> taskDisplay (so quando muda)

// ---------- Tarefa 1: leitura dos controles ----------
void taskInput(void *) {
  pinMode(J1_SW, INPUT_PULLUP);
  pinMode(J2_SW, INPUT_PULLUP);

  const int sw[2] = {J1_SW, J2_SW};
  const int px[2] = {J1_X, J2_X};
  const int py[2] = {J1_Y, J2_Y};

  int  stableCount[2] = {0, 0};
  bool stablePressed[2] = {false, false};

  TickType_t last = xTaskGetTickCount();
  for (;;) {
    InputState s;
    for (int i = 0; i < 2; i++) {
      s.j[i].x = analogRead(px[i]);
      s.j[i].y = analogRead(py[i]);

      // debounce simples: precisa de 3 leituras iguais para mudar
      bool raw = (digitalRead(sw[i]) == LOW);  // pull-up: apertado = LOW
      if (raw != stablePressed[i]) {
        if (++stableCount[i] >= 3) {
          stablePressed[i] = raw;
          stableCount[i] = 0;
        }
      } else {
        stableCount[i] = 0;
      }
      s.j[i].pressed = stablePressed[i];
    }
    xQueueOverwrite(qInput, &s);
    vTaskDelayUntil(&last, pdMS_TO_TICKS(SAMPLE_MS));
  }
}

// ---------- Logica do jogo: calcula a direcao ----------
Direction calcDirection(const JoyRaw &j) {
  if (j.pressed) return DIR_PRESS;

  int dx = j.x - ADC_CENTER;
  int dy = j.y - ADC_CENTER;
  if (abs(dx) < ADC_DEADZONE && abs(dy) < ADC_DEADZONE) return DIR_NONE;

  // eixo dominante vence (evita diagonais)
  if (abs(dx) >= abs(dy)) return dx > 0 ? DIR_RIGHT : DIR_LEFT;
  return dy > 0 ? DIR_DOWN : DIR_UP;  // se ficar invertido, troque UP/DOWN
}

// ---------- Tarefa 2: jogo ----------
void taskGame(void *) {
  DirState last = {{DIR_NONE, DIR_NONE}};
  for (;;) {
    InputState in;
    if (xQueueReceive(qInput, &in, portMAX_DELAY) == pdTRUE) {
      DirState cur;
      for (int i = 0; i < 2; i++) cur.d[i] = calcDirection(in.j[i]);

      if (cur.d[0] != last.d[0] || cur.d[1] != last.d[1]) {
        last = cur;
        xQueueSend(qDir, &cur, 0);
      }
    }
  }
}

// ---------- Tarefa 3: display (terminal por enquanto) ----------
const char *symbol(Direction d) {
  switch (d) {
    case DIR_UP:    return "\xE2\x86\x91";  // seta cima
    case DIR_DOWN:  return "\xE2\x86\x93";  // seta baixo
    case DIR_LEFT:  return "\xE2\x86\x90";  // seta esquerda
    case DIR_RIGHT: return "\xE2\x86\x92";  // seta direita
    case DIR_PRESS: return "\xE2\x97\x8F";  // circulo
    default:        return "-";
  }
}

void taskDisplay(void *) {
  for (;;) {
    DirState s;
    if (xQueueReceive(qDir, &s, portMAX_DELAY) == pdTRUE) {
      Serial.printf("J1: %s   J2: %s\n", symbol(s.d[0]), symbol(s.d[1]));
    }
  }
}

// ---------- Setup ----------
void setup() {
  Serial.begin(115200);

  qInput = xQueueCreate(1, sizeof(InputState));
  qDir   = xQueueCreate(8, sizeof(DirState));

  xTaskCreatePinnedToCore(taskInput,   "input",   2048, nullptr, 3, nullptr, 0);
  xTaskCreatePinnedToCore(taskGame,    "game",    2048, nullptr, 2, nullptr, 1);
  xTaskCreatePinnedToCore(taskDisplay, "display", 3072, nullptr, 1, nullptr, 1);
}

/*
  Como Esp32 já utiliza OS freeRTOs, montamos o trabalho em tarefas
  assim garantimos que tanto os controles quanto o jogo e o display não ocuparão 100% CPU
*/
void loop() {
  vTaskDelete(nullptr);
}
