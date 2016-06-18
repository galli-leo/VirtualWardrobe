import sqlite3
from DBWrappers import Category
DBFILE = "shirt_db.db"

class Database(object):
    """Our connection to the database"""
    def __init__(self, dbname=DBFILE):
        super(Database, self).__init__()
        self.dbname = dbname
        self.conn = sqlite3.connect(self.dbname)
        self.conn.row_factory = sqlite3.Row
        self.cursor = self.conn.cursor()

    def select(self, table, fields=[], condition=[], raw_values=False):
        #TODO: add fields and conditions
        self.cursor.execute("SELECT * from "+table+"")
        rows = self.cursor.fetchall()
        if raw_values:
            return rows
        if table == "categories":
            cats = []
            for row in rows:
                cat = Category(row)
                cats.append(cat)
            return cats


if __name__ == "__main__":
    DB = Database()
    for row in DB.select("categories", raw_values=True):
        print(row["fullname"])
