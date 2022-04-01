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
    int servers_number;                 // total number of servers
    int clients_number;                 // total number of clients
    int failureProbability;             // probability of failure (value between 0 and 100)
    int recoveryProbability;            // probability of recovery from failure (value between 0 and 100)
    int messageLossProbability;         // probability of losing a message (value between 0 and 100)

    serverState state = FOLLOWER;

    // A server remains in follower state as long as it receives valid notifications (RPCs) from a leader or candidate.
    simtime_t electionTimeout;              // timeout to pass from FOLLOWER state towards CANDIDATE state
    cMessage *electionTimeoutEvent;         // holds pointer to the electionTimeout self-message

    simtime_t heartbeatTimeout;             // timeout to make current Leader send heartbeat messages to other servers
    cMessage *heartbeatTimeoutEvent;        // holds pointer to the electionTimeout self-message

    simtime_t appendEntriesTimeout;         // timeout to make current Leader send new log entries to other servers
    cMessage *appendEntriesTimeoutEvent;    // holds pointer to the appendEntriesTimeout self-message

    simtime_t checkFailureTimeout;          // timeout to check if the server failed
    cMessage *checkFailureTimeoutEvent;     // holds pointer to the checkFailureTimeout self-message

    simtime_t checkRecoveryTimeout;         // timeout to check if the server recovered from its failure
    cMessage *checkRecoveryTimeoutEvent;    // holds pointer to the checkRecoveryTimeout self-message


    // Persistent state on each server
    int currentTerm = 0;                //latest term server has seen (initialized to 0 on first boot, increases monotonically)
    int currentLeader = -1;             //index of the current leader
    int votedFor = -1;                  //candidateId that received vote in current term (or -1 if none)
    int votesNumber = 0;                //number of received votes to be elected as a Leader
    std::list<LogEntry> logEntries;     //each entry contains commands for state machine, and term when entry was received by leader (first index is 1)
    std::list<Command> stateMachine;    //server's state machine after commands executions

    // Volatile state on each server
    int commitIndex = 0; //index of highest log entry known to be committed (initialized to 0, increases monotonically)
    int lastApplied = 0; //index of highest log entry applied to state machine (initialized to 0, increases monotonically)
    std::vector<ServerAppendEntriesMsg *> appendEntriesVect;        // for each server, current appendEntries message sent by the Leader
    std::vector<ServerClientRequestInfo> clientRequestInfoVect;     // for each server, info about the most recent executed client request

    // Volatile state on leader (reinitialized after election)
    std::vector<int> nextIndex;     // for each server, index of the next log entry to send to that server (initialized to leader last log index + 1)
    std::vector<int> matchIndex;    // for each server, index of highest log entry known to be replicated on server (initialized to 0, increases monotonically)


    // statistics to monitor and associated vector
    cOutVector stateVector;
    cOutVector totMsgSentVector;
    cOutVector totMsgReceivedVector;
    cOutVector totRequestVoteMsgSentVector;
    cOutVector totAppendEntriesMsgSentVector;
    cOutVector totReplyToClientRequestMsgSentVector;
    cOutVector totReplyToRequestVoteMsgSentVector;
    cOutVector totReplyToAppendEntriesMsgSentVector;
    cOutVector electionsMeanTimeVector;     // monitor how many Leader elections happen and the mean time to elect a new Leader
    int totMsgSent;
    int totMsgReceived;
    int totRequestVoteMsgSent;
    int totAppendEntriesMsgSent;
    int totReplyToClientRequestMsgSent;
    int totReplyToRequestVoteMsgSent;
    int totReplyToAppendEntriesMsgSent;

    /*
     * This variable is used to store the time in which a new election phase began (value initialized to -1 after each new Leader election)
     * Note that only a value different from -1 is possible at each time between all servers (idea: for each new election store just
     * the first election phase time (e.g., if Leader fails at 2, the new election phase is started at 4 by server 2, then only
     * newElectionPhase of server 2 is set to 4 while all the others are -1, if this election phase fails and another begins at time 8
     * by server 3, there will be no updates since we want to monitor the time elapsed between the FIRST election phase time and the actual
     * Leader election)
     */
    simtime_t newElectionPhaseTime = -1;


  public:
    Server();
    virtual ~Server();

  protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;
    void forwardMessage(cMessage *msg);
    void appendEntries(bool isLeaderheartbeat);
    void handleFailureTimeoutEvent();
    void handleRecoveryTimeoutEvent();
    void handleElectionTimeoutEvent();
    void handleLeaderheartbeatTimeoutEvent();
    void handleRequestVoteMsg(ServerRequestVoteMsg *requestVoteMsg);
    void handleReplyVoteMsg(ServerReplyVoteMsg *replyVoteMsg);
    void handleClientRequestMsg(ClientRequestMsg *clientRequestMsg);
    void sendRequestVoteMsg();
    void sendReplyVoteMsg(ServerRequestVoteMsg* requestVoteMsg);
    void handleAppendEntriesMsg(ServerAppendEntriesMsg *appendEntriesMsg);
    void processHeartbeatMsg(ServerAppendEntriesMsg *appendEntriesMsg);
    void processAppendEntriesMsg(ServerAppendEntriesMsg *appendEntriesMsg);
    void handleReplyHeartbeatMsg(ServerReplyAppendEntriesMsg *replyAppendEntriesMsg);
    void handleReplyAppendEntriesMsg(ServerReplyAppendEntriesMsg *replyAppendEntriesMsg);
    void passToFollowerState(int updatedTerm=-1);
    void passToLeaderState();
    void passToFailedState();
    void updateCommitIndex();
    void updateStateMachine(int oldCommitIndex, int newCommitIndex);
    void collectLeaderElectionsInfo();
    ServerRequestVoteMsg *generateRequestVoteMsg();
    ServerAppendEntriesMsg *generateAppendEntriesMsg(bool isLeaderheartbeat, int destServer);
    ServerReplyClientRequestMsg *generateServerReplyClientRequestMsg (bool isRedirection, int dest);
};

