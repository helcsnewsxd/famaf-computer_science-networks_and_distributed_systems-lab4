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

package redes23lab4g31;

//
// TODO auto-generated type
//
module Node
{
    parameters:
        @display("i=block/routing");
        int interfaces;
    gates:
        inout toNod[interfaces];
    submodules:
        app: App {
            parameters:
                @display("p=75,50");
        }
        net: Net {
            parameters:
                @display("p=75,120");
        }
        lnk[interfaces]: Lnk {
            parameters:
                @display("p=75,190,r,70");
        }
    connections allowunconnected:
        app.toNet <--> net.toApp;
        for i=0..interfaces-1 {
            net.toLnk++ <--> lnk[i].toNet;
            lnk[i].toOut <--> toNod[i];
        }
}
