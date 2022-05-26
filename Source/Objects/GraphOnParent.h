
struct GraphOnParent final : public GUIObject
{
    bool isLocked = false;

   public:
    // Graph On Parent
    GraphOnParent(void* obj, Box* box) : GUIObject(obj, box), subpatch({ptr, cnv->pd})
    {
        setInterceptsMouseClicks(box->locked == var(false), true);

        updateCanvas();

        initialise();

        resized();
        updateDrawables();
    }

    void checkBounds() override
    {
        // Apply size limits
        int w = jlimit(25, maxSize, box->getWidth());
        int h = jlimit(25, maxSize, box->getHeight());

        if (w != box->getWidth() || h != box->getHeight())
        {
            box->setSize(w, h);
        }
    }

    void updateBounds() override
    {
        int x = 0, y = 0, w = 0, h = 0;
        libpd_get_object_bounds(cnv->patch.getPointer(), ptr, &x, &y, &w, &h);

        auto* glist = static_cast<_glist*>(ptr);
        box->setObjectBounds({x, y, glist->gl_pixwidth, glist->gl_pixheight});
    }

    ~GraphOnParent() override
    {
        closeOpenedSubpatchers();
    }

    void applyBounds() override
    {
        libpd_moveobj(cnv->patch.getPointer(), static_cast<t_gobj*>(ptr), box->getX() + Box::margin, box->getY() + Box::margin);

        auto* graph = static_cast<_glist*>(ptr);
        graph->gl_pixwidth = getWidth();
        graph->gl_pixheight = getHeight();
    }

    void lock(bool locked) override
    {
        isLocked = locked;
        setInterceptsMouseClicks(isLocked, isLocked);
    }

    void mouseDown(const MouseEvent& e) override
    {
        GUIObject::mouseDown(e);
        
        if (!isLocked)
        {
            box->mouseDown(e.getEventRelativeTo(box));
        }
    }

    void mouseDrag(const MouseEvent& e) override
    {
        if (!isLocked)
        {
            box->mouseDrag(e.getEventRelativeTo(box));
        }
    }

    void mouseUp(const MouseEvent& e) override
    {
        if (!isLocked)
        {
            box->mouseUp(e.getEventRelativeTo(box));
        }
    }

    void updateCanvas()
    {
        if (!canvas)
        {
            canvas = std::make_unique<Canvas>(cnv->main, subpatch, this);

            // Make sure that the graph doesn't become the current canvas
            cnv->patch.setCurrent(true);
            cnv->main.updateCommandStatus();
        }

        auto b = getPatch()->getBounds();
        // canvas->checkBounds();
        canvas->setBounds(-b.getX(), -b.getY(), b.getWidth() + b.getX(), b.getHeight() + b.getY());
        canvas->setLookAndFeel(&LookAndFeel::getDefaultLookAndFeel());
    }

    void updateValue() override
    {
        updateCanvas();

        if (!canvas) return;

        for (auto& box : canvas->boxes)
        {
            if (box->gui)
            {
                box->gui->updateValue();
            }
        }
    }

    void updateDrawables() override
    {
        if (!canvas) return;
        for (auto& box : canvas->boxes)
        {
            if (box->gui)
            {
                box->gui->updateDrawables();
            }
        }
    }

    // override to make transparent
    void paint(Graphics& g) override
    {
        auto outlineColour = box->findColour(cnv->isSelected(box) && !cnv->isGraph ? PlugDataColour::highlightColourId : PlugDataColour::canvasOutlineColourId);
        
        g.setColour(outlineColour);
        g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(0.5f), 2.0f, 1.0f);
    }

    pd::Patch* getPatch() override
    {
        return &subpatch;
    }

    Canvas* getCanvas() override
    {
        return canvas.get();
    }

   private:
    pd::Patch subpatch;
    std::unique_ptr<Canvas> canvas;
};
