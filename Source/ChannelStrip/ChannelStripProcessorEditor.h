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
    struct ProcessorParam
    {
        ProcessorParam(String i, String n, float min, float max, float def)
        {
            id = i;
            name = n;
            minV = min;
            maxV = max;
            defaultV = def;
        }

        String id;      // parameter ID
        String name;    // parameter name
        float minV;     // minimum value
        float maxV;     // maximum value
        float defaultV; // default value
    };

public:
    ProcessorEditorBase(ProcessorBase& processor);
    virtual ~ProcessorEditorBase();

    //==============================================================================
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

    static std::vector<ProcessorEditorBase::ProcessorParam> getProcessorParams();

private:
};

//==============================================================================
class HPFilterProcessorEditor  : public ProcessorEditorBase
{
public:
    HPFilterProcessorEditor(ProcessorBase& processor);
    ~HPFilterProcessorEditor() override;

    static std::vector<ProcessorEditorBase::ProcessorParam> getProcessorParams();

private:
};

//==============================================================================
class LPFilterProcessorEditor : public ProcessorEditorBase
{
public:
    LPFilterProcessorEditor(ProcessorBase& processor);
    ~LPFilterProcessorEditor() override;

    static std::vector<ProcessorEditorBase::ProcessorParam> getProcessorParams();

private:
};
