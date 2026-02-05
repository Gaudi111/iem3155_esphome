// iem_datetime.h
// Utility to parse Schneider iEM3000 DATETIME fields returned as HEXBYTES.
// Returns "YYYY-MM-DD HH:MM" or "unknown" on invalid data.
//
// Suggested PATH for Home Assistant: /config/esphome/includes/iem_datetime.h
//
// For referencing in yaml file for ESPHome (your specific configuration may change):
//
// esphome:
// name: iem3155_gateway
// platform: ESP8266
// board: nodemcuv2
//  includes:
//    - iem_datetime.h   # looks for /config/esphome/includes/iem_datetime.h
//
// Example sensor:
// 
// text_sensor:
//   - platform: modbus_controller
//     modbus_controller_id: iem3155
//     id: meter_current_date_raw
//     name: "Meter Current Date Raw"
//     register_type: holding
//     address: 1844          # manual 1845–1848; ESPHome address = 1845 - 1
//     register_count: 4
//     response_size: 8
//     raw_encode: HEXBYTES
// 
//   - platform: template
//     id: meter_current_date
//     name: "Meter Current Date"
//     update_interval: 60s
//     lambda: |-
//       return parse_schneider_datetime_hex(id(meter_current_date_raw).state);
// 
// Format reference (Schneider DOCA0005EN-15):
//   Word0: Year (0..99) -> 2000 + (W0 & 0x7F)
//   Word1: Month (bits 11..8), Weekday (7..5), Day (4..0)
//   Word2: Hour (bits 12..8), Minute (5..0)
//   Word3: Millisecond (not used in the final string)
//
// This helper expects a hex string containing 8 bytes (16 hex chars).
// It strips any non-hex characters before parsing.

#pragma once
#include <string>
#include <vector>
#include <cstdio>
#include <cctype>

inline std::string parse_schneider_datetime_hex(const std::string& hex_in) {
  if (hex_in.empty()) return "unknown";

  // Keep only hex characters
  std::string hex;
  hex.reserve(hex_in.size());
  for (char c : hex_in) {
    if (std::isxdigit(static_cast<unsigned char>(c))) hex.push_back(c);
  }
  if (hex.size() < 16) return "unknown";  // we need at least 8 bytes

  // Convert 2 hex chars -> one byte
  auto hex2nib = char x -> int {
    if (x >= '0' && x <= '9') return x - '0';
    if (x >= 'A' && x <= 'F') return x - 'A' + 10;
    if (x >= 'a' && x <= 'f') return x - 'a' + 10;
    return -1;
  };

  auto hex2byte = char h, char l -> int {
    int hi = hex2nib(h), lo = hex2nib(l);
    if (hi < 0 || lo < 0) return -1;
    return (hi << 4) | lo;
  };

  std::vector<uint8_t> bytes;
  bytes.reserve(hex.size() / 2);
  for (size_t i = 0; i + 1 < hex.size(); i += 2) {
    int b = hex2byte(hex[i], hex[i + 1]);
    if (b < 0) return "unknown";
    bytes.push_back(static_cast<uint8_t>(b));
  }
  if (bytes.size() < 8) return "unknown";

  // Words are big-endian per Modbus word
  auto word_at = int idx -> uint16_t {
    return (static_cast<uint16_t>(bytes[idx * 2]) << 8) |
           static_cast<uint16_t>(bytes[idx * 2 + 1]);
  };

  const uint16_t w_year = word_at(0);
  const uint16_t w_mowd = word_at(1);
  const uint16_t w_hm   = word_at(2);
  // const uint16_t w_ms = word_at(3);  // not used

  // Decode per Schneider DATETIME
  const int year   = 2000 + (w_year & 0x7F);       // 0..99 -> 2000..2099
  const int month  = (w_mowd >> 8) & 0x0F;         // bits 11..8
  const int day    =  w_mowd       & 0x1F;         // bits 4..0
  const int hour   = (w_hm >> 8)   & 0x1F;         // bits 12..8
  const int minute =  w_hm         & 0x3F;         // bits 5..0

  // Basic sanity checks
  if (month < 1 || month > 12 || day < 1 || day > 31 ||
      hour  < 0 || hour  > 23  || minute < 0 || minute > 59) {
    return "unknown";
  }

  char out[24];
  std::snprintf(out, sizeof(out), "%04d-%02d-%02d %02d:%02d",
                year, month, day, hour, minute);
  return std::string(out);
}
