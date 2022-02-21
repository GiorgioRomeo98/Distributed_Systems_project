/*
 * Server.cc
 *
 *  Created on: 17 Feb 2022
 *      Authors: Giorgio Romeo, Daria Preda
 */


#include <string.h>
#include <omnetpp.h>

#include "logEntry.h"
#include "serverState.h"
#include "serverRequestVoteMsg_m.h"
#include "serverReplyVoteMsg_m.h"
#include "serverAppendEntriesMsg_m.h"
#include "clientRequestMsg_m.h"
#include "serverReplyClientRequestMsg_m.h"

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

    simtime_t heartbitTimeout;
    cMessage *heartbitTimeoutEvent;

    ServerRequestVoteMsg *requestVoteMsg;
    ServerAppendEntriesMsg * appendEntriesMsg;


    // Persistent state on each server
    int currentTerm = 0;        //latest term server has seen (initialized to 0 on first boot, increases monotonically)
    int currentLeader = -1;     //index of the current leader
    int votedFor = -1;          //candidateId that received vote in current term (or -1 if none)
    int votesNumber = 0;
    std::list<LogEntry> logEntries;     //each entry contains commanda for state machine, and term when entry was received by leader (first index is 1)
    std::list<LogEntry> stateMachine;   //commands applied to the server's state machine

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
    void handleLeaderHeartbitTimeoutEvent();
    void handleRequestVoteMsg(ServerRequestVoteMsg *requestVoteMsg);
    void handleReplyVoteMsg(ServerReplyVoteMsg *replyVoteMsg);
    void handleAppendEntriesMsg(ServerAppendEntriesMsg *appendEntriesMsg);
    void handleClientRequestMsg(ClientRequestMsg *clientRequestMsg);
    void sendRequestVoteMsg();
    void sendReplyVoteMsg(ServerRequestVoteMsg* requestVoteMsg);
    void passToFollowerState(int updatedTerm);
    ServerRequestVoteMsg *generateRequestVoteMsg();
    ServerAppendEntriesMsg *generateAppendEntriesMsg();
};

// The module class needs to be registered with OMNeT++
Define_Module(Server);



Server::Server()
{
    electionTimeoutEvent = heartbitTimeoutEvent = requestVoteMsg = nullptr;
}



Server::~Server()
{
    cancelAndDelete(electionTimeoutEvent);
    cancelAndDelete(heartbitTimeoutEvent);
    delete(requestVoteMsg);

}



void Server::initialize()
{
    // Initialize is called at the beginning of the simulation.
    servers_number = par("servers_number");
    nextIndex.resize(servers_number, logEntries.size()+1);
    matchIndex.resize(servers_number, 0);

    // Timeouts
    electionTimeout = normal(5.0,1.0);
    electionTimeoutEvent = new cMessage("electionTimeoutEvent");
    heartbitTimeout = 2;
    heartbitTimeoutEvent = new cMessage("heartbitTimeoutEvent");

    // server's attributes to watch during simulation
    WATCH(state);
    WATCH(currentTerm);
    WATCH(currentLeader);
    WATCH(votedFor);
    WATCH(votesNumber);
    WATCH_LIST(logEntries);
    WATCH_LIST(stateMachine);
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
    else if (msg == heartbitTimeoutEvent)
        handleLeaderHeartbitTimeoutEvent();
    else if (dynamic_cast<ServerRequestVoteMsg *>(msg))
        handleRequestVoteMsg((ServerRequestVoteMsg *) msg);
    else if (dynamic_cast<ServerReplyVoteMsg *>(msg))
        handleReplyVoteMsg((ServerReplyVoteMsg *) msg);
    else if (dynamic_cast<ServerAppendEntriesMsg *>(msg))
        handleAppendEntriesMsg((ServerAppendEntriesMsg *) msg);
    else if (dynamic_cast<ClientRequestMsg *>(msg))
        handleClientRequestMsg((ClientRequestMsg *) msg);
    else
        delete msg;

}



void Server::handleElectionTimeoutEvent()
{
    // election timeout expired, begin a new election phase: to begin an election, a follower increments its current term and
    // transitions to candidate state. It then votes for itself and issues RequestVote in parallel to each of the other servers.
    bubble("Starting new election phase");
    currentTerm++;
    state = CANDIDATE;
    votedFor = getIndex();
    votesNumber++;
    scheduleAt(simTime()+electionTimeout, electionTimeoutEvent);
    sendRequestVoteMsg();
}


// Server receives a ServerRequestVoteMsg
void Server::handleRequestVoteMsg(ServerRequestVoteMsg *requestVoteMsg)
{
    if (requestVoteMsg->getTerm() > currentTerm)
        passToFollowerState(requestVoteMsg->getTerm());

    sendReplyVoteMsg((ServerRequestVoteMsg *) requestVoteMsg);
    delete requestVoteMsg;
}


// Server receives a ServerReplyVoteMsg
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
            cancelEvent(electionTimeoutEvent);
            handleLeaderHeartbitTimeoutEvent();
        }
    }
    delete replyVoteMsg;
}



