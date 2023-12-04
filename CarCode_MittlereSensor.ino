#define SET_BIT(REG,BIT)    (REG = REG | (1 << BIT))
#define CLEAR_BIT(REG,BIT)  (REG = REG & ~(1 << BIT))
#define GET_BIT(REG,BIT)    ((REG & (1 << BIT)) >> BIT)

#define MOTOR_A1 3
#define MOTOR_A2 5
#define MOTOR_B1 6
#define MOTOR_B2 9
#define RECHTER_SENSOR  A1
#define LINKER_SENSOR   A2
#define BUTTON 2

void drive(int left, int right);

int doDebug = 1;
int doDrive = 0;

void setup() {
  Serial.begin(9600);
  pinMode(RECHTER_SENSOR, INPUT);
  pinMode(LINKER_SENSOR, INPUT);
  pinMode(MOTOR_A1, OUTPUT);
  pinMode(MOTOR_A2, OUTPUT);
  pinMode(MOTOR_B1, OUTPUT);
  pinMode(MOTOR_A2, OUTPUT);
  pinMode(BUTTON, INPUT_PULLUP);
 
  SET_BIT(EICRA, ISC11);               
  SET_BIT(EICRA, ISC10); 
  SET_BIT(EIMSK, INT0);               
  sei();
}


//Wert ist groeser, je naeher objekt
int value_rechts, value_links, diff;

const float k = 7.5;

void loop() {
  value_rechts = analogRead(RECHTER_SENSOR);
  value_links = analogRead(LINKER_SENSOR);
  diff = (value_rechts - value_links)*k;
  
  if (diff > 510)
    diff = 510;
  else if (diff < -510)
    diff = -510;

  if (diff < 0)
    drive(255+diff, 255);

  else if (diff > 0)
    drive(255, 255-diff);

  delay(500);
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
