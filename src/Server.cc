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
#include "serverClientRequestInfo.h"
#include "serverRequestVoteMsg_m.h"
#include "serverReplyVoteMsg_m.h"
#include "serverAppendEntriesMsg_m.h"
#include "serverReplyAppendEntriesMsg_m.h"
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
    int clients_number;

    serverState state = FOLLOWER;

    // A server remains in follower state as long as it receives valid notifications (RPCs) from a leader or candidate.
    simtime_t electionTimeout;          // timeout to pass from FOLLOWER state towards CANDIDATE state
    cMessage *electionTimeoutEvent;     // holds pointer to the electionTimeout self-message

    simtime_t heartbeatTimeout;          // timeout to make current Leader send heartbeat messages to other servers
    cMessage *heartbeatTimeoutEvent;     // holds pointer to the electionTimeout self-message

    simtime_t appendEntriesTimeout;     // timeout to make current Leader send new log entries to other servers
    cMessage *appendEntriesTimeoutEvent;// holds pointer to the appendEntriesTimeout self-message

    ServerRequestVoteMsg *requestVoteMsg;
    ServerAppendEntriesMsg * appendEntriesMsg;


    // Persistent state on each server
    int currentTerm = 0;        //latest term server has seen (initialized to 0 on first boot, increases monotonically)
    int currentLeader = -1;     //index of the current leader
    int votedFor = -1;          //candidateId that received vote in current term (or -1 if none)
    int votesNumber = 0;
    std::list<LogEntry> logEntries;     //each entry contains commands for state machine, and term when entry was received by leader (first index is 1)
    std::list<LogEntry> stateMachine;   //commands applied to the server's state machine

    // Volatile state on each server
    int commitIndex = 0; //index of highest log entry known to be committed (initialized to 0, increases monotonically)
    int lastApplied = 0; //index of highest log entry applied to state machine (initialized to 0, increases monotonically)

    // Volatile state on leader (reinitialized after election)
    std::vector<int> nextIndex;     //for each server, index of the next log entry to send to that server (initialized to leader last log index + 1)
    std::vector<int> matchIndex;    //for each server, index of highest log entry known to be replicated on server (initialized to 0, increases monotonically)
    std::vector<ServerAppendEntriesMsg *> appendEntriesVect;        // for each server, current appendEntries message sent by the Leader
    std::vector<ServerClientRequestInfo *> clientRequestInfoVect;   // for each server, info about the most recent client request


  public:
    Server();
    virtual ~Server();

  protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    void forwardMessage(cMessage *msg);
    void appendEntries(bool isLeaderheartbeat);
    void handleElectionTimeoutEvent();
    void handleLeaderheartbeatTimeoutEvent();
    void handleRequestVoteMsg(ServerRequestVoteMsg *requestVoteMsg);
    void handleReplyVoteMsg(ServerReplyVoteMsg *replyVoteMsg);
    void handleClientRequestMsg(ClientRequestMsg *clientRequestMsg);
    void sendRequestVoteMsg();
    void sendReplyVoteMsg(ServerRequestVoteMsg* requestVoteMsg);
    void handleAppendEntriesMsg(ServerAppendEntriesMsg *appendEntriesMsg);
    void processAppendEntriesMsg(ServerAppendEntriesMsg *appendEntriesMsg);
    void handleReplyAppendEntriesMsg(ServerReplyAppendEntriesMsg *replyAppendEntriesMsg);
    void passToFollowerState(int updatedTerm);
    void passToLeaderState();
    void updateLastApplied();
    ServerRequestVoteMsg *generateRequestVoteMsg();
    ServerAppendEntriesMsg *generateAppendEntriesMsg(bool isLeaderheartbeat, int destServer);
};

// The module class needs to be registered with OMNeT++
Define_Module(Server);



Server::Server()
{
    electionTimeoutEvent = heartbeatTimeoutEvent = appendEntriesTimeoutEvent = nullptr;
    requestVoteMsg = nullptr;
}



