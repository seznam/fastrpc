"""
FastRPC handler for Tornado server. It takes care of getting data from POST
request, converts it from XML-RPC or FastRPC into Python type, calls corresponding
method and finally converts Python type back into XML-RPC or FastRPC.

Author Jan Seifert.
"""
import inspect
import time

import fastrpc
from shelter.core.web import BaseRequestHandler


class FastRpcHandler(BaseRequestHandler):
    supported_ct = set(['text/xml', 'application/x-frpc'])

    def post(self):
        # Kontrola vstupniho content type
        request_ct = self.request.headers['Content-Type']
        if request_ct not in self.supported_ct:
            self.set_status(400)
            return

        # Zpracovani vystupniho content type
        accept_ct = self.request.headers.get('Accept')
        if accept_ct:
            accept_ct_set = set([ct.strip() for ct in accept_ct.split(',')])
        else:
            # Hlavicka Accept neni, pouziju content type z requestu
            accept_ct_set = set([request_ct])
        possible_response_cts = self.supported_ct & accept_ct_set
        # Kontrola pruniku podporovanych a vystupnich content type
        if not possible_response_cts:
            self.set_status(406)
            return

        # Vyhledani a zavolani funkce
        args, method_name = fastrpc.loads(self.request.body)
        method_func = self.context.rpc_methods[method_name]
        try:
            response = (method_func(self.context, *args),)
        except Exception as e:
            self.logger.exception("Call '%s() error'" % method_name)
            msg = "%s: %s" % (e.__class__.__name__, e)
            response = fastrpc.Fault(-500, msg)

        # Vyhledani nejlepsiho content type pro odpoved + serializace dat
        if 'application/x-frpc' in possible_response_cts:
            response_data = fastrpc.dumps(
                response, methodresponse=True, useBinary=True)
            response_ct = 'application/x-frpc'
        elif 'text/xml' in possible_response_cts:
            response_data = fastrpc.dumps(
                response, methodresponse=True, useBinary=False)
            response_ct = 'text/xml'

        # Hlavicky odpovedi
        self.set_header("Content-Type", response_ct)
        self.set_header("Content-Length", len(response_data))
        self.set_header("Server", self.request.host)
        self.set_header("Date", time.strftime("%a, %d %b %Y %H:%M:%S +0000",
                                              time.gmtime()))
        self.set_header("Accept", ",".join(self.supported_ct))
        # Telo odpovedi
        self.write(response_data)


def system_list_methods(context):
    """
    Return an array of all available RPC methods on this server.
    """
    return list(context.rpc_methods.keys())
system_list_methods.signature = 'S:'


def system_method_signature(context, name):
    """
    Given the name of a method, return an array of legal signatures.
    Each signature is an array of strings. The first item of each
    signature is the return type, and any others items are parameter
    types.
    """
    try:
        # TODO: parsovat signatury a vracet slovne
        func = context.rpc_methods[name]
        return getattr(func, 'signature', '')
    except KeyError:
        raise Exception("Method '%s' doesn't exist" % name)
system_method_signature.signature = 's:s'


def system_method_help(context, name):
    """
    Given the name of a method, return a help string.
    """
    try:
        func = context.rpc_methods[name]
        return inspect.getdoc(func)
    except KeyError:
        raise Exception("Method '%s' doesn't exist" % name)
system_method_help.signature = 's:s'


INTROSPECTION_METHODS = {
    'system.listMethods': system_list_methods,
    'system.methodSignature': system_method_signature,
    'system.methodHelp': system_method_help,
}
