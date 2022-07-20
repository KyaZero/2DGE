#pragma once
#include "RenderSystem.h"
#include "VkShader.h"

namespace Wraith
{
    class TestRenderSystem : public RenderSystem<ModelCommand>
    {
    public:
        TestRenderSystem(Device& device);

        bool Init(vk::RenderPass render_pass /*, vk::DescriptorSetLayout global_set_layout*/) override;
        void Render(FrameInfo& frame_info) override;

    private:
        virtual void CreatePipelineLayout(/* [[maybe_unused]] vk::DescriptorSetLayout global_set_layout*/);
        virtual void CreatePipeline([[maybe_unused]] vk::RenderPass render_pass);

        Model m_Model;
    };
}  // namespace Wraith