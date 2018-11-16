# coding=utf-8
"""
Flask extension that allows it to be used as a fastrpc/xmlrpc server.

If no fastrpc is found, xmlrpc will be used instead - for environments where fastrpc is just not possible (windows).

Python 3.5 and later is supported (3.4 if you have typing installed manually).

The current versions only handles binary requests under uWSGI, more on line 102.

Author David Jetelina <david.jetelina@firma.seznam.cz>.
Based on fastrpc aiohttp handler by Dan Milde, which was based on fastrpc Tornado handler by Jan Seifert :).
"""
import inspect
from typing import Any, Callable, List, Set, Optional, Dict, Union

try:
    import fastrpc
except ImportError:
    fastrpc = None
    from xmlrpc import client as xmlrpc
try:
    import uwsgi
except ImportError:
    uwsgi = None
import logging
from traceback import format_exc
from flask import Response, request, Flask
from typeguard import typechecked

try:
    from flask import _app_ctx_stack as stack  # flask 0.9+
except ImportError:
    from flask import _request_ctx_stack as stack

FRPC_CONTENT_TYPE = 'application/x-frpc'
RPC_CONTENT_TYPE = 'text/xml'


class FastRPCHandler:
    """The handler."""

    def __init__(self, app: Flask=None, register_introspection_methods: bool=True,
                 allowed_content_types: Optional[List]=None, url: str='/RPC2', enforce_types: bool=True) -> None:
        """
        :param app:                                 Flask application to be registered
        :param register_introspection_methods:      If the server should have system.listMethods and system.methodHelp
        :param allowed_content_types:               By default accepts application/x-frpc and test/xml
        :param url:                                 URL to be registered, /RPC2 by default
        :param enforce_types:                       Whether type annotations of your methods should be enforced on runtime
        """
        if not fastrpc:
            logging.warning('No fastrpc module found, using xmlrpc instead')
        self.app = app
        self.methods = {}  # type: Dict[str, Callable]
        self.enforce_types = enforce_types

        if allowed_content_types:
            self.allowed_content_types = set(allowed_content_types)
        else:
            self.allowed_content_types = {RPC_CONTENT_TYPE}
            if fastrpc:
                self.allowed_content_types = {FRPC_CONTENT_TYPE, RPC_CONTENT_TYPE}

        if register_introspection_methods:
            self.register_method('system.listMethods', self._system_list_methods)
            self.register_method('system.methodHelp', self._system_method_help)

        self.register_method('server.stat', self.stat)

        self.app.add_url_rule(rule=url, endpoint='RPC', view_func=self.handle, methods=['POST'])

    def register_method(self, method_name: str, func: Callable):
        """
        Register FastRPC method

        :param method_name:     Name of the method - how it will be called from the outside
        :param func:            The function that will handle that call
        """
        if self.enforce_types:
            self.methods[method_name] = typechecked(func)
        else:
            self.methods[method_name] = func

    def handle(self) -> Union[Response, tuple]:
        """
        Handle a FastRPC request, returns Flask response
        """
        accept_cts = self._get_accepted_content_types()

        if not accept_cts.intersection(self.allowed_content_types):
            logging.warning('No supported content type requested: "%s"', accept_cts)
            return 'Content types in Accept not supported', 400

        if request.headers['Content-Type'] not in self.allowed_content_types:
            logging.warning('Content-Type "%s" is not supported', request.headers['Content-Type'])
            return 'Content-Type not supported', 400

        if fastrpc:
            if FRPC_CONTENT_TYPE == request.headers['Content-Type']:
                # We will be loading binary data, which is sent through chunked transfer encoding - not very friendly.
                # Werkzeug doesn't recognize the header, so the data should be in request.stream BUT
                # since chunked transfer encoding isn't sending Content-Length header, as it does not make sense,
                # werkzeug needs some kind of middleware that handles it. In ideal world, we could use stream
                # because the middleware would set request.environ['wsgi.input_terminated'] - I found none that do that
                # which means for now we'll be supporting just uwsgi until I figure out how to do with with the others
                # like gunicorn etc. big TODO !
                if uwsgi is None:
                    raise NotImplementedError("This application needs to be running on uWSGI, I'm sorry! TODO :) ")
                request_data = uwsgi.chunked_read()
            else:
                request_data = request.data
            args, method_name = fastrpc.loads(request_data)
        else:
            args, method_name = xmlrpc.loads(request.data)

        logging.info('Calling method %s with args: %s', method_name, args)

        return self._create_response(method_name, args, accept_cts)

    def _get_accepted_content_types(self) -> Set[str]:
        accept_cts = [content_type.strip() for content_type in request.headers.get('Accept', '').split(',')]
        if not accept_cts:
            accept_cts = [request.headers['Content-Type']]
        return set(accept_cts)

    def _create_response(self, method_name: str, args: Any, accept_cts: Set[str]) -> Response:
        response = None

        if method_name not in self.methods:
            response = {
                'status': 501,
                'statusMessage': "Method '{}' not found.".format(method_name)
            }
        else:
            try:
                method = self.methods[method_name]
                response = method(*args)
            except Exception as ex:
                # .error() instead of .exception(), because that way it doesn't break log parsing when someone
                # is using that, because it inserts the exception into the message itself, instead of paste after
                logging.error('In method call %s: \n%s', method_name, format_exc())
                response = {
                    'status': 500,
                    'statusMessage': str(ex)
                }

        logging.info('Response: %s', response)

        _response_autostatus(response)

        headers = {}
        if FRPC_CONTENT_TYPE in accept_cts and fastrpc:
            use_binary = True
            headers['Content-Type'] = FRPC_CONTENT_TYPE
        else:
            use_binary = False
            headers['Content-Type'] = RPC_CONTENT_TYPE

        if fastrpc:
            headers['Accept'] = ','.join(self.allowed_content_types)
            body = fastrpc.dumps((response,), methodresponse=True, useBinary=use_binary)
        else:
            body = xmlrpc.dumps((response,), methodresponse=True)
        headers['Content-Length'] = str(len(body))

        resp = Response(body, headers=headers)
        return resp

    def _system_list_methods(self) -> List[str]:
        """
        Return list of all available FastRPC methods.
        """
        return list(self.methods.keys())

    def _system_method_help(self, method_name: str) -> str:
        """
        Return help for method.
        """
        try:
            func = self.methods[method_name]
            doc = inspect.getdoc(func)
            return doc if doc else ''
        except KeyError:
            raise Exception("Method '%s' doesn't exist" % method_name)

    def stat(self):
        """Basic method returning status 200"""
        return {}


def _response_autostatus(response: Any) -> None:
    if not isinstance(response, dict):
        return
    if 'status' not in response:
        response['status'] = 200
    if 'statusMessage' not in response and response['status'] == 200:
        response['statusMessage'] = 'OK'
