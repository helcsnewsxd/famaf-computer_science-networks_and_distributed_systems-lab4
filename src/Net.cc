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
#include "Net.h"

Define_Module(Net);

Net::Net() {
}

Net::~Net() {
}

// INITIALIZATION FUNCTION

void Net::initialize() {
    // Set my node information
    nodeName = this->getParentModule()->getIndex();

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

void Net::resetAllNetworkLocalInformation() {
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
}

// Local Graph Representation

void Net::actualizeNetworkLocalInformation(LSPPacket *pkt) {

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

        resetAllNetworkLocalInformation();
        askForNeighbors();

    } else if (isDataPacket((Packet *) msg)) { // Data Packet

        delete(msg);

//        // If this node is the final destination, send to App layer
//        if (((Packet *) msg)->getDestination() == nodeName)
//            send(msg, "toApp$o");
//        else // Re-send the packet
//            send(msg, "toLnk$o", 0); // COMPLETE WITH THE OPTIMAL ROUTE

    } else if (isNeighborInfoPacket((Packet *) msg)) { // Neighbor Packet

        // If i've to complete with my info and return the packet
        if (((Packet *) msg)->getSource() != nodeName)
            completeNeighborInfoAndReturn((NeighborInfoPacket *) msg);
        else { // If i've to actualize with my neighbor info
            actualizeNeighbors((NeighborInfoPacket *) msg);
            delete(msg);
        }

    } else if (isLSPPacket((Packet *) msg)) { // LSP Packet --> Actualize the network information

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
