//
// Generated file, do not edit! Created by nedtool 5.6 from clientRequestMsg.msg.
//

#ifndef __CLIENTREQUESTMSG_M_H
#define __CLIENTREQUESTMSG_M_H

#if defined(__clang__)
#  pragma clang diagnostic ignored "-Wreserved-id-macro"
#endif
#include <omnetpp.h>

// nedtool version check
#define MSGC_VERSION 0x0506
#if (MSGC_VERSION!=OMNETPP_VERSION)
#    error Version mismatch! Probably this file was generated by an earlier version of nedtool: 'make clean' should help.
#endif



// cplusplus {{
#include "command.h"
typedef _command Cmd;
// }}

/**
 * Class generated from <tt>clientRequestMsg.msg:30</tt> by nedtool.
 * <pre>
 * message ClientRequestMsg
 * {
 *     int sourceAddr;		// address of the client that sent the request
 *     int destAddr;		// address of the server
 *     int serialNumber;	// to uniquely identify a client request between all the ones sent by the same client
 *     Cmd command;		// command to process
 * }
 * </pre>
 */
class ClientRequestMsg : public ::omnetpp::cMessage
{
  protected:
    int sourceAddr;
    int destAddr;
    int serialNumber;
    Cmd command;

  private:
    void copy(const ClientRequestMsg& other);

  protected:
    // protected and unimplemented operator==(), to prevent accidental usage
    bool operator==(const ClientRequestMsg&);

  public:
    ClientRequestMsg(const char *name=nullptr, short kind=0);
    ClientRequestMsg(const ClientRequestMsg& other);
    virtual ~ClientRequestMsg();
    ClientRequestMsg& operator=(const ClientRequestMsg& other);
    virtual ClientRequestMsg *dup() const override {return new ClientRequestMsg(*this);}
    virtual void parsimPack(omnetpp::cCommBuffer *b) const override;
    virtual void parsimUnpack(omnetpp::cCommBuffer *b) override;

    // field getter/setter methods
    virtual int getSourceAddr() const;
    virtual void setSourceAddr(int sourceAddr);
    virtual int getDestAddr() const;
    virtual void setDestAddr(int destAddr);
    virtual int getSerialNumber() const;
    virtual void setSerialNumber(int serialNumber);
    virtual Cmd& getCommand();
    virtual const Cmd& getCommand() const {return const_cast<ClientRequestMsg*>(this)->getCommand();}
    virtual void setCommand(const Cmd& command);
};

inline void doParsimPacking(omnetpp::cCommBuffer *b, const ClientRequestMsg& obj) {obj.parsimPack(b);}
inline void doParsimUnpacking(omnetpp::cCommBuffer *b, ClientRequestMsg& obj) {obj.parsimUnpack(b);}


#endif // ifndef __CLIENTREQUESTMSG_M_H

