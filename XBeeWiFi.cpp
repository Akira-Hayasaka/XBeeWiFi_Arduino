//
//  XBeeWiFi.cpp
//  
//
//  Created by Akira Hayasaka on 4/5/12.
//  Copyright (c) 2012 ﾟ･:,｡ﾟ･:,｡★ﾟ･:,｡ﾟ･:,｡☆ﾟ･:,｡ﾟ･:,｡★ﾟ･:,｡ﾟ･:,｡☆. All rights reserved.
//

#include "dbg.h"

#include "XBeeWiFi.h"

#define REVERSE_ENDIAN(x) (uint16_t)(((uint16_t)x >> 8) | ((uint16_t)x << 8))

XBeeWiFi::XBeeWiFi() : XBee() {}

int XBeeWiFi::setup (int security, const char *ssid, const char *pin) {
    int len, r;
    uint8_t cmd[2], val[32];
    AtCommandRequest atRequest;
    
    // SSID
    memcpy(cmd, "ID", 2);
    len = strlen(ssid);
    memcpy(val, ssid, len);
    atRequest.setCommand(cmd);
    atRequest.setCommandValue(val);
    atRequest.setCommandValueLength(len);
    atRequest.setFrameId(getNextFrameId());
    send(atRequest);
    r = getWiResponse(AT_COMMAND_RESPONSE, atRequest.getFrameId());
    DBG("wifi ID: %d\r\n", r);
    if (r < 0) return -1;
    
    if (security != SECURITY_OPEN) {
        // PIN
        memcpy(cmd, "PK", 2);
        len = strlen(pin);
        memcpy(val, pin, len);
        atRequest.setCommand(cmd);
        atRequest.setCommandValue(val);
        atRequest.setCommandValueLength(len);
        atRequest.setFrameId(getNextFrameId());
        send(atRequest);
        r = getWiResponse(AT_COMMAND_RESPONSE, atRequest.getFrameId());
        DBG("wifi PK: %d\r\n", r);
        if (r < 0) return -1;
    }
    
    // security type
    memcpy(cmd, "EE", 2);
    val[0] = security;
    atRequest.setCommand(cmd);
    atRequest.setCommandValue(val);
    atRequest.setCommandValueLength(1);
    atRequest.setFrameId(getNextFrameId());
    send(atRequest);
    r = getWiResponse(AT_COMMAND_RESPONSE, atRequest.getFrameId());
    DBG("wifi EE: %d\r\n", r);
    if (r < 0) return -1;
    
    return 0;
}

int XBeeWiFi::setup (const char *ssid) {
    return setup(SECURITY_OPEN, ssid, NULL);
}

int XBeeWiFi::reset () {
    // RESET
    uint8_t cmd[2] = {'N', 'R'}; // Network reset
    //    uint8_t cmd[2] = {'F', 'R'}; // Software reset
    AtCommandRequest atRequest;
    
    atRequest.setCommand(cmd);
    atRequest.setFrameId(getNextFrameId());
    send(atRequest);
    return getWiResponse(AT_COMMAND_RESPONSE, atRequest.getFrameId());
}

int XBeeWiFi::baud (int b) {
    // RESET
    uint8_t cmd[2] = {'B', 'D'};
    char val[4];
    AtCommandRequest atRequest;
    int r, len;
    
    if (b < 0x100) {
        val[0] = b;
        len = 1;
    } else {
        val[0] = (b >> 24) & 0xff;
        val[1] = (b >> 16) & 0xff;
        val[2] = (b >> 8) & 0xff;
        val[3] = b & 0xff;
        len = 4;
    }
    atRequest.setCommand(cmd);
    atRequest.setCommandValue((uint8_t*)val);
    atRequest.setCommandValueLength(len);
    atRequest.setFrameId(getNextFrameId());
    send(atRequest);
    r = getWiResponse(AT_COMMAND_RESPONSE, atRequest.getFrameId());
    
    if (r == 0) {
        begin(b);
    }
    return r;
}

