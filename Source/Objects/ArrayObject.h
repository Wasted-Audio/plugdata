

extern "C"
{
#include "x_libpd_extra_utils.h"

    void garray_arraydialog(t_garray* x, t_symbol* name, t_floatarg fsize, t_floatarg fflags, t_floatarg deleteit);
}

class PdArray
{
   public:
    enum DrawType
    {
        Points,
        Line,
        Curve
    };

    PdArray(String arrayName, void* arrayInstance) : name(std::move(arrayName)), instance(arrayInstance)
    {
    }

    // The default constructor.
    PdArray() = default;

    // Gets the name of the array.
    String getName() const noexcept
    {
        return name;
    }

    PdArray::DrawType getDrawType() const noexcept
    {
        libpd_set_instance(static_cast<t_pdinstance*>(instance));
        return static_cast<DrawType>(libpd_array_get_style(name.toRawUTF8()));
    }

    // Gets the scale of the array.
    std::array<float, 2> getScale() const noexcept
    {
        float min = -1, max = 1;
        libpd_set_instance(static_cast<t_pdinstance*>(instance));
        libpd_array_get_scale(name.toRawUTF8(), &min, &max);
        return {min, max};
    }

    void setScale(std::array<float, 2> scale) noexcept
    {
        auto& [min, max] = scale;
        libpd_set_instance(static_cast<t_pdinstance*>(instance));

        libpd_array_set_scale(name.toRawUTF8(), min, max);
    }

    // Gets the values of the array.
    void read(std::vector<float>& output) const
    {
        libpd_set_instance(static_cast<t_pdinstance*>(instance));
        int const size = libpd_arraysize(name.toRawUTF8());
        output.resize(static_cast<size_t>(size));
        libpd_read_array(output.data(), name.toRawUTF8(), 0, size);
    }

    // Writes the values of the array.
    void write(std::vector<float> const& input)
    {
        libpd_set_instance(static_cast<t_pdinstance*>(instance));
        libpd_write_array(name.toRawUTF8(), 0, input.data(), static_cast<int>(input.size()));
    }

    // Writes a value of the array.
    void write(const size_t pos, float const input)
    {
        libpd_set_instance(static_cast<t_pdinstance*>(instance));
        libpd_write_array(name.toRawUTF8(), static_cast<int>(pos), &input, 1);
    }

   private:
    String name = "";
    void* instance = nullptr;

    friend class Instance;
};

struct GraphicalArray : public Component
{
   public:
    Box* box;

    GraphicalArray(PlugDataAudioProcessor* instance, PdArray& graph, Box* parent) : array(graph), edited(false), pd(instance), box(parent)
    {
        if (graph.getName().isEmpty()) return;

        vec.reserve(8192);
        temp.reserve(8192);
        try
        {
            array.read(vec);
        }
        catch (...)
        {
            error = true;
        }

        setInterceptsMouseClicks(true, false);
        setOpaque(false);
    }

    void setArray(PdArray& graph)
    {
        if (graph.getName().isEmpty()) return;
        array = graph;
    }

    void paint(Graphics& g) override
    {
        g.setColour(box->findColour(PlugDataColour::toolbarColourId));
        g.fillRoundedRectangle(getLocalBounds().toFloat().reduced(0.5f), 2.0f);

        if (error)
        {
            g.setColour(box->findColour(PlugDataColour::textColourId));
            g.drawText("array " + array.getName() + " is invalid", 0, 0, getWidth(), getHeight(), Justification::centred);
            error = false;
        }
        else
        {
            const auto h = static_cast<float>(getHeight());
            const auto w = static_cast<float>(getWidth());
            if (!vec.empty())
            {
                const std::array<float, 2> scale = array.getScale();
                if (scale[0] >= scale[1]) return;

                switch (array.getDrawType())
                {
                    case PdArray::DrawType::Curve:
                    {
                        const float dh = h / (scale[1] - scale[0]);
                        const float dw = w / static_cast<float>(vec.size() - 1);
                        Path p;
                        p.startNewSubPath(0, h - (std::clamp(vec[0], scale[0], scale[1]) - scale[0]) * dh);
                        for (size_t i = 1; i < vec.size() - 1; i += 2)
                        {
                            const float y1 = h - (std::clamp(vec[i - 1], scale[0], scale[1]) - scale[0]) * dh;
                            const float y2 = h - (std::clamp(vec[i], scale[0], scale[1]) - scale[0]) * dh;
                            const float y3 = h - (std::clamp(vec[i + 1], scale[0], scale[1]) - scale[0]) * dh;
                            p.cubicTo(static_cast<float>(i - 1) * dw, y1, static_cast<float>(i) * dw, y2, static_cast<float>(i + 1) * dw, y3);
                        }
                        g.setColour(box->findColour(PlugDataColour::canvasOutlineColourId));
                        g.strokePath(p, PathStrokeType(1));
                        break;
                    }
                    case PdArray::DrawType::Line:
                    {
                        const float dh = h / (scale[1] - scale[0]);
                        const float dw = w / static_cast<float>(vec.size() - 1);
                        Path p;
                        p.startNewSubPath(0, h - (std::clamp(vec[0], scale[0], scale[1]) - scale[0]) * dh);
                        for (size_t i = 1; i < vec.size(); ++i)
                        {
                            const float y = h - (std::clamp(vec[i], scale[0], scale[1]) - scale[0]) * dh;
                            p.lineTo(static_cast<float>(i) * dw, y);
                        }
                        g.setColour(box->findColour(PlugDataColour::canvasOutlineColourId));
                        g.strokePath(p, PathStrokeType(1));
                        break;
                    }
                    case PdArray::DrawType::Points:
                    {
                        const float dh = h / (scale[1] - scale[0]);
                        const float dw = w / static_cast<float>(vec.size());
                        g.setColour(box->findColour(PlugDataColour::canvasOutlineColourId));
                        for (size_t i = 0; i < vec.size(); ++i)
                        {
                            const float y = h - (std::clamp(vec[i], scale[0], scale[1]) - scale[0]) * dh;
                            g.drawLine(static_cast<float>(i) * dw, y, static_cast<float>(i + 1) * dw, y);
                        }
                        break;
                    }
                    default:
                        break;
                }
            }
        }
    }

