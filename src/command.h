/*
 * command.h
 *
 *  Created on: 18 Feb 2022
 *      Author: Giorgio Romeo, Daria Preda
 *
 */

typedef struct command_t{
            char var;
            int value;
            command_t(char _var, int _value): var(_var), value(_value) {}
} command;
