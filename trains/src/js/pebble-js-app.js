
Date.prototype.format = function(format) //author: meizz
{
	var o = {
			"M+" : this.getMonth()+1, //month
			"d+" : this.getDate(),    //day
			"h+" : this.getHours(),   //hour
			"i+" : this.getHours() + 1,   //hour + 1
			"m+" : this.getMinutes(), //minute
			"s+" : this.getSeconds(), //second
			"q+" : Math.floor((this.getMonth()+3)/3),  //quarter
			"S" : this.getMilliseconds() //millisecond
	}

	if(/(y+)/.test(format)) format=format.replace(RegExp.$1,
			(this.getFullYear()+"").substr(4 - RegExp.$1.length));
	for(var k in o)if(new RegExp("("+ k +")").test(format))
		format = format.replace(RegExp.$1,
				RegExp.$1.length==1 ? o[k] :
					("00"+ o[k]).substr((""+ o[k]).length));
	return format;
}



function train(time, dest, plat, last) {
	this.time = time;
	this.dest = dest;
	this.plat = plat;
	this.last = last;
	console.log("time=[" + time + "], dest=[" + dest + "], plat=[" + plat + "]");
}

function getTable(str, identity) {
	var n = str.indexOf(identity);
	if (n >= 0) {
		m = str.indexOf("</table>",n);
		return str.slice(n,m);
	}
	return "";
}

//Gives the character of the last of a tag
function skipstarttag(str, start, tag) {
	seek = "<" + tag;
	x = str.indexOf(seek,start);
	if (x >= 0)
		x = str.indexOf(">", x + 1) + 1;
	return x;
}



function getEntry(str, start) {
	var tr = skipstarttag(str, start, "tr");
	if (tr < 0) return new train("","","",-1);



	var td1 = skipstarttag(str, tr, "td" );
	if (td1 < 0) return new train("","","",-1);
	var td2 = str.indexOf("</td>",td1);
	if (td2 < 0) return new train("","","",-1);
	var time = str.slice(td1,td2);

	var td3 = skipstarttag(str, td2, "td" );
	if (td3 < 0) return new train("","","",-1);
	var td4 = str.indexOf("</td>",td3);
	if (td4 < 0) return new train("","","",-1);
	var dest = str.slice(td3,td4);
	
	var td0 = skipstarttag(str, td4, "td");
	if (td0 < 0) return new train("","","",-1);
	var td05 = str.indexOf("</td>",td0);
	if (td05 < 0) return new train("","","",-1);

	var td5 = skipstarttag(str, td05, "td" );
	if (td5 < 0) return new train("","","",-1);
	var td6 = str.indexOf("</td>",td5);
	if (td6 < 0) return new train("","","",-1);
	var plat = str.slice(td5,td6);

	return new train(time.trim(), dest.trim(), plat, td6);
}

//http://www.opentraintimes.com/departures/KGX/to/SVG/2013-11-09/0927

function fetchTrans(from, to) {

	// Time
	var baseUrl = "http://ojp.nationalrail.co.uk/service/ldbboard/dep/" + from + "/" + to + "/To";

	var trains = new Array();
	var trainInd = -1;

	var response;
	var req = new XMLHttpRequest();
	var url = baseUrl;

	req.open('GET', url, true);
	console.log("getting for url=" + url);
	req.onload = function(e) {
		if (req.readyState == 4 && req.status == 200) {
			var tab = getTable(req.responseText, 'results trains');
			var n=0;
			while (trainInd < 2) {
				var entry = getEntry(tab, n);
				if (entry.last < 0)
					break;
				n = entry.last;
				trainInd++;
				trains[trainInd] = entry;
			}
			sendToPebble(trains, trainInd, from, to);

		}
	}
	req.send(null);
}


function sendToPebble(trains, trainInd, from, to) {
	Pebble.sendAppMessage({
		"from": from,
		"to": to,
		"time1": (trainInd >= 0 ? trains[0].time : ""),
		"dest1": (trainInd >= 0 ? trains[0].dest : ""),
		"plat1": (trainInd >= 0 ? trains[0].plat : ""),
		"time2": (trainInd >= 1 ? trains[1].time : ""),
		"dest2": (trainInd >= 1 ? trains[1].dest : ""),
		"plat2": (trainInd >= 1 ? trains[1].plat : ""),
		"time3": (trainInd >= 2 ? trains[2].time : ""),
		"dest3": (trainInd >= 2 ? trains[2].dest : ""),
		"plat3": (trainInd >= 2 ? trains[2].plat : "")});
}




function getFrom() {
	var fromValue = localStorage["from"];
	if (fromValue == null)
		fromValue = "KGX";
	return fromValue;
}

function getTo() {
	var toValue = localStorage["to"];
	if (toValue == null)
		toValue = "SVG";
	return toValue;
}

function setFrom(from) {
	localStorage["from"] = from;
}

function setTo(to) {
	localStorage["to"] = to;
}



Pebble.addEventListener("ready",
		function(e) {
	fetchTrans(getFrom(),getTo());
});

Pebble.addEventListener("appmessage",
		function(e) {
	fetchTrans(getFrom(),getTo());
});

Pebble.addEventListener("webviewclosed",
		function(e) {
	console.log("webview closed");
	console.log(e.type);
	var config = e.response;
	var from = config.slice(0,3);
	var to = config.slice(4,7);
	console.log("from=" + from + ", to=" + to);
	setFrom(from);
	setTo(to);
	fetchTrans(getFrom(),getTo());
});

Pebble.addEventListener("showConfiguration",
		function(e) {
	console.log("config");
	Pebble.openURL("http://homepage.ntlworld.com/keith.j.fowler/trains/config.html?from=" + getFrom() + "&to=" + getTo());
});

Pebble.addEventListener("configurationClosed",
		function(e) {
	console.log("Configuration window returned: " + e.configurationData);
});

