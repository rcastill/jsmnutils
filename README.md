# JSMN Utils

This is a C++ (started as a C prototype) utility to map read-only JSON representations. At its
core it uses [zserge/jsmn](https://github.com/zserge/jsmn) as the tokenizer.

This project is intended to be kept simple in order to be easy to use while avoiding bloat code.

## Building

This is a standard CMake based project:

```bash
mkdir build
cd build
cmake ..
make
```

## Example of use

Given this sample:

```json
{
    "bind": "tcp://localhost",

    "flows": [
        {
            "name": "flow1",
            "port": 5000,
            "file": "movie.mp4"
        },
        {
            "name": "flow2",
            "port": 5428,
            "file": "Devin Townsend.mkv"
        }
    ]
}
```

The essential code needed to parse it would be:

```c++
JSONParser parser;
parser.load_file("<my_file>");
JSONElement element = parser.parser();

JSONObject root = element;
std::string bind = root["bind"];

JSONArray flows = root["flows"];
for (int i = 0; i < flows.size(); i++) {
    JSONObject flow = flows[i];
    std::string name = flow["name"];
    int port = flow["port"];
    std::string file = flow["file"];
}
```

A full example can be found at `test/test1.cpp`.
