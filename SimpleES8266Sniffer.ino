#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "sdk_structs.h"
#include "ieee80211_structs.h"
#include "string_utils.h"

extern "C"
{
  #include "user_interface.h"
}


void WiFi_Sniffer_Frame_Handler(uint8_t *buff, uint16_t len){
  //The first layer, cast the rceived buffer into the SDK structure
  const wifi_promiscuous_frame_t *frame = (wifi_promiscuous_frame_t *)buff;
  //The second layer, define a pointer to where the 802.11 frame is within the structure
  const wifi_ieee80211_frame_t *poiframe = (wifi_ieee80211_frame_t *)frame -> payload;
  //The third layer, define pointers to the 802.11 frame header and payload
  const wifi_ieee80211_mac_hdr_t *hdr = &poiframe->hdr;
  const uint8_t *data = poiframe->payload;

  //Pointer to the Frame Control part of the frame header (802.11)
  const wifi_header_frame_control_t *frame_ctrl = (wifi_header_frame_control_t *)&hdr -> frame_ctrl;

  // Parse MAC addresses contained in packet header into human-readable strings
  char addr1[] = "00:00:00:00:00:00\0";
  char addr2[] = "00:00:00:00:00:00\0";
  char addr3[] = "00:00:00:00:00:00\0";

  mac2str(hdr->addr1, addr1);
  mac2str(hdr->addr2, addr2);
  mac2str(hdr->addr3, addr3);
    // Output info to serial
    
  if (frame_ctrl->subtype != BEACON) { // fuck beacon frames jesus der er mange. 
  Serial.printf("\n%s | %s | %s | %u | %02d | %u | %u(%-2u) | %-28s | %u | %u | %u | %u | %u | %u | %u | %u | ",
    addr1,
    addr2,
    addr3,
    wifi_get_channel(),
    frame->rx_ctrl.rssi,
    frame_ctrl->protocol,
    frame_ctrl->type,
    frame_ctrl->subtype,
    wifi_pkt_type2str((wifi_promiscuous_frame_type_t)frame_ctrl->type, (wifi_mgmt_subtypes_t)frame_ctrl->subtype),
    frame_ctrl->to_ds,
    frame_ctrl->from_ds,
    frame_ctrl->more_frag,
    frame_ctrl->retry,
    frame_ctrl->pwr_mgmt,
    frame_ctrl->more_data,
    frame_ctrl->wep,
    frame_ctrl->strict);
    
  }
  // Print ESSID if beacon
  /*
  if (frame_ctrl->type == WIFI_PKT_MGMT && frame_ctrl->subtype == BEACON)
  {
    const wifi_mgmt_beacon_t *beacon_frame = (wifi_mgmt_beacon_t*) poiframe->payload;
    char ssid[32] = {0};

    if (beacon_frame->tag_length >= 32)
    {
      strncpy(ssid, beacon_frame->ssid, 31);
    }
    else
    {
      strncpy(ssid, beacon_frame->ssid, beacon_frame->tag_length);
    }

    Serial.printf("%s", ssid);
  }
  */
  
  
}

 
#define CHANNEL_HOP_INTERVAL_MS   1000
static os_timer_t channelHop_timer;

void channelHop()
{
  // hoping channels 1-13

  uint8_t new_channel = wifi_get_channel() + 1;
  if (new_channel > 13) {
    new_channel = 1;
  }
  wifi_set_channel(new_channel);
}

void setup()
{
  // Serial setup
  Serial.begin(115200);
  delay(10);
  wifi_set_channel(1);

  // Wifi setup
  wifi_set_opmode(STATION_MODE);
  wifi_promiscuous_enable(0);
  WiFi.disconnect();

  // Set sniffer callback
  wifi_set_promiscuous_rx_cb(WiFi_Sniffer_Frame_Handler);
  wifi_promiscuous_enable(1);

  // Print header
  Serial.printf("\n\n     MAC Address 1|      MAC Address 2|      MAC Address 3| Ch| RSSI| Pr| T(S)  |           Frame type         |TDS|FDS| MF|RTR|PWR| MD|ENC|STR|   SSID");

    // setup the channel hoping callback timer
  os_timer_disarm(&channelHop_timer);
  os_timer_setfn(&channelHop_timer, (os_timer_func_t *) channelHop, NULL);
  os_timer_arm(&channelHop_timer, CHANNEL_HOP_INTERVAL_MS, 1);
  

}

//so a thing that is actually kind of amazing is that the handler that is executed in the setup function, enters its own loop "in the background" and runs on
// the os_timer makes an interrupt every once channelhop timer (1second) and performs a function


void loop()
{
 delay(1000);
 Serial.println();
 Serial.printf("Current channel: %u",wifi_get_channel());
}
