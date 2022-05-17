import json
import sys
import re

args = sys.argv
file1 = args[1]
d = {}

def output(obj):
    print(json.dumps(obj))

with open(file1, 'r') as source:
    line = source.read()
    lst = re.split("[. |? |! |.\n|?\n|!\n|\s]", line)
    for word in lst:
        if word:
            word = word.lower()
            if word in d:
                d[word] = d[word] + 1
            else :
                d[word] = 1

output(d)

