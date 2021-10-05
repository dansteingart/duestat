var fs = require('fs')
const express = require('express');
var app = express();
const bodyParser = require('body-parser');
app.use(bodyParser.urlencoded())

const server = require('http').createServer(app);
const io = require('socket.io')(server);
io.on('connection', () => { /* … */ });

app.use('/static', express.static('static'));

parts = process.argv

try
{
		spo = parts[2]
}
catch (e)
{
	console.log("doesn't look like a port was given, guess linux and cross fingers")
	spo = "/dev/ttyACM0";
}

const SerialPort = require('serialport')
const Readline = require('@serialport/parser-readline')
const port = new SerialPort(spo)
const parser = new Readline()

header = "TIME (us)\tPERIOD (us)\tDAC1 (V) \tCELL (V)\tREF (V)\tDAC0 (V)\tOUTPUT (V)\tTARGET\tVCELL (V)\tRES (ohm)\tMODE\tKP\tKI\tKD\tSENDTIME (us)";
console.log(header)
port.pipe(parser)

//setup interval logger
const ll = 200*3600 //approx xx*100 seconds of data
var ts = []
var V1 = []
var V2 = []

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

		last_packet = packet;

		console.log(data)
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

app.get('/rawdata/',function(req,res)
{
	res.send({'data':last_packet});
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

	res.send({'data':out});
})



server.listen(3200);
