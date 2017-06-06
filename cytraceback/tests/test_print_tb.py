import unittest

from .utils import (
    import_example_module, parse_tb_entry, capture_output, STDERR
)


class TestPrintToStderr(unittest.TestCase):
    # This is a very rudimentary test. It just checks that we're calling the
    # right Python functions *somewhere*.

    def test_example(self):
        with capture_output(STDERR) as redir:
            pymod = import_example_module('python_calling_cython')
            pymod.run()  # invokes print_tb

        parsed_tb = map(parse_tb_entry, redir.output.splitlines())
        python_functions = set(s[-1] for s in parsed_tb)

        for pyfun in ('foo', 'bar', 'run', '<module>'):
            self.assertIn(pyfun, python_functions)
