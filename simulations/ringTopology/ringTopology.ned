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

package redes23lab4g31.simulations.ringTopology;
import redes23lab4g31.Node;

network Network
{
    parameters:
        int numNodes = 8;

    submodules:
        node[numNodes]: Node {
            @display("p=100,100,ri,100,100");
            interfaces = 2;
        }

    connections allowunconnected:
        for i = 0..numNodes-1 {
            node[i].toNod[1] <--> { datarate = 1Mbps; delay = 100us; } <--> node[(i+1)%numNodes].toNod[0];
        }
}