#ifndef TPACKETDATA_H
#define TPACKETDATA_H

#pragma once
#include <stdint.h>
#include <string.h>

#define BEACON_FRAME 0
#define PROBE_REQUEST 1
#define PROBE_RESPONSE 2
#define QOS_DATA 3
#define QOS_NULL 4

#pragma pack(push, 1)
typedef struct present_flags{
    uint8_t tsft:1;
    uint8_t flags:1;
    uint8_t rate:1;
    uint8_t channel:1;
    uint8_t fhss:1;
    uint8_t dbm_antenna_sig:1; // antenna signal awlays 5th bits
    uint8_t dbm_antenna_noise:1;
    uint8_t lock_quality:1;
    uint8_t tx_attenuation:1;
    uint8_t db_tx_attenuation:1;
    uint8_t dbm_tx_power:1;
    uint8_t antenna:1;
    uint8_t db_antenna_sig:1;
    uint8_t db_antenna_noise:1;
    uint8_t rx_flags:1;
    uint8_t padding:3;
    uint8_t channel_plus:1;
    uint8_t mcs_info:1;
    uint8_t a_mpdu_stat:1;
    uint8_t vht_info:1;
    uint8_t frame_timestamp:1;
    uint8_t he_info:1;
    uint8_t he_mu_info:1;
    uint8_t padding2:1;
    uint8_t zero_len_psdu:1;
    uint8_t l_sig:1;
    uint8_t reserved:1;
    uint8_t radiotap_ns_next:1;
    uint8_t vendor_ns_next:1;
    uint8_t ext:1;
}Present_flags;

typedef struct Flags{
    uint8_t cfp:1;
    uint8_t preamble:1;
    uint8_t wep:1;
    uint8_t fragmentation:1;
    uint8_t fcs_at_end:1;
    uint8_t data_pad:1;
    uint8_t bad_fcs:1;
    uint8_t short_gi:1;
}Flags;

typedef struct Channel_flags{
    uint8_t padding:4;
    uint8_t turbo:1;
    uint8_t cck:1;
    uint8_t ofdm:1;
    uint8_t two_ghz:1;
    uint8_t five_ghz:1;
    uint8_t passive:1;
    uint8_t dynamic_cck_ofdm:1;
    uint8_t gaussian_freq_shift_keying:1;
    uint8_t gsm:1;
    uint8_t static_turbo:1;
    uint8_t half_rate_chan:1;
    uint8_t quater_rate_chan:1;
}Channel_flags;

typedef struct radiotap_header{
    uint8_t h_revision;
    uint8_t h_pad;
    uint16_t h_len;
    Present_flags presnt_flags;
}radiotap_header;

typedef struct Dot11_Frame_Control_Field{
    uint8_t Version:2;
    uint8_t Type:2;
    uint8_t Subtype:4;
    uint8_t Flags;
}Dot11_Frame_Control_Field;

typedef struct Dot11{
    Dot11_Frame_Control_Field Frame_Control_Field;
    uint16_t duration;
    uint8_t mac1[6];
    uint8_t mac2[6];
    uint8_t mac3[6];
    uint16_t number;
}Dot11;

typedef struct Dot11_data{
    Dot11_Frame_Control_Field Frame_Control_Field;
    uint16_t duration;
    uint8_t mac1[6];
    uint8_t mac2[6];
}Dot11_data;

typedef struct MAC{
    uint8_t mac[6];
    bool operator <(const MAC& var) const
    {
        return memcmp(mac, var.mac, sizeof(mac)) < 0;
    }
}MAC;

typedef struct CONV_MAC{
    uint8_t bssid[6];
    uint8_t station[6];
    bool operator <(const CONV_MAC& var) const
    {
        if(memcmp(bssid, var.bssid, sizeof(bssid)) != 0){
            return memcmp(bssid, var.bssid, sizeof(bssid)) < 0;
        }else{
            return memcmp(station, var.station, sizeof(station)) < 0;
        }
    }
}CONV_MAC;

typedef struct Beacon_values{
    uint8_t PWR;
    uint16_t CH;
    int Beacons = 1;
    char ssid[30];

    bool operator <(const Beacon_values& var) const
    {
        if(PWR != var.PWR){
            return PWR < var.PWR;
        }else if(CH != var.CH){
            return CH < var.CH;
        }else if(Beacons != var.Beacons){
            return Beacons < var.Beacons;
        }else{
            return memcmp(ssid, var.ssid, sizeof(ssid)) < 0;
        }
    }
}Beacon_values;

typedef struct Probe_values{
    uint8_t PWR;
    int Frames = 1;
    uint8_t probe[30];

    bool operator <(const Probe_values& var) const
    {
        if(PWR != var.PWR){
            return PWR < var.PWR;
        }else if(Frames != var.Frames){
            return Frames < var.Frames;
        }else{
            return memcmp(probe, var.probe, sizeof(probe)) < 0;
        }
    }
}Probe_values;

bool compare(Beacon_values& a, Beacon_values& b){
    return a < b;
}
bool compare(Probe_values& a, Probe_values& b){
    return a < b;
}

#pragma pack(pop)

#endif // TPACKETDATA_H
