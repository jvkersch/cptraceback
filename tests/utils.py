# -*- coding: utf-8 -*-
from __future__ import division, print_function

import contextlib
import importlib
import os
import re
import sys

import cptraceback
from .redir import Redirector, STDOUT, STDERR  # noqa

# Regular expressions to parse a traceback
CP_RE = re.compile(
    "(?P<address>0x[a-f0-9]+): \((?P<name>[^-]+)\+(?P<offset>0x[a-f0-9]+)\)"
)
PY_RE = re.compile(
    "\tPython: (?P<pyname>[^-]+) \((?P<pyfile>[^-]+):(?P<pylineno>[0-9]+)\)"
)


def import_example_module(folder):
    base_path = os.path.dirname(cptraceback.__file__)
    mod_path = os.path.join(base_path, "examples", folder)
    try:
        sys.path.append(mod_path)
        mod = importlib.import_module("example")
        return mod
    except ImportError:
        # TODO slightly more useful message here...
        sys.stderr.write("Could not import example module.\n")
        raise
    finally:
        # restore sys.path
        try:
            sys.path.remove(mod_path)
        except ValueError:
            pass


class TBObject(object):

    def __init__(self, address, name, offset,
                 pyname=None, pyfile=None, pylineno=None):
        self.address = address
        self.name = name
        self.offset = offset
        self.pyname = pyname
        self.pyfile = pyfile
        self.pylineno = pylineno

    def __repr__(self):
        fields = []
        attrs = ('address', 'name', 'offset', 'pyname', 'pyfile', 'pylineno')
        for attr in attrs:
            value = getattr(self, attr)
            if value is not None:
                fields.append("{}={!r}".format(attr, value))
        return "TBObject({})".format(", ".join(fields))


def parse_output(tbstring):
    tb_objs = []
    for line in tbstring.splitlines():
        if line[0] != '\t':  # C line
            m = CP_RE.match(line)
            if m is None:
                raise ValueError("Invalid C traceback line: {!r}".format(line))
            tb_obj = TBObject(**m.groupdict())
            tb_objs.append(tb_obj)
        else:
            m = PY_RE.match(line)
            if m is None:
                raise ValueError(
                    "Invalid Python traceback line: {!r}".format(line)
                )
            # Update last tb object with Python info
            if len(tb_objs) == 0:
                raise ValueError(
                    "Orphan Python traceback line: {!r}".format(line)
                )
            tb_obj = tb_objs[-1]
            for attr in ('pyname', 'pyfile', 'pylineno'):
                setattr(tb_obj, attr, m.groupdict()[attr])
    return tb_objs


@contextlib.contextmanager
def capture_output(fd):
    r = Redirector(fd=fd)
    r.start()
    try:
        yield r
    finally:
        r.stop()
