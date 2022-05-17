from curses.ascii import isdigit
import sys

args = sys.argv
file1 = args[1]
file2 = args[2]

with open(file1, 'r') as source:
    with open(file2, 'w') as destination:
        line = source.readline()
        while line:
            temp = [];
            for ch in line:
                if isdigit(ch):
                    temp.append(ch)
            temp.insert(3, '-')
            temp.insert(7, '-')
            for ch in temp:
                destination.write(ch)
            line = source.readline()
            if line:
                destination.write('\n')
