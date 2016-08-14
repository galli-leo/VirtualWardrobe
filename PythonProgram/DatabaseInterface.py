import sqlite3
#from DBWrappers import Category, ClothingItem
import DBWrappers
import os
import Vars



class Database(object):
    """Our connection to the database"""
    def __init__(self, dbname=None):
        super(Database, self).__init__()
        self.dbname = dbname
        if self.dbname is None:
            self.dbname = Vars.DBFILE
        self.conn = sqlite3.connect(self.dbname)
        sqlite3.register_adapter(bool, int)
        sqlite3.register_converter("BOOLEAN", lambda v: bool(int(v)))
        self.conn.row_factory = sqlite3.Row
        self.obj_map = {"categories" : DBWrappers.Category, "clothes" : DBWrappers.ClothingItem}

    def convertRowToObject(self, row, ptype):
        if isinstance(row, list):
            arr = []
            for r in row:
                obj = ptype.fromRow(r)
                arr.append(obj)
            return arr
        else:
            return ptype(row)

    def select(self, table, fields=[], condition={}, raw_values=False):
        #TODO: add fields and conditions
        cursor = self.conn.cursor()
        statement = "SELECT * from " + table
        if condition:
            statement += " WHERE "
            for key,value in condition.iteritems():
                statement += (key + "=" + value + " ")
        cursor.execute(statement)
        rows = cursor.fetchall()
        if raw_values:
            return rows
        if table in self.obj_map:
            return self.convertRowToObject(rows, self.obj_map[table])

    def createNewEntry(self, table, fields={}):
        cursor = self.conn.cursor()
        cursor.execute("INSERT into "+table+" ("+",".join(fields.keys())+") VALUES ("+",".join(fields.values())+")")
        self.conn.commit()
        return cursor.lastrowid

    def delete(self, table, id):
        cursor = self.conn.cursor()
        cursor.execute("DELETE from "+table+" WHERE id=?", id)

    def update(self, table, values, conditions):
        pass

    def insert(self, table, values={}):
        pass


if __name__ == "__main__":
    DB = Database()
    Vars.DB = DB
    for row in DB.select("categories", raw_values=True):
        pass
        #print(row["fullname"])
