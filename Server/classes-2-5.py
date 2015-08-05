#!/C/Python27/python.exe
import sys
class A:
  """Class A desribes a class"""
  integer = 1
  def func(self):
    print "Hello, int=%d", self.integer

print( sys.version )

a = A()

a.func()

raw_input()
