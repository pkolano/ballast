# Ballast

NASA's Dynamic SSH Load Balancer: Ballast (*BAL*ancing *L*oad *A*cross *S*ys*T*ems)

## Info

Ballast is a tool for balancing user load across SSH servers.
Supports metric based policies (CPU load, etc) and/or user-specific policies.
It includes a simple client, a lightweight server and a data collection agent.

* Homepage: [people.nas.nasa.gov/~kolano/projects/ballast.html](http://people.nas.nasa.gov/~kolano/projects/ballast.html)
* License: [NASA Open Source Agreement Version 1.3](https://en.wikipedia.org/wiki/NASA_Open_Source_Agreement)
* Upstream: [Ballast Sourceforge Project](http://sourceforge.net/projects/ballast/files/)

## Balancing Load Across Systems (Ballast)

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
to the Ballast server.  The server aggregates the load data received from
all agents and stores it in a suitable form for making balancing
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

## Details

* Architecture (PDF): [doc/ballast.pdf](doc/ballast.pdf)
* Installation Details: [INSTALL](INSTALL)
* Changelog: [CHANGES](CHANGES)
* Usage details: [doc/usage.txt](doc/usage.txt)
* Policy syntax: [doc/policy.txt](doc/policy.txt)

## Diagram

![architecture diagram](https://cloud.githubusercontent.com/assets/145113/12710216/364e8014-c869-11e5-9aaf-0999a6d05903.png)

## Etc

Questions, comments, fixes, and/or enhancements welcome.
Contact [Paul Kolano](http://people.nas.nasa.gov/~kolano/).

This is an unofficial repo which tracks changes from the [offical ballast release tarballs](http://sourceforge.net/projects/ballast/files/).
