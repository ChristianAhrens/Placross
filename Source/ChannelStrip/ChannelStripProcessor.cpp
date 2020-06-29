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

void ProcessorBase::prepareToPlay(double sampleRate, int samplesPerBlock)
{
	m_sampleRate = sampleRate;
	m_samplesPerBlock = samplesPerBlock;

	updateParameterValues();
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
	: ProcessorBase()
{
	initParameters();
}

void GainProcessor::processBlock(AudioSampleBuffer& buffer, MidiBuffer&)
{
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

void GainProcessor::updateParameterValues()
{
	m_gain.setGainLinear(getParameters().getUnchecked(m_IdToIdxMap.at("gain"))->getValue());

	dsp::ProcessSpec spec{ m_sampleRate, static_cast<uint32> (m_samplesPerBlock), 2 };
	m_gain.prepare(spec);
}

const String GainProcessor::getName() const 
{ 
	return "Gain"; 
}

void GainProcessor::initParameters()
{
	for (auto param : GainProcessorEditor::getProcessorParams())
	{
		auto newAPF = new AudioParameterFloat(
			param.id,
			param.name,
			param.minV,
			param.maxV,
			param.defaultV);
		newAPF->setValueNotifyingHost(param.defaultV);
		addParameter(newAPF);
		m_IdToIdxMap.insert(std::make_pair(param.id, newAPF->getParameterIndex()));
	}
}


HPFilterProcessor::HPFilterProcessor()
	: ProcessorBase()
{
	initParameters();
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

void HPFilterProcessor::updateParameterValues()
{
	*m_filter.state = *dsp::IIR::Coefficients<float>::makeHighPass(m_sampleRate, getParameters().getUnchecked(m_IdToIdxMap.at("hpff"))->getValue());

	dsp::ProcessSpec spec{ m_sampleRate, static_cast<uint32> (m_samplesPerBlock), 2 };
	m_filter.prepare(spec);
}

const String HPFilterProcessor::getName() const
{ 
	return "HPFilter"; 
}

void HPFilterProcessor::initParameters()
{
	for (auto param : HPFilterProcessorEditor::getProcessorParams())
	{
		auto newAPF = new AudioParameterFloat(
			param.id,
			param.name,
			param.minV,
			param.maxV,
			param.defaultV);
		newAPF->setValueNotifyingHost(param.defaultV);
		addParameter(newAPF);
		m_IdToIdxMap.insert(std::make_pair(param.id, newAPF->getParameterIndex()));
	}
}


LPFilterProcessor::LPFilterProcessor()
	: ProcessorBase()
{
	initParameters();
}

void LPFilterProcessor::processBlock(AudioSampleBuffer& buffer, MidiBuffer&)
{
	return;
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

void LPFilterProcessor::updateParameterValues()
{
	*m_filter.state = *dsp::IIR::Coefficients<float>::makeLowPass(m_sampleRate, getParameters().getUnchecked(m_IdToIdxMap.at("lpff"))->getValue());
	
	dsp::ProcessSpec spec{ m_sampleRate, static_cast<uint32> (m_samplesPerBlock), 2 };
	m_filter.prepare(spec);
}

const String LPFilterProcessor::getName() const 
{ 
	return "LPFilter"; 
}

void LPFilterProcessor::initParameters()
{
	for (auto param : LPFilterProcessorEditor::getProcessorParams())
	{
		auto newAPF = new AudioParameterFloat(
			param.id,
			param.name,
			param.minV,
			param.maxV,
			param.defaultV);
		newAPF->setValueNotifyingHost(param.defaultV);
		addParameter(newAPF);
		m_IdToIdxMap.insert(std::make_pair(param.id, newAPF->getParameterIndex()));
	}
}
