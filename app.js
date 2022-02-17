'use strict'

// C library API
const ffi = require('ffi-napi');

// Express App (Routes)
const express = require("express");
const app     = express();
const path    = require("path");
const fileUpload = require('express-fileupload');


app.use(fileUpload());
app.use(express.static(path.join(__dirname+'/uploads')));

// Minimization
const fs = require('fs');
const JavaScriptObfuscator = require('javascript-obfuscator');

const mysql = require('mysql2');
var connection = null;

// declaring early to validate upload
var libm = ffi.Library('./libvcparser', {
  'fileToJSON': ['string', ['string'] ],
  'createCardFromJSON': ['string',['string','string','string','string','string']],
  'updateValue': ['string', ['string', 'string', 'string']],
  'addPropToFile': ['string', ['string', 'string']]
});


app.get('/create/:filename', function(req, res){
  let filename = "./uploads/" + req.params.filename;


  //check if file already exist
  if (fs.existsSync(filename)) {
    res.status(409);
    res.send("File already exists. Please rename");
    return;
  }
  let fileJSON = req.query.fn;
  var propJSON = req.query.props;
  var birthJSON = req.query.birth;
  var anniJSON = req.query.anni;


  console.log("----")
  console.log(filename);
  console.log(fileJSON);
  console.log(propJSON);
  console.log(birthJSON);
  console.log(anniJSON);
  console.log("----")

  var response = libm.createCardFromJSON(filename, fileJSON, propJSON, birthJSON, anniJSON);

  console.log(response.localeCompare("OK"))

  if(response.localeCompare("OK") == 0){
    res.status(200);
    res.send("OK");
  } else {
    res.status(400);
    res.send(response);
  }


});

// Important, pass in port as in `npm run dev 1234`, do not change
const portNum = process.argv[2];

// Send HTML at root, do not change
app.get('/',function(req,res){
  res.sendFile(path.join(__dirname+'/public/index.html'));
});



// Send Style, do not change
app.get('/style.css',function(req,res){
  //Feel free to change the contents of style.css to prettify your Web app
  res.sendFile(path.join(__dirname+'/public/style.css'));
});

// Send obfuscated JS, do not change
app.get('/index.js',function(req,res){
  fs.readFile(path.join(__dirname+'/public/index.js'), 'utf8', function(err, contents) {
    const minimizedContents = JavaScriptObfuscator.obfuscate(contents, {compact: true, controlFlowFlattening: true});
    res.contentType('application/javascript');
    res.send(minimizedContents._obfuscatedCode);
  });
});

//Respond to POST requests that upload files to uploads/ directory
app.post('/upload', function(req, res) {
  if(!req.files) {
    return res.status(400).send('No files were uploaded.');
  }
 
  let uploadFile = req.files.uploadFile;

  // Use the mv() method to place the file somewhere on your server
  uploadFile.mv('uploads/' + uploadFile.name, function(err) {
    if(err) {
      return res.status(500).send(err);
    }

    res.redirect('/');
  });
});

//Respond to GET requests for files in the uploads/ directory
app.get('/uploads/:name', function(req , res){
  fs.stat('uploads/' + req.params.name, function(err, stat) {
    if(err == null) {
      var query = "INSERT INTO DOWNLOAD (d_descr, file_id, download_time) values ('downloaded " + req.params.name +  "', (SELECT file_id from FILE where file_name = '" + req.params.name + "'), NOW())"
      console.log(query);
      connection.query(query, function (err, rows, fields){
        if(err)
          console.log("Error " + err);
        else{
          console.log("Success for " + req.query.query);
          console.log(rows);
        }
      });
      
      res.sendFile(path.join(__dirname+'/uploads/' + req.params.name));
    } else {
      console.log('Error in file downloading route: '+err);
      res.send('');
    }
  });
});

//******************** Your code goes here ******************** 


