void loop()
{       
if (getUpdates())
{
  if (memory.getUInt("mes_id")!= message_id && !str_text.equalsIgnoreCase("null"))
  {
    memory.putUInt("mes_id", message_id);
    memory.putInt("mes_count", memory.getInt("mes_count") + 1);
    memory.putInt("zatknis", memory.getInt("zatknis") - 1);
    
    if (chat_id < 0) flag_chat = true; else flag_chat = false; 
    
    Log();
    /*
    Serial.println(int64ToAscii(chat_id));
    Serial.println(str_first_name);
    Serial.println(user_id);
    Serial.println(message_id);
    Serial.println(str_text);
    */
    if (memory.getBool("listen") && !flag_chat) {str_answer = (String) update_id + "\n" + message_id + "\n" + user_id + "\n" + str_first_name + "\n"  + int64ToAscii(chat_id) + "\n"  + str_chat_title + "\n\n" + str_text; sendMessage(main_id);}
    
    switch (user_id)
    {
    case gordei_id:
    str_first_name = "Гордей";
    break;
    case plot_id:
    str_first_name = "Плот";
    break;
    case dimon_id:
    str_first_name = "Димон";
    break;   
    case main_id:
    str_first_name = "Кон";
    break; 
    case valera_id:
    str_first_name = "Крапивин";
    break;
    default:
    str_first_name = str_first_name;
    }

    str_speech = "";
    str_answer = "";
    memory.putInt("moment", 0);
    if (chat_id == -1001347460420) time_waiting = millis();
    
    if        (str_text.indexOf("/")>=0)   Manag ();
    else
    {
      str_text = LowCase (str_text);
      if        (str_text.indexOf("заткнись")>=0) {memory.putInt("zatknis", 10);}
      else if   (str_text.indexOf("хуля")>=0) {Randomstr_speech("/2.txt");}
      else if   (str_text.indexOf("5")>=0 || str_text.indexOf("прив")>=0 || str_text.indexOf("hi")>=0 || str_text.indexOf("Hi")>=0 || str_text.indexOf("HI")>=0) {Randomstr_speech("/4.txt");} 
      else if   (str_text.indexOf("ганд")>=0 || str_text.indexOf("гонд")>=0) {Randomstr_speech("/5.txt");} 
      else if   (str_text.indexOf(" я ")>=0 || str_text.indexOf("дума")>=0 || str_text.indexOf("кажет")>=0) {Randomstr_speech("/6.txt");} 
      else
          {
          Searchstr_speech("/1.txt"); 
          if (str_speech == 0) {if (memory.getInt("mes_count") > random(3, 6)) Randomstr_speech("/2.txt");} 
          }
    }

  
    if (str_speech !=0 && memory.getInt("zatknis") <=0)
    {
    str_answer = "";
    int key = random(0, 100); 
    if (key % 2 == 0) str_answer += str_first_name + ", " + str_speech; else str_answer += FirstCharUp (str_speech);
    memory.putInt("mes_count", 0);
    sendMessage(chat_id);
    }
    else if (memory.getInt("zatknis") == 10) {str_answer = "Молчу, в пизде торчу"; sendMessage(chat_id);}




}
}

if (millis() - time_waiting > 60000){time_waiting = millis(); memory.putInt("moment", memory.getInt("moment") + 1);}
if (memory.getInt("moment") > 60) {memory.putInt("moment", 0); Randomstr_speech("/3.txt"); str_answer = str_speech; sendMessage(-1001347460420); str_answer = "";}
delay(1000);
/*Сброс сторожевого таймера*/
xTimerChangePeriod(timerWatchDog,  pdMS_TO_TICKS(10000), 0);
}
