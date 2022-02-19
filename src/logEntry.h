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
    Command entry_command;

    _logEntry(int _term, char _var, int _value): term(_term), entry_command(_var, _value) {}

}LogEntry;

std::ostream& operator<<(std::ostream& os, const LogEntry& log_elem)
{
    os << "term=" << log_elem.term << "; command: " << log_elem.entry_command.var << " <-- " << log_elem.entry_command.value; // no endl!
    return os;
}

#endif /* LOGENTRY_H_ */
