#include "graph.h"
#include <algorithm>

std::vector<AudioNode> audioNodes;
std::vector<ConditionNode> conditionNodes;
int nextId = 100;
int nextLinkId = 1;
int currentPlayingNodeId = -1;
AudioManager audioManager;

AudioNode* FindNodeById(int nodeId) {
    for (auto& n : audioNodes)
        if (n.id == nodeId) return &n;
    return nullptr;
}

ConditionNode* FindConditionNodeById(int nodeId) {
    for (auto& n : conditionNodes)
        if (n.id == nodeId) return &n;
    return nullptr;
}

bool LinkExists(int startAttr, int endAttr) {
    for (const auto& l : links)
        if (l.startAttr == startAttr && l.endAttr == endAttr)
            return true;
    return false;
}

AudioNode CreateAudioNode(const std::string& name, const std::string& path) {
    AudioNode n;
    n.id = nextId++;
    n.inputPin = nextId++;
    n.outputPin = nextId++;
    n.name = name;
    n.audioIndex = audioManager.LoadWav(path.c_str());
    return n;
}

ConditionNode CreateConditionNode(const std::string& text, bool value) {
    ConditionNode n;
    n.id = nextId++;
    n.inputPin1 = nextId++;
    n.inputPin2 = nextId++;
    n.outputPin = nextId++;
    n.conditionText = text;
    n.conditionValue = value;
    return n;
}
