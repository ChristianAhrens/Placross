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

class ChannelStripParameterListener : private AudioProcessorParameter::Listener,
    private AudioProcessorListener,
    private Timer
{
public:
    ChannelStripParameterListener(AudioProcessor& proc, const Array<AudioProcessorParameter*>& params, int parameterIndex = -1)
        : m_processor(proc), m_parameters(params), m_isLegacyParam(params.isEmpty() ? false : juce::LegacyAudioParameter::isLegacy(params[0])), m_singleParameterIndex(parameterIndex)
    {
        if (m_isLegacyParam)
            m_processor.addListener(this);
        else
        {
            for (auto parameter : m_parameters)
                parameter->addListener(this);
        }

        startTimer(100);
    }

    ~ChannelStripParameterListener() override
    {
        if (m_isLegacyParam)
            m_processor.removeListener(this);
        else
        {
            for (auto parameter : m_parameters)
                parameter->removeListener(this);
        }
    }

    const Array<AudioProcessorParameter*>& getParameters() const noexcept
    {
        return m_parameters;
    }

    AudioProcessorParameter& getParameter(int parameterIndex = -1) const noexcept
    {
        if (parameterIndex > -1 && m_parameters.size() > parameterIndex)
            return *m_parameters.getUnchecked(parameterIndex);
        else if (m_singleParameterIndex > -1 && m_parameters.size() > m_singleParameterIndex)
            return *m_parameters.getUnchecked(m_singleParameterIndex);
        else if (!m_parameters.isEmpty())
            return *m_parameters.getFirst();
        else
            jassertfalse;
    }

    virtual void handleNewParameterValue(int parameterIndex) = 0;

private:
    //==============================================================================
    void parameterValueChanged(int, float) override
    {
        m_parameterValueHasChanged = 1;
    }

    void parameterGestureChanged(int, bool) override {}

    //==============================================================================
    void audioProcessorParameterChanged(AudioProcessor*, int index, float) override
    {
        for (auto parameter : m_parameters)
        {
            if (index == parameter->getParameterIndex())
                m_parameterValueHasChanged = 1;
        }
    }

    void audioProcessorChanged(AudioProcessor*) override {}

    //==============================================================================
    void timerCallback() override
    {
        if (m_parameterValueHasChanged.compareAndSetBool(0, 1))
        {
            handleNewParameterValue(m_singleParameterIndex);
            startTimerHz(50);
        }
        else
        {
            startTimer(jmin(250, getTimerInterval() + 10));
        }
    }

    AudioProcessor& m_processor;
    const Array<AudioProcessorParameter*>& m_parameters;
    const int m_singleParameterIndex;
    Atomic<int> m_parameterValueHasChanged{ 0 };
    const bool m_isLegacyParam;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChannelStripParameterListener)
};

