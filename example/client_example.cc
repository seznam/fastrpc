#include <cstdio>
#include <string>
#include <cstdlib>
#include <frpc.h>
#include <frpcserverproxy.h>

#include <frpchttperror.h>
#include <frpcfault.h>
#include <frpcstreamerror.h>

using namespace FRPC;

int main(int argc, char *argv[]) {
    Pool_t pool; // memory pool
    ServerProxy_t::Config_t config;

    std::string url;
    if(argc > 1 && atoi(argv[1])){
        url = "http://localhost:";url +=  argv[1];url +="/RPC2";
    } else {
        url = "http://localhost:2424/RPC2";
    }

    ServerProxy_t client(url.c_str(), config);

    try  {
        // Calling introspection method system.listMethods()
        Value_t &retVal1 = client(pool, "test1",pool.Int(-424343202538277637),pool.String("c++"));
        printf("Output from list method is: ");
        printValue(retVal1);

        std::string str;
        dumpFastrpcTree(retVal1, str,3);
        // format for logs
        printf("\nOther format from listMethod is:%s\n", str.c_str());


        Struct_t &result = Struct(client(pool, "test2", pool.Int(2),
                                        pool.String("hello")));

        printf("\nresult[\"status\"]=%li\n", Int(result["status"]).getValue());
        printf("\nresult[\"struct\"][\"int\"]=%li\n", Int(result["struct"]["int"]).getValue());
        printf("\nresult[\"array\"][2]=%s\n", String(result["array"][2]).getValue().c_str());
	printf("\nOutput from test2 method is: ");
	printValue(result);
					
	
					
        printf("\nOutput from test1 method is: ");
        printValue(client(pool, "test1", pool.Struct("now",pool.LocalTime(),"id",pool.Int(22),"Status",pool.String("OK")),pool.Double(2.587),
                                        pool.String("hello")));
    } catch (const Fault_t &f) {
        printf("Fault Status:%d Message:%s \n", f.errorNum(),
               f.message().c_str() );
    } catch (const HTTPError_t &e) {
        printf("Http Error Status:%d Message:%s \n", e.errorNum(),
               e.message().c_str() );
    } catch (const StreamError_t &e) {
        printf("Stream Error  Message:%s \n", e.message().c_str());
    }

    return EXIT_SUCCESS;
}

