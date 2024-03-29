#define SET_BIT(REG,BIT)    (REG = REG | (1 << BIT))
#define CLEAR_BIT(REG,BIT)  (REG = REG & ~(1 << BIT))
#define GET_BIT(REG,BIT)    ((REG & (1 << BIT)) >> BIT)

#define p_lf 3
#define p_lb 5
#define p_rf 6
#define p_rb 9
#define RECHTER_SENSOR  A1
#define LINKER_SENSOR   A2
#define MITTLERER_SENSOR A0
#define STOP_BUTTON 2
#define START_BUTTON 1

void drive(int left, int right);

int doDrive = 1;
int numSamples = 30;

void setup() {
  Serial.begin(9600);
  pinMode(RECHTER_SENSOR, INPUT);
  pinMode(LINKER_SENSOR, INPUT);
  pinMode(p_lf, OUTPUT);
  pinMode(p_lb, OUTPUT);
  pinMode(p_rf, OUTPUT);
  pinMode(p_rb, OUTPUT);
  pinMode(STOP_BUTTON, INPUT_PULLUP);
  pinMode(START_BUTTON, INPUT_PULLUP);
 
  SET_BIT(EICRA, ISC11);               
  SET_BIT(EICRA, ISC10); 
  SET_BIT(EIMSK, INT0);               
  sei();
}

//Wert ist groeser, je naeher objekt
int value_rechts, value_links, value_middle, diff;
int vals[3];
int Threshhold_begin_curve = 7500;
int Threshhold_end_curve = 6000;

float k = 0.067;
int state = 0;
/*
 * 0: Mittenregelung
 * 1: Kurve
 */

void get_vals() {
  
  vals[0] = 0; vals[1] = 0; vals[2] = 0;
  for (int i = 0; i < numSamples; i++)
    vals[0] += analogRead(LINKER_SENSOR);
  for (int i = 0; i < numSamples; i++)
    vals[1] += analogRead(MITTLERER_SENSOR);
  for (int i = 0; i < numSamples; i++)
    vals[2] += analogRead(RECHTER_SENSOR); 
}

void loop() {
  // Linker Sensor "schaut" nach rechts
  get_vals();
  value_links = vals[0];
  value_middle = vals[1];
  value_rechts = vals[2];
  
  while (!doDrive) {
    drive(0, 0);
    doDrive = !digitalRead(START_BUTTON);

    get_vals();
    Serial.print(vals[0]); Serial.print(",");
    Serial.print(vals[1]); Serial.print(",");
    Serial.println(vals[2]); Serial.print(",");
    Serial.println();
    delay(50);
  }

  if (value_middle < Threshhold_begin_curve && state == 0) {
    k = 0.35;
    state = 1;
  }

  if (value_middle < Threshhold_end_curve && state == 1) {
    k = 0.067;
    state = 0;
  }
  
  diff = (value_rechts - value_links)*k;

  if (diff > 255)
    diff = 255;
  else if (diff < -255)
    diff = -255;

  if (diff <= 0)
    drive(255, 255+diff);

  else if (diff > 0)
    drive(255-diff, 255);
}

void drive(int left, int right) {
  if (left > 0) {
    analogWrite(p_lf, left);
    analogWrite(p_lb, 0);
  } 
  else {
    analogWrite(p_lb, -left);
    analogWrite(p_lf, 0);
  }

  if (right > 0) {
    analogWrite(p_rf, right);
    analogWrite(p_rb, 0);
  } 
  else {
    analogWrite(p_rb, -right);
    analogWrite(p_rf, 0);
  }
}



ISR(INT0_vect){
  doDrive = 0;
}
