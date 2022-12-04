#include <internal/example_property_im_draw.h>
void im_draw_basic_widgets (attr_table& p) {

    // Basic Widgets Demo  ==============================================================================================
    ImGui::Text("Basic Widget Demo");

    // Widget Demo from imgui_demo.cpp...
    // Normal Button
    if (ImGui::Button("Button")) {
        p.pint["buttonData"]++;
    }
    ImGui::SameLine();
    ImGui::Text("Times Clicked: %u", p.pint["buttonData"]);

    // Checkbox - needs bool type
    // ImGui::Checkbox("checkbox", p.pint["checkData"]);
    
    // Radio buttons
    
    ImGui::RadioButton("radio a", &p.pint["radioData"], 0); ImGui::SameLine();
    ImGui::RadioButton("radio b", &p.pint["radioData"], 1); ImGui::SameLine();
    ImGui::RadioButton("radio c", &p.pint["radioData"], 2);
    

    // Color buttons, demonstrate using PushID() to add unique identifier in the ID stack, and changing style.
    for (int i = 0; i < 7; i++)
    {
        if (i > 0)
        ImGui::SameLine();
        ImGui::PushID(i);
        ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(i / 7.0f, 0.6f, 0.6f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(i / 7.0f, 0.7f, 0.7f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(i / 7.0f, 0.8f, 0.8f));
        ImGui::Button("Click");
        ImGui::PopStyleColor(3);
        ImGui::PopID();
    }

    // Use AlignTextToFramePadding() to align text baseline to the baseline of framed elements (otherwise a Text+SameLine+Button sequence will have the text a little too high by default)
    ImGui::AlignTextToFramePadding();
    ImGui::Text("Hold to repeat:");
    ImGui::SameLine();

    // Arrow buttons with Repeater
    float spacing = ImGui::GetStyle().ItemInnerSpacing.x;
    ImGui::PushButtonRepeat(true);
    if (ImGui::ArrowButton("##left", ImGuiDir_Left)) { p.pint["repeaterData"]--; }
    ImGui::SameLine(0.0f, spacing);
    if (ImGui::ArrowButton("##right", ImGuiDir_Right)) { p.pint["repeaterData"]++; }
    ImGui::PopButtonRepeat();
    ImGui::SameLine();
    ImGui::Text("%d", p.pint["repeaterData"]);

    // The input widgets also require you to manually disable the editor shortcuts so the view doesn't fly around.
    // (note that this is a per-frame setting, so it disables it for all text boxes.  I left it here so you could find it!)
    ax::NodeEditor::EnableShortcuts(ImGui::GetIO().WantTextInput);
    // The input widgets require some guidance on their widths, or else they're very large. (note matching pop at the end).
    ImGui::PushItemWidth(200);
    ImGui::InputTextWithHint("input text (w/ hint)", "enter text here", &p.pstring["inputTextData"]);
    
    ImGui::InputFloat("input float", &p.pfloat["floatData"], 0.01f, 1.0f, "%.3f");


    ImGui::DragFloat("drag float", &p.pfloat["dragData"], 0.005f);

    ImGui::DragFloat("drag small float", &p.pfloat["dragData2"], 0.0001f, 0.0f, 0.0f, "%.06f ns");
    

    ImGui::PopItemWidth();
}