Server::~Server()
{
    cancelAndDelete(electionTimeoutEvent);
    cancelAndDelete(heartbeatTimeoutEvent);
    cancelAndDelete(appendEntriesTimeoutEvent);
    delete(requestVoteMsg);
    for (int i=0; i < servers_number; i++)
        delete appendEntriesVect[i];


}



void Server::initialize()
{
    // Initialize is called at the beginning of the simulation.

    // set the begin marker of logEntries and stateMachine list
    logEntries.push_front(LogEntry());
    stateMachine.push_front(LogEntry());

    servers_number = par("servers_number");
    clients_number = par("clients_number");
    nextIndex.resize(servers_number, logEntries.size());
    matchIndex.resize(servers_number, 0);
    appendEntriesVect.resize(servers_number, nullptr);
    clientRequestInfoVect.resize(clients_number, new ServerClientRequestInfo());

    // Timeouts
    electionTimeout = normal(5.0, 1);
    electionTimeoutEvent = new cMessage("electionTimeoutEvent");
    heartbeatTimeout = 2;
    heartbeatTimeoutEvent = new cMessage("heartbeatTimeoutEvent");
    appendEntriesTimeout = 5;
    appendEntriesTimeoutEvent = new cMessage("appendEntriesTimeoutEvent");

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
}



void Server::handleMessage(cMessage *msg)
{
    // The handleMessage() method is called whenever a message arrives at the module.
    if (msg == electionTimeoutEvent)
        handleElectionTimeoutEvent();
    else if (msg == heartbeatTimeoutEvent)
        appendEntries(true);
    else if (msg == appendEntriesTimeoutEvent)
        appendEntries(false);
    else if (dynamic_cast<ServerRequestVoteMsg *>(msg))
        handleRequestVoteMsg((ServerRequestVoteMsg *) msg);
    else if (dynamic_cast<ServerReplyVoteMsg *>(msg))
        handleReplyVoteMsg((ServerReplyVoteMsg *) msg);
    else if (dynamic_cast<ServerAppendEntriesMsg *>(msg))
        handleAppendEntriesMsg((ServerAppendEntriesMsg *) msg);
    else if (dynamic_cast<ServerReplyAppendEntriesMsg *>(msg))
        handleReplyAppendEntriesMsg((ServerReplyAppendEntriesMsg *) msg);
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
    EV << "Elapsed leader election timeout for server_" << getIndex() << "\n";
    currentTerm++;
    state = CANDIDATE;
    votedFor = getIndex();
    votesNumber++;
    scheduleAt(simTime()+electionTimeout, electionTimeoutEvent);
    sendRequestVoteMsg();
}



// Leaders send periodic heartbeats (AppendEntriesMsgs that carry no log entries) to all followers in order to maintain their authority.
void Server::handleLeaderheartbeatTimeoutEvent()
{
    EV << "Elapsed heartbeat timeout for leader server_" << getIndex() << "\n";
    for (int i = 0; i < servers_number; i++)
        if (i != getIndex()){
            ServerAppendEntriesMsg *heartbeatAppendEntries = generateAppendEntriesMsg(true, i);
            EV << "Server_" << getIndex() << " forwarding message " << heartbeatAppendEntries << " towards server_" << i << "\n";
            // send message
            send((ServerAppendEntriesMsg *)heartbeatAppendEntries, "gate$o", i);
        }
    scheduleAt(simTime()+heartbeatTimeout, heartbeatTimeoutEvent);

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
        int majority = (servers_number/2) + 1;
        if (votesNumber >= majority)
            passToLeaderState();
    }
    delete replyVoteMsg;
}




