"""
FastRPC handler for Aiohttp.

Asynchronous handler takes data from XML-RPC/FastRPC POST request, converts them into python objects, calls corresponding
user method and finally creates XML-RPC/FastRPC response.

Python 3.5 and later is supported.

Author Daniel Milde <daniel.milde@firma.seznam.cz>.
Inspired by Jan Seifert's FastRpcHandler for Tornado.
"""

import inspect
import logging
from typing import Any, Callable, Dict, List, Set

from aiohttp import web
import fastrpc

logger = logging.getLogger('aiohttp_fastprc')

FRPC_CONTENT_TYPE = 'application/x-frpc'
RPC_CONTENT_TYPE = 'text/xml'


class FastRPCHandler:
    def __init__(self, register_introspection_methods=True, allowed_content_types=None):
        self.methods = {}

        if allowed_content_types:
            self.allowed_content_types = set(allowed_content_types)
        else:
            self.allowed_content_types = set([
                FRPC_CONTENT_TYPE,
                RPC_CONTENT_TYPE,
            ])

        if register_introspection_methods:
            self.register_method('system.listMethods', self._system_list_methods)
            self.register_method('system.methodHelp', self._system_method_help)

    def register_method(self, method_name: str, func: Callable) -> None:
        """
        Register FastRPC method
        """
        self.methods[method_name] = func

    async def handle(self, request: Any) -> Any:
        """
        Handle FastRPC request, returns aiohttp response
        """
        accept_cts = self._get_accepted_content_types(request)

        if not accept_cts.intersection(self.allowed_content_types):
            logger.warn('No supported content type requested: "%s"', accept_cts)
            return web.Response(status=400, body='Content types in Accept not supported')

        if request.headers['Content-Type'] not in self.allowed_content_types:
            logger.warn('Conent-Type "%s" is not supported', request.headers['Content-Type'])
            return web.Response(status=400, body='Content-Type not supported')

        (args, method_name) = fastrpc.loads(await request.content.read())

        logger.debug('Calling method %s with args: %s', method_name, args)

        return await self._create_response(method_name, args, accept_cts)

    def _get_accepted_content_types(self, request: Any) -> Set[str]:
        accept_cts = [content_type.strip() for content_type in request.headers.get('Accept', '').split(',') if content_type]
        if not accept_cts:
            accept_cts = [request.headers['Content-Type']]
        return set(accept_cts)

    async def _create_response(self, method_name, args, accept_cts):
        response = None

        if method_name not in self.methods:
            response = {
                'status': 501,
                'statusMessage': "Method '{}' not found.".format(method_name)
            }
        else:
            try:
                method = self.methods[method_name]
                response = await method(*args)
            except Exception as ex:
                logger.exception('In method call')
                response = {
                    'status': 500,
                    'statusMessage': str(ex)
                }

        logger.debug('Response: %s', response)

        headers = {}
        if FRPC_CONTENT_TYPE in accept_cts:
            use_binary = fastrpc.ALWAYS
            headers['Content-Type'] = FRPC_CONTENT_TYPE
        else:
            use_binary = fastrpc.NEVER
            headers['Content-Type'] = RPC_CONTENT_TYPE

        body = fastrpc.dumps((response,), methodresponse=True, useBinary=use_binary)
        headers['Content-Length'] = str(len(body))
        headers['Accept'] = ','.join(self.allowed_content_types)

        return web.Response(
            body=body,
            headers=headers,
        )

    async def _system_list_methods(self) -> List[str]:
        """
        Return list of all available FastRPC methods.
        """
        return list(self.methods.keys())

    async def _system_method_help(self, method_name: str) -> str:
        """
        Return help for method.
        """
        try:
            func = self.methods[method_name]
            doc = inspect.getdoc(func)
            return doc if doc else ''
        except KeyError:
            raise Exception("Method '%s' doesn't exist" % method_name)
