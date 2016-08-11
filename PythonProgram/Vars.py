import Logger
import os
LOG = Logger.Logger(log_level=0, folder=None)
TEXTURE_FOLDER = "textures"
CATEGORIES = []
DBFILE = "shirt_db.db"
REALPATH = ""
DB = None

def setRealPath(path):
    global REALPATH, DBFILE, TEXTURE_FOLDER
    REALPATH = path
    DBFILE = os.path.join(path, DBFILE)
    print(path, DBFILE)
    TEXTURE_FOLDER = os.path.join(path, TEXTURE_FOLDER)
