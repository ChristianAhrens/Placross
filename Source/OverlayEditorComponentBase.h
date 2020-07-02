/*
  ==============================================================================

    OverlayEditorComponentBase.h
    Created: 2 Jul 2020 11:32:30am
    Author:  musah

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
class OverlayEditorComponentBase  :   public Component, public DrawableButton::Listener
{
public:
    class Listener
    {
    public:
        virtual ~Listener() {};
        virtual void onRoutingEditingFinished(std::multimap<int, int> const& newRouting) = 0;
        virtual void toggleEditor() = 0;
    };

public:
    //==============================================================================
    OverlayEditorComponentBase();
    ~OverlayEditorComponentBase() override;

    void addListener(Listener *l);

    //==============================================================================
    void paint (Graphics& g) override;
    void resized() override;

    //==============================================================================
    void buttonClicked(Button* button) override;

private:
    //==============================================================================
    Listener* m_listener{ nullptr };

    //==============================================================================
    std::unique_ptr<DrawableButton> m_closeButton;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OverlayEditorComponentBase)
};