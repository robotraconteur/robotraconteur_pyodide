import os
import sys
import argparse
import http.server
import socketserver
import pathlib

TEST_PATH = os.path.dirname(os.path.abspath(__file__)) + "/test"
ROOT_PATH = os.path.dirname(os.path.abspath(__file__)) + "/serve_root"
BUILD_PATH =  os.path.abspath(os.path.dirname(os.path.abspath(__file__)) + '/../../../pyodide/dist')

assert os.path.isfile(BUILD_PATH + "/pyodide.js"), "Pyodide dist dir not found"


class Handler(http.server.SimpleHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, directory=args[2].root_dir, **kwargs)
    def end_headers(self):
        self.send_header('Access-Control-Allow-Origin', '*')
        super().end_headers()
    def translate_path(self, path):
        if self.path.startswith('/build'):
            if self.path == '/build' or self.path == '/build/':
                return self.server.build_dir
            else:
                return self.server.build_dir + path[len('/build'):]
        elif self.path.startswith('/test'):
            if self.path == '/test' or self.path == '/test/':
                return self.server.test_dir
            else:
                return self.server.test_dir + path[len('/test'):]
        else:
            return super().translate_path(path)


class Server(socketserver.TCPServer):
    def __init__(self, host_port_tuple, streamhandler, root_dir, build_dir, test_dir):
        super().__init__(host_port_tuple, streamhandler)
        self.root_dir = root_dir
        self.build_dir = build_dir
        self.test_dir = test_dir


Handler.extensions_map['.wasm'] = 'application/wasm'


def make_parser(parser):
    parser.description = ('Start a server with the supplied '
                          'build_dir, test_dir, root_dir, and port.')
    parser.add_argument('--root_dir', action='store', type=str,
                        default=ROOT_PATH, help='set the server root path')
    parser.add_argument('--build_dir', action='store', type=str,
                        default=BUILD_PATH, help='set the Pyodide build directory containing WASM files')
    parser.add_argument('--test_dir', action='store', type=str,
                        default=TEST_PATH, help='set the Pyodide directory containing test files')
    parser.add_argument('--port', action='store', type=int,
                        default=8000, help='set the PORT number')
    return parser


def server(port, root_dir, build_dir, test_dir):
    httpd = Server(('', port), Handler, root_dir, build_dir, test_dir)
    return httpd


def main(args):
    root_dir = args.root_dir
    build_dir = args.build_dir
    test_dir = args.test_dir
    port = args.port
    httpd = server(port, root_dir, build_dir, test_dir)
    
    print("serving from {0} at localhost:".format(build_dir) + str(port))
    try:
        httpd.serve_forever()
    except KeyboardInterrupt:
        print("\n...shutting down http server")
        httpd.shutdown()
        sys.exit()


if __name__ == "__main__":
    parser = make_parser(argparse.ArgumentParser())
    args = parser.parse_args()
    main(args)
