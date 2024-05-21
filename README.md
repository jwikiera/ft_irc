# ft_irc

## What is this?
The laziest irc server implementation you will find. It is a 42 school project and you will find the details in the
subject pdf. TL:DR: write an irc server in c++98 handling multiple clients using select, poll, or an equivalent.
I used the `irssi` client and the `ngircd` server in addition to the irc specification in order to know what my own
server will be sending to its clients. I use socat as a proxy to know exactly what data is exchanged. One could call
it reverse engineering but easy.
