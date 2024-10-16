/*
 // Copyright (c) 2021-2022 Timothy Schoen
 // For information on usage and redistribution, and for a DISCLAIMER OF ALL
 // WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 */
#include <juce_gui_basics/juce_gui_basics.h>
#include "Utility/Config.h"
#include "Utility/Fonts.h"

#include "Sidebar/Sidebar.h"
#include "Statusbar.h"
#include "Canvas.h"
#include "Object.h"
#include "Connection.h"
#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "LookAndFeel.h"
#include "Components/SuggestionComponent.h"
#include "CanvasViewport.h"

#include "Objects/ObjectBase.h"

#include "Dialogs/Dialogs.h"
#include "Components/GraphArea.h"
#include "Utility/RateReducer.h"

extern "C" {
void canvas_setgraph(t_glist* x, int flag, int nogoprect);
}

Canvas::Canvas(PluginEditor* parent, pd::Patch::Ptr p, Component* parentGraph)
    : NVGComponent(this)
    , editor(parent)
    , pd(parent->pd)
    , refCountedPatch(p)
    , patch(*p)
    , canvasOrigin(Point<int>(infiniteCanvasSize / 2, infiniteCanvasSize / 2))
    , graphArea(nullptr)
    , pathUpdater(new ConnectionPathUpdater(this))
    , globalMouseListener(this)
{

    addAndMakeVisible(objectLayer);
    addAndMakeVisible(connectionLayer);

    objectLayer.setInterceptsMouseClicks(false, true);
    connectionLayer.setInterceptsMouseClicks(false, true);

    if (auto patchPtr = patch.getPointer()) {
        isGraphChild = glist_isgraph(patchPtr.get());
    }

    hideNameAndArgs = static_cast<bool>(patch.getPointer()->gl_hidetext);
    xRange = Array<var> { var(patch.getPointer()->gl_x1), var(patch.getPointer()->gl_x2) };
    yRange = Array<var> { var(patch.getPointer()->gl_y2), var(patch.getPointer()->gl_y1) };

    pd->registerMessageListener(patch.getUncheckedPointer(), this);

    isGraphChild.addListener(this);
    hideNameAndArgs.addListener(this);
    xRange.addListener(this);
    yRange.addListener(this);

    auto patchBounds = patch.getBounds();
    patchWidth = patchBounds.getWidth();
    patchHeight = patchBounds.getHeight();

    patchWidth.addListener(this);
    patchHeight.addListener(this);

    globalMouseListener.globalMouseMove = [this](MouseEvent const& e) {
        lastMouseX = e.x;
        lastMouseY = e.y;
    };
    globalMouseListener.globalMouseDrag = [this](MouseEvent const& e) {
        lastMouseX = e.x;
        lastMouseY = e.y;
    };

    suggestor = std::make_unique<SuggestionComponent>();

    // Check if canvas belongs to a graph
    if (parentGraph) {
        setLookAndFeel(&editor->getLookAndFeel());
        parentGraph->addAndMakeVisible(this);
        setInterceptsMouseClicks(false, true);
        isGraph = true;
    } else {
        isGraph = false;
    }
    if (!isGraph) {
        auto* canvasViewport = new CanvasViewport(editor, this);

        canvasViewport->setViewedComponent(this, false);

        canvasViewport->onScroll = [this]() {
            if (suggestor) {
                suggestor->updateBounds();
            }
            if (graphArea) {
                graphArea->updateBounds();
            }
        };

        canvasViewport->setScrollBarsShown(true, true, true, true);

        viewport.reset(canvasViewport); // Owned by the tabbar, but doesn't exist for graph!
        restoreViewportState();
    }

    commandLocked.referTo(pd->commandLocked);
    commandLocked.addListener(this);

    // init border for testing
    propertyChanged("border", SettingsFile::getInstance()->getPropertyAsValue("border"));

    // Add draggable border for setting graph position
    if (getValue<bool>(isGraphChild) && !isGraph) {
        graphArea = std::make_unique<GraphArea>(this);
        addAndMakeVisible(*graphArea);
        graphArea->setAlwaysOnTop(true);
    }

    setSize(infiniteCanvasSize, infiniteCanvasSize);

    // initialize to default zoom
    auto defaultZoom = SettingsFile::getInstance()->getPropertyAsValue("default_zoom");
    zoomScale.setValue(getValue<float>(defaultZoom) / 100.0f);
    zoomScale.addListener(this);

    // Add lasso component
    addAndMakeVisible(&lasso);
    lasso.setAlwaysOnTop(true);

    setWantsKeyboardFocus(true);

    if (!isGraph) {
        presentationMode.addListener(this);
    } else {
        presentationMode = false;
    }
    performSynchronise();

    // Start in unlocked mode if the patch is empty
    if (objects.isEmpty()) {
        locked = false;
        patch.getPointer()->gl_edit = false;
    } else {
        locked = !patch.getPointer()->gl_edit;
    }

    locked.addListener(this);

    editor->addModifierKeyListener(this);

    updateOverlays();
    orderConnections();

    parameters.addParamBool("Is graph", cGeneral, &isGraphChild, { "No", "Yes" }, 0);
    parameters.addParamBool("Hide name and arguments", cGeneral, &hideNameAndArgs, { "No", "Yes" }, 0);
    parameters.addParamRange("X range", cGeneral, &xRange, { 0.0f, 1.0f });
    parameters.addParamRange("Y range", cGeneral, &yRange, { 1.0f, 0.0f });

    auto onInteractionFn = [this](bool state) {
        dimensionsAreBeingEdited = state;
        repaint();
    };

    parameters.addParamInt("Width", cDimensions, &patchWidth, 527, onInteractionFn);
    parameters.addParamInt("Height", cDimensions, &patchHeight, 327, onInteractionFn);

    updatePatchSnapshot();
}

Canvas::~Canvas()
{
    saveViewportState();
    zoomScale.removeListener(this);
    editor->removeModifierKeyListener(this);
    pd->unregisterMessageListener(patch.getUncheckedPointer(), this);
}

bool Canvas::updateFramebuffers(NVGcontext* nvg, Rectangle<int> invalidRegion, int maxUpdateTimeMs)
{
    auto start = Time::getMillisecondCounter();
    auto pixelScale = getRenderScale();
    auto zoom = isScrolling ? 2.0f : getValue<float>(zoomScale);

    int const logicalIoletsSize = 16 * 4;
    int const ioletBufferSize = logicalIoletsSize * pixelScale * zoom;

    // First, check if we need to update our iolet buffer
    if (ioletBuffer.needsUpdate(ioletBufferSize, ioletBufferSize)) {
        ioletBuffer.renderToFramebuffer(nvg, ioletBufferSize, ioletBufferSize, [this, zoom, ioletBufferSize, pixelScale](NVGcontext* nvg) {
            nvgViewport(0, 0, ioletBufferSize, ioletBufferSize);
            nvgClear(nvg);

            nvgBeginFrame(nvg, logicalIoletsSize * zoom, logicalIoletsSize * zoom, pixelScale);
            nvgScale(nvg, zoom, zoom);

            auto renderIolet = [](NVGcontext* nvg, Rectangle<float> bounds, NVGcolor background, NVGcolor outline) {
                if (PlugDataLook::getUseSquareIolets()) {
                    nvgBeginPath(nvg);
                    nvgRect(nvg, bounds.getX(), bounds.getY(), bounds.getWidth(), bounds.getHeight());

                    nvgFillColor(nvg, background);
                    nvgFill(nvg);

                    nvgStrokeColor(nvg, outline);
                    nvgStroke(nvg);
                } else {
                    nvgBeginPath(nvg);
                    nvgFillColor(nvg, background);
                    nvgCircle(nvg, bounds.getCentreX(), bounds.getCentreY(), bounds.getWidth() / 2.0f);
                    nvgFill(nvg);

                    nvgStrokeColor(nvg, outline);
                    nvgStroke(nvg);
                }
            };

            auto ioletColours = std::vector<Colour> {
                findColour(PlugDataColour::dataColourId),
                findColour(PlugDataColour::signalColourId),
                findColour(PlugDataColour::gemColourId),
                findColour(PlugDataColour::canvasBackgroundColourId).contrasting(0.5f)
            };

            auto outlineColour = findNVGColour(PlugDataColour::objectOutlineColourId);
            for (int i = 0; i < 4; i++) {
                auto backgroundColour = convertColour(ioletColours[i]);
                auto ioletRow = Rectangle<float>(0, i * 16 + 0.5f, logicalIoletsSize, 12.5f);
                renderIolet(nvg, ioletRow.removeFromLeft(16).reduced(4.0f), backgroundColour, outlineColour); // normal
                renderIolet(nvg, ioletRow.removeFromLeft(16).reduced(2.5f), backgroundColour, outlineColour); // hovered
            }

            nvgEndFrame(nvg);
        });

        editor->nvgSurface.invalidateAll();
    }

    int const resizerLogicalSize = 9;
    int const resizerBufferSize = resizerLogicalSize * pixelScale * zoom;

    if (resizeHandleImage.needsUpdate(resizerBufferSize, resizerBufferSize)) {
        resizeHandleImage = NVGImage(nvg, resizerBufferSize, resizerBufferSize, [this, pixelScale, zoom](Graphics& g) {
            g.addTransform(AffineTransform::scale(pixelScale * zoom, pixelScale * zoom));

            auto b = Rectangle<int>(0, 0, 9, 9);
            // use the path with a hole in it to exclude the inner rounded rect from painting
            Path outerArea;
            outerArea.addRectangle(b);
            outerArea.setUsingNonZeroWinding(false);

            Path innerArea;

            auto innerRect = b.translated(Object::margin / 2, Object::margin / 2);
            innerArea.addRoundedRectangle(innerRect, Corners::objectCornerRadius);
            outerArea.addPath(innerArea);
            g.reduceClipRegion(outerArea);

            g.setColour(findColour(PlugDataColour::objectSelectedOutlineColourId));
            g.fillRoundedRectangle(0.0f, 0.0f, 9.0f, 9.0f, Corners::objectCornerRadius);
        });

        editor->nvgSurface.invalidateAll();
    }

    // Then, check if object framebuffers need to be updated
    if (isScrolling) {
        if (viewport)
            invalidRegion = (invalidRegion + viewport->getViewPosition()) / zoom;
        for (auto* obj : objects) {
            auto b = obj->getBounds();
            if (b.intersects(invalidRegion)) {
                obj->updateFramebuffer(nvg);

                auto elapsed = Time::getMillisecondCounter() - start;
                if (elapsed > maxUpdateTimeMs) {
                    return false;
                }
            }
        }
    }

    return true;
}

