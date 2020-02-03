#pragma once 

#include <fwpmk.h>

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//   Copyright (c) 2014 Microsoft Corporation.  All Rights Reserved.
//
//   Module Name:
//      HelperFunctions_Headers.h
//
//   Abstract:
//      This module contains definitions and prototypes of kernel helper functions that assist with  
//         MAC, IP, and Transport header operations.
//
//   Author:
//      Dusty Harper      (DHarper)
//
//   Revision History:
//
//      [ Month ][Day] [Year] - [Revision]-[ Comments ]
//      May       01,   2010  -     1.0   -  Creation
//      December  13,   2013  -     1.1   -  Enhance annotations and add
//                                              KrnlHlprIPHeaderGetDestinationAddressField, 
//                                              KrnlHlprIPHeaderGetSourceAddressField,
//                                              KrnlHlprIPHeaderGetVersionField,
//                                              KrnlHlprTransportHeaderGetSourcePortField,
//                                              KrnlHlprTransportHeaderGetDestinationPortField
//
////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma warning(push)
#pragma warning(disable: 4201) /// NAMELESS_STRUCT_UNION

#define ETHERNET_ADDRESS_SIZE 6

#define IPV4_ADDRESS_SIZE  4
#define IPV6_ADDRESS_SIZE 16

#define IPV4 4
#define IPV6 6

#define IPV4_HEADER_MIN_SIZE 20
#define IPV6_HEADER_MIN_SIZE 40
#define ICMP_HEADER_MIN_SIZE  8
#define TCP_HEADER_MIN_SIZE  20
#define UDP_HEADER_MIN_SIZE   8

#define ICMPV4  1
#define TCP     6
#define UDP    17
#define ICMPV6 58

/**
 @macro="htonl"
  
   Purpose:  Convert ULONG in Host Byte Order to Network Byte Order.                            <br>
                                                                                                <br>
   Notes:                                                                                       <br>
                                                                                                <br>
   MSDN_Ref:                                                                                    <br>
*/
#define htonl(l)                  \
   ((((l) & 0xFF000000) >> 24) | \
   (((l) & 0x00FF0000) >> 8)  |  \
   (((l) & 0x0000FF00) << 8)  |  \
   (((l) & 0x000000FF) << 24))
 
/**
 @macro="htons"
  
   Purpose:  Convert USHORT in Host Byte Order to Network Byte Order.                           <br>
                                                                                                <br>
   Notes:                                                                                       <br>
                                                                                                <br>
   MSDN_Ref:                                                                                    <br>
*/
#define htons(s) \
   ((((s) >> 8) & 0x00FF) | \
   (((s) << 8) & 0xFF00))
 
/**
 @macro="ntohl"
  
   Purpose:  Convert ULONG in Network Byte Order to Host Byte Order.                            <br>
                                                                                                <br>
   Notes:                                                                                       <br>
                                                                                                <br>
   MSDN_Ref:                                                                                    <br>
*/
#define ntohl(l)                   \
   ((((l) >> 24) & 0x000000FFL) | \
   (((l) >>  8) & 0x0000FF00L) |  \
   (((l) <<  8) & 0x00FF0000L) |  \
   (((l) << 24) & 0xFF000000L))
 
/**
 @macro="ntohs"
  
   Purpose:  Convert USHORT in Network Byte Order to Host Byte Order.                           <br>
                                                                                                <br>
   Notes:                                                                                       <br>
                                                                                                <br>
   MSDN_Ref:                                                                                    <br>
*/
#define ntohs(s)                     \
   ((USHORT)((((s) & 0x00ff) << 8) | \
   (((s) & 0xff00) >> 8)))
 

static const UINT8 IPV4_LOOPBACK_ADDRESS[] = {0x01,
                                              0x00,
                                              0x00,
                                              0x7F}; /// 127.0.0.1 ( in network byte order)
static const UINT8 IPV6_LOOPBACK_ADDRESS[] = {0x00,
                                              0x00,
                                              0x00,
                                              0x00,
                                              0x00,
                                              0x00,
                                              0x00,
                                              0x00,
                                              0x00,
                                              0x00,
                                              0x00,
                                              0x00,
                                              0x00,
                                              0x00,
                                              0x00,
                                              0x01}; /// ::1



/*
    RFC 894 - A Standard for the Transmission of     <br>
              IP Datagrams over Ethernet Networks    <br>
                                                     <br>
    0                   1                   2        <br>
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3  <br>
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ <br>
   |                                               | <br>
   +            Destination MAC Address            + <br>
   |                                               | <br>
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ <br>
   |                                               | <br>
   +               Source MAC Address              + <br>
   |                                               | <br>
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ <br>
   |              Type             |    Data...    | <br>
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ <br>
                                                     <br>
   RFC_REF: http://www.faqs.org/rfcs/rfc894.html     <br>
*/
typedef struct _ETHERNET_II_HEADER_
{
   BYTE   pDestinationAddress[6];
   BYTE   pSourceAddress[6];
   UINT16 type;
}ETHERNET_II_HEADER;

