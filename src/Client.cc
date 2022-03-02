/*
 * Client.cc
 *
 *  Created on: 18 Feb 2022
 *      Author: Giorgio Romeo, Daria Preda
 */

#include <omnetpp.h>
#include "clientRequestMsg_m.h"
#include "serverReplyClientRequestMsg_m.h"

using namespace omnetpp;

class Client: public cSimpleModule
{
  private:
    int addr;               // client source address
    int serialNumber;       // request message serial number
    int servers_number;     // total number of servers
    int serverLeader;       // address of the most recent leader

    Command currentCommand;

    simtime_t requestMsgTimeout;        // timeout
    cMessage *requestMsgTimeoutEvent;   // holds pointer to the requestMsgTimeout self-message
    ClientRequestMsg *currentRequestMsg;

  public:
    Client();
    virtual ~Client();

  protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    void handleRequestMsgTimeoutEvent(cMessage *timeout);
    void handleServerReplyClientRequestMsg(ServerReplyClientRequestMsg *serverReplyClientRequestMsg);
    void sendRequest(ClientRequestMsg * msg);
    ClientRequestMsg *generateRequestMsg();
};

Define_Module(Client);

Client::Client()
{
    requestMsgTimeoutEvent = currentRequestMsg = nullptr;
}

Client::~Client()
{
    cancelAndDelete(requestMsgTimeoutEvent);
    delete currentRequestMsg;
}

void Client::initialize()
{
    // Initialize is called at the beginning of the simulation.

    addr = getIndex();
    serialNumber = 0;
    servers_number = par("servers_number");
    serverLeader = -1;

    // Timeouts
    requestMsgTimeout = 8;
    requestMsgTimeoutEvent = new cMessage("requestMsgTimeoutEvent");

    // client's attributes to watch during simulation
    WATCH(currentCommand);

    // schedule the time when client will send its first request
    scheduleAt(simTime()+requestMsgTimeout, requestMsgTimeoutEvent);


}

void Client::handleMessage(cMessage *msg)
{
    if (msg == requestMsgTimeoutEvent)
        handleRequestMsgTimeoutEvent(msg);
    else if (dynamic_cast<ServerReplyClientRequestMsg *>(msg))
        handleServerReplyClientRequestMsg((ServerReplyClientRequestMsg *) msg);
    else delete msg;
}



void Client::handleRequestMsgTimeoutEvent(cMessage *timeout)
{
    // either the Client should send the first request or it did not receive a response within the time to re-send the same request
    serverLeader = -1; // the Client should assume that the Server did not respond in time because it failed

    // check if it is client's first request
    if (currentRequestMsg == nullptr){
        bubble("Sending new request");
        currentRequestMsg = generateRequestMsg();
    }else // Re-sending most recent request
        bubble("Resending request");

    sendRequest(currentRequestMsg);
    scheduleAt(simTime()+requestMsgTimeout, requestMsgTimeoutEvent);
}



void Client::handleServerReplyClientRequestMsg(ServerReplyClientRequestMsg *serverReplyClientRequestMsg)
{
    // most recent leader known by the server that replied to the client request
    int replyMostRecentLeader = serverReplyClientRequestMsg->getLeaderAddr();
    serverLeader = replyMostRecentLeader;

    // destination server's address of the last client request message
    int oldDestServer = currentRequestMsg->getDestAddr();

    if (oldDestServer != replyMostRecentLeader){
        EV << "client_" << getIndex() << " sent request to the wrong server (server_" << oldDestServer << " was not the leader)\n";
        currentRequestMsg->setDestAddr(replyMostRecentLeader);
        bubble("Resending request");
    }else{
        EV << "client_" << getIndex() << " received a result=" << serverReplyClientRequestMsg->getResult() <<  " from server_" << replyMostRecentLeader << "\n";
        /* Processing result */

        bubble("Sending new request");
        //free memory and generate new request message
        delete currentRequestMsg;
        currentRequestMsg = generateRequestMsg();
    }
    sendRequest(currentRequestMsg);
    cancelEvent(requestMsgTimeoutEvent);
    scheduleAt(simTime()+requestMsgTimeout, requestMsgTimeoutEvent);

    delete serverReplyClientRequestMsg;

}


ClientRequestMsg * Client::generateRequestMsg()
{
    int serverAddr = serverLeader;
    if (serverLeader == -1)
        // send a request to one of the servers randomly
        serverAddr = intuniform(0, servers_number-1);

    // Generate a message with a different name every time.
    char msgName[40];
    sprintf(msgName, "request_%d", ++serialNumber);
    ClientRequestMsg *msg = new ClientRequestMsg(msgName);

    // assign source and destination address to the message
    msg->setSourceAddr(addr);
    msg->setDestAddr(serverAddr);
    msg->setSerialNumber(serialNumber);

    // assign command to the message
    currentCommand = Command('a' + rand()%26, rand()%10);
    msg->setCommand(currentCommand);

    return msg;
}

void Client::sendRequest(ClientRequestMsg *msg)
{
    // Duplicate message and send the copy.
    ClientRequestMsg *copy = (ClientRequestMsg *)msg->dup();


    EV << "client_" << getIndex() << " forwarding message " << msg << " towards server_" << copy->getDestAddr()
       <<" with command: " << currentCommand.var << " <-- " << currentCommand.value << "\n";

    /*
    * If Client can send a list of commands (for sake of simplicity, just 1 command as suggested from RAFT paper)
    for (auto const &cmd : msg->getCommand())
        EV << cmd.var << " <-- " << cmd.value << "; ";
    EV << "\n";
    */

    send(copy, "gate$o");
}


