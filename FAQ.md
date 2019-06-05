# Frequently Asked Questions


## how to start that thing?
`docker run -it --rm -p 4242:4242 cursate`
it will then listen for incoming TCP stream of JSONs on port `4242`.

## what is the required input format?
TCP stream of JSONs.
that's it.
no need for any extra new lines, or any particular formatting.
if parsing failed along the ride, it will auto-fix and try starting from next working JSON.

to end the transmission just close the socket.


## broken display under screen
try setting TERM to sth useful:
`docker run -e TERM=screen-256color -it --rm -p 4242:4242 cursate`


## broken display under tmux
never heard of - sorry... :P


## broken display under PuTTY
try setting TERM to sth useful:
`docker run -e TERM=linux -it --rm -p 4242:4242 cursate`
probably not the best choice, but it's usable at least...

for better support try starting `tmux` (or `screen` with fixed `$TERM` setting) and then run from there.
