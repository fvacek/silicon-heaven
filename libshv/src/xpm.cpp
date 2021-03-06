#include "xpm.h"

#include <vector>
#include <iostream>

namespace shv {

namespace {
/*
std::vector<std::string> split(const std::string& s, char separator)
{
	std::vector<std::string> ret;
	std::string::size_type prev_pos = 0, pos = 0;
	while((pos = s.find(separator, pos)) != std::string::npos)
	{
		std::string substring(s.substr(prev_pos, pos-prev_pos) );
		if(substring.length() > 0)
			ret.push_back(substring);
		prev_pos = ++pos;
	}
	std::basic_string substring = s.substr(prev_pos, pos-prev_pos);
	if(substring.length() > 0)
		ret.push_back(substring); // Last word
	return ret;
}
*/
int skip_white_space(const std::string& s, std::size_t ix)
{
	while(ix < s.length() && s[ix] <= ' ')
		ix++;
	return ix;
}
/*
int find_white_space(const std::string& s, std::size_t ix)
{
	while(ix < s.length() && s[ix] > ' ')
		ix++;
	return ix;
}
*/
std::string ltrim(const std::string& s)
{
	int ix = skip_white_space(s, 0);
	return s.substr(ix);
}
}

Xpm::Color Xpm::Color::Black = Xpm::Color::fromRGB(0x00, 0x00, 0x00);
Xpm::Color Xpm::Color::White = Xpm::Color::fromRGB(0xff, 0xff, 0xff);
Xpm::Color Xpm::Color::Red = Xpm::Color::fromRGB(0xff, 0x00, 0x00);
Xpm::Color Xpm::Color::Green = Xpm::Color::fromRGB(0x00, 0xff, 0x00);
Xpm::Color Xpm::Color::Blue = Xpm::Color::fromRGB(0x00, 0x00, 0xff);
Xpm::Color Xpm::Color::Magenta = Xpm::Color::fromRGB(0xff, 0x00, 0xff);
Xpm::Color Xpm::Color::Cyan = Xpm::Color::fromRGB(0x00, 0xff, 0xff);
Xpm::Color Xpm::Color::Yellow = Xpm::Color::fromRGB(0xff, 0xff, 0x00);

Xpm::Color Xpm::Color::fromString(const std::string &def)
{
	if(def.at(0) == '#') {
		std::string s = def.substr(1); // skip #
		int rgb = std::stoi(s, nullptr, 16);
		Color color = Color::fromRGB(rgb >> 16, (rgb >> 8) % 256, rgb % 256);
		return color;
	}
	else if(def == "None") {
		return Color();
	}
	throw std::runtime_error("bad color format: " + def);
	return Color();
}

//============================================================
// Xpm
//============================================================
Xpm::Xpm()
{
}

// https://en.wikipedia.org/wiki/X_PixMap
void Xpm::wrapData(const char *data[])
{
	int line_ix = 0;
	std::string header(data[line_ix++]);

	header = ltrim(header);
	std::size_t ix;
	m_width = std::stoi(header, &ix);

	header = ltrim(header.substr(ix));
	m_height = std::stoi(header, &ix);

	header = ltrim(header.substr(ix));
	int n_colors = std::stoi(header, &ix);

	header = ltrim(header.substr(ix));
	m_colorKeyLength = std::stoi(header, &ix);
	if(m_colorKeyLength > sizeof(ColorKey))
		throw std::runtime_error(std::string("color key too long"));

	for (int i = 0; i < n_colors; ++i) {
		std::string s(data[line_ix++]);
		ColorKey ck = colorKeyAt(s.c_str(), m_colorKeyLength);
		ix = skip_white_space(s, m_colorKeyLength);
		char c_type = s.at(ix++);
		if(c_type != 'c')
			throw std::runtime_error("only 'c' type of color definiton is supported");
		ix = skip_white_space(s, ix);
		s = s.substr(ix);
		Color color = Color::fromString(s);
		m_colors[ck] = color;
	}
	m_pixels = data + line_ix;
}

Xpm::ColorKey Xpm::colorKeyAt(const char *pc, int key_len)
{
	ColorKey ck = 0;
	for (int j = 0; j < key_len; ++j) {
		if(j > 0)
			ck <<= 8;
		ck = ck | (uint8_t)pc[j];
	}
	return ck;
}

const Xpm::Color &Xpm::colorAt(int row, int col) const
{
	const char *pc = m_pixels[row] + (col * m_colorKeyLength);
	ColorKey ck = colorKeyAt(pc, m_colorKeyLength);
	return m_colors.at(ck);
}

//============================================================
// XpmAtlas
//============================================================

void XpmAtlas::wrapData(const char *data[], int data_size)
{
	Super::wrapData(data);
	int line_ix = 1 + m_colors.size() + m_height;
	for (; line_ix < data_size; ++line_ix) {
		const char *line = data[line_ix];
		std::string s(line);
		std::string::size_type ix = s.find('=');
		if(ix != std::string::npos) {
			std::string key = s.substr(0, ix);
			std::string val = s.substr(ix+1);
			//std::cerr << key << "->" << val << std::endl;
			if(key == "glyphWidth")
				m_glyphWidth = std::stoi(val);
			else if(key == "glyphHeight")
				m_glyphHeight = std::stoi(val);
		}
	}
}

//============================================================
// XpmAtlas
//============================================================
void XpmFont::wrapData(const char *data[], int data_size)
{
	Super::wrapData(data, data_size);
	int line_ix = 1 + m_colors.size() + m_height;
	for (; line_ix < data_size; ++line_ix) {
		const char *line = data[line_ix];
		std::string s(line);
		std::string::size_type ix = s.find('=');
		if(ix != std::string::npos) {
			std::string key = s.substr(0, ix);
			std::string val = s.substr(ix+1);
			//std::cerr << key << "->" << val << std::endl;
			if(key == "firstGlyphAscii")
				m_firstGlyphAscii = std::stoi(val);
		}
	}
}

}
