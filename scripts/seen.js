function getNick(raw)
{
	var aSource = raw.split("!");
	return aSource[0];
}

var seen = function(source, command, params)
{
	var nick = getNick(source);
	var msg = params[1];
	var split = params[1].split(" ");

	var conn = DB.connectByPredef("default");
	DB.query(conn, "SET NAMES latin1", []);

	if (split[0] == ".seen") {
		if (split.length != 2 || split[1] == "") {
			IRC.msg(params[0], "Nice try. Usage: .seen nick");
		} else if (split[1].toLowerCase() == nick.toLowerCase()) {
			IRC.msg(params[0], "Yeah, right.");
		} else {
			var nicks = IRC.getNicks(params[0]);
			var found = false;
			for (var n in nicks) {
				if (nicks[n].toLowerCase() == split[1].toLowerCase()) {
					found = true;
					break;
				}
			}

			if (found) {
				IRC.msg(params[0], split[1] + " is in the channel right now.");
			} else {
				var sql2 = "SELECT from_unixtime(msg_timestamp) d, msg_text, msg_channel FROM msg WHERE msg_nick = '?' ORDER BY msg_timestamp DESC LIMIT 1";
				var qry = DB.query(conn, sql2, [split[1]]);
				if (qry != null && qry.length > 0) {
					var row = qry[0];
					msg = "I last saw " + split[1] + " in " + row["msg_channel"] + " on " + row["d"] + " saying: " + row["msg_text"];
					IRC.msg(params[0], msg);
				} else {
					IRC.msg(params[0], "I don't know " + split[1] + ". :(");
				}
			}
		}
	}
}

Bot.addCallback("privmsg", seen);