// Callback from canvasViewport to perform actual rendering
void Canvas::performRender(NVGcontext* nvg, Rectangle<int> invalidRegion)
{
    auto backgroundColour = convertColour(findColour(PlugDataColour::canvasBackgroundColourId));
    auto dotsColour = convertColour(findColour(PlugDataColour::canvasDotsColourId));

    auto const halfSize = infiniteCanvasSize / 2;
    auto const zoom = getValue<float>(zoomScale);

    // apply translation to the canvas nvg objects
    nvgSave(nvg);

    if (viewport) {
        nvgTranslate(nvg, -viewport->getViewPositionX(), -viewport->getViewPositionY());
        nvgScale(nvg, zoom, zoom);

        invalidRegion = invalidRegion.translated(viewport->getViewPositionX(), viewport->getViewPositionY());
        invalidRegion /= zoom;

        nvgFillColor(nvg, backgroundColour);
        nvgFillRect(nvg, invalidRegion.getX(), invalidRegion.getY(), invalidRegion.getWidth(), invalidRegion.getHeight());
    }

    if (viewport && !getValue<bool>(locked)) {
        nvgBeginPath(nvg);
        nvgRect(nvg, 0, 0, infiniteCanvasSize, infiniteCanvasSize);

        auto feather = getRenderScale() > 1.0f ? 0.25f : 0.75f;
        if (getValue<float>(zoomScale) >= 1.0f) {
            nvgSave(nvg);
            nvgTranslate(nvg, canvasOrigin.x % objectGrid.gridSize, canvasOrigin.y % objectGrid.gridSize); // Make sure grid aligns with origin
            NVGpaint dots = nvgDotPattern(nvg, dotsColour, nvgRGBA(0, 0, 0, 0), objectGrid.gridSize, 1.0f, feather);
            nvgFillPaint(nvg, dots);
            nvgFill(nvg);
            nvgRestore(nvg);
        } else {
            nvgSave(nvg);
            nvgTranslate(nvg, canvasOrigin.x % (objectGrid.gridSize * 4), canvasOrigin.y % (objectGrid.gridSize * 4)); // Make sure grid aligns with origin
            auto darkDotColour = convertColour(findColour(PlugDataColour::canvasBackgroundColourId).contrasting());
            auto scaledDotSize = jmap(zoom, 1.0f, 0.25f, 1.0f, 4.0f);
            if (zoom < 0.3f && getRenderScale() <= 1.0f)
                scaledDotSize = jmap(zoom, 0.3f, 0.25f, 4.0f, 8.0f);

            for (int i = 0; i < 4; i++) {
                nvgTranslate(nvg, objectGrid.gridSize, 0);

                NVGpaint dots = nvgDotPattern(nvg, i == 3 ? darkDotColour : dotsColour, nvgRGBA(0, 0, 0, 0), objectGrid.gridSize * 4, scaledDotSize, feather + 0.2f);
                nvgFillPaint(nvg, dots);
                nvgFill(nvg);
            }
            nvgRestore(nvg);
            nvgSave(nvg);

            for (int i = 0; i < 4; i++) {
                nvgTranslate(nvg, 0, objectGrid.gridSize);
                NVGpaint dots = nvgDotPattern(nvg, i == 3 ? darkDotColour : dotsColour, nvgRGBA(0, 0, 0, 0), objectGrid.gridSize * 4, scaledDotSize, feather + 0.2f);
                nvgFillPaint(nvg, dots);
                nvgFill(nvg);
            }

            nvgRestore(nvg);
        }
    }
    auto drawBorder = [this, nvg, backgroundColour, zoom, dotsColour](bool bg, bool fg) {
        if (viewport && (showOrigin || showBorder) && !::getValue<bool>(presentationMode)) {
            nvgBeginPath(nvg);

            const auto borderWidth = getValue<float>(patchWidth);
            const auto borderHeight = getValue<float>(patchHeight);
            const auto pos = Point<int>(halfSize, halfSize);

            // Origin line paths. Draw both from {0, 0} so the strokes touch at the origin
            nvgMoveTo(nvg, pos.x, pos.y);
            nvgLineTo(nvg, pos.x, pos.y + (showOrigin ? halfSize : borderHeight));
            nvgMoveTo(nvg, pos.x, pos.y);
            nvgLineTo(nvg, pos.x + (showOrigin ? halfSize : borderWidth), pos.y);

            if (showBorder) {
                nvgMoveTo(nvg, pos.x + borderWidth, pos.y + borderHeight);
                nvgLineTo(nvg, pos.x + borderWidth, pos.y);
                nvgMoveTo(nvg, pos.x + borderWidth, pos.y + borderHeight);
                nvgLineTo(nvg, pos.x, pos.y + borderHeight);
            }

            if (bg) {
                // place solid line behind (to fake removeing grid points for now)
                nvgLineStyle(nvg, NVG_LINE_SOLID);
                nvgStrokeColor(nvg, backgroundColour);
                nvgStrokeWidth(nvg, 6.0f);
                nvgStroke(nvg);
            }
            if (fg) {
                auto scaledStrokeSize = zoom < 1.0f ? jmap(zoom, 1.0f, 0.25f, 1.5f, 4.0f) : 1.5f;
                if (zoom < 0.3f && getRenderScale() <= 1.0f)
                    scaledStrokeSize = jmap(zoom, 0.3f, 0.25f, 4.0f, 8.0f);

                // draw 0,0 point lines
                nvgLineStyle(nvg, NVG_LINE_DASHED);

                nvgStrokeColor(nvg, dotsColour);
                nvgStrokeWidth(nvg, scaledStrokeSize);
                nvgDashLength(nvg, 8.0f);
                nvgStroke(nvg);

                // Connect origin lines at {0, 0}
                nvgBeginPath(nvg);
                nvgMoveTo(nvg, pos.x + 2.0f, pos.y);
                nvgLineTo(nvg, pos.x, pos.y);
                nvgLineTo(nvg, pos.x, pos.y + 2.0f);
                nvgLineStyle(nvg, NVG_LINE_SOLID);
                nvgStrokeWidth(nvg, 1.25f);
                nvgStroke(nvg);
            }
        }
    };

    if (!dimensionsAreBeingEdited)
        drawBorder(true, true);
    else
        drawBorder(true, false);

    // Render objects like [drawcurve], [fillcurve] etc. at the back
    for (auto drawable : drawables) {
        if (drawable) {
            auto* component = dynamic_cast<Component*>(drawable.get());
            if (invalidRegion.intersects(component->getBounds())) {
                drawable->render(nvg);
            }
        }
    }

    if (::getValue<bool>(presentationMode) || isGraph) {
        renderAllObjects(nvg, invalidRegion);
        // render presentation mode as clipped 'virtual' plugin view
        if (::getValue<bool>(presentationMode)) {
            auto const borderWidth = getValue<float>(patchWidth);
            auto const borderHeight = getValue<float>(patchHeight);
            auto const pos = Point<int>(halfSize, halfSize);
            auto const scale = getValue<float>(zoomScale);
            auto const windowCorner = Corners::windowCornerRadius / scale;

            auto const bgColour = convertColour(findColour(PlugDataColour::presentationBackgroundColourId));
            auto const windowOutlineColour = convertColour(findColour(PlugDataColour::presentationBackgroundColourId).contrasting(0.3f));

            nvgSave(nvg);

            // background colour to crop outside of border area
            nvgBeginPath(nvg);
            nvgRect(nvg, 0, 0, infiniteCanvasSize, infiniteCanvasSize);
            nvgPathWinding(nvg, NVG_HOLE);
            nvgRoundedRect(nvg, pos.getX(), pos.getY(), borderWidth, borderHeight, windowCorner);
            nvgFillColor(nvg, bgColour);
            nvgFill(nvg);

            // background drop shadow to simulate a virtual plugin
            nvgBeginPath(nvg);
            nvgRect(nvg, 0, 0, infiniteCanvasSize, infiniteCanvasSize);
            nvgPathWinding(nvg, NVG_HOLE);
            nvgRoundedRect(nvg, pos.getX(), pos.getY(), borderWidth, borderHeight, windowCorner);

            int const shadowSize = 24 / scale;
            auto borderArea = Rectangle<int>(0, 0, borderWidth, borderHeight).expanded(shadowSize);
            if (presentationShadowImage.needsUpdate(borderArea.getWidth(), borderArea.getHeight())) {
                presentationShadowImage = NVGImage(nvg, borderArea.getWidth(), borderArea.getHeight(), [borderArea, shadowSize, windowCorner](Graphics& g) {
                    auto shadowPath = Path();
                    shadowPath.addRoundedRectangle(borderArea.reduced(shadowSize).withPosition(shadowSize, shadowSize), windowCorner);
                    StackShadow::renderDropShadow(g, shadowPath, Colours::black, shadowSize, Point<int>(0, 2));
                });
            }
            auto shadowImage = nvgImagePattern(nvg, pos.getX() - shadowSize, pos.getY() - shadowSize, borderArea.getWidth(), borderArea.getHeight(), 0, presentationShadowImage.getImageId(), 0.16f);

            nvgStrokeColor(nvg, windowOutlineColour);
            nvgStrokeWidth(nvg, 0.5f / scale);
            nvgFillPaint(nvg, shadowImage);
            nvgFill(nvg);
            nvgStroke(nvg);

            nvgRestore(nvg);
        }
    }
    // render connections infront or behind objects depending on lock mode or overlay setting
    else {
        if (connectionsBehind) {
            renderAllConnections(nvg, invalidRegion);
            renderAllObjects(nvg, invalidRegion);
        } else {
            renderAllObjects(nvg, invalidRegion);
            renderAllConnections(nvg, invalidRegion);
        }
    }

    for (auto* connection : connectionsBeingCreated) {
        nvgSave(nvg);
        connection->render(nvg);
        nvgRestore(nvg);
    }

    if (graphArea) {
        nvgSave(nvg);
        nvgTranslate(nvg, graphArea->getX(), graphArea->getY());
        graphArea->render(nvg);
        nvgRestore(nvg);
    }

    objectGrid.render(nvg);

    if (viewport && lasso.isVisible() && !lasso.getBounds().isEmpty()) {
        auto lassoBounds = lasso.getBounds().toFloat().reduced(1.0f);
        auto smallestSide = lassoBounds.getWidth() < lassoBounds.getHeight() ? lassoBounds.getWidth() : lassoBounds.getHeight();

        auto fillColour = convertColour(findColour(PlugDataColour::objectSelectedOutlineColourId).withAlpha(0.075f));
        auto outlineColour = convertColour(findColour(PlugDataColour::canvasBackgroundColourId).interpolatedWith(findColour(PlugDataColour::objectSelectedOutlineColourId), 0.65f));

        nvgBeginPath(nvg);
        nvgFillColor(nvg, fillColour);
        nvgRect(nvg, lassoBounds.getX(), lassoBounds.getY(), lassoBounds.getWidth(), lassoBounds.getHeight());
        nvgFill(nvg);
        nvgStrokeColor(nvg, outlineColour);
        nvgStrokeWidth(nvg, smallestSide < 1.0f ? 0.5f : 1.0f); // if one of the sides is smaller than 1px, we need to adjust the stroke width to prevent drawing out of bounds
        nvgStroke(nvg);
    }

    suggestor->renderAutocompletion(nvg);

    if (dimensionsAreBeingEdited)
        drawBorder(false, true);

    nvgRestore(nvg);

    // Draw scrollbars
    if (viewport) {
        reinterpret_cast<CanvasViewport*>(viewport.get())->render(nvg);
    }
}

