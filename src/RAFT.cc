/*
 * RAFT.cc
 *
 *  Created on: 17 Feb 2022
 *      Authors: Giorgio Romeo, Daria Preda
 */


#include <string.h>
#include <omnetpp.h>

using namespace omnetpp;

/**
 * Derive the Node class from cSimpleModule.
 */
class Node : public cSimpleModule
{
  protected:
    // The following redefined virtual function holds the algorithm.
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void forwardMessage(cMessage *msg);
};

// The module class needs to be registered with OMNeT++
Define_Module(Node);

void Node::initialize()
{
    // Initialize is called at the beginning of the simulation.
    if (getIndex() == 0) {
        // create and send first message on gate "out". "tictocMsg" is an
        // arbitrary string which will be the name of the message object.
        cMessage *msg = new cMessage("tictocMsg");
        EV << "Forwarding message " << msg << " on gate[" << 1 << "]\n";
        send(msg, "gate$o", 1);
    }
}

void Node::handleMessage(cMessage *msg)
{
    // The handleMessage() method is called whenever a message arrives
    // at the module. Here, we just send it to the other module
    forwardMessage(msg);
}

void Node::forwardMessage(cMessage *msg)
{
    // random gate.
    int n;
    int k;
    do{
        n = gateSize("gate");
        k = intuniform(0, n-1);
    }while(k == getIndex());

    EV << "Forwarding message " << msg << " on gate[" << k << "]\n";
    send(msg, "gate$o", k);
}