void Server::handleAppendEntriesMsg(ServerAppendEntriesMsg *appendEntriesMsg)
{
    currentLeader = appendEntriesMsg->getLeaderId();
    // check if it is a leader heartbit message (entries should be empty)
    if (appendEntriesMsg->getEntries().empty())
        EV << "Server_" << getIndex() << " received heartbit message " << appendEntriesMsg << "\n";
        if (appendEntriesMsg->getTerm() >= currentTerm) //CHECK >= SIGN
            passToFollowerState(appendEntriesMsg->getTerm());
    delete (appendEntriesMsg);

}


void Server::handleClientRequestMsg(ClientRequestMsg *clientRequestMsg)
{
    int clientAddr = clientRequestMsg->getSourceAddr();

    char msgName[40];
    sprintf(msgName, "reply_%d from server_%d to client_%d request", currentTerm, getIndex(), clientAddr);
    ServerReplyClientRequestMsg *replyClientRequestMsg = new ServerReplyClientRequestMsg(msgName);

    replyClientRequestMsg->setSourceAddr(getIndex());
    replyClientRequestMsg->setDestAddr(clientAddr);

    if (currentLeader != -1)
        if (currentLeader != getIndex()){
            EV << "server_" << getIndex() << " received a client request even if it is not the Leader...server_" << getIndex()
               << " forwarding leader address to client_" << clientAddr << "\n";
            replyClientRequestMsg->setLeaderAddr(currentLeader);
        }
    send((ClientRequestMsg *)replyClientRequestMsg, "gate$o", getIndex());

    delete clientRequestMsg;
}



// Leaders send periodic heartbeats (AppendEntriesMsgs that carry no log entries) to all followers in order to maintain their authority.
void Server::handleLeaderHeartbitTimeoutEvent()
{
    ServerAppendEntriesMsg *heartbitAppendEntries = generateAppendEntriesMsg();

    for (int i = 0; i < servers_number; i++)
        if (i != getIndex()){
            EV << "Forwarding message " << heartbitAppendEntries << " towards server_" << i << "\n";
            // Duplicate message and send the copy
            send((ServerAppendEntriesMsg *)heartbitAppendEntries->dup(), "gate$o", i);
        }
    scheduleAt(simTime()+heartbitTimeout, heartbitTimeoutEvent);

    delete heartbitAppendEntries;
}



// Invoked by leader to replicate log entries; also used as heartbeat
bool Server::appendEntries(int term, int leaderId, int prevLogIndex, int prevLogTerm, int entries[], int leaderCommit)
{



    return true;
}


// Invoked by candidates to gather votes
void Server::sendRequestVoteMsg()
{
    // to free memory
    delete requestVoteMsg;
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

    int sourceTerm = requestVoteMsg->getTerm();
    /*if (sourceTerm > currentTerm and state != FOLLOWER)
        passToFollowerState(sourceTerm);
    */

    bool voteGranted = false;
    if (sourceTerm >= currentTerm){  //CHECK THE >= SIGN
        if ((votedFor == -1 or votedFor == requestVoteMsg->getCandidateId()) /*and candidate’s log is at least as up-to-date as receiver’s log, grant vote*/)
            voteGranted = true;
    }

    replyVoteMsg->setSource(getIndex());
    replyVoteMsg->setTerm(currentTerm);
    replyVoteMsg->setVoteGranted(voteGranted);

    int dest = requestVoteMsg->getSource();
    // sending the message
    EV << "Forwarding message " << replyVoteMsg << " towards server_" << dest << "\n";
    // Duplicate message and send the copy
    send(replyVoteMsg, "gate$o", dest);
}



void Server::passToFollowerState(int updatedTerm)
{
    state = FOLLOWER;
    currentTerm = updatedTerm;
    votedFor = -1;
    votesNumber = 0;
    cancelEvent(electionTimeoutEvent);
    scheduleAt(simTime()+electionTimeout, electionTimeoutEvent);

}



ServerRequestVoteMsg *Server::generateRequestVoteMsg()
{
    char msgName[40];
    sprintf(msgName, "requestVote_%d from server_%d", currentTerm, getIndex());
    ServerRequestVoteMsg *msg = new ServerRequestVoteMsg(msgName);

    // assign values to message fields
    msg->setSource(getIndex());
    msg->setTerm(currentTerm);
    msg->setCandidateId(getIndex());
    msg->setLastLogIndex(-1);
    msg->setLastLogTerm(-1);

    return msg;
}



ServerAppendEntriesMsg *Server::generateAppendEntriesMsg()
{
    char msgName[40];
    //CHANGE HEARTBIT
    sprintf(msgName, "heartbit_%d from server_%d", currentTerm, getIndex());
    ServerAppendEntriesMsg *msg = new ServerAppendEntriesMsg(msgName);

    // assign values to message fields
    msg->setTerm(currentTerm);
    msg->setLeaderId(getIndex());
    msg->setPrevLogIndex(-1);
    msg->setPrevLogTerm(-1);
    //msg->entries;
    msg->setLeaderCommit(commitIndex);

    return msg;
}

