#include "SceneHierarchyPanel.h"

#include <imgui/imgui.h>
#include <imgui/misc/cpp/imgui_stdlib.h>

#include "Core/StringID.h"
#include "Scene/Components.h"

namespace Wraith
{
    SceneHierarchyPanel::SceneHierarchyPanel(std::shared_ptr<Scene> context)
        : m_SelectionContext()
    {
        SetContext(context);
    }

    void SceneHierarchyPanel::SetContext(std::shared_ptr<Scene> context)
    {
        m_Context = context;
    }

    void SceneHierarchyPanel::OnUIRender()
    {
        ImGui::Begin("Scene Hierarchy");

        auto& registry = m_Context->m_Registry;
        registry.each([&](auto e) {
            Entity entity{ e, &registry };
            DrawEntityNode(entity);
        });

        if (ImGui::IsMouseClicked(0) && ImGui::IsWindowHovered())
            m_SelectionContext = {};

        ImGui::End();

        // Move this stuff to PropertiesPanel
        ImGui::Begin("Properties");
        if (m_SelectionContext)
        {
            DrawComponents(m_SelectionContext);
        }
        ImGui::End();
    }

    void SceneHierarchyPanel::DrawEntityNode(Entity entity)
    {
        auto& tag = entity.GetComponent<TagComponent>().tag;

        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanFullWidth;

        if (entity == m_SelectionContext)
            flags |= ImGuiTreeNodeFlags_Selected;

        bool opened = ImGui::TreeNodeEx((void*)(u64)(u32)entity, flags, tag.c_str());
        if (ImGui::IsItemClicked())
        {
            m_SelectionContext = entity;
        }

        if (opened)
        {
            ImGui::TreePop();
        }
    }

    void SceneHierarchyPanel::DrawComponents(Entity entity)
    {
        if (entity.HasComponent<TagComponent>())
        {
            auto& tag = entity.GetComponent<TagComponent>().tag;
            ImGui::InputText("Tag", &tag);
        }

        if (entity.HasComponent<TransformComponent>())
        {
            if (ImGui::CollapsingHeader("Transform Component"))
            {
                auto& transform = entity.GetComponent<TransformComponent>();

                ImGui::DragFloat3("Position", &transform.position.x, 0.01f);
                ImGui::DragFloat3("Scale", &transform.scale.x, 0.001f, 0.001f, 10.0f);
                Vec3f rotation_in_degrees = { Degrees(transform.rotation.x),
                                              Degrees(transform.rotation.y),
                                              Degrees(transform.rotation.z) };
                ImGui::DragFloat3("Rotation", &rotation_in_degrees.x, 0.1f);
                transform.rotation = { Radians(rotation_in_degrees.x),
                                       Radians(rotation_in_degrees.y),
                                       Radians(rotation_in_degrees.z) };
            }
        }

        if (entity.HasComponent<TextComponent>())
        {
            if (ImGui::CollapsingHeader("Text Component"))
            {
                auto& text = entity.GetComponent<TextComponent>();
                ImGui::InputTextMultiline("Text", &text.text);
                if (ImGui::InputText("Font", &text.font))
                {
                    text.font_id = StringID(text.font);
                }

                ImGui::DragInt("Font Size", &text.font_size, 1, 1, 256);

                ImGui::Separator();

                ImGui::RadioButton("Left", &text.justification, 0);
                ImGui::SameLine();
                ImGui::RadioButton("Center", &text.justification, 1);
                ImGui::SameLine();
                ImGui::RadioButton("Right", &text.justification, 2);

                ImGui::RadioButton("Top", &text.alignment, 0);
                ImGui::SameLine();
                ImGui::RadioButton("Middle", &text.alignment, 1);
                ImGui::SameLine();
                ImGui::RadioButton("Bottom", &text.alignment, 2);

                ImGui::RadioButton("LTR", &text.direction, 0);
                ImGui::SameLine();
                ImGui::RadioButton("RTL", &text.direction, 1);

                ImGui::Separator();

                ImGui::ColorEdit4("Color", &text.color.x);
                ImGui::SliderFloat("Blend Mode", &text.blend_mode, 0.0f, 1.0f, "Additive - %.2f - Alpha Blend");
            }
        }

        if (entity.HasComponent<CameraComponent>())
        {
            if (ImGui::CollapsingHeader("Camera Component"))
            {
                // auto& camera_component = entity.GetComponent<CameraComponent>();
            }
        }
    }
}  // namespace Wraith