// Invoked by leader to replicate log entries; also used as heartbeat
void Server::appendEntries(bool isLeaderheartbeat)
{
    if(isLeaderheartbeat)
        handleLeaderheartbeatTimeoutEvent();
    else{
        for (int i = 0; i < servers_number; i++)
                if (i != getIndex() and logEntries.size()-1 >= nextIndex[i]){
                    ServerAppendEntriesMsg *msg = generateAppendEntriesMsg(false, i);
                    EV << "Server_" << getIndex() << " forwarding message " << msg << " towards server_" << i << "\n";
                    // deleting a previously stored appendEntries message
                    delete appendEntriesVect[i];
                    // storing the message and sending a copy
                    appendEntriesVect[i] = msg;
                    send((ServerAppendEntriesMsg *)msg->dup(), "gate$o", i);
                }
        scheduleAt(simTime()+appendEntriesTimeout, appendEntriesTimeoutEvent);
    }
}



void Server::handleAppendEntriesMsg(ServerAppendEntriesMsg *appendEntriesMsg)
{
    currentLeader = appendEntriesMsg->getLeaderId();
    // check if it is a leader heartbeat message (entries should be empty)
    if (appendEntriesMsg->getEntries().empty()){
        EV << "Server_" << getIndex() << " received heartbeat message " << appendEntriesMsg << " from server_" << appendEntriesMsg->getLeaderId() << "\n";
        if (appendEntriesMsg->getTerm() >= currentTerm) // TODO: CHECK >= SIGN
            passToFollowerState(appendEntriesMsg->getTerm());
    }else{
        EV << "Server_" << getIndex() << " received appendEntries message " << appendEntriesMsg << " from server_" << appendEntriesMsg->getLeaderId() << "\n";
        processAppendEntriesMsg(appendEntriesMsg);
    }

    delete (appendEntriesMsg);
}



void Server::processAppendEntriesMsg(ServerAppendEntriesMsg *appendEntriesMsg)
{
    std::list<_logEntry> newLogEntries = appendEntriesMsg->getEntries();
    std::list<LogEntry>::iterator it = logEntries.begin();
    bool success = true;
    // Reply false if term < currentTerm or log does not contain an entry at prevLogIndex whose term matches prevLogTerm
    advance(it, appendEntriesMsg->getPrevLogIndex()); // meaningful only if logEntries.size()-1 >= appendEntriesMsg->getPrevLogIndex()
    if (appendEntriesMsg->getTerm() < currentTerm or logEntries.size()-1 < appendEntriesMsg->getPrevLogIndex() or (*it).term != appendEntriesMsg->getPrevLogTerm())
        success = false;
    else{
        // If an existing entry conflicts with a new one (same index but different terms), delete the existing entry and all that follow it
        // if the dimension of the logEntries list is less than the smallest index in the new entries the above scenario cannot happens
        // (-1 because there is a marker at position 0)
        std::list<LogEntry>::iterator iter = newLogEntries.begin();
        std::list<LogEntry>::iterator it = logEntries.begin();
        advance(it, (*iter).index);
        bool entriesToDelete = false;
        while (!entriesToDelete and iter != newLogEntries.end() and logEntries.size()-1 >= (*iter).index){
            if ((*it).term != (*iter).term)
                entriesToDelete = true;
            else{
                it++, iter++;
            }
        }for (; it != logEntries.end(); it++)
            logEntries.erase(it);

        // Append any new entries not already in the log
        iter = newLogEntries.begin();
        it = logEntries.begin();
        advance(it, newLogEntries.front().index);
        bool isAlreadyInserted = true;
        while(isAlreadyInserted and iter != newLogEntries.end() and logEntries.size()-1 >= (*iter).index){
            if ((*it).term != (*iter).term)
                isAlreadyInserted = false;
            else{
                it++; iter++;
            }
        }for (; iter != newLogEntries.end(); iter++)
            logEntries.push_back(*iter);

        // If leaderCommit > commitIndex, set commitIndex = min(leaderCommit, index of last new entry)
        int leaderCommit = appendEntriesMsg->getLeaderCommit();
        int oldcommitIndex = commitIndex;
        if (leaderCommit > commitIndex)
            commitIndex = std::min(leaderCommit, newLogEntries.back().index);
        if (oldcommitIndex < commitIndex)
            lastApplied = commitIndex;  //TODO implement state machine
    }
    char msgName[40];
    sprintf(msgName, "replyAppendEntries_%d", currentTerm);
    ServerReplyAppendEntriesMsg *msg = new ServerReplyAppendEntriesMsg(msgName);
    msg->setSource(getIndex());
    msg->setTerm(currentTerm);
    msg->setSuccess(success);
    EV << "Server_" << getIndex() << " forwarding message "<< msg << " towards server_" << appendEntriesMsg->getLeaderId() << "\n";
    send(msg, "gate$o", appendEntriesMsg->getLeaderId());
}



