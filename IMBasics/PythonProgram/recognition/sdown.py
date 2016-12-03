import urllib
import os

def go(l, d):
    try:
        os.mkdir(d)
    except:
        pass
    input_file = open(l,'r')
    count = 0
    for line in input_file:
        count += 1
        line = line.replace('\n', '')
        URL = line
        IMAGE = d + "/" + "image" + str(count) + ".jpg"
        urllib.urlretrieve(URL, IMAGE)
