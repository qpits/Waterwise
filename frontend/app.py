
from flask import Flask
from flask import request, jsonify
from flask_openapi3 import OpenAPI, Info
from config import Config
from flask import render_template,session,redirect,url_for
import ssl
import requests
from DB import DB
from datetime import datetime
import json
from markupsafe import escape
import setproctitle


appname = "WaterwiseApp!"
db_path = "./database.db"
static_path = "./static"
templates_path = "./templates"

info = Info(title=appname, version="1.0.0")
app = OpenAPI(__name__, info=info,template_folder=templates_path, static_folder=static_path)

myconfig = Config
app.config.from_object(myconfig)
tempdir=''

# S->sensore    A->abitazione   T->torre acquedotto
elementi = [
    ["Duck St 134","stabile",36.115694, -97.062748,'S','null'],
    ["6th Av 102","stabile",36.115747, -97.058589,'S','null'],
    ["Maple Ave 51","stabile",36.120665, -97.061172,'S','null'],
    ["Maple Ave 30","stabile",36.120623, -97.063954,'A','null'],
    ["001","stabile",36.120631, -97.062740,'A','null'],
    ["Duck St 123","stabile",36.118438, -97.062754,'A','null'],
    ["003","stabile",36.115676, -97.060866,'A','null'],
    ["Husband St 5","stabile",36.117403, -97.059956,'A','null'],
    ["Maple Ave 58","stabile",36.120644, -97.059973,'A','null'],
    ["002","stabile",36.118980, -97.057280,'A','null'],
    ["6th Av 110","stabile",36.115759, -97.057268,'A','null'],
    ["Maple Ave 1","stabile",36.120623, -97.064978,'T','null']
    
]




@app.route('/get-history', methods = ['POST'])

#     get all history of sensor like a list of rows in this format -> (id_sys,last_time_checked, stato) starting from:
#
#    -address -> address of sensor
#
#    sended by user

def getHistory():

    request_data = request.get_json()
    address = request_data['address']
    
    db = DB()
    db.create_connection(db_path)
    id_sys=(db.select_id_sys(address))

    if id_sys == -1:
        text="<div> L'indirizzo fornito ( "+escape(address)+" ) non è presente nel DB. Per favore inserisci un indirizzo valido! </div>"
        return text,418
    
    history=db.get_history(address)

    list_of_tuples=[]
    for  i in range(len(history)):

        # Sample list of tuples of tuples
        list_of_tuples.append((("Data", str(history[i][2])), ("Stato", str(history[i][3]))))
                    

    # Convert the list of tuples of tuples to a list of dictionaries
    list_of_dicts = [dict(inner_tuple) for inner_tuple in list_of_tuples]

    # Convert the list of dictionaries to a JSON string
    text = json.dumps(list_of_dicts, indent=2)

    return text,200


@app.route('/insert-history-test', methods = ['POST'])

#   !!THIS IS A TEST FUNCTION FOR TESTING THE CORRECT OPERATION OF REQUEST BY USERS!!

#     insert a row in sistems_history (id_sys,last_time_checked, stato) starting from:
#
#    -address -> address of sensor
#    -status  -> status of sensor (stabile or perdita rilevata)
#
#    sended by user

def insert2():

    request_data = request.get_json()
    address = request_data['address']
    status = request_data['status']

    last_time_checked=str(datetime.now())

    db = DB()
    db.create_connection(db_path)
    id_sys=(db.select_id_sys(address))

    if id_sys == -1:
        text="<div> L'indirizzo fornito ( "+escape(address)+" ) non è presente nel DB. Per favore inserisci un indirizzo valido! </div>"
        return text,418
    
    if status != "stabile" and  status != "perdita rilevata":
       text="<div> Lo stato inserito ( "+escape(status)+" ) non è ammissibile. Per favore inserisci uno stato valido ('stabile' o 'perdita rilevata')! </div>"
       return text,418
  

    text="<div> address:"+escape(address)+"<br> status:"+escape(status)+"<br> last_time_checked:"+last_time_checked+"</div>"

   

    return text,200


@app.route('/insert-history', methods = ['POST'])

