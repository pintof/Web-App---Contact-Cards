// Put all onload AJAX calls here, and event listeners

var currentCard = {};
var fileLogPanel = document.getElementById("fileLogPanel");
var cardViewPanel = document.getElementById("cardViewPanel");
var cardDD = document.getElementById("cardDD");
var createCard = document.getElementById("createCard");
var queryTable = document.getElementById("queryTable");

var queryObj;

var parentTemp;

var acceptedNames = ["SOURCE","KIND","XML","FN","N","NICKNAME","PHOTO","GENDER","ADR",
    "TEL","EMAIL","IMPP","LANG","TZ","TITLE","ROLE","LOGO","ORG","MEMBER","RELATED","GEO",
    "CATEGORIES","NOTE","PRODID","REV","SOUND","UID","CLIENTPIDMAP","URL","KEY","FBURL",
    "CALADRURI","CALURI"];


$(document).ready(function() {

    //Hide & Populate subquery radio boxes
    $('#query1').click(function(e){
        hideAllSubquery();
        document.getElementById("subquery1").hidden = false;
    });

    $('#query2').click(function(e){
        hideAllSubquery();
    });

    $('#query3').click(function(e){
        hideAllSubquery();
        document.getElementById("subquery3").hidden = false;
    });

    $('#query4').click(function(e){
        hideAllSubquery();
        document.getElementById("subquery4").hidden = false;
    });

    $('#query5').click(function(e){
        hideAllSubquery();
        document.getElementById("subquery5").hidden = false;
    });

    $('#executeQuery').click(function(e){
        var query;
        var queryType = 0;
        
        //Query 1
        if($("input[id='query1']:checked").val() != null && $("input[id='sortQuery1Name']:checked").val() != null){
            query = "select file_name, num_props, name, birthday, anniversary, creation_time from FILE order by name";
            queryType = 1;
        }else if($("input[id='query1']:checked").val() != null && $("input[id='sortQuery1Filename']:checked").val() != null){
            query = "select file_name, num_props, name, birthday, anniversary, creation_time from FILE order by file_name";
            queryType = 1;
        }

        //Query 2
        if ($("input[id='query2']:checked").val() != null){
            query = "select name, birthday from FILE order by birthday"; //where birthday is not null
            queryType = 2;
        }

        //Query 3
        if ($("input[id='query3']:checked").val() != null && $("input[id='sortQuery3Name']:checked").val() != null){
            query = "SELECT temp1.name, temp1.anniversary FROM FILE as temp1, FILE as temp2 where temp1.name <> temp2.name and temp1.anniversary = temp2.anniversary order by name";
            queryType = 3;
        }else if ($("input[id='query3']:checked").val() != null && $("input[id='sortQuery3Anniversary']:checked").val() != null){
            query = "SELECT temp1.name, temp1.anniversary FROM FILE as temp1, FILE as temp2 where temp1.name <> temp2.name and temp1.anniversary = temp2.anniversary order by anniversary";
            queryType = 3;
        }

        //Query 4
        if ($("input[id='query4']:checked").val() != null && $("input[id='sortQuery4Name']:checked").val() != null){
            var startDate = parseDateTimeString(document.getElementById("startDate").value.replace(/-/g,"").replace(":","")+"00");
            var endDate = parseDateTimeString(document.getElementById("endDate").value.replace(/-/g,"").replace(":","")+"00");
            console.log(startDate);
            query = "select file_name, num_props, name, birthday, anniversary, creation_time from FILE where creation_time between '" + startDate + "' and '" + endDate+"'" + " order by name";
            queryType = 4;
        }else if ($("input[id='query4']:checked").val() != null && $("input[id='sortQuery4Filename']:checked").val() != null){
            var startDate = parseDateTimeString(document.getElementById("startDate").value.replace(/-/g,"").replace(":","")+"00");
            var endDate = parseDateTimeString(document.getElementById("endDate").value.replace(/-/g,"").replace(":","")+"00");
            console.log(startDate);
            query = "select file_name, num_props, name, birthday, anniversary, creation_time from FILE where creation_time between '" + startDate + "' and '" + endDate+"'" + " order by file_name";
            queryType = 4;
        }else if ($("input[id='query4']:checked").val() != null && $("input[id='sortQuery4CreationDate']:checked").val() != null){
            var startDate = parseDateTimeString(document.getElementById("startDate").value.replace(/-/g,"").replace(":","")+"00");
            var endDate = parseDateTimeString(document.getElementById("endDate").value.replace(/-/g,"").replace(":","")+"00");
            console.log(startDate);
            query = "select file_name, num_props, name, birthday, anniversary, creation_time from FILE where creation_time between '" + startDate + "' and '" + endDate+"'" + " order by creation_time";
            queryType = 4;
        }

        //Query 5
        if ($("input[id='query5']:checked").val() != null && $("input[id='sortQuery5DownloadCount']:checked").val() != null){
            var N = document.getElementById("N").value;
            if (N > 0){
                queryType = 5;
                query = "SELECT COUNT(DOWNLOAD.download_id) AS download_counts, MAX(DOWNLOAD.download_time) as most_recent_download_date, DOWNLOAD.d_descr as download_description, FILE.file_name, FILE.name FROM DOWNLOAD, FILE WHERE DOWNLOAD.file_id = FILE.file_id GROUP BY FILE.file_name ORDER BY download_counts DESC LIMIT "+N;
            }else{
                alert("Invalid Integer Entry for N");
            }
        }else if ($("input[id='query5']:checked").val() != null && $("input[id='sortQuery5Filename']:checked").val() != null){
            var N = document.getElementById("N").value;
            if (N > 0){
                queryType = 5;
                query = "select * from (SELECT COUNT(DOWNLOAD.download_id) AS download_counts, MAX(DOWNLOAD.download_time) as most_recent_download_date, DOWNLOAD.d_descr as download_description, FILE.file_name, FILE.name FROM DOWNLOAD, FILE WHERE DOWNLOAD.file_id = FILE.file_id GROUP BY FILE.file_name ORDER BY download_counts DESC LIMIT " +N+ ") as TEMP order by TEMP.file_name";
            }else{
                alert("Invalid Integer Entry for N");
            }
        }

        $.ajax({
            type: 'get',
            data: {
                'query' : query
            },
            dataType: 'json',
            url: '/executeQuery/',
            success: function(data){
                console.log(data);
                queryObj = data;

                if(data.length == 0){
                    return;
                }

                var cols = Object.keys(data[0]);

                $("#queryTable tr").remove();

                var row = queryTable.insertRow(0);
                
                for(var i = 0; i < cols.length; i++){
                    row.insertCell(i).innerHTML = cols[i];
                }

                for(var i = 0; i < data.length; i++){
                    var row = queryTable.insertRow(i+1);
                    for(var j = 0; j < cols.length; j++){
                        row.insertCell(j).innerHTML = Object.values(data[i])[j];
                    }
                }

            }, 
            fail: function(error){
                console.log(error);
            }
        });

    });


    $('#dbStatus').click(function(e){
        displayStatus();
    });

	$('#submitLogin').click(function(e){
		var user = $("#user").val();
		var pass = $("#pass").val();
		var dbname = $("#dbname").val();
		$.ajax({
			type: 'get',
			data: {
				'userName' : user,
				'password' : pass,
				'database' : dbname
			},
			dataType: 'json',
            url: '/login/',
            cache: false,
			success: function(data){
                console.log(data);
				if(data.type != 'ok'){
					alert("Error in login. Please try again");
				}else{
					alert("SUCCESS!");
                    $('#login').modal('hide');
                    //document.getElementById("showModal").hidden = false;
				}
			}, 
            error: function(err){
                alert("Error: " + err.responseText);
            }
		});
	});


    $("#addProp").submit(function(e) {
        e.preventDefault();
        if(Object.keys(currentCard).length == 0){
            alert("Can't add property to no file!");
            return;
        }

        fileName = cardDD.value;

        var propName = document.getElementById("propName").value.trim();
        var propVal = document.getElementById("propVal").value.trim();

        if( propName.length == 0 || propVal.length == 0){
            alert("Both property name and value are required!");
            return;
        }

        var newProp = {}
        //{"group":"prop group","name":"prop name","values":["val1","val2",…,"valN"]}
        newProp.group = "";
        newProp.name = propName;
        newProp.values = [propVal];

        if(acceptedNames.includes(propName) == false){
            alert("INVALID PROPERTY NAME: " + propName);
            return;
        }

        $.ajax({
            type: 'GET',
            dataType: 'text',
            url: '/addProp/' + fileName,
            data: {
                newJson: JSON.stringify(newProp),
            }, success: function(data){
                loadFiles();
                alert("Property successfully added!");
                var query = "UPDATE FILE set num_props=num_props+1 WHERE file_name = '"+ fileName+"' ";
                sendQuery(query);
            }, 
            error: function(err){
                alert("Error: " + err.responseText);
            }
        }); 
    });


    $("#createCard").submit(function(e) {
        e.preventDefault();
        var fileName = document.getElementById("fileVal").value;
        if(fileName.length <= 4){
            alert("File name too short, or missing extension");
            return;
        }
        var fnObj = {}
        fnVal = document.getElementById("fnVal").value;
        if(fnVal.length == 0){
            alert("FN can not be empty!");
            return;
        }
        fnObj.FN = fnVal;
        var propertyParse = parseProperties();

        if(typeof propertyParse === 'string' || propertyParse instanceof String){
            alert(propertyParse);
            return;
        }

        var propObj = propertyParse.addProps;
        var birthObj = propertyParse.birth;
        var anniObj = propertyParse.anni;

        $.ajax({
            type: 'GET',
            dataType: 'text',
            url: '/create/' + fileName,
            data: {
                fn: JSON.stringify(fnObj),
                props: JSON.stringify(propObj),
                birth: JSON.stringify(birthObj),
                anni: JSON.stringify(anniObj)
            }, success: function(data){
                loadFiles();
                alert("File successfully created!");
                storeFiles();
            }, 
            error: function(err){
                alert("Error: " + err.responseText);
            }
        }); 
    });

    var template = '<div class="input-group"><input type="text" class="form-control"/></div>';
    var minusButton = '<span class="btn input-group-addon delete-field">(-)</span>';

    $("#uploadForm").submit(function(e) {
        e.preventDefault();
        var formData = new FormData();
//YYYY-MM-DD HH:MM:SS.ffffff //INSERT INTO t1 VALUES ("2012-04-19 13:08:22"),
        var toUpload = document.getElementById("fileToBeUploaded");
        if(toUpload.value == ""){
            return;
        }
        var obj = new Object();
        obj.uploadFile = toUpload.files[0]; 
        formData.append("uploadFile", obj.uploadFile);

        let url = "/getObj/" + toUpload.files[0].name;
        $.ajax({
            type: 'get',           
            dataType: 'json',   
            url: url, 
            success: function (data) {
                if(data.length != 1){
                    $.ajax({
                        type : 'post',
                        url : '/upload',
                        data: formData,
                        processData: false,
                        contentType: false,
                        success: function(data){
                            $.ajax({
                                type: 'get',           
                                dataType: 'json',   
                                url: url, 
                                success: function (data) {
                                    if(data.length == 1){
                                        alert("Success! File was uploaded successfully");
                                        loadFiles();
                                        storeFiles();
                                    } else{
                                        $.ajax({
                                            type: 'delete',           
                                            dataType: 'json',   
                                            url: '/'+ toUpload.files[0].name
                                        });
                                        alert("Error! File is not valid.");
                                    }
                                }, 
                                fail: function(err) {
                                    alert("Error! File was not uploaded successfully");
                                }
                            });
                        },
                        fail: function(error) {
                            alert("Error! File was not uploaded successfully");
                        }
                    });
                } else{
                    alert("Error! File already exist on server (consider renaming).");
                }
            }, 
            fail: function(err) {
                alert("Error reading current files");
            }
        });     

    });

    $("#cardDD").change(function() {
        updateCardTable();
    });


    $("#store").click(function(e){
        storeFiles();
    });

    $('#clear').click(function(e){
        //document.getElementById("clear").hidden = true;
        deleteFiles();
        setTimeout(displayStatus(), 1000);
    });
    

});

