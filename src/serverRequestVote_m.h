//
// Generated file, do not edit! Created by nedtool 5.6 from serverRequestVote.msg.
//

#ifndef __SERVERREQUESTVOTE_M_H
#define __SERVERREQUESTVOTE_M_H

#if defined(__clang__)
#  pragma clang diagnostic ignored "-Wreserved-id-macro"
#endif
#include <omnetpp.h>

// nedtool version check
#define MSGC_VERSION 0x0506
#if (MSGC_VERSION!=OMNETPP_VERSION)
#    error Version mismatch! Probably this file was generated by an earlier version of nedtool: 'make clean' should help.
#endif



/**
 * Class generated from <tt>serverRequestVote.msg:16</tt> by nedtool.
 * <pre>
 * message ServerRequestVote
 * {
 *     int term;	//candidate�s term
 *     int candidateId;	//candidate requesting vote
 *     int lastLogIndex;	// index of candidate�s last log entry
 *     int lastLogTerm;	// term of candidate�s last log entry
 * }
 * </pre>
 */
class ServerRequestVote : public ::omnetpp::cMessage
{
  protected:
    int term;
    int candidateId;
    int lastLogIndex;
    int lastLogTerm;

  private:
    void copy(const ServerRequestVote& other);

  protected:
    // protected and unimplemented operator==(), to prevent accidental usage
    bool operator==(const ServerRequestVote&);

  public:
    ServerRequestVote(const char *name=nullptr, short kind=0);
    ServerRequestVote(const ServerRequestVote& other);
    virtual ~ServerRequestVote();
    ServerRequestVote& operator=(const ServerRequestVote& other);
    virtual ServerRequestVote *dup() const override {return new ServerRequestVote(*this);}
    virtual void parsimPack(omnetpp::cCommBuffer *b) const override;
    virtual void parsimUnpack(omnetpp::cCommBuffer *b) override;

    // field getter/setter methods
    virtual int getTerm() const;
    virtual void setTerm(int term);
    virtual int getCandidateId() const;
    virtual void setCandidateId(int candidateId);
    virtual int getLastLogIndex() const;
    virtual void setLastLogIndex(int lastLogIndex);
    virtual int getLastLogTerm() const;
    virtual void setLastLogTerm(int lastLogTerm);
};

inline void doParsimPacking(omnetpp::cCommBuffer *b, const ServerRequestVote& obj) {obj.parsimPack(b);}
inline void doParsimUnpacking(omnetpp::cCommBuffer *b, ServerRequestVote& obj) {obj.parsimUnpack(b);}


#endif // ifndef __SERVERREQUESTVOTE_M_H
