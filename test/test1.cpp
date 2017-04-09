#include <iostream>
#include "jsmnutils.h"

void show_flow(const JSONObject &flow)
{
    std::string name = flow["name"];
    int port = flow["port"];
    std::string file = flow["file"];

    std::cout << "[" << name << "]" << std::endl;
    std::cout << "port = " << port << std::endl;
    std::cout << "file = " << file << std::endl;
}

int main(int argc, const char *argv[])
{
    JSONParser parser;
    parser.load_file("input_prototype.json");
    JSONElement element = parser.parse();

    try {
        JSONObject object = element;
        std::string bind = object["bind"];
        std::cout << "g:bind = " <<
            bind << std::endl << std::endl;
        
        JSONArray flows = object["flows"];
        std::cout << "Flows"
            << std::endl << "====="
            << std::endl << std::endl;

        for (int i = 0; i < flows.size(); i++) {
            JSONObject flow = flows[i];
            show_flow(flow);
            std::cout << std::endl;
        }
    
    } catch(JSMNUtilsEx &ex) {
        std::cout << ex.what() << std::endl;
    }

    return 0;
}
