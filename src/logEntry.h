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
    int index;              // index of the log entry
    int term;               // term of the log entry
    Command entryCommand;   // command of the log entry

    _logEntry(): index(0), term(), entryCommand() {} // default constructor
    _logEntry(int _index, int _term, Command _entryCommand):
             index(_index), term(_term), entryCommand(_entryCommand.var, _entryCommand.value) {}

}LogEntry;

inline std::ostream& operator<<(std::ostream& os, const LogEntry& log_elem)
{
    os << "index=" << log_elem.index << "; term=" << log_elem.term << "; command: "
       << log_elem.entryCommand.var << " <-- " << log_elem.entryCommand.value; // no endl!
    return os;
}

#endif /* LOGENTRY_H_ */
