import DBWrappers
from TextureRecognizers import CategoryRecognizer
from TextureCreators import NoiseCreator
import importlib
import os
import glob
import sys
from dynreload import recompile


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

def createNewItemWithTextures(back1, back2):
    pass

def categorize(id, front_image):
    print CategoryRecognizer().recognizeCategory(None, None)
    pass