void Server::handleReplyAppendEntriesMsg(ServerReplyAppendEntriesMsg *replyAppendEntriesMsg)
{
    int sourceAddr = replyAppendEntriesMsg->getSource();
    EV << "Server_" << getIndex() << " received appendEntries reply message " << replyAppendEntriesMsg << " from server_" << sourceAddr << "\n";
    if (replyAppendEntriesMsg->getSuccess()){
        nextIndex[sourceAddr] = appendEntriesVect[sourceAddr]->getEntries().back().index + 1;
        matchIndex[sourceAddr] = appendEntriesVect[sourceAddr]->getEntries().back().index;
        updateLastApplied();
    }else
        nextIndex[sourceAddr]--;

    delete replyAppendEntriesMsg;
}



void Server::handleClientRequestMsg(ClientRequestMsg *clientRequestMsg)
{
    int clientAddr = clientRequestMsg->getSourceAddr();

    if (currentLeader != -1){
        if (currentLeader != getIndex()){
            EV << "server_" << getIndex() << " received a request from client_" << clientAddr << " even if it is not the Leader...server_"
               << getIndex() << " forwarding leader address to client_" << clientAddr << "\n";
            char msgName[40];
            sprintf(msgName, "reply_%d to client_%d request", currentTerm, clientAddr);
            ServerReplyClientRequestMsg *replyClientRequestMsg = new ServerReplyClientRequestMsg(msgName);
            replyClientRequestMsg->setSourceAddr(getIndex());
            replyClientRequestMsg->setDestAddr(clientAddr);
            replyClientRequestMsg->setLeaderAddr(currentLeader);
            send((ServerReplyClientRequestMsg *)replyClientRequestMsg, "gate$o", getIndex());
        }else{
            // TODO: implement serialNumber handling (same request should not be re-executed)
            int entryIndex = logEntries.size(); // remember marker at position 0
            int entryTerm = currentTerm;
            int serialNumber = clientRequestMsg->getSerialNumber();
            Command entryCommand = clientRequestMsg->getCommand();
            EV << "server_" << getIndex() << " received a request from client_" << clientAddr
               << " (command: " << entryCommand << "; serialNumber=" << serialNumber <<")\n";
            logEntries.push_back(LogEntry(entryIndex, entryTerm, entryCommand));
        }
    }

    delete clientRequestMsg;
}



// Invoked by candidates to gather votes
void Server::sendRequestVoteMsg()
{
    // to free memory
    delete requestVoteMsg;
    requestVoteMsg = generateRequestVoteMsg();

    for (int i = 0; i < servers_number; i++)
        if (i != getIndex()){
            EV << "Server_" << getIndex() << " forwarding message " << requestVoteMsg << " towards server_" << i << "\n";
            // Duplicate message and send the copy
            send((ServerRequestVoteMsg *)requestVoteMsg->dup(), "gate$o", i);
        }
}



