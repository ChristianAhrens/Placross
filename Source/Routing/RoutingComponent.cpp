/*
  ==============================================================================

    RoutingComponent.cpp
    Created: 24 Jun 2020 11:52:17am
    Author:  Christian Ahrens

  ==============================================================================
*/

#include "RoutingComponent.h"

#include "../submodules/JUCE-AppBasics/Source/Image_utils.hpp"

RoutingComponent::RoutingComponent()
{
	m_sumButton = std::make_unique<DrawableButton>(String(), DrawableButton::ButtonStyle::ImageFitted);
    std::unique_ptr<juce::Drawable> NormalImage, OverImage, DownImage, DisabledImage, NormalOnImage, OverOnImage, DownOnImage, DisabledOnImage;
    JUCEAppBasics::Image_utils::getDrawableButtonImages(BinaryData::mediation24px_svg, NormalImage, OverImage, DownImage, DisabledImage, NormalOnImage, OverOnImage, DownOnImage, DisabledOnImage);
    // set the images to button
    m_sumButton->setImages(NormalImage.get(), OverImage.get(), DownImage.get(), DisabledImage.get(), NormalOnImage.get(), OverOnImage.get(), DownOnImage.get(), DisabledOnImage.get());
    addAndMakeVisible(m_sumButton.get());
    m_sumButton->addListener(this);

    setIOCount(2, 2);
}

RoutingComponent::~RoutingComponent()
{
    clearRouting();
}

void RoutingComponent::setIOCount(int inputChannelCount, int outputChannelCount)
{
    if ((m_inputChannelCount != inputChannelCount) || (m_outputChannelCount != outputChannelCount))
    {
        m_inputChannelCount = inputChannelCount;
        m_outputChannelCount = outputChannelCount;

        initialiseRouting();
    }
}

void RoutingComponent::resized()
{
    OverlayToggleComponentBase::resized();

    if (getCurrentOverlayState() == minimized)
    {
        FlexBox fb;
        fb.flexDirection = FlexBox::Direction::column;
        fb.justifyContent = FlexBox::JustifyContent::center;
        fb.items.addArray({ FlexItem(*m_sumButton).withFlex(1).withMaxHeight(30) });
        fb.performLayout(getOverlayBounds().toFloat());
    }
    else if (getCurrentOverlayState() == maximized)
    {
        auto matrixNodeSize = 40;
        auto matrixWidth = (m_inputChannelCount + 1) * matrixNodeSize;
        auto matrixHeight = (m_outputChannelCount + 1) * matrixNodeSize;
        auto xPos = static_cast<int>(0.5f * (getWidth() - matrixWidth) - (0.5f * matrixNodeSize));
        auto yPos = static_cast<int>(0.5f * (getHeight() - matrixHeight) - (0.5f * matrixNodeSize));
        Rectangle<int> gridRect(xPos, yPos, matrixWidth, matrixHeight);

        Grid grid;
        grid.alignItems = Grid::AlignItems::center;
        grid.alignContent = Grid::AlignContent::center;

        grid.templateColumns.add(Grid::TrackInfo(1_fr));
        grid.templateRows.add(Grid::TrackInfo(1_fr));
        grid.items.add(GridItem());
        for (int i = 0; i < m_inputChannelCount; ++i)
        {
            auto origX = static_cast<float>(xPos + matrixNodeSize * (i + 1));
            auto origY = static_cast<float>(yPos + matrixNodeSize);
            m_inputLabels.at(i)->setTransform(AffineTransform::rotation(-MathConstants<float>::halfPi, origX, origY).translated(static_cast<float>(matrixNodeSize), 0.0f));

            grid.templateColumns.add(Grid::TrackInfo(1_fr));
            grid.items.add(GridItem(*m_inputLabels.at(i)));
        }

        for (int j = 0; j < m_outputChannelCount; ++j)
        {
            grid.templateRows.add(Grid::TrackInfo(1_fr));
            grid.items.add(GridItem(*m_outputLabels.at(j)));
            for (int k = 0; k < m_inputChannelCount; ++k)
            {
                grid.items.add(GridItem(*m_nodeButtons.at(k).at(j)));
            }
        }

        grid.performLayout(gridRect);
    }
}

void RoutingComponent::buttonClicked(Button* button)
{
    if (button == m_sumButton.get())
    {
        
    }
    else
    {
        auto newRoutingMap = std::multimap<int, int>{};
        auto changesPresent = false;

        if (m_inputChannelCount == m_nodeButtons.size())
        {
            for (int i = 0; i < m_inputChannelCount; ++i)
            {
                if (m_outputChannelCount == m_nodeButtons[i].size())
                {
                    for (int j = 0; j < m_outputChannelCount; ++j)
                    {
                        // we normally would have to check in m_routingMap for the current value of the node,
                        // but for the moment do not want to take the lock from processing thread
                        // - lets see through debugging if we will still have to implement this
                        changesPresent = true;

                        if (m_nodeButtons[i][j]->getToggleState())
                        {
                            newRoutingMap.insert(std::make_pair(i, j));
                        }
                    }
                }
                else
                    jassertfalse;
            }
        }
        else
            jassertfalse;

        if(changesPresent)
            onRoutingEditingFinished(newRoutingMap);
    }
}

