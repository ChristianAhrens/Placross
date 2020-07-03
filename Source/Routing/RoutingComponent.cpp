/*
  ==============================================================================

    RoutingComponent.cpp
    Created: 24 Jun 2020 11:52:17am
    Author:  Christian Ahrens

  ==============================================================================
*/

#include "RoutingComponent.h"

RoutingComponent::RoutingComponent()
{
	m_sumButton = std::make_unique<DrawableButton>(String(), DrawableButton::ButtonStyle::ImageFitted);
    std::unique_ptr<XmlElement> Routing_svg_xml = XmlDocument::parse(BinaryData::mediation24px_svg);
    // create svg images from resources for regular state
    std::unique_ptr<juce::Drawable> drawableRoutingNormalImage = Drawable::createFromSVG(*(Routing_svg_xml.get()));
    drawableRoutingNormalImage->replaceColour(Colours::black, Colours::white);
    std::unique_ptr<juce::Drawable> drawableRoutingOverImage = Drawable::createFromSVG(*(Routing_svg_xml.get()));
    drawableRoutingOverImage->replaceColour(Colours::black, Colours::lightgrey);
    std::unique_ptr<juce::Drawable> drawableRoutingDownImage = Drawable::createFromSVG(*(Routing_svg_xml.get()));
    drawableRoutingDownImage->replaceColour(Colours::black, Colours::grey);
    // create svg images from resources for ON state
    std::unique_ptr<juce::Drawable> drawableRoutingNormalOnImage = Drawable::createFromSVG(*(Routing_svg_xml.get()));
    drawableRoutingNormalOnImage->replaceColour(Colours::black, Colours::white);
    std::unique_ptr<juce::Drawable> drawableRoutingOverOnImage = Drawable::createFromSVG(*(Routing_svg_xml.get()));
    drawableRoutingOverOnImage->replaceColour(Colours::black, Colours::white);
    std::unique_ptr<juce::Drawable> drawableRoutingDownOnImage = Drawable::createFromSVG(*(Routing_svg_xml.get()));
    drawableRoutingDownOnImage->replaceColour(Colours::black, Colours::white);
    // set the images to button
    m_sumButton->setImages(drawableRoutingNormalImage.get(), drawableRoutingOverImage.get(), drawableRoutingDownImage.get(), nullptr, drawableRoutingNormalOnImage.get(), drawableRoutingOverOnImage.get(), drawableRoutingDownOnImage.get(), nullptr);
    addAndMakeVisible(m_sumButton.get());
    m_sumButton->addListener(this);

    setIOCount(2, 2);
	initialiseRouting();

	setSize(600, 460);
}

RoutingComponent::~RoutingComponent()
{
}

void RoutingComponent::setIOCount(int inputChannelCount, int outputChannelCount)
{
    m_inputChannelCount = inputChannelCount;
    m_outputChannelCount = outputChannelCount;
}

void RoutingComponent::paint(Graphics& g)
{
	g.setColour(getLookAndFeel().findColour(TextEditor::ColourIds::highlightColourId));
	g.fillRoundedRectangle(getLocalBounds().toFloat(), 10.0f);
}

void RoutingComponent::resized()
{
	FlexBox fb;
	fb.flexDirection = FlexBox::Direction::column;
	fb.justifyContent = FlexBox::JustifyContent::center;
	fb.items.addArray({ FlexItem(*m_sumButton).withFlex(1) });
	fb.performLayout(getLocalBounds().toFloat());

    if (m_editor)
    {
        auto parent = getParentComponent();
        if (parent)
        {
            m_editor->setBounds(parent->getBounds().reduced(10));
        }
    }
}

void RoutingComponent::buttonClicked(Button* button)
{
    if (button == m_sumButton.get())
    {
        toggleEditor();
    }
}

void RoutingComponent::initialiseRouting()
{
    for (int in = 0; in < m_inputChannelCount; ++in)
        for (int out = 0; out < m_outputChannelCount; ++out)
            m_routingMap.insert(std::make_pair(in, out));
}

void RoutingComponent::audioDeviceIOCallback(const float** inputChannelData,
	int numInputChannels,
	float** outputChannelData,
	int numOutputChannels,
	int numSamples)
{
    const ScopedLock sl(routingLock);

    if (m_routingMap.empty())
        return;

    AudioBuffer<float> buffer;
    buffer.setSize(numOutputChannels, numSamples);

    for (int in = 0; in < numInputChannels; ++in)
    {
        if(m_routingMap.count(in) == 0)
            continue;

        auto range = m_routingMap.equal_range(in);
        for (auto rit = range.first; rit != range.second; rit++)
        {
            for (int out = 0; out < numOutputChannels; ++out)
            {
                if (rit->second == out)
                {
                    buffer.addFrom(out, 0, inputChannelData[in], numSamples);
                }
            }
        }
    }

    //(outputChannelData, buffer.getArrayOfReadPointers(), numOutputChannels * numSamples * sizeof(float));
}

void RoutingComponent::audioDeviceAboutToStart(AudioIODevice* device)
{
}

void RoutingComponent::audioDeviceStopped()
{
}

void RoutingComponent::audioDeviceError(const juce::String &errorMessage)
{
}

void RoutingComponent::onRoutingEditingFinished(std::multimap<int, int> const& newRouting)
{
    const ScopedLock sl(routingLock);

    m_routingMap = newRouting;
}

void RoutingComponent::toggleEditor()
{
    auto parent = getParentComponent();
    if (parent)
    {
        if (m_editor)
        {
            onRoutingEditingFinished(m_editor->getRouting());
            m_editor->setVisible(false);
            parent->removeChildComponent(m_editor.get());
            m_editor.reset();
        }
        else
        {
            m_editor = std::make_unique<RoutingEditorComponent>(m_inputChannelCount, m_outputChannelCount);
            m_editor->setRouting(m_routingMap);
            m_editor->addListener(this);
            parent->addAndMakeVisible(m_editor.get());
            resized();
        }
    }
}