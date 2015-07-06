An example Pure Data external using [pd-lib-builder](https://github.com/pure-data/pd-lib-builder). You can use this project to bootstrap your own Pure Data external development.

	$ git clone --recursive https://github.com/pure-data/helloworld.git
	$ cd helloworld
	$ make pdincludepath=/path/to/pure-data/src

Make sure you use the `--recursive` flag when checking out the repository so that the pd-lib-builder dependency is also checked out.

You can also issue `git submodule init; git submodule update` to fetch the dependency.
