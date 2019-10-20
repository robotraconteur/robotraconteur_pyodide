<p align="center"><img src="https://robotraconteurpublicfiles.s3.amazonaws.com/RRheader2.jpg"></p>

# Robot Raconteur for Pyodide

**WARNING: This does not work yet!**

Robot Raconteur for Pyodide is a heavily modified fork of the Robot Raconteur core libraries intended to run on Pyodide within a browser.

https://github.com/iodide-project/pyodide

Several changes are necessary to build with Emscripten and run in a browser environment:

* Remove all references to boost\_thread, boost\_atomic, and boost\_asio, and run single-threaded. 
* Remove all blocking functions from C++ and Python
* Implement a transport that uses Emscripten WebSockets
* Use Emscripten functions for timers, post, and communication

## License

The Robot Raconteur core library is Apache 2.0 licensed.

"Robot Raconteur" and the Robot Raconteur logo are registered trademarks of Wason Technology, LLC. All rights reserved.

Robot Raconteur is patent pending.

Robot Raconteur is developed by Dr. John Wason, Wason Technology, LLC
