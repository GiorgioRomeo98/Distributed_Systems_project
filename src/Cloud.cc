/*
 * Cloud.cc
 *
 *  Created on: 18 Feb 2022
 *      Author: Giorgio Romeo, Daria Preda
 */


#include <omnetpp.h>
#include "clientRequestMsg_m.h"
#include "serverReplyClientRequestMsg_m.h"

using namespace omnetpp;

/**
 * Represents the network "cloud" between clients and the server;
 */
class Cloud : public cSimpleModule
{
  private:
    simtime_t propDelay;

  protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    void handleClientRequestMsg(ClientRequestMsg *clientRequestMsg);
    void handleServerReplyClientRequestMsg(ServerReplyClientRequestMsg *replyClientRequestMsg);
};

Define_Module(Cloud);

void Cloud::initialize()
{
    // Initialize is called at the beginning of the simulation.

    propDelay = (double)par("propDelay");
}

void Cloud::handleMessage(cMessage *msg)
{
    if (dynamic_cast<ClientRequestMsg *>(msg))
        handleClientRequestMsg((ClientRequestMsg *) msg);
    else if (dynamic_cast<ServerReplyClientRequestMsg *>(msg))
        handleServerReplyClientRequestMsg((ServerReplyClientRequestMsg *) msg);

}

void Cloud::handleClientRequestMsg(ClientRequestMsg * clientRequestMsg)
{
    // determine destination address
    int dest = clientRequestMsg->getDestAddr();
    EV << "Relaying message to server_" << dest << endl;

    // send msg to destination after the delay
    sendDelayed(clientRequestMsg, propDelay, "gate_s$o", dest);
}



void Cloud::handleServerReplyClientRequestMsg(ServerReplyClientRequestMsg *replyClientRequestMsg)
{
    // determine destination address
    int dest = replyClientRequestMsg->getDestAddr();
    EV << "Relaying message to client_" << dest << endl;

    // send msg to destination after the delay
    sendDelayed(replyClientRequestMsg, propDelay, "gate_c$o", dest);
}

