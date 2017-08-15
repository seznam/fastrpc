from aiohttp import web
from fastrpc.handler.aiohttp import FastRPCHandler


async def add(a, b):
    """
    Adds `a` to `b`.
    """
    return a + b


handler = FastRPCHandler()
handler.register_method('add', add)

app = web.Application()
app.router.add_post('/RPC2', handler.handle)

web.run_app(app, port=5000)
