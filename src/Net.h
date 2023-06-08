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
#include "NeighborInfo_m.h"
#include "LSP_m.h"

using namespace omnetpp;
using namespace std;

/**
 * TODO - Generated class
 */

class Net : public cSimpleModule
{
  private:
    // Node Info
    static const int cntNeighbor = 2;
    int neighborReached; // the cnt of known neighbors
    int neighbor[cntNeighbor]; // the ith element is the neighbor name if i travel to the ith gate
                            // -1 if i don't know the name

    // Network Representation
    map<int, int> id, idRev; // NodeName to Index to represent it in the graph. The next map (idRev) is the reverse
    vector<pair<int, int>> graph; // The ith element is the (left, right) neighbors of the node
    vector<bool> LSPVis; // True if the node get the LSP information with node with id equals to the index (ith)
    int cntLSPVis;
    int cntNodesGraph;
    virtual int getID(int nodeName);
    virtual int getIDRev(int nodeName);

    // Gate to Send
    virtual int getGateToSend(int nodeName);
    vector<int> gateToSend;

    //metrics
    cOutVector fwdDataPkt;
    cOutVector rcvDataPkt;
    cOutVector sentNeighbor;
    cOutVector rcvNeighbor;
    cOutVector fwdNeighbor;
    cOutVector sentLSP;
    cOutVector rcvLSP;

  public:
    Net();
    virtual ~Net();
  protected:
    virtual void initialize();
    virtual void finish();
    virtual void handleMessage(cMessage *msg);

    // Node name
    int nodeName;

    // Data Packet Info
    virtual bool isDataPacket(cMessage *msg);

    // Neighbor Info
    virtual bool isNeighborInfo(cMessage *msg);
    virtual void askNeighborInfo();
    virtual void actualizeNeighborInfo(NeighborInfo *pkt);

    // LSP Info
    virtual bool isLSPInfo(cMessage *msg);
    virtual void sendLSP(LSP *pkt);
    virtual void actualizeNetworkRepresentation(LSP *pkt);
};

#endif
