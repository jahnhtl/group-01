#define SET_BIT(REG,BIT)    (REG = REG | (1 << BIT))
#define CLEAR_BIT(REG,BIT)  (REG = REG & ~(1 << BIT))
#define GET_BIT(REG,BIT)    ((REG & (1 << BIT)) >> BIT)

#define p_lf 3
#define p_lb 5
#define p_rf 6
#define p_rb 9
#define RECHTER_SENSOR  A1
#define LINKER_SENSOR   A5
#define MITTLERER_SENSOR A0
#define STOP_BUTTON 2
#define START_BUTTON 1

#define p_led 4

void drive(int left, int right);

int doDrive = 0;
int numSamples = 3;

uint8_t values_left[100];
uint8_t values_middle[100];
uint8_t values_right[100];
int average_right, average_left, average_middle, diff;
int index = 0;

void setup() {
  Serial.begin(9600);
  pinMode(RECHTER_SENSOR, INPUT);
  pinMode(LINKER_SENSOR, INPUT);
  pinMode(MITTLERER_SENSOR, INPUT);
  
  pinMode(p_lf, OUTPUT);
  pinMode(p_lb, OUTPUT);
  pinMode(p_rf, OUTPUT);
  pinMode(p_rb, OUTPUT);
  pinMode(p_led, OUTPUT);
  
  pinMode(STOP_BUTTON, INPUT_PULLUP);
  pinMode(START_BUTTON, INPUT_PULLUP);
 
  SET_BIT(EICRA, ISC11);               
  SET_BIT(EICRA, ISC10); 
  SET_BIT(EIMSK, INT0);               
  sei();
}

//Wert ist groeser, je naeher objekt
int Threshhold_begin_curve = 15000;
int Threshhold_begin_curve_side = 5300;

int Threshhold_end_curve = 10000;

float k = 0.006;
int state = 0;
/*
 * 0: Mittenregelung
 * 1: Linkskurve
 * 2: Rechtskurve
 */

void get_vals() {
  values_left[index] = analogRead(LINKER_SENSOR);
  values_middle[index] = analogRead(MITTLERER_SENSOR);
  values_right[index] = analogRead(RECHTER_SENSOR);
  index += 1;
  if (index == 100)
    index = 0;

  average_left = 0;
  average_middle = 0;
  average_right = 0; 

  for (int i = 0; i < 100; i++) {
    average_left += values_left[i];
    average_middle += values_middle[i];
    average_right += values_right[i];
  }
}

void loop() {
  // Linker Sensor "schaut" nach rechts
  get_vals();
  
  while (!doDrive) {
    drive(0, 0);
    doDrive = !digitalRead(START_BUTTON);

    for (int i=0; i<50;i++)
      get_vals();
    Serial.print(analogRead(LINKER_SENSOR)); Serial.print(",");
    Serial.print(analogRead(RECHTER_SENSOR)); Serial.print(",");
    Serial.println(analogRead(MITTLERER_SENSOR)); Serial.print(",");
    Serial.println();
    delay(50);
  }

  switch (state) {
    case (0):
      digitalWrite(p_led, 0);
      diff = (average_right - average_left)*k;

      if (diff > 128)
        diff = 128;
      else if (diff < -128)
        diff = -128;
    
      if (diff <= 0)
        drive(255, 255+diff);
    
      else if (diff > 0)
        drive(255-diff, 255); 
 
      if (average_middle > Threshhold_begin_curve || (average_left < Threshhold_begin_curve_side || average_right < Threshhold_begin_curve_side)) {
        if (average_right > average_left)
          state = 1;
        else
          state = 2;
      }
      break;
      
    case (1):
      digitalWrite(p_led, 1);
      drive(32, 128);
        if (average_middle < Threshhold_end_curve && average_left > Threshhold_begin_curve_side && average_right > Threshhold_begin_curve_side )
          state = 0; 
      break;
      
    case (2):
      digitalWrite(p_led, 1);
      drive(128, 32);
      if (average_middle < Threshhold_end_curve && average_left > Threshhold_begin_curve_side && average_right > Threshhold_begin_curve_side )
        state = 0; 
      break;
  }
}

void drive(int left, int right) {
  left /= 2;
  right /= 2;
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