// The module class needs to be registered with OMNeT++
Define_Module(Server);



Server::Server()
{
    electionTimeoutEvent = heartbeatTimeoutEvent = appendEntriesTimeoutEvent = nullptr;
    checkFailureTimeoutEvent = checkRecoveryTimeoutEvent = nullptr;
}



Server::~Server()
{
    cancelAndDelete(electionTimeoutEvent);
    cancelAndDelete(heartbeatTimeoutEvent);
    cancelAndDelete(appendEntriesTimeoutEvent);
    cancelAndDelete(checkFailureTimeoutEvent);
    cancelAndDelete(checkRecoveryTimeoutEvent);
    for (int i=0; i < servers_number; i++)
        delete appendEntriesVect[i];

}



void Server::initialize()
{
    // Initialize is called at the beginning of the simulation.

    // set the begin marker of logEntrieslist
    logEntries.push_front(LogEntry());

    servers_number = getParentModule()->par("servers_number");
    clients_number = getParentModule()->par("clients_number");
    failureProbability = par("failureProbability");
    recoveryProbability = par("recoveryProbability");
    messageLossProbability = par("messageLossProbability");
    nextIndex.resize(servers_number, logEntries.size());
    matchIndex.resize(servers_number, 0);
    appendEntriesVect.resize(servers_number, nullptr);
    clientRequestInfoVect.resize(clients_number, ServerClientRequestInfo());

    // Timeouts
    electionTimeout = par("electionTimeout");
    electionTimeoutEvent = new cMessage("electionTimeoutEvent");
    heartbeatTimeout = par("heartbeatTimeout");
    heartbeatTimeoutEvent = new cMessage("heartbeatTimeoutEvent");
    appendEntriesTimeout = par("appendEntriesTimeout");
    appendEntriesTimeoutEvent = new cMessage("appendEntriesTimeoutEvent");
    checkFailureTimeout = par("checkFailureTimeout");
    checkFailureTimeoutEvent = new cMessage("checkFailureTimeoutEvent");
    checkRecoveryTimeout = par("checkRecoveryTimeout");
    checkRecoveryTimeoutEvent = new cMessage("checkRecoveryTimeoutEvent");

    // server's attributes to watch during simulation
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
    WATCH(state);

    // assign name to statistics variables
    stateVector.setName("serverState");
    totMsgSentVector.setName("totMsgSent");
    totMsgReceivedVector.setName("totMsgReceived");
    totRequestVoteMsgSentVector.setName("totRequestVoteMsgSent");
    totAppendEntriesMsgSentVector.setName("totAppendEntriesMsgSent");
    totReplyToClientRequestMsgSentVector.setName("totReplyToClientRequestMsgSent");
    totReplyToRequestVoteMsgSentVector.setName("totReplyToRequestVoteMsgSent");
    totReplyToAppendEntriesMsgSentVector.setName("totReplyToAppendEntriesMsgSent");
    electionsMeanTimeVector.setName("electionsMeanTime");

    // set initial statistics variables
    totMsgSent = 0;
    totMsgReceived = 0;
    totRequestVoteMsgSent = 0;
    totAppendEntriesMsgSent = 0;
    totReplyToClientRequestMsgSent = 0;
    totReplyToRequestVoteMsgSent = 0;
    totReplyToAppendEntriesMsgSent = 0;

    // record initial value of state variable
    stateVector.record(state);

    // start Leader election timeout and check failure timeout
    scheduleAt(simTime()+electionTimeout, electionTimeoutEvent);
    scheduleAt(simTime()+checkFailureTimeout, checkFailureTimeoutEvent);
}



