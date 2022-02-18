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

// Created: 2022-1-17 for project PredaRomeo_DSProject

package predaromeo_dsproject;

import ned.DelayChannel;

simple Server
{
    parameters:
        @display("i=device/server");
        int servers_number = default(5);
    gates:
        inout gate[];
}

simple Client
{
    parameters:
        int addr = default(-1);
        volatile double sendIaTime @unit(s);
        @display("i=device/pc2");
    gates:
        inout gate;
}

simple Cloud
{
    parameters:
        @display("i=misc/cloud");
        double propDelay @unit(s);
    gates:
        inout gate_s[];
        inout gate_c[];
}


//
// A generated network with full graph topology.
//
network RAFT_Network
{
    parameters:
        int servers_number = default(5);
        int clients_number = default(2);
    types:
        channel Channel extends ned.DelayChannel
        {
            delay = 100ms;
        }
    submodules:
        server[servers_number]: Server {
            @display("p=,,ring");
            gates:
                gate[servers_number];
        }
        client[clients_number]: Client {
            gates:
                gate;
        }
        cloud: Cloud {
            gates:
                gate_s[servers_number];
                gate_c[clients_number];
        }
    connections allowunconnected:
        for i=0..servers_number-2, for j=i+1..servers_number-1 {
            server[i].gate[j] <--> Channel <--> server[j].gate[i];
        }

        for i=0..servers_number-1 {
            cloud.gate_s[i] <--> Channel <--> server[i].gate[i];
        }

        for i=0..clients_number-1 {
            client[i].gate <--> Channel <--> cloud.gate_c[i];
        }


}