int XBeeWiFi::setAddress () {
    // DHCP
    uint8_t cmd[2] = {'M', 'A'};
    char val[1] = {0};
    AtCommandRequest atRequest;
    
    _nameserver = IPAddress(0,0,0,0);
    _nameport = DNS_PORT;
    
    atRequest.setCommand(cmd);
    atRequest.setCommandValue((uint8_t*)val);
    atRequest.setCommandValueLength(1);
    atRequest.setFrameId(getNextFrameId());
    send(atRequest);
    
    return getWiResponse(AT_COMMAND_RESPONSE, atRequest.getFrameId());
}

int XBeeWiFi::setAddress (IPAddress &ipaddr, IPAddress &netmask, IPAddress &gateway, IPAddress &nameserver) {
    uint8_t cmd[2];
    char val[32];
    AtCommandRequest atRequest;
    
    // Static
    memcpy(cmd, "MA", 2);
    val[0] = 1;
    atRequest.setCommand(cmd);
    atRequest.setCommandValue((uint8_t*)val);
    atRequest.setCommandValueLength(1);
    atRequest.setFrameId(getNextFrameId());
    send(atRequest);
    getWiResponse(AT_COMMAND_RESPONSE, atRequest.getFrameId());
    
    // IP address
    memcpy(cmd, "MY", 2);
    sprintf(val, "%d.%d.%d.%d", ipaddr[0], ipaddr[1], ipaddr[2], ipaddr[3]);
    atRequest.setCommand(cmd);
    atRequest.setCommandValue((uint8_t*)val);
    atRequest.setCommandValueLength(strlen(val));
    atRequest.setFrameId(getNextFrameId());
    send(atRequest);
    getWiResponse(AT_COMMAND_RESPONSE, atRequest.getFrameId());
    
    // sub netmask
    memcpy(cmd, "NK", 2);
    sprintf(val, "%d.%d.%d.%d", netmask[0], netmask[1], netmask[2], netmask[3]);
    atRequest.setCommand(cmd);
    atRequest.setCommandValue((uint8_t*)val);
    atRequest.setCommandValueLength(strlen(val));
    atRequest.setFrameId(getNextFrameId());
    send(atRequest);
    getWiResponse(AT_COMMAND_RESPONSE, atRequest.getFrameId());
    
    // default gateway
    memcpy(cmd, "GW", 2);
    sprintf(val, "%d.%d.%d.%d", gateway[0], gateway[1], gateway[2], gateway[3]);
    atRequest.setCommand(cmd);
    atRequest.setCommandValue((uint8_t*)val);
    atRequest.setCommandValueLength(strlen(val));
    atRequest.setFrameId(getNextFrameId());
    send(atRequest);
    getWiResponse(AT_COMMAND_RESPONSE, atRequest.getFrameId());
    
    // name server
    _nameserver = nameserver;
    _nameport = DNS_PORT;
    
    return 0;
}

int XBeeWiFi::getAddress (IPAddress &ipaddr, IPAddress &netmask, IPAddress &gateway, IPAddress &nameserver) {
    int r;
    uint8_t cmd[2];
    AtCommandRequest atRequest;
    AtCommandResponse atResponse;
    
    memcpy(cmd, "MY", 2);
    atRequest.setCommand(cmd);
    atRequest.clearCommandValue();
    atRequest.setFrameId(getNextFrameId());
    send(atRequest);
    r = getWiResponse(AT_COMMAND_RESPONSE, atRequest.getFrameId());
    DBG("wifi MY: %d\r\n", r);
    if (r >= 0) {
        r = getWiAddr(ipaddr);
    }
    
    memcpy(cmd, "MK", 2);
    atRequest.setCommand(cmd);
    atRequest.clearCommandValue();
    atRequest.setFrameId(getNextFrameId());
    send(atRequest);
    r = getWiResponse(AT_COMMAND_RESPONSE, atRequest.getFrameId());
    DBG("wifi MK: %d\r\n", r);
    if (r >= 0) {
        r = getWiAddr(netmask);
    }
    
    memcpy(cmd, "GW", 2);
    atRequest.setCommand(cmd);
    atRequest.clearCommandValue();
    atRequest.setFrameId(getNextFrameId());
    send(atRequest);
    r = getWiResponse(AT_COMMAND_RESPONSE, atRequest.getFrameId());
    DBG("wifi GW: %d\r\n", r);
    if (r >= 0) {
        r = getWiAddr(gateway);
    }
    
    nameserver = _nameserver;
    
    return 0;
}

