import re, os, sys, datetime, time
import urllib2
import requests
from bs4 import BeautifulSoup as bs
import json

def go(keyword):
    d = "g_"+keyword
    try:
        os.makedir(d)
    except:
        pass
    opener = urllib2.build_opener()
    #opener.addheaders = [('User-Agent', 'Mozilla/5.0')]
    url = "https://www.google.ch/search?async=_id:rg_s,_pms:s&ei=CHUIWLDrLMjoaqfHmsAI&yv=2&q=tshirt&start=100&asearch=ichunk&num=30&newwindow=1&tbm=isch&vet=10ahUKEwiwj7Le8OjPAhVItBoKHaejBogQuT0IMSgB.CHUIWLDrLMjoaqfHmsAI.i&ved=0ahUKEwiwj7Le8OjPAhVItBoKHaejBogQuT0IMSgB&ijn=2"#.format(keyword)
    #url = "https://www.google.ch/search?q={0}&num=30&source=lnms&tbm=isch&sa=X&ved=0ahUKEwj8zrb_qufPAhWCJcAKHWlsDKUQ_AUICCgB&biw=1920&bih=950".format(keyword)
    try:
        print(url)
        r = requests.get(url)
        print(r.text.encode("utf-8"))
        j = r.json()#json.loads(r)
        s = bs(j[1][1])
        search = b.find("div", {"id":"search"})
        imgs = search.findAll("img")
        return s
    except Exception as e:
        print(e)
