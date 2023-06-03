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

using namespace omnetpp;

/**
 * TODO - Generated class
 */

class Net : public cSimpleModule
{
  private:
    static const int cntNeighbor = 2;
    int neighborReached; // the cnt of known neighbors
    int neighbor[cntNeighbor]; // the ith element is the neighbor name if i travel to the ith gate
                            // -1 if i don't know the name
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
    virtual void actualizeNeighborInfo(cMessage *msg);
};

#endif
