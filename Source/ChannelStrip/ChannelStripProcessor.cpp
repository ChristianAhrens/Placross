/*
  ==============================================================================

    ChannelStripProcessor.cpp
    Created: 24 Jun 2020 11:52:17am
    Author:  Christian Ahrens

  ==============================================================================
*/

#include "ChannelStripProcessor.h"
#include "ChannelStripProcessorEditor.h"

ProcessorBase::ProcessorBase()
{
}

void ProcessorBase::prepareToPlay(double, int)
{
}

void ProcessorBase::releaseResources() 
{
}

void ProcessorBase::processBlock(AudioSampleBuffer&, MidiBuffer&) 
{
}

const String ProcessorBase::getName() const 
{
	return {}; 
}

bool ProcessorBase::acceptsMidi() const 
{
	return false; 
}

bool ProcessorBase::producesMidi() const 
{
	return false; 
}

double ProcessorBase::getTailLengthSeconds() const 
{
	return 0; 
}

int ProcessorBase::getNumPrograms() 
{ 
	return 0; 
}

int ProcessorBase::getCurrentProgram() 
{ 
	return 0; 
}

void ProcessorBase::setCurrentProgram(int) 
{
}

const String ProcessorBase::getProgramName(int) 
{ 
	return {}; 
}

void ProcessorBase::changeProgramName(int, const String&) 
{
}

void ProcessorBase::getStateInformation(MemoryBlock&) 
{
}

void ProcessorBase::setStateInformation(const void*, int)
{
}

juce::Range<double> ProcessorBase::getParameterRange(int parameterIndex)
{
	for (auto parameterNode : getParameterTree())
		if (parameterNode->getParameter()->getParameterIndex() == parameterIndex)
		{
			AudioParameterFloat* floatParam = dynamic_cast<AudioParameterFloat*>(parameterNode->getParameter());
			if (floatParam)
			{
				auto rangef = floatParam->getNormalisableRange().getRange();
				return juce::Range<double>(rangef.getStart(), rangef.getEnd());
			}
		}
	
	return juce::Range<double>{};
}

double ProcessorBase::getParameterStepWidth(int parameterIndex)
{
	for (auto parameterNode : getParameterTree())
		if (parameterNode->getParameter()->getParameterIndex() == parameterIndex)
		{
			AudioParameterFloat* floatParam = dynamic_cast<AudioParameterFloat*>(parameterNode->getParameter());
			if (floatParam)
			{
				return floatParam->getNormalisableRange().interval;
			}
		}

	return double{};
}


GainProcessor::GainProcessor()
{
	addParameter(m_gainValue = new AudioParameterFloat(
		"gain", // parameter ID
		"Gain", // parameter name
		0.0f,   // minimum value
		1.0f,   // maximum value
		1.0f)); // default value

	m_gain.setGainLinear(*m_gainValue);
}

void GainProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
	dsp::ProcessSpec spec{ sampleRate, static_cast<uint32> (samplesPerBlock), 2 };
	m_gain.prepare(spec);
}

void GainProcessor::processBlock(AudioSampleBuffer& buffer, MidiBuffer&)
{
	m_gain.setGainDecibels(*m_gainValue);

	dsp::AudioBlock<float> block(buffer);
	dsp::ProcessContextReplacing<float> context(block);
	m_gain.process(context);
}

void GainProcessor::reset()
{
	m_gain.reset();
}

AudioProcessorEditor* GainProcessor::createEditor()
{
	auto editor = std::make_unique<GainProcessorEditor>(*this);
	editor->setSize(50, 60);

	return editor.release();
}

bool GainProcessor::hasEditor() const
{
	return true;
}

const String GainProcessor::getName() const 
{ 
	return "Gain"; 
}


HPFilterProcessor::HPFilterProcessor() 
{
	addParameter(m_filterValue = new AudioParameterFloat(
		"hpf", // parameter ID
		"HPFilter", // parameter name
		20.0f,   // minimum value
		20000.0f,   // maximum value
		20.0f)); // default value

}

void HPFilterProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
	*m_filter.state = *dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, *m_filterValue);

	dsp::ProcessSpec spec{ sampleRate, static_cast<uint32> (samplesPerBlock), 2 };
	m_filter.prepare(spec);
}

void HPFilterProcessor::processBlock(AudioSampleBuffer& buffer, MidiBuffer&)
{
	dsp::AudioBlock<float> block(buffer);
	dsp::ProcessContextReplacing<float> context(block);
	m_filter.process(context);
}

void HPFilterProcessor::reset()
{
	m_filter.reset();
}

AudioProcessorEditor* HPFilterProcessor::createEditor()
{
	auto editor = std::make_unique<HPFilterProcessorEditor>(*this);
	editor->setSize(50, 60);

	return editor.release();
}

bool HPFilterProcessor::hasEditor() const
{
	return true;
}

const String HPFilterProcessor::getName() const
{ 
	return "HPFilter"; 
}


LPFilterProcessor::LPFilterProcessor()
{
	addParameter(m_filterValue = new AudioParameterFloat(
		"lpf", // parameter ID
		"LPFilter", // parameter name
		20.0f,   // minimum value
		20000.0f,   // maximum value
		20000.0f)); // default value
}

void LPFilterProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
	*m_filter.state = *dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, *m_filterValue);

	dsp::ProcessSpec spec{ sampleRate, static_cast<uint32> (samplesPerBlock), 2 };
	m_filter.prepare(spec);
}

void LPFilterProcessor::processBlock(AudioSampleBuffer& buffer, MidiBuffer&)
{
	dsp::AudioBlock<float> block(buffer);
	dsp::ProcessContextReplacing<float> context(block);
	m_filter.process(context);
}

void LPFilterProcessor::reset()
{
	m_filter.reset();
}

AudioProcessorEditor* LPFilterProcessor::createEditor()
{
	auto editor = std::make_unique<LPFilterProcessorEditor>(*this);
	editor->setSize(50, 60);

	return editor.release();
}

bool LPFilterProcessor::hasEditor() const
{
	return true;
}

const String LPFilterProcessor::getName() const 
{ 
	return "LPFilter"; 
}
