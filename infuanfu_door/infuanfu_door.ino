#include <SPI.h>
#include "PN532_SPI.h"
#include "PN532.h"
#include "NfcAdapter.h"



class Job {
  String _name = "unnamed";
  
  protected:
  Job(){}
  Job(String name) {
    _name = name;
  }
  
  public:
  String name() { return _name; }
  virtual void run() = 0;
};

class ScheduledJob {
  public:
  unsigned long _when_millis;
  bool _after_rollover;
  Job* _job;
  bool _active;

  void clear() {
    _active = false;
    _after_rollover = false;
    _job = 0;
    _when_millis = 0;
  }

  void setTo(Job* job, unsigned long when, bool rollover) {
    _job = job;
    _when_millis = when;
    _after_rollover = rollover;
    _active = true;
  }
};

class Reactor {
  private:
  unsigned long _current_millis = 0;
  unsigned long _previous_millis = 0;
  #define MAX_JOBS 10
  ScheduledJob _jobs[MAX_JOBS] = { ScheduledJob() };
  byte _jobs_active = 0;

  ScheduledJob& getFreeScheduledJob() {
    for(byte i=0; i<MAX_JOBS;++i) {
      if(!_jobs[i]._active) {
        return _jobs[i];
      }
    }
    Serial.println("ERROR: overwriting last job slot because we ran out of jobs.");
    return _jobs[MAX_JOBS-1];
  }

  void fixRolloverJobs() {
    for(byte i=0; i<MAX_JOBS;++i) {
      if(_jobs[i]._active && _jobs[i]._after_rollover) {
        _jobs[i]._after_rollover = false;
      }
    }
  }

  Job* getNextReadyJob(unsigned long now) {
    for(byte i=0; i<MAX_JOBS;++i) {
      ScheduledJob& cj = _jobs[i];
      Serial.println("DEBUG: inspecting job " + cj._job->name() + " runnable at " + cj._when_millis + ", aro=" + cj._after_rollover + ", active=" + cj._active);
       
      if(_jobs[i]._active && !_jobs[i]._after_rollover && _jobs[i]._when_millis <= now) {
        // TODO: get the one with the least when_millis value
        Job* job = _jobs[i]._job;
        
        // free the scheduling slot
        _jobs[i].clear();
        _jobs_active--;

        // return the job
        return job;
      }
    }
    
    // nothing to do
    return 0;
  }
  
  public:
  void schedule(Job* job, unsigned long in_millis) {
    
    if(_jobs_active == MAX_JOBS-1) {
      Serial.println("ERROR: cannot schedule job. no more slots.");
      return;
    }

    unsigned long now = millis();
    unsigned long when = now + in_millis;

    ScheduledJob& sched = getFreeScheduledJob();
    sched.setTo(job, when, when < now);
    _jobs_active++;
    Serial.println("DEBUG: scheduled job " + job->name() + ". now active: " + _jobs_active);
  }

  void run() {
    _previous_millis = _current_millis;
    _current_millis = millis();

    if(_current_millis < _previous_millis) {
      // we had a rollover
      Serial.println("DEBUG: handling rollover");
      fixRolloverJobs();
    }

    Job* job = 0;
    while((job = getNextReadyJob(_current_millis)) != 0) {
      Serial.println("DEBUG: running job: " + job->name());
      job->run();      
    }
    
    Serial.println("DEBUG: no more jobs to run.");
  }
};


String const myUID = "70 4D 7B 28"; // replace this UID with your NFC tag's UID
int const greenLedPin = 3; // green led active for correct key notification
int const redLedPin = 4; // red led active for incorrect key notification

PN532_SPI interface(SPI, 10); // create a SPI interface for the shield with the SPI CS terminal at digital pin 10
NfcAdapter nfc = NfcAdapter(interface); // create an NFC adapter object

Reactor reactor = Reactor();

class WatchdogJob : public Job {
  public:
  WatchdogJob() : Job("WatchdogJob") {
  }

  virtual void run() {
    Serial.println("hello from watchdog");
    reactor.schedule(this,1000);
  }
};

WatchdogJob watchdogJob = WatchdogJob();

void setup(void) {
    Serial.begin(115200); // start serial comm
    Serial.println("infuanfu door lock");
    reactor.schedule(&watchdogJob, 0);
    /*
    Serial.println("NDEF Reader");
    nfc.begin(); // begin NFC comm
    
    // make LED pins outputs
    pinMode(greenLedPin,OUTPUT);
    pinMode(redLedPin,OUTPUT);
    
    // turn off the LEDs
    digitalWrite(greenLedPin,LOW);
    digitalWrite(redLedPin,LOW);
    */
}




void loop(void) {
  /*
    Serial.println("Scanning...");
    if (nfc.tagPresent()) // check if an NFC tag is present on the antenna area
    {
        NfcTag tag = nfc.read(); // read the NFC tag
        String scannedUID = tag.getUidString(); // get the NFC tag's UID

        Serial.print("ID: ");
        Serial.println(scannedUID);
        
        if( myUID.compareTo(scannedUID) == 0) // compare the NFC tag's UID with the correct tag's UID (a match exists when compareTo returns 0)
        {
          // The correct NFC tag was active
          Serial.println("Correct Key");
          // Blink the green LED and make sure the RED led is off
          digitalWrite(greenLedPin,HIGH);
          digitalWrite(redLedPin,LOW);
          
          delay(1500);
          digitalWrite(greenLedPin,LOW);
          // put your here to trigger the unlocking mechanism (e.g. motor, transducer)
        }else{
          // an incorrect NFC tag was active
          Serial.println("Incorrect key");
          // blink the red LED and make sure the green LED is off
          digitalWrite(greenLedPin,LOW);
          digitalWrite(redLedPin,HIGH);
          
          delay(500);
          digitalWrite(redLedPin,LOW);
          delay(500);
          digitalWrite(redLedPin,HIGH);
          delay(500);
          digitalWrite(redLedPin,LOW);
          // DO NOT UNLOCK! an incorrect NFC tag was active. 
          // put your code here to trigger an alarm (e.g. buzzard, speaker) or do something else
        }
    }
    */

  reactor.run();
  delay(2000);
}
