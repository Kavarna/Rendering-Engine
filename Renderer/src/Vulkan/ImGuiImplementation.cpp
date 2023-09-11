#include "ImGuiImplementation.h"

#include "backends/imgui_impl_glfw.cpp"

#include "Buffer.h"
#include "CommandList.h"
#include "Image.h"
#include "Pipeline.h"
#include "Renderer.h"
#include "RootSignature.h"

/* TODO: Find a way to reduce this - Add a way to remove a image from the descriptor set */
static constexpr uint32_t MAXIMUM_NUMBER_OF_DESCRIPTOR_SET_INSTANCES = 1001;

struct PerFrameBuffers
{
    std::unique_ptr<Vulkan::Buffer> VertexBuffer;
    std::unique_ptr<Vulkan::Buffer> IndexBuffer;
};

// Each viewport will hold 1 PerFrameBuffers
struct ImGui_ImplVulkanH_WindowRenderBuffers
{
    uint32_t Index;
    uint32_t Count;
    std::vector<PerFrameBuffers> FrameRenderBuffers;
};

struct BackendData
{
    std::unique_ptr<Vulkan::DescriptorSet> DescriptorSet;
    std::unique_ptr<Vulkan::RootSignature> RootSignature;

    std::unique_ptr<Vulkan::Pipeline> Pipeline;

    std::atomic<uint32_t> CurrentDescriptorSet;

    ImGui_ImplVulkan_InitInfo VulkanInitInfo;
    VkDeviceSize BufferMemoryAlignment;
    VkShaderModule ShaderModuleVert;
    VkShaderModule ShaderModuleFrag;

    // Font data
    std::unique_ptr<Vulkan::Image> FontImage;
    std::unique_ptr<Vulkan::Buffer> UploadBuffer;

    // Render buffers for main window
    ImGui_ImplVulkanH_WindowRenderBuffers MainWindowRenderBuffers;

    BackendData()
    {
        memset((void *)this, 0, sizeof(*this));
        BufferMemoryAlignment = 256;
    }
};

// For multi-viewport support:
// Helper structure we store in the void* RenderUserData field of each ImGuiViewport to easily retrieve our backend
// data.
struct ImGui_ImplVulkan_ViewportData
{
    bool WindowOwned;
    ImGui_ImplVulkanH_WindowRenderBuffers RenderBuffers; // Used by all viewports

    ImGui_ImplVulkan_ViewportData()
    {
        WindowOwned = false;
        memset(&RenderBuffers, 0, sizeof(RenderBuffers));
    }
    ~ImGui_ImplVulkan_ViewportData()
    {
    }
};

static BackendData *GetBackendData()
{
    return (BackendData *)ImGui::GetIO().BackendRendererUserData;
}

