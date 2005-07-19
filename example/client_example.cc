/***************************************************************************
 *   Copyright (C) 2004 by Miroslav Talasek                                *
 ***************************************************************************/
#define __ENABLE_WSTRING
#include <stdio.h>
#include <string>
#include <frpc.h>
#include <frpcserverproxy.h>
#include <frpchttperror.h>
#include <frpcfault.h>
#include <frpcstreamerror.h>






using namespace FRPC;

int main(int argc, char *argv[])
{
    Pool_t pool; //memory pool
    ServerProxy_t::Config_t config;
    ServerProxy_t client("http://localhost:2424/RPC2",config);

    try
    {
        //Calling introspection method system.listMethods()
        Value_t &retVal1 = client(pool,"system.listMethods");
        //Human readable format :) (only to stdout)
        printf("retVal1 is: ");
        printValue(retVal1);

        std::string str;
        dumpFastrpcTree(retVal1,str,3);
        //format for logs
        printf("\nretVal1 is:%s\n",str.c_str());


        printValue(client(pool,"system.methodSignature",
                          pool.String(String(Array(retVal1)[4]).getString())));

        Struct_t result = Struct(client(pool,"test1",pool.Int(2),pool.String("hello")));

        printValue(result);
        printf("\nStatus is %ld\n",Int(result["status"]).getValue());



    }
    catch (const Fault_t &f)
    {
        printf("Fault Status:%ld Message:%s \n",f.errorNum(),f.message().c_str() );
    }
    catch (const HTTPError_t &e)
    {
        printf("Http Error Status:%ld Message:%s \n",e.errorNum(),e.message().c_str() );
    }
    catch (const StreamError_t &e)
    {
        printf("Stream Error  Message:%s \n",e.message().c_str() );
    }


    return EXIT_SUCCESS;
}
