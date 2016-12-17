from DBWrappers import ClothingItem, Category
import DBWrappers
from TextureRecognizers import CategoryRecognizer
from TextureCreators import NoiseCreator
import DatabaseInterface
import importlib
import os
import glob
import sys
import Vars
from Vars import LOG
from dynreload import recompile
from PIL import Image
import recognition as recog
from datetime import datetime
import shutil
import atexit

@atexit.register
def goodbye():
    purgeTMP()

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
    LOG.enabled = False
    clothing = ClothingItem()
    LOG.debug("Created new clothing item: {0}".format(clothing.id))
    img1 = Image.open(back1)
    img2 = Image.open(back2)
    LOG.debug("Opened images.")
    clothing.addImage(img1, "back1")
    clothing.addImage(img2, "back2")
    LOG.debug("Added images.")
    clothing.createFinalTexture()
    LOG.enabled = True
    return clothing.id

def createNewItemWithTexturesFromCWD():
    #Hack so this function can be called from any thread
    tempDB = Vars.DB
    Vars.DB = DatabaseInterface.Database()
    clothing = ClothingItem()
    LOG.debug("Created new clothing item: {0}".format(clothing.id))
    img1 = Image.open("back1.png")
    img2 = Image.open("back2.png")
    LOG.debug("Opened images.")
    clothing.addImage(img1, "back1")
    clothing.addImage(img2, "back2")
    LOG.debug("Added images.")
    clothing.createFinalTexture()
    Vars.DB = tempDB
    return clothing.id

def newWithTexturesFromCWD(cat):
    #Hack so this function can be called from any thread
    tempDB = Vars.DB
    Vars.DB = DatabaseInterface.Database()
    clothing = ClothingItem(category = cat)
    LOG.debug("Created new clothing item: {0}".format(clothing.id))
    img1 = Image.open("back1.png")
    img2 = Image.open("back2.png")
    LOG.debug("Opened images.")
    clothing.addImage(img1, "back1")
    clothing.addImage(img2, "back2")
    LOG.debug("Added images.")
    clothing.createFinalTexture()
    Vars.DB = tempDB
    return clothing.id

def addPrintToItemFromCWD(id):
    #Hack so this function can be called from any thread
    tempDB = Vars.DB
    Vars.DB = DatabaseInterface.Database()
    Vars.DB.update("clothes", {"printedtexture" : "1"}, {"id" : str(id)})
    img = Image.open("print_testing.png")
    items = Vars.DB.select("clothes", condition = {"id" : str(id)})
    cloth = items[0]
    cloth.addImage(img, "print")
    cloth.createFinalTexture()
    Vars.DB = tempDB
    return

def recreateTextureForID(id):
    items = Vars.DB.select("clothes", condition = {"id" : str(id)})
    items[0].createFinalTexture()

def categorize(id, front_image):
    print CategoryRecognizer().recognizeCategory(None, None)
    pass

def initWithPath(path):
    global clothes
    Vars.setRealPath(path)
    clothes = DBWrappers.loadCategories()
    initRecogModel()
    return Vars.DBFILE

def initRecogModel(model="E:\\DeepLearning\\Clothing Recognition\\protocol\\seven\\model_weights.pkl"):
    recog.load(model)

def purgeTMP():
    for f in os.listdir(os.path.join(Vars.REALPATH, "tmp/prediction")):
        p = os.path.join(Vars.REALPATH, "tmp/prediction", f)
        try:
            pass
            #os.unlink(p)
        except:
            pass

def predictCategory():
    t = datetime.utcnow().strftime('%Y-%m-%d %H-%M-%S+%f')[:-3]
    f = os.path.join(Vars.REALPATH, "tmp", "prediction", "{0}.bmp".format(t))
    print(f)
    shutil.copy("predict.bmp", f)
    img = Image.open(f)
    half_the_width = img.size[0] / 2
    crop_width = 690 / 2
    img = img.crop((half_the_width - crop_width, 0, half_the_width + crop_width, 1080))
    img.save(f)
    result = recog.predict(f)
    pred_str = "predicted_{0}_confidence_{1}.bmp".format(result[1], result[2])
    shutil.move(f, os.path.join(Vars.REALPATH, "tmp", "prediction", pred_str))
    return result
    proba = result[2]
    if proba < .01:
        return 1 #T-Shirt is the most difficult to detect, if uncertain just use T-Shirt (Could go wrong!!!)
    else:
        if result[0] == 3 and result[2] > .55:
            return 6
        if result[0] == 4 and result[2] > .79:
            return 4
        if result[0] == 1 and result[2] > .50:
            return 1
        if result[0] == 5 and result[2] > .30:
            return 5
        else:
            return 420 #Blazeit
        return str(result[1]) + ", " +  str(proba)

#Vars.TEXTURE_FOLDER = os.path.join("PythonProgram", "textures")
#Vars.DBFILE = os.path.join("PythonProgram", "shirt_db.db")
#Vars.REALPATH = "PythonProgram"
#print(Vars.DBFILE)
#
clothes = None

def test():
    createNewItemWithTextures("back1.png", "back2.png")

if __name__ == "__main__":
    #createNewItemWithTextures("back1.png", "back2.png")
    initWithPath(".")
    #addPrintToItemFromCWD(145)
    recreateTextureForID(164)
    #newWithTexturesFromCWD(6)