void Server::sendReplyVoteMsg(ServerRequestVoteMsg* requestVoteMsg)
{
    char msgName[40];
    sprintf(msgName, "replyVote_%d", currentTerm);
    ServerReplyVoteMsg *replyVoteMsg = new ServerReplyVoteMsg(msgName);

    int candidateTerm = requestVoteMsg->getTerm();
    int candidateLastLogTerm = requestVoteMsg->getLastLogTerm();
    int candidateLastLogIndex = requestVoteMsg->getLastLogIndex();
    int dest = requestVoteMsg->getCandidateId();
    int lastLogTerm = logEntries.back().term;
    int lastLogIndex = logEntries.back().index;

    bool voteGranted = false;
    if (candidateTerm >= currentTerm){
        // If votedFor is null or candidateId, and candidate’s log is at least as up-to-date as receiver’s log, grant vote
        if ((votedFor == -1 or votedFor == requestVoteMsg->getCandidateId()) and
            (candidateLastLogTerm > lastLogTerm or (candidateLastLogTerm == lastLogTerm and candidateLastLogIndex >= lastLogIndex)))
            voteGranted = true;
    }

    replyVoteMsg->setSource(getIndex());
    replyVoteMsg->setTerm(currentTerm);
    replyVoteMsg->setVoteGranted(voteGranted);


    // sending the message
    EV << "Server_" << getIndex() << " forwarding message " << replyVoteMsg << " towards server_" << dest << "\n";
    // Duplicate message and send the copy
    send(replyVoteMsg, "gate$o", dest);
}



void Server::updateLastApplied()
{
    int majority = (servers_number/2) + 1;
    int replications;

    for (int i = 0; i < servers_number; i++){
        replications = 0;
        if (lastApplied < matchIndex[i]){
            replications++;
            for (int j=0; j < servers_number; j++)
                if (j != i and matchIndex[j] >= matchIndex[i])
                    replications++;
            if (replications >= majority)
                lastApplied = matchIndex[i];
        }
    }
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



void Server::passToLeaderState()
{
    state = LEADER;
    currentLeader = getIndex();
    bubble("Now I am the leader!");
    EV << "server_" << getIndex() << " is the new leader after obtaining " << votesNumber << " votes\n";

    // re-initializing variables
    votedFor = -1;
    votesNumber = 0;
    cancelEvent(electionTimeoutEvent);
    scheduleAt(simTime()+appendEntriesTimeout, appendEntriesTimeoutEvent);
    appendEntries(true);
}



ServerRequestVoteMsg *Server::generateRequestVoteMsg()
{
    char msgName[40];
    sprintf(msgName, "requestVote_%d", currentTerm);
    ServerRequestVoteMsg *msg = new ServerRequestVoteMsg(msgName);

    // assign values to message fields
    msg->setTerm(currentTerm);
    msg->setCandidateId(getIndex());
    msg->setLastLogIndex(-1);
    msg->setLastLogTerm(-1);

    return msg;
}



ServerAppendEntriesMsg *Server::generateAppendEntriesMsg(bool isLeaderheartbeat, int destServer)
{
    char msgName[40];
    ServerAppendEntriesMsg *msg;
    if (isLeaderheartbeat){
        sprintf(msgName, "heartbeat_%d", currentTerm);
        msg = new ServerAppendEntriesMsg(msgName);
        // assign values to appendEntries (heartbeat) message fields
        msg->setTerm(currentTerm);
        msg->setLeaderId(getIndex());
    }else{
        sprintf(msgName, "appendEntries_%d", currentTerm);
        msg = new ServerAppendEntriesMsg(msgName);

        // assign values to appendEntries message fields
        msg->setTerm(currentTerm);
        msg->setLeaderId(getIndex());
        msg->setLeaderCommit(commitIndex);
        msg->setPrevLogIndex(nextIndex[destServer]-1);

        std::list<LogEntry>::iterator it = logEntries.begin();  // Initialize iterator to list
        advance(it, nextIndex[destServer]-1);
        msg->setPrevLogTerm((*it).term);
        std::list<_logEntry> entries;
        for(it++; it != logEntries.end(); it++)
            entries.push_back(*it);
        msg->setEntries(entries);
    }
    return msg;
}

