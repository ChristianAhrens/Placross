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

void ProcessorBase::initParameters()
{
	for (auto param : getProcessorParams())
	{
		auto newAPF = new AudioParameterFloat(
			param.id,
			param.name,
			param.minV,
			param.maxV,
			param.defaultV);
		newAPF->setValueNotifyingHost(getNormalizedValue(newAPF));
		newAPF->addListener(this);
		addParameter(newAPF);
		m_IdToIdxMap.insert(std::make_pair(param.id, newAPF->getParameterIndex()));
	}
}

void ProcessorBase::parameterGestureChanged(int parameterIndex, bool gestureIsStarting)
{
	ignoreUnused(parameterIndex);
	ignoreUnused(gestureIsStarting);
}

AudioProcessorEditor* ProcessorBase::createEditor()
{
	auto editor = std::make_unique<ChannelStripProcessorEditor>(*this);
	editor->setSize(50, 60);

	return editor.release();
}

bool ProcessorBase::hasEditor() const
{
	return true;
}

float ProcessorBase::getMappedValue(AudioProcessorParameter* param)
{
	auto floatParam = dynamic_cast<AudioParameterFloat*>(param);
	if (floatParam)
	{
		auto normalizedVal = param->getValue();
		auto minVal = floatParam->getNormalisableRange().getRange().getStart();
		auto maxVal = floatParam->getNormalisableRange().getRange().getEnd();

		return normalizedVal * (maxVal - minVal) + minVal;
	}
	else
		return 0.0f;
}

float ProcessorBase::getNormalizedValue(AudioProcessorParameter* param)
{
	auto floatParam = dynamic_cast<AudioParameterFloat*>(param);
	if (floatParam)
	{
		auto normalizedVal = param->getValue();
		//auto minVal = floatParam->getNormalisableRange().getRange().getStart();
		//auto maxVal = floatParam->getNormalisableRange().getRange().getEnd();

		return normalizedVal;
	}
	else
		return 0.0f;
}


GainProcessor::GainProcessor()
	: ProcessorBase()
{
	initParameters();
}

std::vector<ProcessorBase::ProcessorParam> GainProcessor::getProcessorParams()
{
	return std::vector<ProcessorBase::ProcessorParam>{ {"gain", "Gain", 0.0f, 1.0f, 1.0f} };
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

void GainProcessor::parameterValueChanged(int parameterIndex, float newValue)
{
	if (parameterIndex == m_IdToIdxMap.at("gain"))
	{
		//DBG(String(__FUNCTION__) + " gain " + String(newValue));
		m_gain.setGainLinear(newValue);

		dsp::ProcessSpec spec{ m_sampleRate, static_cast<uint32> (m_samplesPerBlock), 1 };
		m_gain.prepare(spec);
	}
}

void GainProcessor::updateParameterValues()
{
	auto idx = m_IdToIdxMap.at("gain");
	parameterValueChanged(idx, getMappedValue(getParameters().getUnchecked(idx)));
}

const String GainProcessor::getName() const 
{ 
	return "Gain"; 
}


HPFilterProcessor::HPFilterProcessor()
	: ProcessorBase()
{
	initParameters();
}

std::vector<ProcessorBase::ProcessorParam> HPFilterProcessor::getProcessorParams()
{
	return std::vector<ProcessorBase::ProcessorParam>{ {"hpff", "Highpass freq.", 20.0f, 20000.0f, 20.0f} };
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

void HPFilterProcessor::parameterValueChanged(int parameterIndex, float newValue)
{
	if (parameterIndex == m_IdToIdxMap.at("hpff"))
	{
		//DBG(String(__FUNCTION__) + " hpff " + String(newValue));
		*m_filter.state = *dsp::IIR::Coefficients<float>::makeHighPass(m_sampleRate, newValue);

		dsp::ProcessSpec spec{ m_sampleRate, static_cast<uint32> (m_samplesPerBlock), 1 };
		m_filter.prepare(spec);
	}
}

void HPFilterProcessor::updateParameterValues()
{
	auto idx = m_IdToIdxMap.at("hpff");
	parameterValueChanged(idx, getMappedValue(getParameters().getUnchecked(idx)));
}

const String HPFilterProcessor::getName() const
{ 
	return "HPFilter"; 
}


LPFilterProcessor::LPFilterProcessor()
	: ProcessorBase()
{
	initParameters();
}

std::vector<ProcessorBase::ProcessorParam> LPFilterProcessor::getProcessorParams()
{
	return std::vector<ProcessorBase::ProcessorParam>{ {"lpff", "Lowpass freq.", 20.0f, 20000.0f, 20000.0f} };
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

void LPFilterProcessor::parameterValueChanged(int parameterIndex, float newValue)
{
	if (parameterIndex == m_IdToIdxMap.at("lpff"))
	{
		//DBG(String(__FUNCTION__) + " lpff " + String(newValue));
		*m_filter.state = *dsp::IIR::Coefficients<float>::makeLowPass(m_sampleRate, newValue);

		dsp::ProcessSpec spec{ m_sampleRate, static_cast<uint32> (m_samplesPerBlock), 1 };
		m_filter.prepare(spec);
	}
}

void LPFilterProcessor::updateParameterValues()
{
	auto idx = m_IdToIdxMap.at("lpff");
	parameterValueChanged(idx, getMappedValue(getParameters().getUnchecked(idx)));
}

const String LPFilterProcessor::getName() const 
{ 
	return "LPFilter"; 
}
