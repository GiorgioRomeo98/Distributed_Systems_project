/*
 * serverClientRequestInfo.h
 *
 *  Created on: 25 Feb 2022
 *      Author: Giorgio Romeo, Daria Preda
 */

#ifndef SERVERCLIENTREQUESTINFO_H_
#define SERVERCLIENTREQUESTINFO_H_


typedef struct _serverClientRequestInfo{
            int index;              // index of the command stored in the Leader logEntries (used when Leader applies the command to the state machine and
                                    // need to respond to the client that sent originally the request)
            int serialNumber;       // serial number of the most recent client request the Leader computed the result of
            int result;             // represent the result of the command execution
            _serverClientRequestInfo(): index(-1), serialNumber(-1), result() {} // default constructor
            _serverClientRequestInfo(int _index, int _serialNumber, int _result): index(_index), serialNumber(_serialNumber), result(_result) {}
} ServerClientRequestInfo;


#endif /* SERVERCLIENTREQUESTINFO_H_ */