app.get('/getObj/:fileName', function(req, res){
  let filePath = "./uploads/" + req.params.fileName
  var rArray = [];
  var jsonResult = libm.fileToJSON(filePath);
  if(jsonResult.includes("{")){
    var obj = JSON.parse(jsonResult)
    rArray.push(obj);
  } else{ 
    console.log("File:" + filePath + " returned error " + jsonResult);
  }
    res.send(rArray);
});

app.delete('/:fileName', function(req, res){
    try {
      fs.unlinkSync("./uploads/" + req.params.fileName);
      res.status(200);
      res.send("deleted successfully");
    } catch(err) {
        console.error(err);
        res.status(400);
        res.send("deleted unsuccessfully");
    }

});

app.get('/addProp/:fileName', function(req, res){
  var filePath = './uploads/' + req.params.fileName;
  var newJson = req.query.newJson;

  var result = libm.addPropToFile(filePath, newJson);

  console.log(result);

  console.log(result.localeCompare("OK"));

  if(result.localeCompare("OK") == 0){
    res.send("OK");
  } else {
    res.status(400);
    res.send(result);
  }
});


app.get('/updateVal/:fileName', function(req, res){

  var filePath = './uploads/' + req.params.fileName;

  var newJson = req.query.newJson;
  var oldJson = req.query.oldJson;


  var result = libm.updateValue(filePath, oldJson, newJson);

  console.log(result);

  console.log(result.localeCompare("OK"));

  if(result.localeCompare("OK") == 0){
    res.send("OK");
  } else {
    res.status(400);
    res.send(result);
  }

});


app.get('/uploads/', function(req, res){
    var result = fs.readdirSync("./uploads/");
    var rArray = [];
    for(var i = 0; i < result.length; i++){
      var filePath = "./uploads/" + result[i];
      var jsonResult = libm.fileToJSON(filePath);
      if(jsonResult.includes("{")){
        var obj = JSON.parse(jsonResult)
        obj.fileName = result[i];
        obj.fn = obj.FN.values[0];
        obj.propNum = obj.list.length;
        obj.filePath = filePath;
        rArray.push(obj);
      } else{ 
        console.log("File:" + filePath + " returned error " + jsonResult);
      }
    }
    res.send(rArray);
});


//Here's a change
//Sample endpoint
app.get('/endpoint1', function(req , res){
  let retStr = req.query.stuff + " " + req.query.junk;
  res.send({
    stuff: retStr
  });
});


app.get('/executeQuery', function(req, res){
  console.log(req.query.query);
  connection.query(req.query.query, function (err, rows, fields){
		if(err)
			console.log("Error " + err);
		else{
			console.log("Success for " + req.query.query);
      console.log(rows);
			res.send(rows);
		}
	});
});

app.get('/login', function (req, res){

  connection = mysql.createConnection({
		host: 'dursley.socs.uoguelph.ca',
		user: req.query.userName,
		password: req.query.password,
		database : req.query.database
  });
  
  connection.connect(function(err){
    var toReturn = {};
    if(err){
      res.send(err.message);
    } else{

    var fileTable =  'create table IF NOT EXISTS FILE (file_id INT PRIMARY KEY AUTO_INCREMENT, file_name VARCHAR(60) NOT NULL, num_props INT NOT NULL, name VARCHAR(256) NOT NULL, birthday DATETIME, anniversary DATETIME, creation_time DATETIME NOT NULL);';
    var downloadTable = 'create table IF NOT EXISTS DOWNLOAD (download_id INT AUTO_INCREMENT PRIMARY KEY, d_descr VARCHAR(256), file_id INT not null , foreign key(file_id) references FILE(file_id) on delete cascade, download_time DATETIME NOT NULL);';
    
    connection.query(fileTable, function(err, result){
      if(err){
        console.log(err);
      } else{
        console.log("file table created");
      }
    });
    connection.query(downloadTable, function(err, result){
      if(err){
        console.log(err);
      } else{
        console.log("Download table created");
      }
    });

    res.status(200);
    toReturn = {"type" : "ok"};
    res.send(toReturn);
    }

  });

});


app.listen(portNum);
console.log('Running app at localhost: ' + portNum);