void RoutingComponent::initialiseRouting()
{
    clearRouting();

    for (int in = 0; in < m_inputChannelCount; ++in)
    {
        for (int out = 0; out < m_outputChannelCount; ++out)
        {
            m_routingMap.insert(std::make_pair(in, out));
        }
    }

    for (int i = 0; i < m_inputChannelCount; ++i)
    {
        auto label = std::make_unique<Label>();
        label->setText("In " + String(i + 1), dontSendNotification);
        addAndMakeVisible(label.get());
        m_inputLabels.push_back(std::move(label));
    }

    for (int j = 0; j < m_outputChannelCount; ++j)
    {
        auto label = std::make_unique<Label>();
        label->setText("Out " + String(j + 1), dontSendNotification);
        addAndMakeVisible(label.get());
        m_outputLabels.push_back(std::move(label));
    }

    std::unique_ptr<juce::Drawable> NormalImage, OverImage, DownImage, DisabledImage, NormalOnImage, OverOnImage, DownOnImage, DisabledOnImage;
    JUCEAppBasics::Image_utils::getDrawableButtonImages(BinaryData::radio_button_checked24px_svg, NormalImage, OverImage, DownImage, DisabledImage, NormalOnImage, OverOnImage, DownOnImage, DisabledOnImage);
    JUCEAppBasics::Image_utils::getDrawableButtonImages(BinaryData::radio_button_unchecked24px_svg, NormalImage, OverImage, DownImage, DisabledImage);

    for (int i = 0; i < m_inputChannelCount; ++i)
    {
        std::vector<std::unique_ptr<DrawableButton>> v;
        for (int j = 0; j < m_outputChannelCount; ++j)
        {
            auto drawableButton = std::make_unique<DrawableButton>(String(), DrawableButton::ButtonStyle::ImageFitted);
            drawableButton->setClickingTogglesState(true);
            drawableButton->setImages(NormalImage.get(), OverImage.get(), DownImage.get(), DisabledImage.get(), NormalOnImage.get(), OverOnImage.get(), DownOnImage.get(), DisabledOnImage.get());
            drawableButton->addListener(this);
            addAndMakeVisible(drawableButton.get());
            v.push_back(std::move(drawableButton));
        }
        m_nodeButtons.push_back(std::move(v));
    }

    setRouting(m_routingMap);
}

void RoutingComponent::clearRouting()
{
    m_routingMap.clear();
    
    for (auto const& label : m_inputLabels)
        removeChildComponent(label.get());
    m_inputLabels.clear();

    for (auto const& label : m_outputLabels)
        removeChildComponent(label.get());
    m_outputLabels.clear();

    for (auto const& nodeButtonRow : m_nodeButtons)
        for (auto const& button : nodeButtonRow)
            removeChildComponent(button.get());
    m_nodeButtons.clear();
}

void RoutingComponent::setRouting(std::multimap<int, int> const& routingMap)
{
    onRoutingEditingFinished(routingMap);

    if (m_inputChannelCount == m_nodeButtons.size())
    {
        for (int in = 0; in < m_inputChannelCount; ++in)
        {
            if (m_outputChannelCount == m_nodeButtons[in].size())
            {
                auto range = m_routingMap.equal_range(in);
                std::unordered_set<int> activeOuts;
                for (auto rit = range.first; rit != range.second; rit++)
                    activeOuts.insert(rit->second);
                for (int out = 0; out < m_outputChannelCount; ++out)
                {
                    if (m_routingMap.count(in) > 0 && activeOuts.count(out) > 0)
                    {
                        m_nodeButtons[in][out]->setToggleState(true, dontSendNotification);
                    }
                    else
                    {
                        m_nodeButtons[in][out]->setToggleState(false, dontSendNotification);
                    }
                }
            }
            else
                jassertfalse;
        }
    }
    else
        jassertfalse;
}

void RoutingComponent::audioDeviceIOCallback(const float** inputChannelData, int numInputChannels, float** outputChannelData, int numOutputChannels, int numSamples)
{
    const ScopedLock sl(m_routingLock);

    m_routingOutputBuffer.setSize(numOutputChannels, numSamples, false, true, true);
    m_routingOutputBuffer.clear();

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
                    m_routingOutputBuffer.addFrom(out, 0, inputChannelData[in], numSamples);
                }
            }
        }
    }

    for (int out = 0; out < numOutputChannels; ++out)
        memcpy(outputChannelData[out], m_routingOutputBuffer.getReadPointer(out), numSamples * sizeof(float));
}

void RoutingComponent::audioDeviceAboutToStart(AudioIODevice* device)
{
    ignoreUnused(device);
}

void RoutingComponent::audioDeviceStopped()
{
}

void RoutingComponent::audioDeviceError(const juce::String &errorMessage)
{
    ignoreUnused(errorMessage);
}

void RoutingComponent::onRoutingEditingFinished(std::multimap<int, int> const& newRouting)
{
    const ScopedLock sl(m_routingLock);

    m_routingMap = newRouting;
}

void RoutingComponent::changeOverlayState()
{
    OverlayToggleComponentBase::changeOverlayState();

    toggleMinimizedMaximizedElementVisibility(getCurrentOverlayState() == maximized);
}

void RoutingComponent::toggleMinimizedMaximizedElementVisibility(bool maximized)
{
    m_sumButton->setVisible(!maximized);

    for (auto const& nodeButtonRow : m_nodeButtons)
        for (auto const& button : nodeButtonRow)
            button->setVisible(maximized);
}