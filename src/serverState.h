/*
 * serverState.h
 *
 *  Created on: 19 Feb 2022
 *      Author: Giorgio Romeo, Daria Preda
 */

#ifndef SERVERSTATE_H_
#define SERVERSTATE_H_

enum serverState {
    FAILED = -1,
    FOLLOWER = 0,
    CANDIDATE = 1,
    LEADER = 2
};



#endif /* SERVERSTATE_H_ */