void Server::handleMessage(cMessage *msg)
{
    // The handleMessage() method is called whenever a message arrives at the module.

    if (msg == checkRecoveryTimeoutEvent)
        handleRecoveryTimeoutEvent();
    else if (state == FAILED){
        totMsgReceivedVector.record(++totMsgReceived);
        delete msg;
    }
    else if (msg == checkFailureTimeoutEvent)
        handleFailureTimeoutEvent();
    else if (msg == electionTimeoutEvent)
        handleElectionTimeoutEvent();
    else if (msg == heartbeatTimeoutEvent)
        appendEntries(true);
    else if (msg == appendEntriesTimeoutEvent)
        appendEntries(false);
    else if (intuniform(1, 100) <= messageLossProbability){
        bubble("Message loss!");
        EV << "Server_" << getIndex() << " lost message " << msg << "\n";
        totMsgReceivedVector.record(++totMsgReceived);
        delete msg;
    }
    else if (dynamic_cast<ServerRequestVoteMsg *>(msg))
        handleRequestVoteMsg((ServerRequestVoteMsg *) msg);
    else if (dynamic_cast<ServerReplyVoteMsg *>(msg))
        handleReplyVoteMsg((ServerReplyVoteMsg *) msg);
    else if (dynamic_cast<ServerAppendEntriesMsg *>(msg))
        handleAppendEntriesMsg((ServerAppendEntriesMsg *) msg);
    else if (dynamic_cast<ServerReplyAppendEntriesMsg *>(msg) and ((ServerReplyAppendEntriesMsg *) msg)->getIsHeartbeatReply())
        handleReplyHeartbeatMsg((ServerReplyAppendEntriesMsg *) msg);
    else if (dynamic_cast<ServerReplyAppendEntriesMsg *>(msg))
        handleReplyAppendEntriesMsg((ServerReplyAppendEntriesMsg *) msg);
    else if (dynamic_cast<ClientRequestMsg *>(msg))
        handleClientRequestMsg((ClientRequestMsg *) msg);
    else
        throw "impossible case";

}



void Server::finish()
{
    stateVector.record(state);
}



void Server::handleFailureTimeoutEvent()
{
    if (intuniform(1, 100) <= failureProbability)
        passToFailedState();
    else
        scheduleAt(simTime()+checkFailureTimeout, checkFailureTimeoutEvent);

}



void Server::handleRecoveryTimeoutEvent()
{
    if (intuniform(1, 100) <= recoveryProbability){
        bubble("Back to life");
        EV << "server_" << getIndex() << " has recovered from failure!\n";
        passToFollowerState(currentTerm);
        scheduleAt(simTime()+checkFailureTimeout, checkFailureTimeoutEvent);
    }else
        scheduleAt(simTime()+checkRecoveryTimeout, checkRecoveryTimeoutEvent);

}



