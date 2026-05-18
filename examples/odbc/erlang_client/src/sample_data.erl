-module(sample_data).
-export([series/0, seasons/0, episodes/0, format_date/1]).

series() ->
    [
        [1, "IT Crowd", "The IT Crowd is a British sitcom by Channel 4.", days_from_date({2006, 2, 3})],
        [2, "Silicon Valley", "Silicon Valley is an American comedy series.", days_from_date({2014, 4, 6})]
    ].

seasons() ->
    [
        [1, 1, "Season 1", days_from_date({2006, 2, 3}), days_from_date({2006, 5, 5})],
        [1, 2, "Season 2", days_from_date({2007, 8, 24}), days_from_date({2007, 11, 16})],
        [1, 3, "Season 3", days_from_date({2008, 11, 21}), days_from_date({2008, 12, 26})],
        [1, 4, "Season 4", days_from_date({2010, 6, 25}), days_from_date({2010, 7, 30})],
        [2, 1, "Season 1", days_from_date({2014, 4, 6}), days_from_date({2014, 6, 15})],
        [2, 2, "Season 2", days_from_date({2015, 4, 12}), days_from_date({2015, 6, 14})],
        [2, 3, "Season 3", days_from_date({2016, 4, 24}), days_from_date({2016, 6, 26})],
        [2, 4, "Season 4", days_from_date({2017, 4, 23}), days_from_date({2017, 6, 25})],
        [2, 5, "Season 5", days_from_date({2018, 3, 25}), days_from_date({2018, 5, 13})],
        [2, 6, "Season 6", days_from_date({2019, 10, 27}), days_from_date({2019, 12, 8})]
    ].

episodes() ->
    [
        [1, 1, 1, "Yesterday's Jam", days_from_date({2006, 2, 3})],
        [1, 1, 2, "Calamity Jen", days_from_date({2006, 2, 10})],
        [1, 1, 3, "Fifty-Fifty", days_from_date({2006, 2, 17})],
        [1, 1, 4, "The Red Door", days_from_date({2006, 2, 24})],
        [1, 1, 5, "The Haunting of Bill Crouse", days_from_date({2006, 3, 3})],
        [1, 1, 6, "Aunt Irma Visits", days_from_date({2006, 3, 10})],
        [1, 2, 1, "The Work Outing", days_from_date({2007, 8, 24})],
        [1, 2, 2, "Return of the Golden Child", days_from_date({2007, 8, 31})],
        [1, 2, 3, "Moss and the German", days_from_date({2007, 9, 7})],
        [2, 1, 1, "Minimum Viable Product", days_from_date({2014, 4, 6})],
        [2, 1, 2, "The Cap Table", days_from_date({2014, 4, 13})],
        [2, 1, 3, "Articles of Incorporation", days_from_date({2014, 4, 20})],
        [2, 1, 4, "Fiduciary Duties", days_from_date({2014, 4, 27})],
        [2, 1, 5, "Signaling Risk", days_from_date({2014, 5, 4})],
        [2, 3, 1, "Founder Friendly", days_from_date({2016, 4, 24})],
        [2, 3, 2, "Two in the Box", days_from_date({2016, 5, 1})],
        [2, 3, 3, "Meinertzhagen's Haversack", days_from_date({2016, 5, 8})],
        [2, 3, 4, "Maleant Data Systems Solutions", days_from_date({2016, 5, 15})],
        [2, 5, 1, "Grow Fast or Die Slow", days_from_date({2018, 3, 25})],
        [2, 5, 2, "Reorientation", days_from_date({2018, 4, 1})],
        [2, 5, 3, "Chief Operating Officer", days_from_date({2018, 4, 8})],
        [2, 5, 4, "Tech Evangelist", days_from_date({2018, 4, 15})],
        [2, 5, 5, "Facial Recognition", days_from_date({2018, 4, 22})],
        [2, 6, 1, "Artificial Emotional Intelligence", days_from_date({2019, 10, 27})],
        [2, 6, 2, "Blood Money", days_from_date({2019, 11, 3})],
        [2, 6, 3, "Hooli Smokes!", days_from_date({2019, 11, 10})],
        [2, 6, 4, "Maximizing Alphaness", days_from_date({2019, 11, 17})],
        [2, 6, 5, "Tethics", days_from_date({2019, 11, 24})],
        [2, 6, 6, "RussFest", days_from_date({2019, 12, 1})],
        [2, 6, 7, "Exit Event", days_from_date({2019, 12, 8})]
    ].

days_from_date({Year, Month, Day}) ->
    calendar:date_to_gregorian_days(Year, Month, Day) - calendar:date_to_gregorian_days(1970, 1, 1).

format_date(Days) ->
    Date = calendar:gregorian_days_to_date(Days + calendar:date_to_gregorian_days(1970, 1, 1)),
    {Year, Month, Day} = Date,
    io_lib:format("~4..0B-~2..0B-~2..0B", [Year, Month, Day]).
