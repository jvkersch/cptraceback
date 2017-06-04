from cymod import my_cython_fun


class A(object):

    def foo(self):

        def bar():
            my_cython_fun()

        bar()


def run():
    a = A()
    a.foo()


if __name__ == '__main__':
    run()
