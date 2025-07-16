#ifndef SECRETS_H
#define SECRETS_H

const char* OTA_USER = "user";
const char* OTA_PASS = "pass";

const char* WIFI_SSID = "SSID";
const char* WIFI_PASS = "SSID_PASS";

/*
 * IP: 192.168.0.10
 * GateWay: 192.168.0.254
 * Subnet: 255.255.255.0 /24
 * DNS: 8.8.8.8
*/
IPAddress local_IP(192, 168, 0, 10);
IPAddress gateway(192, 168, 0, 254);
IPAddress subnet(255, 255, 255, 0);
IPAddress dns(8, 8, 8, 8);

#endif
