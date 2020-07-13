/*
  ==============================================================================

    AudioPlayerTitleTableModel.cpp
    Created: 13 Jul 2020 3:50:56pm
    Author:  Christian Ahrens

  ==============================================================================
*/

#include "AudioPlayerTitleTableModel.h"

AudioPlayerTitleTableModel::AudioPlayerTitleTableModel()
    : TableListBoxModel()
{

}

AudioPlayerTitleTableModel::~AudioPlayerTitleTableModel()
{

}

int AudioPlayerTitleTableModel::getNumRows()
{
	return 20;
}

void AudioPlayerTitleTableModel::paintRowBackground(Graphics& g, int rowNumber, int width, int height, bool rowIsSelected)
{
	ignoreUnused(rowNumber);

	// Selected rows have a different background color.
	if (rowIsSelected)
		g.setColour(Colours::green.brighter());
	else
		g.setColour(Colours::green);
	g.fillRect(0, 0, width, height - 1);

	// Line between rows.
	g.setColour(Colours::green.darker());
	g.fillRect(0, height - 1, width, height - 1);
}

void AudioPlayerTitleTableModel::paintCell(Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected)
{
	ignoreUnused(g);
	ignoreUnused(rowNumber);
	ignoreUnused(columnId);
	ignoreUnused(width);
	ignoreUnused(height);
	ignoreUnused(rowIsSelected);
}