function storeFiles(){
    // sendQuery("Delete from FILE;");
    $.ajax({
        type: 'get',
        dataType: 'json',
        url:'/uploads/',
        success: function(data){
            for(var i = 0; i < data.length; i++){ //CAST('20171221' AS DATETIME);
                if(data[i].hasOwnProperty('filePath')){					
                    //document.getElementById("clear").hidden = false;		
                    var sqlStatement = "INSERT INTO FILE (file_Name, num_props, name, birthday, anniversary, creation_time)";
                    sqlStatement += " SELECT * FROM (SELECT '" + data[i].fileName + "','" + data[i].propNum + "','" + data[i].fn + " ', CAST('" + parseDateTime(data[i].birth) + "' AS DATETIME) as birthday, CAST('" + parseDateTime(data[i].anni) + "' AS DATETIME) as anniversary, NOW()) as tmp"
                    sqlStatement +=  " WHERE NOT EXISTS (SELECT file_name FROM FILE WHERE file_name = '" + data[i].fileName + "') LIMIT 1 "
                    // sqlStatement += " VALUES ('"+ data[i].fileName + "','" + data[i].propNum + "','" + data[i].fn + " ', NULL, NULL, NOW())  "
                    sendQuery(sqlStatement);
                } else{
                    
                }
            }
            setTimeout(displayStatus(), 1000);

            //  getCount();
        },
        fail: function(error){
            console.log(error);
        }
    });
}

