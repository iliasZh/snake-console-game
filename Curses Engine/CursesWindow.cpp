#include "CursesWindow.h"
#include "Utilities.h"
#include <sstream>

curses::Curses::Window::~Window()
{
	// clear box and contents
	//std::string fill(width, ' ');
	//for (int i = 0; i < height; ++i)
	//{
	//	mvwprintw(win, i, 0, fill.c_str()); // win, y, x, str
	//}
	//wrefresh(win);
	//wborder(win, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
	Clear();
	delwin(win);
}

void curses::Curses::Window::DrawBox(Color fg, Color bg)
{
	const bool isDefault = (fg == Color::White) && (bg == Color::Black);
	chtype colorPair = GetColorPair(fg, bg);
	
	if (isDefault)
	{
		box(win, 0, 0);
	}
	else
	{
		wattron(win, colorPair);
		box(win, 0, 0);
		wattroff(win, colorPair);
	}

	wrefresh(win);
}

void curses::Curses::Window::Refresh()
{
	wrefresh(win);
}

void curses::Curses::Window::Touch()
{
	touchwin(win);
}

void curses::Curses::Window::Write(int x, int y, std::u8string str, Color fg, Color bg)
{
	assert(x >= 0 && x < width);
	assert(y >= 0 && y < height);
	assert(count_codepoints(str) < width - x); // may not work properly, i don't know shit about unicode

	const bool isDefault = (fg == Color::White) && (bg == Color::Black);
	chtype colorPair = GetColorPair(fg, bg);

	if (isDefault)
	{
		mvwprintw(win, y, x, reinterpret_cast<const char*>(str.c_str()));
	}
	else
	{
		wattron(win, colorPair);
		mvwprintw(win, y, x, reinterpret_cast<const char*>(str.c_str()));
		wattroff(win, colorPair);
	}
}

void curses::Curses::Window::GetCh()
{
	flushinp();
	wgetch(win);
}

void curses::Curses::Window::Clear()
{
	wclear(win);
}

int curses::Curses::Window::GetCursorX()
{
	return getcurx(win);
}

int curses::Curses::Window::GetCursorY()
{
	return getcury(win);
}

curses::Curses::Curses()
{
	if (++instances == 1)
	{
		initscr();
		SetEchoMode(echoEnabled);
		SetCursorMode(cursorMode);
		if (HasColors())
		{
			start_color();
			for (int i = 0; i < numOfColors; ++i) // foreground
			{
				for (int j = 0; j < numOfColors; ++j) // background
				{
					init_pair(i * numOfColors + j, i, j);
				}
			}
		}
		else
		{
			THROW_CURSES_EXCEPTION("Curses constructor", "your terminal does not support colors");
		}
	}
	else
	{
		THROW_CURSES_EXCEPTION("Curses constructor", "attempting to create more than one instance of Curses");
	}
}

curses::Curses::~Curses()
{
	--instances;
	endwin();
}

void curses::Curses::AddWindow(std::string name, int startX, int startY, int width, int height)
{
	// the only method to construct in-place that seems to be working

	/*
	A container's emplace member constructs an element using the supplied arguments.

	The value_type of your map is std::pair<const int, Foo> and that type has no constructor
	taking the arguments{ 5, 5, 'a', 'b' } i.e. this wouldn't work:

	std::pair<const int, Foo> value{ 5, 5, 'a', 'b' };
	map.emplace(value);

	You need to call emplace with arguments that match one of pair's constructors.

	With a conforming C++11 implementation you can use :

	mymap.emplace(std::piecewise_construct, std::make_tuple(5), std::make_tuple(5, 'a', 'b'));
	*/

	auto res = windows.emplace
	(
		std::piecewise_construct,
		std::make_tuple(name),
		std::make_tuple(startX, startY, width, height)
	);

	if (!(res.second)) // element already exists
	{
		std::stringstream ss;
		ss << "element \"" << name << "\" already exists.\n"
			<< "Use operator[ ] to get a reference";
		THROW_CURSES_EXCEPTION("Curses::AddWindow()", ss.str());
	}

	return;
}

curses::Curses::Window& curses::Curses::operator[](std::string name)
{
    auto it = windows.find(name);	// returns windows.end() if not found
    if (it == windows.end())
    {
        std::stringstream ss;
        ss << '\"' << name << '\"' << " window does not exist";
        THROW_CURSES_EXCEPTION("Curses::operator[ ]", ss.str());
    }
    else
    {
        return it->second;
    }
}

bool curses::Curses::DeleteWindow(std::string name)
{
	return (windows.erase(name) == 1);
}

void curses::Curses::SetCursorMode(CursorMode mode)
{
	cursorMode = mode;
	curs_set((int)mode);
}

void curses::Curses::SetEchoMode(bool enable)
{
	if (enable)
		echo();
	else
		noecho();

	echoEnabled = enable;
}

chtype curses::Curses::GetColorPair(Color f, Color b)
{
	return COLOR_PAIR((int)f * numOfColors + (int)b);
}