int XBeeWiFi::getWiAddr (IPAddress &ipaddr) {
    int ip1, ip2, ip3, ip4;
    AtCommandResponse atResponse;
    
    getResponse().getAtCommandResponse(atResponse);
    if (atResponse.isOk() && atResponse.getValueLength() >= 7) {
        sscanf((char*)atResponse.getValue(), "%d.%d.%d.%d", &ip1, &ip2, &ip3, &ip4);
        ipaddr = IPAddress(ip1, ip2, ip3, ip4);
        return 0;
    }
    return -1;
}

int XBeeWiFi::setTimeout (int timeout) {
    // timeout
    uint8_t cmd[2] = {'T', 'P'};
    char val[1];
    AtCommandRequest atRequest;
    
    val[0] = timeout;
    atRequest.setCommand(cmd);
    atRequest.setCommandValue((uint8_t*)val);
    atRequest.setCommandValueLength(1);
    atRequest.setFrameId(getNextFrameId());
    send(atRequest);
    
    return getWiResponse(AT_COMMAND_RESPONSE);
}

int XBeeWiFi::getStatus () {
    // AI
    uint8_t cmd[2] = {'A', 'I'};
    AtCommandRequest atRequest;
    
    atRequest.setCommand(cmd);
    atRequest.setFrameId(getNextFrameId());
    send(atRequest);
    
    return getWiResponse(AT_COMMAND_RESPONSE, atRequest.getFrameId());
}

int XBeeWiFi::getWiResponse (int apiId, int frameid, int timeout) {
    AtCommandResponse atResponse;
    
    if (readPacket(timeout)) {
        if (frameid && frameid != getResponse().getFrameData()[0]) {
            DBG("Expected AT response but got %x (frame %d)\r\n", getResponse().getApiId(), getResponse().getFrameData()[0]);
            if (! readPacket(timeout)) return -1;
            DBG("Retry ok\r\n");
        }
        
        if (getResponse().getApiId() == apiId) {
            getResponse().getAtCommandResponse(atResponse);
            
            if (getResponse().getApiId() == IPv4_RX_FRAME) {
                return 0;
            } else
                if (atResponse.isOk()) {
                    
                    //                if (getResponse().getFrameDataLength() > 4) {
                    return atResponse.getValue()[0];
                    
                } else {
                    DBG("Command return error code: %x\r\n", atResponse.getStatus());
                }
        } else {
            DBG("Expected AT response but got %x\r\n", getResponse().getApiId());
        }
    } else {
        if (getResponse().isError()) {
            DBG("Error reading packet.  Error code: %x\r\n", getResponse().getErrorCode());  
        } else {
            DBG("No response from radio\r\n");  
        }
    }
    
    return -1;
}

int XBeeWiFi::setNameserver (IPAddress &nameserver, int port = DNS_PORT) {
    _nameserver = nameserver;
    _nameport = port;
    return 0;
}

