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

const bool DEBUG = false; // True if you want comments to DEBUG

Define_Module(Net);

Net::Net() {
}

Net::~Net() {
}

void Net::initialize() {
    nodeName = this->getParentModule()->getIndex();
    cntNodesGraph = 0;
    cntLSPVis = 0;
    neighborReached = 0;
    memset(neighbor, 0, sizeof(neighbor));

    // Get the information from the neighbors
    askNeighborInfo();
}

void Net::finish() {
}

// Data Packet Info

bool Net::isDataPacket(cMessage *msg) {
    Packet *pkt = (Packet *)msg;
    return pkt->getDestination() >= 0;
}

// Network Info

bool Net::isNeighborInfo(cMessage *msg) {
    Packet *pkt = (Packet *) msg;
    return pkt->getDestination() == -1;
}

void Net::askNeighborInfo() {
    // Send the ask of neighbor names to all the gates
    for (int i = 0; i < cntNeighbor; i++) {
        NeighborInfo *pkt = new NeighborInfo();
        pkt->setGateIndex(i);
        pkt->setSource(nodeName);
        pkt->setDestination(-1);
        send((cMessage *)pkt, "toLnk$o", i);
    }
}

void Net::actualizeNeighborInfo(NeighborInfo *pkt) {
    neighbor[pkt->getGateIndex()] = pkt->getNeighborName();
    neighborReached++;

    if (DEBUG) {
        cout << "For node " << nodeName << " i get neighborName " << pkt->getNeighborName() << \
                " and cntReached is " << neighborReached << endl << endl;

        if (neighborReached == cntNeighbor) {
            cout << "For node " << nodeName << " i've this neighbors: " << endl;
            for (int i = 0; i < cntNeighbor; i++) cout << neighbor[i] << ' ';
            cout << endl << endl;
        }
    }

    // I can send my LSP information
    if (neighborReached == cntNeighbor) {
        LSP *pktLSP = new LSP();
        pktLSP->setNode(nodeName);
        for (int i = 0; i < cntNeighbor; i++)
            pktLSP->setNeighbor(i, neighbor[i]);
        pktLSP->setSource(nodeName);
        pktLSP->setDestination(-2);
        actualizeNetworkRepresentation(pktLSP);
        delete(pktLSP);
    }
}

// LSP Info

int Net::getID(int nodeName) {
    if (id.find(nodeName) != id.end())
        return id[nodeName];

    id[nodeName] = cntNodesGraph;
    idRev[cntNodesGraph] = nodeName;

    LSPVis.push_back(false);
    graph.push_back(make_pair(-1, -1));
    gateToSend.push_back(-1);

    return cntNodesGraph++;
}

int Net::getIDRev(int nodeIndex) {
    assert(idRev.find(nodeIndex) != idRev.end());
    return idRev[nodeIndex];
}

bool Net::isLSPInfo(cMessage *msg) {
    Packet *pkt = (Packet *) msg;
    return pkt->getDestination() == -2;
}

void Net::sendLSP(LSP *pkt) {
    // Send the LSP to all the gates
    for (int i = 0; i < cntNeighbor; i++) {
        LSP *pktLSP = pkt->dup();
        send((cMessage *)pktLSP, "toLnk$o", i);
    }
}

void Net::actualizeNetworkRepresentation(LSP *pkt) {
    // Get the information
    int nodeLSP = pkt->getNode();
    int nodeLSPIndex = getID(nodeLSP);

    // If i get early the LSP information of this node (i don't actualize or send to other nodes)
    if (LSPVis[nodeLSPIndex]) {
        return;
    }

    // Add the information into the network representation as a graph
    int neighborLSP[2];
    for (int i = 0; i < cntNeighbor; i++)
        neighborLSP[i] = pkt->getNeighbor(i);
    graph[nodeLSPIndex] = make_pair(getID(neighborLSP[0]), getID(neighborLSP[1]));
    LSPVis[nodeLSPIndex] = true;
    cntLSPVis++;

    // Send LSP to all neighbors
    sendLSP(pkt);

    // If i finish the Network Representation using LSP messages, calculate the gate for the nodes
    if (cntNodesGraph == cntLSPVis && DEBUG) {
        // Check the minimum distance with BFS
        queue<pair<int, int>> q;
        q.push(make_pair(graph[getID(nodeName)].first, 0));
        q.push(make_pair(graph[getID(nodeName)].second, 1));

        while (!q.empty()) {
            pair<int, int> actNode = q.front();
            q.pop();

            gateToSend[actNode.first] = actNode.second;

            // Check the distance to the neighbor (left if snd is 0. Right otherwise)
            if (actNode.second == 0) {
                if (gateToSend[graph[actNode.first].first] == -1) {
                    q.push(make_pair(graph[actNode.first].first, 0));
                }
            } else {
                if (gateToSend[graph[actNode.first].second] == -1) {
                    q.push(make_pair(graph[actNode.first].second, 1));
                }
            }
        }

        if (DEBUG) { // Print DEBUG
            cout << "For node " << nodeName << " i've this representation: " << endl;
            for (int i = 0; i < cntNodesGraph; i++) {
                cout << "Node (" << i << "," << getIDRev(i) << "): ";
                cout << "{(" << graph[i].first << "," << getIDRev(graph[i].first) << "), ";
                cout << "(" << graph[i].second<< "," << getIDRev(graph[i].second) << ")}";
                cout << " I go to this node going to " << (gateToSend[i]==0?"left":"right");
                cout << endl;
            }
            cout << string(40, '-') << endl;
        }
    }
}

// Gate to Send

int Net::getGateToSend(int nodeName) {
    if (id.find(nodeName) == id.end())
        return 0;
    return max(gateToSend[getID(nodeName)], 0);
}

// Message Handler

void Net::handleMessage(cMessage *msg) {

    if (isDataPacket(msg)) {
        Packet *pkt = (Packet *)msg;

        // If this node is the final destination, send to App
        if (pkt->getDestination() == nodeName)
            send((cMessage *)pkt, "toApp$o");
        else { // Re-send the packet
            send((cMessage *)pkt, "toLnk$o", getGateToSend(pkt->getDestination()));
        }

    } else if (isNeighborInfo(msg)) {

        NeighborInfo *pkt = (NeighborInfo*)msg;

        if (pkt->getSource() == nodeName) { // The packet has the information that i need
            actualizeNeighborInfo(pkt);
            delete(msg);
        } else { // I've to fill my information in the packet to return to source
            pkt->setNeighborName(nodeName);
            send((cMessage *)pkt, "toLnk$o", pkt->getArrivalGate()->getIndex());
        }

    } else if (isLSPInfo(msg)) {

        LSP *pkt = (LSP *)msg;
        actualizeNetworkRepresentation(pkt);
        delete(msg);

    } else {
        perror("Invalid MSG type");
        delete(msg);
    }
}
