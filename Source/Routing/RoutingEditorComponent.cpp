/*
  ==============================================================================

    RoutingEditorComponent.cpp
    Created: 2 Jul 2020 11:32:30am
    Author:  musah

  ==============================================================================
*/

#include "RoutingEditorComponent.h"

RoutingEditorComponent::RoutingEditorComponent(int RoutingInputChannelCount, int RoutingOutputChannelCount)
    : OverlayEditorComponentBase()
{
    m_inputChannelCount = RoutingInputChannelCount;
    m_outputChannelCount = RoutingOutputChannelCount;

    for (int i = 0; i < m_inputChannelCount; ++i)
    {
        for (int j = 0; j < m_outputChannelCount; ++j)
        {
            m_routingMap.insert(std::make_pair(i, j));
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

    // create svg images from resources for regular state
    std::unique_ptr<XmlElement> Node_normal_svg_xml = XmlDocument::parse(BinaryData::radio_button_unchecked24px_svg);
    std::unique_ptr<juce::Drawable> drawableNodeNormalImage = Drawable::createFromSVG(*(Node_normal_svg_xml.get()));
    drawableNodeNormalImage->replaceColour(Colours::black, Colours::white);
    std::unique_ptr<juce::Drawable> drawableNodeOverImage = Drawable::createFromSVG(*(Node_normal_svg_xml.get()));
    drawableNodeOverImage->replaceColour(Colours::black, Colours::lightgrey);
    std::unique_ptr<juce::Drawable> drawableNodeDownImage = Drawable::createFromSVG(*(Node_normal_svg_xml.get()));
    drawableNodeDownImage->replaceColour(Colours::black, Colours::grey);
    // create svg images from resources for ON state
    std::unique_ptr<XmlElement> Node_ON_svg_xml = XmlDocument::parse(BinaryData::radio_button_checked24px_svg);
    std::unique_ptr<juce::Drawable> drawableNodeNormalOnImage = Drawable::createFromSVG(*(Node_ON_svg_xml.get()));
    drawableNodeNormalOnImage->replaceColour(Colours::black, Colours::white);
    std::unique_ptr<juce::Drawable> drawableNodeOverOnImage = Drawable::createFromSVG(*(Node_ON_svg_xml.get()));
    drawableNodeOverOnImage->replaceColour(Colours::black, Colours::lightgrey);
    std::unique_ptr<juce::Drawable> drawableNodeDownOnImage = Drawable::createFromSVG(*(Node_ON_svg_xml.get()));
    drawableNodeDownOnImage->replaceColour(Colours::black, Colours::grey);
    
    for (int i = 0; i < m_inputChannelCount; ++i)
    {
        std::vector<std::unique_ptr<DrawableButton>> v;
        for (int j = 0; j < m_outputChannelCount; ++j)
        {
            auto drawableButton = std::make_unique<DrawableButton>(String(), DrawableButton::ButtonStyle::ImageFitted);
            drawableButton->setClickingTogglesState(true);
            drawableButton->setImages(drawableNodeNormalImage.get(), drawableNodeOverImage.get(), drawableNodeDownImage.get(), nullptr, drawableNodeNormalOnImage.get(), drawableNodeOverOnImage.get(), drawableNodeDownOnImage.get(), nullptr);
            addAndMakeVisible(drawableButton.get());
            v.push_back(std::move(drawableButton));
        }
        m_nodeButtons.push_back(std::move(v));
    }
}

void RoutingEditorComponent::setRouting(std::multimap<int, int> const& routingMap)
{
    m_routingMap = routingMap;

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

std::multimap<int, int> const& RoutingEditorComponent::getRouting()
{
    m_routingMap.clear();

    if (m_inputChannelCount == m_nodeButtons.size())
    {
        for (int i = 0; i < m_inputChannelCount; ++i)
        {
            if (m_outputChannelCount == m_nodeButtons[i].size())
            {
                for (int j = 0; j < m_outputChannelCount; ++j)
                {
                    if (m_nodeButtons[i][j]->getToggleState())
                    {
                        m_routingMap.insert(std::make_pair(i, j));
                    }
                }
            }
            else
                jassertfalse;
        }
    }
    else
        jassertfalse;

    return m_routingMap;
}

void RoutingEditorComponent::paint(Graphics& g)
{
    OverlayEditorComponentBase::paint(g);
}

void RoutingEditorComponent::resized()
{
    OverlayEditorComponentBase::resized();

    auto matrixNodeSize = 40;
    auto matrixWidth = (m_inputChannelCount + 1) * matrixNodeSize;
    auto matrixHeight = (m_outputChannelCount + 1) * matrixNodeSize;
    auto xPos = 0.5f * (getWidth() - matrixWidth) - (0.5f * matrixNodeSize);
    auto yPos = 0.5f * (getHeight() - matrixHeight) - (0.5f * matrixNodeSize);
    Rectangle<int> gridRect(xPos, yPos, matrixWidth, matrixHeight);

    Grid grid;
    grid.alignItems = Grid::AlignItems::center;
    grid.alignContent = Grid::AlignContent::center;
    
    grid.templateColumns.add(Grid::TrackInfo(1_fr));
    grid.templateRows.add(Grid::TrackInfo(1_fr));
    grid.items.add(GridItem());
    for (int i = 0; i < m_inputChannelCount; ++i)
    {
        m_inputLabels.at(i)->setTransform(AffineTransform::rotation(-MathConstants<float>::halfPi, xPos + matrixNodeSize*(i+1), yPos + matrixNodeSize).translated(matrixNodeSize, 0));

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