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

#include <stdio.h>
#include <queue>
#include "Net.h"

Define_Module(Net);

Net::Net() {
}

Net::~Net() {
}

// INITIALIZATION FUNCTION

void Net::initialize() {
    //metrics
    fwdDataPkt.setName("Forwarded DataPackets");
    rcvDataPkt.setName("Received DataPackets");
    sentNeighbor.setName("Sent NeighborPkt");
    rcvNeighbor.setName("Rcv NeighborPkt");
    fwdNeighbor.setName("Fwd NeighborPkt");
    sentLSP.setName("Sent LSP");
    rcvLSP.setName("Rcv LSP");
    // Set my node information
    nodeName = this->getParentModule()->getIndex();

    // Network information
    cntNodesGraph = 0;

    // Create the actualization event
    actualizeNetworkInformation = new cMessage("actualizeNetworkInformation");
    scheduleAt(simTime(), actualizeNetworkInformation);
}

void Net::finish() {
    cancelAndDelete(actualizeNetworkInformation);
}

// NEIGHBOR NODES INFORMATION

NeighborInfoPacket *Net::createNeighborInfoPacket(int gateID) {
    NeighborInfoPacket *pkt = new NeighborInfoPacket();
    pkt->setSource(nodeName);
    pkt->setDestination(-1);
    pkt->setHopCount(0);
    pkt->setGateIndex(gateID);
    return pkt;
}

void Net::askForNeighbors() {
    // Send Neighbor Packet to get information from active gates

    cntGates = this->gateSize("toLnk$o");

    for (int gateID = 0; gateID < cntGates; gateID++) {
        // Check if the gate has connection or not
        if (!this->getParentModule()->gate("toNod$o", gateID)->isConnected())
            continue;
        NeighborInfoPacket *pkt = createNeighborInfoPacket(gateID);
        send((cMessage *) pkt, "toLnk$o", gateID);
        sentNeighbor.record(1);
        cntNeighborConnected++;
    }
}

void Net::completeNeighborInfoAndReturn(NeighborInfoPacket *pkt) {
    // Complete with my information
    pkt->setNeighborName(nodeName);

    // Return to the node
    send((cMessage *)pkt, "toLnk$o", pkt->getArrivalGate()->getIndex());
}

void Net::actualizeNeighbors(NeighborInfoPacket *pkt) {
    int gateIndex = pkt->getGateIndex();
    int neighborName = pkt->getNeighborName();

    neighborList.push_back(make_pair(neighborName, gateIndex));
    cntNeighborReached++;

    if (cntNeighborConnected == cntNeighborReached) { // I've all the information of my neighbors
        LSPPacket *LSPInformation = calculateMyLSPInformation();
        actualizeNetworkLocalInformation(LSPInformation); // I should send my LSP information to the network
                                                          // and actualize the local representation
        delete(LSPInformation);
    }
}

// NETWORK LOCAL INFORMATION

void Net::resetNeighborInformation() {
    // Reset my neighbor's information
    cntGates = 0;
    cntNeighborConnected = cntNeighborReached = 0;
    neighborList.clear();
}

// LSP Information

LSPPacket *Net::calculateMyLSPInformation() {
    LSPPacket *LSPInformation = new LSPPacket();
    LSPInformation->setSource(nodeName);
    LSPInformation->setDestination(-2);
    LSPInformation->setHopCount(0);

    LSPInformation->setNode(nodeName);
    LSPInformation->setNeighborListArraySize(cntNeighborConnected);
    for (int i = 0; i < cntNeighborConnected; i++)
        LSPInformation->setNeighborList(i, neighborList[i].first);
    return LSPInformation;
}

void Net::sendLSPInformation(LSPPacket *pkt) {
    // Send the LSP Info to the network
    for (pair<int, int> neighbor : neighborList)
        send((cMessage *) pkt->dup(), "toLnk$o", neighbor.second);
        sentLSP.record(1);
}

// Mapping nodeName and graphIndex (return the index or create the new entry for it)

int Net::getID(int name) {
    if (id.find(name) != id.end())
        return id[name];

    id[name] = cntNodesGraph;
    idRev[cntNodesGraph] = name;

    graphNetwork.push_back({});
    distToGo.push_back(-1);
    gateToSend.push_back(-1);

    return cntNodesGraph++;
}

int Net::getIDRev(int nodeIndex) {
    assert(idRev.find(nodeIndex) != idRev.end());
    return idRev[nodeIndex];
}

// Local Graph Representation

