#define SET_BIT(REG,BIT)    (REG = REG | (1 << BIT))
#define CLEAR_BIT(REG,BIT)  (REG = REG & ~(1 << BIT))
#define GET_BIT(REG,BIT)    ((REG & (1 << BIT)) >> BIT)

#define p_lf 3
#define p_lb 5
#define p_rf 6
#define p_rb 9
#define RECHTER_SENSOR  A5
#define LINKER_SENSOR   A1
#define MITTLERER_SENSOR A0
#define STOP_BUTTON 2
#define START_BUTTON 1

#define p_led 4

void drive(int left, int right);

int doDrive = 0;
const int numSamples = 16;
const int k = 12;

uint16_t values_left[numSamples];
uint16_t values_middle[numSamples];
uint16_t values_right[numSamples];
uint16_t average_right_raw, average_left_raw, average_middle_raw;
int diff;
uint16_t left, middle, right;
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
//int Threshhold_begin_curve = 1500;
int Threshhold_begin_curve_side = 800;
int Threshhold_end_curve_side = 1000;
//int Threshhold_end_curve = 0;

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
  if (index == numSamples)
    index = 0;

  average_left_raw = 0;
  average_middle_raw = 0;
  average_right_raw = 0; 

  for (int i = 0; i < numSamples; i++) {
    average_left_raw += values_left[i];
    average_middle_raw += values_middle[i];
    average_right_raw += values_right[i];
  }

  average_left_raw /= numSamples;
  average_middle_raw /= numSamples;
  average_right_raw /= numSamples;
}

void loop() {
  // Linker Sensor "schaut" nach rechts
  get_vals();

  right = (uint16_t) (6336/(average_left_raw - 15.15)) - 2;
  middle = (uint16_t) (18670.08/(average_middle_raw + 13.44)) - 11;
  left = (uint16_t) ((5202.9/(average_right_raw - 32.97)) - 12);

  if (right > 100)  right = 100;
  if (left > 100)   left = 100;
  if (middle > 150) middle = 150;

  diff = (int) (right - left);

  if (!doDrive) {
    drive(0, 0);
  
    Serial.print(left); Serial.println(" cm");
    Serial.print(middle); Serial.println(" cm");
    Serial.print(right); Serial.println(" cm");
    Serial.println();
    Serial.print("diff: "); Serial.print(diff); Serial.println(" cm");
    
    delay(100);
    return;
  }

  switch (state) {
    case (0):
      digitalWrite(p_led, 0);
      if (diff > 20)
        diff = 20;
      else if (diff < -20)
        diff = -20;
    
      if (diff <= 0)
        drive(255 + diff*k, 255);
    
      else if (diff > 0)
        drive(255, 255 - diff*k); 

      if ((left > 90 || right > 90)) {
        if (left > right)
          state = 2;

        else
          state = 1;
      }
      break;
      
    case (1):
      digitalWrite(p_led, 1);
      drive(32, 128);
        // average_middle < Threshhold_end_curve &&
        if ( left < 60 && right < 60 )
          state = 0; 
      break;
      
    case (2):
      digitalWrite(p_led, 1);
      drive(128, 32);
      // average_middle < Threshhold_end_curve &&
      if ( left < 60 && right < 60 )
        state = 0; 
      break;
  }
}

void drive(int left, int right) {
  left /= 1;
  right /= 1;
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
  doDrive = !doDrive;
}
