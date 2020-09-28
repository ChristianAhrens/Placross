/*
  ==============================================================================

    ChannelStripProcessorEditor.cpp
    Created: 24 Jun 2020 11:52:17am
    Author:  Christian Ahrens

  ==============================================================================
*/

#include "ChannelStripProcessorEditor.h"

#include "ChannelStripProcessor.h"

#include <juce_audio_processors/format_types/juce_LegacyAudioParameter.cpp>

class ParameterListener : private AudioProcessorParameter::Listener,
    private AudioProcessorListener,
    private Timer
{
public:
    ParameterListener(AudioProcessor& proc, AudioProcessorParameter& param)
        : processor(proc), parameter(param), isLegacyParam(juce::LegacyAudioParameter::isLegacy(&param))
    {
        if (isLegacyParam)
            processor.addListener(this);
        else
            parameter.addListener(this);

        startTimer(100);
    }

    ~ParameterListener() override
    {
        if (isLegacyParam)
            processor.removeListener(this);
        else
            parameter.removeListener(this);
    }

    AudioProcessorParameter& getParameter() const noexcept
    {
        return parameter;
    }

    virtual void handleNewParameterValue() = 0;

private:
    //==============================================================================
    void parameterValueChanged(int, float) override
    {
        parameterValueHasChanged = 1;
    }

    void parameterGestureChanged(int, bool) override {}

    //==============================================================================
    void audioProcessorParameterChanged(AudioProcessor*, int index, float) override
    {
        if (index == parameter.getParameterIndex())
            parameterValueHasChanged = 1;
    }

    void audioProcessorChanged(AudioProcessor*) override {}

    //==============================================================================
    void timerCallback() override
    {
        if (parameterValueHasChanged.compareAndSetBool(0, 1))
        {
            handleNewParameterValue();
            startTimerHz(50);
        }
        else
        {
            startTimer(jmin(250, getTimerInterval() + 10));
        }
    }

    AudioProcessor& processor;
    AudioProcessorParameter& parameter;
    Atomic<int> parameterValueHasChanged{ 0 };
    const bool isLegacyParam;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ParameterListener)
};

//==============================================================================
class BooleanParameterComponent final : public Component,
    private ParameterListener
{
public:
    BooleanParameterComponent(AudioProcessor& proc, AudioProcessorParameter& param)
        : ParameterListener(proc, param)
    {
        // Set the initial value.
        handleNewParameterValue();

        button.onClick = [this] { buttonClicked(); };

        addAndMakeVisible(button);

        setSize(80, 40);
    }

    void paint(Graphics& /*g*/) override
    {
        //g.fillAll(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));
    }

    void resized() override
    {
        auto area = getLocalBounds();
        area.removeFromLeft(8);
        button.setBounds(area.reduced(0, 10));
    }

private:
    void handleNewParameterValue() override
    {
        button.setToggleState(isParameterOn(), dontSendNotification);
    }

    void buttonClicked()
    {
        if (isParameterOn() != button.getToggleState())
        {
            getParameter().beginChangeGesture();
            getParameter().setValueNotifyingHost(button.getToggleState() ? 1.0f : 0.0f);
            getParameter().endChangeGesture();
        }
    }

    bool isParameterOn() const { return getParameter().getValue() >= 0.5f; }

    ToggleButton button;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BooleanParameterComponent)
};

