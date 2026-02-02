#include "sequencerGui.h"
#include "sequencerMain.h"
#include <imgui_internal.h>

#include <algorithm>
#include <string>
#include <cctype>

//------------------------------------------------------------------------------
void OhmfluxTrackerGui::DrawAlgorithmHoverFunc(const opl3::Instrument inst){
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort)) {
        if (ImGui::BeginTooltip())
        {
            if (inst.isFourOp)
            {
                int algo = (inst.pairs[0].connection) | (inst.pairs[1].connection << 1);
                switch (algo) {
                    case 0: Draw4OP_Algorithm0Overlay(); break;
                    case 1: Draw4OP_Algorithm1Overlay(); break;
                    case 2: Draw4OP_Algorithm2Overlay(); break;
                    case 3: Draw4OP_Algorithm3Overlay(); break;
                }
            } else {
                if (inst.pairs[0].connection == 0) Draw2OP_FM_Overlay();
                else Draw2OP_Additive_Overlay();
            }
            ImGui::EndTooltip();
        }
    }
}
//------------------------------------------------------------------------------
void OhmfluxTrackerGui::Draw4OP_Algorithm0Overlay(float nodeSize, float spacing) {

    ImGui::Text("Algorithm 0: Serial FM Chain (1 -> 2 -> 3 -> 4)");
    ImGui::Separator();

    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 startPos = ImGui::GetCursorScreenPos();
    startPos.x += 10; startPos.y += 10;

    ImU32 colorMod = ImColor(50, 100, 200);     // Blue (Modulators)
    ImU32 colorCarrier = ImColor(200, 100, 50); // Orange (Final Carrier)
    ImU32 colorLine = ImColor(200, 200, 200, 200);

    auto drawOp = [&](ImVec2 pos, const char* label, bool isCarrier) {
        drawList->AddRectFilled(pos, {pos.x + nodeSize, pos.y + nodeSize}, isCarrier ? colorCarrier : colorMod, 4.0f);
        drawList->AddRect(pos, {pos.x + nodeSize, pos.y + nodeSize}, ImColor(255, 255, 255), 4.0f);
        ImVec2 textSize = ImGui::CalcTextSize(label);
        drawList->AddText({pos.x + (nodeSize - textSize.x) * 0.5f, pos.y + (nodeSize - textSize.y) * 0.5f}, ImColor(255, 255, 255), label);
    };

    // Node Positions in a single horizontal line
    ImVec2 ops[4];
    for(int i = 0; i < 4; i++) {
        ops[i] = { startPos.x + i * (nodeSize + spacing), startPos.y };
    }

    // Draw Lines between operators
    for(int i = 0; i < 3; i++) {
        drawList->AddLine({ops[i].x + nodeSize, ops[i].y + nodeSize/2}, {ops[i+1].x, ops[i+1].y + nodeSize/2}, colorLine, 2.0f);
    }

    // Final Audio Out line from Op 4
    float outX = ops[3].x + nodeSize + 20;
    drawList->AddLine({ops[3].x + nodeSize, ops[3].y + nodeSize/2}, {outX, ops[3].y + nodeSize/2}, colorLine, 2.0f);

    // Draw Nodes (1, 2, 3 are modulators; 4 is carrier)
    drawOp(ops[0], "1", false);
    drawOp(ops[1], "2", false);
    drawOp(ops[2], "3", false);
    drawOp(ops[3], "4", true);

    // Reserve space: 4 nodes + 3 gaps + padding
    ImGui::Dummy(ImVec2(4 * nodeSize + 3 * spacing + 30, nodeSize + 20));


}
//------------------------------------------------------------------------------
void OhmfluxTrackerGui::Draw4OP_Algorithm1Overlay(float nodeSize, float spacing) {

    ImGui::Text("Algorithm 1: (1 -> 2 -> 3) + 4");
    ImGui::Separator();
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 start = ImGui::GetCursorScreenPos();
    start.x += 10; start.y += 10;

    ImU32 colMod = ImColor(50, 100, 200); ImU32 colCar = ImColor(200, 100, 50);
    ImU32 colLine = ImColor(200, 200, 200, 200);

    auto drawOp = [&](ImVec2 p, const char* l, bool c) {
        drawList->AddRectFilled(p, {p.x+nodeSize, p.y+nodeSize}, c ? colCar : colMod, 4.0f);
        drawList->AddRect(p, {p.x+nodeSize, p.y+nodeSize}, ImColor(255,255,255), 4.0f);
        ImVec2 ts = ImGui::CalcTextSize(l);
        drawList->AddText({p.x+(nodeSize-ts.x)*0.5f, p.y+(nodeSize-ts.y)*0.5f}, ImColor(255,255,255), l);
    };

    ImVec2 op1 = start;
    ImVec2 op2 = {start.x + nodeSize + spacing, start.y};
    ImVec2 op3 = {start.x + (nodeSize + spacing) * 2, start.y};
    ImVec2 op4 = {start.x + (nodeSize + spacing) * 2, start.y + nodeSize + spacing};

    // Lines 1->2->3
    drawList->AddLine({op1.x+nodeSize, op1.y+nodeSize/2}, {op2.x, op2.y+nodeSize/2}, colLine, 2.0f);
    drawList->AddLine({op2.x+nodeSize, op2.y+nodeSize/2}, {op3.x, op3.y+nodeSize/2}, colLine, 2.0f);

    // Final Outputs
    float outX = op3.x + nodeSize + 20;
    drawList->AddLine({op3.x+nodeSize, op3.y+nodeSize/2}, {outX, op3.y+nodeSize/2}, colLine, 2.0f);
    drawList->AddLine({op4.x+nodeSize, op4.y+nodeSize/2}, {outX, op4.y+nodeSize/2}, colLine, 2.0f);
    drawList->AddLine({outX, op3.y+nodeSize/2}, {outX, op4.y+nodeSize/2}, colLine, 2.0f);

    drawOp(op1, "1", false); drawOp(op2, "2", false); drawOp(op3, "3", true); drawOp(op4, "4", true);
    ImGui::Dummy(ImVec2((nodeSize + spacing) * 3 + 20, (nodeSize + spacing) * 2));
}
//------------------------------------------------------------------------------
void OhmfluxTrackerGui::Draw4OP_Algorithm2Overlay(float nodeSize, float spacing) {

    ImGui::Text("Algorithm 2: 1 + (2 -> 3 -> 4)");
    ImGui::Separator();
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 start = ImGui::GetCursorScreenPos();
    start.x += 10; start.y += 10;

    ImU32 colMod = ImColor(50, 100, 200); ImU32 colCar = ImColor(200, 100, 50);
    ImU32 colLine = ImColor(200, 200, 200, 200);

    auto drawOp = [&](ImVec2 p, const char* l, bool c) {
        drawList->AddRectFilled(p, {p.x+nodeSize, p.y+nodeSize}, c ? colCar : colMod, 4.0f);
        drawList->AddRect(p, {p.x+nodeSize, p.y+nodeSize}, ImColor(255,255,255), 4.0f);
        ImVec2 ts = ImGui::CalcTextSize(l);
        drawList->AddText({p.x+(nodeSize-ts.x)*0.5f, p.y+(nodeSize-ts.y)*0.5f}, ImColor(255,255,255), l);
    };

    ImVec2 op1 = {start.x + (nodeSize + spacing) * 2, start.y};
    ImVec2 op2 = start;
    ImVec2 op3 = {start.x + nodeSize + spacing, start.y + nodeSize + spacing}; // Offset for visual clarity
    ImVec2 op4 = {start.x + (nodeSize + spacing) * 2, start.y + nodeSize + spacing};

    // Lines 2->3->4
    drawList->AddLine({op2.x+nodeSize, op2.y+nodeSize/2}, {op3.x, op3.y-nodeSize/2}, colLine, 2.0f); // Diagonal logic
    drawList->AddLine({op3.x+nodeSize, op3.y+nodeSize/2}, {op4.x, op4.y+nodeSize/2}, colLine, 2.0f);

    // Final Outputs
    float outX = op1.x + nodeSize + 20;
    drawList->AddLine({op1.x+nodeSize, op1.y+nodeSize/2}, {outX, op1.y+nodeSize/2}, colLine, 2.0f);
    drawList->AddLine({op4.x+nodeSize, op4.y+nodeSize/2}, {outX, op4.y+nodeSize/2}, colLine, 2.0f);
    drawList->AddLine({outX, op1.y+nodeSize/2}, {outX, op4.y+nodeSize/2}, colLine, 2.0f);

    drawOp(op1, "1", true); drawOp(op2, "2", false); drawOp(op3, "3", false); drawOp(op4, "4", true);
    ImGui::Dummy(ImVec2((nodeSize + spacing) * 3 + 20, (nodeSize + spacing) * 2));
}
//------------------------------------------------------------------------------
void OhmfluxTrackerGui::Draw4OP_Algorithm3Overlay(float nodeSize, float spacing) {

    ImGui::Text("Algorithm 3: Parallel 2-OP Pairs");
    ImGui::Separator();

    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 startPos = ImGui::GetCursorScreenPos();
    startPos.x += 10; startPos.y += 10; // Padding inside tooltip

    ImU32 colorNode = ImColor(50, 100, 200);    // Blue (Modulator)
    ImU32 colorCarrier = ImColor(200, 100, 50); // Orange (Carrier)
    ImU32 colorLine = ImColor(200, 200, 200, 200);

    auto drawOp = [&](ImVec2 pos, const char* label, bool isCarrier) {
        drawList->AddRectFilled(pos, {pos.x + nodeSize, pos.y + nodeSize}, isCarrier ? colorCarrier : colorNode, 4.0f);
        drawList->AddRect(pos, {pos.x + nodeSize, pos.y + nodeSize}, ImColor(255, 255, 255), 4.0f);
        ImVec2 textSize = ImGui::CalcTextSize(label);
        drawList->AddText({pos.x + (nodeSize - textSize.x) * 0.5f, pos.y + (nodeSize - textSize.y) * 0.5f}, ImColor(255, 255, 255), label);
    };

    // Node Positions
    ImVec2 op1 = startPos;
    ImVec2 op2 = {startPos.x + nodeSize + spacing, startPos.y};
    ImVec2 op3 = {startPos.x, startPos.y + nodeSize + spacing};
    ImVec2 op4 = {startPos.x + nodeSize + spacing, startPos.y + nodeSize + spacing};

    // Lines
    drawList->AddLine({op1.x + nodeSize, op1.y + nodeSize/2}, {op2.x, op2.y + nodeSize/2}, colorLine, 2.0f);
    drawList->AddLine({op3.x + nodeSize, op3.y + nodeSize/2}, {op4.x, op4.y + nodeSize/2}, colorLine, 2.0f);

    // Audio Out Visual
    float outX = op2.x + nodeSize + 15;
    drawList->AddLine({op2.x + nodeSize, op2.y + nodeSize/2}, {outX, op2.y + nodeSize/2}, colorLine, 2.0f);
    drawList->AddLine({op4.x + nodeSize, op4.y + nodeSize/2}, {outX, op4.y + nodeSize/2}, colorLine, 2.0f);
    drawList->AddLine({outX, op2.y + nodeSize/2}, {outX, op4.y + nodeSize/2}, colorLine, 2.0f);

    // Draw Nodes
    drawOp(op1, "1", false); drawOp(op2, "2", true);
    drawOp(op3, "3", false); drawOp(op4, "4", true);

    // Reserve space so the tooltip window sizes correctly
    ImGui::Dummy(ImVec2((nodeSize + spacing) * 2 + 20, (nodeSize + spacing) * 2 + 20));
}
//------------------------------------------------------------------------------
void OhmfluxTrackerGui::Draw2OP_Additive_Overlay(float nodeSize, float spacing) {

    ImGui::Text("2-OP Additiv: Op 1 + Op 2");
    ImGui::Separator();
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 start = ImGui::GetCursorScreenPos();
    start.x += 15; start.y += 15;

    ImU32 colCar = ImColor(200, 100, 50);
    ImU32 colLine = ImColor(200, 200, 200, 200);

    auto drawOp = [&](ImVec2 p, const char* l) {
        drawList->AddRectFilled(p, {p.x+nodeSize, p.y+nodeSize}, colCar, 4.0f);
        drawList->AddRect(p, {p.x+nodeSize, p.y+nodeSize}, ImColor(255,255,255), 4.0f);
        ImVec2 ts = ImGui::CalcTextSize(l);
        drawList->AddText({p.x+(nodeSize-ts.x)*0.5f, p.y+(nodeSize-ts.y)*0.5f}, ImColor(255,255,255), l);
    };

    ImVec2 op1 = start;
    ImVec2 op2 = {start.x, start.y + nodeSize + spacing};

    drawList->AddBezierQuadratic({op1.x+5, op1.y}, {op1.x+nodeSize/2, op1.y-15}, {op1.x+nodeSize-5, op1.y}, colLine, 2.0f);

    float outX = op1.x + nodeSize + 20;
    drawList->AddLine({op1.x+nodeSize, op1.y+nodeSize/2}, {outX, op1.y+nodeSize/2}, colLine, 2.0f);
    drawList->AddLine({op2.x+nodeSize, op2.y+nodeSize/2}, {outX, op2.y+nodeSize/2}, colLine, 2.0f);
    drawList->AddLine({outX, op1.y+nodeSize/2}, {outX, op2.y+nodeSize/2}, colLine, 2.0f);

    drawOp(op1, "1"); drawOp(op2, "2");
    ImGui::Dummy(ImVec2(nodeSize + 40, (nodeSize + spacing) * 2));
}
//------------------------------------------------------------------------------
void OhmfluxTrackerGui::Draw2OP_FM_Overlay(float nodeSize, float spacing) {

    ImGui::Text("2-OP FM: Modulator -> Carrier");
    ImGui::Separator();
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 start = ImGui::GetCursorScreenPos();
    start.x += 15; start.y += 15;

    ImU32 colMod = ImColor(50, 100, 200); ImU32 colCar = ImColor(200, 100, 50);
    ImU32 colLine = ImColor(200, 200, 200, 200);

    auto drawOp = [&](ImVec2 p, const char* l, bool c) {
        drawList->AddRectFilled(p, {p.x+nodeSize, p.y+nodeSize}, c ? colCar : colMod, 4.0f);
        drawList->AddRect(p, {p.x+nodeSize, p.y+nodeSize}, ImColor(255,255,255), 4.0f);
        ImVec2 ts = ImGui::CalcTextSize(l);
        drawList->AddText({p.x+(nodeSize-ts.x)*0.5f, p.y+(nodeSize-ts.y)*0.5f}, ImColor(255,255,255), l);
    };

    ImVec2 op1 = start;
    ImVec2 op2 = {start.x + nodeSize + spacing, start.y};

    drawList->AddBezierQuadratic(
        {op1.x + nodeSize*0.2f, op1.y},
        {op1.x + nodeSize*0.5f, op1.y - 15},
        {op1.x + nodeSize*0.8f, op1.y}, colLine, 2.0f);

    drawList->AddLine({op1.x+nodeSize, op1.y+nodeSize/2}, {op2.x, op2.y+nodeSize/2}, colLine, 2.0f);

    drawList->AddLine({op2.x+nodeSize, op2.y+nodeSize/2}, {op2.x+nodeSize+20, op2.y+nodeSize/2}, colLine, 2.0f);

    drawOp(op1, "1", false); drawOp(op2, "2", true);
    ImGui::Dummy(ImVec2(nodeSize * 2 + spacing + 40, nodeSize + 20));
}
//------------------------------------------------------------------------------
