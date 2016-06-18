import sqlite3
DBFILE = "shirt_db.db"



class Database(object):
    """Our connection to the database"""
    def __init__(self, dbname=DBFILE):
        super(Database, self).__init__()
        self.dbname = dbname
        self.conn = sqlite3.connect(self.dbname)
        self.conn.row_factory = sqlite3.Row
        self.cursor = self.conn.cursor()

    def select(self, table, fields=[], condition=[]):
        #TODO: add fields and conditions
        self.cursor.execute("SELECT * from "+table+"")
        return self.cursor.fetchall()


if __name__ == "__main__":
    DB = Database()
    for row in DB.select("categories"):
        print(row["fullname"])