void Server::handleElectionTimeoutEvent()
{
    // election timeout expired, begin a new election phase: to begin an election, a follower increments its current term and
    // transitions to candidate state. It then votes for itself and issues RequestVote in parallel to each of the other servers.
    bubble("Starting new election phase");
    EV << "Elapsed leader election timeout for server_" << getIndex() << "\n";
    currentTerm++;
    stateVector.record(state);
    state = CANDIDATE;
    stateVector.record(state);
    votedFor = getIndex();
    votesNumber = 1;
    scheduleAt(simTime()+electionTimeout, electionTimeoutEvent);
    sendRequestVoteMsg();

    // update newElectionPhaseTime
    bool alreadyUpdated = false;    // if newElectionPhaseTime is already different from -1 for a server, we skip the update
    for (int i = 0; i < servers_number and !alreadyUpdated; i++){
        std::string serverPath = "server[" + std::to_string(i) + "]";
        Server *server = (Server *)(getParentModule()->getModuleByPath(serverPath.c_str()));
        if (server->newElectionPhaseTime != -1)
            alreadyUpdated = true;
    }
    if (!alreadyUpdated)
        newElectionPhaseTime = simTime();
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
            totMsgSentVector.record(++totMsgSent);
            totAppendEntriesMsgSentVector.record(++totAppendEntriesMsgSent);
        }
    scheduleAt(simTime()+heartbeatTimeout, heartbeatTimeoutEvent);

}



// Server receives a ServerRequestVoteMsg
void Server::handleRequestVoteMsg(ServerRequestVoteMsg *requestVoteMsg)
{
    totMsgReceivedVector.record(++totMsgReceived);
    if (requestVoteMsg->getTerm() > currentTerm)
        passToFollowerState(requestVoteMsg->getTerm());

    sendReplyVoteMsg((ServerRequestVoteMsg *) requestVoteMsg);
    delete requestVoteMsg;
}


// Server receives a ServerReplyVoteMsg
void Server::handleReplyVoteMsg(ServerReplyVoteMsg *replyVoteMsg)
{
    totMsgReceivedVector.record(++totMsgReceived);
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
            if (i != getIndex() /*and logEntries.size()-1 >= nextIndex[i]*/){
                ServerAppendEntriesMsg *msg = generateAppendEntriesMsg(false, i);
                EV << "Server_" << getIndex() << " forwarding message " << msg << " towards server_" << i << "\n";
                // deleting the previously stored appendEntries message for server i
                delete appendEntriesVect[i];
                // storing the message and sending a copy
                appendEntriesVect[i] = msg;
                send((ServerAppendEntriesMsg *)msg->dup(), "gate$o", i);
                totMsgSentVector.record(++totMsgSent);
                totAppendEntriesMsgSentVector.record(++totAppendEntriesMsgSent);
            }
            scheduleAt(simTime()+appendEntriesTimeout, appendEntriesTimeoutEvent);
    }
}



void Server::handleAppendEntriesMsg(ServerAppendEntriesMsg *appendEntriesMsg)
{
    totMsgReceivedVector.record(++totMsgReceived);
    if (appendEntriesMsg->getTerm() >= currentTerm){
        currentLeader = appendEntriesMsg->getLeaderId();
        passToFollowerState(appendEntriesMsg->getTerm());
    }

    // check if it is a leader heartbeat message (log entries should be empty)
    if (appendEntriesMsg->getEntries().empty())
        processHeartbeatMsg(appendEntriesMsg);
    else
        processAppendEntriesMsg(appendEntriesMsg);

    delete (appendEntriesMsg);
}


/*
 * Leader periodically sends heartbeat messages to all other servers
 */
