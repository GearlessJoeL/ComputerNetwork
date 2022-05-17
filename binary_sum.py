import sys
args = sys.argv
a = int(args[1], 2)
b = int(args[2], 2)
print(str(bin(a+b))[2:])