function sendQuery(query){
    $.ajax({
        type: 'get',
        data: {
            'query' : query
        },
        dataType: 'json',
        url: '/executeQuery/',
        success: function(data){
        }, 
        fail: function(error){
            console.log(error);
        }
    });
}

function loadFiles(){
    $("#cardViewPanel tr").remove();
    $("#fileLogPanel tr").remove();

    var row = cardViewPanel.insertRow(0);
    row.insertCell(0).innerHTML = "Property #";
    row.insertCell(1).innerHTML = "Name";
    row.insertCell(2).innerHTML = "Values (click to edit)";
    row.insertCell(3).innerHTML = "Parameters (click to view)";


    row = fileLogPanel.insertRow(0);
    row.insertCell(0).innerHTML = "File Name (click to download)";
    row.insertCell(1).innerHTML = "Individual's Name";
    row.insertCell(2).innerHTML = "Additional Properties";

    $.ajax({
        type: 'get',            //Request type
        dataType: 'json',       //Data type - we will use JSON for almost everything 
        url: '/uploads/',   //The server endpoint we are connecting to
        success: function (data) {

            console.log(data);

            updateFileTable(data);

            if(data.length > 0){
                //document.getElementById("clear").hidden = false;
                current = cardDD.selectedIndex;
                updateCardDD(data);
                cardDD.selectedIndex = current;
                updateCardTable();
            } 

            $('#blah').html("On page load, read "+ data.length +" file(s) from server");

        },
        fail: function(error) {
            //do nothing 
        }
    });

}

