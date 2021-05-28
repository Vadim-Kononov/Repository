// ThingSpeak, IFTTT и отправка GET запроса
void SendThingSpeak (String key, String value_1, String value_2, String value_3, String value_4, String value_5, String value_6, String value_7, String value_8)
{
  String url = "https://api.thingspeak.com/update?api_key=" + key + "&field1=" + value_1 + "&field2=" + value_2 + "&field3=" + value_3 + "&field4=" + value_4 + "&field5=" + value_5 + "&field6=" + value_6 + "&field7=" + value_7 + "&field8=" + value_8;
  url.replace(" ", "%20");
  GET ("api.thingspeak.com", url);
}



void SendIFTTT (String key, String event_name, String value_1, String value_2, String value_3)
{
  String url = "https://maker.ifttt.com/trigger/" + event_name + "/with/key/" + key + "?value1=" + value_1 + "&value2=" + value_2 + "&value3=" + value_3;
  url.replace(" ", "%20");
  GET ("maker.ifttt.com", url);
}


void GET (String host_string, String url)
{
  digitalWrite(Onboard_Led, HIGH);

  char host [30];
  host_string.toCharArray(host, host_string.length() + 1);


#ifdef Serial_On_GET
  Serial.print("\nConnecting to ");
  Serial.println(host);
#endif

  WiFiClient client;

  if (!client.connect((char*)host, 80))
  {
#ifdef Serial_On_GET
    Serial.println("Connection failed !");
#endif
    return;
  }

#ifdef Serial_On_GET
  Serial.print("Requesting URL: ");
  Serial.println(url);
#endif

  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n");
  unsigned long timeout = millis();
  while (client.available() == 0)
  {
    if (millis() - timeout > 3000)
    {
#ifdef Serial_On_GET
      Serial.println(">>> Client Timeout !");
#endif
      client.stop();
      return;
    }
  }
  // Read all the lines of the reply from server and print them to Serial
#ifdef Serial_On_GET
  while (client.available()) {
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }
  Serial.println("\n\nСlosing connection\n");
#endif

  digitalWrite(Onboard_Led, LOW);
}
