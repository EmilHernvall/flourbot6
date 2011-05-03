function getNick(raw)
{
	var aSource = raw.split("!");
	return aSource[0];
}

function logAddress(conn, channel, nick, current)
{
	var sqlLookup = "SELECT * FROM address WHERE address_channel = '?' AND address_source = '?' AND address_target = '?'";
	var lookupResult = DB.query(conn, sqlLookup, [channel, nick, current]);
	var updateSql = "";
	if (lookupResult != null && lookupResult.length > 0) {
		updateSql = "UPDATE address SET address_count = address_count + 1 WHERE address_channel = '?' AND address_source = '?' AND address_target = '?'";
	} else {
		updateSql = "INSERT INTO address (address_channel, address_source, address_target, address_count) VALUES ('?', '?', '?', 1)";
	}

	DB.query(conn, updateSql, [channel, nick, current]);
}

var privmsgHandler = function(source, command, params)
{
	var nick = getNick(source);
	var msg = params[1];
	var split = params[1].split(" ");

	var conn = DB.connectByPredef("default");
	DB.query(conn, "SET NAMES latin1", []);

	var sql = "INSERT INTO msg (msg_timestamp, msg_nick, msg_channel, msg_text) ";
	sql += "VALUES (unix_timestamp(), '?', '?', '?')";
	
	DB.query(conn, sql, [nick, params[0], msg]);

	if (msg.match(/^[^:, ]+(:|,) .*$/)) {
		var msgSplit = msg.split(/,|:/);
		var findNick = msgSplit[0].toLowerCase();

		if (IRC.isUserOnline(params[0], msgSplit[0])) {
			Console.println("Logging highlight for " + msgSplit[0] + " by " + nick + ".");
			logAddress(conn, params[0], nick, msgSplit[0]);
		}
	}

	for (var i in split) {
		var word = split[i].toLowerCase();
		if (IRC.isUserOnline(params[0], split[i])) {
			Console.println("Logging highlight for " + split[i] + " by " + nick + ".");
			logAddress(conn, params[0], nick, split[i]);			
		}
	}
}

var nickHandler = function(source, command, params)
{
	var conn = DB.connectByPredef("default");
	DB.query(conn, "SET NAMES latin1", []);

	var sql = "INSERT INTO nick (nick_nick, nick_channel, nick_type, nick_timestamp, nick_text, nick_issuer, nick_totalcount) ";
	sql += "VALUES ('?', '?', '?', unix_timestamp(), '?', '?', '?')";

	if (command == "quit") {
		// for each channel this user is on
		var nick = getNick(source);
		var msg = params.length > 0 ? params[0] : "";
		DB.query(conn, sql, [nick, "", command, msg, "", 0]);
		return;
	}

	if (command == "kick") {
		var nick = params[1];
		var channel = params[0];
		var msg = params.length > 2 ? params[2] : "";
		var issuer = getNick(source);
		var totalCount = IRC.getNickCount(channel);
		DB.query(conn, sql, [nick, channel, command, msg, issuer, totalCount]);
	} else {
		var nick = getNick(source);
		var channel = params[0];
		var msg = params.length > 1 ? params[1] : "";
		var totalCount = IRC.getNickCount(channel);
		DB.query(conn, sql, [nick, channel, command, msg, "", totalCount]);
	}
}

Bot.addCallback("privmsg", privmsgHandler);
Bot.addCallback("join", nickHandler);
Bot.addCallback("part", nickHandler);
Bot.addCallback("kick", nickHandler);
Bot.addCallback("quit", nickHandler);
//Bot.addCallback("kill", nickHandler);
