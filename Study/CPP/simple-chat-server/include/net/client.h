#ifndef __CLIENT_H__
#define __CLIENT_H__

#include "common.h"

class std::string;
class robi_net::Socket;

namespace robi_net {

class Client
{
private:
    robi_net::Socket *cli_socket;
    std::string *user_name_;
    int64 last_sync_time;
    // ...
public:
    Client();
    ~Client();
};}

#endif