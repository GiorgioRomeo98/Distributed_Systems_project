/*
 * command.h
 *
 *  Created on: 18 Feb 2022
 *      Author: Giorgio Romeo, Daria Preda
 *
 */

#ifndef COMMAND_H_
#define COMMAND_H_

typedef struct _command{
            char var;
            int value;
            _command(): var(), value() {} // default constructor
            _command(char _var, int _value): var(_var), value(_value) {}
} Command;

inline std::ostream& operator<<(std::ostream& os, const Command& cmd)
{
    os << cmd.var << " <-- " << cmd.value; // no endl!
    return os;
}


#endif /* COMMAND_H_ */
