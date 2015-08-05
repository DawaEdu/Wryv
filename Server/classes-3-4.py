#!/C/Python34/python.exe
import sys
class A:
  i = 1
  def p( self ):
    print( "HI %d" % self.i )
    print( "HI ", self.i )

print( sys.version )
a = A()
a.p()
input()

