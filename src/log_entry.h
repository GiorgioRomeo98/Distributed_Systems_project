/*
 * log_entry.h
 *
 *  Created on: 18 Feb 2022
 *      Author: Giorgio Romeo, Daria Preda
 */

struct log_entry
{
    int term;
    struct command_t{
            char var;
            int value;
            command_t(char _var, int _value): var(_var), value(_value) {}
    } command;

    log_entry(int _term, char _var, int _value): term(_term), command(_var, _value) {}

};

std::ostream& operator<<(std::ostream& os, const log_entry& log_elem)
{
    os << "term=" << log_elem.term << "; command: " << log_elem.command.var << " <-- " << log_elem.command.value; // no endl!
    return os;
}
