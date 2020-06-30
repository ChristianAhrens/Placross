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
	m_sumButton = std::make_unique<DrawableButton>(String(), DrawableButton::ButtonStyle::ImageStretched);
	addAndMakeVisible(m_sumButton.get());
	m_sumButton->addListener(this);

	initialiseRouting();

	setSize(600, 460);
}

RoutingComponent::~RoutingComponent()
{
}

void RoutingComponent::paint(Graphics& g)
{
	g.fillAll(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));

	g.setColour(getLookAndFeel().findColour(TextEditor::ColourIds::highlightColourId));
	g.fillRoundedRectangle(getLocalBounds().toFloat(), 10.0f);
}

void RoutingComponent::resized()
{
	FlexBox fb;
	fb.flexDirection = FlexBox::Direction::column;
	fb.justifyContent = FlexBox::JustifyContent::center;
	fb.items.addArray({ FlexItem(*m_sumButton).withFlex(1) });
	fb.performLayout(getLocalBounds().reduced(10).toFloat());
}

void RoutingComponent::buttonClicked(Button* button)
{
}

void RoutingComponent::initialiseRouting()
{

}

void RoutingComponent::audioDeviceIOCallback(const float** inputChannelData,
	int numInputChannels,
	float** outputChannelData,
	int numOutputChannels,
	int numSamples)
{
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