    void mouseDown(const MouseEvent& e) override
    {
        if (error) return;
        edited = true;

        const auto s = static_cast<float>(vec.size() - 1);
        const auto w = static_cast<float>(getWidth());
        const auto x = static_cast<float>(e.x);

        lastIndex = static_cast<size_t>(std::round(std::clamp(x / w, 0.f, 1.f) * s));

        mouseDrag(e);
    }

    void mouseDrag(const MouseEvent& e) override
    {
        if (error) return;

        const auto s = static_cast<float>(vec.size() - 1);
        const auto w = static_cast<float>(getWidth());
        const auto h = static_cast<float>(getHeight());
        const auto x = static_cast<float>(e.x);
        const auto y = static_cast<float>(e.y);

        const std::array<float, 2> scale = array.getScale();
        const int index = static_cast<int>(std::round(std::clamp(x / w, 0.f, 1.f) * s));

        float start = vec[lastIndex];
        float current = (1.f - std::clamp(y / h, 0.f, 1.f)) * (scale[1] - scale[0]) + scale[0];

        int interpStart = std::min(index, lastIndex);
        int interpEnd = std::max(index, lastIndex);

        float min = index == interpStart ? current : start;
        float max = index == interpStart ? start : current;

        // const CriticalSection* cs = pd->getCallbackLock();

        // Fix to make sure we don't leave any gaps while dragging
        for (int n = interpStart; n <= interpEnd; n++)
        {
            vec[n] = jmap<float>(n, interpStart, interpEnd + 1, min, max);
        }

        // Don't want to touch vec on the other thread, so we copy the vector into the lambda
        auto changed = std::vector<float>(vec.begin() + interpStart, vec.begin() + interpEnd + 1);

        pd->enqueueFunction(
            [this, interpStart, changed]() mutable
            {
                try
                {
                    for (int n = 0; n < changed.size(); n++)
                    {
                        array.write(interpStart + n, changed[n]);
                    }
                }
                catch (...)
                {
                    error = true;
                }
            });

        lastIndex = index;

        pd->enqueueMessages(stringArray.toStdString(), array.getName().toStdString(), {});
        repaint();
    }

    void mouseUp(const MouseEvent& e) override
    {
        if (error) return;
        edited = false;
    }

    void update()
    {
        if (!edited)
        {
            error = false;
            try
            {
                array.read(temp);
            }
            catch (...)
            {
                error = true;
            }
            if (temp != vec)
            {
                vec.swap(temp);
                repaint();
            }
        }
    }

    size_t getArraySize() const noexcept
    {
        return vec.size();
    }

    PdArray array;
    std::vector<float> vec;
    std::vector<float> temp;
    std::atomic<bool> edited;
    bool error = false;
    const String stringArray = "array";

    int lastIndex = 0;

    PlugDataAudioProcessor* pd;
};

struct ArrayObject final : public GUIObject
{
   public:
    // Array component
    ArrayObject(void* obj, Box* box) : GUIObject(obj, box), array(getArray()), graph(cnv->pd, array, box)
    {
        setInterceptsMouseClicks(false, true);
        graph.setBounds(getLocalBounds());
        addAndMakeVisible(&graph);

        auto scale = array.getScale();
        Array<var> arr = {var(scale[0]), var(scale[1])};
        range = var(arr);
        size = var(static_cast<int>(graph.getArraySize()));

        name = String(array.getName());
        drawMode = static_cast<int>(array.getDrawType()) + 1;

        labelColour = box->findColour(PlugDataColour::textColourId).toString();

        updateLabel();

        initialise();
    }

