//
// Generated file, do not edit! Created by nedtool 5.6 from serverReplyVoteMsg.msg.
//

#ifndef __SERVERREPLYVOTEMSG_M_H
#define __SERVERREPLYVOTEMSG_M_H

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
 * Class generated from <tt>serverReplyVoteMsg.msg:16</tt> by nedtool.
 * <pre>
 * message ServerReplyVoteMsg
 * {
 *     int source;			// address of the sender server
 *     int term;			// current term of the replying server (for candidate to update itself)
 *     bool voteGranted;	// true means candidate received vote
 * }
 * </pre>
 */
class ServerReplyVoteMsg : public ::omnetpp::cMessage
{
  protected:
    int source;
    int term;
    bool voteGranted;

  private:
    void copy(const ServerReplyVoteMsg& other);

  protected:
    // protected and unimplemented operator==(), to prevent accidental usage
    bool operator==(const ServerReplyVoteMsg&);

  public:
    ServerReplyVoteMsg(const char *name=nullptr, short kind=0);
    ServerReplyVoteMsg(const ServerReplyVoteMsg& other);
    virtual ~ServerReplyVoteMsg();
    ServerReplyVoteMsg& operator=(const ServerReplyVoteMsg& other);
    virtual ServerReplyVoteMsg *dup() const override {return new ServerReplyVoteMsg(*this);}
    virtual void parsimPack(omnetpp::cCommBuffer *b) const override;
    virtual void parsimUnpack(omnetpp::cCommBuffer *b) override;

    // field getter/setter methods
    virtual int getSource() const;
    virtual void setSource(int source);
    virtual int getTerm() const;
    virtual void setTerm(int term);
    virtual bool getVoteGranted() const;
    virtual void setVoteGranted(bool voteGranted);
};

inline void doParsimPacking(omnetpp::cCommBuffer *b, const ServerReplyVoteMsg& obj) {obj.parsimPack(b);}
inline void doParsimUnpacking(omnetpp::cCommBuffer *b, ServerReplyVoteMsg& obj) {obj.parsimUnpack(b);}


#endif // ifndef __SERVERREPLYVOTEMSG_M_H

