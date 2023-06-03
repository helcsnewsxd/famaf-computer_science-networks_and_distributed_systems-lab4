//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#ifndef __REDES23LAB4G31_NET_H_
#define __REDES23LAB4G31_NET_H_

#include <omnetpp.h>
#include "Packet_m.h"
#include "NeighborInfoPacket_m.h"
#include "LSPPacket_m.h"

using namespace omnetpp;
using namespace std;

/**
 * TODO - Generated class
 */

class Net : public cSimpleModule
{
  private:
    // Actual Node Information
    int nodeName;

    // Neighbor Nodes Information
    int cntGates;
    int cntNeighborConnected;
    int cntNeighborReached;
    vector<pair<int, int>> neighborList; // (neighborName, gateToGo)

    // Actualization Information
    const SimTime TIME_ACTUALIZATION = SimTime(10, SIMTIME_S); // Actualize every 10 seconds
    cMessage *actualizeNetworkInformation;

    // Network Local Information

  public:
    Net();
    virtual ~Net();
  protected:
    // More important functions
    virtual void initialize();
    virtual void finish();
    virtual void handleMessage(cMessage *msg);

    // Data Packet
    virtual bool isDataPacket(Packet *pkt);

    // Neighbor Nodes Information
    virtual bool isNeighborInfoPacket(Packet *pkt); // Destination = -1
    virtual NeighborInfoPacket *createNeighborInfoPacket(int gateID);
    virtual void askForNeighbors();
    virtual void completeNeighborInfoAndReturn(NeighborInfoPacket *pkt);
    virtual void actualizeNeighbors(NeighborInfoPacket *pkt);

    // Actualization Information
    virtual bool isActualizationMsg(cMessage *msg);

    // Network Local Information
    virtual void resetAllNetworkLocalInformation();
    virtual LSPPacket *calculateMyLSPInformation();
    virtual void sendLSPInformation(LSPPacket *pkt);
    virtual bool isLSPPacket(Packet *pkt); // Destination = -2;
    virtual void actualizeNetworkLocalInformation(LSPPacket *pkt);

    // DEBUG Functions
    virtual void printNodeInformation();
    virtual void printNeighborInformation();
};

#endif