    void updateLabel() override
    {
        int fontHeight = 14.0f;

        const String text = array.getName();

        if (text.isNotEmpty())
        {
            if (!label)
            {
                label = std::make_unique<Label>();
            }

            auto bounds = box->getBounds().reduced(Box::margin).removeFromTop(fontHeight + 2);

            bounds.translate(2, -(fontHeight + 2));

            label->setFont(Font(fontHeight));
            label->setJustificationType(Justification::centredLeft);
            label->setBounds(bounds);
            label->setBorderSize(BorderSize<int>(0, 0, 0, 0));
            label->setMinimumHorizontalScale(1.f);
            label->setText(text, dontSendNotification);
            label->setEditable(false, false);
            label->setInterceptsMouseClicks(false, false);

            label->setColour(Label::textColourId, box->findColour(PlugDataColour::textColourId));

            box->cnv->addAndMakeVisible(label.get());
        }
    }

    void updateBounds() override
    {
        int x = 0, y = 0, w = 0, h = 0;
        libpd_get_object_bounds(cnv->patch.getPointer(), ptr, &x, &y, &w, &h);

        auto* glist = static_cast<_glist*>(ptr);
        box->setObjectBounds({x, y, glist->gl_pixwidth, glist->gl_pixheight});
    }

    void checkBounds() override
    {
        // Apply size limits
        int w = jlimit(100, maxSize, box->getWidth());
        int h = jlimit(40, maxSize, box->getHeight());

        if (w != box->getWidth() || h != box->getHeight())
        {
            box->setSize(w, h);
        }
    }

    ObjectParameters defineParameters() override
    {
        return {
            {"Name", tString, cGeneral, &name, {}},
            {"Size", tInt, cGeneral, &size, {}},
            {"Draw Mode", tCombo, cGeneral, &drawMode, {"Points", "Polygon", "Bezier Curve"}},
            {"Y Range", tRange, cGeneral, &range, {}},
            {"Save Contents", tBool, cGeneral, &saveContents, {"No", "Yes"}},
        };
    }

    void applyBounds() override
    {
        auto b = box->getObjectBounds();
        libpd_moveobj(cnv->patch.getPointer(), static_cast<t_gobj*>(ptr), b.getX(), b.getY());

        auto* array = static_cast<_glist*>(ptr);
        array->gl_pixwidth = b.getWidth();
        array->gl_pixheight = b.getHeight();
    }

    void resized() override
    {
        graph.setBounds(getLocalBounds());
    }

    void updateSettings()
    {
        auto arrName = name.getValue().toString();
        auto arrSize = static_cast<int>(size.getValue());
        auto arrDrawMode = static_cast<int>(drawMode.getValue()) - 1;
        
        // This flag is swapped for some reason
        if(arrDrawMode == 0){
            arrDrawMode = 1;
        }
        else if(arrDrawMode == 1){
            arrDrawMode = 0;
        }
        
        auto arrSaveContents = static_cast<bool>(saveContents.getValue());

        int flags = arrSaveContents + 2 * static_cast<int>(arrDrawMode);

        cnv->pd->enqueueFunction(
            [this, arrName, arrSize, flags]() mutable
            {
                auto* garray = static_cast<t_garray*>(libpd_array_get_byname(array.getName().toRawUTF8()));
                garray_arraydialog(garray, gensym(arrName.toRawUTF8()), arrSize, static_cast<float>(flags), 0.0f);

                MessageManager::callAsync(
                    [this]()
                    {
                        array = getArray();
                        graph.setArray(array);
                        updateLabel();
                    });
            });

        repaint();
    }

    void updateValue() override
    {
        graph.update();
    }

    void valueChanged(Value& value) override
    {
        if (value.refersToSameSourceAs(name) || value.refersToSameSourceAs(size) || value.refersToSameSourceAs(drawMode) || value.refersToSameSourceAs(saveContents))
        {
            updateSettings();
        }
        else if (value.refersToSameSourceAs(range))
        {
            auto min = static_cast<float>(range.getValue().getArray()->getReference(0));
            auto max = static_cast<float>(range.getValue().getArray()->getReference(1));
            graph.array.setScale({min, max});
            repaint();
        }
        else
        {
            GUIObject::valueChanged(value);
        }
    }

    void paintOverChildren(Graphics& g) override
    {
        auto outlineColour = box->findColour(cnv->isSelected(box) && !cnv->isGraph ? PlugDataColour::highlightColourId : PlugDataColour::canvasOutlineColourId);
        g.setColour(outlineColour);
        g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(0.5f), 2.0f, 1.0f);
    }

    PdArray getArray() const noexcept
    {
        return {libpd_array_get_name(static_cast<t_canvas*>(ptr)->gl_list), cnv->pd->m_instance};
    }

   private:
    Value name, size, drawMode, saveContents, range;

    PdArray array;
    GraphicalArray graph;
};
