// Sketch de ejemplo para enviar tramas PRT-7 por el puerto serie
// a 9600 baudios, una línea por segundo.

void setup() {
  Serial.begin(9600);
  delay(2000); // esperar a que el puerto se estabilice
}

const char* lines[] = {
  "L,H",
  "L,O",
  "L,L",
  "M,2",
  "L,A",
  "L,Space",
  "L,W",
  "M,-2",
  "L,O",
  "L,R",
  "L,L",
  "L,D",
  "END"
};

void loop() {
  static int i = 0;
  static unsigned long last = 0;
  const unsigned long interval = 1000; // 1 segundo

  unsigned long now = millis();
  if (now - last >= interval) {
    last = now;
    if (i < (int)(sizeof(lines)/sizeof(lines[0]))) {
      Serial.println(lines[i++]);
    }
  }
}