//==============================================================================
class BooleanParameterComponent final : public Component,
    private ChannelStripParameterListener
{
public:
    BooleanParameterComponent(AudioProcessor& proc, const Array<AudioProcessorParameter*>& params, int parameterIndex = -1)
        : ChannelStripParameterListener(proc, params, parameterIndex)
    {
        // Set the initial value.
        handleNewParameterValue(parameterIndex);

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
    void handleNewParameterValue(int parameterIndex) override
    {
        ignoreUnused(parameterIndex);

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
    private ChannelStripParameterListener
{
public:
    SwitchParameterComponent(AudioProcessor& proc, const Array<AudioProcessorParameter*>& params, int parameterIndex = -1)
        : ChannelStripParameterListener(proc, params, parameterIndex)
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
        handleNewParameterValue(parameterIndex);

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
    void handleNewParameterValue(int parameterIndex) override
    {
        ignoreUnused(parameterIndex);

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
    private ChannelStripParameterListener
{
public:
    ChoiceParameterComponent(AudioProcessor& proc, const Array<AudioProcessorParameter*>& params, int parameterIndex = -1)
        : ChannelStripParameterListener(proc, params, parameterIndex),
        parameterValues(getParameter().getAllValueStrings())
    {
        box.addItemList(parameterValues, 1);

        // Set the initial value.
        handleNewParameterValue(parameterIndex);

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
    void handleNewParameterValue(int parameterIndex) override
    {
        auto index = parameterValues.indexOf(getParameter(parameterIndex).getCurrentValueAsText());

        if (index < 0)
        {
            // The parameter is producing some unexpected text, so we'll do
            // some linear interpolation.
            index = roundToInt(getParameter(parameterIndex).getValue() * (parameterValues.size() - 1));
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
class CustomColouredParameter
{
public:
    CustomColouredParameter() {};

    virtual void setCustomColour(const Colour& colour) = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CustomColouredParameter)
};

//==============================================================================
class SliderParameterComponent final :
    public Component,
    private ChannelStripParameterListener, 
    public CustomColouredParameter
{
public:
    SliderParameterComponent(AudioProcessor& proc, const Array<AudioProcessorParameter*>& params, int parameterIndex = -1)
        : ChannelStripParameterListener(proc, params, parameterIndex)
    {
        auto rangeMin = 0.0f;
        auto rangeMax = 1.0f;
        auto defaultVal = 1.0f;
        auto rangeStep = 1.0f;
        auto fParam = dynamic_cast<AudioParameterFloat*>(&getParameter(parameterIndex));
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
        handleNewParameterValue(parameterIndex);

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

    void setCustomColour(const Colour& colour) override
    {
        m_slider.setColour(Slider::ColourIds::thumbColourId, colour);
    }

private:
    void handleNewParameterValue(int parameterIndex) override
    {
        if (!m_isDragging)
        {
            auto defaultVal = 1.0f;
            auto param = &getParameter(parameterIndex);

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
class FilterParameterComponent :
    private ChannelStripParameterListener, 
    public CustomColouredParameter,
    public Component
{
public:
    FilterParameterComponent(ChannelStripProcessorBase& proc)
        : ChannelStripParameterListener(proc, proc.getParameters()), m_processor(proc)
    {
        m_type = proc.getType();

        m_filterPathThickness = 4.0;
        m_magResponseColour = getLookAndFeel().findColour(TableHeaderComponent::ColourIds::outlineColourId);

        m_minFrequency = 20.0;
        m_maxFrequency = 20000.0;
        m_maxDecibels = 20.0;




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

        // Set the initial value.
        handleNewParameterValue(-1);

        setSize(80, 80);
    };

    void setCustomColour(const Colour& colour) override
    {
        m_magResponseColour = colour;
    }

    void paint(Graphics& g) override
    {
        g.setColour(getLookAndFeel().findColour(ResizableWindow::backgroundColourId).darker());
        g.fillRect(getBounds());

        m_magnitudeResponsePath.clear();

        switch (m_type)
        {
        case ChannelStripProcessorBase::ChannelStripProcessorType::CSPT_LowPass:
            drawLowpass();
            break;
        case ChannelStripProcessorBase::ChannelStripProcessorType::CSPT_HighPass:
            drawHighpass();
            break;
        }

        //Close the response path drawn
        m_magnitudeResponsePath.closeSubPath();

        g.setColour(m_magResponseColour);
        g.strokePath(m_magnitudeResponsePath, PathStrokeType(m_filterPathThickness));

        // draw cutoff thumb
        auto freq = m_processor.getFilterFequency();
        auto thumbDiameter = 3.0f * m_filterPathThickness;
        auto thumbXPos = frequencyToxAxis(freq);
        auto thumbYPos = dbToYAxis(m_processor.getMagnitudeResponse(freq));
        g.fillEllipse(thumbXPos - 0.5f * thumbDiameter, thumbYPos - 0.5f * thumbDiameter, thumbDiameter, thumbDiameter);

        /*
           Fill area under/inside path with same colour at a lower alpha / highlight value.
           Try setting magResponseColour.withAlpha value to different values to get a fill shade you like.
         */
        g.setColour(m_magResponseColour.withAlpha((uint8)0x9a));
        g.fillPath(m_magnitudeResponsePath);

        g.setColour(getLookAndFeel().findColour(TableHeaderComponent::ColourIds::outlineColourId));
        g.drawRect(getBounds());
    }

    void mouseDown(const MouseEvent& e) override
    {
        thumbStartedDragging();
        mouseDrag(e); // forward event to drag handling method to avoid code copy (we want to do the same thing with position)
    }
    void mouseDrag(const MouseEvent& e) override
    {
        auto pos = e.getPosition();

        auto newFreqVal = xAxisToFrequency(pos.getX());
        auto clippedFreqVal = jlimit(m_minFrequency, m_maxFrequency, newFreqVal);

        auto newGainVal = Decibels::decibelsToGain(yAxisToGaindB(pos.getY()));
        auto clippedGainVal = jlimit(0.0f, 1.0f, newGainVal);

        thumbValueChanged(newFreqVal, newGainVal);
        //thumbValueChanged(clippedFreqVal, clippedGainVal);
    }
    void mouseUp(const MouseEvent& e) override
    {
        thumbStoppedDragging();
    }

private:
    void drawLowpass()
    {
        float freq = 0.0;
        float magnitudeDBValue = 0.0;

        /*
            LowPass so start path on left hand side of component i.e at 0.0f - using 0.0f - (filterPathThickness/2) for asthetic purposes
            Try commeting out - (filterPathThickness/2) to see the effect. This line hides the highlighted path edge so that the path edge
            highlight shows only on the top of the magnitude response path.
         */
        magnitudeDBValue = m_processor.getMagnitudeResponse(m_minFrequency);
        m_magnitudeResponsePath.startNewSubPath((0.0f - (m_filterPathThickness / 2)), (getBottom() - (m_filterPathThickness / 2)));
        m_magnitudeResponsePath.lineTo((0.0f - (m_filterPathThickness / 2)), dbToYAxis(magnitudeDBValue));

        for (float xPos = 0.0; xPos < ((float)getWidth() + (m_filterPathThickness / 2)); xPos += (m_filterPathThickness / 2))
        {
            //Get the frequency value for the filter's magnitude response calculation
            freq = xAxisToFrequency(xPos);
            magnitudeDBValue = m_processor.getMagnitudeResponse(freq);
            m_magnitudeResponsePath.lineTo(xPos, dbToYAxis(magnitudeDBValue));
        }

        magnitudeDBValue = m_processor.getMagnitudeResponse(m_maxFrequency);
        m_magnitudeResponsePath.lineTo(((float)getWidth() + (m_filterPathThickness / 2)), dbToYAxis(magnitudeDBValue));

        /*
            Dirty Trick to close the path nicely when cutoff is at max level (this is not apparent for virtual analogue filters that have not been
            oversampled when cutoff is close to nyquist as the response is pulled to zero). Try commenting this line out and running the plugin at
            higher sample rate i.e 96khz to see the visual result of the path closing without this.
         */
        m_magnitudeResponsePath.lineTo(((float)getWidth() + (m_filterPathThickness / 2)), (getBottom() - (m_filterPathThickness / 2)));

    }

    void drawHighpass()
    {
        float freq = 0.0;
        float magnitudeDBValue = 0.0;

        //If HighPass start path on right hand side of component i.e at component width.
        magnitudeDBValue = m_processor.getMagnitudeResponse(m_maxFrequency);
        m_magnitudeResponsePath.startNewSubPath(((float)getWidth() + (m_filterPathThickness / 2)), (getBottom() - (m_filterPathThickness / 2)));
        m_magnitudeResponsePath.lineTo(((float)getWidth() + (m_filterPathThickness / 2)), dbToYAxis(magnitudeDBValue));

        for (float xPos = ((float)getWidth()); xPos > (m_filterPathThickness / 2); xPos -= (m_filterPathThickness / 2))
        {
            //Get the frequency value for the filter's magnitude response calculation
            freq = xAxisToFrequency(xPos);
            magnitudeDBValue = m_processor.getMagnitudeResponse(freq);
            m_magnitudeResponsePath.lineTo(xPos, dbToYAxis(magnitudeDBValue));
        }

        magnitudeDBValue = m_processor.getMagnitudeResponse(m_minFrequency);
        m_magnitudeResponsePath.lineTo((0.0f - (m_filterPathThickness / 2)), dbToYAxis(magnitudeDBValue));

        /*
            Dirty trick again to close the path nicely when cutoff at min level for High Pass - try cmmenting this line out to se the visual
            effect on the reponse path closing without it
         */
        m_magnitudeResponsePath.lineTo((0.0f - (m_filterPathThickness / 2)), (getBottom() - (m_filterPathThickness / 2)));
    }

    float xAxisToFrequency(float xPos)
    {
        auto width = static_cast<float>(getWidth());

        //Computes frequency from position on x axis of component. So if the xPos is equal to the component width the value returned will be maxFrequency.
        auto frequency = m_minFrequency * std::pow((m_maxFrequency / m_minFrequency), (xPos / width));

        return frequency;
    }
    float frequencyToxAxis(float freq)
    {
        auto width = static_cast<float>(getWidth());

        //Computes position on x axis of component from given frequency.
        auto xPos = width * (std::log(freq / m_minFrequency) / std::log(m_maxFrequency / m_minFrequency));

        return xPos;
    }

    float dbToYAxis(float dbGain)
    {
        float height = static_cast<float>(getHeight());

        //Scale gain with this value, height of component is divded by maxDB * 2 for -max to +max db response display
        float scale = -(height) / (m_maxDecibels * 2);
        float scaledDbGain = dbGain * scale;

        /*
            Negative db values will result in a negative yposition so add height/2 to result to scale into
            correct component position for drawing. Test these calculations with a dbGain value equal to maxDecibels and
            yPostion computed will be equal to the filterResponseDisplay's height as is correct.
         */
        float yPosition = scaledDbGain + (height / 2);
        return yPosition;
    }
    float yAxisToGaindB(float yPos)
    {
        auto height = static_cast<float>(getHeight());
        auto y = height - yPos;

        auto gainRatio = ((y - 0.5f * height) / (0.5f * height));
        auto scaledDbGain = gainRatio * m_maxDecibels;

        return scaledDbGain;
    }

    void handleNewParameterValue(int parameterIndex) override
    {
        if (!m_isDragging)
        {
            //auto defaultVal = 1.0f;
            //auto param = &getParameter(parameterIndex);
            //
            //auto fParam = dynamic_cast<AudioParameterFloat*>(param);
            //if (fParam)
            //    defaultVal = *fParam;
            //else
            //    defaultVal = param->getValue();
            //
            //m_slider.setValue(defaultVal, dontSendNotification);
        }
    }

    void thumbValueChanged(float newFreqVal, float newGainVal)
    {
        // handle changed freq val
        if (getParameter(0).getValue() != newFreqVal)
        {
            if (!m_isDragging)
                getParameter(0).beginChangeGesture();
        
            getParameter(0).setValueNotifyingHost(newFreqVal);
        
            if (!m_isDragging)
                getParameter(0).endChangeGesture();
        }

        // handle changed gain val
        if (getParameter(1).getValue() != newGainVal)
        {
            if (!m_isDragging)
                getParameter(1).beginChangeGesture();

            getParameter(1).setValueNotifyingHost(newGainVal);

            if (!m_isDragging)
                getParameter(1).endChangeGesture();
        }

        repaint();
    }

    void thumbStartedDragging()
    {
        m_isDragging = true;

        for(auto parameter : getParameters())
            parameter->beginChangeGesture();
    }

    void thumbStoppedDragging()
    {
        m_isDragging = false;

        for (auto parameter : getParameters())
            parameter->endChangeGesture();
    }

    bool m_isDragging = false;

    ChannelStripProcessorBase::ChannelStripProcessorType    m_type = ChannelStripProcessorBase::ChannelStripProcessorType::CSPT_Invalid;
    ChannelStripProcessorBase&                              m_processor;
    Path                                                    m_magnitudeResponsePath;
    float                                                   m_filterPathThickness;
    Colour                                                  m_magResponseColour;

    float                                                   m_minFrequency;
    float                                                   m_maxFrequency;
    float                                                   m_maxDecibels;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FilterParameterComponent)
};

//==============================================================================
class ChannelStripParameterDisplayComponent : public Component
{
public:
    ChannelStripParameterDisplayComponent(ChannelStripProcessorBase& processor, int parameterIndex = -1)
        : m_parameters(processor.getParameters()), m_singleParameterIndex(parameterIndex)
    {
        if (m_singleParameterIndex != -1)
            m_parameterName.setText(m_parameters.getUnchecked(m_singleParameterIndex)->getName(128), dontSendNotification);
        else
            m_parameterName.setText(processor.getName(), dontSendNotification);
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
        auto customColourComp = dynamic_cast<CustomColouredParameter*>(m_parameterComp.get());
        if (customColourComp)
            customColourComp->setCustomColour(colour);
    }

private:
    const Array<AudioProcessorParameter*>& m_parameters;
    const int m_singleParameterIndex;
    Label m_parameterName;
    std::unique_ptr<Component> m_parameterComp;

    std::unique_ptr<Component> createParameterComp(ChannelStripProcessorBase& processor) const
    {
        // create a custom highpass filter parameter component if the processor is one of our own highpass type
        if (processor.getType() == ChannelStripProcessorBase::CSPT_HighPass)
        {
            return std::make_unique<FilterParameterComponent>(processor);
        }

        // create a custom lowpass filter parameter component if the processor is one of our own lowpass type
        if (processor.getType() == ChannelStripProcessorBase::CSPT_LowPass)
        {
            return std::make_unique<FilterParameterComponent>(processor);
        }

        // The AU, AUv3 and VST (only via a .vstxml file) SDKs support
        // marking a parameter as boolean. If you want consistency across
        // all  formats then it might be best to use a
        // SwitchParameterComponent instead.
        if (m_parameters.size() == 1 && m_parameters.getFirst()->isBoolean())
            return std::make_unique<BooleanParameterComponent>(processor, m_parameters, m_parameters.getFirst()->getParameterIndex());

        // Most hosts display any parameter with just two steps as a switch.
        if (m_parameters.size() == 1 && m_parameters.getFirst()->getNumSteps() == 2)
            return std::make_unique<SwitchParameterComponent>(processor, m_parameters, m_parameters.getFirst()->getParameterIndex());

        // If we have a list of strings to represent the different states a
        // parameter can be in then we should present a dropdown allowing a
        // user to pick one of them.
        if (m_parameters.size() == 1 && !m_parameters.getFirst()->getAllValueStrings().isEmpty()
            && std::abs(m_parameters.getFirst()->getNumSteps() - m_parameters.getFirst()->getAllValueStrings().size()) <= 1)
            return std::make_unique<ChoiceParameterComponent>(processor, m_parameters, m_parameters.getFirst()->getParameterIndex());

        // Everything else can be represented as a slider.
        return std::make_unique<SliderParameterComponent>(processor, m_parameters, m_singleParameterIndex);
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
                    addAndMakeVisible(m_paramComponents.add(new ChannelStripParameterDisplayComponent(processor, param->getParameterIndex())));
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
        content->setSize(m_view.getWidth(), m_view.getHeight());
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