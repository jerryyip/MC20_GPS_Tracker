/*
 * MC20_BT.cpp
 * A library for SeeedStudio GPS Tracker BT 
 *
 * Copyright (c) 2017 seeed technology inc.
 * Website    : www.seeed.cc
 * Author     : lawliet zou, lambor
 * Create Time: April 2017
 * Change Log :
 *
 * The MIT License (MIT)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "MC20_GNSS.h"


bool GNSS::initialize()
{
    return true;
}

bool GNSS::open_GNSS(int mode)
{
  bool ret = true;

  switch(mode){
    case GNSS_DEFAULT_MODE:
      ret = open_GNSS_default_mode();   // Default GNSS mode
      break;
    case EPO_QUICK_MODE:
      ret = open_GNSS_EPO_quick_mode(); // Quick mode with EPO
      break;
    case EPO_LP_MODE:
      ret = open_GNSS_EPO_LP_mode();   // Low power consumption mode with EPO
      break;
    case EPO_RL_MODE:
      ret = open_GNSS_RL_mode();     // Reference-location mode
      break;
  };
  
  return ret;
}

bool GNSS::open_GNSS_default_mode(void)
{
  return open_GNSS();
}

bool GNSS::open_GNSS_EPO_quick_mode(void)
{
  //Open GNSS funtion
  if(!open_GNSS()){
    return false;
  }

  //
  if(!settingContext()){
    return false;
  }

  // Check network register status
  if(!isNetworkRegistered()){
    return false;
  }

  // Check time synchronization status
  if(!isTimeSynchronized()){
    return true;  // Return true to work on 
  }

  // Enable EPO funciton
  if(!enableEPO()){
    return false;
  }

  // Trigger EPO funciton
  if(!triggerEPO()){
    return false;
  }

  return true;
}
bool GNSS::open_GNSS_EPO_LP_mode(void)
{
  //
  if(!settingContext()){
    return false;
  }

  // Check network register status
  if(!isNetworkRegistered()){
    return false;
  }

  // Check time synchronization status
  if(!isTimeSynchronized()){
    //Open GNSS funtion
    if(!open_GNSS()){
      return false;
    }
    return true;  // Return true to work on 
  }

  // Enable EPO funciton
  if(!enableEPO()){
    return false;
  }

  // Trigger EPO funciton
  if(!triggerEPO()){
    return false;
  }

  //Open GNSS funtion
  if(!open_GNSS()){
    return false;
  }

  return true;
}
bool GNSS::open_GNSS_RL_mode(void)
{
  int errCounts = 0;
  char buffer[128];
  MC20_clean_buffer(buffer, 128);

  //
  if(!settingContext()){
    return false;
  }

  // Check network register status
  if(!isNetworkRegistered()){
    return false;
  }

  // Check time synchronization status
  if(!isTimeSynchronized()){
    //Open GNSS funtion
    if(!open_GNSS()){
      return false;
    }
    return true;  // Return true to work on 
  }

  // Write in reference-location
  // sprintf(buffer, "AT+QGREFLOC=%f,%f\n\r", ref_longitude, ref_latitude);
  sprintf(buffer, "AT+QGREFLOC=22.584322,113.966678\n\r");
  if(!MC20_check_with_cmd("AT+QGREFLOC=22.584322,113.966678\n\r", "OK", CMD, 2, 2000)){
    errCounts++;
    if(errCounts > 3)
    {
      return false;
    }
    delay(1000);
  }

  // Enable EPO funciton
  if(!enableEPO()){
    return false;
  }

  // Trigger EPO funciton
  if(!triggerEPO()){
    return false;
  }

  //Open GNSS funtion
  if(!open_GNSS()){
    return false;
  }

  return true;
}

bool GNSS::getCoordinate(void)
{
    
    int i = 0;
    int j = 0;
    int tmp = 0;
    char *p = NULL;
    char buffer[1024];
    char strLine[128];
    char *header = "$GNGGA,";

    p = &header[0];

    MC20_clean_buffer(buffer, 1024);
    MC20_send_cmd("AT+QGNSSRD?\n\r");
    MC20_read_buffer(buffer, 1024, 2);
    // SerialUSB.println(buffer);
    if(NULL != strstr("+CME ERROR:", buffer))
    {
      return false;
    }
    while(buffer[i] != '\0'){
        if(buffer[i] ==  *(p+j)){
            j++;
            // SerialUSB.print(i);
            // SerialUSB.println(buffer[i]);
            if(j >= 7) {
                p = &buffer[i];
                i = 0;
                while(*(p++) != '\n'){
                    // SerialUSB.write(*p);
                    strLine[i++] = *p;
                }
                strLine[i] = '\0';
                //SerialUSB.println(strLine);  // 093359.000,2235.0189,N,11357.9816,E,2,17,0.80,35.6,M,-2.5,M,,*51
                p = strtok(strLine, ",");
                p = strtok(NULL, ",");
                longitude = strtod(p, NULL);
                tmp = (int)(longitude / 100);
                longitude = (double)(tmp + (longitude - tmp*100)/60.0);
                p = strtok(NULL, ",");
                p = strtok(NULL, ",");
                latitude = strtod(p, NULL);
                tmp = (int)(latitude / 100);
                latitude = (double)(tmp + (latitude - tmp*100)/60.0);
                break;
            }
        } else {
            j = 0;
        }
        i++;
    }

    return true;
}

bool GNSS::dataFlowMode(void)
{
    // Make sure that "#define UART_DEBUG" is uncomment.
    return MC20_check_with_cmd("AT+QGNSSRD?\n\r", "OK", CMD);   
}

bool GNSS::open_GNSS(void)
{
  int errCounts = 0;

  //Open GNSS funtion
  while(!MC20_check_with_cmd("AT+QGNSSC?\n\r", "+QGNSSC: 1", CMD, 2, 2000)){
      errCounts ++;
      if(errCounts > 5){
        return false;
      }
      MC20_check_with_cmd("AT+QGNSSC=1\n\r", "OK", CMD, 2, 2000);
      delay(1000);
  }

  return true;
}

bool GNSS::settingContext(void)
{
  int errCounts = 0;

  //Setting context
  while(!MC20_check_with_cmd("AT+QIFGCNT=2\n\r", "OK", CMD, 2, 2000)){
    errCounts++;
    if(errCounts > 3)
    {
      return false;
    }
    delay(1000);
  }

  return true;
}

bool GNSS::isNetworkRegistered(void)
{
  int errCounts = 0;

  //
  while(!MC20_check_with_cmd("AT+CREG?\n\r", "+CREG: 0,1", CMD, 2, 2000)){
    errCounts++;
    if(errCounts > 30)    // Check for 30 times
    {
      return false;
    }
    delay(1000);
  }

  errCounts = 0;
  while(!MC20_check_with_cmd("AT+CGREG?\n\r", "+CGREG: 0,1", CMD, 2, 2000)){
    errCounts++;
    if(errCounts > 30)    // Check for 30 times
    {
      return false;
    }
    delay(1000);
  }

  return true;
}

bool GNSS::isTimeSynchronized(void)
{
  int errCounts = 0;

  // Check time synchronization status
  errCounts = 0;
  while(!MC20_check_with_cmd("AT+QGNSSTS?\n\r", "+QGNSSTS: 1", CMD, 2, 2000)){
    errCounts++;
    if(errCounts > 10)
    {
      return true;
    }
    delay(1000);
  }  

  return true;
}

bool GNSS::enableEPO(void)
{
  int errCounts = 0;

  //
  if(!MC20_check_with_cmd("AT+QGNSSEPO=1\n\r", "OK", CMD, 2, 2000)){
    errCounts++;
    if(errCounts > 3)
    {
      return false;
    }
    delay(1000);
  }
  
  return true;
}

bool GNSS::triggerEPO(void)
{
  int errCounts = 0;

  //
  if(!MC20_check_with_cmd("AT+QGEPOAID\n\r", "OK", CMD, 2, 2000)){
    errCounts++;
    if(errCounts > 3)
    {
      return false;
    }
    delay(1000);
  }

  return true;
}
