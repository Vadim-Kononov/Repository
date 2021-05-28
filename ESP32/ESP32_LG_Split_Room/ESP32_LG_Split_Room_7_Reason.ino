// Получение причины последней перезагрузки
String print_reset_reason(RESET_REASON reason)
{

  String value;
  switch ( reason)
  {
    case 1 : value = "POWERON_RESET"; break;          /**<1,  Vbat power on reset*/
    case 3 : value = "SW_RESET"; break;               /**<3,  Software reset digital core*/
    case 4 : value = "OWDT_RESET"; break;             /**<4,  Legacy watch dog reset digital core*/
    case 5 : value = "DEEPSLEEP_RESET"; break;        /**<5,  Deep Sleep reset digital core*/
    case 6 : value = "SDIO_RESET"; break;             /**<6,  Reset by SLC module, reset digital core*/
    case 7 : value = "TG0WDT_SYS_RESET"; break;       /**<7,  Timer Group0 Watch dog reset digital core*/
    case 8 : value = "TG1WDT_SYS_RESET"; break;       /**<8,  Timer Group1 Watch dog reset digital core*/
    case 9 : value = "RTCWDT_SYS_RESET"; break;       /**<9,  RTC Watch dog Reset digital core*/
    case 10 : value = "INTRUSION_RESET"; break;       /**<10, Instrusion tested to reset CPU*/
    case 11 : value = "TGWDT_CPU_RESET"; break;       /**<11, Time Group reset CPU*/
    case 12 : value = "SW_CPU_RESET"; break;          /**<12, Software reset CPU*/
    case 13 : value = "RTCWDT_CPU_RESET"; break;      /**<13, RTC Watch dog Reset CPU*/
    case 14 : value = "EXT_CPU_RESET"; break;         /**<14, for APP CPU, reseted by PRO CPU*/
    case 15 : value = "RTCWDT_BROWN_OUT_RESET"; break;/**<15, Reset when the vdd voltage is not stable*/
    case 16 : value = "RTCWDT_RTC_RESET"; break;      /**<16, RTC Watch dog reset digital core and rtc module*/
    default : value = "NO_MEAN";
  }
  return value;
}
