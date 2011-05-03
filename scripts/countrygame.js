var countries = {"United Arab Emirates":"Abu Dhabi",
"Nigeria":"Abuja",
"Ghana":"Accra",
"Ethiopia":"Addis Ababa",
"Algeria":"Algiers",
"Jordan":"Amman",
"Netherlands":"Amsterdam",
"Andorra":"Andorra la Vella",
"Turkey":"Ankara",
"Madagascar":"Antananarivo",
"Samoa":"Apia",
"Turkmenistan":"Ashgabat",
"Eritrea":"Asmara",
"Kazakhstan":"Astana",
"Paraguay":"Asunción",
"Greece":"Athens",
"Iraq":"Baghdad",
"Azerbaijan":"Baku",
"Mali":"Bamako",
"Brunei":"Bandar Seri Begawan",
"Thailand":"Bangkok",
"Central African Republic":"Bangui",
"Gambia":"Banjul",
"Saint Kitts and Nevis":"Basseterre",
"China":"Beijing",
"Lebanon":"Beirut",
"Serbia":"Belgrade",
"Belize":"Belmopan",
"Germany":"Berlin",
"Switzerland":"Bern",
"Kyrgyzstan":"Bishkek",
"Guinea-Bissau":"Bissau",
"Colombia":"Bogotá",
"Brazil":"Brasília",
"Slovakia":"Bratislava",
"Republic of the Congo":"Brazzaville",
"Barbados":"Bridgetown",
"Belgium":"Brussels",
"Romania":"Bucharest",
"Hungary":"Budapest",
"Argentina":"Buenos Aires",
"Burundi":"Bujumbura",
"Egypt":"Cairo",
"Australia":"Canberra",
"Venezuela":"Caracas",
"Saint Lucia":"Castries",
"Moldova":"Chisinau",
"Guinea":"Conakry",
"Denmark":"Copenhagen",
"Senegal":"Dakar",
"Syria":"Damascus",
"Bangladesh":"Dhaka",
"East Timor":"Dili",
"Djibouti":"Djibouti",
"Tanzania":"Dodoma",
"Qatar":"Doha",
"Ireland":"Dublin",
"Tajikistan":"Dushanbe",
"Sierra Leone":"Freetown",
"Tuvalu":"Funafuti",
"Botswana":"Gaborone",
"Guyana":"Georgetown",
"Guatemala":"Guatemala City",
"Vietnam":"Hanoi",
"Zimbabwe":"Harare",
"Cuba":"Havana",
"Finland":"Helsinki",
"Solomon Islands":"Honiara",
"Pakistan":"Islamabad",
"Indonesia":"Jakarta",
"Israel":"Jerusalem",
"Afghanistan":"Kabul",
"Uganda":"Kampala",
"Nepal":"Kathmandu",
"Sudan":"Khartoum",
"Ukraine":"Kiev",
"Rwanda":"Kigali",
"Jamaica":"Kingston",
"Saint Vincent and the Grenadines":"Kingstown",
"Democratic Republic of the Congo":"Kinshasa",
"Malaysia":"Kuala Lumpur",
"Kuwait":"Kuwait City",
"Bolivia":"La Paz",
"Gabon":"Libreville",
"Malawi":"Lilongwe",
"Peru":"Lima",
"Portugal":"Lisbon",
"Slovenia":"Ljubljana",
"Togo":"Lomé",
"United Kingdom":"London",
"Angola":"Luanda",
"Zambia":"Lusaka",
"Luxembourg":"Luxembourg City",
"Spain":"Madrid",
"Marshall Islands":"Majuro",
"Equatorial Guinea":"Malabo",
"Maldives":"Malé",
"Nicaragua":"Managua",
"Bahrain":"Manama",
"Philippines":"Manila",
"Mozambique":"Maputo",
"Lesotho":"Maseru",
"Swaziland":"Mbabane",
"Palau":"Melekeok",
"Mexico":"Mexico City",
"Belarus":"Minsk",
"Somalia":"Mogadishu",
"Monaco":"Monaco",
"Liberia":"Monrovia",
"Uruguay":"Montevideo",
"Comoros":"Moroni",
"Russia":"Moscow",
"Oman":"Muscat",
"Kenya":"Nairobi",
"Bahamas":"Nassau",
"Myanmar":"Naypyidaw",
"Chad":"N'Djamena",
"India":"New Delhi",
"Niger":"Niamey",
"Cyprus":"Nicosia",
"Mauritania":"Nouakchott",
"Tonga":"Nuku'alofa",
"Norway":"Oslo",
"Canada":"Ottawa",
"Burkina Faso":"Ouagadougou",
"Federated States of Micronesia":"Palikir",
"Panama":"Panama City",
"Suriname":"Paramaribo",
"France":"Paris",
"Cambodia":"Phnom Penh",
"Montenegro":"Podgorica",
"Mauritius":"Port Louis",
"Papua New Guinea":"Port Moresby",
"Vanuatu":"Port Vila",
"Haiti":"Port-au-Prince",
"Trinidad and Tobago":"Port of Spain",
"Benin":"Porto-Novo",
"Czech Republic":"Prague",
"Cape Verde":"Praia",
"South Africa":"Pretoria",
"Malaysia":"Putrajaya",
"North Korea":"Pyongyang",
"Ecuador":"Quito",
"Morocco":"Rabat",
"Iceland":"Reykjavík",
"Latvia":"Riga",
"Saudi Arabia":"Riyadh",
"Italy":"Rome",
"Dominica":"Roseau",
"Costa Rica":"San José",
"San Marino":"San Marino",
"El Salvador":"San Salvador",
"Yemen":"Sanaá",
"Chile":"Santiago",
"Dominican Republic":"Santo Domingo",
"São Tomé and Príncipe":"São Tomé",
"Bosnia and Herzegovina":"Sarajevo",
"South Korea":"Seoul",
"Singapore":"Singapore",
"Macedonia":"Skopje",
"Bulgaria":"Sofia",
"Kiribati":"South Tarawa",
"Sri Lanka":"Sri Jayawardenepura",
"Grenada":"St. George's",
"Antigua and Barbuda":"St. John's",
"Sweden":"Stockholm",
"Bolivia":"Sucre",
"Fiji":"Suva",
"Estonia":"Tallinn",
"Uzbekistan":"Tashkent",
"Georgia":"Tbilisi",
"Honduras":"Tegucigalpa",
"Iran":"Tehran",
"Bhutan":"Thimphu",
"Albania":"Tirana",
"Transnistria":"Tiraspol",
"Japan":"Tokyo",
"Libya":"Tripoli",
"Tunisia":"Tunis",
"Mongolia":"Ulaanbaatar",
"Liechtenstein":"Vaduz",
"Malta":"Valletta",
"Vatican City":"Vatican City",
"Seychelles":"Victoria",
"Austria":"Vienna",
"Laos":"Vientiane",
"Lithuania":"Vilnius",
"Poland":"Warsaw",
"United States":"Washington, D.C.",
"New Zealand":"Wellington",
"Namibia":"Windhoek",
"Côte d'Ivoire":"Yamoussoukro",
"Cameroon":"Yaoundé",
"Nauru":"Yaren",
"Armenia":"Yerevan",
"Croatia":"Zagreb"};

