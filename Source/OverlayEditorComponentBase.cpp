/*
  ==============================================================================

    OverlayEditorComponentBase.cpp
    Created: 2 Jul 2020 11:32:30am
    Author:  musah

  ==============================================================================
*/

#include "OverlayEditorComponentBase.h"

#include "../submodules/JUCE-AppBasics/Source/Image_utils.hpp"

OverlayEditorComponentBase::OverlayEditorComponentBase()
{
    m_closeButton = std::make_unique<DrawableButton>(String(), DrawableButton::ButtonStyle::ImageFitted);

    std::unique_ptr<juce::Drawable> NormalImage, OverImage, DownImage, DisabledImage, NormalOnImage, OverOnImage, DownOnImage, DisabledOnImage;
    JUCEAppBasics::Image_utils::getDrawableButtonImages(BinaryData::close_fullscreen24px_svg, NormalImage, OverImage, DownImage, DisabledImage, NormalOnImage, OverOnImage, DownOnImage, DisabledOnImage);
    // set the images to button
    m_closeButton->setImages(NormalImage.get(), OverImage.get(), DownImage.get(), DisabledImage.get(), NormalOnImage.get(), OverOnImage.get(), DownOnImage.get(), DisabledOnImage.get());
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