float Canvas::getRenderScale() const
{
    return editor->nvgSurface.getRenderScale();
}

void Canvas::updatePatchSnapshot()
{
    auto patchFile = patch.getCurrentFile();

    if (patchFile.existsAsFile()) {
        auto recentlyOpenedTree = SettingsFile::getInstance()->getValueTree().getChildWithName("RecentlyOpened");
        for (int i = 0; i < recentlyOpenedTree.getNumChildren(); i++) {
            auto recentlyOpenedFile = File(recentlyOpenedTree.getChild(i).getProperty("Path").toString());

            // Check if patch is in the recently opened list
            if (File(recentlyOpenedFile) == patchFile) {
                // If so, generate an svg sihouette that we can show on the welcome page
                String svgSilhouette;

                auto regionOfInterest = Rectangle<int>();
                for (auto* object : objects) {
                    regionOfInterest = regionOfInterest.getUnion(object->getBounds().reduced(Object::margin));
                }

                MemoryOutputStream objectBoundsStream;

                for (auto* object : objects) {
                    auto rect = object->getBounds().reduced(Object::margin) - regionOfInterest.getPosition();
                    objectBoundsStream.writeCompressedInt(rect.getX());
                    objectBoundsStream.writeCompressedInt(rect.getY());
                    objectBoundsStream.writeCompressedInt(rect.getWidth());
                    objectBoundsStream.writeCompressedInt(rect.getHeight());
                }

                recentlyOpenedTree.getChild(i).setProperty("PatchImage", Base64::toBase64(objectBoundsStream.getData(), objectBoundsStream.getDataSize()), nullptr);
                break;
            }
        }
    }
}

void Canvas::renderAllObjects(NVGcontext* nvg, Rectangle<int> area)
{
    for (auto* obj : objects) {
        auto b = obj->getBounds();

        nvgSave(nvg);
        nvgTranslate(nvg, b.getX(), b.getY());
        if (b.intersects(area) && obj->isVisible()) {
            obj->render(nvg);
        }
        nvgRestore(nvg);

        // Draw label in canvas coordinates
        obj->renderLabel(nvg);
    }
}
void Canvas::renderAllConnections(NVGcontext* nvg, Rectangle<int> area)
{
    if (!connectionLayer.isVisible())
        return;

    Array<Connection*> connectionsToDraw;

    for (auto* connection : connections) {
        nvgSave(nvg);
        if (connection->intersectsRectangle(area) && connection->isVisible()) {
            connection->render(nvg);
            if (showConnectionOrder)
                connectionsToDraw.add(connection);
        }
        nvgRestore(nvg);
    }
    if (!connectionsToDraw.isEmpty()) {
        for (auto* connection : connectionsToDraw) {
            nvgSave(nvg);
            connection->renderConnectionOrder(nvg);
            nvgRestore(nvg);
        }
    }
}

void Canvas::propertyChanged(String const& name, var const& value)
{
    switch (hash(name)) {
    case hash("grid_size"):
        repaint();
        break;
    case hash("border"):
        showBorder = static_cast<int>(value);
        repaint();
        break;
    case hash("edit"):
    case hash("lock"):
    case hash("run"):
    case hash("alt"):
    case hash("alt_mode"): {
        updateOverlays();
        break;
    }
    }
}

bool Canvas::shouldShowObjectActivity()
{
    return showObjectActivity && !presentationMode.getValue() && !isGraph;
}

bool Canvas::shouldShowIndex()
{
    return showIndex && !presentationMode.getValue();
}

bool Canvas::shouldShowConnectionDirection()
{
    return showConnectionDirection;
}

bool Canvas::shouldShowConnectionActivity()
{
    return showConnectionActivity;
}

int Canvas::getOverlays() const
{
    int overlayState = 0;

    auto overlaysTree = SettingsFile::getInstance()->getValueTree().getChildWithName("Overlays");

    auto altModeEnabled = overlaysTree.getProperty("alt_mode");

    if (!locked.getValue()) {
        overlayState = overlaysTree.getProperty("edit");
    }
    if (locked.getValue() || commandLocked.getValue()) {
        overlayState = overlaysTree.getProperty("lock");
    }
    if (presentationMode.getValue()) {
        overlayState = overlaysTree.getProperty("run");
    }
    if (altModeEnabled) {
        overlayState = overlaysTree.getProperty("alt");
    }

    return overlayState;
}

void Canvas::updateOverlays()
{
    int overlayState = getOverlays();

    showBorder = overlayState & Border;
    showOrigin = overlayState & Origin;
    showConnectionOrder = overlayState & Order;
    connectionsBehind = overlayState & Behind;
    showObjectActivity = overlayState & ActivationState;
    showIndex = overlayState & Index;
    showConnectionDirection = overlayState & Direction;
    showConnectionActivity = overlayState & ConnectionActivity;

    orderConnections();

    repaint();
}

void Canvas::jumpToOrigin()
{
    if (viewport)
        viewport->setViewPosition((canvasOrigin + Point<int>(1, 1)).transformedBy(getTransform()));
}

void Canvas::restoreViewportState()
{
    if (viewport) {
        viewport->setViewPosition((patch.lastViewportPosition + canvasOrigin).transformedBy(getTransform()));
        zoomScale.setValue(patch.lastViewportScale);
        setTransform(AffineTransform().scaled(patch.lastViewportScale));
    }
}

void Canvas::saveViewportState()
{
    if (viewport) {
        patch.lastViewportPosition = viewport->getViewPosition().transformedBy(getTransform().inverted()) - canvasOrigin;
        patch.lastViewportScale = getValue<float>(zoomScale);
    }
}

void Canvas::zoomToFitAll()
{
    if (objects.isEmpty() || !viewport)
        return;

    auto scale = getValue<float>(zoomScale);

    auto regionOfInterest = Rectangle<int>(canvasOrigin.x, canvasOrigin.y, getValue<float>(patchWidth), getValue<float>(patchHeight));

    if (!presentationMode.getValue()) {
        for (auto* object : objects) {
            regionOfInterest = regionOfInterest.getUnion(object->getBounds().reduced(Object::margin));
        }
    }

    // Add a bit of margin to make it nice
    regionOfInterest = regionOfInterest.expanded(16);

    auto viewArea = viewport->getViewArea() / scale;

    auto roiHeight = static_cast<float>(regionOfInterest.getHeight());
    auto roiWidth = static_cast<float>(regionOfInterest.getWidth());
    auto viewHeight = viewArea.getHeight();
    auto viewWidth = viewArea.getWidth();

    if (roiWidth > viewWidth || roiHeight > viewHeight) {
        auto scaleWidth = viewWidth / roiWidth;
        auto scaleHeight = viewHeight / roiHeight;
        scale = jmin(scaleWidth, scaleHeight);

        auto transform = getTransform();
        transform = transform.scaled(scale);
        setTransform(transform);

        scale = std::sqrt(std::abs(transform.getDeterminant()));
        zoomScale.setValue(scale);
    }

    auto viewportCentre = viewport->getViewArea().withZeroOrigin().getCentre();
    auto newViewPos = regionOfInterest.transformedBy(getTransform()).getCentre() - viewportCentre;
    viewport->setViewPosition(newViewPos);
}

void Canvas::tabChanged()
{
    patch.setCurrent();

    synchronise();
    updateDrawables();

    for (auto* obj : objects) {
        if (!obj->gui)
            continue;

        obj->gui->tabChanged();
    }

    editor->statusbar->updateZoomLevel();
    editor->repaint(); // Make sure everything it up to date
}

