package com.rocode.skrc;

interface IRCService {

     void rotate0();
     void rotate90();
     void rotate180();
     void rotate270();
     void rotateRecovery();
     
     boolean isSetDisallowPhoneCall();
     String getDisallowPhoneNumberPrefix();
}
