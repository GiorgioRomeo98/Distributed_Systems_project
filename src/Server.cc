/*
 * Server.cc
 *
 *  Created on: 17 Feb 2022
 *      Authors: Giorgio Romeo, Daria Preda
 */


#include <string.h>
#include <omnetpp.h>

#include "logEntry.h"
#include "serverRequestVoteMsg_m.h"
#include "serverReplyVoteMsg_m.h"
#include "serverState.h"

using namespace omnetpp;

/**
 * Derive the Server class from cSimpleModule.
 */
class Server: public cSimpleModule
{

  private:
    int servers_number;

    serverState state = FOLLOWER;

    // A server remains in follower state as long as it receives valid notifications (RPCs) from a leader or candidate.
    simtime_t electionTimeout;  // timeout to pass from FOLLOWER state towards CANDIDATE state
    cMessage *electionTimeoutEvent;  // holds pointer to the electionTimeout self-message

    ServerRequestVoteMsg *requestVoteMsg;

    // Persistent state on each server
    int currentTerm = 0; //latest term server has seen (initialized to 0 on first boot, increases monotonically)
    int votedFor = -1; //candidateId that received vote in current term (or -1 if none)
    int votesNumber = 0;
    std::list<LogEntry> log_entries; //each entry contains command for state machine, and term when entry was received by leader (first index is 1)

    // Volatile state on each server
    int commitIndex = 0; //index of highest log entry known to be committed (initialized to 0, increases monotonically)
    int lastApplied = 0; //index of highest log entry applied to state machine (initialized to 0, increases monotonically)

    // Volatile state on leader (reinitialized after election)
    std::vector<int> nextIndex; //for each server, index of the next log entry to send to that server (initialized to leader last log index + 1)
    std::vector<int> matchIndex; //for each server, index of highest log entry known to be replicated on server (initialized to 0, increases monotonically)

  public:
    Server();
    virtual ~Server();

  protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    void forwardMessage(cMessage *msg);
    bool appendEntries(int term, int leaderId, int prevLogIndex, int prevLogTerm, int entries[], int leaderCommit);
    void handleElectionTimeoutEvent();
    void handleReplyVoteMsg(ServerReplyVoteMsg *replyVoteMsg);
    void sendRequestVoteMsg();
    void sendReplyVoteMsg(ServerRequestVoteMsg* requestVoteMsg);
    ServerRequestVoteMsg *generateRequestVoteMsg();
};

// The module class needs to be registered with OMNeT++
Define_Module(Server);



Server::Server()
{
    electionTimeoutEvent = requestVoteMsg = nullptr;
}



Server::~Server()
{
    cancelAndDelete(electionTimeoutEvent);
    delete(requestVoteMsg);

}



void Server::initialize()
{
    // Initialize is called at the beginning of the simulation.
    servers_number = par("servers_number");
    electionTimeout = exponential(5);
    electionTimeoutEvent = new cMessage("electionTimeoutEvent");
    nextIndex.resize(servers_number, 1);
    matchIndex.resize(servers_number, 0);


    // server's attributes to watch during simulation
    WATCH(state);
    WATCH(currentTerm);
    WATCH(votedFor);
    WATCH(votesNumber);
    WATCH_LIST(log_entries);
    WATCH(commitIndex);
    WATCH(lastApplied);
    WATCH_VECTOR(nextIndex);
    WATCH_VECTOR(matchIndex);

    scheduleAt(simTime()+electionTimeout, electionTimeoutEvent);

    /*if (getIndex() == 0) {
        // create and send first message on gate "out". "tictocMsg" is an
        // arbitrary string which will be the name of the message object.
        cMessage *msg = new cMessage("tictocMsg");
        scheduleAt(0.0, msg);
    }*/
}



void Server::handleMessage(cMessage *msg)
{
    // The handleMessage() method is called whenever a message arrives at the module.
    if (msg == electionTimeoutEvent)
        handleElectionTimeoutEvent();
    else if (dynamic_cast<ServerRequestVoteMsg *>(msg)){
        sendReplyVoteMsg((ServerRequestVoteMsg *) msg);
        delete msg;
    }
    else if (dynamic_cast<ServerReplyVoteMsg *>(msg)){
        handleReplyVoteMsg((ServerReplyVoteMsg *) msg);
    }
}




void Server::handleElectionTimeoutEvent()
{
    // election timeout expired, begin a new election phase: to begin an election, a follower increments its current term and
    // transitions to candidate state. It then votes for itself and issues RequestVote in parallel to each of the other servers.
    currentTerm++;
    state = CANDIDATE;
    votedFor = getIndex();
    votesNumber++;
    sendRequestVoteMsg();
    //scheduleAt(simTime() + timeout, timeoutEvent);
}



void Server::handleReplyVoteMsg(ServerReplyVoteMsg *replyVoteMsg)
{
    if (replyVoteMsg->getVoteGranted() and state == CANDIDATE){
        votesNumber++;
        int majority = ceil((double)servers_number/2);
        if (votesNumber >= majority){
            state = LEADER;
            bubble("Now I am the leader!");
            EV << "server_" << getIndex() << " is the new leader after obtaining " << votesNumber << " votes\n";
            // re-initializing variables
            votedFor = -1;
            votesNumber = 0;
        }
    }
}




bool Server::appendEntries(int term, int leaderId, int prevLogIndex, int prevLogTerm, int entries[], int leaderCommit)
{
    return true;
}



void Server::sendRequestVoteMsg()
{
    requestVoteMsg = generateRequestVoteMsg();

    for (int i = 0; i < servers_number; i++)
        if (i != getIndex()){
            EV << "Forwarding message " << requestVoteMsg << " towards server_" << i << "\n";
            // Duplicate message and send the copy
            send((ServerRequestVoteMsg *)requestVoteMsg->dup(), "gate$o", i);
        }

}



void Server::sendReplyVoteMsg(ServerRequestVoteMsg* requestVoteMsg)
{
    char msgName[40];
    sprintf(msgName, "replyVote_%d from server_%d", currentTerm, getIndex());
    ServerReplyVoteMsg *replyVoteMsg = new ServerReplyVoteMsg(msgName);

    bool voteGranted = false;
    if (requestVoteMsg->getTerm() >= currentTerm)
        if ((votedFor == -1 or votedFor == requestVoteMsg->getCandidateId()) /*and candidate’s log is at least as up-to-date as receiver’s log, grant vote*/)
            voteGranted = true;

    replyVoteMsg->setSource(getIndex());
    replyVoteMsg->setTerm(currentTerm);
    replyVoteMsg->setVoteGranted(voteGranted);

    int dest = requestVoteMsg->getSource();
    // sending the message
    EV << "Forwarding message " << replyVoteMsg << " towards server_" << dest << "\n";
    // Duplicate message and send the copy
    send(replyVoteMsg, "gate$o", dest);
}



ServerRequestVoteMsg *Server::generateRequestVoteMsg()
{
    char msgName[40];
    sprintf(msgName, "requestVote_%d from server_%d", currentTerm, getIndex());
    ServerRequestVoteMsg *msg = new ServerRequestVoteMsg(msgName);

    // assign source and destination address to the message
    msg->setSource(getIndex());
    msg->setTerm(currentTerm);
    msg->setCandidateId(getIndex());
    msg->setLastLogIndex(-1);
    msg->setLastLogTerm(-1);

    return msg;
}


