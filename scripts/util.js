function getNick(raw)
{
	var aSource = raw.split("!");
	return aSource[0];
}

var joinHandler = function(source, command, params)
{
	if (nick == "Aderyn") {
		IRC.mode(params[0], "+o", "Aderyn");
	}
}

var msgHandler = function(source, command, params)
{
	var nick = getNick(source);
	var msg = params[1];
	var split = params[1].split(" ");

	if (split[0] == ".raw" && nick == "Aderyn") {
		var command = msg.substring(5);
		Console.println("command from aderyn: " + command);
		IRC.raw(command);
	}
	else if (split[0] == ".channels") {
		var channels = IRC.getChannels();
		var res = "I'm currently on: ";
		for (var i = 0; i < channels.length; i++) {
			res += channels[i] + " ";
		}
		IRC.msg(params[0], res);
	}
	/*else if (split[0] == ".nicks") {
		var nicks = IRC.getNicks(split[1]);
		Console.println("Nicks in " + split[1] + ":");
		for (var i = 0; i < nicks.length; i++) {
			Console.println(nicks[i]);
		}
	}*/
}

//Bot.addCallback("join", joinHandler);
Bot.addCallback("privmsg", msgHandler);
