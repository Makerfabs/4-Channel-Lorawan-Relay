#include <stdio.h>
#include <string.h>
#include <SoftwareSerial.h>

#define DEBUG true

#define AT_TIMEOUT 3000

#define DEVEUI "70B3D57ED0058C91"
#define APPEUI "0000000000000000"
#define APPKEY "04182A42F99A4809AA76619F61C89ED7"

#define LORAWAN_TX 3
#define LORAWAN_RX 4
#define LORAWAN_RST 10

#define RELAY_1 6
#define RELAY_2 7
#define RELAY_3 A3
#define RELAY_4 A2

const int relay_list[4] = {RELAY_1, RELAY_2, RELAY_3, RELAY_4};

int led_flag = 0;
SoftwareSerial softSerial1(LORAWAN_TX, LORAWAN_RX);

void setup()
{
    Serial.begin(9600);
    softSerial1.begin(9600); // your esp's baud rate might be different
    delay(1000);

    Serial.println("Restarting....");

    relay_flash();

    pinMode(LORAWAN_RST, OUTPUT);

    digitalWrite(LORAWAN_RST, 0);
    delay(2000);
    digitalWrite(LORAWAN_RST, 1);
    Serial.println("Lorawan module restart over.");
    delay(5000);

    sendData("AT", 3000, DEBUG);
    sendData("AT", 3000, DEBUG);

    sendData("AT+CDEVEUI=" + String(DEVEUI), 3000, DEBUG);
    sendData("AT+CAPPEUI=" + String(APPEUI), 3000, DEBUG);
    sendData("AT+CAPPKEY=" + String(APPKEY), 3000, DEBUG);
    sendData("AT+CJOINMODE=0", 3000, DEBUG);
    sendData("AT+CCLASS=0", 30000, DEBUG);
    sendData("AT+CJOIN=1,0,8,8", 30000, DEBUG);
}

long int runtime = 20000;
void loop()
{
    if ((millis() - runtime) > 10000)
    {

        String downlink = "";
        if (led_flag == 1)
            downlink = lorawan_tx_rx("F8", 20000);
        if (led_flag == 0)
            downlink = lorawan_tx_rx("F7", 20000);

        if (downlink.endsWith("F8"))
            led_flag = 1;
        if (downlink.endsWith("F7"))
            led_flag = 0;

        Serial.print("LED STATUS:");
        if (led_flag == 1)
        {
            Serial.println("ALL ON");
            for (int j = 0; j < 4; j++)
            {
                digitalWrite(relay_list[j], HIGH);
                delay(200);
            }
        }
        else
        {

            Serial.println("ALL OFF");
            for (int j = 0; j < 4; j++)
            {
                digitalWrite(relay_list[j], LOW);
                delay(200);
            }
        }
        runtime = millis();
    }
    while (softSerial1.available() > 0)
    {
        Serial.write(softSerial1.read());
        yield();
    }
}

String sendData(String command, const int timeout, boolean debug)
{
    String response = "";
    softSerial1.println(command);
    long int time = millis();
    while ((time + timeout) > millis())
    {
        while (softSerial1.available())
        {
            char c = softSerial1.read();
            response += c;
        }
    }
    if (debug)
    {
        Serial.print(response);
    }
    return response;
}

String lorawan_tx_rx(String msg_hex, const int timeout)
{
    String response = "";
    String rec_head = "OK+RECV:";

    char msg[50] = "";
    sprintf(msg, "AT+DTRX=1,2,5,%s", msg_hex.c_str());

    Serial.println(msg);
    softSerial1.println(msg);

    long int time = millis();
    while ((time + timeout) > millis())
    {
        while (softSerial1.available())
        {
            char c = softSerial1.read();

            if (c == '\r')
                continue;
            else if (c == '\n')
            {
                Serial.println(response);

                if (response.indexOf(rec_head) != -1)
                {
                    Serial.println("-----------Get downlink msg-----------");

                    String result = response.substring(response.indexOf(rec_head) + rec_head.length());

                    Serial.println(result);
                    Serial.println("-----------Over-----------");
                    return result;
                }
                else if (response.indexOf("ERR+SENT") != -1)
                {
                    Serial.println("-----------Get downlink msg-----------");

                    String result = "SEND FAILED";

                    Serial.println(result);
                    Serial.println("-----------Over-----------");
                    return result;
                }

                response = "";
            }
            else
                response += c;
        }
    }
    Serial.println(response);

    return "";
}

void relay_flash()
{
    pinMode(RELAY_1, OUTPUT);
    pinMode(RELAY_2, OUTPUT);
    pinMode(RELAY_3, OUTPUT);
    pinMode(RELAY_4, OUTPUT);

    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            digitalWrite(relay_list[j], HIGH);
            delay(200);
        }
        for (int j = 0; j < 4; j++)
        {
            digitalWrite(relay_list[j], LOW);
            delay(200);
        }
    }
}