//==============================================================================
class SwitchParameterComponent final : public Component,
    private ParameterListener
{
public:
    SwitchParameterComponent(AudioProcessor& proc, AudioProcessorParameter& param)
        : ParameterListener(proc, param)
    {
        for (auto& button : buttons)
        {
            button.setRadioGroupId(293847);
            button.setClickingTogglesState(true);
        }

        buttons[0].setButtonText(getParameter().getText(0.0f, 16));
        buttons[1].setButtonText(getParameter().getText(1.0f, 16));

        buttons[0].setConnectedEdges(Button::ConnectedOnRight);
        buttons[1].setConnectedEdges(Button::ConnectedOnLeft);

        // Set the initial value.
        buttons[0].setToggleState(true, dontSendNotification);
        handleNewParameterValue();

        buttons[1].onStateChange = [this] { rightButtonChanged(); };

        for (auto& button : buttons)
            addAndMakeVisible(button);

        setSize(80, 40);
    }

    void paint(Graphics& /*g*/) override
    {
        //g.fillAll(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));
    }

    void resized() override
    {
        auto area = getLocalBounds().reduced(0, 8);
        area.removeFromLeft(8);

        for (auto& button : buttons)
            button.setBounds(area.removeFromLeft(80));
    }

private:
    void handleNewParameterValue() override
    {
        bool newState = isParameterOn();

        if (buttons[1].getToggleState() != newState)
        {
            buttons[1].setToggleState(newState, dontSendNotification);
            buttons[0].setToggleState(!newState, dontSendNotification);
        }
    }

    void rightButtonChanged()
    {
        auto buttonState = buttons[1].getToggleState();

        if (isParameterOn() != buttonState)
        {
            getParameter().beginChangeGesture();

            if (getParameter().getAllValueStrings().isEmpty())
            {
                getParameter().setValueNotifyingHost(buttonState ? 1.0f : 0.0f);
            }
            else
            {
                // When a parameter provides a list of strings we must set its
                // value using those strings, rather than a float, because VSTs can
                // have uneven spacing between the different allowed values and we
                // want the snapping behaviour to be consistent with what we do with
                // a combo box.
                auto selectedText = buttons[buttonState ? 1 : 0].getButtonText();
                getParameter().setValueNotifyingHost(getParameter().getValueForText(selectedText));
            }

            getParameter().endChangeGesture();
        }
    }

    bool isParameterOn() const
    {
        if (getParameter().getAllValueStrings().isEmpty())
            return getParameter().getValue() > 0.5f;

        auto index = getParameter().getAllValueStrings()
            .indexOf(getParameter().getCurrentValueAsText());

        if (index < 0)
        {
            // The parameter is producing some unexpected text, so we'll do
            // some linear interpolation.
            index = roundToInt(getParameter().getValue());
        }

        return index == 1;
    }

    TextButton buttons[2];

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SwitchParameterComponent)
};

//==============================================================================
class ChoiceParameterComponent final : public Component,
    private ParameterListener
{
public:
    ChoiceParameterComponent(AudioProcessor& proc, AudioProcessorParameter& param)
        : ParameterListener(proc, param),
        parameterValues(getParameter().getAllValueStrings())
    {
        box.addItemList(parameterValues, 1);

        // Set the initial value.
        handleNewParameterValue();

        box.onChange = [this] { boxChanged(); };
        addAndMakeVisible(box);

        setSize(80, 40);
    }

    void paint(Graphics& /*g*/) override
    {
        //g.fillAll(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));
    }

    void resized() override
    {
        auto area = getLocalBounds();
        area.removeFromLeft(8);
        box.setBounds(area.reduced(0, 10));
    }

private:
    void handleNewParameterValue() override
    {
        auto index = parameterValues.indexOf(getParameter().getCurrentValueAsText());

        if (index < 0)
        {
            // The parameter is producing some unexpected text, so we'll do
            // some linear interpolation.
            index = roundToInt(getParameter().getValue() * (parameterValues.size() - 1));
        }

        box.setSelectedItemIndex(index);
    }

    void boxChanged()
    {
        if (getParameter().getCurrentValueAsText() != box.getText())
        {
            getParameter().beginChangeGesture();

            // When a parameter provides a list of strings we must set its
            // value using those strings, rather than a float, because VSTs can
            // have uneven spacing between the different allowed values.
            getParameter().setValueNotifyingHost(getParameter().getValueForText(box.getText()));

            getParameter().endChangeGesture();
        }
    }

    ComboBox box;
    const StringArray parameterValues;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChoiceParameterComponent)
};

