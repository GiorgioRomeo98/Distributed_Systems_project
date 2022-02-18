//
// Generated file, do not edit! Created by nedtool 5.6 from clientRequest.msg.
//

#ifndef __CLIENTREQUEST_M_H
#define __CLIENTREQUEST_M_H

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
typedef std::list<command_t> commandList;
// }}

/**
 * Class generated from <tt>clientRequest.msg:24</tt> by nedtool.
 * <pre>
 * message ClientRequest
 * {
 *     int source_addr;
 *     int destination_addr;
 *     commandList commands;
 * }
 * </pre>
 */
class ClientRequest : public ::omnetpp::cMessage
{
  protected:
    int source_addr;
    int destination_addr;
    commandList commands;

  private:
    void copy(const ClientRequest& other);

  protected:
    // protected and unimplemented operator==(), to prevent accidental usage
    bool operator==(const ClientRequest&);

  public:
    ClientRequest(const char *name=nullptr, short kind=0);
    ClientRequest(const ClientRequest& other);
    virtual ~ClientRequest();
    ClientRequest& operator=(const ClientRequest& other);
    virtual ClientRequest *dup() const override {return new ClientRequest(*this);}
    virtual void parsimPack(omnetpp::cCommBuffer *b) const override;
    virtual void parsimUnpack(omnetpp::cCommBuffer *b) override;

    // field getter/setter methods
    virtual int getSource_addr() const;
    virtual void setSource_addr(int source_addr);
    virtual int getDestination_addr() const;
    virtual void setDestination_addr(int destination_addr);
    virtual commandList& getCommands();
    virtual const commandList& getCommands() const {return const_cast<ClientRequest*>(this)->getCommands();}
    virtual void setCommands(const commandList& commands);
};

inline void doParsimPacking(omnetpp::cCommBuffer *b, const ClientRequest& obj) {obj.parsimPack(b);}
inline void doParsimUnpacking(omnetpp::cCommBuffer *b, ClientRequest& obj) {obj.parsimUnpack(b);}


#endif // ifndef __CLIENTREQUEST_M_H