/*
    RFC 1042 - A Standard for the Transmission of    <br>
               IP Datagrams over IEEE 802 Networks   <br>
                                                     <br>
    0                   1                   2        <br>
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3  <br>
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ <br>
   |                                               | <br>
   +            Destination MAC Address            + <br>
   |                                               | <br>
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ <br>
   |                                               | <br>
   +               Source MAC Address              + <br>
   |                                               | <br>
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ <br>
   |            Length             |      DSAP     | <br>
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ <br>
   |      SSAP     | Control Byte  |    OUI ...    > <br>
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ <br>
   <           OUI (cont.)         |    Type ...   > <br>
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ <br>
   < Type (cont.)  |            Data ...           | <br>
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ <br>
                                                     <br>
   RFC_REF: http://www.faqs.org/rfcs/rfc1042.html    <br>
*/
typedef struct _ETHERNET_SNAP_HEADER_
{
   BYTE   pDestinationAddress[6];
   BYTE   pSourceAddress[6];
   UINT16 length;
   UINT8  destinationSAP; /// Destination Subnetwork Access Protocol
   UINT8  sourceSAP;      /// Source  Subnetwork Access Protocol
   UINT8  controlByte;
   UINT8  pOUI[3];        /// Organizationally Unique Identifier
   UINT16 type;
}ETHERNET_SNAP_HEADER;

/*
                     RFC 791 - Internet Protocol                     <br>
                                                                     <br>
    0                   1                   2                   3    <br>
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1  <br>
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ <br>
   |Version|  IHL  |Type of Service|         Total Length          | <br>
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ <br>
   |        Identification         |Flags|     Fragment Offset     | <br>
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ <br>
   |  Time to Live |     Protocol  |        Header Checksum        | <br>
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ <br>
   |                        Source Address                         | <br>
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ <br>
   |                      Destination Address                      | <br>
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ <br>
   |                    Options                    |    Padding    | <br>
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ <br>
                                                                     <br>
   RFC_REF: http://www.faqs.org/rfcs/rfc791.html                     <br>
*/
typedef struct _IP_HEADER_V4_
{
   union
   {
      UINT8 versionAndHeaderLength;
      struct
      {
         UINT8 headerLength : 4;
         UINT8 version : 4;
      };
   };
   union
   {
      UINT8  typeOfService;
      UINT8  differentiatedServicesCodePoint;
      struct
      {
         UINT8 explicitCongestionNotification : 2;
         UINT8 typeOfService : 6;
      };
   };
   UINT16 totalLength;
   UINT16 identification;
   union
   {
      UINT16 flagsAndFragmentOffset;
      struct
      {
         UINT16 fragmentOffset : 13;
         UINT16 flags : 3;
      };
   };
   UINT8  timeToLive;
   UINT8  protocol;
   UINT16 checksum;
   BYTE   pSourceAddress[sizeof(UINT32)];
   BYTE   pDestinationAddress[sizeof(UINT32)];
}IP_HEADER_V4, *PIP_HEADER_V4;

/*
      RFC 2460 - Internet Protocol, Version 6 (IPv6) Specification   <br>
                                                                     <br>
    0                   1                   2                   3    <br>
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1  <br>
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ <br>
   |Version| Traffic Class |              Flow Label               | <br>
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ <br>
   |        Payload Length         |  Next Header  |   Hop Limit   | <br>
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ <br>
   |                                                               | <br>
   +                                                               + <br>
   |                                                               | <br>
   +                        Source Address                         + <br>
   |                                                               | <br>
   +                                                               + <br>
   |                                                               | <br>
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ <br>
   |                                                               | <br>
   +                                                               + <br>
   |                                                               | <br>
   +                      Destination Address                      + <br>
   |                                                               | <br>
   +                                                               + <br>
   |                                                               | <br>
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ <br>
                                                                     <br>
   RFC_REF: http://www.faqs.org/rfcs/rfc2460.html                    <br>
*/
typedef struct _IP_HEADER_V6_
{
   union
   {
      UINT8 pVersionTrafficClassAndFlowLabel[4];
      struct
      {
       UINT8 r1 : 4;
       UINT8 value : 4;
       UINT8 r2;
       UINT8 r3;
       UINT8 r4;
      }version;
   };
   UINT16 payloadLength;
   UINT8  nextHeader;
   UINT8  hopLimit;
   BYTE   pSourceAddress[16];
   BYTE   pDestinationAddress[16];
}IP_HEADER_V6, *PIP_HEADER_V6;

