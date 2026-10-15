//esp01.c
#include <string.h>
#include <stdio.h>
#include "types.h"
#include "uart0.h"
#include "delay.h"
#include "lcd.h"


extern s8 buff[200];
extern volatile u8 i;   // NOTE: also change the definition in uart0.c to
                                    // "volatile u8 i;" and make sure your
                                    // UART RX interrupt is the thing incrementing it.

// Waits for at least 'min_bytes' to arrive in buff, or until timeout_ms elapses.
// Returns 1 if min_bytes were received, 0 if it timed out.
static u8 wait_rx(u8 min_bytes, u32 timeout_ms)
{
        u32 waited = 0;
        while(i < min_bytes && waited < timeout_ms)
        {
                delay_ms(1);
                waited++;
        }
        return (i >= min_bytes);
}

// Waits for the '>' send-prompt after AT+CIPSEND, or until timeout_ms elapses.
// Returns 1 if '>' was seen, 0 if it timed out.
static u8 wait_prompt(u32 timeout_ms)
{
        u32 waited = 0;
        while(waited < timeout_ms)
        {
                if(strchr(buff, '>') != 0)
                        return 1;
                delay_ms(1);
                waited++;
        }
        return 0;
}

void wifi_init(void)
{
        cmdLcd(0x01);
        cmdLcd(0x80);
        strLcd("AT");
        delay_ms(1000);
        UART0_Str("AT\r\n");
        i=0;memset(buff,'\0',200);
        wait_rx(4, 3000);
        delay_ms(500);
        buff[i] = '\0';
                cmdLcd(0x01);
                cmdLcd(0x80);
        strLcd(buff);
        delay_ms(2000);
        if(strstr(buff,"OK"))
        {
                cmdLcd(0xC0);
                strLcd("OK");
                delay_ms(1000);
        }
        else
        {
                cmdLcd(0xC0);
                strLcd("ERROR");
                delay_ms(1000);
                return;
        }
				 cmdLcd(0x01);
        cmdLcd(0x80);
        strLcd("ATE0");
        delay_ms(1000);
        UART0_Str("ATE0\r\n");
        i=0;memset(buff,'\0',200);
        wait_rx(4, 3000);
        delay_ms(500);
        buff[i] = '\0';
        cmdLcd(0x01);
        cmdLcd(0x80);
        strLcd(buff);
        delay_ms(2000);
        if(strstr(buff,"OK"))
        {
                cmdLcd(0xC0);
                strLcd("OK");
                delay_ms(1000);
        }
        else
        {
                cmdLcd(0xC0);
                strLcd("ERROR");
                delay_ms(1000);
                return;
        }


        cmdLcd(0x01);
        cmdLcd(0x80);
        strLcd("AT+CIPMUX");
        delay_ms(1000);
        UART0_Str("AT+CIPMUX=0\r\n");
        i=0;memset(buff,'\0',200);
        wait_rx(4, 3000);
        delay_ms(500);
        buff[i] = '\0';
				 cmdLcd(0x01);
        cmdLcd(0x80);
        strLcd(buff);
        delay_ms(2000);
        if(strstr(buff,"OK"))
        {
                cmdLcd(0xC0);
                strLcd("OK");
                delay_ms(1000);
        }
        else
        {
                cmdLcd(0xC0);
                strLcd("ERROR");
                delay_ms(1000);
                return;
        }

        cmdLcd(0x01);
        cmdLcd(0x80);
        strLcd("AT+CWQAP");
        delay_ms(1000);
        UART0_Str("AT+CWQAP\r\n");
        i=0;memset(buff,'\0',200);
        wait_rx(4, 3000);
        delay_ms(1500);
        buff[i] = '\0';
        cmdLcd(0x01);
        cmdLcd(0x80);
        strLcd(buff);
        delay_ms(2000);
        if(strstr(buff,"OK"))
        {
                cmdLcd(0xC0);
                strLcd("OK");
                delay_ms(1000);
        }
				  else
        {
                cmdLcd(0xC0);
                strLcd("ERROR");
                delay_ms(1000);
                return;
        }

}

void esp01_connectAP(void)
{
        cmdLcd(0x01);
        cmdLcd(0x80);
        strLcd("AT+CWJAP");
        delay_ms(1000);
        //need to change the wifi network name and password
        UART0_Str("AT+CWJAP=\"Nikhitha\",\"12345678\"\r\n");
        i=0;memset(buff,'\0',200);
        wait_rx(4, 10000);   // joining an AP can take several seconds
        delay_ms(2500);
        buff[i] = '\0';
        cmdLcd(0x01);
        cmdLcd(0x80);
        strLcd(buff);
        delay_ms(2000);
        if(strstr(buff,"WIFI CONNECTED"))
        {
                cmdLcd(0xC0);
                strLcd("OK");
                delay_ms(1000);
        }
				  else
        {
                cmdLcd(0xC0);
                strLcd("ERROR");
                delay_ms(1000);
                return;
        }
}
        