void Server::processHeartbeatMsg(ServerAppendEntriesMsg *appendEntriesMsg)
{
    EV << "Server_" << getIndex() << " received heartbeat message " << appendEntriesMsg << " from server_" << appendEntriesMsg->getLeaderId() << "\n";

    bool success = true;
    if (appendEntriesMsg->getTerm() < currentTerm)
        success = false;
    else{
        passToFollowerState(appendEntriesMsg->getTerm());
        int oldCommitIndex = commitIndex;
        if (commitIndex < appendEntriesMsg->getLeaderCommit()){
            oldCommitIndex = commitIndex;
            commitIndex = std::min(appendEntriesMsg->getLeaderCommit(), logEntries.back().index);
            if (oldCommitIndex < commitIndex)
                updateStateMachine(oldCommitIndex,  commitIndex);
        }
    }

    char msgName[40];
    sprintf(msgName, "replyHeartbeat_%d", currentTerm);
    ServerReplyAppendEntriesMsg *msg = new ServerReplyAppendEntriesMsg(msgName);
    msg->setSource(getIndex());
    msg->setTerm(currentTerm);
    msg->setSuccess(success);
    msg->setIsHeartbeatReply(true);

    EV << "Server_" << getIndex() << " forwarding reply heartbeat message " << msg << " towards server_" << appendEntriesMsg->getLeaderId() << "\n";
    send(msg, "gate$o", appendEntriesMsg->getLeaderId());
    totMsgSentVector.record(++totMsgSent);
    totReplyToAppendEntriesMsgSentVector.record(++totReplyToAppendEntriesMsgSent);

}


/*
 * Leader periodically sends AppendEntries messages to all other servers (if there are log entries to send)
 */
void Server::processAppendEntriesMsg(ServerAppendEntriesMsg *appendEntriesMsg)
{
    EV << "Server_" << getIndex() << " received appendEntries message " << appendEntriesMsg << " from server_" << appendEntriesMsg->getLeaderId() << "\n";
    if (appendEntriesMsg->getTerm() >= currentTerm)
        passToFollowerState(appendEntriesMsg->getTerm());

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
            updateStateMachine(oldcommitIndex, commitIndex);

        //update matchIndex (all the new log entries are now replicated in this server)
        matchIndex[getIndex()] = logEntries.back().index;
    }
    char msgName[40];
    sprintf(msgName, "replyAppendEntries_%d", currentTerm);
    ServerReplyAppendEntriesMsg *msg = new ServerReplyAppendEntriesMsg(msgName);
    msg->setSource(getIndex());
    msg->setTerm(currentTerm);
    msg->setSuccess(success);
    msg->setIsHeartbeatReply(false);
    EV << "Server_" << getIndex() << " forwarding message "<< msg << " towards server_" << appendEntriesMsg->getLeaderId() << "\n";
    send(msg, "gate$o", appendEntriesMsg->getLeaderId());
    totMsgSentVector.record(++totMsgSent);
    totReplyToAppendEntriesMsgSentVector.record(++totReplyToAppendEntriesMsgSent);
}


/*
 * Handling of a reply to a heartbeat message sent by each server to the Leader
 */
void Server::handleReplyHeartbeatMsg(ServerReplyAppendEntriesMsg *replyAppendEntriesMsg)
{
    totMsgReceivedVector.record(++totMsgReceived);
    int sourceAddr = replyAppendEntriesMsg->getSource();
    EV << "Server_" << getIndex() << " received heartbeat reply message " << replyAppendEntriesMsg << " from server_" << sourceAddr << "\n";
    if (!replyAppendEntriesMsg->getSuccess() and replyAppendEntriesMsg->getTerm() > currentTerm)
        passToFollowerState(replyAppendEntriesMsg->getTerm());
    delete replyAppendEntriesMsg;
}


