from fastrpc.handler.flask import FastRPCHandler
import flask


app = flask.Flask(__name__)
rpc = FastRPCHandler(app)


def add(a, b):
    """
    Adds `a` to `b`.
    """
    return a + b


rpc.register_method('add', add)

app.run(port=5000)
