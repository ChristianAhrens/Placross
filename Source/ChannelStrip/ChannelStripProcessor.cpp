/*
  ==============================================================================

    ChannelStripProcessor.cpp
    Created: 24 Jun 2020 11:52:17am
    Author:  Christian Ahrens

  ==============================================================================
*/

#pragma once

#include "ChannelStripProcessor.h"

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

AudioProcessorEditor* ProcessorBase::createEditor() 
{
	return nullptr; 
}

bool ProcessorBase::hasEditor() const 
{
	return false; 
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


OscillatorProcessor30Hz::OscillatorProcessor30Hz()
{
	oscillator.setFrequency(30.0f);
	oscillator.initialise([](float x) { return std::sin(x); });
}

void OscillatorProcessor30Hz::prepareToPlay(double sampleRate, int samplesPerBlock)
{
	dsp::ProcessSpec spec{ sampleRate, static_cast<uint32> (samplesPerBlock) };
	oscillator.prepare(spec);
}

void OscillatorProcessor30Hz::processBlock(AudioSampleBuffer& buffer, MidiBuffer&)
{
	dsp::AudioBlock<float> block(buffer);
	dsp::ProcessContextReplacing<float> context(block);
	oscillator.process(context);
}

void OscillatorProcessor30Hz::reset()
{
	oscillator.reset();
}

const String OscillatorProcessor30Hz::getName() const 
{ 
	return "30HzOscillator"; 
}


OscillatorProcessor440Hz::OscillatorProcessor440Hz()
{
	oscillator.setFrequency(440.0f);
	oscillator.initialise([](float x) { return std::sin(x); });
}

void OscillatorProcessor440Hz::prepareToPlay(double sampleRate, int samplesPerBlock)
{
	dsp::ProcessSpec spec{ sampleRate, static_cast<uint32> (samplesPerBlock) };
	oscillator.prepare(spec);
}

void OscillatorProcessor440Hz::processBlock(AudioSampleBuffer& buffer, MidiBuffer&)
{
	dsp::AudioBlock<float> block(buffer);
	dsp::ProcessContextReplacing<float> context(block);
	oscillator.process(context);
}

void OscillatorProcessor440Hz::reset()
{
	oscillator.reset();
}

const String OscillatorProcessor440Hz::getName() const 
{ 
	return "440HzOscillator"; 
}


OscillatorProcessor2kHz::OscillatorProcessor2kHz()
{
	oscillator.setFrequency(2000.0f);
	oscillator.initialise([](float x) { return std::sin(x); });
}

void OscillatorProcessor2kHz::prepareToPlay(double sampleRate, int samplesPerBlock)
{
	dsp::ProcessSpec spec{ sampleRate, static_cast<uint32> (samplesPerBlock) };
	oscillator.prepare(spec);
}

void OscillatorProcessor2kHz::processBlock(AudioSampleBuffer& buffer, MidiBuffer&)
{
	dsp::AudioBlock<float> block(buffer);
	dsp::ProcessContextReplacing<float> context(block);
	oscillator.process(context);
}

void OscillatorProcessor2kHz::reset()
{
	oscillator.reset();
}

const String OscillatorProcessor2kHz::getName() const 
{ 
	return "2kHzOscillator"; 
}


GainProcessor::GainProcessor()
{
	gain.setGainDecibels(-6.0f);
}

void GainProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
	dsp::ProcessSpec spec{ sampleRate, static_cast<uint32> (samplesPerBlock), 2 };
	gain.prepare(spec);
}

void GainProcessor::processBlock(AudioSampleBuffer& buffer, MidiBuffer&)
{
	dsp::AudioBlock<float> block(buffer);
	dsp::ProcessContextReplacing<float> context(block);
	gain.process(context);
}

void GainProcessor::reset()
{
	gain.reset();
}

const String GainProcessor::getName() const { return "Gain"; }


HPFilterProcessor::HPFilterProcessor() {}

void HPFilterProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
	*filter.state = *dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, 1000.0f);

	dsp::ProcessSpec spec{ sampleRate, static_cast<uint32> (samplesPerBlock), 2 };
	filter.prepare(spec);
}

void HPFilterProcessor::processBlock(AudioSampleBuffer& buffer, MidiBuffer&)
{
	dsp::AudioBlock<float> block(buffer);
	dsp::ProcessContextReplacing<float> context(block);
	filter.process(context);
}

void HPFilterProcessor::reset()
{
	filter.reset();
}

const String HPFilterProcessor::getName() const
{ 
	return "HPFilter"; 
}


LPFilterProcessor::LPFilterProcessor()
{
}

void LPFilterProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
	*filter.state = *dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, 1000.0f);

	dsp::ProcessSpec spec{ sampleRate, static_cast<uint32> (samplesPerBlock), 2 };
	filter.prepare(spec);
}

void LPFilterProcessor::processBlock(AudioSampleBuffer& buffer, MidiBuffer&)
{
	dsp::AudioBlock<float> block(buffer);
	dsp::ProcessContextReplacing<float> context(block);
	filter.process(context);
}

void LPFilterProcessor::reset()
{
	filter.reset();
}

const String LPFilterProcessor::getName() const 
{ 
	return "LPFilter"; 
}