void Server::handleReplyAppendEntriesMsg(ServerReplyAppendEntriesMsg *replyAppendEntriesMsg)
{
    totMsgReceivedVector.record(++totMsgReceived);
    int sourceAddr = replyAppendEntriesMsg->getSource();
    EV << "Server_" << getIndex() << " received appendEntries reply message " << replyAppendEntriesMsg << " from server_" << sourceAddr << "\n";
    if (replyAppendEntriesMsg->getSuccess()){
        nextIndex[sourceAddr] = appendEntriesVect[sourceAddr]->getEntries().back().index + 1;
        matchIndex[sourceAddr] = appendEntriesVect[sourceAddr]->getEntries().back().index;
        updateCommitIndex();
    }else{
        if (replyAppendEntriesMsg->getTerm() > currentTerm)
            passToFollowerState(replyAppendEntriesMsg->getTerm());
        else{
            EV << "Server_" << sourceAddr << " has log entries inconsistencies w.r.t "<< "leader Server_" << getIndex() << " log entries\n";
            nextIndex[sourceAddr]--;
            ServerAppendEntriesMsg *msg = generateAppendEntriesMsg(false, sourceAddr);
            EV << "Server_" << getIndex() << " forwarding message " << msg << " towards server_" << sourceAddr << "\n";
            // deleting the previously stored appendEntries message for server sourceAddr
            delete appendEntriesVect[sourceAddr];
            // storing the message and sending a copy
            appendEntriesVect[sourceAddr] = msg;
            send((ServerAppendEntriesMsg *)msg->dup(), "gate$o", sourceAddr);
            totMsgSentVector.record(++totMsgSent);
            totAppendEntriesMsgSentVector.record(++totAppendEntriesMsgSent);
        }
    }

    delete replyAppendEntriesMsg;
}



void Server::handleClientRequestMsg(ClientRequestMsg *clientRequestMsg)
{
    totMsgReceivedVector.record(++totMsgReceived);
    int clientAddr = clientRequestMsg->getSourceAddr();

    // check if there exists a Leader
    if (currentLeader != -1){
        // if the server that received the request is not the leader, it sends the address of the most recent known Leader
        if (currentLeader != getIndex()){
            EV << "server_" << getIndex() << " received a request from client_" << clientAddr << " even if it is not the Leader...server_"
               << getIndex() << " forwarding leader address to client_" << clientAddr << "\n";
            ServerReplyClientRequestMsg * replyClientRequestMsg = generateServerReplyClientRequestMsg(false, clientAddr);
            send(replyClientRequestMsg, "gate$o", getIndex());

        }
        else{  // the current Leader received the request
            // if the request has been already executed then the Leader immediately sends a response
            ServerClientRequestInfo requestInfo = clientRequestInfoVect[clientAddr];
            if (requestInfo.serialNumber == clientRequestMsg->getSerialNumber()){
                ServerReplyClientRequestMsg * replyClientRequestMsg = generateServerReplyClientRequestMsg(false, clientAddr);
                EV << "server_" << getIndex() << " already received this request from client_" << clientAddr << "...server_"
                   << getIndex() << " forwarding result to client_" << clientAddr << "\n";
                send(replyClientRequestMsg, "gate$o", getIndex());
            }else{
                //if the request is already present (but not executed given the previous if statement), then do nothing, otherwise add a new log
                std::list<LogEntry>::iterator it = logEntries.begin();
                for(advance(it,commitIndex); it != logEntries.end(); it++)
                    if ((*it).source == clientAddr and (*it).serialNumber == clientRequestMsg->getSerialNumber()){
                        delete clientRequestMsg;
                        return;
                    }

                // add new log entry
                int entryIndex = logEntries.size(); // remember marker at position 0
                int entryTerm = currentTerm;
                int serialNumber = clientRequestMsg->getSerialNumber();
                Command entryCommand = clientRequestMsg->getCommand();
                EV << "server_" << getIndex() << " received a request from client_" << clientAddr
                   << " (command: " << entryCommand << "; serialNumber=" << serialNumber <<")\n";
                logEntries.push_back(LogEntry(entryIndex, entryTerm, entryCommand, clientAddr, serialNumber));
                matchIndex[getIndex()] = entryIndex;
            }
        }
    }

    delete clientRequestMsg;
}



