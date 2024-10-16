/*
 // Copyright (c) 2021-2022 Timothy Schoen and Alex Mitchell
 // For information on usage and redistribution, and for a DISCLAIMER OF ALL
 // WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 */

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_opengl/juce_opengl.h>
using namespace juce::gl;

#include <nanovg.h>
#ifdef NANOVG_GL_IMPLEMENTATION
#    include <nanovg_gl.h>
#    include <nanovg_gl_utils.h>
#endif

#include "NVGSurface.h"

#include "PluginEditor.h"
#include "PluginProcessor.h"
#include "Components/WelcomePanel.h"

#define ENABLE_FPS_COUNT 0

class FrameTimer {
public:
    FrameTimer()
    {
        startTime = getNow();
        prevTime = startTime;
    }

    void render(NVGcontext* nvg)
    {
        nvgFillColor(nvg, nvgRGBA(40, 40, 40, 255));
        nvgFillRect(nvg, 0, 0, 40, 22);

        nvgFontSize(nvg, 20.0f);
        nvgTextAlign(nvg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
        nvgFillColor(nvg, nvgRGBA(240, 240, 240, 255));
        char fpsBuf[16];
        snprintf(fpsBuf, 16, "%d", static_cast<int>(round(1.0f / getAverageFrameTime())));
        nvgText(nvg, 7, 2, fpsBuf, nullptr);
    }
    void addFrameTime()
    {
        auto timeSeconds = getTime();
        auto dt = timeSeconds - prevTime;
        perf_head = (perf_head + 1) % 32;
        frame_times[perf_head] = dt;
        prevTime = timeSeconds;
    }

    double getTime() { return getNow() - startTime; }

private:
    double getNow()
    {
        auto ticks = Time::getHighResolutionTicks();
        return Time::highResolutionTicksToSeconds(ticks);
    }

    float getAverageFrameTime()
    {
        float avg = 0;
        for (int i = 0; i < 32; i++) {
            avg += frame_times[i];
        }
        return avg / (float)32;
    }

    float frame_times[32] = {};
    int perf_head = 0;
    double startTime = 0, prevTime = 0;
};

NVGSurface::NVGSurface(PluginEditor* e)
    : editor(e)
{
#ifdef NANOVG_GL_IMPLEMENTATION
    glContext = std::make_unique<OpenGLContext>();
    glContext->setOpenGLVersionRequired(OpenGLContext::OpenGLVersion::openGL3_2);
    glContext->setMultisamplingEnabled(false);
    glContext->setSwapInterval(0);
#endif

#if ENABLE_FPS_COUNT
    frameTimer = std::make_unique<FrameTimer>();
#endif

    setInterceptsMouseClicks(false, false);
    setWantsKeyboardFocus(false);

    setSize(1, 1);

    // Start rendering asynchronously, so we are sure the window has been added to the desktop
    // kind of a hack, but works well enough
    MessageManager::callAsync([_this = SafePointer(this)]() {
        if (_this) {
            // Render on vblank
            _this->vBlankAttachment = std::make_unique<VBlankAttachment>(_this.getComponent(), [_this]() {
                if (_this) {
                    _this->editor->pd->setThis();
                    _this->editor->pd->messageDispatcher->dequeueMessages();
                    _this->render();
                }
            });
        }
    });
}

NVGSurface::~NVGSurface()
{
    detachContext();
}

void NVGSurface::initialise()
{
#ifdef NANOVG_METAL_IMPLEMENTATION
    auto* peer = getPeer()->getNativeHandle();
    auto* view = OSUtils::MTLCreateView(peer, 0, 0, getWidth(), getHeight());
    setView(view);
    setVisible(true);

    auto renderScale = getRenderScale();
    
    lastRenderScale = renderScale;
    nvg = nvgCreateContext(view, NVG_ANTIALIAS | NVG_TRIPLE_BUFFER, getWidth() * renderScale, getHeight() * renderScale);
    resized();
#else
    setVisible(true);
    glContext->attachTo(*this);
    glContext->initialiseOnThread();
    glContext->makeActive();
    nvg = nvgCreateContext(NVG_ANTIALIAS);
#endif

    surfaces[nvg] = this;

    invalidateAll();

    if (!nvg)
        std::cerr << "could not initialise nvg" << std::endl;
    nvgCreateFontMem(nvg, "Inter", (unsigned char*)BinaryData::InterRegular_ttf, BinaryData::InterRegular_ttfSize, 0);
    nvgCreateFontMem(nvg, "Inter-Regular", (unsigned char*)BinaryData::InterRegular_ttf, BinaryData::InterRegular_ttfSize, 0);
    nvgCreateFontMem(nvg, "Inter-Bold", (unsigned char*)BinaryData::InterBold_ttf, BinaryData::InterBold_ttfSize, 0);
    nvgCreateFontMem(nvg, "Inter-SemiBold", (unsigned char*)BinaryData::InterSemiBold_ttf, BinaryData::InterSemiBold_ttfSize, 0);
    nvgCreateFontMem(nvg, "Inter-Tabular", (unsigned char*)BinaryData::InterTabular_ttf, BinaryData::InterTabular_ttfSize, 0);
    nvgCreateFontMem(nvg, "icon_font-Regular", (unsigned char*)BinaryData::IconFont_ttf, BinaryData::IconFont_ttfSize, 0);
}

void NVGSurface::detachContext()
{
    if (makeContextActive()) {
        NVGFramebuffer::clearAll(nvg);
        NVGImage::clearAll(nvg);

        if (invalidFBO) {
            nvgDeleteFramebuffer(invalidFBO);
            invalidFBO = nullptr;
        }
        if (mainFBO) {
            nvgDeleteFramebuffer(mainFBO);
            mainFBO = nullptr;
        }
        if (nvg) {
            nvgDeleteContext(nvg);
            nvg = nullptr;
            surfaces.erase(nvg);
        }

#ifdef NANOVG_METAL_IMPLEMENTATION
        if (auto* view = getView()) {
            OSUtils::MTLDeleteView(view);
            setView(nullptr);
        }
#else
        glContext->detach();
#endif
    }
}

void NVGSurface::updateBufferSize()
{
    float pixelScale = getRenderScale();
    int scaledWidth = getWidth() * pixelScale;
    int scaledHeight = getHeight() * pixelScale;

    if (fbWidth != scaledWidth || fbHeight != scaledHeight || !mainFBO) {
        if (invalidFBO)
            nvgDeleteFramebuffer(invalidFBO);
        if (mainFBO)
            nvgDeleteFramebuffer(mainFBO);
        mainFBO = nvgCreateFramebuffer(nvg, scaledWidth, scaledHeight, NVG_IMAGE_PREMULTIPLIED);
        invalidFBO = nvgCreateFramebuffer(nvg, scaledWidth, scaledHeight, NVG_IMAGE_PREMULTIPLIED);
        fbWidth = scaledWidth;
        fbHeight = scaledHeight;
        invalidArea = getLocalBounds();
    }
}

#ifdef NANOVG_GL_IMPLEMENTATION
void NVGSurface::timerCallback()
{
    updateBounds(newBounds);
    if (getBounds() == newBounds)
        stopTimer();
}
#endif

void NVGSurface::lookAndFeelChanged()
{
    if (makeContextActive()) {
        NVGFramebuffer::clearAll(nvg);
        NVGImage::clearAll(nvg);
        invalidateAll();
    }
}

void NVGSurface::triggerRepaint()
{
    needsBufferSwap = true;
}

bool NVGSurface::makeContextActive()
{
#ifdef NANOVG_METAL_IMPLEMENTATION
    // No need to make context active with Metal, so just check if we have initialised and return that
    return getView() != nullptr && nvg != nullptr;
#else
    if (glContext)
        return glContext->makeActive();
    return false;
#endif
}

float NVGSurface::getRenderScale() const
{
#ifdef NANOVG_METAL_IMPLEMENTATION
    auto desktopScale = Desktop::getInstance().getGlobalScaleFactor();
    return OSUtils::MTLGetPixelScale(getView()) * desktopScale;
#else
    return glContext->getRenderingScale();
#endif
}

void NVGSurface::updateBounds(Rectangle<int> bounds)
{
#ifdef NANOVG_GL_IMPLEMENTATION
    newBounds = bounds;
    if (hresize)
        setBounds(bounds.withHeight(getHeight()));
    else
        setBounds(bounds.withWidth(getWidth()));

    resizing = true;
#else
    setBounds(bounds);
#endif
}

void NVGSurface::resized()
{
#ifdef NANOVG_METAL_IMPLEMENTATION
    if (auto* view = getView()) {
        auto renderScale = getRenderScale();
        auto* topLevel = getTopLevelComponent();
        auto bounds = topLevel->getLocalArea(this, getLocalBounds()).toFloat() * renderScale;
        mnvgSetViewBounds(view, bounds.getWidth(), bounds.getHeight());
    }
#endif
}

void NVGSurface::invalidateAll()
{
    invalidArea = getLocalBounds();
}

void NVGSurface::invalidateArea(Rectangle<int> area)
{
    invalidArea = invalidArea.getUnion(area);
}

void NVGSurface::render()
{
#if ENABLE_FPS_COUNT
    frameTimer->addFrameTime();
#endif

    auto startTime = Time::getMillisecondCounter();
    
    if(!getPeer()) {
        return;
    }
    
    if (!nvg) {
        initialise();
    }

    if (!makeContextActive())
        return;

    auto pixelScale = getRenderScale();
#if NANOVG_METAL_IMPLEMENTATION
    if(lastRenderScale != pixelScale)
    {
        detachContext();
        return; // Render on next frame
    }
#endif
    
    updateBufferSize();

    if (!invalidArea.isEmpty()) {
        auto invalidated = invalidArea.expanded(1);

        // First, draw only the invalidated region to a separate framebuffer
        // I've found that nvgScissor doesn't always clip everything, meaning that there will be graphical glitches if we don't do this
        nvgBindFramebuffer(invalidFBO);
        nvgViewport(0, 0, getWidth() * pixelScale, getHeight() * pixelScale);
        nvgClear(nvg);

        nvgBeginFrame(nvg, getWidth(), getHeight(), pixelScale);
        nvgScissor(nvg, invalidated.getX(), invalidated.getY(), invalidated.getWidth(), invalidated.getHeight());

        editor->renderArea(nvg, invalidated);
        nvgEndFrame(nvg);

        nvgBindFramebuffer(mainFBO);
        nvgViewport(0, 0, getWidth() * pixelScale, getHeight() * pixelScale);
        nvgBeginFrame(nvg, getWidth(), getHeight(), pixelScale);
        nvgBeginPath(nvg);
        nvgScissor(nvg, invalidated.getX(), invalidated.getY(), invalidated.getWidth(), invalidated.getHeight());

        nvgFillPaint(nvg, nvgImagePattern(nvg, 0, 0, getWidth(), getHeight(), 0, invalidFBO->image, 1));
        nvgFillRect(nvg, invalidated.getX(), invalidated.getY(), invalidated.getWidth(), invalidated.getHeight());

#if ENABLE_FB_DEBUGGING
        static Random rng;
        nvgFillColor(nvg, nvgRGBA(rng.nextInt(255), rng.nextInt(255), rng.nextInt(255), 0x50));
        nvgFillRect(nvg, 0, 0, getWidth(), getHeight());
#endif

        nvgEndFrame(nvg);

        nvgBindFramebuffer(nullptr);
        needsBufferSwap = true;
        invalidArea = Rectangle<int>(0, 0, 0, 0);
    }

    if (needsBufferSwap) {
        float pixelScale = getRenderScale();
        nvgViewport(0, 0, getWidth() * pixelScale, getHeight() * pixelScale);

        nvgBeginFrame(nvg, getWidth(), getHeight(), pixelScale);

        nvgSave(nvg);
        nvgScissor(nvg, 0, 0, getWidth(), getHeight());
        nvgFillPaint(nvg, nvgImagePattern(nvg, 0, 0, getWidth(), getHeight(), 0, mainFBO->image, 1));
        nvgFillRect(nvg, 0, 0, getWidth(), getHeight());
        nvgRestore(nvg);

#if ENABLE_FPS_COUNT
        nvgSave(nvg);
        frameTimer->render(nvg);
        nvgRestore(nvg);
#endif

        nvgEndFrame(nvg);

#ifdef NANOVG_GL_IMPLEMENTATION
        glContext->swapBuffers();
        if (resizing) {
            hresize = !hresize;
            resizing = false;
        }
        if (getBounds() != newBounds)
            startTimerHz(60);
#endif
        needsBufferSwap = false;
    }

    auto elapsed = Time::getMillisecondCounter() - startTime;
    // We update frambuffers after we call swapBuffers to make sure the frame is on time
    if (elapsed < 14) {
        for (auto* cnv : editor->getTabComponent().getVisibleCanvases()) {
            cnv->updateFramebuffers(nvg, cnv->getLocalBounds(), 14 - elapsed);
        }
    }
}

NVGSurface* NVGSurface::getSurfaceForContext(NVGcontext* nvg)
{
    if (!surfaces.count(nvg))
        return nullptr;

    return surfaces[nvg];
}
