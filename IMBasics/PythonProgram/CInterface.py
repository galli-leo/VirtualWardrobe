from DBWrappers import ClothingItem, Category
import DBWrappers
from TextureRecognizers import CategoryRecognizer
from TextureCreators import NoiseCreator
import importlib
import os
import glob
import sys
import Vars
from Vars import LOG
from dynreload import recompile
from PIL import Image


def loadModule(path = "modules/recognizers"):
    python_files = glob.glob(path+"/*.py")
    for f in python_files:
        head, tail = os.path.split(f)
        print(head, tail)
        (filename, ext) = os.path.splitext(tail)
        sys.path.append(head)
        module = importlib.import_module(filename)
        print(recompile(filename))
        reload(sys.modules[filename])

def loadModules(path = "modules"):
    loadModule(os.path.join(path, "recognizers"))
    loadModule(os.path.join(path, "creators"))

def reloadImportantModules():
    reload(TexutreRecognizers)
    reload(TexutreCreators)

def helloFromPython(name):
    return "Hello {0} from Python!".format(name)

def getCurrentPath():
    return os.getcwd()

def createNewItemWithTextures(back1, back2):
    clothing = ClothingItem()
    LOG.debug("Created new clothing item: {0}".format(clothing.id))
    img1 = Image.open(back1)
    img2 = Image.open(back2)
    LOG.debug("Opened images.")
    clothing.addImage(img1, "back1")
    clothing.addImage(img2, "back2")
    LOG.debug("Added images.")
    clothing.createFinalTexture()
    return clothing.id

def categorize(id, front_image):
    print CategoryRecognizer().recognizeCategory(None, None)
    pass

Vars.TEXTURE_FOLDER = os.path.join("PythonProgram", "textures")
Vars.DBFILE = os.path.join("PythonProgram", "shirt_db.db")
Vars.REALPATH = "PythonProgram"
print(Vars.DBFILE)
#clothes = DBWrappers.loadCategories()

def test():
    createNewItemWithTextures("back1.png", "back2.png")

if __name__ == "__main__":
    createNewItemWithTextures("back1.png", "back2.png")
