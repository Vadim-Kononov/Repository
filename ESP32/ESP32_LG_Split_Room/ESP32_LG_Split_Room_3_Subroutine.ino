// Отправка кода на излучатель
void Ac_Send_Code(int cod)
{  
  irsend.sendLG(command_value [cod], 28);
  SendMqtt ("split/code",command_name [cod]);
// Логика для подсчета температуры охлаждения при включенном сплите  
  if (cod == 0) {split_flag = true; tem_stored = tem_split;}
  if (cod == 1) {split_flag = false;}
}


// Перевод секунд в минуты и часы
String Hour_display (int timer)
{
  int hour, minute, second;
  String st_hour, st_minute, st_second;

  hour = timer / 3600;
  minute = (timer % 3600) / 60;
  second = (timer % 3600) % 60;

  if (hour >= 10 && hour <= 99) st_hour = String (hour) + ":"; else if (hour > 0 && hour <= 9) st_hour = "0" + String (hour) + ":"; else st_hour = String ("");
  if (minute >= 10) st_minute = String (minute); else st_minute = "0" + String (minute);
  if (second >= 10) st_second = String (second); else st_second = "0" + String (second);

  return st_hour + st_minute + ":" + st_second;
}



// Отправка кода отключения по времени для страховки
void Break_Time (int sec)
{
if      (sec > 7200) Ac_Send_Code(9);
else if (sec > 3600) Ac_Send_Code(8);
else                 Ac_Send_Code(7); 
}
