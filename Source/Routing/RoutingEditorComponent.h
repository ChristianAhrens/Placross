/*
  ==============================================================================

    RoutingEditorComponent.h
    Created: 2 Jul 2020 11:32:30am
    Author:  musah

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#include "../OverlayEditorComponentBase.h"

//==============================================================================
class RoutingEditorComponent  :   public OverlayEditorComponentBase
{
public:
    //==============================================================================
    RoutingEditorComponent(int RoutingInputChannelCount, int RoutingOutputChannelCount);

    std::multimap<int, int> const& getRouting();

    //==============================================================================
    void paint(Graphics& g) override;
    void resized() override;

private:
    //==============================================================================
    int m_inputChannelCount{ 0 };
    int m_outputChannelCount{ 0 };

    std::multimap<int, int> m_routingMap{};

    std::vector<std::unique_ptr<Label>>                          m_inputLabels;
    std::vector<std::unique_ptr<Label>>                          m_outputLabels;
    std::vector<std::vector<std::unique_ptr<DrawableButton>>>    m_nodeButtons;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RoutingEditorComponent)
};