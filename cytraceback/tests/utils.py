# -*- coding: utf-8 -*-
from __future__ import division, print_function

import contextlib
import os
import sys

import cytraceback
from .redir import Redirector, STDOUT, STDERR  # noqa


def import_example_module():
    mod_path = os.path.dirname(cytraceback.__file__)
    try:
        sys.path.append(mod_path)
        from example import pymod
        return pymod
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
