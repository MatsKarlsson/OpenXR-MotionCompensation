// MIT License
//
// Copyright(c) 2022 Matthieu Bucchianeri, Sebastian Veith
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this softwareand associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright noticeand this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once
#include "graphics.h"

namespace openxr_api_layer::graphics
{
    // Colors
    constexpr XrVector3f Red{1.f, 0.f, 0.f};
    constexpr XrVector3f DarkRed{0.25f, 0.f, 0.f};
    constexpr XrVector3f LightRed{1.f, 0.15f, 0.15f};
    constexpr XrVector3f Green{0.f, 1.f, 0.f};
    constexpr XrVector3f DarkGreen{0.f, 0.25f, 0.f};
    constexpr XrVector3f LightGreen{0.15f, 1.f, 0.15f};
    constexpr XrVector3f Blue{0.f, 0.f, 1.f};
    constexpr XrVector3f DarkBlue{0.f, 0.f, 0.25f};
    constexpr XrVector3f LightBlue{0.15f, 0.15f, 1.f};
    constexpr XrVector3f Yellow{1.f, 1.f, 0.f};
    constexpr XrVector3f DarkYellow{0.25f, 0.25f, 0.f};
    constexpr XrVector3f LightYellow{1.f, 1.f, 0.15f};
    constexpr XrVector3f Cyan{0.f, 1.f, 1.f};
    constexpr XrVector3f DarkCyan{0.f, 0.25f, 0.25f};
    constexpr XrVector3f LightCyan{0.15f, 1.f, 1.f};
    constexpr XrVector3f Magenta{1.f, 0.f, 1.f};
    constexpr XrVector3f DarkMagenta{0.25f, 0.f, 0.25f};
    constexpr XrVector3f LightMagenta{1.f, 0.15f, 1.f};

    struct SwapchainImages
    {
        std::vector<std::shared_ptr<IGraphicsTexture>> chain{};
    };

    struct SwapchainState
    {
        std::vector<SwapchainImages> images{};
        uint32_t acquiredImageIndex{0};
        bool delayedRelease{false};
    };

    class Overlay
    {
      public:
        void DestroySession(XrSession session);
        void CreateSwapchain(XrSwapchain swapchain, const XrSwapchainCreateInfo* createInfo);
        void DestroySwapchain(XrSwapchain swapchain);
        XrResult AcquireSwapchainImage(XrSwapchain swapchain,
                                       const XrSwapchainImageAcquireInfo* acquireInfo,
                                       uint32_t* index);
        XrResult ReleaseSwapchainImage(XrSwapchain swapchain, const XrSwapchainImageReleaseInfo* releaseInfo);
        void BeginFrame();
        void SetMarkerSize();
        bool ToggleOverlay();        
        void DrawOverlay(const XrPosef& referencePose,
                         const XrPosef& delta,
                         bool mcActivated,
                         XrSession session,
                         XrFrameEndInfo* chainFrameEndInfo,
                         OpenXrLayer* openXrLayer);
        void DeleteResources();

        bool m_D3D12inUse{false};
        bool m_Initialized{false};

      private:
        // I'd rather have that swapchain tracking stuff in composition/device space. But to not interfere when overlay
        // isn't even used, decided to go for lazy init of composition framework, which unfortunately is too late to
        // capture swapchain creation
        struct SwapchainState
        {
            std::vector<ID3D11Texture2D*> texturesD3D11;
            std::vector<ID3D12Resource*> texturesD3D12;
            DXGI_FORMAT format;
            uint32_t index;
            bool doRelease;
        };

        static std::vector<SimpleMeshVertex> CreateMarker(bool reference);
        static std::vector<SimpleMeshVertex> CreateMarkerMesh(const XrVector3f& top,
                                                              const XrVector3f& innerMiddle,
                                                              const XrVector3f& outerMiddle,
                                                              const XrVector3f& bottom,
                                                              const XrVector3f& darkColor,
                                                              const XrVector3f& pureColor,
                                                              const XrVector3f& lightColor);
        void CopyAppTexture(IGraphicsDevice* appDevice,
                            XrSwapchain swapchain,
                            const SwapchainState& swapchainState,
                            ID3D11Device* device,
                            ID3D11DeviceContext* context,
                            ID3D11Texture2D* rgbTexture);

        bool m_OverlayActive{false};
        XrVector3f m_MarkerSize{0.1f, 0.1f, 0.1f};
        std::shared_ptr<ISimpleMesh> m_MeshRGB{}, m_MeshCMY{};
        std::map<XrSwapchain, SwapchainState> m_Swapchains{};
        std::vector<std::shared_ptr<ISwapchain>> m_MarkerSwapchains{};
        std::vector<std::shared_ptr<IGraphicsTexture>> m_MarkerDepthTextures{};
        std::vector<XrCompositionLayerProjectionView>* m_CreatedViews{nullptr};
        XrCompositionLayerProjection* m_CreatedProjectionLayer{};
        std::vector<const XrCompositionLayerBaseHeader*> m_BaseLayerVector{};
        std::mutex m_DrawMutex;
        std::set<XrSession> m_InitializedSessions{};
    };
} // namespace openxr_api_layer::graphics