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

void AudioPlayerTitleTableModel::setCellColours(const Colour& normalColour, const Colour& highlightColour, const Colour& lineColour, const Colour& textColour)
{
	m_normalColour = normalColour;
	m_highlightColour = highlightColour;
	m_lineColour = lineColour;
	m_textColour = textColour;
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
	ignoreUnused(rowIsSelected);

	auto cellRect = Rectangle<int>(width, height);

	auto mapKey = rowNumber + 1;

	if (m_IdTitleKV.count(mapKey) > 0)
	{
		String cellText;

		if (columnId == 1)
		{
			File f(m_IdTitleKV.at(mapKey).first);

			cellText = f.getFileName();
		}
		else if (columnId == 2)
		{
			double secondsInMilliPrecision = m_IdTitleKV.at(mapKey).second * 0.001f;
			RelativeTime position(secondsInMilliPrecision);

			auto hours = ((int)position.inHours()) % 60;
			auto minutes = ((int)position.inMinutes()) % 60;
			auto seconds = ((int)position.inSeconds()) % 60;
			auto millis = ((int)position.inMilliseconds()) % 1000;

			cellText = String::formatted("%02d:%02d:%02d:%03d", hours, minutes, seconds, millis);
		}
		else
		{
			cellText = String();
		}

		g.setColour(m_textColour);
		g.drawFittedText(cellText, cellRect, Justification::centredLeft, 1);
	}
}

void AudioPlayerTitleTableModel::cellClicked (int rowNumber, int columnId, const MouseEvent& e)
{
    ignoreUnused(columnId);
    ignoreUnused(e);
    
    auto mapKey = rowNumber + 1;
    if (m_IdTitleKV.count(mapKey) > 0)
    {
        auto titleName = m_IdTitleKV.at(mapKey).first;
        titleSelected(titleName);
    }
}

void AudioPlayerTitleTableModel::cellDoubleClicked (int rowNumber, int columnId, const MouseEvent& e)
{
    ignoreUnused(rowNumber);
    ignoreUnused(columnId);
    ignoreUnused(e);
}

std::vector<int> AudioPlayerTitleTableModel::addTitles(const std::vector<std::pair<std::string, int>> &titles)
{
    std::vector<int> rowNumbers;
	for (auto const& title : titles)
        rowNumbers.push_back (addTitle(title));
    
    return rowNumbers;
}

int AudioPlayerTitleTableModel::addTitle(const std::pair<std::string, int> &title)
{
	for(auto const & titleKV : m_IdTitleKV)
    {
		if(titleKV.second == title)
        {
            return (titleKV.first - 1);
        }
    }

    int titleId = getNextTitleId();
	m_IdTitleKV.insert(std::make_pair(titleId, title));
    int rowNumber = (titleId - 1);
    
    return rowNumber;
}

int AudioPlayerTitleTableModel::getNextTitleId()
{
	return ++m_TitleId;
}

bool AudioPlayerTitleTableModel::selectNextTitle(const SparseSet<int>& currentSelectedRows)
{
    jassert(currentSelectedRows.size() == 1); // we only support single selection at the moment
    
    auto mapKey = currentSelectedRows[0] + 1;
    auto nextMapKey = mapKey + 1;
    
    if (m_IdTitleKV.count(nextMapKey) == 1)
    {
        auto titleName = m_IdTitleKV.at(nextMapKey).first;
        titleSelected(titleName);
        return true;
    }
    else
        return false;
}

bool AudioPlayerTitleTableModel::selectPrevTitle(const SparseSet<int>& currentSelectedRows)
{
    jassert(currentSelectedRows.size() == 1); // we only support single selection at the moment
    
    auto mapKey = currentSelectedRows[0] + 1;
    auto prevMapKey = mapKey - 1;
    
    if (m_IdTitleKV.count(prevMapKey) == 1)
    {
        auto titleName = m_IdTitleKV.at(prevMapKey).first;
        titleSelected(titleName);
        return true;
    }
    else
        return false;
}
