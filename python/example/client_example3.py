#!/usr/bin/env python
# coding=utf-8
"""
  Simple client initialized from configuration file

Copyright (C) 2013 by Tomáš Golembiovský
"""

import fastrpc
import ConfigParser


cfg = ConfigParser.RawConfigParser()
cfg.read("client.conf")

client = fastrpc.ServerProxy(cfg, "proxy")

try:
    result = client.system.listMethods()

    print result
    print client.system.methodSignature(result[4])

except fastrpc.Fault as f:
    print f
except fastrpc.ProtocolError as e:
    print e