//==============================================================================
class SliderParameterComponent final : public Component,
    private ParameterListener
{
public:
    SliderParameterComponent(AudioProcessor& proc, AudioProcessorParameter& param)
        : ParameterListener(proc, param)
    {
        auto rangeMin = 0.0f;
        auto rangeMax = 1.0f;
        auto defaultVal = 1.0f;
        auto rangeStep = 1.0f;
        auto fParam = dynamic_cast<AudioParameterFloat*>(&getParameter());
        if (fParam)
        {
            rangeMin = fParam->getNormalisableRange().getRange().getStart();
            rangeMax = fParam->getNormalisableRange().getRange().getEnd();
            defaultVal = *fParam;
            rangeStep = fParam->getNormalisableRange().interval;
        }

        m_slider.setRange(rangeMin, rangeMax, rangeStep);
        m_slider.setSkewFactorFromMidPoint(rangeMax/5);

        addAndMakeVisible(m_slider);

        // Set the initial value.
        handleNewParameterValue();

        m_slider.onValueChange = [this] { sliderValueChanged(); };
        m_slider.onDragStart = [this] { sliderStartedDragging(); };
        m_slider.onDragEnd = [this] { sliderStoppedDragging(); };

        setSize(80, 80);
    }

    void paint(Graphics& /*g*/) override
    {
        //g.fillAll(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));
    }

    void resized() override
    {
        m_slider.setBounds(getLocalBounds());
    }

    void setKnobColour(const Colour& colour)
    {
        m_slider.setColour(Slider::ColourIds::thumbColourId, colour);
    }

private:
    void handleNewParameterValue() override
    {
        if (!m_isDragging)
        {
            auto defaultVal = 1.0f;
            auto param = &getParameter();

            auto fParam = dynamic_cast<AudioParameterFloat*>(param);
            if (fParam)
                defaultVal = *fParam;
            else
                defaultVal = param->getValue();
            
            m_slider.setValue(defaultVal, dontSendNotification);
        }
    }

    void sliderValueChanged()
    {
        auto newVal = (float)m_slider.getValue();

        if (getParameter().getValue() != newVal)
        {
            if (!m_isDragging)
                getParameter().beginChangeGesture();

            getParameter().setValueNotifyingHost((float)m_slider.getValue());

            if (!m_isDragging)
                getParameter().endChangeGesture();
        }
    }

    void sliderStartedDragging()
    {
        m_isDragging = true;
        getParameter().beginChangeGesture();
    }

    void sliderStoppedDragging()
    {
        m_isDragging = false;
        getParameter().endChangeGesture();
    }

private:
    Slider m_slider{ Slider::Rotary, Slider::TextEntryBoxPosition::TextBoxBelow };
    bool m_isDragging = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SliderParameterComponent)
};

//==============================================================================
class ChannelStripParameterDisplayComponent : public Component
{
public:
    ChannelStripParameterDisplayComponent(ChannelStripProcessorBase& processor, AudioProcessorParameter& param)
        : m_parameter(param)
    {
        m_parameterName.setText(m_parameter.getName(128), dontSendNotification);
        m_parameterName.setJustificationType(Justification::left);
        addAndMakeVisible(m_parameterName);

        m_parameterComp = createParameterComp(processor);
        if (m_parameterComp)
        {
            addAndMakeVisible(*m_parameterComp);

            int maxWidth = jmax(400, m_parameterComp->getWidth());
            int height = jmax(20, 20 + m_parameterComp->getHeight());

            setSize(maxWidth, height);
        }
    }

    ChannelStripParameterDisplayComponent(ChannelStripProcessorBase& processor)
        : m_parameter(*processor.getParameters()[0])
    {
        m_parameterComp = createParameterComp(processor);
        if (m_parameterComp)
        {
            addAndMakeVisible(*m_parameterComp);

            int maxWidth = jmax(400, m_parameterComp->getWidth());
            int height = jmax(20, 20 + m_parameterComp->getHeight());

            setSize(maxWidth, height);
        }
    }

    void resized() override
    {
        FlexBox fb;
        fb.flexDirection = FlexBox::Direction::column;
        fb.justifyContent = FlexBox::JustifyContent::flexStart;
        fb.items.addArray({
            FlexItem(m_parameterName).withFlex(1),
            FlexItem(*m_parameterComp).withFlex(5)
            });
        fb.performLayout(getLocalBounds().toFloat());
    }

    void setChannelColour(const Colour& colour)
    {
        auto sliderComp = dynamic_cast<SliderParameterComponent*>(m_parameterComp.get());
        if (sliderComp)
            sliderComp->setKnobColour(colour);
    }

private:
    AudioProcessorParameter& m_parameter;
    Label m_parameterName;
    std::unique_ptr<Component> m_parameterComp;

    std::unique_ptr<Component> createParameterComp(ChannelStripProcessorBase& processor) const
    {
        // create a custom highpass filter parameter component if the processor is one of our own highpass type
        if (processor.getType() == ChannelStripProcessorBase::CSPT_HighPass)
        {
            // return std::make_unique<HighPassParameterComponent>(processor);
            return nullptr;
        }

        // create a custom lowpass filter parameter component if the processor is one of our own lowpass type
        if (processor.getType() == ChannelStripProcessorBase::CSPT_LowPass)
        {
            // return std::make_unique<LowPassParameterComponent>(processor);
            return nullptr;
        }

        // The AU, AUv3 and VST (only via a .vstxml file) SDKs support
        // marking a parameter as boolean. If you want consistency across
        // all  formats then it might be best to use a
        // SwitchParameterComponent instead.
        if (m_parameter.isBoolean())
            return std::make_unique<BooleanParameterComponent>(processor, m_parameter);

        // Most hosts display any parameter with just two steps as a switch.
        if (m_parameter.getNumSteps() == 2)
            return std::make_unique<SwitchParameterComponent>(processor, m_parameter);

        // If we have a list of strings to represent the different states a
        // parameter can be in then we should present a dropdown allowing a
        // user to pick one of them.
        if (!m_parameter.getAllValueStrings().isEmpty()
            && std::abs(m_parameter.getNumSteps() - m_parameter.getAllValueStrings().size()) <= 1)
            return std::make_unique<ChoiceParameterComponent>(processor, m_parameter);

        // Everything else can be represented as a slider.
        return std::make_unique<SliderParameterComponent>(processor, m_parameter);
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChannelStripParameterDisplayComponent)
};

