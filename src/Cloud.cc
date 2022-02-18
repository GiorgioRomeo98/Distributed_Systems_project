/*
 * Cloud.cc
 *
 *  Created on: 18 Feb 2022
 *      Author: Giorgio Romeo, Daria Preda
 */


#include <omnetpp.h>

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
};

Define_Module(Cloud);

void Cloud::initialize()
{
    propDelay = (double)par("propDelay");
}

void Cloud::handleMessage(cMessage *msg)
{
    // determine destination address
    /*NetPkt *pkt = check_and_cast<NetPkt *>(msg);
    int dest = pkt->getDestAddress();
    EV << "Relaying packet to addr=" << dest << endl;*/

    // send msg to destination after the delay
    //sendDelayed(pkt, propDelay, "g$o", dest);
}