// Invoked by candidates to gather votes
void Server::sendRequestVoteMsg()
{
    ServerRequestVoteMsg *requestVoteMsg = generateRequestVoteMsg();

    for (int i = 0; i < servers_number; i++)
        if (i != getIndex()){
            EV << "Server_" << getIndex() << " forwarding message " << requestVoteMsg << " towards server_" << i << "\n";
            // Duplicate message and send the copy
            send((ServerRequestVoteMsg *)requestVoteMsg->dup(), "gate$o", i);
            totMsgSentVector.record(++totMsgSent);
            totRequestVoteMsgSentVector.record(++totRequestVoteMsgSent);
        }
    delete requestVoteMsg;
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
            (candidateLastLogTerm > lastLogTerm or (candidateLastLogTerm == lastLogTerm and candidateLastLogIndex >= lastLogIndex))){
            voteGranted = true;
            votedFor = requestVoteMsg->getCandidateId();
        }
    }

    replyVoteMsg->setSource(getIndex());
    replyVoteMsg->setTerm(currentTerm);
    replyVoteMsg->setVoteGranted(voteGranted);


    // sending the message
    EV << "Server_" << getIndex() << " forwarding message " << replyVoteMsg << " towards server_" << dest << "\n";
    // Duplicate message and send the copy
    send(replyVoteMsg, "gate$o", dest);
    totMsgSentVector.record(++totMsgSent);
    totReplyToClientRequestMsgSentVector.record(++totReplyToClientRequestMsgSent);
}



void Server::updateCommitIndex()
{
    int majority = (servers_number/2) + 1;
    int replications;
    int oldCommitIndex = commitIndex;
    std::list<LogEntry>::iterator it;

    // for each server, we take the highest index of the entry the Leader knows is replicated in that server --> if the index is greater than commitIndex
    // we can check if the entry has been replicated in the majority of the servers, if it is the case we update commitIndex
    // Careful: a leader cannot immediately conclude that an entry from a previous term is committed once it is stored on a majority of servers.
    //          Raft never commits log entries from previous terms by counting replicas. Only log entries from the leader’s current term are committed
    //          by counting replicas; once an entry from the current term has been committed in this way, then all prior entries are committed indirectly
    for (int i = 0; i < servers_number; i++){
        replications = 0;
        if (commitIndex < matchIndex[i]){
            it = logEntries.begin();
            advance(it, matchIndex[i]);
            /* this check would we possible if we had a dynamic number of client. Indeed, being the number of client static, there would
             * be an high probability that the leader does not receive a new command (one that is not already stored), thus commitIndex
             * would be never updated
             * if ((*it).term < currentTerm)
                continue;*/
            replications++;
            for (int j=0; j < servers_number; j++)
                if (j != i and matchIndex[j] >= matchIndex[i])
                    replications++;
            if (replications >= majority)
                commitIndex = matchIndex[i];
        }
    }
    if (oldCommitIndex < commitIndex)
        updateStateMachine(oldCommitIndex, commitIndex);
}



void Server::updateStateMachine(int oldCommitIndex, int newCommitIndex)
{
    // Initialize iterator to list
    std::list<LogEntry>::iterator it = logEntries.begin();
    int index = oldCommitIndex+1;
    for (advance(it, oldCommitIndex+1); index <= newCommitIndex; it++, index++){
        int varUpdated = false;
        char commandVar = (*it).entryCommand.var;
        int commandValue = (*it).entryCommand.value;
        for (std::list<Command>::iterator iter = stateMachine.begin(); iter != stateMachine.end() and !varUpdated; iter++)
            if ((*iter).var == commandVar){
                (*iter).value = commandValue;
                varUpdated = true;
            }
        if (!varUpdated){
            stateMachine.push_back(Command(commandVar,commandValue));
        }
        int clientAddr = (*it).source;
        // storing the result of the command execution in order to reply immediately if Client sent again the same request
        // (as a symbolic result we insert the command value)
        int result = commandValue;
        clientRequestInfoVect[clientAddr] = ServerClientRequestInfo((*it).serialNumber, commandValue);
        // forwarding the response of the executed command only if Server is the Leader (command executed when applied to state machine)
        if(getIndex() == currentLeader){
            EV << "server_" << getIndex() << " forwarding result=" << result << " to client_" << clientAddr << "\n";
            ServerReplyClientRequestMsg * replyMsg = generateServerReplyClientRequestMsg(false, clientAddr);
            send(replyMsg, "gate$o", getIndex());
            totMsgSentVector.record(++totMsgSent);
            totReplyToClientRequestMsgSentVector.record(++totReplyToClientRequestMsgSent);
        }
    }
    lastApplied = newCommitIndex;
}



