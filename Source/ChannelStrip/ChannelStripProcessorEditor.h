/*
  ==============================================================================

    ChannelStripProcessorEditor.h
    Created: 24 Jun 2020 11:52:17am
    Author:  Christian Ahrens

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class  ChannelStripProcessorEditor : public AudioProcessorEditor
{
public:
    //==============================================================================
    ChannelStripProcessorEditor(AudioProcessor&);
    ~ChannelStripProcessorEditor() override;

    void setChannelColour(const Colour& colour);

    //==============================================================================
    void resized() override;

    // This constructor has been changed to take a reference instead of a pointer
    JUCE_DEPRECATED_WITH_BODY(ChannelStripProcessorEditor(AudioProcessor* p), : ChannelStripProcessorEditor(*p) {})

private:
    //==============================================================================
    struct Pimpl;
    std::unique_ptr<Pimpl> pimpl;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChannelStripProcessorEditor)
};