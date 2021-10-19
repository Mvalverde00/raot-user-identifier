# Motivation
In RAoT, players are free to change their usernames as often as they wish, even to other players' usernames.
This can be quite problematic, as you may have people impersonating others or pretending to be a Trainee and then top fragging.

Fortunately, all players also have an immutable GUID which uniquely identifies them, regardless of what they set their username to.  This GUID is publicly available to anyone in the lobby.

This program allows you identify users by analyzing the game's network traffic to automatically read (username, GUID) pairs of all players who connect to the lobby. These pairs are
then coalesced on a server along with information on how long the user was connected with the given username, to give a measure of confidence for the username.
A simple front end website allows the user to query for specific GUIDs and see all associated usernames which have been recorded. Thus, you can verify if someone is who they claim
to be, or if that Trainee who hits one too many shots is really a pro who has been playing for months.

To date, 1636 unique (username, GUID) pairs have been recorded!

A live demo of the website front end is available at http://137.184.17.37/

# Specifics
My analysis of the game traffic is incomplete, and there are a fair bit of complications as to how details are being serialized-- it seems like sometimes, information 
appears in different locations (although this is surely impossible and there must be some underlying pattern which governs the data layout).  From my efforts, I was able
to identify a "player join packet" and "player disconnect packet", which allows me to collect a (username, GUID) pair and track how long they are connected for.

I have also identified a "player killed packet" and "player parried packet", although unfortunately I can not discern how they are being encoded (the killing/killed players seem
to be encoded using some sort of ID which I can't link to the username/GUID).  If I can figure this out, I could add stat tracking to the website (e.g. K/D ratio, most kills, etc.).
