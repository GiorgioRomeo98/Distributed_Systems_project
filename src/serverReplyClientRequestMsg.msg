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

message ServerReplyClientRequestMsg {
    int sourceAddr;		// address of the server replying
    int destAddr;		// address of the client that sent the request
    int leaderAddr;		// if client request is sent to a FOLLOWER server, it replies with the address of the most recent leader it knows. 
    int result;			// to simulate the result response of the leader to the client request message
}