// Makes sure Wi-Fi is actually joined before we try to talk to the server.
// If AT+CIPSTATUS reports "STATUS:5" (Wi-Fi not connected -- e.g. router was
// off and just came back), this re-runs esp01_connectAP() a few times.
// Returns 1 if Wi-Fi is confirmed up, 0 if it gave up after retries.
static u8 ensure_wifi_connected(void)
{
        u8 retry;

        for(retry = 0; retry < 3; retry++)
        {
                cmdLcd(0x01);
                cmdLcd(0x80);
                strLcd("AT+CIPSTATUS");
                delay_ms(1000);
                UART0_Str("AT+CIPSTATUS\r\n");
                i=0;memset(buff,'\0',200);
                wait_rx(5, 3000);
                delay_ms(1000);
                buff[i] = '\0';
                cmdLcd(0x01);
                cmdLcd(0x80);
                strLcd(buff);
                delay_ms(1500);

                if(!strstr(buff,"STATUS:5"))
                {
                        // Not in the "Wi-Fi disconnected" state -- good to go.
                        return 1;
                }

                // Wi-Fi is down (router was off, etc.) -- try to rejoin.
                cmdLcd(0x01);
                cmdLcd(0x80);
                strLcd("WIFI LOST");
                delay_ms(1000);
                cmdLcd(0xC0);
                strLcd("RECONNECTING");
                delay_ms(1000);

                esp01_connectAP();
        }

        // Still not connected after retries.
        cmdLcd(0x01);
        cmdLcd(0x80);
        strLcd("WIFI FAILED");
        delay_ms(1000);
        return 0;
}

void esp01_sendToThingspeak(s8 *val)
{
        s8 cmd[32],getReq[64];
        u32 payloadLen;
        u8 alreadyConnected = 0;

        // Make sure Wi-Fi is actually up first (handles router going off/on).
        if(!ensure_wifi_connected())
        {
                return;   // couldn't reconnect -- nothing more to do this cycle
        }

        // Check current link status first, so we don't blindly try to open
        // a new connection when one is already up (or in a bad state).
        cmdLcd(0x01);
        cmdLcd(0x80);
        strLcd("AT+CIPSTATUS");
        delay_ms(1000);
        UART0_Str("AT+CIPSTATUS\r\n");
        i=0;memset(buff,'\0',200);
        wait_rx(5, 3000);
        delay_ms(1000);
        buff[i] = '\0';
        cmdLcd(0x01);
        cmdLcd(0x80);
        strLcd(buff);
        delay_ms(1500);

        if(strstr(buff,"STATUS:3"))
        {
                // Already connected to the server, no need to CIPSTART again.
                alreadyConnected = 1;
        }
        else
        {
                cmdLcd(0x01);
                cmdLcd(0x80);
                strLcd("AT+CIPSTART");
                delay_ms(1000);
                UART0_Str("AT+CIPSTART=\"TCP\",\"api.thingspeak.com\",80\r\n");
                i=0;memset(buff,'\0',200);
                wait_rx(5, 5000);
                delay_ms(2500);
                buff[i] = '\0';
                cmdLcd(0x01);
                cmdLcd(0x80);
                strLcd(buff);
                delay_ms(2000);
        }

        if(alreadyConnected || strstr(buff,"CONNECT") || strstr(buff,"ALREADY CONNECTED"))
        {
                cmdLcd(0xC0);
                strLcd("OK");
                delay_ms(1000);

                // Build the actual HTTP GET request first so we know its exact length,
                // then tell the ESP that exact byte count via AT+CIPSEND.
                // NOTE: make sure 'cmd' / your GET buffer is large enough for the
                // longest 'val' you expect (temperature/humidity readings etc.)
                //need to change the thingspeak write API key accordind to your channel
                sprintf(getReq,"GET /update?api_key=K34KI5XQ9DBIS5K9&field1=%s\r\n\r\n", val);
                payloadLen = strlen(getReq);
				cmdLcd(0xd4);
				u32Lcd(payloadLen);
                cmdLcd(0x01);
                cmdLcd(0x80);
                strLcd("AT+CIPSEND");
                delay_ms(1000);
                sprintf(cmd, "AT+CIPSEND=%u\r\n", payloadLen);
                i=0;memset(buff,'\0',200);
                UART0_Str(cmd);
                wait_prompt(2000);   // wait for '>' before sending payload, with timeout fallback
                delay_ms(300);

                i=0;memset(buff,'\0',200);
                UART0_Str(getReq);
                wait_rx(5, 8000);
                delay_ms(2000);
                buff[i] = '\0';
                cmdLcd(0x01);
                cmdLcd(0x80);
                strLcd(buff);
                delay_ms(2000);
                if(strstr(buff,"SEND OK"))
                {
                        cmdLcd(0x01);
                        strLcd("DATA UPDATED");
                        delay_ms(1000);
                }
                else
                {
                        cmdLcd(0x01);
                        strLcd("DATA NOT UPDATED");
                        delay_ms(1000);
                }

                // Close the connection so the next send doesn't rely on
                // "ALREADY CONNECTED" and the socket doesn't linger open.
                cmdLcd(0x01);
                cmdLcd(0x80);
                strLcd("AT+CIPCLOSE");
                delay_ms(1000);
                UART0_Str("AT+CIPCLOSE\r\n");
                i=0;memset(buff,'\0',200);
                wait_rx(5, 3000);
                delay_ms(1000);
                buff[i] = '\0';
                cmdLcd(0x01);
                cmdLcd(0x80);
                strLcd(buff);
                delay_ms(1500);

        }
        else
        {
                cmdLcd(0xC0);
                strLcd("ERROR");
                delay_ms(1000);
                return;
        }


}