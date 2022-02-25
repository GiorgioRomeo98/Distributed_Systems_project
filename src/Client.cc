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

    simtime_t requestMsgTimeout;  // timeout
    cMessage *requestMsgTimeoutEvent;  // holds pointer to the requestMsgTimeout self-message
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
    // Set the pointer to nullptr, so that the destructor won't crash even if initialize() doesn't get called because of a runtime
    // error or user cancellation during the startup process.
    //event = tictocMsg = nullptr;

    addr = getIndex();
    serialNumber = 0;
    servers_number = par("servers_number");
    serverLeader = -1;

    WATCH(currentCommand);

    // client's attributes to watch during simulation

    if (getIndex() >= 0){   //TODO: fix >=
        requestMsgTimeoutEvent = new cMessage("requestMsgTimeoutEvent");
        scheduleAt(simTime()+par("sendIaTime").doubleValue(), requestMsgTimeoutEvent);
    }

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
    //free memory and generate new request message
    delete currentRequestMsg;
    currentRequestMsg = generateRequestMsg();

    bubble("Sending request");
    sendRequest(currentRequestMsg);

    // set again the timeout to send a new request
    // TODO: DIFFER TIMEOUT FOR A NEW REQUEST AND A TIMEOUT FOR RESENDING AN OLD REQUEST WITH NO REPLIES

    scheduleAt(simTime()+par("sendIaTime").doubleValue(), requestMsgTimeoutEvent);
}



void Client::handleServerReplyClientRequestMsg(ServerReplyClientRequestMsg *serverReplyClientRequestMsg)
{
    // most recent leader known by the server that replied to the client request
    int replyMostRecentLeader = serverReplyClientRequestMsg->getLeaderAddr();

    // destination server's address of the last client request message
    int oldDestServer = currentRequestMsg->getDestAddr();

    if (oldDestServer != replyMostRecentLeader){
        serverLeader = replyMostRecentLeader;
        EV << "client_" << getIndex() << " sent request to the wrong server (server_" << oldDestServer << " was not the leader)\n";
        //free memory and generate new request message
        delete currentRequestMsg;
        currentRequestMsg = generateRequestMsg();
        sendRequest(currentRequestMsg);
    }else{
        if (serverLeader == -1)
            serverLeader = replyMostRecentLeader;
        /* TODO: PROCESS RESULT */ bubble("Processing result");
    }
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

void Client::sendRequest(ClientRequestMsg * msg)
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


