#pragma once

#include "CursesWindow.h"

class UIWindow : public curses::Curses::Window
{
public:
	typedef curses::Curses::Window Window;
	typedef curses::Curses::Color Color;
public:
	using Window::Window;
	void WriteCentered(int line, std::u8string text, Color fg = Color::White, Color bg = Color::Black)
	{
		const int size = (int)count_codepoints(text);
		assert(size <= Width() - 2);
		Write(Width() / 2 - size / 2, line, text, fg, bg);
	}
	void AddButton(std::u8string text)
	{
		assert(count_codepoints(text) <= Width() - 2 * buttonMargin - 2);
		buttons.emplace_back(std::move(text));

		if (buttons.size() == 1) currButton = 0;
	}
	void ChangeButton(int index, std::u8string newText)
	{
		assert(index >= 0 && index < (int)buttons.size());

		buttons[index] = newText;
	}
	void DrawButtons()
	{
		if (!buttons.empty())
		{
			int currLine = buttonsStartLine;

			int maxTextLen = 0;
			for (const auto& text : buttons)
			{
				const int currTextLen = (int)count_codepoints(text);
				if (maxTextLen < currTextLen) maxTextLen = currTextLen;
			}

			for (int i = 0; i < buttons.size(); ++i, currLine += (buttonSpacing + 1))
			{
				const int lenDiff = maxTextLen - (int)count_codepoints(buttons[i]);

				const int lmlen = buttonMargin + lenDiff / 2;			// left margin len
				const int rmlen = buttonMargin + lenDiff - lenDiff / 2;	// right margin len

				std::u8string marginL(lmlen - 1, ' ');
				std::u8string marginR(rmlen - 1, ' ');

				if (i != currButton)
				{
					std::u8string text = marginL + u8' ' + buttons[i] + u8' ' + marginR;
					WriteCentered(currLine, text, buttonText, buttonBase);
				}
				else
				{
					std::u8string text = marginL + u8'>' + buttons[i] + u8'<' + marginR;
					WriteCentered(currLine, text, buttonTextHighlight, buttonHighlight);
				}
			}
		}
		Refresh();
	}
	void Center()
	{
		int menuHeight = (int)buttons.size() * (buttonSpacing + 1) - buttonSpacing;
		buttonsStartLine = Height() / 2 - menuHeight / 2;
	}
	int GetUpperLine() const
	{
		return buttonsStartLine;
	}
	int GetLowerLine() const
	{
		if (buttons.empty())
			return buttonsStartLine;
		else
			return buttonsStartLine + ((int)buttons.size() - 1) * (buttonSpacing + 1);
	}
	void ShiftStartLine(int offset)
	{
		buttonsStartLine += offset;
		assert(GetUpperLine() >= 0 && GetLowerLine() < Height());
	}
	void OnButtonNext()
	{
		assert(!buttons.empty());
		if (++currButton == (int)buttons.size())
		{
			if (bWrapping)
				currButton = 0;
			else
				currButton = (int)buttons.size() - 1;
		}
	}
	void OnButtonPrev()
	{
		assert(!buttons.empty());
		if (--currButton == -1)
		{
			if (bWrapping)
				currButton = (int)buttons.size() - 1;
			else
				currButton = 0;
		}
	}
	void SetWrappingMode(bool wrap) { bWrapping = wrap; }
	int CurrentButtonIndex()
	{
		return currButton;
	}
	void SetButtonSpacing(int spacing)
	{
		assert(spacing >= 0);
		buttonSpacing = spacing;
	}
	void SetCurrButton(int pos)
	{
		assert(!buttons.empty());
		assert(pos >= 0 && pos < (int)buttons.size());
		currButton = 0;
	}
private:
	std::vector<std::u8string> buttons;
	int buttonsStartLine = 2;				// starting line for buttons
	int buttonSpacing = 0;					// lines in between buttons
	int currButton = -1;
	int buttonMargin = 2;
	Color buttonText = Color::Black;
	Color buttonTextHighlight = Color::White;
	Color buttonHighlight = Color::Green;
	Color buttonBase = Color::Cyan;
	bool bWrapping = false;
};