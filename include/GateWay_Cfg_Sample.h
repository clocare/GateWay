/********************************************************/ 
/* Author : Nourhan Mansour                             */
/* Date   : 3/6/2021                                    */
/* Version: 1.0                                         */
/* File   : GATEWAY_CFG.h                               */
/********************************************************/ 
#ifndef GATEWAY_CFG_SAMPLE_H
#define GATEWAY_CFG_SAMPLE_H

#define STM_SERIAL_RX              D7   
#define STM_SERIAL_TX              D8

#define WIFI_SSID               "SSID"
#define WIFI_PASSWORD           "PASS"
// No need
#define USERNAME                "USERNAME"
#define PASSWORD                 "PASSWORD"

#define POST_SENSORS             "/Sensor"
#define POST_LOGIN_PATIENT       "/loginPatient"

#define GET_CHECK_ID "/CheckPatient/" //+ ID appended
#define POST_LOGIN_EMP "/loginEmployee"
#define POST_Add_PATIENT "/addPatientByEmp"

#endif 