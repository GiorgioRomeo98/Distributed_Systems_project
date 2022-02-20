//
// Generated file, do not edit! Created by nedtool 5.6 from appendEntriesMsg.msg.
//

// Disable warnings about unused variables, empty switch stmts, etc:
#ifdef _MSC_VER
#  pragma warning(disable:4101)
#  pragma warning(disable:4065)
#endif

#if defined(__clang__)
#  pragma clang diagnostic ignored "-Wshadow"
#  pragma clang diagnostic ignored "-Wconversion"
#  pragma clang diagnostic ignored "-Wunused-parameter"
#  pragma clang diagnostic ignored "-Wc++98-compat"
#  pragma clang diagnostic ignored "-Wunreachable-code-break"
#  pragma clang diagnostic ignored "-Wold-style-cast"
#elif defined(__GNUC__)
#  pragma GCC diagnostic ignored "-Wshadow"
#  pragma GCC diagnostic ignored "-Wconversion"
#  pragma GCC diagnostic ignored "-Wunused-parameter"
#  pragma GCC diagnostic ignored "-Wold-style-cast"
#  pragma GCC diagnostic ignored "-Wsuggest-attribute=noreturn"
#  pragma GCC diagnostic ignored "-Wfloat-conversion"
#endif

#include <iostream>
#include <sstream>
#include "appendEntriesMsg_m.h"

namespace omnetpp {

// Template pack/unpack rules. They are declared *after* a1l type-specific pack functions for multiple reasons.
// They are in the omnetpp namespace, to allow them to be found by argument-dependent lookup via the cCommBuffer argument

// Packing/unpacking an std::vector
template<typename T, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::vector<T,A>& v)
{
    int n = v.size();
    doParsimPacking(buffer, n);
    for (int i = 0; i < n; i++)
        doParsimPacking(buffer, v[i]);
}

template<typename T, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::vector<T,A>& v)
{
    int n;
    doParsimUnpacking(buffer, n);
    v.resize(n);
    for (int i = 0; i < n; i++)
        doParsimUnpacking(buffer, v[i]);
}

// Packing/unpacking an std::list
template<typename T, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::list<T,A>& l)
{
    doParsimPacking(buffer, (int)l.size());
    for (typename std::list<T,A>::const_iterator it = l.begin(); it != l.end(); ++it)
        doParsimPacking(buffer, (T&)*it);
}

template<typename T, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::list<T,A>& l)
{
    int n;
    doParsimUnpacking(buffer, n);
    for (int i=0; i<n; i++) {
        l.push_back(T());
        doParsimUnpacking(buffer, l.back());
    }
}

// Packing/unpacking an std::set
template<typename T, typename Tr, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::set<T,Tr,A>& s)
{
    doParsimPacking(buffer, (int)s.size());
    for (typename std::set<T,Tr,A>::const_iterator it = s.begin(); it != s.end(); ++it)
        doParsimPacking(buffer, *it);
}

template<typename T, typename Tr, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::set<T,Tr,A>& s)
{
    int n;
    doParsimUnpacking(buffer, n);
    for (int i=0; i<n; i++) {
        T x;
        doParsimUnpacking(buffer, x);
        s.insert(x);
    }
}

// Packing/unpacking an std::map
template<typename K, typename V, typename Tr, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::map<K,V,Tr,A>& m)
{
    doParsimPacking(buffer, (int)m.size());
    for (typename std::map<K,V,Tr,A>::const_iterator it = m.begin(); it != m.end(); ++it) {
        doParsimPacking(buffer, it->first);
        doParsimPacking(buffer, it->second);
    }
}

template<typename K, typename V, typename Tr, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::map<K,V,Tr,A>& m)
{
    int n;
    doParsimUnpacking(buffer, n);
    for (int i=0; i<n; i++) {
        K k; V v;
        doParsimUnpacking(buffer, k);
        doParsimUnpacking(buffer, v);
        m[k] = v;
    }
}

// Default pack/unpack function for arrays
template<typename T>
void doParsimArrayPacking(omnetpp::cCommBuffer *b, const T *t, int n)
{
    for (int i = 0; i < n; i++)
        doParsimPacking(b, t[i]);
}