void Canvas::save(std::function<void()> const& nestedCallback)
{
    Canvas* canvasToSave = this;
    if (patch.isSubpatch()) {
        for (auto& parentCanvas : editor->getCanvases()) {
            if (patch.getRoot() == parentCanvas->patch.getPointer().get()) {
                canvasToSave = parentCanvas;
            }
        }
    }

    if (canvasToSave->patch.getCurrentFile().existsAsFile()) {
        canvasToSave->updatePatchSnapshot();
        canvasToSave->patch.savePatch();
        SettingsFile::getInstance()->addToRecentlyOpened(canvasToSave->patch.getCurrentFile());
        nestedCallback();
        pd->titleChanged();
    } else {
        saveAs(nestedCallback);
    }
}

void Canvas::saveAs(std::function<void()> const& nestedCallback)
{
    Dialogs::showSaveDialog([this, nestedCallback](URL resultURL) mutable {
        auto result = resultURL.getLocalFile();
        if (result.getFullPathName().isNotEmpty()) {
            if (result.exists())
                result.deleteFile();

            if (!result.hasFileExtension("pd"))
                result = result.getFullPathName() + ".pd";

            updatePatchSnapshot();
            patch.savePatch(resultURL);
            SettingsFile::getInstance()->addToRecentlyOpened(result);
            pd->titleChanged();
        }

        nestedCallback();
    },
        "*.pd", "Patch", this);
}

void Canvas::handleAsyncUpdate()
{
    performSynchronise();
}

void Canvas::synchronise()
{
    triggerAsyncUpdate();
}

void Canvas::synchroniseSplitCanvas()
{
    for (auto* canvas : editor->getTabComponent().getVisibleCanvases()) {
        canvas->synchronise();
    }
}

// Synchronise state with pure-data
// Used for loading and for complicated actions like undo/redo
void Canvas::performSynchronise()
{
    if(auto patchPtr = patch.getPointer()) {
        patch.setCurrent();
        pd->sendMessagesFromQueue();
    }
    else {
        return;
    }

    // Remove deleted connections
    for (int n = connections.size() - 1; n >= 0; n--) {
        if (!connections[n]->getPointer()) {
            connections.remove(n);
        }
    }

    // Remove deleted objects
    for (int n = objects.size() - 1; n >= 0; n--) {
        auto* object = objects[n];

        // If the object is showing it's initial editor, meaning no object was assigned yet, allow it to exist without pointing to an object
        if ((!object->getPointer() || patch.objectWasDeleted(object->getPointer())) && !object->isInitialEditorShown()) {
            setSelected(object, false, false);
            objects.remove(n);
        }
    }

    // Check for connections that need to be remade because of invalid iolets
    for (int n = connections.size() - 1; n >= 0; n--) {
        if (!connections[n]->inlet || !connections[n]->outlet) {
            connections.remove(n);
        }
    }

    auto pdObjects = patch.getObjects();

    for (auto object : pdObjects) {
        auto* it = std::find_if(objects.begin(), objects.end(), [&object](Object* b) { return b->getPointer() && b->getPointer() == object.getRawUnchecked<void>(); });
        if (!object.isValid())
            continue;

        if (it == objects.end()) {
            auto* newBox = objects.add(new Object(object, this));
            newBox->toFront(false);

            if (newBox->gui && newBox->gui->getLabel())
                newBox->gui->getLabel()->toFront(false);
        } else {
            auto* object = *it;

            // Check if number of inlets/outlets is correct
            object->updateIolets();
            object->updateBounds();

            object->toFront(false);
            if (object->gui && object->gui->getLabel())
                object->gui->getLabel()->toFront(false);
            if (object->gui)
                object->gui->update();
        }
    }

    // Make sure objects have the same order
    std::sort(objects.begin(), objects.end(),
        [&pdObjects](Object* first, Object* second) mutable {
            size_t idx1 = std::find(pdObjects.begin(), pdObjects.end(), first->getPointer()) - pdObjects.begin();
            size_t idx2 = std::find(pdObjects.begin(), pdObjects.end(), second->getPointer()) - pdObjects.begin();

            return idx1 < idx2;
        });

    auto pdConnections = patch.getConnections();

    for (auto& connection : pdConnections) {
        auto& [ptr, inno, inobj, outno, outobj] = connection;

        Iolet *inlet = nullptr, *outlet = nullptr;

        // Find the objects that this connection is connected to
        for (auto* obj : objects) {
            if (outobj && &outobj->te_g == obj->getPointer()) {

                // Check if we have enough outlets, should never return false
                if (isPositiveAndBelow(obj->numInputs + outno, obj->iolets.size())) {
                    outlet = obj->iolets[obj->numInputs + outno];
                } else {
                    break;
                }
            }
            if (inobj && &inobj->te_g == obj->getPointer()) {

                // Check if we have enough inlets, should never return false
                if (isPositiveAndBelow(inno, obj->iolets.size())) {
                    inlet = obj->iolets[inno];
                } else {
                    break;
                }
            }
        }

        // This shouldn't be necessary, but just to be sure...
        if (!inlet || !outlet) {
            jassertfalse;
            continue;
        }

        auto* it = std::find_if(connections.begin(), connections.end(),
            [c_ptr = ptr](auto* c) {
                return c_ptr == c->getPointer();
            });

        if (it == connections.end()) {
            connections.add(new Connection(this, inlet, outlet, ptr));
        } else {
            auto& c = *(*it);

            // This is necessary to make resorting a subpatchers iolets work
            // And it can't hurt to check if the connection is valid anyway
            if (c.inlet != inlet || c.outlet != outlet) {
                int idx = connections.indexOf(*it);
                connections.removeObject(*it);
                connections.insert(idx, new Connection(this, inlet, outlet, ptr));
            } else {
                c.popPathState();
            }
        }
    }

    if (!isGraph) {
        setTransform(AffineTransform().scaled(getValue<float>(zoomScale)));
    }

    if (graphArea)
        graphArea->updateBounds();

    editor->updateCommandStatus();
    repaint();

    needsSearchUpdate = true;

    pd->updateObjectImplementations();
}

void Canvas::updateDrawables()
{
    for (auto* object : objects) {
        if (object->gui) {
            object->gui->updateDrawables();
        }
    }
}

void Canvas::commandKeyChanged(bool isHeld)
{
    commandLocked = isHeld;
}

void Canvas::middleMouseChanged(bool isHeld)
{
    checkPanDragMode();
}

void Canvas::altKeyChanged(bool isHeld)
{
    SettingsFile::getInstance()->getValueTree().getChildWithName("Overlays").setProperty("alt_mode", isHeld, nullptr);
}

void Canvas::mouseDown(MouseEvent const& e)
{
    PopupMenu::dismissAllActiveMenus();

    if (checkPanDragMode())
        return;

    auto* source = e.originalComponent;

    // Left-click
    if (!e.mods.isRightButtonDown()) {

        if (source == this) {
            dragState.duplicateOffset = {0, 0};
            dragState.lastDuplicateOffset = {0, 0};
            dragState.wasDuplicated = false;
            cancelConnectionCreation();

            if (e.mods.isCommandDown()) {
                // Lock if cmd + click on canvas
                deselectAll();

                presentationMode.setValue(false);

                // when command + click on canvas, swap between locked / edit mode
                locked.setValue(!locked.getValue());
                locked.getValueSource().sendChangeMessage(true);

                updateOverlays();
            }
            if (!e.mods.isShiftDown()) {
                deselectAll();
            }

            if (!(e.source.isTouch() && e.source.getIndex() != 0) && !getValue<bool>(locked)) {
                lasso.beginLasso(e.getEventRelativeTo(this), this);
                isDraggingLasso = true;
            }
        }

        // Update selected object in sidebar when we click a object
        if (source && source->findParentComponentOfClass<Object>()) {
            updateSidebarSelection();
        }

        editor->updateCommandStatus();
    }
    // Right click
    else {
        Dialogs::showCanvasRightClickMenu(this, source, e.getScreenPosition());
    }
}

bool Canvas::hitTest(int x, int y)
{
    // allow panning to happen anywhere, even when in presentation mode
    if (panningModifierDown())
        return true;

    // disregard mouse drag if outside of patch
    if (::getValue<bool>(presentationMode)) {
        if (isPointOutsidePluginArea(Point<int>(x, y)))
            return false;
    }
    return true;
}

void Canvas::mouseDrag(MouseEvent const& e)
{
    if (canvasRateReducer.tooFast() || panningModifierDown())
        return;

    if (connectingWithDrag) {
        for (auto* obj : objects) {
            for (auto* iolet : obj->iolets) {
                iolet->mouseDrag(e.getEventRelativeTo(iolet));
            }
        }
    }

    bool objectIsBeingEdited = ObjectBase::isBeingEdited();

    // Ignore on graphs or when locked
    if ((isGraph || locked == var(true) || commandLocked == var(true)) && !objectIsBeingEdited) {
        bool hasToggled = false;

        // Behaviour for dragging over toggles, bang and radiogroup to toggle them
        for (auto* object : objects) {
            if (!object->getBounds().contains(e.getEventRelativeTo(this).getPosition()) || !object->gui)
                continue;

            if (auto* obj = object->gui.get()) {
                obj->toggleObject(e.getEventRelativeTo(obj).getPosition());
                hasToggled = true;
                break;
            }
        }

        if (!hasToggled) {
            for (auto* object : objects) {
                if (auto* obj = object->gui.get()) {
                    obj->untoggleObject();
                }
            }
        }

        return;
    }

    auto viewportEvent = e.getEventRelativeTo(viewport.get());
    if (viewport && !ObjectBase::isBeingEdited() && autoscroll(viewportEvent)) {
        beginDragAutoRepeat(25);
    }

    // Drag lasso
    if (!(e.source.isTouch() && e.source.getIndex() != 0)) {
        lasso.dragLasso(e);
    }
}

