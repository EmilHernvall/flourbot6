var peak = function(source, command, params)
{
	var msg = params[1];
	var split = params[1].split(" ");

	var conn = DB.connectByPredef("default");
	DB.query(conn, "SET NAMES latin1", []);

	if (split[0] == ".peak") {
		var sql2 = "SELECT max(nick_totalcount) max, from_unixtime(min(nick_timestamp)) since FROM nick WHERE nick_channel = '?'";
		var qry = DB.query(conn, sql2, [params[0]]);
		if (qry != null && qry.length > 0) {
			var row = qry[0];
			msg = "Channel peak for " + params[0] + " is " + row["max"] + ". Logging since " + row["since"] + ".";
			IRC.msg(params[0], msg);
		} else {
			IRC.msg(params[0], "I don't know " + split[1] + ". :(");
		}
	}
}

Bot.addCallback("privmsg", peak);