//contenteditable="true"

function updateCardDD(data){
    var length = cardDD.options.length;

    for(let i = 0; i < length; i++){
        cardDD.remove(0);
    }

    for(let i = 0; i < data.length; i++){
        var option = document.createElement("option");
        option.text = data[i].fileName;
        cardDD.add(option, cardDD[0]);
    }
}

function updateFileTable(data){
        for(let i = 0; i < data.length; i++){
            var row = fileLogPanel.insertRow(i+1);
            row.insertCell(0).innerHTML = "<a href=\"" + data[i].filePath + "\">" + data[i].fileName + "</a>";
            row.insertCell(1).innerHTML = data[i].fn;
            row.insertCell(2).innerHTML = data[i].propNum;
        }
        if(data.length == 0){
            var row = fileLogPanel.insertRow(1);
            row.insertCell(0).innerHTML = "No files uploaded";
            row.insertCell(1).innerHTML = "N/A";
            row.insertCell(2).innerHTML = "N/A";
        }    
}
//<a href="javascript:ShowOld(2367, 146986, 2);">

function updateCardTable(){
    console.log(cardDD.value);

    url = "/getObj/" + cardDD.value + "";

    $.ajax({
        type: 'get',           
        dataType: 'json',   
        url: url, 
        success: function (data) {
            $("#cardViewPanel tr").remove();

            var row = cardViewPanel.insertRow(0);
            row.insertCell(0).innerHTML = "Property #";
            row.insertCell(1).innerHTML = "Name";
            row.insertCell(2).innerHTML = "Values (click to edit)";
            row.insertCell(3).innerHTML = "Parameters (click to view)";

            currentCard = data[0];
            console.log(data[0]);

            var count = 1;

            var row = cardViewPanel.insertRow(1);
            row.insertCell(0).innerHTML = count;
            row.insertCell(1).innerHTML = "FN";
            var valueRow = row.insertCell(2);
            valueRow.innerHTML = currentCard.FN.values[0];
            if(currentCard.FN.values.length == 1){
                valueRow.setAttribute("onclick", "changeValue(this);");
            }
            row.insertCell(3).innerHTML = currentCard.FN.parameters.length;

            count++;

            if(currentCard.birth != undefined && currentCard.birth != ""){
                var row = cardViewPanel.insertRow(count);

                var birthText = "";

                if(currentCard.birth.isText){
                    birthText = currentCard.birth.text;
                } else {
                    if(currentCard.birth.date != ""){
                        birthText += "Date: " + currentCard.birth.date + " "
                    }
                    if (currentCard.birth.time != ""){
                        birthText += "Time: " + currentCard.birth.time;
                        if (currentCard.birth.isUTC){
                            birthText += " (UTC)"
                        }
                    }

                }

                row.insertCell(0).innerHTML = count;
                row.insertCell(1).innerHTML = "BDAY";
                row.insertCell(2).innerHTML = birthText;
                row.insertCell(3).innerHTML = 0;
                count++;
            }          

            if(currentCard.anni != undefined && currentCard.anni != ""){
                var row = cardViewPanel.insertRow(count);

                var birthText = "";

                if(currentCard.anni.isText){
                    birthText = currentCard.anni.text;
                } else {
                    if(currentCard.anni.date != ""){
                        birthText += "Date: " + currentCard.anni.date + " "
                    }
                    if (currentCard.anni.time != ""){
                        birthText += "Time: " + currentCard.anni.time;
                        if (currentCard.anni.isUTC){
                            birthText += " (UTC)"
                        }
                    }

                }

                row.insertCell(0).innerHTML = count;
                row.insertCell(1).innerHTML = "ANNIVERSARY";
                row.insertCell(2).innerHTML = birthText;
                row.insertCell(3).innerHTML = 0;
                count++;
            }      

            for(let i = 0; i < currentCard.list.length; i++){
                row = cardViewPanel.insertRow(i+count);
                row.insertCell(0).innerHTML = i+count;
                row.insertCell(1).innerHTML = currentCard.list[i].name;
                var valueRow = row.insertCell(2);
                valueRow.innerHTML = currentCard.list[i].values;
                if(currentCard.list[i].values.length == 1){
                    valueRow.setAttribute("onclick", "changeValue(this);");
                }
                var params = row.insertCell(3);
                params.innerHTML = currentCard.list[i].parameters.length;
                params.setAttribute("onclick", "parametersToString("+ i +")");

            }

        },
        fail: function(error) {
            //do nothing 
        }
    });
}
//document.getElementById("createCard").getElementsByTagName("div")[0].children[3].value;

