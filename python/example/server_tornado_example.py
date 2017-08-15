import argparse

from fastrpc.handler.tornado import FastRpcHandler
from shelter.core import context, config
import tornado.ioloop
import tornado.web


def add(context, a, b):
    """
    Adds `a` to `b`.
    """
    return a + b


class Config(config.Config):
    pass


class Context(context.Context):
    rpc_methods = {'add': add}


settings = {}
config = Config(settings, argparse.Namespace())
context = Context(config)
url = tornado.web.URLSpec(r'/RPC2', FastRpcHandler)
url.target_kwargs['context'] = context
url.target_kwargs['interface'] = None

application = tornado.web.Application([url])
application.listen(5000)
tornado.ioloop.IOLoop.current().start()
