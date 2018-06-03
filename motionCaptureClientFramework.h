#ifndef MOTIONCAPTURECLIENTFRAMEWORK_H
#define MOTIONCAPTURECLIENTFRAMEWORK_H

#include <iostream>
#include <boost/optional.hpp>
// #include <agile/state_t.hpp>
#include <chrono>
#include <thread>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <cstring>


/// @todo: varun move these defines to a enum
#define NAT_CONNECT                 0
#define NAT_SERVERINFO              1
#define NAT_REQUEST                 2
#define NAT_RESPONSE                3
#define NAT_REQUEST_MODELDEF        4
#define NAT_MODELDEF                5
#define NAT_REQUEST_FRAMEOFDATA     6
#define NAT_FRAMEOFDATA             7
#define NAT_MESSAGESTRING           8
#define NAT_UNRECOGNIZED_REQUEST    100

#define MAX_PACKETSIZE              100000    // actual packet size is dynamic
#define MAX_NAMELENGTH              256
#define PATH_MAX 										1

// WARNING: this might not be a constant!
#define SERVER_CLOCK_FREQ 3507520

#define MULTICAST_ADDRESS       "239.255.42.99"
#define PORT_COMMAND            1510      // NatNet Command channel
#define PORT_DATA               1511      // NatNet Data channel


namespace agile {


    typedef struct {
        char szName[MAX_NAMELENGTH];            // sending app's name
        uint8_t Version[4];                     // [major.minor.build.revision]
        uint8_t NatNetVersion[4];               // [major.minor.build.revision]
    } sSender;

    typedef struct sSender_Server {
        sSender Common;
        // host's high resolution clock frequency (ticks per second)
        uint64_t HighResClockFrequency;
        uint16_t DataPort;
        bool IsMulticast;
        uint8_t MulticastGroupAddress[4];
    } sSender_Server;

    typedef struct {
        uint16_t iMessage;                      // message ID (e.g. NAT_FRAMEOFDATA)
        uint16_t nDataBytes;                    // Num bytes in payload
        union {
            uint8_t cData[MAX_PACKETSIZE];
            char szData[MAX_PACKETSIZE];
            uint32_t lData[MAX_PACKETSIZE / sizeof(uint32_t)];
            float fData[MAX_PACKETSIZE / sizeof(float)];
            sSender Sender;
            sSender_Server SenderServer;
        } Data;                                 // Payload incoming from NatNet Server
    } sPacket;

    struct Marker
    {
        int id;
        double x;
        double y;
        double z;
        double size;
        double residual;
    };

    struct Packet
    {
        int message_id;

        std::string model_name;
        int rigid_body_id;
        double pos[3];
        double orientation[4];
        std::vector<Marker> markers_;

        bool tracking_valid;
        float mean_marker_error;
        int labeled_marker_count;

        int frame_number;
        double timestamp;
        uint64_t mid_exposure_timestamp;
        uint64_t camera_data_received_timestamp;
        uint64_t transmit_timestamp;

        double mid_exposure_ros_timestamp;

    };

class motionCaptureClientFramework
{


    // Sockets
    int CommandSocket;
    int DataSocket;
    in_addr ServerAddress;
    sockaddr_in HostAddr;

    // Versioning
    int NatNetVersion[4] = {3, 0, 0, 0};
    int ServerVersion[4] = {0, 0, 0, 0};

    // Command mode global variables
    int gCommandResponse = 0;
    int gCommandResponseSize = 0;
    unsigned char gCommandResponseString[PATH_MAX];
    void CommandListenThread();



public:
    motionCaptureClientFramework();

    void spin();

    bool isOK() {return ok_;}

    void setMulticastAddress (std::string address_) {multicast_address = address_;}

    Packet getPacket() {return output_packet_;}

    bool IPAddress_StringToAddr(char *szNameOrAddress, struct in_addr *Address) const;

    int CreateCommandSocket (in_addr_t IP_Address, unsigned short uPort);

    void setDataSocket (int sock) {DataSocket = sock;}

private:
    bool ok_;

    bool DecodeTimecode(unsigned int inTimecode,
                    unsigned int inTimecodeSubframe,
                    int *hour,
                    int *minute,
                    int *second,
                    int *frame,
                    int *subframe);

    // Takes timecode and assigns it to a string
    bool TimecodeStringify(unsigned int inTimecode,
                       unsigned int inTimecodeSubframe,
                       char *Buffer,
                       size_t BufferSize);

    void DecodeMarkerID(int sourceID, int *pOutEntityID, int *pOutMemberID);

    void Unpack (char *pData, std::vector<Packet> &outputs);

    int SendCommand (char *szCommand);

    std::string multicast_address;

    Packet output_packet_;

};
}

#endif // MOTIONCAPTURECLIENTFRAMEWORK_H