function parametersToString(arrIndex){

    arr = currentCard.list[arrIndex].parameters;

    var toReturn = ""

    for(var i = 0; i < arr.length; i++){
        toReturn += "name: " + arr[i].name +  "     value: " + arr[i].value + "\n";
    }
    alert(toReturn);    
}

function addField() {
    var div = document.createElement("div");
    var fieldLabel = document.createElement("label");
    var field = document.createElement("input");
    var valueLabel = document.createElement("label");  
    var value = document.createElement("input");      

    fieldLabel.innerHTML = "Property Name: ";
    valueLabel.innerHTML = "Value: ";
    field.setAttribute("type", "text");
    field.setAttribute("name", "field"); 

    value.setAttribute("type", "text");
    value.setAttribute("name", "value"); 

    div.appendChild(fieldLabel);
    div.appendChild(field);
    div.appendChild(valueLabel);
    div.appendChild(value);

    var deleteBtn = document.createElement("button");
    deleteBtn.innerHTML = "delete property";
    deleteBtn.setAttribute("onclick", "deleteParent(this);");
    deleteBtn.setAttribute("type", "button");
    div.appendChild(deleteBtn);
    createCard.appendChild(div);

}

function parseProperties(){

    var props = {};

    props.birth = {};
    props.anni = {};
    props.addProps = [];
//{"isText":false,"date":"19540203","time":"123012","text":"","isUTC":true}
    var properties = document.getElementById("createCard").getElementsByTagName("div");
    console.log(properties.length);



    for(let i = 0; i < properties.length; i++){
        let curProp = properties[i];
        let name = curProp.children[1].value.trim();
        let value = curProp.children[3].value.trim();

        if(name.length == 0 || value.length == 0){
            return "Fields can not be empty! Please fill in or remove any unused fields: ";
        }
        else if(name.localeCompare("BDAY") == 0){
            props.birth.isText = true;
            props.birth.date = "";
            props.birth.time = "";
            props.birth.text = value;
            props.birth.isUTC = false;
        }
        else if(name.localeCompare("ANNIVERSARY") == 0){
            props.anni.isText = true;
            props.anni.date = "";
            props.anni.time = "";
            props.anni.text = value;
            props.anni.isUTC = false;           
        }
        else {
            if(acceptedNames.includes(name) == false){
                return "INVALID PROPERTY NAME: " + name;
            }
            let prop = {}
            prop.group = "";
            prop.name = name
            prop.values = [];
            prop.values[0] = value
            props.addProps[props.addProps.length] = prop;
        }
    }

    // console.log(JSON.stringify(props));
    return props;
}


