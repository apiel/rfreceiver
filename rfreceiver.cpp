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

int pulseIn(string gpio, int timeout)
{
   struct timeval tn, t0, t1;
   long micros;
   gettimeofday(&t0, NULL);
   micros = 0;
   while (digitalRead(gpio) != "0")
   {
      gettimeofday(&tn, NULL);
      if (tn.tv_sec > t0.tv_sec) micros = 1000000L; else micros = 0;
      micros += (tn.tv_usec - t0.tv_usec);
      if (micros > timeout) return 0;
   }
   gettimeofday(&t1, NULL);
   while (digitalRead(gpio) == "0")
   {
      gettimeofday(&tn, NULL);
      if (tn.tv_sec > t0.tv_sec) micros = 1000000L; else micros = 0;
      micros = micros + (tn.tv_usec - t0.tv_usec);
      if (micros > timeout) return 0;
   }
   if (tn.tv_sec > t1.tv_sec) micros = 1000000L; else micros = 0;
   micros = micros + (tn.tv_usec - t1.tv_usec);
   return micros;
}

int main() {
    schedulerRealtime();

    ifstream ifs("config.json");
    Json::Reader reader;
    Json::Value obj;
    reader.parse(ifs, obj); // reader can also read strings

    string gpio = obj["gpio"].asString();
    const Json::Value& protocoles = obj["protocoles"]; // array of characters


    cout << "Gpio: " << gpio << endl;
    for (int i = 0; i < protocoles.size(); i++){
        cout << "name: " << protocoles[i]["name"].asString() << endl;
        cout << "bit: " << protocoles[i]["bit"].asUInt() << endl;
        cout << "trigger between " << protocoles[i]["trigger"][0].asUInt()
             << " and "  << protocoles[i]["trigger"][1].asUInt() << endl;
        cout << "zero between " << protocoles[i]["zero"][0].asUInt()
             << " and "  << protocoles[i]["zero"][1].asUInt() << endl;
        cout << "one between " << protocoles[i]["one"][0].asUInt()
             << " and "  << protocoles[i]["one"][1].asUInt() << endl;
        cout << endl;
    }


    unsigned long t = 0;
	for(;;) {
        t = pulseIn(gpio, 1000000);
        for (int i = 0; i < protocoles.size(); i++){
            if (t > protocoles[i]["trigger"][0].asUInt() 
             && t < protocoles[i]["trigger"][1].asUInt()) {
                string code = ""; 
                int zeroMin = protocoles[i]["zero"][0].asUInt();
                int zeroMax = protocoles[i]["zero"][1].asUInt();
                int oneMin = protocoles[i]["one"][0].asUInt();
                int oneMax = protocoles[i]["one"][1].asUInt();
                int bit = protocoles[i]["bit"].asUInt();

                while(bit > 0){ // we could also do while code.length() < bit
                    t = pulseIn(gpio, 1000000);
                    if(t > oneMin && t < oneMax) {
                        code += "1";
                        bit--;
				    }
                    else if(t > zeroMin && t < zeroMax) {
                        code += "0";
                        bit--;
				    }
                }
                cout << protocoles[i]["name"].asString() << " " << code << endl;
            }
        }
    }
}

