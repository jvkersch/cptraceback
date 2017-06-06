import unittest

from .utils import (
    import_example_module, parse_output, capture_output, STDERR
)


class TestPrintToStderr(unittest.TestCase):
    # This is a very rudimentary test. It just checks that we're calling the
    # right Python functions *somewhere*.

    def test_example(self):
        pymod = import_example_module('python_calling_cython')
        with capture_output(STDERR) as redir:
            pymod.run()  # invokes print_tb

        tb_objs = parse_output(redir.output)
        python_functions = set(tb.pyname for tb in tb_objs)

        for pyfun in ('foo', 'bar', 'run', '<module>'):
            self.assertIn(pyfun, python_functions)