/*
             RFC 792 - Internet Control Message Protocol             <br>
                                                                     <br>
    0                   1                   2                   3    <br>
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1  <br>
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ <br>
   |     Type      |     Code      |           Checksum            | <br>
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ <br>
   |              Variable (Dependent on Type / Code)              | <br>
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ <br>
*/
typedef struct _ICMP_HEADER_V4_
{
   UINT8  type;
   UINT8  code;
   UINT16 checksum;
/*
   union
   {
      ECHO_MESSAGE                    echo;
      DESTINATION_UNREACHABLE_MESSAGE destinationUnreachable;
      SOURCE_QUENCH_MESSAGE           sourceQuench;
      REDIRECT_MESSAGE                redirect;
      TIME_EXCEEDED_MESSAGE           timeExceeded;
      PARAMETER_PROBLEM_MESSAGE       parameterProblem;
      TIMESTAMP_MESSAGE               timestamp;
      INFORMATION_MESSAGE             information;
   };
*/
}ICMP_HEADER_V4, *PICMP_HEADER_V4;

/*
        RFC 2463 - Internet Control Message Protocol for IPv6        <br>
                                                                     <br>
    0                   1                   2                   3    <br>
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1  <br>
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ <br>
   |     Type      |     Code      |           Checksum            | <br>
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ <br>
   |              Variable (Dependent on Type / Code)              | <br>
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ <br>
*/
typedef struct _ICMP_HEADER_V6_
{
   UINT8  type;
   UINT8  code;
   UINT16 checksum;
/*   union
   {
      ECHO_MESSAGE                    echo;
      DESTINATION_UNREACHABLE_MESSAGE destinationUnreachable;
      SOURCE_QUENCH_MESSAGE           sourceQuench;
      REDIRECT_MESSAGE                redirect;
      TIME_EXCEEDED_MESSAGE           timeExceeded;
      PARAMETER_PROBLEM_MESSAGE       parameterProblem;
      TIMESTAMP_MESSAGE               timestamp;
      INFORMATION_MESSAGE             information;
   };*/
}ICMP_HEADER_V6, *PICMP_HEADER_V6;

/*
               RFC 793 - Transmission Control Protocol               <br>
                                                                     <br>
    0                   1                   2                   3    <br>
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1  <br>
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ <br>
   |          Source Port          |       Destination Port        | <br>
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ <br>
   |                        Sequence Number                        | <br>
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ <br>
   |                     Acknowledgment Number                     | <br>
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ <br>
   |Offset |Rsvd |N|C|E|U|A|P|R|S|F|            Window             | <br>
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ <br>
   |           Checksum            |        Urgent Pointer         | <br>
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ <br>
   |                    Options                    |    Padding    | <br>
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ <br>
                                                                     <br>
   RFC_REF: http://www.faqs.org/rfcs/rfc793.html                     <br>
*/
typedef struct _TCP_HEADER_
{
   UINT16 sourcePort;
   UINT16 destinationPort;
   UINT32 sequenceNumber;
   UINT32 acknowledgementNumber;
   union
   {
      UINT8 dataOffsetReservedAndNS;
      struct
      {
         UINT8 nonceSum : 1;
         UINT8 reserved : 3;
         UINT8 dataOffset : 4;
      }dORNS;
   };
   union
   {
      UINT8 controlBits;
      struct
      {
         UINT8 FIN : 1;
         UINT8 SYN : 1;
         UINT8 RST : 1;
         UINT8 PSH : 1;
         UINT8 ACK : 1;
         UINT8 URG : 1;
         UINT8 ECE : 1;
         UINT8 CWR : 1;
      };
   };
   UINT16 window;
   UINT16 checksum;
   UINT16 urgentPointer;
}TCP_HEADER, *PTCP_HEADER;

/*
                    RFC 768 - User Datagram Protocol                 <br>
                                                                     <br>
    0                   1                   2                   3    <br>
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1  <br>
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ <br>
   |          Source Port          |       Destination Port        | <br>
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ <br>
   |            Length             |           Checksum            | <br>
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ <br>
                                                                     <br>
   RFC_REF: http://www.faqs.org/rfcs/rfc768.html                     <br>
*/
typedef struct _UDP_HEADER_
{
   UINT16 sourcePort;
   UINT16 destinationPort;
   UINT16 length;
   UINT16 checksum;
}UDP_HEADER, *PUDP_HEADER;

#pragma warning(pop) /// NAMELESS_STRUCT_UNION
