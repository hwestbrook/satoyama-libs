#include "saboten.h"
#include <utilsawesome.h>
#include <chibi.h>
// #if ARDUINO >= 100
//     #include "Arduino.h"
// #else
//     #include "WProgram.h"
// #endif

#include <avr/sleep.h>


Board::Board(){
  this->sensors = new std::vector<BaseSensor*>(0);
}


void Board::register_sensor(BaseSensor *sensor){
  this->sensors->push_back(sensor);
}

void Board::read_sensors(unsigned char *buffer){
  int n_sensors = this->sensors->size();
  for(int i; i<n_sensors; i++){
    this->sensors->at(i)->read(buffer);
  }
}




Saboten::Saboten(unsigned int serial_baud_rate){
  this->rtc = new PCF2127(0, 0, 0, Saboten::RTC_CHIPSELECT_PIN);
  // chibiCmdInit(serial_baud_rate);

  pinMode(Saboten::HIGH_GAIN_MODE_PIN, OUTPUT);
  digitalWrite(Saboten::HIGH_GAIN_MODE_PIN, LOW);

  pinMode(Saboten::RTC_CHIPSELECT_PIN, OUTPUT);
  digitalWrite(Saboten::RTC_CHIPSELECT_PIN, HIGH);

  pinMode(Saboten::SD_CHIPSELECT_PIN, INPUT);
  digitalWrite(Saboten::SD_CHIPSELECT_PIN, HIGH);

  pinMode(Saboten::SD_DETECT_PIN, INPUT);
  digitalWrite(Saboten::SD_DETECT_PIN, LOW);

  

  this->rtc->enableMinuteInterrupt();
  // this->rtc->enableSecondInterrupt();
  this->rtc->setInterruptToPulse();
  attachInterrupt(2, this->rtcInterrupt, FALLING);

  chibiInit();
  //  set up chibi regs for external P/A
  chibiRegWrite(0x4, 0xA0);
  // high gain mode
  digitalWrite(Saboten::HIGH_GAIN_MODE_PIN, HIGH);
}

void Saboten::sleep_mcu(){
  attachInterrupt(2, rtcInterrupt, FALLING);
  delay(100);

  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_radio();
  sleep_enable();        // setting up for sleep ...
  
  ADCSRA &= ~(1 << ADEN);    // Disable ADC
  sleep_mode();

  sleep_disable();
  wakeup_radio();
  ADCSRA |= (1 << ADEN); // Enable ADC
}

char* Saboten::timestamp(){
  unsigned char year;
  unsigned char month;
  unsigned char day;
  unsigned char weekday;
  this->rtc->readDate(&year, &month, &day, &weekday);
  unsigned char hour;
  unsigned char minute;
  unsigned char second;
  this->rtc->readTime(&hour, &minute, &second);
  
  char s[19];
  sprintf(s, "%d-%d-%d %d:%d:%d", year, month, day, hour, minute, second);
  return s;
};

void Saboten::set_datetime(int year, int month, int day, int hour, int minute, int second){
  this->rtc->writeTime(hour, minute, second);
  this->rtc->writeDate(year, month, day, 0);
};

void Saboten::read_board_diagnostics(unsigned char *buffer){
  this->read_battery_voltage(buffer); 
}

void Saboten::read_battery_voltage(unsigned char *buffer){
  unsigned int vbat = analogRead(Saboten::BATTERY_VOLTAGE_PIN);
  double batt = ((vbat/1023.0) * Saboten::ADC_REFERENCE_VOLTAGE) * 2;
  Reading_new battery_voltage = {"vbat", batt, this->timestamp()};
  add_to_tx_buf_new_new(buffer, &battery_voltage);
}



void Saboten::rtcInterrupt(){
  detachInterrupt(2);
}

void Saboten::sleep_radio(){
	digitalWrite(Saboten::HIGH_GAIN_MODE_PIN, LOW);
  // set up chibi regs to turn off external P/A
  chibiRegWrite(0x4, 0x20);
  chibiSleepRadio(1);  
}

void Saboten::wakeup_radio(){
	digitalWrite(Saboten::HIGH_GAIN_MODE_PIN, HIGH);
  // set up chibi regs to turn on external P/A
  chibiRegWrite(0x4, 0xA0);
  chibiSleepRadio(0);
}





Chibi::Chibi(unsigned int serial_baud_rate){
  // this->rtc = new PCF2127(0, 0, 0, Saboten::RTC_CHIPSELECT_PIN);
  // chibiCmdInit(serial_baud_rate);

  // pinMode(Saboten::HIGH_GAIN_MODE_PIN, OUTPUT);
  // digitalWrite(Saboten::HIGH_GAIN_MODE_PIN, LOW);

  // pinMode(Saboten::RTC_CHIPSELECT_PIN, OUTPUT);
  // digitalWrite(Saboten::RTC_CHIPSELECT_PIN, HIGH);

  // pinMode(Saboten::SD_CHIPSELECT_PIN, INPUT);
  // digitalWrite(Saboten::SD_CHIPSELECT_PIN, HIGH);

  // pinMode(Saboten::SD_DETECT_PIN, INPUT);
  // digitalWrite(Saboten::SD_DETECT_PIN, LOW);

  

  // this->rtc->enableMinuteInterrupt();
  // // this->rtc->enableSecondInterrupt();
  // this->rtc->setInterruptToPulse();
  // attachInterrupt(2, this->rtcInterrupt, FALLING);

  chibiInit();
  //  set up chibi regs for external P/A
  // chibiRegWrite(0x4, 0xA0);
  // // high gain mode
  // digitalWrite(Saboten::HIGH_GAIN_MODE_PIN, HIGH);
}

void Chibi::read_board_diagnostics(unsigned char *buffer){
  this->read_battery_voltage(buffer); 
}

void Chibi::read_battery_voltage(unsigned char *buffer){
  unsigned int vbat = analogRead(Chibi::BATTERY_VOLTAGE_PIN);
  double batt = ((vbat/1023.0) * Chibi::ADC_REFERENCE_VOLTAGE) * 2;
  Reading_new battery_voltage = {"vbat", batt, 0};
  add_to_tx_buf_new_new(buffer, &battery_voltage);
}


