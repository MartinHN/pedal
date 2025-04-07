#include <WiFiUdp.h>
WiFiUDP udp;
WiFiUDP udpRcv;
#include <OSCMessage.h>


String macroName = "macro4";
float minPedal = 0;
float maxPedal = 1;

void setMacroNum(int n){
    if(n<0 || n>10){
        Serial.println ("macro num out of range");
        return;
    }
    // TODO set configurable
    if (n == 0) {
      minPedal = 0.5;
      maxPedal = .75;
    } else if (n == 2) {
      minPedal = 0.25;
      maxPedal = .75;
    } else {
      minPedal = 0;
      maxPedal = 1;
    }
    macroName = String("macro")+String(n);

}
void setupOSC() { udpRcv.begin(udpLocalPort); }


void sendMsg(float v) {
  OSCMessage msg("/obsidian/session");
  msg.add(macroName.c_str()).add(v);
  udp.beginPacket(udpAddress, udpRemotePort);
  msg.send(udp);
  udp.endPacket();
}

bool receiveOSC(OSCMessage &bundle) {

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
