Balancing Load Across Systems (Ballast)
=======================================

Ballast is a framework that was specifically designed for SSH load
balancing by taking advantage of the aliasing and proxying capabilities
available in most SSH clients.  Within a traditional SSH bastion
architecture, where internal resources can only be accessed from the
outside via a hardened bastion system, Ballast is completely transparent
to users and does not require any external modifications.  Bastions and
internal hosts must have the Ballast client and a pipe-based TCP relay
utility such as netcat installed, which can be easily managed across
internal hosts by configuration management tools.

Ballast consists of a Ballast agent on each balanced host, a Ballast
server on one or more data servers, and a Ballast client on each bastion
and internal host from which the user might access balanced hosts.  The
Ballast agent periodically collects system load information and sends it
to the Ballast server.  The server aggregates the load data received
from all agents and stores it in a suitable form for making balancing
decisions.  When the user invokes SSH to a host alias designated for
balancing, SSH triggers the Ballast client, which contacts the server to
resolve balancing aliases to actual balanced host names.  The server
consults its data and returns one or more hosts, one of which the client
connects to via netcat (or equivalent), after which the login proceeds
normally.

Unlike traditional load balancers, Ballast has the ability to adjust
its balancing strategy based on the invoking user.  By tailoring system
selections specifically to each individual user, the utility to each
user can be maximized instead of choosing a system that may be best
according to a common metric, but not ideal for the user's needs.

Ballast is in active production at the NASA Advanced Supercomputing
Facility (https://www.nas.nasa.gov/hecc/support/kb/entry/233) and has 
handled over 75M balancing requests (as of May 2018) since deployment
in June 2009.

For full details of the Ballast architecture, see
https://pkolano.github.io/papers/iscc10.pdf.  For installation details,
see "INSTALL".  For usage details, see "doc/usage.txt".  For balancing
policy syntax, see "doc/policy.txt".

Questions, comments, fixes, and/or enhancements welcome.

--Paul Kolano <paul.kolano@nasa.gov>

