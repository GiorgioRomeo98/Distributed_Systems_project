/*
 * logEntry.h
 *
 *  Created on: 18 Feb 2022
 *      Author: Giorgio Romeo, Daria Preda
 */

#ifndef LOGENTRY_H_
#define LOGENTRY_H_


#include "command.h"

typedef struct _logEntry
{
    int term;
    Command entryCommand;

    _logEntry(): term(), entryCommand() {} // default constructor
    _logEntry(int _term, char _var, int _value): term(_term), entryCommand(_var, _value) {}

}LogEntry;

inline std::ostream& operator<<(std::ostream& os, const LogEntry& log_elem)
{
    os << "term=" << log_elem.term << "; command: " << log_elem.entryCommand.var << " <-- " << log_elem.entryCommand.value; // no endl!
    return os;
}

#endif /* LOGENTRY_H_ */
