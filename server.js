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


header = "time,dt,V1,V2,OS,dts"
console.log(header);
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
chunk = header+"\n"

parser.on('data',  function(data)
	{
		packet = data.trim().split("\t").map(x=>+x)
		packet.unshift(Date.now())
    str = packet.toString().replace("[","").replace("]","")

		ts = pushpop(packet[0],ts,ll)
		V1 = pushpop(packet[1],V1,ll)
		V2 = pushpop(packet[2],V2,ll)

		chunk += str+"\n"

			if (Date.now() > (start+period))
			{
				// async file write
				one = "two";
			  //fs.writeFile("data/data_"+Date.now().toString()+".csv",chunk, (err) =>
				// {
		  	//  	if (err) throw err;
				// }
			//);

		 //reset array
		 chunk = header+"\n";
		 start = Date.now();

		}

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

server.listen(3200);
