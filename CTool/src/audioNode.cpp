#include <string>

struct AudioNode {
    int id = -1;
    int inputPin = -1;
    int outputPin = -1;
    int audioIndex = -1;
    bool to_delete = false;
    std::string name = "";
};


AudioNode MakeAudioNode(int id, int inputPin, int outputPin, int audioIndex, const std::string& name) {
    AudioNode n;
    n.id = id;
    n.inputPin = inputPin;
    n.outputPin = outputPin;
    n.audioIndex = audioIndex;
    n.name = name;
    return n;
}
