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



// In addition to announcing types to the message compiler, their C++ declarations also need to be available
// at compile time so that the generated code will actually compile.
cplusplus {{
#include "command.h"
typedef _command Cmd;
}};


// To use a C++ type (class, struct a typedef) defined outside the msg file, that type needs to be announced to the message compiler.
class noncobject Cmd;


message ClientRequestMsg {
    int sourceAddr;		// address of the client that sent the request
    int destAddr;		// address of the server
    int serialNumber;	// to uniquely identify a client request between all the ones sent by the same client
    Cmd command;		// command to process
}