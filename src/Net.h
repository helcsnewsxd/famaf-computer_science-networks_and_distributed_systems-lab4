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
    int cntNeighborConnected; // I consider that this NEVER is Zero
    int cntNeighborReached;
    vector<pair<int, int>> neighborList; // (neighborName, gateToGo)

    // Actualization Information
    cMessage *actualizeNetworkInformation;

    // Network Local Information
    vector<int> distToGo;
    int cntNodesGraph;
    vector<vector<int>> graphNetwork; // Network representation

    // Mapping nodeName and graphIndex
    map<int, int> id, idRev; // // NodeName to Index to represent it in the graph. The next map (idRev) is the reverse

    // Gate to Send the Packet
    vector<int> gateToSend;

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
    virtual void resetNeighborInformation();
    virtual LSPPacket *calculateMyLSPInformation();
    virtual void sendLSPInformation(LSPPacket *pkt);
    virtual bool isLSPPacket(Packet *pkt); // Destination = -2;
    virtual void actualizeNetworkLocalInformation(LSPPacket *pkt);

    // Mapping nodeName and graphIndex (return the index or create the new entry for it)
    virtual int getID(int nodeName);
    virtual int getIDRev(int nodeName);

    // Return the best gate (index) to send the packet
    virtual int getBestGate(int destination);

    // DEBUG Functions
    virtual void printNodeInformation();
    virtual void printNeighborInformation();
    virtual void printNetworkInformation();
};

#endif
