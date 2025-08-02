#include <WiFiUdp.h>

#define CUSTOM_OSC 1
#define ASYNC_OSC 0
#if CUSTOM_OSC
#include "CustomUDP.h"
CustomUDP udp;
#elif ASYNC_OSC
#include <AsyncUDP.h>
AsyncUDP udp;
AsyncUDPMessage cachedAsyncMsg;

#else
WiFiUDP udp;
#endif
WiFiUDP udpRcv;
#include <OSCMessage.h>


String macroName = "macro4";
String globMacroName = "macroGlobal4";
OSCMessage cachedFloatMsg("/obsidian/session");
OSCMessage cachedBRecMsg("/obsidian/session");
String bRecMacroName = String("macroGlobal") + String(0);
OSCMessage cachedB0Msg("/obsidian/session");
String b0MacroName = String("macroGlobal") + String(1);
OSCMessage cachedB1Msg("/obsidian/session");
String b1MacroName = String("macroGlobal") + String(2);

float minPedal = 0;
float maxPedal = 1;

enum ButtonMode { RecMode, ToggleMode };
ButtonMode buttonMode = RecMode;

void setButtonMode(int n) {
  if (n == 0)
    buttonMode = RecMode;
  else
    buttonMode = ToggleMode;
}
void setMacroNum(int n){
    if(n<0 || n>10){
        Serial.println ("macro num out of range");
        return;
    }
    // TODO set configurable
    if (n == 0) { // kick tune
      minPedal = 0.5;
      maxPedal = .75;
    } else if (n == 2) { // samples tune
      minPedal = 0.25;
      maxPedal = .75;
    } else if (n == 3) { // samples cut
      minPedal = 0.1;
      maxPedal = 1;
    } else { // default pedal
      minPedal = 0;
      maxPedal = 1;
    }
    macroName = String("macro") + String(n);
    cachedFloatMsg.set(0, macroName.c_str());
}
void setupOSC() {
  cachedFloatMsg.add(macroName.c_str()).add(float(0.f));
  cachedBRecMsg.add(bRecMacroName.c_str()).add(float(0.f));
  cachedB0Msg.add(b0MacroName.c_str()).add(float(0.f));
  cachedB1Msg.add(b1MacroName.c_str()).add(float(0.f));
  udpRcv.begin(WiFi.localIP(), udpLocalPort);
  //   udp.begin(WiFi.localIP(), udpLocalPort);
}

void sendMsg(OSCMessage &msg, const IPAddress &addr) {
#if ASYNC_OSC
#pragma error not supported ?
  cachedAsyncMsg.flush();
  msg.send(cachedAsyncMsg);
  udp.sendTo(cachedAsyncMsg, udpAddress, udpRemotePort, TCPIP_ADAPTER_IF_STA);
  // return true;
#else
  udp.beginPacket(addr, udpRemotePort);
  msg.send(udp);
  // return bool(
  udp.endPacket();
  // );
#endif
}
bool broadCastMsg(OSCMessage &msg, bool onlypi1 = false) {

  for (auto &a : pi1UdpAddresses)
    sendMsg(msg, a);

  if (!onlypi1)
    for (auto &a : pi2UdpAddresses)
      sendMsg(msg, a);

  return true;
}

bool sendFloatMsg(float v) {
  cachedFloatMsg.set(1, float(v));
  broadCastMsg(cachedFloatMsg);
  OSCMessage globPedalMsg(&cachedFloatMsg);
  globPedalMsg.set(0, globMacroName.c_str());
  broadCastMsg(globPedalMsg);

  return true;
}

bool sendBtnMsg(int bNum, bool v) {
  if (buttonMode == RecMode) {
    cachedBRecMsg.set(1, bNum ? 1.f : 0.f);
    broadCastMsg(cachedBRecMsg, true);
  } else {
    if (bNum == 0) {
      cachedB0Msg.set(1, v ? 1.f : 0.f);
      broadCastMsg(cachedB0Msg, true);
    } else if (bNum == 1) {
      cachedB1Msg.set(1, v ? 1.f : 0.f);
      broadCastMsg(cachedB1Msg, true);
    } else {
      Serial.println("cant find button ");
    }
  }
  Serial.println("sent Btn " + String(bNum) + " :: " + String(v ? "1" : "0"));
  return true;
}

bool receiveOSC(OSCMessage &bundle) {

  //   return false;
  // while (udpRcv.available()) {

  int size = udpRcv.parsePacket();
  if (size == 0)
    return false;
  while (size > 0) {
    // Serial.print("size : ");
    // Serial.println(size);
    while (size--) {
      bundle.fill(udpRcv.read());
    }
    // if (bundle.hasError() || ) {
    // size = udpRcv.parsePacket();

    //   while (size--) {
    //     bundle.fill(udpRcv.read());
    //   }
  }
  if (udpRcv.available()) {
    Serial.print("more udp data available");
  }

  if (bundle.hasError()) {
    Serial.print("bundle err : ");
    Serial.println(bundle.getError());
    return false;
  }
  if (bundle.fullMatch("/macroNum")) {
    if (bundle.isInt(0))
      setMacroNum( bundle.getInt(0));
    else if (bundle.isFloat(0))
      setMacroNum(int(bundle.getFloat(0)));
    else
      Serial.println("unknown type for macronum");
  }
  if (bundle.fullMatch("/buttonMode")) {
    if (bundle.isInt(0))
      setButtonMode(bundle.getInt(0));
    else if (bundle.isFloat(0))
      setButtonMode(int(bundle.getFloat(0)));
    else
      Serial.println("unknown type for macronum");
  }
  return true;
}

#if 0

void sendOSCResp(OSCMessage &m) {
    udpRcv.beginPacket(udpRcv.remoteIP(), udpRcv.remotePort());
    m.send(udpRcv);
    udpRcv.endPacket();
    udpRcv.flush();
  
    // DBGOSC((String("sent resp : ") + m.getAddress() + "(" + String(m.size()) +
    //         " args) , ".remoteIP().toString() + " " +
    //         String(udpRcv.remotePort()))
    //            .c_str());
  }
#endif  
