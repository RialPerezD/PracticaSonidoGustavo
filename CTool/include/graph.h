#ifndef GRAPH_H
#define GRAPH_H

#include <vector>
#include <string>
#include "link.h"
#include "sound.h"
#include <audioNode.h>

extern std::vector<AudioNode> audioNodes;
extern std::vector<Link> links;
extern std::vector<ConditionNode> conditionNodes;
extern int nextId;
extern int nextLinkId;
extern int currentPlayingNodeId;
extern AudioManager audioManager;

AudioNode* FindNodeById(int nodeId);
bool LinkExists(int startAttr, int endAttr);

AudioNode CreateAudioNode(const std::string& name, const std::string& path);
ConditionNode CreateConditionNode(const std::string& text, bool value);


#endif