#     insert a row in sistems_history (id_sys,last_time_checked, stato) starting from:
#
#    -address -> address of sensor
#    -status  -> status of sensor (stabile or perdita rilevata)
#
#    sended by user

def insert():

    request_data = request.get_json()
    address = request_data['address']
    status = request_data['status']

    app.logger.debug('received address: %s, status: %s', address, status)


    last_time_checked=str(datetime.now())

    db = DB()
    db.create_connection(db_path)
    id_sys=(db.select_id_sys(address))

    if id_sys == -1:
        text="<div> L'indirizzo fornito ( "+escape(address)+" ) non è presente nel DB. Per favore inserisci un indirizzo valido! </div>"
        return text,418
    
    if status != "stabile" and  status != "perdita rilevata":
       text="<div> Lo stato inserito ( "+escape(status)+" ) non è ammissibile. Per favore inserisci uno stato valido ('stabile' o 'perdita rilevata')! </div>"
       return text,418
    

    db.insert_registration(id_sys, last_time_checked, status)
        
    text="<div> address:"+escape(address)+"<br> status:"+escape(status)+"<br> last_time_checked:"+last_time_checked+"</div>"

    return text,200   

#     FATTO ORA

@app.route('/response/<address>', methods=['GET'])
def get_status(address):
        
        db = DB()
        db.create_connection(db_path)

        id_sys=(db.select_id_sys(address))

        if id_sys == -1:
            text="<div> L'indirizzo fornito ( "+escape(address)+" ) non è presente nel DB. Per favore inserisci un indirizzo valido! </div>"
            return text,418
        
        history=db.get_history(address)
        print(history)
        status=history[0][3]

        # se è stata inserita una rilevazione
        # 0 = stabile--> NON accendere il led
        # 1 = perdita rilevata--> accendere il led

        text='0'

        if status == 'perdita rilevata':
            text='1'

        return text,200



@app.route('/showhistory',methods = ['POST'])
def show():

#   show all history of sensor status registration when the user click on history_icon in the main page near the address of sensor

    request_data = request.get_json()
    address = request_data['address']
    db = DB()
    db.create_connection(db_path)
    history=db.get_history(address)

    return history

@app.route('/')
def mein():
    db = DB()
    db.create_connection(db_path)

    system_list = db.get_all_systems()
    #last_history_list = db.get_last_history()

    #print(system_list)


    data = {'sys':system_list}#, 'his': last_history_list}

    return render_template(tempdir + 'homepage.html', data=data)

def setup_db():
    db = DB()
    db.create_connection(db_path)
    db.delete_table("monitored_systems")
    db.delete_table("systems_history")
    
    first_creation_monitored_systems=db.setup()

    
    if not first_creation_monitored_systems : #se è la prima volta che creo la tabella, inserisco i valori dei sensori
        for s in range(len(elementi)):
            tipo=''
            if(elementi[s][4]=='S'):
                tipo='Sensore'
            if(elementi[s][4]=='A'):
                tipo='Abitazione'
            if(elementi[s][4]=='T'):
                tipo='Torre acquedotto'

            db.insert_sensor(elementi[s][0],tipo,elementi[s][2],elementi[s][3],elementi[s][5])
        
        #per prova (solo la prima volta che creo le tabelle) popolo la tabella systems_history riportando lo stato presente negli elenchi soprastanti
        for s in range(len(elementi)):
            id_sys=db.select_id_sys(elementi[s][0])
            last_time_checked=datetime.now()
            db.insert_registration(id_sys, last_time_checked,elementi[s][1])

if __name__ == '__main__':
    #setproctitle.setproctitle('waterwise-server')
    port = 8008
    interface = '192.168.1.71'
    
    certName='/etc/letsencrypt/live/watermelon.gitmyfruit.it/fullchain.pem'
    keyName='/etc/letsencrypt/live/watermelon.gitmyfruit.it/privkey.pem'

    context = ssl.SSLContext(ssl.PROTOCOL_TLSv1_2)
    context.load_cert_chain(certName,keyName)
    setup_db()
    app.run(host=interface,port=port,ssl_context=context)
