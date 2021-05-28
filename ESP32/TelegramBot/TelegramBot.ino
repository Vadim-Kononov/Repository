#include "FS.h"
#include "SD.h"
#include "SPI.h"

#include <WiFiClientSecure.h>
#define ARDUINOJSON_USE_LONG_LONG   1 
#define ARDUINOJSON_DECODE_UNICODE  1  
#include <ArduinoJson.h>

#include <Preferences.h>
#include "soc/rtc_cntl_reg.h"

// This is GandiStandardSSLCA2.pem, the root Certificate Authority that signed 
// the server certifcate for the demo server https://jigsaw.w3.org in this
// example. This certificate is valid until Sep 11 23:59:59 2024 GMT
const char* rootCACertificate = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIDxTCCAq2gAwIBAgIBADANBgkqhkiG9w0BAQsFADCBgzELMAkGA1UEBhMCVVMx\n" \
"EDAOBgNVBAgTB0FyaXpvbmExEzARBgNVBAcTClNjb3R0c2RhbGUxGjAYBgNVBAoT\n" \
"EUdvRGFkZHkuY29tLCBJbmMuMTEwLwYDVQQDEyhHbyBEYWRkeSBSb290IENlcnRp\n" \
"ZmljYXRlIEF1dGhvcml0eSAtIEcyMB4XDTA5MDkwMTAwMDAwMFoXDTM3MTIzMTIz\n" \
"NTk1OVowgYMxCzAJBgNVBAYTAlVTMRAwDgYDVQQIEwdBcml6b25hMRMwEQYDVQQH\n" \
"EwpTY290dHNkYWxlMRowGAYDVQQKExFHb0RhZGR5LmNvbSwgSW5jLjExMC8GA1UE\n" \
"AxMoR28gRGFkZHkgUm9vdCBDZXJ0aWZpY2F0ZSBBdXRob3JpdHkgLSBHMjCCASIw\n" \
"DQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAL9xYgjx+lk09xvJGKP3gElY6SKD\n" \
"E6bFIEMBO4Tx5oVJnyfq9oQbTqC023CYxzIBsQU+B07u9PpPL1kwIuerGVZr4oAH\n" \
"/PMWdYA5UXvl+TW2dE6pjYIT5LY/qQOD+qK+ihVqf94Lw7YZFAXK6sOoBJQ7Rnwy\n" \
"DfMAZiLIjWltNowRGLfTshxgtDj6AozO091GB94KPutdfMh8+7ArU6SSYmlRJQVh\n" \
"GkSBjCypQ5Yj36w6gZoOKcUcqeldHraenjAKOc7xiID7S13MMuyFYkMlNAJWJwGR\n" \
"tDtwKj9useiciAF9n9T521NtYJ2/LOdYq7hfRvzOxBsDPAnrSTFcaUaz4EcCAwEA\n" \
"AaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMCAQYwHQYDVR0OBBYE\n" \
"FDqahQcQZyi27/a9BUFuIMGU2g/eMA0GCSqGSIb3DQEBCwUAA4IBAQCZ21151fmX\n" \
"WWcDYfF+OwYxdS2hII5PZYe096acvNjpL9DbWu7PdIxztDhC2gV7+AJ1uP2lsdeu\n" \
"9tfeE8tTEH6KRtGX+rcuKxGrkLAngPnon1rpN5+r5N9ss4UXnT3ZJE95kTXWXwTr\n" \
"gIOrmgIttRD02JDHBHNA7XIloKmf7J6raBKZV8aPEjoJpL1E/QYVN8Gb5DKj7Tjo\n" \
"2GTzLH4U/ALqn83/B2gX2yKQOC16jdFU8WnjXzPKej17CuPKf1855eJ1usV2GDPO\n" \
"LPAvTK33sefOT6jEm0pUBsV/fdUID+Ic/n4XuKxe9tQWskMJDE32p2u0mYRlynqI\n" \
"4uJEvlz36hz1\n" \
"-----END CERTIFICATE-----\n";




WiFiClientSecure telegramClient;
Preferences memory;

#define TELEGRAM_HOST  "api.telegram.org"
#define TELEGRAM_PORT       443
#define SIZE_MAX            3072
#define SIZE_MIN            2816
#define SIZE_STR_SPEECH     1000
#define NUMBER_OF_ROWS      100
#define FILES_SUM           6

char* m_token = "1698490767:AAEuJZKQfuTiEDcw-7nuDS3vn3p-uV4mWuU"; //Таня
//char* m_token = "1594675437:AAE3ZqDLEtyABcpe4xXU3IglJrq2CWRBWyg"; //Тест

bool
flag_chat = true;

uint8_t
note,
marker,
point;


int32_t
user_id,
message_id,
update_id,
time_waiting;

const int32_t
gordei_id = 1531122286,
plot_id = 388900696,
dimon_id = 96728541,
main_id = 1067229173,
valera_id = 1640573929;

int64_t
chat_id;

String
str_response,
str_first_name,
str_chat_title,
str_text,
str_speech,
str_answer,
str_temp,
str_1,
str_2,
str_3,
str_4,
str_5,
str_6,
str_7,
str_8;


TimerHandle_t timerWatchDog = xTimerCreate("timerWatchDog", pdMS_TO_TICKS(10000), pdTRUE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(WatchDog));
