/*
 * Server.cc
 *
 *  Created on: 17 Feb 2022
 *      Authors: Giorgio Romeo, Daria Preda
 */


#include <string.h>
#include <omnetpp.h>

#include "log_entry.h"

using namespace omnetpp;

/**
 * Derive the Node class from cSimpleModule.
 */
class Server: public cSimpleModule
{
  private:
    int servers_number;

    // Persistent state on each server
    int currentTerm = 0; //latest term server has seen (initialized to 0 on first boot, increases monotonically)
    int votedFor = -1; //candidateId that received vote in current term (or null if none)
    std::list<log_entry> log_entries; //each entry contains command for state machine, and term when entry was received by leader (first index is 1)

    // Volatile state on each server
    int commitIndex = 0; //index of highest log entry known to be committed (initialized to 0, increases monotonically)
    int lastApplied = 0; //index of highest log entry applied to state machine (initialized to 0, increases monotonically)

    // Volatile state on leader (reinitialized after election)
    std::vector<int> nextIndex; //for each server, index of the next log entry to send to that server (initialized to leader last log index + 1)
    std::vector<int> matchIndex; //for each server, index of highest log entry known to be replicated on server (initialized to 0, increases monotonically)

  protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void forwardMessage(cMessage *msg);
    virtual bool appendEntries(int term, int leaderId, int prevLogIndex, int prevLogTerm, int entries[], int leaderCommit);
    virtual void requestVote(int term, int candidateId, int lastLogIndex, int LastLogTerm);
};

// The module class needs to be registered with OMNeT++
Define_Module(Server);

void Server::initialize()
{
    // Initialize is called at the beginning of the simulation.

    servers_number = par("servers_number");
    nextIndex.resize(servers_number, 1);
    matchIndex.resize(servers_number, 0);


    // server's attributes to watch during simulation
    WATCH(currentTerm);
    WATCH(votedFor);
    WATCH_LIST(log_entries);
    WATCH(commitIndex);
    WATCH(lastApplied);
    WATCH_VECTOR(nextIndex);
    WATCH_VECTOR(matchIndex);


    if (getIndex() == 0) {
        // create and send first message on gate "out". "tictocMsg" is an
        // arbitrary string which will be the name of the message object.
        cMessage *msg = new cMessage("tictocMsg");
        scheduleAt(0.0, msg);
    }
}

void Server::handleMessage(cMessage *msg)
{
    // The handleMessage() method is called whenever a message arrives
    // at the module. Here, we just send it to the other module
    forwardMessage(msg);
}

void Server::forwardMessage(cMessage *msg)
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

bool Server::appendEntries(int term, int leaderId, int prevLogIndex, int prevLogTerm, int entries[], int leaderCommit)
{
    return true;
}

void Server::requestVote(int term, int candidateId, int lastLogIndex, int LastLogTerm)
{

}


