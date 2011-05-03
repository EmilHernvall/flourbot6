var msgHandler = function(source, command, params)
{
	var split = params[1].split(" ");

	if (split[0] == ".nicks") {
		Console.println(IRC.getNickCount(params[0]) + " nicks in this channel.");
		var nicks = IRC.getNicks(params[0]);
		for (var n in nicks) {
			Console.println(nicks[n]);
		}
	}
	else if (split[0] == ".try") {
		Console.println(IRC.isUserOnline(params[0], split[1]));
	}
}

Console.println("test.js is active!");
Bot.addCallback("privmsg", msgHandler);