bool Canvas::autoscroll(MouseEvent const& e)
{
    if (!viewport)
        return false;

    auto x = viewport->getViewPositionX();
    auto y = viewport->getViewPositionY();
    auto oldY = y;
    auto oldX = x;

    auto pos = e.getPosition();

    if (pos.x > viewport->getWidth()) {
        x += std::clamp((pos.x - viewport->getWidth()) / 6, 1, 14);
    } else if (pos.x < 0) {
        x -= std::clamp(-pos.x / 6, 1, 14);
    }
    if (pos.y > viewport->getHeight()) {
        y += std::clamp((pos.y - viewport->getHeight()) / 6, 1, 14);
    } else if (pos.y < 0) {
        y -= std::clamp(-pos.y / 6, 1, 14);
    }

    if (x != oldX || y != oldY) {
        viewport->setViewPosition(x, y);
        return true;
    }

    return false;
}

Point<int> Canvas::getLastMousePosition()
{
    return { lastMouseX, lastMouseY };
}

void Canvas::mouseUp(MouseEvent const& e)
{
    setPanDragMode(false);
    setMouseCursor(MouseCursor::NormalCursor);

    connectionCancelled = false;

    // Double-click canvas to create new object
    if (e.mods.isLeftButtonDown() && (e.getNumberOfClicks() == 2) && (e.originalComponent == this) && !isGraph && !getValue<bool>(locked)) {
        objects.add(new Object(this, "", e.getPosition()));
        deselectAll();
        setSelected(objects[objects.size() - 1], true); // Select newly created object
    }

    // Make sure the drag-over toggle action is ended
    if (!isDraggingLasso) {
        for (auto* object : objects) {
            if (auto* obj = object->gui.get()) {
                obj->untoggleObject();
            }
        }
    }

    updateSidebarSelection();

    editor->updateCommandStatus();

    lasso.endLasso();
    isDraggingLasso = false;
    for (auto* object : objects)
        object->originalBounds = Rectangle<int>(0, 0, 0, 0);

    // TODO: this is a hack, find a better solution
    if (connectingWithDrag) {
        for (auto* obj : objects) {
            for (auto* iolet : obj->iolets) {
                auto relativeEvent = e.getEventRelativeTo(this);
                if (iolet->getCanvasBounds().expanded(50).contains(relativeEvent.getPosition())) {
                    iolet->mouseUp(relativeEvent);
                }
            }
        }
    }
}

void Canvas::updateSidebarSelection()
{
#if JUCE_IOS
    editor->showTouchSelectionHelper(selectedComponents.getNumSelected());
#endif

    auto lassoSelection = getSelectionOfType<Object>();

    if (lassoSelection.size() > 0) {
        Array<ObjectParameters> allParameters;
        for (auto* object : lassoSelection) {
            if (!object->gui)
                continue;
            auto parameters = object->gui ? object->gui->getParameters() : ObjectParameters();
            auto showOnSelect = object->gui && object->gui->showParametersWhenSelected();
            if (showOnSelect) {
                allParameters.add(parameters);
            }
        }

        if (!allParameters.isEmpty() || editor->sidebar->isPinned()) {
            String objectName = "(" + String(lassoSelection.size()) + " selected)";
            if (lassoSelection.size() == 1 && lassoSelection.getFirst()) {
                objectName = lassoSelection.getFirst()->getType(false);
            }

            editor->sidebar->showParameters(objectName, allParameters);
        } else {
            editor->sidebar->hideParameters();
        }
    } else {
        editor->sidebar->hideParameters();
    }
}

bool Canvas::keyPressed(KeyPress const& key)
{
    if (editor->getCurrentCanvas() != this || isGraph)
        return false;

    int keycode = key.getKeyCode();

    auto moveSelection = [this](int x, int y) {
        auto objects = getSelectionOfType<Object>();
        if (objects.isEmpty())
            return false;

        std::vector<t_gobj*> pdObjects;

        for (auto* object : objects) {
            if (auto* ptr = object->getPointer()) {
                pdObjects.push_back(ptr);
            }
        }

        patch.moveObjects(pdObjects, x, y);

        // Update object bounds and store the total bounds of the selection
        auto totalBounds = Rectangle<int>();
        for (auto* object : objects) {
            object->updateBounds();
            totalBounds = totalBounds.getUnion(object->getBounds());
        }

        // TODO: consider calculating the totalBounds with object->getBounds().reduced(Object::margin)
        // then adding viewport padding in screen pixels so it's consistent regardless of scale
        auto scale = ::getValue<float>(zoomScale);
        auto viewportPadding = 10;

        auto viewX = viewport->getViewPositionX() / scale;
        auto viewY = viewport->getViewPositionY() / scale;
        auto viewWidth = (viewport->getWidth() - viewportPadding) / scale;
        auto viewHeight = (viewport->getHeight() - viewportPadding) / scale;
        if (x < 0 && totalBounds.getX() < viewX) {
            viewX = totalBounds.getX();
        } else if (totalBounds.getRight() > viewX + viewWidth) {
            viewX = totalBounds.getRight() - viewWidth;
        }
        if (y < 0 && totalBounds.getY() < viewY) {
            viewY = totalBounds.getY();
        } else if (totalBounds.getBottom() > viewY + viewHeight) {
            viewY = totalBounds.getBottom() - viewHeight;
        }
        viewport->setViewPosition(viewX * scale, viewY * scale);
        return true;
    };

    // Cancel connections being created by ESC key
    if (keycode == KeyPress::escapeKey && !connectionsBeingCreated.isEmpty()) {
        cancelConnectionCreation();
        return true;
    }

    // Move objects with arrow keys
    int moveDistance = objectGrid.gridSize;
    if (key.getModifiers().isShiftDown()) {
        moveDistance = 1;
    } else if (key.getModifiers().isCommandDown()) {
        moveDistance *= 4;
    }

    if (keycode == KeyPress::leftKey) {
        moveSelection(-moveDistance, 0);
        return false;
    }
    if (keycode == KeyPress::rightKey) {
        moveSelection(moveDistance, 0);
        return false;
    }
    if (keycode == KeyPress::upKey) {
        moveSelection(0, -moveDistance);
        return false;
    }
    if (keycode == KeyPress::downKey) {
        moveSelection(0, moveDistance);
        return false;
    }

    return false;
}

void Canvas::deselectAll()
{
    selectedComponents.deselectAll();

    editor->sidebar->hideParameters();
}

void Canvas::hideAllActiveEditors()
{
    for (auto* object : objects) {
        object->hideEditor();
    }
}

void Canvas::copySelection()
{
    // Tell pd to select all objects that are currently selected
    std::vector<t_gobj*> objects;
    for (auto* object : getSelectionOfType<Object>()) {
        if (auto* ptr = object->getPointer()) {
            objects.push_back(ptr);
        }
    }

    // Tell pd to copy
    patch.copy(objects);
    patch.deselectAll();
}

void Canvas::focusGained(FocusChangeType cause)
{
    pd->enqueueFunctionAsync([_this = SafePointer(this), this, hasFocus = static_cast<float>(hasKeyboardFocus(true))]() {
        if (!_this)
            return;
        auto* glist = patch.getPointer().get();
        if (!glist)
            return;

        // canvas.active listener
        char buf[MAXPDSTRING];
        snprintf(buf, MAXPDSTRING - 1, ".x%lx.c", (unsigned long)glist);
        pd->sendMessage("#active_gui", "_focus", { pd::Atom(pd->generateSymbol(buf)), hasFocus });

        // cyclone focus listeners
        pd->sendMessage("#hammergui", "_focus", { pd::Atom(pd->generateSymbol(buf)), hasFocus });
    });
}

void Canvas::focusLost(FocusChangeType cause)
{
    pd->enqueueFunctionAsync([_this = SafePointer(this), this, focused = hasKeyboardFocus(true)]() {
        if (!_this) return;
        auto* glist = patch.getPointer().get();
        if (!glist)
            return;

        // canvas.active listener
        char buf[MAXPDSTRING];
        snprintf(buf, MAXPDSTRING - 1, ".x%lx.c", (unsigned long)glist);
        pd->sendMessage("#active_gui", "_focus", { pd->generateSymbol(buf), static_cast<float>(focused) });

        if (!_this) return;
        // cyclone focus listeners
        pd->sendMessage("#hammergui", "_focus", { pd->generateSymbol(buf), static_cast<float>(focused) });
    });
}

void Canvas::dragAndDropPaste(String const& patchString, Point<int> mousePos, int patchWidth, int patchHeight, String name)
{
    locked = false;
    presentationMode = false;

    // force the valueChanged to run, and wait for them to return
    locked.getValueSource().sendChangeMessage(true);
    presentationMode.getValueSource().sendChangeMessage(true);

    MessageManager::callAsync([_this = SafePointer(this)]() {
        if (_this)
            _this->grabKeyboardFocus();
    });

    auto undoText = String("Add object");
    if (name.isNotEmpty())
        undoText = String("Add " + name.toLowerCase());

    patch.startUndoSequence(undoText);

    auto patchSize = Point<int>(patchWidth, patchHeight);
    String translatedObjects = pd::Patch::translatePatchAsString(patchString, mousePos - (patchSize / 2.0f));

    if (auto patchPtr = patch.getPointer()) {
        pd::Interface::paste(patchPtr.get(), translatedObjects.toRawUTF8());
    }

    deselectAll();

    // Load state from pd
    performSynchronise();

    patch.setCurrent();

    std::vector<t_gobj*> pastedObjects;

    auto* patchPtr = patch.getPointer().get();
    if (!patchPtr)
        return;

    pd->lockAudioThread();
    for (auto* object : objects) {
        auto* objectPtr = static_cast<t_gobj*>(object->getPointer());
        if (objectPtr && glist_isselected(patchPtr, objectPtr)) {
            setSelected(object, true);
            pastedObjects.emplace_back(objectPtr);
        }
    }
    pd->unlockAudioThread();

    patch.deselectAll();
    pastedObjects.clear();
    patch.endUndoSequence(undoText);

    updateSidebarSelection();
}

