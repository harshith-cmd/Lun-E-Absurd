#include "network.h"
#include "drive.h"
#include "page_index.h"
#include "NetworkEvents.h"
#include "NetworkInterface.h"
#include "NetworkManager.h"
#include "ESP32_NOW.h"
#include "WiFi.h"
#include "esp_http_server.h"
#include <lwip/sockets.h>

static httpd_handle_t webServer = NULL;

// ==================== ESP-NOW ====================
class ControllerPeer : public ESP_NOW_Peer {
public:
  ControllerPeer(const uint8_t *mac, uint8_t ch)
      : ESP_NOW_Peer(mac, ch, WIFI_IF_AP, nullptr) {}
  bool begin() { return add(); }
  void onReceive(const uint8_t *data, size_t len, bool broadcast) override {
    if (len != sizeof(ControlPacket)) return;
    ControlPacket rx;
    memcpy(&rx, data, sizeof(rx));
    drive(rx.throttle, rx.turn);
  }
};

static std::vector<ControllerPeer *> controllers;

static void onNewPeer(const esp_now_recv_info_t *info, const uint8_t *data, int len, void *arg) {
  if (memcmp(info->des_addr, ESP_NOW.BROADCAST_ADDR, 6) != 0) return;
  ControllerPeer *peer = new ControllerPeer(info->src_addr, activeChannel);
  if (!peer->begin()) { delete peer; return; }
  controllers.push_back(peer);
  Serial.printf("[NOW] Paired CH%d\n", activeChannel);
}

// ==================== HTTP / WEBSOCKET ====================
static esp_err_t wsHandler(httpd_req_t *req) {
  if (req->method == HTTP_GET) return ESP_OK;
  httpd_ws_frame_t frame;
  uint8_t buf[32];
  memset(&frame, 0, sizeof(frame));
  frame.payload = buf;
  frame.type = HTTPD_WS_TYPE_TEXT;
  esp_err_t ret = httpd_ws_recv_frame(req, &frame, sizeof(buf) - 1);
  if (ret != ESP_OK) return ret;
  buf[frame.len] = 0;
  int t = 0, s = 0;
  char *comma = strchr((char *)buf, ',');
  if (comma) { *comma = 0; t = atoi((char *)buf); s = atoi(comma + 1); }
  drive(t, s);
  return ESP_OK;
}

static esp_err_t indexHandler(httpd_req_t *req) {
  httpd_resp_set_type(req, "text/html");
  return httpd_resp_send(req, PAGE_INDEX, strlen(PAGE_INDEX));
}

void startWebServer() {
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.server_port = 80;
  config.stack_size = 8192;
  config.max_uri_handlers = 4;
  config.recv_wait_timeout = 1;
  config.send_wait_timeout = 1;
  config.open_fn = [](httpd_handle_t hd, int sockfd) -> esp_err_t {
    int nodelay = 1;
    setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &nodelay, sizeof(nodelay));
    return ESP_OK;
  };

  if (httpd_start(&webServer, &config) != ESP_OK) return;

  httpd_uri_t wsRoute = { "/ws", HTTP_GET, wsHandler, NULL, .is_websocket = true };
  httpd_register_uri_handler(webServer, &wsRoute);

  httpd_uri_t indexRoute = { "/", HTTP_GET, indexHandler, NULL };
  httpd_register_uri_handler(webServer, &indexRoute);

  Serial.println("[WEB] Server on :80");
}

// ==================== NETWORK SETUP ====================
void setupNetwork() {
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASS, DEFAULT_CHANNEL);
  WiFi.setSleep(false);
  activeChannel = DEFAULT_CHANNEL;

  if (!ESP_NOW.begin()) ESP.restart();
  ESP_NOW.onNewPeer(onNewPeer, nullptr);
  Serial.printf("[NET] AP=%s IP=%s CH=%d\n", AP_SSID, WiFi.softAPIP().toString().c_str(), activeChannel);
}

void networkLoop() {}
