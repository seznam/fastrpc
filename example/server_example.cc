#include <netinet/tcp.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <sys/select.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/fcntl.h>

#include <string.h>
#include <errno.h>

#include <stdio.h>
#include <string>
#include <frpc.h>
#include <frpcmethodregistry.h>

#include <frpcheadmethod.h>
#include <frpcfault.h>
#include <frpcserver.h>

using namespace FRPC;

// this example contains two server methods first is in the class and second
// is global

class Test_t {
public:
    Value_t& methodTest(Pool_t &pool, Array_t &params) {
        // this method repicate input to output

        return params;
    }

    bool head() {
        //  HTTP head method impplementation
        return true;
    }
};

Value_t& methodTest(Pool_t &pool, Array_t &params, int &data) {
    // automatic integrity check (only first level)
    params.checkItems("is");

    return pool.Struct("status", pool.Int(200), "statusMessage",
                       pool.String("OK"), "int", params[0], "string",
                       params[1]);
}


class Callbacks_t : public MethodRegistry_t::Callbacks_t {
    virtual void preRead() {
      //usable in keepalive connection
    }

    virtual void preProcess(const std::string &methodName,
                            const std::string &clientIP,
                            Array_t &params)
    {
        std::string str;
        dumpFastrpcTree(params, str, 2);
        printf("Calling method: %s %s from IP:%s\n",
               methodName.c_str(), str.c_str(), clientIP.c_str());
    }

    virtual void postProcess(const std::string &methodName,
                             const std::string &clientIP,
                             const Array_t &params,
                             const Value_t &result,
                             const MethodRegistry_t::TimeDiff_t &time)
    {
        std::string str;
        dumpFastrpcTree(result, str, 2);

        printf("Method: %s return %s. Execution time %ld sec "
               "and %ld microsec\n", methodName.c_str(), str.c_str(),
               time.second, time.usecond);
    }

    virtual void postProcess(const std::string &methodName,
                             const std::string &clientIP,
                             const Array_t &params,
                             const Fault_t &fault,
                             const MethodRegistry_t::TimeDiff_t &time)
    {
        printf("Method: %s return fault (status:%d message:%s)"
               "Execution time %ld sec and %ld microsec\n",
               methodName.c_str(),fault.errorNum(),fault.message().c_str(),
               time.second, time.usecond);
    }
};

int main(int argc, char *argv[]) {
    Test_t test;
    Callbacks_t callbacks;
    MethodRegistry_t methodRegistry(&callbacks, true);
    Server_t::Config_t config(10000, 10000, false, 0, true, &callbacks);
    Server_t server(config);
    int a=2;

    // register methods
    server.registry().registerMethod("test1",
                                     boundMethod(&Test_t::methodTest, test),
                                     "*:*",
                                     "Method in the class for test");
    server.registry().registerMethod("test2",
                                     unboundMethod(methodTest, a),
                                     "S:is",
                                     "Global method");
    // register head method FOR HTTP HEAD
    server.registry().registerHeadMethod(boundHeadMethod(&Test_t::head, test));

    int sock = 0;
    struct sockaddr_in addr;
    long port= 2424;
    socklen_t sinSize = sizeof(struct sockaddr_in);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    sock = socket(AF_INET, SOCK_STREAM, 0);

    // re-use already bound address/port (if possible)
    int optval = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int)) < 0)
        printf("Cannot set SO_REUSEADDR option "
               "on listen socket (%s)\n", strerror(errno));
    // set TCP_NODELAY for sure
    optval = 1;
    if (setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(int)) < 0)
        printf("Cannot set TCP_NODELAY option "
               "on listen socket (%s)\n", strerror(errno));

    bind(sock,(struct sockaddr*) &addr, sinSize);
    listen(sock, 10);
    printf("Listening on any interfaces on port %ld\n", port);

    // single thread server logic
    struct sockaddr_in clientAddr;
    int fdNew;

    // handle requests
    for(;;) {
        fdNew = accept(sock, (struct sockaddr*) &clientAddr, &sinSize);
        if (fdNew < 0) {
            printf("Cannot accept connection(%s)\n", strerror(errno));
        }
        server.serve(fdNew);
        close(fdNew);
    }

    return EXIT_SUCCESS;
}

