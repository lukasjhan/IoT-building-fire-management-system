/*
FE program
  setup
    conponent
      LED
        D6
      buzzer
        D6
      BT 
        RX7
        TX8
      RBG sensor  
        s0          s0  s1  state
        s1          L   L   powerdown
                    L   H   2%(scaling)
                    H   L   20%
                    H   H   100%
        s2          s2  s3  filter
        s3          L   L   Red
                    L   H   Blue
                    H   L   no filter
                    H   H   green
        LED    
        OUTPUT
  loop
  1.switch state
  1.1.sleep         using lowpower library, 500ms powerdown   done
  1.2.standby       processing cmd from BT master             done
  1.3.health Check  health check                              health check success, calibrating sensor value
  1.4.transmit      reporting health                          done
  1.5.Alarm         fire alarm                                done
  1.7.default                                                 for error
*/
//1439
#include <SoftwareSerial.h>
#include <LowPower.h>

#define ALARM_OUT   6
#define BT_RX       7
#define BT_TX       8
#define TCS_S0      10
#define TCS_S1      9
#define TCS_S2      12
#define TCS_S3      13
#define TCS_LED     
#define TCS_OUTPUT  5
#define BATTERY_CHECK A1


typedef enum state
{
  _ERROR = -1,
  FIRE_ALARM = 0,
  SLEEP,
  STANDBY,
  TRANSMIT,
  HEALTH_CHECK = 15,
  //LOWPOWER
  
  
}State;

State state;
SoftwareSerial BT(BT_RX,BT_TX);
uint8_t h_ecc[16] = {0, 11, 22, 29, 39, 44, 49, 58, 69, 78, 83, 88, 98, 105, 116, 127};

uint8_t to_HECC(int data)
{
  return h_ecc[data];
}
int get_data_from_HECC(uint8_t value)
{
  return value >> 3;
}

State get_opcode(uint8_t rev)
{
  switch(get_data_from_HECC(rev))
  {
    case 0:
      return FIRE_ALARM;
    case 15:
      return HEALTH_CHECK;
    default:
      return _ERROR;
  }
}

uint8_t make_healthcheck_data(bool health_check_success, bool pressure_ok, bool battery_ok)
{
  uint8_t ret = 0;
  if (health_check_success) ret += 1; ret << 1;
  ret << 1; //reserved
  if (pressure_ok) ret += 1; ret << 1;
  if (battery_ok) ret += 1; ret << 1;

  return ret;
}
uint8_t getHealth()
{
  bool health_check_success = true;
  digitalWrite(TCS_S0,HIGH);
  digitalWrite(TCS_S1,HIGH);//sensor on 100%
  digitalWrite(TCS_S2,HIGH);
  digitalWrite(TCS_S3,LOW);//no filter mode
  int sensor_value = pulseIn(TCS_OUTPUT,LOW);
  if(sensor_value == 0)health_check_success=false;
  //calibrated formula here, to convert sensor value into pressure boolean state
  bool pressure_ok;
  digitalWrite(TCS_S0,LOW);
  digitalWrite(TCS_S1,LOW);//sensor off
  bool battery_ok = is_battery_ok();
  //battery check formula here, 0.9 if 9v battery used
  //suppose under 50% of max voltage means out of power
  return to_HECC(make_healthcheck_data(health_check_success,pressure_ok,battery_ok));
}

bool is_battery_ok()
{
  return analogRead(analogRead(BATTERY_CHECK))/0.9 > 5 ? true:false;
}

void setup() {
  pinMode(ALARM_OUT,OUTPUT);
  pinMode(TCS_S0,OUTPUT);
  pinMode(TCS_S1,OUTPUT);
  pinMode(TCS_S2,OUTPUT);
  pinMode(TCS_S3,OUTPUT);
  pinMode(TCS_OUTPUT,INPUT);
  pinMode(BATTERY_CHECK,INPUT);
  BT.begin(9600);
  while(!BT.available())
  {
    digitalWrite(ALARM_OUT,HIGH);
    tone(ALARM_OUT,0);
  }
  state = SLEEP;
  Serial.begin(9600);
}

void loop() {
  uint8_t cmd;
  uint8_t health_report = 0;
  switch(state)
  {
    case SLEEP://low power mode
      LowPower.powerDown(SLEEP_500MS,ADC_OFF,BOD_OFF);
      state = STANDBY;
      break;
    case STANDBY://processing cmd
      cmd = BT.read();
      state = get_opcode(cmd);
      break;
    case HEALTH_CHECK://health check,set health_report
      health_report = getHealth();
      break;
    case TRANSMIT://transmit report
      BT.write(health_report);
      break;
    case FIRE_ALARM://fire alarm
        digitalWrite(ALARM_OUT,HIGH);
        tone(ALARM_OUT,1200,500);
        delay(500);
      break;
    default:
      Serial.println("error code -1");
      //or
      Serial.println("no cmd, sleep");
      state = SLEEP;
      break;
  }
}
