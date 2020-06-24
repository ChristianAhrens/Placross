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

void ProcessorEditorBase::paint(Graphics& g)
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
    processor.setParameter(0, slider->getValue());
}


GainProcessorEditor::GainProcessorEditor(ProcessorBase& processor)
    : ProcessorEditorBase(processor)
{
}

GainProcessorEditor::~GainProcessorEditor()
{
}


HPFilterProcessorEditor::HPFilterProcessorEditor(ProcessorBase& processor)
    : ProcessorEditorBase(processor)
{
}

HPFilterProcessorEditor::~HPFilterProcessorEditor()
{
}


LPFilterProcessorEditor::LPFilterProcessorEditor(ProcessorBase& processor)
    : ProcessorEditorBase(processor)
{
}

LPFilterProcessorEditor::~LPFilterProcessorEditor()
{
}
