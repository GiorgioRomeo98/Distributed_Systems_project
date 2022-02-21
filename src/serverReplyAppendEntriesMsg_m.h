//
// Generated file, do not edit! Created by nedtool 5.6 from serverReplyAppendEntriesMsg.msg.
//

#ifndef __SERVERREPLYAPPENDENTRIESMSG_M_H
#define __SERVERREPLYAPPENDENTRIESMSG_M_H

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
 * Class generated from <tt>serverReplyAppendEntriesMsg.msg:16</tt> by nedtool.
 * <pre>
 * message ServerReplyAppendEntriesMsg
 * {
 *     int term; 		//currentTerm, for leader to update itself
 *     bool success; 	//true if follower contained entry matching	prevLogIndex and prevLogTerm
 * }
 * </pre>
 */
class ServerReplyAppendEntriesMsg : public ::omnetpp::cMessage
{
  protected:
    int term;
    bool success;

  private:
    void copy(const ServerReplyAppendEntriesMsg& other);

  protected:
    // protected and unimplemented operator==(), to prevent accidental usage
    bool operator==(const ServerReplyAppendEntriesMsg&);

  public:
    ServerReplyAppendEntriesMsg(const char *name=nullptr, short kind=0);
    ServerReplyAppendEntriesMsg(const ServerReplyAppendEntriesMsg& other);
    virtual ~ServerReplyAppendEntriesMsg();
    ServerReplyAppendEntriesMsg& operator=(const ServerReplyAppendEntriesMsg& other);
    virtual ServerReplyAppendEntriesMsg *dup() const override {return new ServerReplyAppendEntriesMsg(*this);}
    virtual void parsimPack(omnetpp::cCommBuffer *b) const override;
    virtual void parsimUnpack(omnetpp::cCommBuffer *b) override;

    // field getter/setter methods
    virtual int getTerm() const;
    virtual void setTerm(int term);
    virtual bool getSuccess() const;
    virtual void setSuccess(bool success);
};

inline void doParsimPacking(omnetpp::cCommBuffer *b, const ServerReplyAppendEntriesMsg& obj) {obj.parsimPack(b);}
inline void doParsimUnpacking(omnetpp::cCommBuffer *b, ServerReplyAppendEntriesMsg& obj) {obj.parsimUnpack(b);}


#endif // ifndef __SERVERREPLYAPPENDENTRIESMSG_M_H

