# -*- coding: utf-8 -*-
from __future__ import division, print_function

import contextlib
import importlib
import os
import sys

import cytraceback
from .redir import Redirector, STDOUT, STDERR  # noqa


def import_example_module(folder):
    base_path = os.path.dirname(os.path.dirname(cytraceback.__file__))
    mod_path = os.path.join(base_path, "examples", folder)
    try:
        sys.path.append(mod_path)
        mod = importlib.import_module("example")
        return mod
    except ImportError as e:
        # TODO slightly more useful message here...
        sys.stderr.write("Could not import example module.\n")
        raise
    finally:
        # restore sys.path
        try:
            sys.path.remove(mod_path)
        except ValueError:
            pass


def parse_tb_entry(tbstring):
    fields = tbstring.split('Python:')
    pc, cfun = map(lambda s: s.strip(), fields[0].split(':', 1))
    if len(fields) == 2:
        pymodname, pyfun = map(lambda s: s.strip(), fields[1].split(':', 1))
    else:
        pymodname = ""
        pyfun = ""
    return pc, cfun, pymodname, pyfun


@contextlib.contextmanager
def capture_output(fd):
    r = Redirector(fd=fd)
    r.start()
    try:
        yield r
    finally:
        r.stop()
