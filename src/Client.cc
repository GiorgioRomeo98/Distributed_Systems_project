/*
 * Client.cc
 *
 *  Created on: 18 Feb 2022
 *      Author: Giorgio Romeo, Daria Preda
 */

#include <omnetpp.h>
#include "clientRequestMsg_m.h"

using namespace omnetpp;

class Client: public cSimpleModule
{
  private:
    int addr;   // client source address
    int seq;    // message sequence number
    int servers_number; // total number of servers

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
    ClientRequestMsg *generateNewMessage();
    void sendRequest(ClientRequestMsg * msg);
    void processReply(cMessage *msg);
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
    seq = 0;
    servers_number = par("servers_number");

    WATCH(currentCommand);

    // client's attributes to watch during simulation

    if (getIndex() == 0){
        requestMsgTimeoutEvent = new cMessage("requestMsgTimeoutEvent");
        scheduleAt(simTime()+par("sendIaTime").doubleValue()+100, requestMsgTimeoutEvent);
    }

}

void Client::handleMessage(cMessage *msg)
{
    currentRequestMsg = generateNewMessage();
    sendRequest(currentRequestMsg);
}

ClientRequestMsg * Client::generateNewMessage()
{
    // send a request to one of the servers randomly
    int server_addr = intuniform(0, servers_number-1);

    // Generate a message with a different name every time.
    char msgName[40];
    sprintf(msgName, "request_%d: client_%d --> server_%d", ++seq, addr, server_addr);
    ClientRequestMsg *msg = new ClientRequestMsg(msgName);

    // assign source and destination address to the message
    msg->setSource_addr(addr);
    msg->setDestination_addr(server_addr);

    // assign command to the message
    currentCommand = Command('x',5);
    msg->setCommand(currentCommand);

    return msg;
}

void Client::sendRequest(ClientRequestMsg * msg)
{
    // Duplicate message and send the copy.
    ClientRequestMsg *copy = (ClientRequestMsg *)msg->dup();


    EV << "Forwarding message " << msg << " with command: " << currentCommand.var << " <-- " << currentCommand.value << "\n";
    /*
    * If Client can send a list og commands (for sake of simplicity, just 1 command as suggested from RAFT paper)
    for (auto const &cmd : msg->getCommand())
        EV << cmd.var << " <-- " << cmd.value << "; ";
    EV << "\n";
    */

    send(copy, "gate$o");
}

void Client::processReply(cMessage *msg)
{
    delete msg;
}


