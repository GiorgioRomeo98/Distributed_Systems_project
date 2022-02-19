/*
 * log_entry.h
 *
 *  Created on: 18 Feb 2022
 *      Author: Giorgio Romeo, Daria Preda
 */

#include "command.h"

typedef struct _log_entry
{
    int term;
    Command entry_command;

    _log_entry(int _term, char _var, int _value): term(_term), entry_command(_var, _value) {}

}Log_entry;

std::ostream& operator<<(std::ostream& os, const Log_entry& log_elem)
{
    os << "term=" << log_elem.term << "; command: " << log_elem.entry_command.var << " <-- " << log_elem.entry_command.value; // no endl!
    return os;
}