void Server::passToFollowerState(int updatedTerm)
{
    if(currentTerm < updatedTerm or state!=FOLLOWER){
        stateVector.record(state);
        state = FOLLOWER;
        stateVector.record(state);
        votedFor = -1;
    }
    currentTerm = updatedTerm;
    votesNumber = 0;

    cancelEvent(electionTimeoutEvent);
    cancelEvent(heartbeatTimeoutEvent);
    cancelEvent(appendEntriesTimeoutEvent);
    scheduleAt(simTime()+electionTimeout, electionTimeoutEvent);
}



void Server::passToLeaderState()
{
    stateVector.record(state);
    state = LEADER;
    stateVector.record(state);
    collectLeaderElectionsInfo();
    currentLeader = getIndex();
    bubble("Now I am the leader!");
    EV << "server_" << getIndex() << " is the new leader after obtaining " << votesNumber << " votes\n";


    // re-initializing variables
    votedFor = -1;
    votesNumber = 0;
    for (int i=0; i < servers_number; i++){
        // nextIndex[i] should be initialized to logEntries.size()+1 according to the paper
        // (it may cause a bug if leader receives new entries, fails, becomes again leader)
        nextIndex[i] = commitIndex + 1;
        matchIndex[i] = 0;
    }
    matchIndex[getIndex()] = logEntries.back().index;

    // delete election timeout event, schedule appendEntriesTimeout event and trigger heartbeat message sending (appendEntries(true))
    cancelEvent(electionTimeoutEvent);
    cancelEvent(heartbeatTimeoutEvent);
    cancelEvent(appendEntriesTimeoutEvent);
    scheduleAt(simTime()+appendEntriesTimeout, appendEntriesTimeoutEvent);
    appendEntries(true);
}


void Server::passToFailedState()
{
    bubble("Failure!!!");
    EV << "server_" << getIndex() << " has failed!\n";

    stateVector.record(state);
    state = FAILED;
    votedFor = -1;
    votesNumber = 0;
    stateVector.record(state);

    cancelEvent(electionTimeoutEvent);
    cancelEvent(heartbeatTimeoutEvent);
    cancelEvent(appendEntriesTimeoutEvent);
    scheduleAt(simTime()+checkRecoveryTimeout, checkRecoveryTimeoutEvent);
}


/*
 * This method is used to collect information about Leader elections (total number and mean time)
 */

void Server::collectLeaderElectionsInfo()
{
    for (int i = 0; i < servers_number; i++){
        std::string serverPath = "server[" + std::to_string(i) + "]";
        Server *server = (Server *)(getParentModule()->getModuleByPath(serverPath.c_str()));
        if (server->newElectionPhaseTime != -1){
            Server *server_0 = (Server *)(getParentModule()->getModuleByPath("server[0]"));
            server_0->electionsMeanTimeVector.record(simTime() - server->newElectionPhaseTime);
            server->newElectionPhaseTime = -1;
        }
    }
}



ServerRequestVoteMsg *Server::generateRequestVoteMsg()
{
    char msgName[40];
    sprintf(msgName, "requestVote_%d", currentTerm);
    ServerRequestVoteMsg *msg = new ServerRequestVoteMsg(msgName);

    // assign values to message fields
    msg->setTerm(currentTerm);
    msg->setCandidateId(getIndex());
    msg->setLastLogIndex(logEntries.back().index);
    msg->setLastLogTerm(logEntries.back().term);

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
        msg->setLeaderCommit(commitIndex);
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


// isRedirection is true if the Client sent the request to the wrong server (not the leader), thus the Server should send the address of the most recent Leader
ServerReplyClientRequestMsg * Server::generateServerReplyClientRequestMsg (bool isRedirection, int dest)
{
    char msgName[40];
    sprintf(msgName, "reply_%d", currentTerm);
    ServerReplyClientRequestMsg *replyClientRequestMsg = new ServerReplyClientRequestMsg(msgName);
    replyClientRequestMsg->setSourceAddr(getIndex());
    replyClientRequestMsg->setDestAddr(dest);
    replyClientRequestMsg->setLeaderAddr(currentLeader);
    if (!isRedirection){
        replyClientRequestMsg->setResult(clientRequestInfoVect[dest].result);
        replyClientRequestMsg->setSerialNumber(clientRequestInfoVect[dest].serialNumber);
    }
    return replyClientRequestMsg;
}