template<typename T>
void doParsimArrayUnpacking(omnetpp::cCommBuffer *b, T *t, int n)
{
    for (int i = 0; i < n; i++)
        doParsimUnpacking(b, t[i]);
}

// Default rule to prevent compiler from choosing base class' doParsimPacking() function
template<typename T>
void doParsimPacking(omnetpp::cCommBuffer *, const T& t)
{
    throw omnetpp::cRuntimeError("Parsim error: No doParsimPacking() function for type %s", omnetpp::opp_typename(typeid(t)));
}

template<typename T>
void doParsimUnpacking(omnetpp::cCommBuffer *, T& t)
{
    throw omnetpp::cRuntimeError("Parsim error: No doParsimUnpacking() function for type %s", omnetpp::opp_typename(typeid(t)));
}

}  // namespace omnetpp


// forward
template<typename T, typename A>
std::ostream& operator<<(std::ostream& out, const std::vector<T,A>& vec);

// Template rule which fires if a struct or class doesn't have operator<<
template<typename T>
inline std::ostream& operator<<(std::ostream& out,const T&) {return out;}

// operator<< for std::vector<T>
template<typename T, typename A>
inline std::ostream& operator<<(std::ostream& out, const std::vector<T,A>& vec)
{
    out.put('{');
    for(typename std::vector<T,A>::const_iterator it = vec.begin(); it != vec.end(); ++it)
    {
        if (it != vec.begin()) {
            out.put(','); out.put(' ');
        }
        out << *it;
    }
    out.put('}');
    
    char buf[32];
    sprintf(buf, " (size=%u)", (unsigned int)vec.size());
    out.write(buf, strlen(buf));
    return out;
}

Register_Class(AppendEntriesMsg)

AppendEntriesMsg::AppendEntriesMsg(const char *name, short kind) : ::omnetpp::cMessage(name,kind)
{
    this->term = 0;
    this->leaderId = 0;
    this->prevLogIndex = 0;
    this->prevLogTerm = 0;
    entries_arraysize = 0;
    this->entries = 0;
    this->leaderCommit = 0;
}

AppendEntriesMsg::AppendEntriesMsg(const AppendEntriesMsg& other) : ::omnetpp::cMessage(other)
{
    entries_arraysize = 0;
    this->entries = 0;
    copy(other);
}

AppendEntriesMsg::~AppendEntriesMsg()
{
    delete [] this->entries;
}

AppendEntriesMsg& AppendEntriesMsg::operator=(const AppendEntriesMsg& other)
{
    if (this==&other) return *this;
    ::omnetpp::cMessage::operator=(other);
    copy(other);
    return *this;
}

void AppendEntriesMsg::copy(const AppendEntriesMsg& other)
{
    this->term = other.term;
    this->leaderId = other.leaderId;
    this->prevLogIndex = other.prevLogIndex;
    this->prevLogTerm = other.prevLogTerm;
    delete [] this->entries;
    this->entries = (other.entries_arraysize==0) ? nullptr : new logList[other.entries_arraysize];
    entries_arraysize = other.entries_arraysize;
    for (unsigned int i=0; i<entries_arraysize; i++)
        this->entries[i] = other.entries[i];
    this->leaderCommit = other.leaderCommit;
}

void AppendEntriesMsg::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::omnetpp::cMessage::parsimPack(b);
    doParsimPacking(b,this->term);
    doParsimPacking(b,this->leaderId);
    doParsimPacking(b,this->prevLogIndex);
    doParsimPacking(b,this->prevLogTerm);
    b->pack(entries_arraysize);
    doParsimArrayPacking(b,this->entries,entries_arraysize);
    doParsimPacking(b,this->leaderCommit);
}

void AppendEntriesMsg::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::omnetpp::cMessage::parsimUnpack(b);
    doParsimUnpacking(b,this->term);
    doParsimUnpacking(b,this->leaderId);
    doParsimUnpacking(b,this->prevLogIndex);
    doParsimUnpacking(b,this->prevLogTerm);
    delete [] this->entries;
    b->unpack(entries_arraysize);
    if (entries_arraysize==0) {
        this->entries = 0;
    } else {
        this->entries = new logList[entries_arraysize];
        doParsimArrayUnpacking(b,this->entries,entries_arraysize);
    }
    doParsimUnpacking(b,this->leaderCommit);
}

