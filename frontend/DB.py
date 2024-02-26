import sqlite3
from sqlite3 import Error

class DB:

    def __init__(self):
        self.conn = None
    
    def setup(self):
        first_creation_monitored_systems=True
        
        if self.conn is not None:        
            cur = self.conn.cursor()
            
            #Check monitored_systems table
            exists = True
            table_name = "monitored_systems"
            cur.execute("""
                SELECT name 
                FROM sqlite_master 
                WHERE type='table' AND name=?;
            """, (table_name, ))

            exists = bool(cur.fetchone())

            if not exists:
                print("Creo tabella dei sistemi!")
                first_creation_monitored_systems=exists
                cur.execute("CREATE TABLE monitored_systems ( id_systems INTEGER PRIMARY KEY AUTOINCREMENT, address VARCHAR(50), type VARCHAR(25), lat FLOAT, lon FLOAT, description TEXT(1000))")
                exists = True


            #Check systems_history table
            exists = True
            table_name = "systems_history"
            cur.execute("""
                SELECT name 
                FROM sqlite_master 
                WHERE type='table' AND name=?;
            """, (table_name, ))

            exists = bool(cur.fetchone())

            if not exists:
                print("Creo tabella cronologia!")
                cur.execute("CREATE TABLE systems_history ( id_history INTEGER PRIMARY KEY AUTOINCREMENT, id_sys VARCHAR(50), last_time_checked DATETIME, stato VARCHAR(20),FOREIGN KEY (id_sys) REFERENCES monitored_systems(id_systems))")
                exists = True

            return(first_creation_monitored_systems)    

    def create_connection(self,db_file):
        """ create a database connection to a SQLite database """
        try:
            conn = sqlite3.connect(db_file)
            print(sqlite3.version)
            return conn
        except Error as e:
            print(e)
        finally:
            if conn:
                self.conn = conn
            
    def get_all_systems(self):
        cur = self.conn.cursor()
        
        cur.execute("""SELECT m.*, h.stato
         FROM monitored_systems m JOIN systems_history h ON (m.id_systems = h.id_sys)

            where h.last_time_checked = (select max(h1.last_time_checked) from systems_history h1 where h1.id_sys = h.id_sys) ORDER BY m.id_systems ASC """)
        
        rows = cur.fetchall()
        data = []
        for row in rows:
            data.append(row)
        
        return data
    
    def get_all_history(self):
        cur = self.conn.cursor()

        cur.execute("SELECT * FROM systems_history")

        rows = cur.fetchall()
        data = []
        for row in rows:
            data.append(row)
        

        return data

    def insert_sensor(self, name,type,lat,lon,description):
        cur = self.conn.cursor()

        sqlite_insert_with_param = """INSERT INTO monitored_systems (address,type,lat,lon,description) VALUES(?,?,?,?,?);"""

        data_tuple = (name, type, lat, lon,description)
        cur.execute(sqlite_insert_with_param, data_tuple)
        self.conn.commit()

        return
    
    def insert_registration(self, id_sys, last_time_checked,stato):
        cur = self.conn.cursor()

        sqlite_insert_with_param = """INSERT INTO systems_history ( id_sys, last_time_checked, stato) VALUES(?,?,?);"""

        data_tuple = (id_sys, last_time_checked,stato)
        cur.execute(sqlite_insert_with_param, data_tuple)
        self.conn.commit()

        return
  
    def delete_table(self,tab_name):
        cur = self.conn.cursor()
       
        query = "DROP TABLE " + tab_name + ";"
        
        cur.execute(query)
        return

    def select_id_sys(self, address):
        cur = self.conn.cursor()
       
        query="SELECT id_systems FROM monitored_systems WHERE address =="+ "'"+address+"'"+";"
       
        #print(query)
        cur.execute(query)
        row=cur.fetchone()

        if row is None:
            return -1

        return int(row[0])

    def get_history(self,address):
        data = []
        cur = self.conn.cursor()

        id = self.select_id_sys(address)

        cur.execute("SELECT * FROM systems_history WHERE id_sys =="+ "'"+str(id)+"'"+" ORDER BY last_time_checked DESC;")
        
        rows = cur.fetchall()
        for row in rows:
            data.append(row)
        
        return data

