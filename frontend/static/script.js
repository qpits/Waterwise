/*contatori per slide down e slide up quando si preme il bottone mappa sensori*/
i = 150;
l = 0;
var map;

$( document ).ready(function() {

    //Faccio il parsing degli elementi passati
    received_data = JSON.parse(received_data_unparsed)


    class CustomAlert {
        #idHook
        #title
        #text
        #body
    
        constructor(buttonIdHook, title, text, body) {
            this.#idHook = buttonIdHook;
            this.#title = title;
            this.#text = text;
            this.#body = body;
        }
    
        printCode() {
            document.getElementById('modals').innerHTML +=
                '<div class="modal fade" style="z-index:9999"" id="' + this.#idHook + '"  tabindex="-1" role="dialog" aria-labelledby="exampleModalScrollableTitle" aria-hidden="true">\n' +
                '  <div class="modal-dialog modal-dialog-scrollable" role="document">\n' +
                '    <div class="modal-content" style="width: 100% !important;">\n' +
                '      <div class="modal-header">\n' +
                '        <h5 class="modal-title">' + this.#title + '</h5>\n' +
                '        <button type="button" class="close" data-dismiss="modal" aria-label="Close">\n' +
                '          <span aria-hidden="true">&times;</span>\n' +
                '        </button>\n' +
                '      </div>\n' +
                '      <div class="modal-body">\n' +
                '        <p >' + this.#text + '</p>\n' +
                '      </div>\n' +
                '      <div class="modal-body";>\n' +
                '        <p id="modal-history-title"> System History </p>\n'+
                '        <p id="modal-history-body">' + this.#body + '</p>\n' +    
                '      </div>\n' +
                '    </div>\n' +
                '  </div>\n' +
                '</div>\n'
            ;
    
            let close = $('#option' + this.#idHook + '_close');
            close.on('click', () => {
                $('#' + this.#idHook).remove()
                $('.modal-backdrop').hide()
            });
        }
    
        showAlert() {
            $('#' + this.#idHook).modal('show')
            $('#' + this.#idHook).show();
        }
    }


    $('#go_to_page').on('click', function(){
        $('#sottotitolo').slideUp();
        const x = recursive().then(() => doOtherThings());
        i = 150;
    })

    function doOtherThings(){
        
        slideUpShow($('#data_row'));
        
        setTimeout(function(){
            if(map == undefined){
                map = L.map('map').setView([36.117941, -97.059951], 16.5);
                const googleSat = L.tileLayer('https://{s}.google.com/vt/lyrs=m&x={x}&y={y}&z={z}', {
                        maxZoom: 25,
                        subdomains: ['mt0', 'mt1', 'mt2', 'mt3']
                }).addTo(map)
                appendElements();
            }
            
        },400);
        

        $('.titolo').animate({
            'opacity' : 0
        }, 400, function(){
            $(this).html('BACK').animate({'opacity': 1}, 700);});
        
        $('.titolo').css('cursor','pointer');

    }

    $('.titolo').on('click', function(){

        if($('#sottotitolo').is(":hidden")){

            slideDown2($('#data_row'));

            $('.titolo').animate({
                'opacity' : 0
            }, 400, function(){
                $(this).html('WATERWISE').animate({'opacity': 1}, 400);});


            $('#sottotitolo').slideDown('slow');
            recursiveBack();
            l = 0;
            $('.titolo').css('cursor','default');
        }
        
    });

    async function recursive(){
        setTimeout(function(){
            $('#page_title').css('margin-top',i);
            i--;    
            if (i > 0) recursive()
        }, 2)
    }

    function recursiveBack(){
        setTimeout(function(){
            $('#page_title').css('margin-top',l);
            l++; 
            if (l < 150 ) recursiveBack()
        }, 2)
    }

    function slideUpShow($ele) {
        var height = $ele.height() + 'px';
        $ele.css({
            'margin-top': height,
            'height': height,
            'overflow': 'hide'
        }).show().animate({
            'marginTop': '0px',
            'height': 'unset',
            'overflow': 'unset'
        }, 1200);
    }


    function slideDown2($ele) {
        var height = $ele.height() + 'px';
        $ele.css({
            'margin-top': '0px',
            'height': 'unset',
            'overflow': 'unset'
        }).show().animate({
            'marginTop': height,
            'height': height,
            'overflow': 'hide'
        }, 700);
    }


    let elementi = received_data['sys']
    
    /*[
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
    ]*/

    function appendElements(){

        /*add elementi to map and list*/
        var icon_sensore_stabile = L.icon({
            iconUrl: 'static/icon_sensore_stabile.png',
            iconSize:[40,40],
            iconAnchor:[25,25]
        });
        var icon_sensore_perdita = L.icon({
            iconUrl: 'static/icon_sensore_perdita.png',
            iconSize:[40,40],
            iconAnchor:[25,25]
        });
        var icon_casa_stabile = L.icon({
            iconUrl:'static/icon_casa_stabile.png',
            iconSize:[40,40],
            iconAnchor:[25,25]
        });
        var icon_casa_perdita = L.icon({
            iconUrl:'static/icon_casa_perdita.png',
            iconSize:[40,40],
            iconAnchor:[25,25]
        });
        var icon_torre_acqua = L.icon({
            iconUrl: 'static/icon_torre_acqua.png',
            iconSize:[40,40],
            iconAnchor:[25,25]
        });

        //console.log(elementi.length)

        for(let k = 0;k < elementi.length; k++){
            
            let id_elem = stringToHash(elementi[k][1].toString());
            let tipo='';
            let i='';
            let m='iconNo.png';
            var li = document.createElement("li");

            if(elementi[k][2]=='Sensore'){

                tipo='Sens'
                li.className = 'list-group-item sensore';
                i = icon_sensore_perdita;
                if(elementi[k][6] == 'stabile'){
                    i = icon_sensore_stabile
                    m = "iconOk.png"
                }
            }
            if(elementi[k][2]=='Abitazione'){

                tipo='Casa'
                li.className = 'list-group-item abitazione';
                i = icon_casa_perdita;
                if(elementi[k][6] == 'stabile'){
                    i = icon_casa_stabile
                    m = "iconOk.png"
                }
            }
            if(elementi[k][2]=='Torre acquedotto'){

                tipo='Torre'
                li.className = 'list-group-item torre';
                i = icon_torre_acqua;
                m = "iconOk.png"    
            }

            li.innerHTML= `<div class="row">
            <div class="col-sm-5 list_name focusPin" id="${id_elem}">${elementi[k][1]}</div>
            <div class="col-sm-2"> <img id="h_${id_elem}" class="history_icon" src='static/iconHistory.png' style="width:24px;height:24px;"></div>
            <div class="col-sm-3">${tipo}</div>
            <div class="col-sm-2"><img src='static/${m}' style="width:24px;height:24px;"</div>
            </div> `;

            document.getElementById('lista_elementi').appendChild(li);
            var marker = L.marker([elementi[k][3], elementi[k][4]],{uniqueID: id_elem, icon:i}).addTo(map)
            .bindPopup(elementi[k][1]+": "+elementi[k][2]);
        }
        
        
        drawPolilinea()
    }

    function drawPolilinea(){
        
        /*polilinea tra i vari elementi*/
        let color='';
        let rosso="#d75c3b";
        let verde="#1da65e";
        /*torre acquedotto - casa1*/
        if(elementi[3][6]=='perdita rilevata'){
            color=rosso;
        }
        else{
            color=verde;
        }
        var path = L.polyline(   
            [[elementi[3][3],elementi[3][4]],
            [elementi[11][3],elementi[11][4]]],
            {"delay":400,"weight":3,"color":color,"paused":true,"reverse":false}
        ).addTo(map);
                
        /*casa 1- casa2*/
        if(elementi[3][6]=='perdita rilevata' || elementi[4][6]=='perdita rilevata'){
            color=rosso;
        }
        else {
            color=verde;
        }    
        var path = L.polyline(   
            [[elementi[3][3],elementi[3][4]],
            [elementi[4][3],elementi[4][4]]],
            {"delay":400,"weight":3,"color":color,"paused":true,"reverse":false}
        ).addTo(map);

        /*casa 2- casa3*/
        if(elementi[4][6]=='perdita rilevata' || elementi[5][6]=='perdita rilevata'){
           
        }
        else{
            color=verde;
        }
        var path = L.polyline(   
            [[elementi[4][3],elementi[4][4]],
            [elementi[5][3],elementi[5][4]]],
            {"delay":400,"weight":3,"color":color,"paused":true,"reverse":false}
        ).addTo(map);
              
        /*casa 3- sensore 1*/
        if(elementi[5][6]=='perdita rilevata' || elementi[0][6]=='perdita rilevata'){
            color=rosso;
        }
        else{
            color=verde;
        }
        var path = L.polyline(   
            [[elementi[5][3],elementi[5][4]],
            [elementi[0][3],elementi[0][4]]],
            {"delay":400,"weight":3,"color":color,"paused":true,"reverse":false}
        ).addTo(map);
     
        /*casa 2- sensore 3*/
        if(elementi[4][6]=='perdita rilevata' || elementi[2][6]=='perdita rilevata'){
            color=rosso;
        }
        else{
            color=verde;
        }
        var path = L.polyline(   
            [[elementi[4][3],elementi[4][4]],
            [elementi[2][3],elementi[2][4]]],
            {"delay":400,"weight":3,"color":color,"paused":true,"reverse":false}
        ).addTo(map);  
        
        /*sensore3- casa 6*/
        if( elementi[2][6]=='perdita rilevata' ||elementi[8][6]=='perdita rilevata'){
            color=rosso;
        }
        else{
            color=verde;
        }
        var path = L.polyline(   
            [[elementi[2][3],elementi[2][4]],
            [elementi[8][3],elementi[8][4]],],
            {"delay":400,"weight":3,"color":color,"paused":true,"reverse":false}
        ).addTo(map);
    
        /*casa 6- casa 5*/
        if(elementi[8][6]=='perdita rilevata' || elementi[7][6]=='perdita rilevata'){
            color=rosso;
        }
        else{
            color=verde;
        }
        var path = L.polyline(   
            [[elementi[8][3],elementi[8][4]],
            [elementi[7][3],elementi[7][4]]],
            {"delay":400,"weight":3,"color":color,"paused":true,"reverse":false}
        ).addTo(map);

        /*sensore1- casa 4*/
        if( elementi[0][6]=='perdita rilevata' ||elementi[6][6]=='perdita rilevata'){
            color=rosso;
        }
        else{
            color=verde;
        }
        var path = L.polyline(   
            [[elementi[0][3],elementi[0][4]],
            [elementi[6][3],elementi[6][4]],],
            {"delay":400,"weight":3,"color":color,"paused":true,"reverse":false}
        ).addTo(map);
             
        /*casa 4 - 36.115741, -97.059947*/
        if( elementi[6][6]=='perdita rilevata' ||elementi[1][6]=='perdita rilevata'){
            color=rosso;
        }
        else{
            color=verde;
        }
        var path = L.polyline(   
            [[elementi[6][3],elementi[6][4]],
            [36.115741, -97.059947]],
            {"delay":400,"weight":3,"color":color,"paused":true,"reverse":false}
        ).addTo(map);

        /*casa 5 - 36.115741, -97.059947*/
        if( elementi[7][6]=='perdita rilevata' ||elementi[1][6]=='perdita rilevata'){
            color=rosso;
        }
        else{
            color=verde;
        }
        var path = L.polyline(   
            [[elementi[7][3],elementi[7][4]],
            [36.115741, -97.059947]],
            {"delay":400,"weight":3,"color":color,"paused":true,"reverse":false}
        ).addTo(map);
                  
        /* 36.115741, -97.0599472- sensore 2*/
        if(elementi[6][6]=='perdita rilevata' || elementi[1][6]=='perdita rilevata'){
            color=rosso;
        }
        else{
            color=verde;
        }
        var path = L.polyline(   
            [[36.115741, -97.059947],
            [elementi[1][3],elementi[1][4]]],
            {"delay":400,"weight":3,"color":color,"paused":true,"reverse":false}
        ).addTo(map);

        /*sensore2- casa 8*/
        if( elementi[1][6]=='perdita rilevata' ||elementi[10][6]=='perdita rilevata'){
            color=rosso;
        }
        else{
            color=verde;
        }
        var path = L.polyline(   
            [[elementi[1][3],elementi[1][4]],
            [elementi[10][3],elementi[10][4]],],
            {"delay":400,"weight":3,"color":color,"paused":true,"reverse":false}
        ).addTo(map);

        /*casa 8- casa 7*/
        if(elementi[10][6]=='perdita rilevata' || elementi[9][6]=='perdita rilevata'){
            color=rosso;
        }
        else{
            color=verde;
        }
        var path = L.polyline(   
            [[elementi[10][3],elementi[10][4]],
            [elementi[9][3],elementi[9][4]]],
            {"delay":400,"weight":3,"color":color,"paused":true,"reverse":false}
        ).addTo(map);

        /*casa 7 - 36.120653, -97.057312*/
        if( elementi[9][6]=='perdita rilevata' ||elementi[8][6]=='perdita rilevata'){
            color=rosso;
        }
        else{
            color=verde;
        }
        var path = L.polyline(   
            [[elementi[9][3],elementi[9][4]],
            [36.120653, -97.057312]],
            {"delay":400,"weight":3,"color":color,"paused":true,"reverse":false}
        ).addTo(map);

        /* 36.120653, -97.057312 - casa 6*/
        if( elementi[8][6]=='perdita rilevata' ||elementi[9][6]=='perdita rilevata'){
            color=rosso;
        }
        else{
            color=verde;
        }
        var path = L.polyline(   
            [[elementi[8][3],elementi[8][4]],
            [36.120653, -97.057312]],
            {"delay":400,"weight":3,"color":color,"paused":true,"reverse":false}
        ).addTo(map);
                     
    }


    L.Map.include({
        getMarkerById: function (id) {
            var marker = null;
            this.eachLayer(function (layer) {
                if (layer instanceof L.Marker) {
                    if (layer.options.uniqueID == id) {
                        marker = layer;
                    }
                }
            });
            return marker;
        }
    });

    function stringToHash(string) {
        let hash = 0;
        if (string.length == 0) return hash;
        for (i = 0; i < string.length; i++) {
            char = string.charCodeAt(i);
            hash = ((hash << 5) - hash) + char;
            hash = hash & hash;
        }
        return hash;
    }

    function getData(hashed_id){
        for(k = 0; k < elementi.length; k++){
            if(stringToHash(elementi[k][1]) == hashed_id){
                return elementi[k]
            }
        }
    }


    $('#lista_elementi').on('click','.focusPin', function() {
        let marker = map.getMarkerById($(this).attr('id'));
        map.flyTo(marker._latlng,20);

        if(document.body.clientWidth <= 600){
            $('html, body').animate({
                scrollTop: $("#map").offset().top
            }, 1500);
        }
    })


    $('#lista_elementi').on('click','.history_icon', function(){

        id = $(this).attr('id');

        id = id.slice(2);

        data = getData(id);
        //console.log("data:  "+data)

        result = post_call_show_history(data[1]);

        //console.log(result)

        body=new Array();

        for(let i = 0; i < result.length; i++){
            element=result[i][2]+' / '+result[i][3]+'<br>';
            body.push(element);
        }
        
        body=body.join(' ')
        //console.log('body: '+ body+"\nbody type: "+typeof(body))
        tipo='';
        if(data[2]=='Sensore') tipo='Sensore';
        else if (data[2]=='Abitazione') tipo='Casa';
        else if (data[2]=='Torre acquedotto') tipo='Torre acquedotto';
        
        text ="Sensor Type: "+tipo+"<br>"+ "Description: "+data[5]+"<br>"+"Lat: "+data[3]+"<br>Lon: "+data[4];

        test = new CustomAlert("modal_"+id,data[1], text,body)
        test.printCode();
        test.showAlert(); 
       
    })

    function post_call_show_history(data)
    {

        json = {
            "address": (data)
          }

        var result; 
        
        $.ajax({
            type: "POST",
            url: "/showhistory",
            data: JSON.stringify(json),
            headers: {
                'Content-type':'application/json', 
                'Accept':'application/json'
            },    
            async: false,
            //dataType: "text",
            success: function(r){
                console.log(r);
                result=r;
            },
          });
         
        //console.log('result from post_call_test: '+ result);  
        return result;

        
        
/*
        json = {
            "email": "eve.holt@reqres.in", 
            "password": "pistol"
          }

        
        $.ajax({
            type: "POST",
            url: "/test",
            data: JSON.stringify(json),
            headers: {
                'Content-type':'application/json', 
                'Accept':'application/json'
            },    
            success: function(r){
                console.log(r)
            },
            dataType: "text"
          });
*/
    }




    $("#resize_icon").on("click",function(){
        map.flyTo([36.117941, -97.059951], 16.5);
    })

    //filtraggio degli elementi nella mappa

    current_type = "";
    current_filter = "";

    $("#test_filtro").on("keyup", function() {
        
        current_filter = $(this).val().toLowerCase();

        if(current_filter == "" && current_type != ""){
        $("#lista_elementi li").filter(function() {
            $(this).toggle($(this).hasClass(current_type))
        });
        return
        }

        $("#lista_elementi li").filter(function() {
        if(current_type != ""){
            $(this).toggle(($(this).text().toLowerCase().indexOf(current_filter) > -1) && $(this).hasClass(current_type))

        } else {
            $(this).toggle(($(this).text().toLowerCase().indexOf(current_filter) > -1))

        }
        });
    });

    $("#test_tipo").on("change", function() {

        current_type = this.value;
        if(current_type == ""){
        $("#lista_elementi li").filter(function() {

            $(this).toggle($(this).text().toLowerCase().indexOf(current_filter) > -1)
        });
            return
        }
        
        $("#lista_elementi li").filter(function() {
        
            $(this).toggle(($(this).text().toLowerCase().indexOf(current_filter) > -1) && $(this).hasClass(current_type))
        });

    });

    $('#delete_filters').on('click', function (){

        $('#test_tipo').val("");
        $('#test_filtro').val("");

        current_type = "";
        current_filter = "";


        $("#lista_elementi li").filter(function() {
                $(this).toggle(true)
            });
    });

});