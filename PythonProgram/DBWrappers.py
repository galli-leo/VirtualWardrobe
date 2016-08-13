import DatabaseInterface
import os, glob
from PIL import Image, ImageChops
import Vars
from TextureCreators import TextureCreator, BigTileCreator, TilableCreator, NoiseCreator
from TextureRecognizers import TemplateRecgonizer, CategoryRecognizer
from Vars import LOG

class DatabaseWrapper(object):
    def __init__(self):
        super(DatabaseWrapper, self).__init__()
        self.id = 0

    @classmethod
    def fromRow(cls, row):
        new = cls()
        new.row = row
        new.id = row["id"]
        return new


class Category(object):
    """docstring for Category"""
    def __init__(self):
        super(Category, self).__init__()
        self.id = 0
        self.fullname = ""
        self.name = ""
        self.model = ""
        self.texturetemplate = ""
        self.linkedcategories = ""

    @classmethod
    def fromRow(cls, row):
        new = cls()
        new.row = row
        new.id = row["id"]
        new.fullname = row["fullname"]
        new.name = row["name"]
        new.model = row["3dmodel"]
        new.texturetemplate = row["texturetemplate"]
        new.linkedcategories = row["linkedcategories"]
        return new


    def getPath(self, texture_path=None):
        if texture_path is None:
            texture_path = Vars.TEXTURE_FOLDER
        return os.path.join(texture_path, self.name)

class ClothingItem(object):
    """docstring for ClothingItem"""
    def __init__(self, createNewID = True):
        super(ClothingItem, self).__init__()
        if createNewID:
            self.id = Vars.DB.createNewEntry("clothes", fields={"category":"1", "finaltexture":"NULL", "printedtexture":"0"})
        self.category = Vars.CATEGORIES[1]
        self.frontimage = False
        self.finaltexture = ""
        self.printedtexture = False
        self.texture_creators = {"tshirt" : NoiseCreator, "uncategorized" : NoiseCreator}
        self.diffuse_textures = {"tshirt" : "tshirt_diffuse.png"}


    @classmethod
    def fromRow(cls, row):
        new = cls(False)
        new.row = row
        new.id = row["id"]
        if row["category"] == None:
            new.category = Vars.CATEGORIES[0]
        else:
            new.category = Vars.CATEGORIES[row["category"]]
        new.finaltexture = row["finaltexture"]
        new.frontimage = bool(int(row["frontimage"]))
        new.printedtexture = bool(int(row["printedtexture"]))
        new.loadTextures()

        return new

    def getTexturePath(self, texture_path=None):
        if texture_path is None:
            texture_path = Vars.TEXTURE_FOLDER
        path = os.path.join(texture_path, self.category.name, "%04d" % (self.id,))
        try:
            os.makedirs(path)
        except OSError:
            if not os.path.isdir(self.category.getPath()):
                raise
        return path

    def loadTextures(self):
        path = self.getTexturePath()
        self.texturesamples = []
        self.texturesamples = glob.glob(path+"/*.png")
        self.texturesamples.extend(glob.glob(path+"/*.jpg"))
        self.texturesamples.extend(glob.glob(path+"/*.jpeg"))

    def addImage(self, image, name="back", extension=".png"):
        path = self.getTexturePath()
        image.save(os.path.join(path, name+extension))
        self.loadTextures()

    def addFrontImage(self, image):
        self.addImage(image, name="front")

    def createFinalTexture(self):
        creator_cls = self.texture_creators[self.category.name]
        self.loadTextures()
        printed_texture = None
        if self.printedtexture:
            path = self.getTexturePath()
            printed_texture = Image.open(os.path.join(path, "print"+".png"))
        creator = creator_cls(self.texturesamples, printed_texture)
        final_texture = creator.createTexture()
        dif_text = Image.open(os.path.join(Vars.REALPATH, self.diffuse_textures[self.category.name]))
        final_texture = ImageChops.multiply(final_texture, dif_text.convert("RGBA"))
        final_texture.save(os.path.join(self.getTexturePath(), "final_texture.png"))
        return final_texture

    def guessClothCategory(self, joints):
        #Take front image if available
        if not self.frontimage:
            return Vars.CATEGORIES[0]
        path = self.getTexturePath()
        front_image = os.path.join(path, "front"+".png")
        img = Image.open(front_image)
        (confidence, category) = CategoryRecognizer().recognizeCategory(self.frontimage, joints)
        for cat in Vars.CATEGORIES:
            if cat.name == category:
                self.category = cat





def loadCategories():
    DB = DatabaseInterface.Database()
    Vars.DB = DB
    cats = DB.select("categories")
    Vars.CATEGORIES = cats
    for cat in cats:
        try:
            os.makedirs(cat.getPath())
        except OSError:
            if not os.path.isdir(cat.getPath()):
                raise
    return loadClothes()

def testingRecognizer(clothes):
    template = TemplateRecgonizer()
    match = template.findMatch("recog.png", clothes)
    LOG.debug("Found Match with ID: {0}, confidence: {1}".format(match[1].id, match[0]))

def createNewClothingItem():
    textures = ["back.png", "back2.png"]
    clothing = ClothingItem()
    count = 1
    for text in textures:
        img = Image.open(text)
        clothing.addImage(img, "back{0}".format(count))
        count += 1

def loadClothes():
    DB = DatabaseInterface.Database()
    clothes = DB.select("clothes")
    #text = clothes[len(clothes)-1].createFinalTexture()
    #text.show()
    return clothes
    #createNewClothingItem()
    #testingRecognizer(clothes)

if __name__ == "__main__":
    loadCategories()