void Canvas::pasteSelection()
{
    patch.startUndoSequence("Paste object/s");

    // Paste at mousePos, adds padding if pasted the same place
    auto mousePosition = getMouseXYRelative() - canvasOrigin;
    if (mousePosition == pastedPosition) {
        pastedPadding.addXY(10, 10);
    } else {
        pastedPadding.setXY(-10, -10);
    }
    pastedPosition = mousePosition;

    // Tell pd to paste with offset applied to the clipboard string
    patch.paste(Point<int>(pastedPosition.x + pastedPadding.x, pastedPosition.y + pastedPadding.y));

    deselectAll();

    // Load state from pd
    performSynchronise();

    patch.setCurrent();

    std::vector<t_gobj*> pastedObjects;

    auto* patchPtr = patch.getPointer().get();
    if (!patchPtr)
        return;

    pd->lockAudioThread();
    for (auto* object : objects) {
        auto* objectPtr = static_cast<t_gobj*>(object->getPointer());
        if (objectPtr && glist_isselected(patchPtr, objectPtr)) {
            setSelected(object, true);
            pastedObjects.emplace_back(objectPtr);
        }
    }
    pd->unlockAudioThread();

    patch.deselectAll();
    pastedObjects.clear();
    patch.endUndoSequence("Paste object/s");

    updateSidebarSelection();
}

void Canvas::duplicateSelection()
{
    auto selection = getSelectionOfType<Object>();

    patch.startUndoSequence("Duplicate object/s");

    std::vector<t_gobj*> objectsToDuplicate;
    for (auto* object : selection) {
        if (auto* ptr = object->getPointer()) {
            objectsToDuplicate.push_back(ptr);
        }
    }
    
    // If we previously duplicated and dragged before, and then drag again, the new offset should be relative
    // to the offset we already applied with the previous drag
    if(dragState.lastDuplicateOffset != dragState.duplicateOffset)
    {
        dragState.duplicateOffset += dragState.lastDuplicateOffset;
    }

    dragState.lastDuplicateOffset = dragState.duplicateOffset;
    
    // Tell pd to duplicate
    patch.duplicate(objectsToDuplicate);

    deselectAll();

    // Load state from pd immediately
    performSynchronise();

    auto* patchPtr = patch.getPointer().get();
    if (!patchPtr)
        return;
    
    // Store the duplicated objects for later selection
    Array<Object*> duplicated;
    for (auto* object : objects) {
        auto* objectPtr = static_cast<t_gobj*>(object->getPointer());
        if (objectPtr && glist_isselected(patchPtr, objectPtr)) {
            duplicated.add(object);
        }
    }
    
    // Move duplicated objects if they overlap exisisting objects
    std::vector<t_gobj*> moveObjects;
    for (auto* dup : duplicated) {
        moveObjects.emplace_back(dup->getPointer());
    }

    patch.moveObjects(moveObjects, dragState.duplicateOffset.x, dragState.duplicateOffset.y);
    for (auto* object : objects) {
        object->updateBounds();
    }
    
    // Select the newly duplicated objects
    for (auto* obj : duplicated) {
        setSelected(obj, true);
    }
    
    dragState.wasDuplicated = true;
    
    patch.endUndoSequence("Duplicate object/s");
    patch.deselectAll();
}

void Canvas::removeSelection()
{
    patch.startUndoSequence("Remove object/s");
    // Make sure object isn't selected and stop updating gui
    editor->sidebar->hideParameters();
    editor->showTouchSelectionHelper(false);

    // Find selected objects and make them selected in pd
    std::vector<t_gobj*> objects;
    for (auto* object : getSelectionOfType<Object>()) {
        if (auto* ptr = object->getPointer()) {
            objects.push_back(ptr);
        }
    }

    auto wasDeleted = [&objects](t_gobj* ptr) {
        return std::find(objects.begin(), objects.end(), ptr) != objects.end();
    };

    // remove selection
    patch.removeObjects(objects);

    // Remove connection afterwards and make sure they aren't already deleted
    for (auto* con : connections) {
        if (con->isSelected()) {
            auto* outPtr = con->outobj->getPointer();
            auto* inPtr = con->inobj->getPointer();
            auto* checkedOutPtr = pd::Interface::checkObject(outPtr);
            auto* checkedInPtr = pd::Interface::checkObject(inPtr);
            if (checkedOutPtr && checkedInPtr && (!(wasDeleted(outPtr) || wasDeleted(inPtr)))) {
                patch.removeConnection(checkedOutPtr, con->outIdx, checkedInPtr, con->inIdx, con->getPathState());
            }
        }
    }

    patch.finishRemove(); // Makes sure that the extra removed connections will be grouped in the same undo action

    deselectAll();

    // Load state from pd
    synchronise();
    handleUpdateNowIfNeeded();

    patch.endUndoSequence("Remove object/s");

    patch.deselectAll();

    synchroniseSplitCanvas();
}

void Canvas::removeSelectedConnections()
{
    patch.startUndoSequence("Remove connection/s");

    for (auto* con : connections) {
        if (con->isSelected()) {
            auto* checkedOutPtr = pd::Interface::checkObject(con->outobj->getPointer());
            auto* checkedInPtr = pd::Interface::checkObject(con->inobj->getPointer());
            if (!checkedInPtr || !checkedOutPtr)
                continue;

            patch.removeConnection(checkedOutPtr, con->outIdx, checkedInPtr, con->inIdx, con->getPathState());
        }
    }

    patch.endUndoSequence("Remove connection/s");

    // Load state from pd
    synchronise();
    handleUpdateNowIfNeeded();

    synchroniseSplitCanvas();
}

void Canvas::triggerizeSelection()
{
    auto selectedBoxes = getSelectionOfType<Object>();

    std::vector<t_gobj*> objects;
    for (auto* object : getSelectionOfType<Object>()) {
        if (auto* ptr = object->getPointer()) {
            objects.push_back(ptr);
        }
    }

    if (auto patchPtr = patch.getPointer()) {
        pd::Interface::triggerize(patchPtr.get(), objects);
    }

    synchronise();
}

void Canvas::encapsulateSelection()
{
    auto selectedBoxes = getSelectionOfType<Object>();

    // Sort by index in pd patch
    std::sort(selectedBoxes.begin(), selectedBoxes.end(),
        [this](auto* a, auto* b) -> bool {
            return objects.indexOf(a) < objects.indexOf(b);
        });

    // If two connections have the same target inlet/outlet, we only need 1 [inlet/outlet] object
    auto usedEdges = Array<Iolet*>();
    auto targetEdges = std::map<Iolet*, Array<Iolet*>>();

    auto newInternalConnections = String();
    auto newExternalConnections = std::map<int, Array<Iolet*>>();

    // First, find all the incoming and outgoing connections
    for (auto* connection : connections) {
        if (selectedBoxes.contains(connection->inobj.get()) && !selectedBoxes.contains(connection->outobj.get())) {
            auto* inlet = connection->inlet.get();
            targetEdges[inlet].add(connection->outlet.get());
            usedEdges.addIfNotAlreadyThere(inlet);
        }
    }
    for (auto* connection : connections) {
        if (selectedBoxes.contains(connection->outobj.get()) && !selectedBoxes.contains(connection->inobj.get())) {
            auto* outlet = connection->outlet.get();
            targetEdges[outlet].add(connection->inlet.get());
            usedEdges.addIfNotAlreadyThere(outlet);
        }
    }

    auto newEdgeObjects = String();

    // Sort by position
    std::sort(usedEdges.begin(), usedEdges.end(),
        [](auto* a, auto* b) -> bool {
            // Inlets before outlets
            if (a->isInlet != b->isInlet)
                return a->isInlet;

            auto apos = a->getCanvasBounds().getPosition();
            auto bpos = b->getCanvasBounds().getPosition();

            if (apos.x == bpos.x) {
                return apos.y < bpos.y;
            }

            return apos.x < bpos.x;
        });

    int i = 0;
    int numIn = 0;
    for (auto* iolet : usedEdges) {
        auto type = String(iolet->isInlet ? "inlet" : "outlet") + String(iolet->isSignal ? "~" : "");
        auto* targetEdge = targetEdges[iolet][0];
        auto pos = targetEdge->object->getObjectBounds().getPosition();
        newEdgeObjects += "#X obj " + String(pos.x) + " " + String(pos.y) + " " + type + ";\n";

        int objIdx = selectedBoxes.indexOf(iolet->object);
        int ioletObjectIdx = selectedBoxes.size() + i;
        if (iolet->isInlet) {
            newInternalConnections += "#X connect " + String(ioletObjectIdx) + " 0 " + String(objIdx) + " " + String(iolet->ioletIdx) + ";\n";
            numIn++;
        } else {
            newInternalConnections += "#X connect " + String(objIdx) + " " + String(iolet->ioletIdx) + " " + String(ioletObjectIdx) + " 0;\n";
        }

        for (auto* target : targetEdges[iolet]) {
            newExternalConnections[i].add(target);
        }

        i++;
    }

    patch.deselectAll();

    auto bounds = Rectangle<int>();
    std::vector<t_gobj*> objects;
    for (auto* object : selectedBoxes) {
        if (auto* ptr = object->getPointer()) {
            bounds = bounds.getUnion(object->getBounds());
            objects.push_back(ptr);
        }
    }
    auto centre = bounds.getCentre() - canvasOrigin;

    auto copypasta = String("#N canvas 733 172 450 300 0 1;\n") + "$$_COPY_HERE_$$" + newEdgeObjects + newInternalConnections + "#X restore " + String(centre.x) + " " + String(centre.y) + " pd;\n";

    auto* patchPtr = patch.getPointer().get();
    if (!patchPtr)
        return;

    // Apply the changed on Pd's thread
    pd->lockAudioThread();

    int size;
    char const* text = pd::Interface::copy(patchPtr, &size, objects);
    auto copied = String::fromUTF8(text, size);

    // Wrap it in an undo sequence, to allow undoing everything in 1 step
    patch.startUndoSequence("Encapsulate");

    pd::Interface::removeObjects(patchPtr, objects);

    auto replacement = copypasta.replace("$$_COPY_HERE_$$", copied);

    pd::Interface::paste(patchPtr, replacement.toRawUTF8());
    auto lastObject = patch.getObjects().back();
    if (!lastObject.isValid())
        return;

    auto* newObject = pd::Interface::checkObject(lastObject.getRaw<t_pd>());
    if (!newObject) {
        patch.endUndoSequence("Encapsulate");
        pd->unlockAudioThread();
        return;
    }

    for (auto& [idx, iolets] : newExternalConnections) {
        for (auto* iolet : iolets) {
            if (auto* externalObject = reinterpret_cast<t_object*>(iolet->object->getPointer())) {
                if (iolet->isInlet) {
                    pd::Interface::createConnection(patchPtr, newObject, idx - numIn, externalObject, iolet->ioletIdx);
                } else {
                    pd::Interface::createConnection(patchPtr, externalObject, iolet->ioletIdx, newObject, idx);
                }
            }
        }
    }

    patch.endUndoSequence("Encapsulate");

    pd->unlockAudioThread();

    synchronise();
    handleUpdateNowIfNeeded();

    patch.deselectAll();
}