static void CreatePipeline(VkFormat pipelineFormat, Vulkan::RootSignature *rootSignature,
                           std::unique_ptr<Vulkan::Pipeline> &pipeline)
{
    pipeline = std::make_unique<Vulkan::Pipeline>("ImGui_Pipeline");
    {
        pipeline->AddShader("Shaders/imgui.vert.spv");
        pipeline->AddShader("Shaders/imgui.frag.spv");
        pipeline->SetRootSignature(rootSignature);
    }

    VkVertexInputBindingDescription bindingDesc[1] = {};
    bindingDesc[0].stride = sizeof(ImDrawVert);
    bindingDesc[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputAttributeDescription attributeDesc[3] = {};
    attributeDesc[0].location = 0;
    attributeDesc[0].binding = attributeDesc[0].binding;
    attributeDesc[0].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDesc[0].offset = IM_OFFSETOF(ImDrawVert, pos);
    attributeDesc[1].location = 1;
    attributeDesc[1].binding = attributeDesc[0].binding;
    attributeDesc[1].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDesc[1].offset = IM_OFFSETOF(ImDrawVert, uv);
    attributeDesc[2].location = 2;
    attributeDesc[2].binding = attributeDesc[0].binding;
    attributeDesc[2].format = VK_FORMAT_R8G8B8A8_UNORM;
    attributeDesc[2].offset = IM_OFFSETOF(ImDrawVert, col);

    VkPipelineMultisampleStateCreateInfo &ms_info = pipeline->GetMultisampleStateCreateInfo();

    VkPipelineVertexInputStateCreateInfo &vertexInfo = pipeline->GetVertexInputStateCreateInfo();
    {
        vertexInfo.vertexBindingDescriptionCount = 1;
        vertexInfo.pVertexBindingDescriptions = bindingDesc;
        vertexInfo.vertexAttributeDescriptionCount = 3;
        vertexInfo.pVertexAttributeDescriptions = attributeDesc;
    }

    VkPipelineViewportStateCreateInfo &viewportInfo = pipeline->GetViewportStateCreateInfo();
    {
        viewportInfo.viewportCount = 1;
        viewportInfo.scissorCount = 1;
    }

    VkPipelineRasterizationStateCreateInfo &rasterInfo = pipeline->GetRasterizationStateCreateInfo();
    {
        rasterInfo.cullMode = VK_CULL_MODE_NONE;
    }

    VkPipelineColorBlendAttachmentState colorAttachment[1];
    {
        colorAttachment[0].blendEnable = VK_TRUE;
        colorAttachment[0].srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        colorAttachment[0].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        colorAttachment[0].colorBlendOp = VK_BLEND_OP_ADD;
        colorAttachment[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        colorAttachment[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        colorAttachment[0].alphaBlendOp = VK_BLEND_OP_ADD;
        colorAttachment[0].colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    }

    VkPipelineColorBlendStateCreateInfo &blend_info = pipeline->GetColorBlendStateCreateInfo();
    {
        blend_info.attachmentCount = 1;
        blend_info.pAttachments = colorAttachment;
    }

    VkDynamicState dynamic_states[2] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo &dynamic_state = pipeline->GetDynamicStateCreateInfo();
    {
        dynamic_state.dynamicStateCount = (uint32_t)IM_ARRAYSIZE(dynamic_states);
        dynamic_state.pDynamicStates = dynamic_states;
    }

    pipeline->AddFormatColorOutput(pipelineFormat);

    pipeline->Bake();
}

static void CreteaDeviceObjects()
{
    auto *bd = GetBackendData();
    auto &initInfo = bd->VulkanInitInfo;
    auto renderer = Vulkan::Renderer::Get();
    VkSampler sampler = renderer->GetFontSampler();

    if (bd->DescriptorSet == nullptr && bd->RootSignature == nullptr)
    {
        bd->DescriptorSet = std::make_unique<Vulkan::DescriptorSet>();
        {
            bd->DescriptorSet->AddCombinedImageSampler(0, &sampler, VK_SHADER_STAGE_FRAGMENT_BIT);
        }
        bd->DescriptorSet->Bake(MAXIMUM_NUMBER_OF_DESCRIPTOR_SET_INSTANCES);

        bd->RootSignature = std::make_unique<Vulkan::RootSignature>();
        {
            bd->RootSignature->AddPushRange<float>(0, 4, VK_SHADER_STAGE_VERTEX_BIT);
            bd->RootSignature->AddDescriptorSet(bd->DescriptorSet.get());
        }
        bd->RootSignature->Bake();
    }

    CreatePipeline(renderer->GetBackbufferFormat(), bd->RootSignature.get(), bd->Pipeline);
}

static void SetupRenderState(ImDrawData *drawData, Vulkan::CommandList *cmdList, PerFrameBuffers *rb, int fbWidth,
                             int fbHeight)
{
    auto *bd = GetBackendData();
    cmdList->BindPipeline(bd->Pipeline.get());

    /* Bind vertex and index buffers */
    if (drawData->TotalVtxCount > 0)
    {
        cmdList->BindVertexBuffer(rb->VertexBuffer.get(), 0);
        cmdList->BindIndexBuffer(rb->IndexBuffer.get());
    }

    /* Setup viewport */
    {
        VkViewport viewport;
        viewport.x = 0;
        viewport.y = 0;
        viewport.width = (float)fbWidth;
        viewport.height = (float)fbHeight;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        cmdList->SetViewports({viewport});
    }

    // Setup scale and translation:
    // Our visible imgui space lies from draw_data->DisplayPps (top left) to
    // draw_data->DisplayPos+data_data->DisplaySize (bottom right). DisplayPos is (0,0) for single viewport apps.
    {
        float constants[4];
        float *scale = constants;
        scale[0] = 2.0f / drawData->DisplaySize.x;
        scale[1] = 2.0f / drawData->DisplaySize.y;
        float *translate = &constants[2];
        translate[0] = -1.0f - drawData->DisplayPos.x * scale[0];
        translate[1] = -1.0f - drawData->DisplayPos.y * scale[1];
        cmdList->BindPushRange<float>(bd->RootSignature.get(), 0, 4, scale, VK_SHADER_STAGE_VERTEX_BIT);
    }
}

/* User code functions */
bool ImGui_ImplVulkan_Init(ImGui_ImplVulkan_InitInfo *info)
{
    ImGuiIO &io = ImGui::GetIO();
    CHECK(io.BackendRendererUserData == nullptr) << "Already initialized a renderer backend!";

    // Setup backend capabilities flags
    BackendData *bd = new BackendData();
    io.BackendRendererUserData = (void *)bd;
    io.BackendRendererName = "jnr_imgui_backend";
    io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset; // We can honor the ImDrawCmd::VtxOffset field, allowing
                                                               // for large meshes.
    io.BackendFlags |=
        ImGuiBackendFlags_RendererHasViewports; // We can create multi-viewports on the Renderer side (optional)

    bd->VulkanInitInfo = *info;

    CreteaDeviceObjects();

    // Our render function expect RendererUserData to be storing the window render buffer we need (for the main viewport
    // we won't use ->Window)
    ImGuiViewport *main_viewport = ImGui::GetMainViewport();
    main_viewport->RendererUserData = new ImGui_ImplVulkan_ViewportData();

    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        /* TODO: In case multiple viewports are enabled, handle this */
    }

    return true;
}

void ImGui_ImplVulkan_Shutdown()
{
    ImGuiIO &io = ImGui::GetIO();
    BackendData *bd = GetBackendData();

    ImGuiViewport *main_viewport = ImGui::GetMainViewport();
    ;
    if (main_viewport->RendererUserData)
    {
        delete (ImGui_ImplVulkan_ViewportData *)main_viewport->RendererUserData;
    }
    main_viewport->RendererUserData = nullptr;

    io.BackendRendererName = nullptr;
    io.BackendRendererUserData = nullptr;
    delete bd;
}

void ImGui_ImplVulkan_NewFrame(Vulkan::CommandList *cmdList)
{
    BackendData *bd = GetBackendData();
    IM_ASSERT(bd != nullptr && "Did you call ImGui_ImplVulkan_Init()?");
    IM_UNUSED(bd);
}

void ImGui_ImplVulkan_RenderDrawData(ImDrawData *drawData, Vulkan::CommandList *cmdList)
{
    BackendData *bd = GetBackendData();
    ImGui_ImplVulkan_InitInfo *v = &bd->VulkanInitInfo;

    int fbWidth = (int)(drawData->DisplaySize.x * drawData->FramebufferScale.x);
    int fbHeight = (int)(drawData->DisplaySize.y * drawData->FramebufferScale.y);

    ImGui_ImplVulkan_ViewportData *viewport_renderer_data =
        (ImGui_ImplVulkan_ViewportData *)drawData->OwnerViewport->RendererUserData;
    IM_ASSERT(viewport_renderer_data != nullptr);
    ImGui_ImplVulkanH_WindowRenderBuffers *wrb = &viewport_renderer_data->RenderBuffers;
    if (wrb->FrameRenderBuffers.size() == 0)
    {
        wrb->Index = 0;
        wrb->Count = v->ImageCount;
        // wrb->FrameRenderBuffers = (PerFrameBuffers*)IM_ALLOC(sizeof(PerFrameBuffers) * wrb->Count);
        wrb->FrameRenderBuffers.resize(wrb->Count);
    }
    IM_ASSERT(wrb->Count == v->ImageCount);

    wrb->Index = (wrb->Index + 1) % wrb->Count;

    PerFrameBuffers *rb = &wrb->FrameRenderBuffers[wrb->Index];

    if (drawData->TotalVtxCount > 0)
    {
        size_t vertexSize = drawData->TotalVtxCount * sizeof(ImDrawVert);
        size_t indexSize = drawData->TotalIdxCount * sizeof(ImDrawIdx);
        if (rb->VertexBuffer == nullptr || rb->VertexBuffer->GetSize() < vertexSize)
        {
            rb->VertexBuffer.reset();
            rb->VertexBuffer = std::make_unique<Vulkan::Buffer>(sizeof(ImDrawVert), drawData->TotalVtxCount,
                                                                VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                                                VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
        }
        if (rb->IndexBuffer == nullptr || rb->IndexBuffer->GetSize() < indexSize)
        {
            rb->IndexBuffer.reset();
            rb->IndexBuffer = std::make_unique<Vulkan::Buffer>(sizeof(ImDrawIdx), drawData->TotalIdxCount,
                                                               VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                                                               VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
        }

        ImDrawVert *vtxDst = (ImDrawVert *)rb->VertexBuffer->GetData();
        ImDrawIdx *idxDst = (ImDrawIdx *)rb->IndexBuffer->GetData();

        for (int n = 0; n < drawData->CmdListsCount; n++)
        {
            const ImDrawList *cmd_list = drawData->CmdLists[n];
            memcpy(vtxDst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
            memcpy(idxDst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
            vtxDst += cmd_list->VtxBuffer.Size;
            idxDst += cmd_list->IdxBuffer.Size;
        }
    }

    SetupRenderState(drawData, cmdList, rb, fbWidth, fbHeight);

    ImVec2 clip_off = drawData->DisplayPos;         // (0,0) unless using multi-viewports
    ImVec2 clip_scale = drawData->FramebufferScale; // (1,1) unless using retina display which are often (2,2)

    int global_vtx_offset = 0;
    int global_idx_offset = 0;
    for (int n = 0; n < drawData->CmdListsCount; n++)
    {
        const ImDrawList *cmd_list = drawData->CmdLists[n];
        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
        {
            const ImDrawCmd *pcmd = &cmd_list->CmdBuffer[cmd_i];
            if (pcmd->UserCallback != nullptr)
            {
                // User callback, registered via ImDrawList::AddCallback()
                // (ImDrawCallback_ResetRenderState is a special callback value used by the user to request the renderer
                // to reset render state.)
                if (pcmd->UserCallback == ImDrawCallback_ResetRenderState)
                    SetupRenderState(drawData, cmdList, rb, fbWidth, fbHeight);
                else
                    pcmd->UserCallback(cmd_list, pcmd);
            }
            else
            {
                // Project scissor/clipping rectangles into framebuffer space
                ImVec2 clip_min((pcmd->ClipRect.x - clip_off.x) * clip_scale.x,
                                (pcmd->ClipRect.y - clip_off.y) * clip_scale.y);
                ImVec2 clip_max((pcmd->ClipRect.z - clip_off.x) * clip_scale.x,
                                (pcmd->ClipRect.w - clip_off.y) * clip_scale.y);

                // Clamp to viewport as vkCmdSetScissor() won't accept values that are off bounds
                if (clip_min.x < 0.0f)
                {
                    clip_min.x = 0.0f;
                }
                if (clip_min.y < 0.0f)
                {
                    clip_min.y = 0.0f;
                }
                if (clip_max.x > fbWidth)
                {
                    clip_max.x = (float)fbWidth;
                }
                if (clip_max.y > fbHeight)
                {
                    clip_max.y = (float)fbHeight;
                }
                if (clip_max.x <= clip_min.x || clip_max.y <= clip_min.y)
                    continue;

                // Apply scissor/clipping rectangle
                VkRect2D scissor;
                scissor.offset.x = (int32_t)(clip_min.x);
                scissor.offset.y = (int32_t)(clip_min.y);
                scissor.extent.width = (uint32_t)(clip_max.x - clip_min.x);
                scissor.extent.height = (uint32_t)(clip_max.y - clip_min.y);
                cmdList->SetScissor({scissor});

                // Bind DescriptorSet with font or user texture
                // VkDescriptorSet desc_set[1] = {(VkDescriptorSet)pcmd->TextureId};
                // if (sizeof(ImTextureID) < sizeof(ImU64))
                //{
                //    // We don't support texture switches if ImTextureID hasn't been redefined to be 64-bit. Do a flaky
                //    check that other textures haven't been used. IM_ASSERT(pcmd->TextureId ==
                //    (ImTextureID)bd->FontDescriptorSet); desc_set[0] = bd->FontDescriptorSet;
                //}

                // Draw
                cmdList->BindDescriptorSet(bd->DescriptorSet.get(), (uint32_t)pcmd->TextureId, bd->RootSignature.get());
                cmdList->DrawIndexedInstanced(pcmd->ElemCount, pcmd->IdxOffset + global_idx_offset,
                                              pcmd->VtxOffset + global_vtx_offset);
            }
        }
        global_idx_offset += cmd_list->IdxBuffer.Size;
        global_vtx_offset += cmd_list->VtxBuffer.Size;
    }

    VkRect2D scissor = {{0, 0}, {(uint32_t)fbWidth, (uint32_t)fbHeight}};
    cmdList->SetScissor({scissor});
}

bool ImGui_ImplVulkan_CreateFontsTexture(Vulkan::CommandList *cmdList)
{
    ImGuiIO &io = ImGui::GetIO();
    BackendData *bd = GetBackendData();
    auto renderer = Vulkan::Renderer::Get();
    auto &initInfo = bd->VulkanInitInfo;

    unsigned char *pixels;
    int width, height;
    io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
    size_t upload_size = width * height * 4 * sizeof(char);

    /* Create resources */
    Vulkan::Image::Info2D info2D{};
    {
        info2D.width = width;
        info2D.height = height;
        info2D.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        info2D.format = VK_FORMAT_R8G8B8A8_UNORM;
        info2D.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    }
    bd->FontImage = std::make_unique<Vulkan::Image>(info2D);
    bd->UploadBuffer = std::make_unique<Vulkan::Buffer>(4, width * height, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                                        VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
    /* Copy the font to the upload buffer */
    {
        void *dst = bd->UploadBuffer->GetData();
        memcpy(dst, pixels, upload_size);
    }

    /* Copy buffer to image */
    {
        Vulkan::CommandList::TransitionInfo ti{};
        {
            ti.dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            ti.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            ti.srcStage = VK_PIPELINE_STAGE_HOST_BIT;
            ti.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        }
        bd->FontImage->EnsureAspect(VK_IMAGE_ASPECT_COLOR_BIT);
        cmdList->TransitionImageTo(bd->FontImage.get(), ti);
        cmdList->CopyWholeBufferToImage(bd->FontImage.get(), bd->UploadBuffer.get());
    }

    /* Transition image to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL */
    {
        Vulkan::CommandList::TransitionInfo ti{};
        {
            ti.dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            ti.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            ti.srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            ti.newLayout = Vulkan::Image::IMGUI_IMAGE_LAYOUT;
        }
        cmdList->TransitionImageTo(bd->FontImage.get(), ti);
    }

    ImTextureID fontID =
        ImGui_ImplVulkan_AddTexture(renderer->GetFontSampler(), bd->FontImage->GetImageView(VK_IMAGE_ASPECT_COLOR_BIT));

    io.Fonts->SetTexID(fontID);

    return true;
}

ImTextureID ImGui_ImplVulkan_AddTexture(VkSampler sampler, Vulkan::ImageView imageView)
{
    ImGuiIO &io = ImGui::GetIO();
    BackendData *bd = GetBackendData();
    uint32_t currentInstance = bd->CurrentDescriptorSet++;

    imageView.SetLayout(Vulkan::Image::IMGUI_IMAGE_LAYOUT);
    bd->DescriptorSet->BindCombinedImageSampler(0, imageView, VK_IMAGE_ASPECT_COLOR_BIT, sampler, currentInstance);

    return currentInstance;
}
