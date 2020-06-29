/*
  ==============================================================================

    ChannelStripProcessorEditor.cpp
    Created: 24 Jun 2020 11:52:17am
    Author:  Christian Ahrens

  ==============================================================================
*/

#include "ChannelStripProcessorEditor.h"
#include "ChannelStripProcessor.h"

ProcessorEditorBase::ProcessorEditorBase(ProcessorBase& processor)
    : AudioProcessorEditor(processor), s(processor.getName())
{
    l.setText(processor.getName(), dontSendNotification);
    addAndMakeVisible(l);
    
    s.setSliderStyle(Slider::Rotary);
    s.setTextBoxStyle(Slider::TextBoxBelow, false, 80, 20);
    s.setRange(processor.getParameterRange(0), processor.getParameterStepWidth(0));
    s.setValue(processor.getParameterDefaultValue(0), dontSendNotification);
    s.addListener(this);
    addAndMakeVisible(s);
}

ProcessorEditorBase::~ProcessorEditorBase()
{
}

void ProcessorEditorBase::resized()
{
    FlexBox fb;
    fb.flexDirection = FlexBox::Direction::column;
    fb.justifyContent = FlexBox::JustifyContent::flexStart;
    fb.items.addArray({
        FlexItem(l).withFlex(1),
        FlexItem(s).withFlex(3)
        });
    fb.performLayout(getLocalBounds().toFloat());
}

void ProcessorEditorBase::sliderValueChanged(Slider* slider)
{
    auto params = processor.getParameters();
    for (auto p : params)
    {
        auto pwi = dynamic_cast<AudioProcessorParameterWithID*>(p);
        if (pwi && pwi->paramID == slider->getName())
            p->setValue(static_cast<float>(slider->getValue()));
    }

    if(auto processorBase = dynamic_cast<ProcessorBase*>(&processor))
        processorBase->updateParameterValues();
}


GainProcessorEditor::GainProcessorEditor(ProcessorBase& processor)
    : ProcessorEditorBase(processor)
{
    s.setName(getProcessorParams().empty() ? "" : getProcessorParams().front().id);
}

GainProcessorEditor::~GainProcessorEditor()
{
}

std::vector<ProcessorEditorBase::ProcessorParam> GainProcessorEditor::getProcessorParams()
{
    return std::vector<ProcessorEditorBase::ProcessorParam>{ {"gain", "Gain", 0.0f, 1.0f, 1.0f} };
}


HPFilterProcessorEditor::HPFilterProcessorEditor(ProcessorBase& processor)
    : ProcessorEditorBase(processor)
{
    s.setName(getProcessorParams().empty() ? "" : getProcessorParams().front().id);
}

HPFilterProcessorEditor::~HPFilterProcessorEditor()
{
}

std::vector<ProcessorEditorBase::ProcessorParam> HPFilterProcessorEditor::getProcessorParams()
{
    return std::vector<ProcessorEditorBase::ProcessorParam>{ {"hpff", "HP filter frequency", 20.0f, 20000.0f, 20.0f} };
}


LPFilterProcessorEditor::LPFilterProcessorEditor(ProcessorBase& processor)
    : ProcessorEditorBase(processor)
{
    s.setName(getProcessorParams().empty() ? "" : getProcessorParams().front().id);
}

LPFilterProcessorEditor::~LPFilterProcessorEditor()
{
}

std::vector<ProcessorEditorBase::ProcessorParam> LPFilterProcessorEditor::getProcessorParams()
{
    return std::vector<ProcessorEditorBase::ProcessorParam>{ {"lpff", "LP filter frequency", 20.0f, 20000.0f, 20000.0f} };
}
