/////////////////////////////////////////////////////////////////////////////
//
// 2023-01-01 13:00:00.000
// 01234567890123456789012
//           1         2
//
void parseReceivedTimeString(char* timeString) {
  if(strlen(timeString) < 23) {
    return;
  }
  char year[5];
  char month[3];
  char day[3];
  char hour[3];
  char minute[3];
  char second[3];
  char ms[4];
  memset(year, 0, sizeof(year));
  memset(month, 0, sizeof(month));
  memset(day, 0, sizeof(day));
  memset(hour, 0, sizeof(hour));
  memset(minute, 0, sizeof(minute));
  memset(second, 0, sizeof(second));
  memset(ms, 0, sizeof(ms));

  strncpy(year, &timeString[0], 4);
  strncpy(month, &timeString[5], 2);
  strncpy(day, &timeString[8], 2);
  strncpy(hour, &timeString[11], 2);
  strncpy(minute, &timeString[14], 2);
  strncpy(second, &timeString[17], 2);
  strncpy(ms, &timeString[20], 3);

  startYear = atoi(year);
  startMonth = atoi(month);
  startDay = atoi(day);
  startHour = atoi(hour);
  startMinute = atoi(minute);
  startSecond = atoi(second);
  startMs = atoi(ms);
  startTimeMs = millis();
  startSecOfDay = (startHour * 3600L) + (startMinute * 60L) + startSecond;

  currentYear = startYear;
  currentMonth = startMonth;
  currentDay = startDay;
  currentHour = startHour;
  currentMinute = startMinute;
  currentSecond = startSecond;
  currentMs = startMs;
  currentSecOfDay = startSecOfDay;
}

void calculateTime() {
  // startSecOfDay = (startHour * 3600) + (startMinute * 60) + startSecond;

  DateTime now = rtc.now();
  currentHour = now.hour();
  currentMinute = now.minute();
  currentSecond = now.second();
  currentDay = now.day();
  currentMonth = now.month();
  currentYear = now.year();
  
  currentSecOfDay = (currentHour * 3600L) + (currentMinute * 60L) + currentSecond;
  timeOfDay = currentSecOfDay / 3600.0D;
}