bool Canvas::canConnectSelectedObjects()
{
    auto selection = getSelectionOfType<Object>();
    bool rightSize = selection.size() == 2;

    if (!rightSize)
        return false;

    Object* topObject = selection[0]->getY() > selection[1]->getY() ? selection[1] : selection[0];
    Object* bottomObject = selection[0] == topObject ? selection[1] : selection[0];

    bool hasInlet = bottomObject->numInputs > 0;
    bool hasOutlet = topObject->numOutputs > 0;

    return hasInlet && hasOutlet;
}

bool Canvas::connectSelectedObjects()
{
    auto selection = getSelectionOfType<Object>();
    bool rightSize = selection.size() == 2;

    if (!rightSize)
        return false;

    auto* topObject = selection[0]->getY() > selection[1]->getY() ? selection[1]->getPointer() : selection[0]->getPointer();
    auto* bottomObject = selection[0]->getPointer() == topObject ? selection[1]->getPointer() : selection[0]->getPointer();

    auto* checkedTopObject = pd::Interface::checkObject(topObject);
    auto* checkedBottomObject = pd::Interface::checkObject(bottomObject);

    if (checkedTopObject && checkedBottomObject) {
        patch.createConnection(checkedTopObject, 0, checkedBottomObject, 0);
    }

    synchronise();

    return true;
}

void Canvas::cancelConnectionCreation()
{
    connectionsBeingCreated.clear();
    if (connectingWithDrag) {
        connectingWithDrag = false;
        connectionCancelled = true;
        if (nearestIolet) {
            nearestIolet->isTargeted = false;
            nearestIolet->repaint();
            nearestIolet = nullptr;
        }
    }
}

void Canvas::alignObjects(Align alignment)
{
    auto objects = getSelectionOfType<Object>();

    if (objects.size() < 2)
        return;

    auto sortByXPos = [](Array<Object*>& objects) {
        std::sort(objects.begin(), objects.end(), [](const auto& a, const auto& b) {
            auto aX = a->getBounds().getX();
            auto bX = b->getBounds().getX();
            if (aX == bX) {
                return a->getBounds().getY() < b->getBounds().getY();
            }
            return aX < bX;
        });
    };

    auto sortByYPos = [](Array<Object*>& objects) {
        std::sort(objects.begin(), objects.end(), [](const auto& a, const auto& b) {
            auto aY = a->getBounds().getY();
            auto bY = b->getBounds().getY();
            if (aY == bY)
                return a->getBounds().getX() < b->getBounds().getX();

            return aY < bY;
        });
    };

    auto getBoundingBox = [](Array<Object*>& objects) -> Rectangle<int> {
        auto totalBounds = Rectangle<int>();
        for (auto* object : objects) {
            if (object->getPointer()) {
                totalBounds = totalBounds.getUnion(object->getBounds());
            }
        }
        return totalBounds;
    };

    patch.startUndoSequence("Align objects");

    // mark canvas as dirty, and set undo for all positions
    if(auto patchPtr = patch.getPointer()) {
        canvas_dirty(patchPtr.get(), 1);
        for (auto object : objects) {
            if (auto* ptr = object->getPointer())
                pd::Interface::undoApply(patchPtr.get(), ptr);
        }
    }

    // get the bounding box of all selected objects
    auto selectedBounds = getBoundingBox(objects);

    auto getSpacerX = [selectedBounds](Array<Object*>& objects) -> float {
        auto totalWidths = 0;
        for (auto* object : objects) {
            totalWidths += object->getWidth() - (Object::margin * 2);
        }
        auto selectedBoundsNoMargin = selectedBounds.getWidth() - (Object::margin * 2);
        auto spacer = (selectedBoundsNoMargin - totalWidths) / static_cast<float>(objects.size() - 1);
        return spacer;
    };

    auto getSpacerY = [selectedBounds](Array<Object*>& objects) -> float {
        auto totalWidths = 0;
        for (int i = 0; i < objects.size(); i++) {
            totalWidths += objects[i]->getHeight() - (Object::margin * 2);
        }
        auto selectedBoundsNoMargin = selectedBounds.getHeight() - (Object::margin * 2);
        auto spacer = (selectedBoundsNoMargin - totalWidths) / static_cast<float>(objects.size() - 1);
        return spacer;
    };

    switch (alignment) {
    case Align::Left: {
        auto leftPos = selectedBounds.getTopLeft().getX();
        for (auto* object : objects) {
            patch.moveObjectTo(object->getPointer(), leftPos, object->getBounds().getY());
        }
        break;
    }
    case Align::Right: {
        auto rightPos = selectedBounds.getRight();
        for (auto* object : objects) {
            auto objectBounds = object->getBounds();
            patch.moveObjectTo(object->getPointer(), rightPos - objectBounds.getWidth(), objectBounds.getY());
        }
        break;
    }
    case Align::VCentre: {
        auto centrePos = selectedBounds.getCentreX();
        for (auto* object : objects) {
            auto objectBounds = object->getBounds();
            patch.moveObjectTo(object->getPointer(), centrePos - objectBounds.withZeroOrigin().getCentreX(), objectBounds.getY());
        }
        break;
    }
    case Align::Top: {
        auto topPos = selectedBounds.getTopLeft().y;
        for (auto* object : objects) {
            patch.moveObjectTo(object->getPointer(), object->getX(), topPos);
        }
        break;
    }
    case Align::Bottom: {
        auto bottomPos = selectedBounds.getBottom();
        for (auto* object : objects) {
            auto objectBounds = object->getBounds();
            patch.moveObjectTo(object->getPointer(), objectBounds.getX(), bottomPos - objectBounds.getHeight());
        }
        break;
    }
    case Align::HCentre: {
        auto centerPos = selectedBounds.getCentreY();
        for (auto* object : objects) {
            auto objectBounds = object->getBounds();
            patch.moveObjectTo(object->getPointer(), objectBounds.getX(), centerPos - objectBounds.withZeroOrigin().getCentreY());
        }
        break;
    }
    case Align::HDistribute: {
        sortByXPos(objects);
        float spacer = getSpacerX(objects);
        float offset = objects[0]->getBounds().getX();
        for (int i = 1; i < objects.size() - 1; i++) {
            auto leftObjWidth = objects[i - 1]->getBounds().getWidth() - (Object::margin * 2);
            offset += leftObjWidth + spacer;
            patch.moveObjectTo(objects[i]->getPointer(), offset, objects[i]->getBounds().getY());
        }
        break;
    }
    case Align::VDistribute: {
        sortByYPos(objects);
        float spacer = getSpacerY(objects);
        float offset = objects[0]->getBounds().getY();
        for (int i = 1; i < objects.size() - 1; i++) {
            auto topObjHeight = objects[i - 1]->getBounds().getHeight() - (Object::margin * 2);
            offset += topObjHeight + spacer;
            patch.moveObjectTo(objects[i]->getPointer(), objects[i]->getBounds().getX(), offset);
        }
        break;
    }
    default:
        break;
    }

    performSynchronise();

    for (auto* connection : connections) {
        connection->forceUpdate();
    }

    patch.endUndoSequence("Align objects");
}

void Canvas::undo()
{
    // Tell pd to undo the last action
    patch.undo();

    // Load state from pd
    synchronise();
    handleUpdateNowIfNeeded();

    patch.deselectAll();

    synchroniseSplitCanvas();
    updateSidebarSelection();
}

void Canvas::redo()
{
    // Tell pd to undo the last action
    patch.redo();

    // Load state from pd
    synchronise();
    handleUpdateNowIfNeeded();

    patch.deselectAll();

    synchroniseSplitCanvas();
    updateSidebarSelection();
}

