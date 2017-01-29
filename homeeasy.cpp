#include <iostream>
#include <stdio.h>
#include <fstream>
#include <sys/time.h>

#include <unistd.h> // usleep

using namespace std;

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

unsigned long pulseIn(string gpio)
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
    string gpio = "/sys/class/gpio_sw/PA0/data";

    unsigned long t = 0;
	for(;;) {
        t = pulseIn(gpio);

            if (t > 10500 
             && t < 11500) {
                if (latch(gpio, 2400, 2600)) {



int bit;
int prevBit;
unsigned long sender = 0;
unsigned int recipient = 0;
int bitCount = 0;
int command;
int group;
for(;;) {
            t = pulseIn(gpio);
			if(t > 200 && t < 365)
			{
				bit = 0;
			}
			else if(t > 1000 && t < 1360)
			{
				bit = 1;
			}
			
			if(bitCount % 2 == 1)
			{
				if((prevBit ^ bit) == 0)
				{	// must be either 01 or 10, cannot be 00 or 11
                    // cout << "must be either 01 or 10, cannot be 00 or 11, normally get out" << endl;
                    break;
				}
				else if(bitCount < 53)
				{	// first 26 data bits
					
					sender <<= 1;
					sender |= prevBit;
				}
				else if(bitCount == 53)
				{	// 26th data bit
					
					group = prevBit;
				}
				else if(bitCount == 55)
				{	// 27th data bit
					
					command = prevBit;
				}
				else
				{	// last 4 data bits
					
					recipient <<= 1;
					recipient |= prevBit;
				}
			}
			
			prevBit = bit;
			bitCount++;
			
			if(bitCount == 64)
			{	// message is complete
				cout << "sender: " << sender;
                cout << " recipient: " << recipient;
                cout << " command: " << command;
                cout << " group: " << group << endl;
                break;
			}
}








                }
            }
        
        usleep(100); // save CPU
    }
}