var capitals = {};
var setSize = 0;

var log = {};
var nickLog = {};
var score = {};

function getNick(raw)
{
	var aSource = raw.split("!");
	return aSource[0];
}

function EditDistance(s, t)
{
	s = s.toLowerCase();
	t = t.toLowerCase();

	var d = {};
	for (var i = 0; i <= s.length; i++) {
		d[i] = {};
	}

	for (var i = 0; i <= s.length; i++) {
		d[i][0] = i;
	}

	for (var j = 0; j <= t.length; j++) {
		d[0][j] = j;
	}

	for (var i = 1; i < s.length+1; i++) {
		for (var j = 1; j < t.length+1; j++) {
			var cost;
			if (s.charAt(i-1) == t.charAt(j-1)) {
				cost = 0;
			} else {
				cost = 1;
			}

			d[i][j] = Math.min(d[i-1][j] + 1,
				d[i][j-1] + 1, 
				d[i-1][j-1] + cost);
		}
	}

	return d[s.length][t.length];
}

var game = function(source, command, params)
{
	var target = params[0];
	var nick = getNick(source);
	var msg = params[1];
	var split = params[1].split(" ");

	if (split[0] == ".capital" && split.length > 1) {
		var country = "", sep = "";
		for (var i = 1; i < split.length; i++) {
			country += sep + split[i];
			sep = " ";
		}
		Console.println("Searching for " + country);
		var capital = countries[country];
		if (capital != null) {
			IRC.msg(target, capital);
		}
	}
	else if (split[0] == ".country" && split.length > 1) {
		var capital = "", sep = "";
		for (var i = 1; i < split.length; i++) {
			capital += sep + split[i];
			sep = " ";
		}
		Console.println("Searching for " + capital);
		var country = capitals[capital];
		if (country != null) {
			IRC.msg(target, country);
		}
	}
	else if (split[0] == ".find" && split.length > 1) {
		var guess = "", sep = "";
		for (var i = 1; i < split.length; i++) {
			guess += sep + split[i];
			sep = " ";
		}

		var minValue = 1000, minKey = "";
		for (var country in countries) {
			var d = EditDistance(guess, country);
			if (d == 0) {
				minValue = 0;
				minKey = country;
				break;
			}

			if (d < minValue) {
				minValue = d;
				minKey = country;
			}
		}

		for (var capital in capitals) {
			var d = EditDistance(guess, capital);
			if (d == 0) {
				minValue = 0;
				minKey = capital;
				break;
			}

			if (d < minValue) {
				minValue = d;
				minKey = capital;
			}
		}

		var msg = "Did you mean " + minKey + " (ed: " + minValue + ")?";
		IRC.msg(target, msg);
	}
	else if ((split[0] == ".play" || split[0] == ".p") && split.length > 1) {
		if (log[target] == null) {
			Console.println("Created log for " + target);
			log[target] = [];
			nickLog[target] = [];
		}
		if (score[target] == null) {
			score[target] = {};
		}

		var guess = "", sep = "";
		for (var i = 1; i < split.length; i++) {
			guess += sep + split[i];
			sep = " ";
		}

		var match = false;
		for (var i = 0; i < log[target].length; i++) {
			if (log[target][i] == guess) {
				match = true;
				break;
			}
		}
		
		if (match) {
			IRC.msg(target, "Sorry, " + guess + " has already been said.");
		} 
		else {
			if (countries[guess] != null) {
				IRC.msg(target, "Correct, " + guess + " is a country.");

				log[target].push(guess);
				nickLog[target].push(nick);
				if (score[target][nick] == null) {
					score[target][nick] = 1;
				} else {
					score[target][nick]++;
				}
			}
			else if (capitals[guess] != null) {
				IRC.msg(target, "Correct, " + guess + " is a capital.");
				log[target].push(guess);
				nickLog[target].push(nick);
				if (score[target][nick] == null) {
					score[target][nick] = 1;
				} else {
					score[target][nick]++;
				}
			}
			else {
				IRC.msg(target, "Sorry, there's nothing named " + guess + ". :(");
			}
		}
	}
	else if (split[0] == ".log") {
		if (log[target] == null) {
			Console.println("Created log for " + target);
			log[target] = [];
			nickLog[target] = [];
		}

		var res = "", sep = "";
		var localLog = log[target];
		var localNickLog = nickLog[target];
		for (var i in localLog) {
			res += sep + localLog[i] + " (" + localNickLog[i] + ")";
			sep = ", ";

			if (i % 15 == 0 && i > 0) {
				IRC.msg(target, res);
				sep = "";
				res = "";
			}
		}

		if (res != "") {
			IRC.msg(target, res);
		}
	}
	else if (split[0] == ".sortedlog") {
		if (log[target] == null) {
			Console.println("Created log for " + target);
			log[target] = [];
			nickLog[target] = [];
		}

		var res = "", sep = "";
		var localLog = log[target].slice();
		localLog.sort();

		for (var i in localLog) {
			res += sep + localLog[i];
			sep = ", ";

			if (i % 15 == 0 && i > 0) {
				IRC.msg(target, res);
				sep = "";
				res = "";
			}
		}

		if (res != "") {
			IRC.msg(target, res);
		}
	}
	else if (split[0] == ".score") {
		if (score[target] == null) {
			score[target] = {};
		}

		var msg = "", sep = "";
		for (var n in score[target]) {
			msg += sep + n + ": " + score[target][n];
			sep = ", ";
		}

		IRC.msg(target, "[Current score] " + msg);
	}
	else if (split[0] == ".clear") {
		score[target] = {};
		log[target] = [];
		nickLog[target] = [];
		IRC.msg(target, "Cleared log");
	}
	else if (split[0] == ".status") {
		if (log[target] == null) {
			return;
		}

		var countryCount = 0;
		var capitalCount = 0;

		for (var i in log[target]) {
			var m = log[target][i];
			if (countries[m] != null) {
				countryCount++;
			} 
			else {
				capitalCount++;
			}
		}

		var msg = "There are " + setSize + " countries in the database. ";
		msg += "Completed " + countryCount + " countries (" + Math.round(countryCount/setSize*100) + "%) ";
		msg += "and " + capitalCount + " capitals (" + Math.round(capitalCount/setSize*100) + "%).";

		IRC.msg(target, msg);
	}
	else if (split[0] == ".check" && split.length > 1) {
		if (log[target] == null) {
			return;
		}

		var s = split[1];
		var doneCapitals = 0, doneCountries = 0;
		for (var i in log[target]) {
			var m = log[target][i];
			if (m.indexOf(s) == 0) {
				if (countries[m] != null) {
					doneCountries++;
				} 
				else {
					doneCapitals++;
				}
			}
		}

		var countryCount = 0, capitalCount = 0;
		for (var country in countries) {
			if (country.indexOf(s) == 0) {
				countryCount++;
			}

			var capital = countries[country];
			if (capital.indexOf(s) == 0) {
				capitalCount++;
			}
		}

		var msg = "Checking for capitals and countries starting with " + s + ": ";
		if (countryCount > 0) {
			msg += doneCountries + " out of " + countryCount + " countries (" + Math.round(doneCountries/countryCount*100) + "%). ";
		}
		if (capitalCount > 0) {
			msg += doneCapitals + " out of " + capitalCount + " capitals (" + Math.round(doneCapitals/capitalCount*100) + "%).";
		}
		if (countryCount == 0 && capitalCount == 0) {
			msg += "No hits!";
		}

		IRC.msg(target, msg);
	}
	else if (split[0] == ".random") {
		var rand = Math.floor(Math.random()*setSize*2);
		var i = 0;
		for (var country in countries) {
			if (i == rand) {
				var msg = "Random: " + country;
				IRC.msg(target, msg);
				return;
			}
			i++;
		}

		for (var capital in capitals) {
			if (i == rand) {
				var msg = "Random: " + capital;
				IRC.msg(target, msg);
				break;
			}
			i++;
		}
	}
	else if (split[0] == ".countryhelp") {
		var msg = "[Commands in the country game] Searches: ";
		msg += "capital [country], ";
		msg += "country [capital], ";
		msg += "find [query], random ";
		msg += "Game: play [guess] or p [guess], ";
		msg += "log, sortedlog, status, ";
		msg += "check [letter], clear";

		IRC.msg(target, msg);
	}
}

for (var country in countries) {
	var capital = countries[country];
	capitals[capital] = country;
	setSize++;
}

Bot.addCallback("privmsg", game);

