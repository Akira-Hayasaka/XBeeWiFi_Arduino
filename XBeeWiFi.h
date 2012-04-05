//
//  XBeeWiFi.h
//  
//
//  Created by Akira Hayasaka on 4/5/12.
//  Copyright (c) 2012 ﾟ･:,｡ﾟ･:,｡★ﾟ･:,｡ﾟ･:,｡☆ﾟ･:,｡ﾟ･:,｡★ﾟ･:,｡ﾟ･:,｡☆. All rights reserved.
//

#ifndef _XBeeWiFi_h
#define _XBeeWiFi_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include "../XBee/XBee.h"
#include "../SPI/SPI.h"
#include "../Ethernet/Ethernet.h"
#include "inttypes.h"


// the non-variable length of the frame data (not including frame id or api id or variable data size (e.g. payload, at command set value)
#define IPv4_TRANSMIT_REQUEST_API_LENGTH 10

/**
 * Api Id constants
 */
#define IPv4_TRANSMIT_REQUEST 0x20
#define IPv4_RX_FRAME 0xb0
#define IPv4_TRANSMIT_STATUS 0x89

/// status
#define MAC_FAILUE 0x01
#define PHYSICAL_ERROR 0x04
#define NETWORK_ACK_FAILUE 0x21
#define NOT_ASSOCIATED 0x22
#define NO_RESOURCES 0x32
#define CONNECTIONS_FAILUE 0x76

/// protocol
#define PROTOCOL_UDP 0
#define PROTOCOL_TCP 1

/// option
#define OPTION_LEAVEOPEN 1

/// security
#define SECURITY_OPEN 0
#define SECURITY_WPA 1
#define SECURITY_WPA2 2
#define SECURITY_WEP40 3
#define SECURITY_WEP104 4

/// modem status
#define JOINED_AP 0
#define INITIALIZATION 0x01
#define SSID_NOT_FOUND 0x22
#define SSID_NOT_CONFIGURED 0x23
#define JOIN_FAILED 0x27
#define WAITING_IPADDRESS 0x41
#define WAITING_SOCKETS 0x42
#define SCANNING_SSID 0xff

/// dns
#define DNS_QUERY_A 1
#define DNS_QUERY_NS 2
#define DNS_QUERY_CNAME 5
#define DNS_QUERY_PTR 12
#define DNS_QUERY_MX 15
#define DNS_QUERY_AAAA 28
#define DNS_QUERY_ANY 255
#define DNS_CLASS_IN 1

#define DNS_PORT 53
#define DNS_SRC_PORT 10053
#define DNS_TIMEOUT 15000 // ms

struct DNSHeader 
{
    uint16_t id; 
    uint16_t flags; 
    uint16_t questions; 
    uint16_t answers; 
    uint16_t authorities; 
    uint16_t additional; 
};

struct DnsQuestionEnd 
{ 
    uint16_t type; 
    uint16_t clas; 
};

struct DnsAnswer 
{
    uint16_t name; 
    uint16_t type; 
    uint16_t clas; 
    uint32_t ttl; 
    uint16_t length; 
} __attribute__((packed)); 


/**
 * Primary interface for communicating with an XBee Wi-Fi.
 */
class XBeeWiFi : public XBee 
{
public:
    XBeeWiFi();
    int setup (int security, const char *ssid, const char *pin);
    int setup (const char *ssid);
    int reset ();
    int baud (int b);
    int setAddress ();
    int setAddress (IPAddress &ipaddr, IPAddress &netmask, IPAddress &gateway, IPAddress &nameserver);
    int getAddress (IPAddress &ipaddr, IPAddress &netmask, IPAddress &gateway, IPAddress &nameserver);
    int setTimeout (int timeout);
    int getStatus ();
    int getWiResponse (int apiId, int frameid = 0, int timeout = 3000);
    int setNameserver (IPAddress &nameserver, int port);
    int getHostByName (const char* name, IPAddress &addr);
    
    /**
     * Call with instance of AtCommandResponse only if getApiId() == AT_COMMAND_RESPONSE
     */
    //    void getIPV4RxFrame (XBeeResponse &responses);
    
protected:
    int getWiAddr (IPAddress &ipaddr);
    int createDnsRequest (const char* name, char *buf);
    int getDnsResponse (const uint8_t *buf, int len, IPAddress &addr);
    
private:
    IPAddress _nameserver;
    int _nameport;
    
};

/**
 * Represents a Wi-Fi TX packet that corresponds to Api Id: IPv4_TRANSMIT_REQUEST
 */
class IPv4TransmitRequest : public PayloadRequest {
public:
    /**
     * Creates a unicast IPv4TransmitRequest with the DEFAULT_FRAME_ID
     */
    IPv4TransmitRequest(IPAddress &dstAddr, uint16_t dstPort, uint16_t srcPort, uint8_t protocol, uint8_t option, uint8_t *data, uint16_t dataLength, uint8_t frameId);
    IPv4TransmitRequest(IPAddress &dstAddr, uint16_t dstPort, uint8_t *data, uint16_t dataLength);
    /**
     * Creates a default instance of this class.  At a minimum you must specify
     * a payload, payload length and a destination address before sending this request.
     */
    IPv4TransmitRequest();
    IPAddress& getAddress();
    uint16_t getDstPort();
    uint16_t getSrcPort();
    uint8_t getProtocol();
    uint8_t getOption();
    void setAddress(IPAddress& dstAddr);
    void setDstPort(uint16_t dstPort);
    void setSrcPort(uint16_t srcPort);
    void setProtocol(uint8_t protocol);
    void setOption(uint8_t option);
protected:
    // declare virtual functions
    virtual uint8_t getFrameData(uint8_t pos);
    virtual uint8_t getFrameDataLength();
private:
    IPAddress _dstAddr;
    uint16_t _dstPort;
    uint16_t _srcPort;
    uint8_t _protocol;
    uint8_t _option;
};

/**
 * Represents a Wi-Fi TX status packet
 */
class Transmit_Status : public FrameIdResponse {
public:
    Transmit_Status();
    uint8_t getStatus();
    bool isSuccess();
};

/**
 * Represents a Wi-Fi RX packet
 */
class IPV4RxFrame : public RxDataResponse {
public:
    IPV4RxFrame();
    IPAddress& getSrcAddress();
    uint16_t getDstPort();
    uint16_t getSrcPort();
    uint8_t getProtocol();
    uint8_t getStatus();
    virtual uint8_t getDataLength();
    // frame position where data starts
    virtual uint8_t getDataOffset();
private:
    IPAddress _srcAddr;
};

#endif
