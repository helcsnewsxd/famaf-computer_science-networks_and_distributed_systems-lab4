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
#include "Packet_m.h"
#include "NeighborInfo_m.h"

const bool DEBUG = true;

Define_Module(Net);

Net::Net() {
}

Net::~Net() {
}

void Net::initialize() {
    nodeName = this->getParentModule()->getIndex();
    neighborReached = 0;
    memset(neighbor, 0, sizeof(neighbor));
    askNeighborInfo();
}

void Net::finish() {
}

// Data Packet Info

bool Net::isDataPacket(cMessage *msg) {
    Packet *pkt = (Packet *)msg;
    return pkt->getDestination() != -1;
}

// Network Info

bool Net::isNeighborInfo(cMessage *msg) {
    NeighborInfo *pkt = (NeighborInfo *) msg;
    return pkt->isNeighborInfo();
}

void Net::askNeighborInfo() {
    // Send the ask of neighbor names to all the gates
    for (int i = 0; i < cntNeighbor; i++) {
        NeighborInfo *pkt = new NeighborInfo();
        pkt->setIsNeighborInfo(1);
        pkt->setGateIndex(i);
        pkt->setSource(nodeName);
        pkt->setDestination(-1);
        send((cMessage *)pkt, "toLnk$o", i);
    }
}

void Net::actualizeNeighborInfo(cMessage *msg) {
    NeighborInfo *pkt = (NeighborInfo *)msg;
    neighbor[pkt->getGateIndex()] = pkt->getNeighborName();
    neighborReached++;

    if (DEBUG) {
        std::cout << "For node " << nodeName << " i get neighborName " << pkt->getNeighborName() << \
                " and cntReached is " << neighborReached << endl << endl;

        if (neighborReached == cntNeighbor) {
            std::cout << "For node " << nodeName << " i've this neighbors: " << endl;
            for (int i = 0; i < cntNeighbor; i++) std::cout << neighbor[i] << ' ';
            std::cout << endl << endl;
        }
    }

    delete(pkt);
}

// Message Handler

void Net::handleMessage(cMessage *msg) {

    if (isDataPacket(msg)) {

        Packet *pkt = (Packet *)msg;

        // If this node is the final destination, send to App
        if (pkt->getDestination() == nodeName)
            send((cMessage *)pkt, "toApp$o");
        else { // Re-send the packet
            send((cMessage *)pkt, "toLnk$o", 0);
        }

    } else if (isNeighborInfo(msg)) {

        NeighborInfo *pkt = (NeighborInfo*)msg;

        if (pkt->getSource() == nodeName) // The packet has the information that i need
            actualizeNeighborInfo((cMessage *)pkt);
        else { // I've to fill my information in the packet to return to source
            pkt->setNeighborName(nodeName);
            send((cMessage *)pkt, "toLnk$o", pkt->getArrivalGate()->getIndex());
        }

    } else {
        perror("Invalid MSG type");
        delete(msg);
    }
}