void Canvas::valueChanged(Value& v)
{
    // Update zoom
    if (v.refersToSameSourceAs(zoomScale)) {
        editor->statusbar->updateZoomLevel();
        patch.lastViewportScale = getValue<float>(zoomScale);
        hideSuggestions();
    } else if (v.refersToSameSourceAs(patchWidth)) {
        // limit canvas width to smallest object (11px)
        patchWidth = jmax(11, getValue<int>(patchWidth));
        if (auto cnv = patch.getPointer()) {
            auto x1 = static_cast<float>(cnv->gl_screenx1);
            auto y1 = static_cast<float>(cnv->gl_screeny1);
            auto x2 = static_cast<float>(getValue<int>(patchWidth) + x1);
            auto y2 = static_cast<float>(cnv->gl_screeny2);

            char buf[MAXPDSTRING];
            snprintf(buf, MAXPDSTRING - 1, ".x%lx", (unsigned long)cnv.get());
            pd->sendMessage(buf, "setbounds", { x1, y1, x2, y2 });
        }

        patch.getPointer()->gl_screenx2 = getValue<int>(patchWidth) + patch.getPointer()->gl_screenx1;
        repaint();
    } else if (v.refersToSameSourceAs(patchHeight)) {
        patchHeight = jmax(11, getValue<int>(patchHeight));
        if (auto cnv = patch.getPointer()) {
            auto x1 = static_cast<float>(cnv->gl_screenx1);
            auto y1 = static_cast<float>(cnv->gl_screeny1);
            auto x2 = static_cast<float>(cnv->gl_screenx2);
            auto y2 = static_cast<float>(getValue<int>(patchHeight) + y1);

            char buf[MAXPDSTRING];
            snprintf(buf, MAXPDSTRING - 1, ".x%lx", (unsigned long)cnv.get());
            pd->sendMessage(buf, "setbounds", { x1, y1, x2, y2 });
        }
        repaint();
    }
    // When lock changes
    else if (v.refersToSameSourceAs(locked)) {
        bool editMode = !getValue<bool>(v);

        if (auto ptr = patch.getPointer()) {
            char buf[MAXPDSTRING];
            snprintf(buf, MAXPDSTRING - 1, ".x%lx", (unsigned long)ptr.get());
            pd->sendMessage(buf, "editmode", { static_cast<float>(editMode) });
        }

        cancelConnectionCreation();
        deselectAll();

        // Makes sure no objects keep keyboard focus after locking/unlocking
        if (isShowing() && isVisible())
            grabKeyboardFocus();

        editor->updateCommandStatus();
        updateOverlays();
        orderConnections();
    } else if (v.refersToSameSourceAs(commandLocked)) {
        updateOverlays();
        repaint();
    }
    // Should only get called when the canvas isn't a real graph
    else if (v.refersToSameSourceAs(presentationMode)) {
        connectionLayer.setVisible(!getValue<bool>(presentationMode));
        deselectAll();
    } else if (v.refersToSameSourceAs(hideNameAndArgs)) {
        if (!patch.getPointer())
            return;

        int hideText = getValue<bool>(hideNameAndArgs);
        if (auto glist = patch.getPointer()) {
            hideText = glist->gl_isgraph && hideText;
            canvas_setgraph(glist.get(), glist->gl_isgraph + 2 * hideText, 0);
        }

        hideNameAndArgs = hideText;
    } else if (v.refersToSameSourceAs(isGraphChild)) {

        if (!patch.getPointer())
            return;

        int graphChild = getValue<bool>(isGraphChild);

        if (auto glist = patch.getPointer()) {
            canvas_setgraph(glist.get(), graphChild + 2 * (graphChild && glist->gl_hidetext), 0);
        }

        if (!graphChild) {
            hideNameAndArgs = false;
        }

        if (graphChild && !isGraph) {
            graphArea = std::make_unique<GraphArea>(this);
            addAndMakeVisible(*graphArea);
            graphArea->setAlwaysOnTop(true);
            graphArea->updateBounds();
        } else {
            graphArea.reset(nullptr);
        }

        updateOverlays();
        repaint();
    } else if (v.refersToSameSourceAs(xRange)) {
        if (auto glist = patch.getPointer()) {
            glist->gl_x1 = static_cast<float>(xRange.getValue().getArray()->getReference(0));
            glist->gl_x2 = static_cast<float>(xRange.getValue().getArray()->getReference(1));
        }
        updateDrawables();
    } else if (v.refersToSameSourceAs(yRange)) {
        if (auto glist = patch.getPointer()) {
            glist->gl_y2 = static_cast<float>(yRange.getValue().getArray()->getReference(0));
            glist->gl_y1 = static_cast<float>(yRange.getValue().getArray()->getReference(1));
        }
        updateDrawables();
    }
}

void Canvas::orderConnections()
{
    // move connection layer to back when canvas is locked & connections behind is active
    if (connectionsBehind) {
        connectionLayer.toBack();
    } else
        objectLayer.toBack();

    repaint();
}

void Canvas::showSuggestions(Object* object, TextEditor* textEditor)
{
    suggestor->createCalloutBox(object, textEditor);
}
void Canvas::hideSuggestions()
{
    suggestor->removeCalloutBox();
}

// Makes component selected
void Canvas::setSelected(Component* component, bool shouldNowBeSelected, bool updateCommandStatus)
{
    if (!shouldNowBeSelected) {
        selectedComponents.deselect(component);
    } else {
        selectedComponents.addToSelection(component);
    }

    if (updateCommandStatus) {
        editor->updateCommandStatus();
    }
}

SelectedItemSet<WeakReference<Component>>& Canvas::getLassoSelection()
{
    return selectedComponents;
}

bool Canvas::checkPanDragMode()
{
    auto panDragEnabled = panningModifierDown();
    setPanDragMode(panDragEnabled);

    return panDragEnabled;
}

bool Canvas::setPanDragMode(bool shouldPan)
{
    if (auto* v = dynamic_cast<CanvasViewport*>(viewport.get())) {
        v->enableMousePanning(shouldPan);
        return true;
    }
    return false;
}

bool Canvas::isPointOutsidePluginArea(Point<int> point)
{
    auto const borderWidth = getValue<float>(patchWidth);
    auto const borderHeight = getValue<float>(patchHeight);
    auto const halfSize = infiniteCanvasSize / 2;
    auto const pos = Point<int>(halfSize, halfSize);

    auto pluginBounds = Rectangle<int>(pos.x, pos.y, borderWidth, borderHeight);

    return !pluginBounds.contains(point);
}

void Canvas::findLassoItemsInArea(Array<WeakReference<Component>>& itemsFound, Rectangle<int> const& area)
{
    auto const lassoArea = area.withSize(jmax(area.getWidth(), 1), jmax(area.getHeight(), 1));

    for (auto* object : objects) {
        if (lassoArea.intersects(object->getSelectableBounds())) {
            itemsFound.add(object);
        } else if (!ModifierKeys::getCurrentModifiers().isAnyModifierKeyDown()) {
            setSelected(object, false, false);
        }
    }

    for (auto& connection : connections) {
        // If total bounds don't intersect, there can't be an intersection with the line
        // This is cheaper than checking the path intersection, so do this first
        if (!connection->getBounds().intersects(lassoArea)) {
            setSelected(connection, false, false);
            continue;
        }

        // Check if path intersects with lasso
        if (connection->intersects(lassoArea.toFloat())) {
            itemsFound.add(connection);
        } else if (!ModifierKeys::getCurrentModifiers().isAnyModifierKeyDown()) {
            setSelected(connection, false, false);
        }
    }
}

ObjectParameters& Canvas::getInspectorParameters()
{
    return parameters;
}

bool Canvas::panningModifierDown()
{
#if JUCE_IOS
    return OSUtils::ScrollTracker::isScrolling();
#endif
    auto& commandManager = editor->commandManager;
    // check the command manager for the keycode that is assigned to pan drag key
    auto panDragKeycode = commandManager.getKeyMappings()->getKeyPressesAssignedToCommand(CommandIDs::PanDragKey).getFirst().getKeyCode();

    // get the current modifier keys, removing the left mouse button modifier (as that is what is needed to activate a pan drag with key down)
    auto currentMods = ModifierKeys(ModifierKeys::getCurrentModifiers().getRawFlags() & ~ModifierKeys::leftButtonModifier);

    bool isPanDragKeysActive = false;

    if (KeyPress::isKeyCurrentlyDown(panDragKeycode)) {
        // construct a fake keypress with the current pan drag keycode key, with current modifiers, to test if it matches the command id's code & mods
        auto keyWithMod = KeyPress(panDragKeycode, currentMods, 0);
        isPanDragKeysActive = commandManager.getKeyMappings()->containsMapping(CommandIDs::PanDragKey, keyWithMod);
    }

    return isPanDragKeysActive || ModifierKeys::getCurrentModifiers().isMiddleButtonDown();
}

void Canvas::receiveMessage(t_symbol* symbol, pd::Atom const atoms[8], int numAtoms)
{
    switch (hash(symbol->s_name)) {
    case hash("obj"):
    case hash("msg"):
    case hash("floatatom"):
    case hash("listbox"):
    case hash("symbolatom"):
    case hash("text"):
    case hash("graph"):
    case hash("scalar"):
    case hash("bng"):
    case hash("toggle"):
    case hash("vslider"):
    case hash("hslider"):
    case hash("hdial"):
    case hash("vdial"):
    case hash("hradio"):
    case hash("vradio"):
    case hash("vumeter"):
    case hash("mycnv"):
    case hash("numbox"):
    case hash("connect"):
    case hash("clear"):
    case hash("cut"):
    case hash("disconnect"): {
        // This will trigger an asyncupdater, so it's thread-safe to do this here
        synchronise();
        break;
    }
    case hash("editmode"): {
        if (numAtoms >= 1) {
            int flag = atoms[0].getFloat();
            if (flag % 2 == 0) {
                locked = true;
            } else {
                locked = false;
                presentationMode = false;
            }
        }
        break;
    }
    case hash("setbounds"): {
        if (numAtoms >= 4) {
            auto width = atoms[2].getFloat() - atoms[0].getFloat();
            auto height = atoms[3].getFloat() - atoms[1].getFloat();
            setValueExcludingListener(patchWidth, width, this);
            setValueExcludingListener(patchHeight, height, this);
            repaint();
        }

        break;
    }
    case hash("coords"):
    case hash("donecanvasdialog"): {
        if (auto* cnv = editor->getCurrentCanvas()) {
            cnv->synchronise();
            cnv->synchroniseSplitCanvas();
        }
        break;
    }
    }
}

void Canvas::resized()
{
    connectionLayer.setBounds(getLocalBounds());
    objectLayer.setBounds(getLocalBounds());
}
