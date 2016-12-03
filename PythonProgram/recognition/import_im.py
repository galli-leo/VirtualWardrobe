import os
import re
import win32api
from sys import platform
import sys
PATH = None

def cache_folder(folder=None):
    global PATH
    try:
        if folder == None:
            with open("path.txt", "r") as f:
                PATH = f.read()
        else:
            with open("path.txt", "w") as f:
                f.write(folder)
    except:
        pass


def find_folder(root_folder, rex):
    for root,dirs,files in os.walk(root_folder):
        for d in dirs:
            result = rex.search(d)
            if result:
                return os.path.join(root,d)
                break                         #if you want to find only one


def find_folder_in_all_drives(folder_name):
    #create a regular expression for the file
    rex = re.compile(folder_name)
    if platform == "win32":
        return find_folder( "E:\\", rex )
    else:
        return find_folder("/", rex)

def find_python_path():
    global PATH
    cache_folder()
    if PATH is None:
        im_path = find_folder_in_all_drives("IntelligentMirror")
        PATH = os.path.join(im_path, im_path, "PythonProgram")
        cache_folder(PATH)
        return PATH
    else:
        return PATH

sys.path.append(find_python_path())
