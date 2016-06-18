import DatabaseInterface

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
    def __init__(self, row):
        super(Category, self).__init__()
        self.row = row
        self.id = row["id"]
        self.fullname = row["fullname"]
        self.name = row["name"]
        self.model = row["3dmodel"]
        self.texturetemplate = row["texturetemplate"]
        self.linkedcategories = row["linkedcategories"]

if __name__ == "__main__":
    DB = DatabaseInterface.Database()
    cats = DB.select("categories")

    print cats[0].fullname