void Net::actualizeNetworkLocalInformation(LSPPacket *pkt) {
    // Get the information
    int nodeLSP = pkt->getNode();
    int nodeLSPIndex = getID(nodeLSP);

    if (!graphNetwork[nodeLSPIndex].empty()) // If i'ts LSP repeated packet, not consider
        return;

    // Set the graph information

    vector<int> nodeLSPNeighborsIndex;
    for (int i = 0, szi = pkt->getNeighborListArraySize(); i < szi; i++) {
        int neighborLSPIndex = getID(pkt->getNeighborList(i));
        nodeLSPNeighborsIndex.push_back(neighborLSPIndex);
    }
    graphNetwork[nodeLSPIndex] = nodeLSPNeighborsIndex;


    if (nodeLSP == nodeName) { // The actual node. This happens one time

        distToGo[nodeLSPIndex] = 0;
        gateToSend[nodeLSPIndex] = -2;
        for (pair<int, int> neighbor : neighborList) {
            int neighborID = getID(neighbor.first);
            distToGo[neighborID] = 1;
            gateToSend[neighborID] = neighbor.second;
        }

    } else { // The LSP Packet is from other node

        int actDist = -1, neighborIndexWithLessDist = -1;

        for (int neighbor : graphNetwork[nodeLSPIndex])
            if (actDist == -1 || (actDist > distToGo[neighbor] && distToGo[neighbor] != -1))
                actDist = distToGo[neighbor], neighborIndexWithLessDist = neighbor;

        assert(actDist != -1);

        if (actDist != 0) { // Not consider if it's my neighbor because i should have
                            // the information early (when i check my neighbors)
            gateToSend[nodeLSPIndex] = gateToSend[neighborIndexWithLessDist];
            distToGo[nodeLSPIndex] = actDist+1;
        }

        // Do BFS to actualize network nodes

        queue<int> q;
        q.push(nodeLSPIndex);
        while(!q.empty()) {
            int actNode = q.front();
            q.pop();
            for (int actNeighbor : graphNetwork[actNode]) {
                // If i can get a new minimum path, actualize the information
                if (distToGo[actNeighbor] == -1 || distToGo[actNeighbor] > distToGo[actNode] + 1) {
                    distToGo[actNeighbor] = distToGo[actNode] + 1;
                    gateToSend[actNeighbor] = gateToSend[actNode];
                    q.push(actNeighbor);
                }
            }
        }
    }

    // Send the LSP information to other nodes
    sendLSPInformation(pkt);
}

// GETTING THE BEST ROUTING FOR A DESTINATION

int Net::getBestGate(int destination) {
    int destinationIndex = getID(destination);
    if (gateToSend[destinationIndex] == -1) // If not exists a path or i need actualize the network local info
        return neighborList[0].second; // Send to any
    else return gateToSend[destinationIndex];
}

// MESSAGE HANDLER

// Checking message types

bool Net::isDataPacket(Packet *pkt) {
    return (pkt->getDestination() >= 0);
}

bool Net::isActualizationMsg(cMessage *msg) {
    return (msg == actualizeNetworkInformation);
}

bool Net::isNeighborInfoPacket(Packet *pkt) {
    return (pkt->getDestination() == -1);
}

bool Net::isLSPPacket(Packet *pkt) {
    return (pkt->getDestination() == -2);
}

// Handler

void Net::handleMessage(cMessage *msg) {

    if (isActualizationMsg(msg)) { // I've to actualize the local information

        resetNeighborInformation();
        askForNeighbors();

    } else if (isDataPacket((Packet *) msg)) { // Data Packet

        // If this node is the final destination, send to App layer
        if (((Packet *) msg)->getDestination() == nodeName) {
            send(msg, "toApp$o");
            rcvDataPkt.record(1);
        } else { // Re-send the packet
            Packet *pkt = (Packet *)msg;
            send(pkt, "toLnk$o", getBestGate(((Packet *) pkt)->getDestination()));
            pkt->setHopCount(pkt->getHopCount() + 1);
            fwdDataPkt.record(1);
        }
    } else if (isNeighborInfoPacket((Packet *) msg)) { // Neighbor Packet

        // If i've to complete with my info and return the packet
        if (((Packet *) msg)->getSource() != nodeName){
            fwdNeighbor.record(1);
            completeNeighborInfoAndReturn((NeighborInfoPacket *) msg);
        } else { // If i've to actualize with my neighbor info
            actualizeNeighbors((NeighborInfoPacket *) msg);
            rcvNeighbor.record(1);
            delete(msg);
        }

    } else if (isLSPPacket((Packet *) msg)) { // LSP Packet --> Actualize the network information
        rcvLSP.record(1);
        actualizeNetworkLocalInformation((LSPPacket *) msg);
        delete(msg);

    } else { // MSG Type not considered

        perror("Invalid MSG Type");
        delete(msg);
    }

}

// DEBUG Functions

void Net::printNodeInformation() {
    cout << "NODE INFORMATION" << endl;
    cout << "Node Name = " << nodeName << endl;
    cout << string(40, '-') << endl;
}

void Net::printNeighborInformation() {
    cout << "NEIGHBOR INFORMATION OF NODE " << nodeName << endl;
    cout << "Cnt Gates = " << cntGates << endl;
    cout << "Cnt Neighbor Connected = " << cntNeighborConnected << endl;
    cout << "Cnt Neighbor Reached = " << cntNeighborReached << endl;
    cout << "Neighbors: ";
    for (pair<int, int> neighbor : neighborList) cout << '(' << neighbor.first << ',' << neighbor.second << ") ";
    cout << endl << string(40, '-') << endl;
}

void Net::printNetworkInformation() {
    cout << "NETWORK INFORMATION OF NODE " << nodeName << endl;

    cout << "GRAPH REPRESENTATION:" << endl;
    for (int i = 0; i < cntNodesGraph; i++) {
        cout << "Node " << getIDRev(i) << " with neighbors ";
        for (int neighbor : graphNetwork[i])
            cout << getIDRev(neighbor) << ' ';
        cout << endl;
    }

    cout << "DISTANCES:" << endl;
    for (int i = 0; i < cntNodesGraph; i++)
        cout << "Node " << getIDRev(i) << " with distance " << distToGo[i] << endl;

    cout << "GATES TO SEND" << endl;
    for (int i = 0; i < cntNodesGraph; i++)
            cout << "Node " << getIDRev(i) << " with gate to send " << gateToSend[i] << endl;

    cout << endl << string(40, '=') << endl;
}
