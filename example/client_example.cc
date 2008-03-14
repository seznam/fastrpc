#include <stdio.h>
#include <string>
#include <frpc.h>
#include <frpcserverproxy.h>

#include <frpchttperror.h>
#include <frpcfault.h>
#include <frpcstreamerror.h>

using namespace FRPC;

int main(int argc, char *argv[]) {
    Pool_t pool; // memory pool
    ServerProxy_t::Config_t config;
    ServerProxy_t client("http://localhost:2424/RPC2", config);

    try  {
        // Calling introspection method system.listMethods()
        Value_t &retVal1 = client(pool, "system.listMethods");
        printf("Output from list method is: ");
        printValue(retVal1);

        std::string str;
        dumpFastrpcTree(retVal1, str,3);
        // format for logs
        printf("\nOther format from listMethod is:%s\n", str.c_str());


        Struct_t &result = Struct(client(pool, "test2", pool.Int(2),
                                        pool.String("hello")));

        printf("\nresult[\"status\"]=%i\n", Int(result["status"]).getValue());
	printf("\nOutput from test2 method is: ");
	printValue(result);
					
	
					
        printf("\nOutput from test1 method is: ");
        printValue(client(pool, "test1", pool.Struct("now",pool.DateTime(),"id",pool.Int(22),"Status",pool.String("OK")),pool.Double(2.587),
                                        pool.String("hello")));
    } catch (const Fault_t &f) {
        printf("Fault Status:%ld Message:%s \n", f.errorNum(),
               f.message().c_str() );
    } catch (const HTTPError_t &e) {
        printf("Http Error Status:%ld Message:%s \n", e.errorNum(),
               e.message().c_str() );
    } catch (const StreamError_t &e) {
        printf("Stream Error  Message:%s \n", e.message().c_str());
    }

    return EXIT_SUCCESS;
}

