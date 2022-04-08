//
// Created by Andrej Hyros on 08/04/2022.
//

#include "communicant.h"



void communicant::*get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in *) sa)->sin_addr);
    }
    return &(((struct sockaddr_in6 *) sa)->sin6_addr);
}