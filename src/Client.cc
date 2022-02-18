/*
 * Client.cc
 *
 *  Created on: 18 Feb 2022
 *      Author: Giorgio Romeo, Daria Preda
 */

#include <omnetpp.h>

using namespace omnetpp;

class Client : public cSimpleModule
{
  private:
    int addr;

  protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    void sendRequest();
    void processReply(cMessage *msg);
};

Define_Module(Client);

void Client::initialize()
{
    addr = getIndex();

    cMessage *timer = new cMessage("timer");
    scheduleAt(simTime()+par("sendIaTime").doubleValue(), timer);
}

void Client::handleMessage(cMessage *msg)
{

}

void Client::sendRequest()
{

}

void Client::processReply(cMessage *msg)
{
    delete msg;
}


