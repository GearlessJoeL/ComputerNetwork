from asyncore import read, write
import json
import re
import csv
import sys

means = sys.argv[1]
source = sys.argv[2]

def csv_to_json(source, destination):
    with open(source, 'r') as file:
        reader = list(csv.reader(file))
        with open(destination, 'w') as file:
            file.write('[')
            for line in reader[:-1]:
                file.write('{')
                file.write("\"id\": \"%s\", " % line[0])
                file.write("\"name\": \"%s\", " % line[1])
                file.write("\"type\": \"%s\", " % line[2])
                file.write("\"hardware\": {\"model\": \"%s\", " % line[3])
                file.write("\"sn\": \"%s\"}, " % line[4])
                file.write("\"software\": { \"version\": \"%s\", " % line[5])
                file.write("\"last_update\": \"%s\"}, " % line[6])
                file.write("\"nic\": [{\"type\": \"%s\", " % line[7])
                file.write("\"mac\": \"%s\", " % line[8])
                file.write("\"ipv4\": \"%s\"}" % line[9])
                if line[10]:
                    file.write(", {\"type\": \"%s\", " % line[10])
                    file.write("\"mac\": \"%s\", " % line[11])
                    file.write("\"ipv4\": \"%s\"}" % line[12])
                file.write("], ")
                file.write("\"state\": \"%s\"}, " % line[13])
            if len(reader) > 1:
                line = reader[-1]
                file.write('{')
                file.write("\"id\": \"%s\", " % line[0])
                file.write("\"name\": \"%s\", " % line[1])
                file.write("\"type\": \"%s\", " % line[2])
                file.write("\"hardware\": {\"model\": \"%s\", " % line[3])
                file.write("\"sn\": \"%s\"}, " % line[4])
                file.write("\"software\": { \"version\": \"%s\", " % line[5])
                file.write("\"last_update\": \"%s\"}, " % line[6])
                file.write("\"nic\": [{\"type\": \"%s\", " % line[7])
                file.write("\"mac\": \"%s\", " % line[8])
                file.write("\"ipv4\": \"%s\"}" % line[9])
                if line[10]:
                    file.write(", {\"type\": \"%s\", " % line[10])
                    file.write("\"mac\": \"%s\", " % line[11])
                    file.write("\"ipv4\": \"%s\"}" % line[12])
                file.write("], ")
                file.write("\"state\": \"%s\"}" % line[13])
            file.write(']')

def json_to_csv(source, destination):  
    with open(source, 'r', encoding='utf-8') as file:
        with open(destination, 'w', encoding='utf-8') as des:
            content = file.read()
            content = content[1:-1]
            #dct = json.loads(content)
            devices = content.split("{\"id\"")
            writer = csv.writer(des)
            for device in devices:
                lst = []
                if not device:
                    continue
                device = "{\"id\"" + device
                if device[-1] == ' ':
                    device = device[:-2]
                dct = json.loads(device)
                for val in dct.values():
                    if type(val) == str:
                        lst.append(val)
                    elif type(val) == dict:
                        for v in val.values():
                            lst.append(v)
                    elif type(val) == list:
                        for s in val:
                            for v in s.values():
                                lst.append(v)
                if len(lst) < 14:
                    lst.insert(10, '')
                    lst.insert(11, '')
                    lst.insert(12, '')
                writer.writerow(lst)
        
            
if means == '-b':
    destination = source[:-4] + '.json'
    csv_to_json(source, destination)
elif means == '-p':
    destination = source[:-5] + '.csv'
    json_to_csv(source, destination)