/* ================================================================== */
/* =                       OumiVM.ino                               = */
/* =           (OPTIMIZADO CON MACRO F() Y MENOR BUFFER)            = */
/* ================================================================== */

#include <SPI.h>
#include <SD.h>
#include "vm.h"

const int PIN_MOTOR_ENA = 5;
const int PIN_MOTOR_ENB = 6;
const int PIN_IN1 = 2;
const int PIN_IN2 = 3;
const int PIN_IN3 = 4;
const int PIN_IN4 = 7;
const int PIN_SENSOR_IZQ = 8;
const int PIN_SENSOR_DER = 9;
const int SD_CS_PIN = 10; 

// --- AHORRO DE MEMORIA ---
// Reducido a 256 bytes. Suficiente para scripts medianos.
byte GLOBAL_BYTECODE_BUFFER[256]; 

VM vm;

void setup() {
  Serial.begin(9600);
  // Usamos F() para guardar texto en Flash y no en RAM
  Serial.println(F("--- Robot Oumi (Modo SD) ---"));

  pinMode(PIN_MOTOR_ENA, OUTPUT); pinMode(PIN_MOTOR_ENB, OUTPUT);
  pinMode(PIN_IN1, OUTPUT); pinMode(PIN_IN2, OUTPUT);
  pinMode(PIN_IN3, OUTPUT); pinMode(PIN_IN4, OUTPUT);
  pinMode(PIN_SENSOR_IZQ, INPUT);
  pinMode(PIN_SENSOR_DER, INPUT);
  pinMode(SD_CS_PIN, OUTPUT); 

  Serial.print(F("Init SD..."));
  if (!SD.begin(SD_CS_PIN)) {
    Serial.println(F("FALLO SD"));
    while (1); 
  }
  Serial.println(F("OK"));

  File archivo = SD.open("program.bin", FILE_READ);
  if (!archivo) {
    Serial.println(F("NO 'program.bin'"));
    while (1);
  }

  vm_init(&vm);
  
  int32_t numConstantes = 0;
  archivo.read(&numConstantes, sizeof(int32_t));
  Serial.print(F("Consts: ")); Serial.println(numConstantes);

  for (int i = 0; i < numConstantes; i++) {
    float valFloat;
    archivo.read(&valFloat, sizeof(float));
    vm_add_constant(&vm, (Value)valFloat);
  }

  int32_t tamanoCodigo = 0;
  archivo.read(&tamanoCodigo, sizeof(int32_t));
  Serial.print(F("Bytes: ")); Serial.println(tamanoCodigo);

  if (tamanoCodigo > sizeof(GLOBAL_BYTECODE_BUFFER)) {
    Serial.println(F("ERROR: Programa muy grande para RAM"));
    while(1);
  }

  archivo.read(GLOBAL_BYTECODE_BUFFER, tamanoCodigo);
  archivo.close();

  vm_load_bytecode(&vm, GLOBAL_BYTECODE_BUFFER);
  Serial.println(F("--- Ejecutando ---"));
}

void loop() {
  InterpretResult resultado = vm_interpretar(&vm);

  if (resultado == INTERPRET_RUNTIME_ERROR) {
    Serial.println(F("ERR Runtime"));
    while(1);
  }
  
  if (resultado == INTERPRET_OK) {
     Serial.println(F("FIN"));
     while(1); 
  }
}