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

void AudioPlayerTitleTableModel::setRowBackgroundColours(Colour& normalColour, Colour& highlightColour, Colour& lineColour)
{
	m_normalColour = normalColour;
	m_highlightColour = highlightColour;
	m_lineColour = lineColour;
}

int AudioPlayerTitleTableModel::getNumRows()
{
	return static_cast<int>(m_IdTitleKV.size());
}

void AudioPlayerTitleTableModel::paintRowBackground(Graphics& g, int rowNumber, int width, int height, bool rowIsSelected)
{
	ignoreUnused(rowNumber);

	// Selected rows have a different background color.
	if (rowIsSelected)
		g.setColour(m_highlightColour);
	else
		g.setColour(m_normalColour);
	g.fillRect(0, 0, width, height - 1);

	// Line between rows.
	g.setColour(m_lineColour);
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

void AudioPlayerTitleTableModel::addTitles(const std::vector<std::pair<std::string, int>> &titles)
{
	for (auto const& title : titles)
		addTitle(title);
}

void AudioPlayerTitleTableModel::addTitle(const std::pair<std::string, int> &title)
{
	for(auto const & titleKV : m_IdTitleKV)
		if(titleKV.second == title)
			return;

	m_IdTitleKV.insert(std::make_pair(getNextTitleId(), title));
}

int AudioPlayerTitleTableModel::getNextTitleId()
{
	return ++m_TitleId;
}