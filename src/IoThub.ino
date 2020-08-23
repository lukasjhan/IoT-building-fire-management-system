/*
IoT hub program
  setup
    1.conponent
    BT_RX 7
    BT_TX 8
  loop
    1.switch state
    1.1.adding components
    1.2.standby                           node losing interrupt
    1.3.health check requesting           
    1.4.health check gathering
    1.5.fire alarm 
    1.6.reporting to admin through E-mail
  
*/
#include <SoftwareSerial.h>
#include <Phpoc.h>

#define BT_RX 7
#define BT_TX 8

typedef enum state{
  ADD_COMPONENT,
  STANDBY,
  HEALTH_CHECK_REQUEST,
  HEALTH_CHECK_GATHERING,
  FIRE_ALARM,
  REPORTING_TO_ADMIN
}State;

SoftwareSerial BT(BT_RX,BT_TX);
PhpocEmail email;
PhpocDateTime datetime;


State state = STANDBY;

uint8_t h_ecc[16] = {0, 11, 22, 29, 39, 44, 49, 58, 69, 78, 83, 88, 98, 105, 116, 127};

uint8_t to_HECC(int data)
{
  return h_ecc[data];
}
int get_data_from_HECC(uint8_t value)
{
  
  return value >> 3;
}


void setup() {
  // put your setup code here, to run once:
  BT.begin(9600);
  Phpoc.begin(PF_LOG_SPI | PF_LOG_NET | PF_LOG_APP);
  email.setOutgoingServer("smtp.gmail.com", 587);
  email.setOutgoingLogin("gorhs0918", "njquktrvlilbeowo");
  while(!BT.available());
}

void loop() {
  // put your main code here, to run repeatedly:
  byte report=0;///////////////////////////////////////////////////////////////
  switch(state)
  {
    case ADD_COMPONENT:
      break;
    case STANDBY:
        if(datetime.hour() == 16)
          {
            if(report)////////////////////////////////////////////////////////
            state = REPORTING_TO_ADMIN;
          }
         if(BT.available() == false)
          {
            //losing warning email send
          }
      break;
    case HEALTH_CHECK_REQUEST:
        //sending health check requesting operation in byte
      break;
    case HEALTH_CHECK_GATHERING:
        //make report variable
      break;
    case FIRE_ALARM:
        //send FE fire alarm operation
      break;
    case REPORTING_TO_ADMIN:
        //gmail sendng state
      break;
    default:
        //
      break;
  }

}
