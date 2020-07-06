/*
  ==============================================================================

    OverlayEditorComponentBase.cpp
    Created: 2 Jul 2020 11:32:30am
    Author:  musah

  ==============================================================================
*/

#include "OverlayEditorComponentBase.h"

OverlayEditorComponentBase::OverlayEditorComponentBase()
{
    m_closeButton = std::make_unique<DrawableButton>(String(), DrawableButton::ButtonStyle::ImageFitted);
    std::unique_ptr<XmlElement> Close_svg_xml = XmlDocument::parse(BinaryData::close_fullscreen24px_svg);
    // create svg images from resources for regular state
    std::unique_ptr<juce::Drawable> drawableCloseNormalImage = Drawable::createFromSVG(*(Close_svg_xml.get()));
    drawableCloseNormalImage->replaceColour(Colours::black, Colours::white);
    std::unique_ptr<juce::Drawable> drawableCloseOverImage = Drawable::createFromSVG(*(Close_svg_xml.get()));
    drawableCloseOverImage->replaceColour(Colours::black, Colours::lightgrey);
    std::unique_ptr<juce::Drawable> drawableCloseDownImage = Drawable::createFromSVG(*(Close_svg_xml.get()));
    drawableCloseDownImage->replaceColour(Colours::black, Colours::grey);
    // create svg images from resources for ON state
    std::unique_ptr<juce::Drawable> drawableCloseNormalOnImage = Drawable::createFromSVG(*(Close_svg_xml.get()));
    drawableCloseNormalOnImage->replaceColour(Colours::black, Colours::white);
    std::unique_ptr<juce::Drawable> drawableCloseOverOnImage = Drawable::createFromSVG(*(Close_svg_xml.get()));
    drawableCloseOverOnImage->replaceColour(Colours::black, Colours::white);
    std::unique_ptr<juce::Drawable> drawableCloseDownOnImage = Drawable::createFromSVG(*(Close_svg_xml.get()));
    drawableCloseDownOnImage->replaceColour(Colours::black, Colours::white);
    // set the images to button
    m_closeButton->setImages(drawableCloseNormalImage.get(), drawableCloseOverImage.get(), drawableCloseDownImage.get(), nullptr, drawableCloseNormalOnImage.get(), drawableCloseOverOnImage.get(), drawableCloseDownOnImage.get(), nullptr);
    addAndMakeVisible(m_closeButton.get());
    m_closeButton->addListener(this);
}

OverlayEditorComponentBase::~OverlayEditorComponentBase()
{
	m_overlayListener = nullptr;
}

void OverlayEditorComponentBase::addOverlayListener(OverlayListener *l)
{
    m_overlayListener = l;
}

void OverlayEditorComponentBase::paint(Graphics& g)
{
	g.setColour(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));
	g.fillRect(getLocalBounds().toFloat());
}

void OverlayEditorComponentBase::resized()
{
    m_closeButton->setBounds(Rectangle<int>(getWidth() - 30, 5, 25, 25));
}

void OverlayEditorComponentBase::buttonClicked(Button* button)
{
    if ((button == m_closeButton.get()) && m_overlayListener)
        m_overlayListener->toggleEditor();
}