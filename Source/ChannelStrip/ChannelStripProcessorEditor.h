/*
  ==============================================================================

    ChannelStripProcessorEditor.h
    Created: 24 Jun 2020 11:52:17am
    Author:  Christian Ahrens

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class ProcessorBase;

//==============================================================================
class ProcessorEditorBase : public AudioProcessorEditor, 
                            public Slider::Listener
{
public:
    ProcessorEditorBase(ProcessorBase& processor);
    virtual ~ProcessorEditorBase();

    //==============================================================================
    void paint(Graphics& g) override;
    void resized() override;

    //==============================================================================
    void sliderValueChanged(Slider* slider) override;

protected:
    Label l;
    Slider s;

private:

};

//==============================================================================
class GainProcessorEditor  : public ProcessorEditorBase
{
public:
    GainProcessorEditor(ProcessorBase& processor);
    ~GainProcessorEditor() override;

private:
};

//==============================================================================
class HPFilterProcessorEditor  : public ProcessorEditorBase
{
public:
    HPFilterProcessorEditor(ProcessorBase& processor);
    ~HPFilterProcessorEditor() override;

private:
};

//==============================================================================
class LPFilterProcessorEditor : public ProcessorEditorBase
{
public:
    LPFilterProcessorEditor(ProcessorBase& processor);
    ~LPFilterProcessorEditor() override;

private:
};
