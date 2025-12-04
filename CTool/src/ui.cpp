#include "ui.h"
#include "graph.h"
#include "imgui.h"
#include "imnodes.h"
#include <string>
#include <algorithm>

void RenderUI() {

    auto create_audio_node = [&](const std::string& name, const std::string& path) {
        AudioNode n = CreateAudioNode(name, path);
        audioNodes.push_back(n);
        };

    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("Nodos")) {
            if (ImGui::MenuItem("Intro")) create_audio_node("Intro", "../assets/0_intro.wav");
            if (ImGui::MenuItem("First Piano")) create_audio_node("First Piano", "../assets/1_firstPiano.wav");
            if (ImGui::MenuItem("Ding ding")) create_audio_node("Ding ding", "../assets/2_dingDing.wav");
            if (ImGui::MenuItem("This is crazy frog")) create_audio_node("This is crazy frog", "../assets/3_thisIsCrazyFrog.wav");
            if (ImGui::MenuItem("Body song")) create_audio_node("Body song", "../assets/4_mainSong.wav");
            if (ImGui::MenuItem("Motillo")) create_audio_node("Motillo", "../assets/5_motillo.wav");
            if (ImGui::MenuItem("Condition")) conditionNodes.push_back(CreateConditionNode("New Condition", false));
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    ImNodes::BeginNodeEditor();

    for (auto& n : audioNodes) {
        if (n.to_delete) continue;
        ImNodes::BeginNode(n.id);
        ImNodes::BeginNodeTitleBar();
        ImGui::Text("%s", n.name.c_str());
        ImNodes::EndNodeTitleBar();
        ImGui::Text("Audio Index: %d", n.audioIndex);
        if (n.id == currentPlayingNodeId) ImGui::TextColored({ 0,1,0,1 }, "-> PLAYING");
        if (ImGui::Button(("Play##" + std::to_string(n.id)).c_str())) {
            if (currentPlayingNodeId != -1) {
                AudioNode* prev = FindNodeById(currentPlayingNodeId);
                if (prev) audioManager.Stop(prev->audioIndex);
            }
            audioManager.Play(n.audioIndex, false);
            currentPlayingNodeId = n.id;
        }
        ImGui::SameLine();
        if (ImGui::Button(("Delete##" + std::to_string(n.id)).c_str())) {
            if (n.id == currentPlayingNodeId) {
                audioManager.Stop(n.audioIndex);
                currentPlayingNodeId = -1;
            }
            n.to_delete = true;
        }
        ImNodes::BeginInputAttribute(n.inputPin);
        ImGui::Text("Input");
        ImNodes::EndInputAttribute();
        ImNodes::BeginOutputAttribute(n.outputPin);
        ImGui::Text("Output");
        ImNodes::EndOutputAttribute();
        ImNodes::EndNode();
    }

    for (auto& n : conditionNodes) {
        if (n.to_delete) continue;
        ImNodes::BeginNode(n.id);
        ImNodes::BeginNodeTitleBar();
        ImGui::Text("Condition Node");
        ImNodes::EndNodeTitleBar();
        char buf[256];
        std::snprintf(buf, 256, "%s", n.conditionText.c_str());
        ImGui::InputText("Text", buf, 256);
        n.conditionText = buf;
        ImGui::Checkbox("Value", &n.conditionValue);
        ImNodes::BeginInputAttribute(n.inputPin1);
        ImGui::Text("Input 1");
        ImNodes::EndInputAttribute();
        ImNodes::BeginInputAttribute(n.inputPin2);
        ImGui::Text("Input 2");
        ImNodes::EndInputAttribute();
        ImNodes::BeginOutputAttribute(n.outputPin);
        ImGui::Text("Output");
        ImNodes::EndOutputAttribute();
        ImNodes::EndNode();
    }

    for (const auto& l : links) ImNodes::Link(l.id, l.startAttr, l.endAttr);

    ImNodes::EndNodeEditor();

    int startAttr, endAttr;
    if (ImNodes::IsLinkCreated(&startAttr, &endAttr)) {
        if (!LinkExists(startAttr, endAttr)) links.push_back({ nextLinkId++, startAttr, endAttr });
    }

    int destroyed;
    if (ImNodes::IsLinkDestroyed(&destroyed)) {
        links.erase(std::remove_if(links.begin(), links.end(), [&](const Link& l) { return l.id == destroyed; }), links.end());
    }

    links.erase(std::remove_if(links.begin(), links.end(), [&](const Link& l) {
        for (auto& n : audioNodes)
            if (n.to_delete && (l.startAttr == n.inputPin || l.startAttr == n.outputPin || l.endAttr == n.inputPin || l.endAttr == n.outputPin)) return true;
        for (auto& n : conditionNodes)
            if (n.to_delete && (l.startAttr == n.inputPin1 || l.startAttr == n.inputPin2 || l.startAttr == n.outputPin ||
                l.endAttr == n.inputPin1 || l.endAttr == n.inputPin2 || l.endAttr == n.outputPin)) return true;
        return false;
        }), links.end());

    audioNodes.erase(std::remove_if(audioNodes.begin(), audioNodes.end(), [](const AudioNode& n) { return n.to_delete; }), audioNodes.end());
    conditionNodes.erase(std::remove_if(conditionNodes.begin(), conditionNodes.end(), [](const ConditionNode& n) { return n.to_delete; }), conditionNodes.end());
}
