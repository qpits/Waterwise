# Waterwise

**Introduzione**

/insert-history

Questa API permette di inserire una nuova lettura nello storico di uno specifico sensore.

**URL**

https://watermelon.gitmyfruit.it/insert-history

**URL di prova**

https://watermelon.gitmyfruit.it/insert-history-test

!Questo url non inserirà NULLA nel database ma permette di provare se effettivamente la chiamata API è andata a buon fine o no!

**Metodo**

POST

**Richiesta**

* Headers

```
Content-Type: application/json 
```

* Body 
```
JSON

{
  "address": <string>,
  "status": <string>
}
```

Usa il codice con cautela.

**Parametri:**

* address: indirizzo del sensore, la lista degli indirizzi è a seguire
* status: status del sensore, lista degli stati a seguire


**Risposta**

* Successo:

```
HTML

<div> 
    address:6th Av 110 
    <br> 
    status:perdita rilevata
    <br> 
    last_time_checked:2024-02-28 15:43:12.005911
</div>
```


* Errore:


1) Erorre causato da un indirizzo errato non presente nel DB
```
HTML

<div> 
    L'indirizzo fornito ( 'address inserito' ) non è presente nel DB. 
    Per favore inserisci un indirizzo valido! 
</div>
```

2) Erorre causato da uno stato non valido
```
HTML

<div> 
    Lo stato inserito ('stato inserito' ) non è ammissibile. 
    Per favore inserisci uno stato valido ('stabile' o 'perdita rilevata')! 
</div>
```

* Codici di errore:

    418: I'm a teapot - Parametri errati

    500: Internal Server Error - Errore interno del server

**Esempio**

```
curl -X POST \
  -H "Content-Type: application/json" \
  -d '{
      "address":"6th Av 110",
     "status":"perdita rilevata"
    }' \
  https://watermelon.gitmyfruit.it/insert-history
```

**Lista indirizzi validi**

* Duck St 134
* 6th Av 102
* Maple Ave 51
* Maple Ave 30
* Maple Ave 37
* Duck St 123
* 6th Av 95
* Husband St 5
* Maple Ave 58
* Lewis St 22
* 6th Av 110
* Maple Ave 1


**Lista stati validi**

* stabile
* perdita rilevata

  

**Note**

Assicurarsi di sostituire i valori dei parametri nella richiesta con indirizzi presenti nella lista.
In caso di errori, controllare la descrizione dell'errore nella risposta per risolvere il problema.
