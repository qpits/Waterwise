from flask import Flask
from flask import request, jsonify
from flask_openapi3 import OpenAPI, Info
from config import Config
from flask import render_template
import ssl
import requests
from DB import DB
from datetime import datetime

appname = "WaterwiseApp!"

info = Info(title=appname, version="1.0.0")
app = OpenAPI(appname, info=info,template_folder='/home/omarc/marta/templates', static_folder="/home/omarc/marta/static")

myconfig = Config
app.config.from_object(myconfig)
tempdir=''

# S->sensore    A->abitazione   T->torre acquedotto
elementi = [
    ["Duck St 134","perdita rilevata",36.115694, -97.062748,'S','null'],
    ["6th Av 102","stabile",36.115747, -97.058589,'S','null'],
    ["Maple Ave 51","stabile",36.120665, -97.061172,'S','null'],
    ["Maple Ave 30","stabile",36.120623, -97.063954,'A','null'],
    ["Maple Ave 37","stabile",36.120631, -97.062740,'A','null'],
    ["Duck St 123","perdita rilevata",36.118438, -97.062754,'A','null'],
    ["6th Av 95","stabile",36.115676, -97.060866,'A','null'],
    ["Husband St 5","stabile",36.117403, -97.059956,'A','null'],
    ["Maple Ave 58","stabile",36.120644, -97.059973,'A','null'],
    ["Lewis St 22","perdita rilevata",36.118980, -97.057280,'A','null'],
    ["6th Av 110","stabile",36.115759, -97.057268,'A','null'],
    ["Maple Ave 1","stabile",36.120623, -97.064978,'T','null']
]



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
    db.create_connection("/home/omarc/marta/database.db")
    id_sys=(db.select_id_sys(address))

    if id_sys == -1:
        text="<div> L'indirizzo fornito ( "+address+" ) non è presente nel DB. Per favore inserisci un indirizzo valido! </div>"
        return text,418
    
    if status != "stabile" and  status != "perdita rilevata":
       text="<div> Lo stato inserito ( "+status+" ) non è ammissibile. Per favore inserisci uno stato valido ('stabile' o 'perdita rilevata')! </div>"
       return text,418
    

    text="<div> address:"+address+"<br> status:"+status+"<br> last_time_checked:"+last_time_checked+"</div>"

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


    last_time_checked=str(datetime.now())

    db = DB()
    db.create_connection("/home/omarc/marta/database.db")
    id_sys=(db.select_id_sys(address))

    if id_sys == -1:
        text="<div> L'indirizzo fornito ( "+address+" ) non è presente nel DB. Per favore inserisci un indirizzo valido! </div>"
        return text,418
    
    if status != "stabile" and  status != "perdita rilevata":
       text="<div> Lo stato inserito ( "+status+" ) non è ammissibile. Per favore inserisci uno stato valido ('stabile' o 'perdita rilevata')! </div>"
       return text,418
    

    db.insert_registration(id_sys, last_time_checked, status)

    text="<div> address:"+address+"<br> status:"+status+"<br> last_time_checked:"+last_time_checked+"</div>"

    return text,200



@app.route('/showhistory',methods = ['POST'])
def show():

#   show all history of sensor status registration when the user click on history_icon in the main page near the address of sensor

    request_data = request.get_json()
    address = request_data['address']
    db = DB()
    db.create_connection("/home/omarc/marta/database.db")
    history=db.get_history(address)

    return history

@app.route('/')
def mein():
    db = DB()
    db.create_connection("/home/omarc/marta/database.db")
   
    #delete tables

    #db.delete_table("monitored_systems")
    #db.delete_table("systems_history")
    
    first_creation_monitored_systems=db.setup()


    #last_time_checked=datetime.now()
    #db.insert_registration(1, last_time_checked,'stabile')
    #db.insert_registration(1, '2022-01-16 12:31:55.682819','perdita rilevata')
    #last_time_checked=datetime.now()
    #db.insert_registration(1, '2025-02-16 12:31:55.682819','stabile')  #ultima per id_sys 1


    #db.insert_registration(5, '2024-02-16 12:31:55.682819','stabile')
    #db.insert_registration(5, '2099-02-16 12:31:55.682819','perdita rilevata') #ultima per id_sys 5
    
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


    system_list = db.get_all_systems()
    #last_history_list = db.get_last_history()

    print(system_list)


    data = {'sys':system_list}#, 'his': last_history_list}

    return render_template(tempdir + 'homepage.html', data=data)

if __name__ == '__main__':
    port = 8008
    interface = '192.168.1.71'
    
    certName='/etc/letsencrypt/live/watermelon.gitmyfruit.it/fullchain.pem'
    keyName='/etc/letsencrypt/live/watermelon.gitmyfruit.it/privkey.pem'

    context = ssl.SSLContext(ssl.PROTOCOL_TLSv1_2)
    context.load_cert_chain(certName,keyName)
    app.run(host=interface,port=port,ssl_context=context)