// *** Note: wifi is turned off when XBee send the port 53 udp packet.
int XBeeWiFi::getHostByName (const char* name, IPAddress &addr) {
    char buf[600];
    int r, len;
    uint8_t cmd[2] = {'C', '0'};
    char val[2];
    IPv4TransmitRequest dnsRequest;
    AtCommandRequest atRequest;
    AtCommandResponse atResponse;
    
    if (!strcmp(name, "localhost")) {
        addr = IPAddress(127, 0, 0, 1);
        return 0;
    }
    
    // bind src port
    val[0] = (DNS_SRC_PORT >> 8) & 0xff;
    val[1] = DNS_SRC_PORT & 0xff;
    atRequest.setCommand(cmd);
    atRequest.setCommandValue((uint8_t*)val);
    atRequest.setCommandValueLength(2);
    atRequest.setFrameId(getNextFrameId());
    send(atRequest);
    r = getWiResponse(AT_COMMAND_RESPONSE);
    DBG("wifi C0: %d\r\n", r);
    
    // send DNS request
    len = createDnsRequest(name, buf);
    dnsRequest.setAddress(_nameserver);
    dnsRequest.setDstPort(_nameport);
    dnsRequest.setSrcPort(DNS_SRC_PORT);
    dnsRequest.setProtocol(PROTOCOL_UDP);
    dnsRequest.setPayload((uint8_t*)buf);
    dnsRequest.setPayloadLength(len);
    
    // wait responce
    unsigned long start = millis();    
    while (int((millis() - start)) < DNS_TIMEOUT) {
        dnsRequest.setFrameId(getNextFrameId());
        send(dnsRequest);
        
        r = getWiResponse(TX_STATUS_RESPONSE, dnsRequest.getFrameId());
        DBG("wifi TX: %d\r\n", r);
        
        if (r >= 0) {
            // recv DNS request
            r = getWiResponse(IPv4_RX_FRAME, 0, 3000);
            DBG("wifi RX: %d\r\n", r);
            if (r >= 0) {
                getResponse().getAtCommandResponse(atResponse);
                return getDnsResponse(atResponse.getValue() + 6, atResponse.getValueLength() - 6, addr);
            }
        } else {
            break;
        }
    }
    
    return -1;
}

int XBeeWiFi::createDnsRequest (const char* name, char *buf) {
    struct DNSHeader *dnsHeader;
    struct DnsQuestionEnd *dnsEnd;
    int len, num;
    
    // DNS header
    dnsHeader = (DNSHeader*)buf;
    dnsHeader->id = REVERSE_ENDIAN(0xdead);
    dnsHeader->flags = REVERSE_ENDIAN(0x100);
    dnsHeader->questions = REVERSE_ENDIAN(1);
    dnsHeader->answers = 0;
    dnsHeader->authorities = 0;
    dnsHeader->additional = 0;
    
    // DNS question
    len = sizeof(DNSHeader);
    while ((num = (int)strchr(name, '.')) != NULL) {
        num = num - (int)name;
        buf[len] = num;
        len ++;
        strncpy(&buf[len], name, num); 
        name = name + num + 1;
        len = len + num;
    }
    
    if ((num = strlen(name)) != NULL) {
        buf[len] = num;
        len ++; 
        strncpy(&buf[len], name, num); 
        len = len + num;
    }
    buf[len] = 0;
    len ++; 
    
    dnsEnd = (DnsQuestionEnd*)&buf[len];
    dnsEnd->type = REVERSE_ENDIAN(DNS_QUERY_A);
    dnsEnd->clas = REVERSE_ENDIAN(DNS_CLASS_IN);
    
    return len + sizeof(DnsQuestionEnd);
}

int XBeeWiFi::getDnsResponse (const uint8_t *buf, int len, IPAddress &addr) {
    int i;
    struct DNSHeader *dnsHeader;
    struct DnsAnswer *dnsAnswer;
    
    // DNS header
    dnsHeader = (DNSHeader*)buf;
    if (REVERSE_ENDIAN(dnsHeader->id) != 0xdead || (REVERSE_ENDIAN(dnsHeader->flags) & 0x800f) != 0x8000) {
        return -1;
    }
    
    // skip question
    for (i = sizeof(DNSHeader); buf[i] && i < len; i ++);
    i = i + 1 + sizeof(DnsQuestionEnd);
    
    // DNS answer
    while (i < len) {
        dnsAnswer = (DnsAnswer*)&buf[i];
        if (dnsAnswer->clas != REVERSE_ENDIAN(DNS_CLASS_IN)) {
            return -1;
        }
        
        i = i + sizeof(DnsAnswer);
        if (dnsAnswer->type == REVERSE_ENDIAN(DNS_QUERY_A)) {
            addr = IPAddress(buf[i], buf[i + 1], buf[i + 2], buf[i + 3]);
            return 0;
        }
        i = i + dnsAnswer->length;
    }
    
    return -1;
}


IPv4TransmitRequest::IPv4TransmitRequest() : PayloadRequest(IPv4_TRANSMIT_REQUEST, DEFAULT_FRAME_ID, NULL, 0) {
}

