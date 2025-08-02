#include "OSCMessage.h"
#include <StreamString.h>
#include <errno.h>
#include <lwip/netdb.h>
#include <lwip/sockets.h>

#undef write
#undef read

struct CustomUDP : public Print {

  
  int udp_server=-1;
  IPAddress remote_ip ;
  uint16_t remote_port = 0;
  char *tx_buffer = nullptr;
  size_t tx_buffer_len = 0;

  int beginPacket(IPAddress ip, uint16_t port) {

    remote_port = port;
    remote_ip = ip;

    // allocate tx_buffer if is necessary
    if (!tx_buffer) {
      tx_buffer = (char *)malloc(1460);
      if (!tx_buffer) {
        log_e("could not create tx buffer: %d", errno);
        return 0;
      }
    }

    tx_buffer_len = 0;

    // check whereas socket is already open
    if (udp_server != -1)
      return 1;

    if ((udp_server = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
      log_e("could not create socket: %d", errno);
      return 0;
    }

    // fcntl(udp_server, F_SETFL, O_NONBLOCK);
    fcntl(udp_server, F_SETFL, 0);

    // //make receive calls nonblocking
    // struct timeval tm;
    // tm.tv_sec = 0;
    // tm.tv_usec = 1000;

    // int err = setsockopt(udp_server, SOL_SOCKET, SO_SNDTIMEO, &tm, sizeof(tm));
    // if (err < 0)
    // {
    //     log_e("setsockopt failed. error: %d", err);
    //     return 0;
    // }

    return 1;
  }

  size_t write(uint8_t data) override {
    if (tx_buffer_len == 1460) {
      Serial.print("packet too long, fragmenting!!!");
      endPacket();
      tx_buffer_len = 0;
    }
    tx_buffer[tx_buffer_len++] = data;
    return 1;
  }

  size_t write(const uint8_t *buffer, size_t size) {
    size_t i;
    for (i = 0; i < size; i++)
      write(buffer[i]);
    return i;
  }

  bool endPacket() {
    struct sockaddr_in recipient;
    recipient.sin_addr.s_addr = (uint32_t)remote_ip;
    recipient.sin_family = AF_INET;
    recipient.sin_port = htons(remote_port);
    int sent = sendto(udp_server, tx_buffer, tx_buffer_len, 0,
                      (struct sockaddr *)&recipient, sizeof(recipient));
    if (sent < 0) {
      log_e("could not send data: %d", errno);
      return false;
    }
    return true;
  }
};
