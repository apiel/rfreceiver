#include <iostream>
#include <stdio.h>
#include <fstream>
#include <sys/time.h>
#include <jsoncpp/json/json.h> // or jsoncpp/json.h , or json/json.h etc.

using namespace std;

void schedulerRealtime() {
	struct sched_param p;
	p.__sched_priority = sched_get_priority_max(SCHED_RR);
	if( sched_setscheduler( 0, SCHED_RR, &p ) == -1 ) {
	    perror("Failed to switch to realtime scheduler.");
	}
}

string digitalRead(string gpio) {
  string line;
  ifstream myfile (gpio.c_str());
  if (myfile.is_open())
  {
    getline (myfile,line);
    myfile.close();
  }

  return line;
}

unsigned long getCurrentMicroTime(void)
{
    struct timeval t;
    unsigned long micros;
    gettimeofday(&t, NULL);
    micros = t.tv_sec * 1000000L + t.tv_usec;
    
    return micros;
}

unsigned long lastPulseMicro = getCurrentMicroTime();
string lastPulseValue = "0";

// unsigned long pulseIn(string gpio) // 2
// {
//     unsigned long currentMicrosTime;
//     unsigned long micros;
//     string gpioValue;

//     do {
//         currentMicrosTime = getCurrentMicroTime();
//         micros = currentMicrosTime - lastPulseMicro;
//         gpioValue = digitalRead(gpio);
//     } while (gpioValue == lastPulseValue && micros < 1000000);

//     lastPulseMicro = currentMicrosTime;
//     lastPulseValue = gpioValue;

//     // cout << micros << endl;
//     return micros;
// }

// unsigned long pulseIn(string gpio) // 3
// {
//     long micros;
//     unsigned long now;
//     unsigned long start = getCurrentMicroTime();
//     do {
//         now = getCurrentMicroTime();
//         micros = now - start;
//     } while (digitalRead(gpio) != "0" && micros < 1000000);

//     do {
//         now = getCurrentMicroTime();
//         micros = now - start;
//     } while (digitalRead(gpio) == "0" && micros < 1000000);

// cout << micros << endl;

//     return micros;
// }

// unsigned long pulseIn(string gpio) // 3
// {
//     long micros;
//     unsigned long now;
//     unsigned long start = getCurrentMicroTime();
//     string value = digitalRead(gpio);
//     do {
//         now = getCurrentMicroTime();
//         micros = now - start;
//     } while (digitalRead(gpio) != value && micros < 1000000);

// cout << micros << endl;

//     return micros;
// }

unsigned long pulseIn(string gpio) // 1
{
   struct timeval tn, t0, t1;
   long micros = 0;
   gettimeofday(&t0, NULL);
   while (digitalRead(gpio) != "0")
   {
      gettimeofday(&tn, NULL);
      if (tn.tv_sec > t0.tv_sec) micros = 1000000L; else micros = 0;
      micros += (tn.tv_usec - t0.tv_usec);
      if (micros > 1000000) return 0;
   }
   gettimeofday(&t1, NULL);
   while (digitalRead(gpio) == "0")
   {
      gettimeofday(&tn, NULL);
      if (tn.tv_sec > t0.tv_sec) micros = 1000000L; else micros = 0;
      micros += (tn.tv_usec - t0.tv_usec);
      if (micros > 1000000) return 0;
   }
   if (tn.tv_sec > t1.tv_sec) micros = 1000000L; else micros = 0;
   micros += (tn.tv_usec - t1.tv_usec);
   return micros;
}

string getCode(string gpio, int zeroMin, int zeroMax, int oneMin, int oneMax, int bit) {
    unsigned long t;
    string code = ""; 
    while(bit > 0){ // we could also do while code.length() < bit
        t = pulseIn(gpio);
        if(t > oneMin && t < oneMax) {
            code += "1";
            bit--;
        }
        else if(t > zeroMin && t < zeroMax) {
            code += "0";
            bit--;
        }
    }
    return code;
}

bool latch(string gpio, int latchMin, int latchMax) {
    if (latchMin > 0 && latchMax > 0) {
        unsigned long t;
        for (int retry = 0; retry < 10; retry++) { // max retry 10
            t = pulseIn(gpio);
            if (t > latchMin && t < latchMax) {
                return true;
            }
        }
        return false;
    }
    return true;
}

int main() {
    schedulerRealtime();

    ifstream ifs("config.json");
    Json::Reader reader;
    Json::Value obj;
    reader.parse(ifs, obj); // reader can also read strings

    string gpio = obj["gpio"].asString();
    const Json::Value& protocoles = obj["protocoles"]; // array of characters

    unsigned long t = 0;
	for(;;) {
        t = pulseIn(gpio);
        for (int i = 0; i < protocoles.size(); i++) {
            if (t > protocoles[i]["trigger"][0].asUInt() 
             && t < protocoles[i]["trigger"][1].asUInt()) {
                int latchMin = protocoles[i]["latch"][0].asUInt();
                int latchMax = protocoles[i]["latch"][1].asUInt();
                if (latch(gpio, latchMin, latchMax)) {
                    int zeroMin = protocoles[i]["zero"][0].asUInt();
                    int zeroMax = protocoles[i]["zero"][1].asUInt();
                    int oneMin = protocoles[i]["one"][0].asUInt();
                    int oneMax = protocoles[i]["one"][1].asUInt();
                    int bit = protocoles[i]["bit"].asUInt();

                    string code = getCode(gpio, zeroMin, zeroMax, oneMin, oneMax, bit);
                    cout << protocoles[i]["name"].asString() << " " << code << endl;
                }
            }
        }
    }
}