function deleteParent(elem){
    // alert(elem.parentNode);
    var parent = elem.parentNode;
    parent.parentNode.removeChild(parent);
    console.log("pressed");
}

//{"group":"Name stuff","name":"FN","values":["Simon Perreault"]}
function changeValue(elem){
    var parent = elem.parentNode;
    //parentTemp = parent;
    console.log(parent);
    var name = parent.getElementsByTagName("td")[1].innerHTML;
    var val = parent.getElementsByTagName("td")[2].innerHTML;
    var newVal = prompt("Please enter new value for " + name + ":", val).trim();

    if(newVal != null && val.localeCompare(newVal) != 0  && newVal.length != 0){

        var newProp = {}
        var oldProp = {}

        newProp.group = "";
        oldProp.group = "";

        newProp.name = name;
        oldProp.name = name;


        newProp.values = [newVal]
        oldProp.values = [val]

        fileName = cardDD.value;


        console.log(JSON.stringify(newProp) + " " + JSON.stringify(oldProp))

        $.ajax({
            type: 'get',            //Request type
            dataType: 'text',       //Data type - we will use JSON for almost everything 
            url: '/updateVal/' + fileName,
            data: {
                oldJson: JSON.stringify(oldProp),
                newJson: JSON.stringify(newProp),
            },
            success: function (data) {
                console.log(data);
                loadFiles();
                if(newProp.name == "FN"){
                    var query = "UPDATE FILE set name='" + newVal + "' WHERE file_name = '"+ fileName+"' ";
                    sendQuery(query);
                }
            },
            fail: function(error) {
                alert("Error updating value" + error.responseText);
            },
            error: function(error) {
                alert("Error updating value" + error.responseText);
            }
        });

    } else {
        alert("Invalid value! Please try again with valid value");
    }
}


function getLogin(){
    $('#login').modal({
        backdrop : 'static',
        keyboard: false
    });
}


getLogin();
loadFiles();

function deleteFiles(){
    sendQuery("Delete from FILE;");
}


function displayStatus(){
    var query = ("SELECT ( SELECT COUNT(file_id) from FILE) as file, (SELECT COUNT(download_id) FROM DOWNLOAD) as download;");
        
    $.ajax({
        type: 'get',
        data: {
            'query' : query
        },
        dataType: 'json',
        url: '/executeQuery/',
        success: function(data){
            alert("Database has " + (data[0].file) + " files and " + (data[0].download) + " downloads");
        }, 
        fail: function(error){
            console.log(error);
        }
    });
}


function parseDateTime(vcfdateObj){
    var vcfdate = vcfdateObj.date + "t" + vcfdateObj.time;
    console.log(vcfdate);
    if (vcfdate == null || vcfdate == undefined || vcfdate.length < 15){
        return "NULL";
    }
    var date = vcfdate.substr(0,4) + "-" + vcfdate.substr(4,2) + "-" + vcfdate.substr(6,2) + " " + vcfdate.substr(9,2) + ":" + vcfdate.substr(11,2) + ":" + vcfdate.substr(13,2);
    return date;
}

function parseDateTimeString(vcfdate){
    console.log(vcfdate);
    if (vcfdate == null || vcfdate == undefined || vcfdate.length < 15){
        return "NULL";
    }
    var date = vcfdate.substr(0,4) + "-" + vcfdate.substr(4,2) + "-" + vcfdate.substr(6,2) + " " + vcfdate.substr(9,2) + ":" + vcfdate.substr(11,2) + ":" + vcfdate.substr(13,2);
    return date;
}

function hideAllSubquery(){
    document.getElementById("subquery1").hidden = true;
    document.getElementById("subquery3").hidden = true;
    document.getElementById("subquery4").hidden = true;
    document.getElementById("subquery5").hidden = true;
}