IPv4TransmitRequest::IPv4TransmitRequest(IPAddress &dstAddr, uint16_t dstPort, uint16_t srcPort, uint8_t protocol, uint8_t option, uint8_t *data, uint16_t dataLength, uint8_t frameId): PayloadRequest(IPv4_TRANSMIT_REQUEST, frameId, data, dataLength) {
    _dstAddr = dstAddr;
    _dstPort = dstPort;
    _srcPort = srcPort;
    _protocol = protocol;
    _option = option;
}

IPv4TransmitRequest::IPv4TransmitRequest(IPAddress &dstAddr, uint16_t dstPort, uint8_t *data, uint16_t dataLength): PayloadRequest(IPv4_TRANSMIT_REQUEST, DEFAULT_FRAME_ID, data, dataLength) {
    _dstAddr = dstAddr;
    _dstPort = dstPort;
    _srcPort = 0x270f;
    _protocol = PROTOCOL_UDP;
    _option = 0;
}

uint8_t IPv4TransmitRequest::getFrameData(uint8_t pos) {
    if (pos == 0) {
        return _dstAddr[0];
    } else if (pos == 1) {
        return _dstAddr[1];
    } else if (pos == 2) {
        return _dstAddr[2];
    } else if (pos == 3) {
        return _dstAddr[3];
    } else if (pos == 4) {
        return (_dstPort >> 8) & 0xff;
    } else if (pos == 5) {
        return _dstPort & 0xff;
    } else if (pos == 6) {
        return (_srcPort >> 8) & 0xff;
    } else if (pos == 7) {
        return _srcPort & 0xff;
    } else if (pos == 8) {
        return _protocol;
    } else if (pos == 9) {
        return _option;
    } else {
        return getPayload()[pos - IPv4_TRANSMIT_REQUEST_API_LENGTH];
    }
}

uint8_t IPv4TransmitRequest::getFrameDataLength() {
    return IPv4_TRANSMIT_REQUEST_API_LENGTH + getPayloadLength();
}

IPAddress& IPv4TransmitRequest::getAddress() {
    return _dstAddr;
}

uint16_t IPv4TransmitRequest::getDstPort() {
    return _dstPort;
}

uint16_t IPv4TransmitRequest::getSrcPort() {
    return _srcPort;
}

uint8_t IPv4TransmitRequest::getProtocol() {
    return _protocol;
}

uint8_t IPv4TransmitRequest::getOption() {
    return _option;
}

void IPv4TransmitRequest::setAddress(IPAddress &dstAddr) {
    _dstAddr = dstAddr;
}

void IPv4TransmitRequest::setDstPort(uint16_t dstPort) {
    _dstPort = dstPort;
}

void IPv4TransmitRequest::setSrcPort(uint16_t srcPort) {
    _srcPort = srcPort;
}

void IPv4TransmitRequest::setProtocol(uint8_t protocol) {
    _protocol = protocol;
}

void IPv4TransmitRequest::setOption(uint8_t option) {
    _option = option;
}


Transmit_Status::Transmit_Status() : FrameIdResponse() {
    
}

uint8_t Transmit_Status::getStatus() {
    return getFrameData()[1];
}

bool Transmit_Status::isSuccess() {
    return getStatus() == SUCCESS;
}


IPV4RxFrame::IPV4RxFrame () {
}

IPAddress& IPV4RxFrame::getSrcAddress() {
    _srcAddr = IPAddress(getFrameData()[0], getFrameData()[1], getFrameData()[2], getFrameData()[3]);
    return _srcAddr;
}

uint16_t IPV4RxFrame::getDstPort() {
    return (getFrameData()[4] << 8) + getFrameData()[5];
}

uint16_t IPV4RxFrame::getSrcPort() {
    return (getFrameData()[6] << 8) + getFrameData()[7];
}

uint8_t IPV4RxFrame::getProtocol() {
    return getFrameData()[8];
}

uint8_t IPV4RxFrame::getStatus() {
    return getFrameData()[9];
}

// markers to read data from packet array.  this is the index, so the 12th item in the array
uint8_t IPV4RxFrame::getDataOffset() {
    return 10;
}

uint8_t IPV4RxFrame::getDataLength() {
    return getPacketLength() - getDataOffset() - 1;
}

