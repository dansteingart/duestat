var fs = require('fs')
const express = require('express');
var app = express();
const bodyParser = require('body-parser');
app.use(bodyParser.urlencoded({extended:true})); //deprecated not sure what to do here....
var glob = require("glob")

const server = require('http').createServer(app);
const io = require('socket.io')(server);
io.on('connection', () => { /* … */ });

app.use('/static', express.static('static'));

parts = process.argv
  
try {spo = parts[2]}
catch (e)
{
	console.log("doesn't look like a serial port was given, guess linux and cross fingers")
	spo = "/dev/ttyACM0";
}

console.log("Attempting to connect to a duestat @ "+spo)

verbose = false;
for (i in parts)
{
	if (parts[i] == "-v") verbose = true; 
}


const SerialPort = require('serialport')
const Readline = require('@serialport/parser-readline')
const port = new SerialPort(spo)
const parser = new Readline()

header = "TIME (us)\tPERIOD (us)\tDAC1 (V)\tCELL (V)\tREF (V)\tDAC0 (V)\tOUTPUT (V)\tTARGET\tVCELL (V)\tRES (ohm)\tMODE\tKP\tKI\tKD\tSENDTIME (us)";
if (verbose) console.log(header)
port.pipe(parser)

//setup logger
var fn;
var fnj;
var fnc;
var logger = false;

try {
	fs.mkdirSync("data")	
} catch (error) {
	
}

function pushpop(value,array,length)
{
	array.push(value)
	if (array.length > length) array.pop(0)
	return array
}

//to be used later for analyzed data
function toCSV(obj)
{
	out = Object.keys(j)
	out.sort()
	h = out.toString() +"\n"
	data = h
	for (i = 0; i < obj[out[0]].length; i++)
	{
		  for (j = 0; j < out.length; j++)
		  {
				data += `${out[j][i]}`
		  }
		  data += "\n"
	}
}

start = Date.now()
period = 5*1000 //ms
last_packet = undefined;

parser.on('data',  function(data)
	{
		//map to numbers except for mode
		packet = data.trim().split("\t")
		mode = packet[9]
		packet = packet.map(x=>+x)
		packet[9] = mode  
		packet.unshift(Date.now())
        str = packet.toString().replace("[","").replace("]","")

		if (logger) fs.appendFile("data/"+fnc,str+"\n",err=>{})
		last_packet = packet;
		if (verbose) console.log(data)
		io.emit('news',packet)

	}

)

app.get('/', function (req, res) {
  res.sendFile(__dirname + '/html/shower.html');
});


app.post('/setting/',function(req,res)
{
	const data = req.body;
	port.write(JSON.stringify(data)+"\n");

	res.send({'success':true});

})


app.post('/exp_start/',function(req,res)
{
	logger = false;
	const exp = req.body;
	now = Date.now()

	if (exp.hasOwnProperty('name')) fn = exp['name']+"_"+now 
	else fn = "experiment_"+now
		
	fnj = fn+".json"
	fnc = fn+".csv"

	fs.writeFile("data/"+fnj,JSON.stringify(exp)+"\n",err=>{})
	fs.writeFile("data/"+fnc,header.replace(/\t/g,",")+"\n",err=>{})
	logger = true;

	res.send({'success':true,'logger':logger,'exp':exp['name']+"_"+now});

})

app.post('/exp_stop/',function(req,res)
{
	logger = false;
	res.send({'success':true,});

})


app.get('/rawdata/',function(req,res)
{
	res.send(last_packet);
})


app.get('/data/',function(req,res)
{
	out = {}
	temp = JSON.parse(JSON.stringify(last_packet))

	out['time_utns'] = temp[0]
	out['cell_V']  = temp[3]-temp[5]
	out['i_A'] = (temp[2]-temp[3])/temp[9]
	out['mode'] = temp[10]
	out['target'] = temp[7]

	res.send(out);
})

app.get('/experiments/',function(req,res)
{
	res.sendFile(__dirname + '/html/experiments.html');
})

app.get('/get_experiment_list/',function(req,res)
{
	res.send(JSON.stringify(glob.sync("data/*.json")).replace(/data\//g,"").replace(/.json/g,""))
})

app.get('/get_experiment_data/:fil?',function(req,res)
{
    if(req.params.fil)
	{
		if (req.params.fil.search(".csv") > -1) res.sendFile(__dirname + '/data/'+req.params.fil);
		else res.send(fs.readFileSync(__dirname + '/data/'+req.params.fil+".csv"));
	}
	else res.send("I need something to do.")

})

app.get('/get_experiment_meta/:fil?',function(req,res)
{
    if(req.params.fil)
	{
		if (req.params.fil.search(".json")> -1) res.sendFile(__dirname + '/data/'+req.params.fil);
		else res.send(fs.readFileSync(__dirname + '/data/'+req.params.fil+".json"));
	}
	else res.send("I need something to do.")
})


server.listen(3200);