int AppendEntriesMsg::getTerm() const
{
    return this->term;
}

void AppendEntriesMsg::setTerm(int term)
{
    this->term = term;
}

int AppendEntriesMsg::getLeaderId() const
{
    return this->leaderId;
}

void AppendEntriesMsg::setLeaderId(int leaderId)
{
    this->leaderId = leaderId;
}

int AppendEntriesMsg::getPrevLogIndex() const
{
    return this->prevLogIndex;
}

void AppendEntriesMsg::setPrevLogIndex(int prevLogIndex)
{
    this->prevLogIndex = prevLogIndex;
}

int AppendEntriesMsg::getPrevLogTerm() const
{
    return this->prevLogTerm;
}

void AppendEntriesMsg::setPrevLogTerm(int prevLogTerm)
{
    this->prevLogTerm = prevLogTerm;
}

void AppendEntriesMsg::setEntriesArraySize(unsigned int size)
{
    logList *entries2 = (size==0) ? nullptr : new logList[size];
    unsigned int sz = entries_arraysize < size ? entries_arraysize : size;
    for (unsigned int i=0; i<sz; i++)
        entries2[i] = this->entries[i];
    entries_arraysize = size;
    delete [] this->entries;
    this->entries = entries2;
}

unsigned int AppendEntriesMsg::getEntriesArraySize() const
{
    return entries_arraysize;
}

logList& AppendEntriesMsg::getEntries(unsigned int k)
{
    if (k>=entries_arraysize) throw omnetpp::cRuntimeError("Array of size %d indexed by %d", entries_arraysize, k);
    return this->entries[k];
}

void AppendEntriesMsg::setEntries(unsigned int k, const logList& entries)
{
    if (k>=entries_arraysize) throw omnetpp::cRuntimeError("Array of size %d indexed by %d", entries_arraysize, k);
    this->entries[k] = entries;
}

int AppendEntriesMsg::getLeaderCommit() const
{
    return this->leaderCommit;
}

void AppendEntriesMsg::setLeaderCommit(int leaderCommit)
{
    this->leaderCommit = leaderCommit;
}

class AppendEntriesMsgDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertynames;
  public:
    AppendEntriesMsgDescriptor();
    virtual ~AppendEntriesMsgDescriptor();

    virtual bool doesSupport(omnetpp::cObject *obj) const override;
    virtual const char **getPropertyNames() const override;
    virtual const char *getProperty(const char *propertyname) const override;
    virtual int getFieldCount() const override;
    virtual const char *getFieldName(int field) const override;
    virtual int findField(const char *fieldName) const override;
    virtual unsigned int getFieldTypeFlags(int field) const override;
    virtual const char *getFieldTypeString(int field) const override;
    virtual const char **getFieldPropertyNames(int field) const override;
    virtual const char *getFieldProperty(int field, const char *propertyname) const override;
    virtual int getFieldArraySize(void *object, int field) const override;

    virtual const char *getFieldDynamicTypeString(void *object, int field, int i) const override;
    virtual std::string getFieldValueAsString(void *object, int field, int i) const override;
    virtual bool setFieldValueAsString(void *object, int field, int i, const char *value) const override;

    virtual const char *getFieldStructName(int field) const override;
    virtual void *getFieldStructValuePointer(void *object, int field, int i) const override;
};

Register_ClassDescriptor(AppendEntriesMsgDescriptor)

AppendEntriesMsgDescriptor::AppendEntriesMsgDescriptor() : omnetpp::cClassDescriptor("AppendEntriesMsg", "omnetpp::cMessage")
{
    propertynames = nullptr;
}

AppendEntriesMsgDescriptor::~AppendEntriesMsgDescriptor()
{
    delete[] propertynames;
}

bool AppendEntriesMsgDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<AppendEntriesMsg *>(obj)!=nullptr;
}

const char **AppendEntriesMsgDescriptor::getPropertyNames() const
{
    if (!propertynames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
        const char **basenames = basedesc ? basedesc->getPropertyNames() : nullptr;
        propertynames = mergeLists(basenames, names);
    }
    return propertynames;
}

const char *AppendEntriesMsgDescriptor::getProperty(const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : nullptr;
}

int AppendEntriesMsgDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 6+basedesc->getFieldCount() : 6;
}

unsigned int AppendEntriesMsgDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeFlags(field);
        field -= basedesc->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISARRAY | FD_ISCOMPOUND,
        FD_ISEDITABLE,
    };
    return (field>=0 && field<6) ? fieldTypeFlags[field] : 0;
}

const char *AppendEntriesMsgDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldName(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldNames[] = {
        "term",
        "leaderId",
        "prevLogIndex",
        "prevLogTerm",
        "entries",
        "leaderCommit",
    };
    return (field>=0 && field<6) ? fieldNames[field] : nullptr;
}

int AppendEntriesMsgDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount() : 0;
    if (fieldName[0]=='t' && strcmp(fieldName, "term")==0) return base+0;
    if (fieldName[0]=='l' && strcmp(fieldName, "leaderId")==0) return base+1;
    if (fieldName[0]=='p' && strcmp(fieldName, "prevLogIndex")==0) return base+2;
    if (fieldName[0]=='p' && strcmp(fieldName, "prevLogTerm")==0) return base+3;
    if (fieldName[0]=='e' && strcmp(fieldName, "entries")==0) return base+4;
    if (fieldName[0]=='l' && strcmp(fieldName, "leaderCommit")==0) return base+5;
    return basedesc ? basedesc->findField(fieldName) : -1;
}

const char *AppendEntriesMsgDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeString(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "int",
        "int",
        "int",
        "int",
        "logList",
        "int",
    };
    return (field>=0 && field<6) ? fieldTypeStrings[field] : nullptr;
}

const char **AppendEntriesMsgDescriptor::getFieldPropertyNames(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldPropertyNames(field);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

const char *AppendEntriesMsgDescriptor::getFieldProperty(int field, const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldProperty(field, propertyname);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

int AppendEntriesMsgDescriptor::getFieldArraySize(void *object, int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldArraySize(object, field);
        field -= basedesc->getFieldCount();
    }
    AppendEntriesMsg *pp = (AppendEntriesMsg *)object; (void)pp;
    switch (field) {
        case 4: return pp->getEntriesArraySize();
        default: return 0;
    }
}

const char *AppendEntriesMsgDescriptor::getFieldDynamicTypeString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldDynamicTypeString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    AppendEntriesMsg *pp = (AppendEntriesMsg *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string AppendEntriesMsgDescriptor::getFieldValueAsString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldValueAsString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    AppendEntriesMsg *pp = (AppendEntriesMsg *)object; (void)pp;
    switch (field) {
        case 0: return long2string(pp->getTerm());
        case 1: return long2string(pp->getLeaderId());
        case 2: return long2string(pp->getPrevLogIndex());
        case 3: return long2string(pp->getPrevLogTerm());
        case 4: {std::stringstream out; out << pp->getEntries(i); return out.str();}
        case 5: return long2string(pp->getLeaderCommit());
        default: return "";
    }
}

bool AppendEntriesMsgDescriptor::setFieldValueAsString(void *object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->setFieldValueAsString(object,field,i,value);
        field -= basedesc->getFieldCount();
    }
    AppendEntriesMsg *pp = (AppendEntriesMsg *)object; (void)pp;
    switch (field) {
        case 0: pp->setTerm(string2long(value)); return true;
        case 1: pp->setLeaderId(string2long(value)); return true;
        case 2: pp->setPrevLogIndex(string2long(value)); return true;
        case 3: pp->setPrevLogTerm(string2long(value)); return true;
        case 5: pp->setLeaderCommit(string2long(value)); return true;
        default: return false;
    }
}

const char *AppendEntriesMsgDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructName(field);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        case 4: return omnetpp::opp_typename(typeid(logList));
        default: return nullptr;
    };
}

void *AppendEntriesMsgDescriptor::getFieldStructValuePointer(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructValuePointer(object, field, i);
        field -= basedesc->getFieldCount();
    }
    AppendEntriesMsg *pp = (AppendEntriesMsg *)object; (void)pp;
    switch (field) {
        case 4: return (void *)(&pp->getEntries(i)); break;
        default: return nullptr;
    }
}