//==============================================================================
class ChannelStripParametersPanel : public Component
{
public:
    ChannelStripParametersPanel(ChannelStripProcessorBase& processor)
    {
        switch (processor.getType())
        {
        case ChannelStripProcessorBase::CSPT_HighPass:
        case ChannelStripProcessorBase::CSPT_LowPass:
            addAndMakeVisible(m_paramComponents.add(new ChannelStripParameterDisplayComponent(processor)));
            break;
        case ChannelStripProcessorBase::CSPT_Gain:
        case ChannelStripProcessorBase::CSPT_Invalid:
            for (auto* param : processor.getParameters())
                if (param->isAutomatable())
                    addAndMakeVisible(m_paramComponents.add(new ChannelStripParameterDisplayComponent(processor, *param)));
            break;
        }

        int maxWidth = 400;
        int height = 0;

        for (auto& comp : m_paramComponents)
        {
            maxWidth = jmax(maxWidth, comp->getWidth());
            height += comp->getHeight();
        }

        setSize(maxWidth, height);
    }

    ~ChannelStripParametersPanel() override
    {
        m_paramComponents.clear();
    }

    void paint(Graphics& /*g*/) override
    {
        //g.fillAll(getLookAndFeel().findColour(TextEditor::ColourIds::highlightColourId));
    }

    void resized() override
    {
        FlexBox fb;
        fb.flexDirection = FlexBox::Direction::column;
        fb.justifyContent = FlexBox::JustifyContent::flexStart;
        for (auto* comp : m_paramComponents)
        {
            fb.items.add(FlexItem(*comp).withFlex(1));
        }
        fb.performLayout(getLocalBounds().toFloat());
    }

    void setChannelColour(const Colour& colour)
    {
        for (auto* comp : m_paramComponents)
        {
            comp->setChannelColour(colour);
        }
    }

private:
    OwnedArray<ChannelStripParameterDisplayComponent> m_paramComponents;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChannelStripParametersPanel)
};

//==============================================================================
struct ChannelStripProcessorEditor::Pimpl
{
    Pimpl(ChannelStripProcessorEditor& parent) : m_owner(parent)
    {
        auto* p = dynamic_cast<ChannelStripProcessorBase*>(parent.getAudioProcessor());
        jassert(p != nullptr);

        m_owner.setOpaque(true);

        m_view.setViewedComponent(new ChannelStripParametersPanel(*p));
        m_owner.addAndMakeVisible(m_view);

        m_view.setScrollBarsShown(true, true);
    }

    ~Pimpl()
    {
        m_view.setViewedComponent(nullptr, false);
    }

    void resize(Rectangle<int> size)
    {
        m_view.setBounds(size);
        auto content = m_view.getViewedComponent();
        content->setSize(m_view.getWidth(), content->getHeight());
    }

    void setChannelColour(const Colour& colour)
    {
        auto ppanel = dynamic_cast<ChannelStripParametersPanel*>(m_view.getViewedComponent());
        if (ppanel)
            ppanel->setChannelColour(colour);
    }

    Viewport* GetView()
    {
        return &m_view;
    }

private:
    //==============================================================================
    ChannelStripProcessorEditor& m_owner;
    juce::LegacyAudioParametersWrapper m_legacyParameters;
    Viewport m_view;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Pimpl)
};

//==============================================================================
ChannelStripProcessorEditor::ChannelStripProcessorEditor(ChannelStripProcessorBase& p)
    : AudioProcessorEditor(p), pimpl(new Pimpl(*this))
{
    setSize(pimpl->GetView()->getViewedComponent()->getWidth() + pimpl->GetView()->getVerticalScrollBar().getWidth(),
        jmin(pimpl->GetView()->getViewedComponent()->getHeight(), 400));

    setOpaque(false);
}

ChannelStripProcessorEditor::~ChannelStripProcessorEditor()
{
}

void ChannelStripProcessorEditor::resized()
{
    pimpl->resize(getLocalBounds());
}

void ChannelStripProcessorEditor::setChannelColour(const Colour& colour)
{
    pimpl->setChannelColour(colour);
}