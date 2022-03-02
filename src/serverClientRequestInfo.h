/*
 * serverClientRequestInfo.h
 *
 *  Created on: 25 Feb 2022
 *      Author: Giorgio Romeo, Daria Preda
 */

#ifndef SERVERCLIENTREQUESTINFO_H_
#define SERVERCLIENTREQUESTINFO_H_


typedef struct _serverClientRequestInfo{
            int serialNumber;       // serial number of the most recent client request the Leader computed the result of
            int result;             // represent the result of the command execution
            _serverClientRequestInfo(): serialNumber(-1), result() {} // default constructor
            _serverClientRequestInfo(int _serialNumber, int _result): serialNumber(_serialNumber), result(_result) {}
} ServerClientRequestInfo;


#endif /* SERVERCLIENTREQUESTINFO_H_ */
