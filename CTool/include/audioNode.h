#ifndef AUDIO_NODE_H
#define AUDIO_NODE_H

#include <string>

struct AudioNode {
    int id = -1;
    int inputPin = -1;
    int outputPin = -1;
    int audioIndex = -1;
    bool to_delete = false;
    std::string name = "";
};


struct ConditionNode {
    int id = -1;
    int inputPin1 = -1;
    int inputPin2 = -1;
    int outputPin = -1;     
    std::string conditionText = "";
    bool conditionValue = false;
    bool to_delete = false;
};

#endif
