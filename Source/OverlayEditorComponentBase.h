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
    class OverlayListener
    {
    public:
        virtual ~OverlayListener() {};
        virtual void toggleEditor() = 0;
    };

public:
    class OverlayParent
    {
    public:
        virtual ~OverlayParent() {};
        virtual void setOverlayEditor(OverlayEditorComponentBase* editor) = 0;
        virtual bool isEditorActive() = 0;

    protected:
        OverlayEditorComponentBase *m_overlayEditor{ nullptr };
    };

public:
    //==============================================================================
    OverlayEditorComponentBase();
    ~OverlayEditorComponentBase() override;

    void addOverlayListener(OverlayListener *l);

    //==============================================================================
    void paint (Graphics& g) override;
    void resized() override;

    //==============================================================================
    void buttonClicked(Button* button) override;

private:
    //==============================================================================
    OverlayListener* m_overlayListener{ nullptr };

    //==============================================================================
    std::unique_ptr<DrawableButton> m_closeButton;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OverlayEditorComponentBase)
};