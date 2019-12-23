#include "vex.h"
#include "uilib.h"

#include <functional>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <sstream>
#include <memory>

using namespace std;
using namespace vex;
using namespace uilib;

/*-----------------------
        Display
-----------------------*/

color Display::backgroundColor = color::black;
vector<Element*> Display::elements;
function<void()> Display::whenPressed;

color Display::getBackgroundColor() {
	return backgroundColor;
}	

void Display::clearScreen() {
	for(auto &e : elements) {
		e->setRendering(false);
	}
}
void Display::setBackgroundColor(color backgroundColor) {
	Display::backgroundColor = backgroundColor;
}

int Display::getFontCellHeight(fontType font) {
	if(font == fontType::mono12) {
		return 12;
	} else if(font == fontType::mono15) {
		return 15;
	} else if(font == fontType::mono20 || font == fontType::prop20) {
		return 20;
	} else if(font == fontType::mono30 || font == fontType::prop30) {
		return 30;
	} else if(font == fontType::mono40 || font == fontType::prop40) {
		return 40;
	} else {
		return 60;
	}
}
int Display::getFontCellWidth(fontType font) {
	int cellHeight = getFontCellHeight(font);
	return cellHeight / 2;
}

void Display::pressed(function<void()> whenPressed) {
  Display::whenPressed = whenPressed;
}

void Display::handlePress() {
  if(whenPressed) {
    whenPressed();
  }
	for(auto &e : elements) {
		if(e->isRendering()) {
			e->handlePress();
		}
	}
}

void Display::addElement(Element* e) {
  elements.push_back(e);
}
void Display::removeElement(Element* e) {
  elements.erase(remove(elements.begin(), elements.end(), e), elements.end());
}

bool sortElements(Element* e1, Element* e2) {
  return e1->getLayer() < e2->getLayer();
}

int Display::render() {
	while(true) {
		Brain.Screen.clearScreen(backgroundColor);

    sort(elements.begin(), elements.end(), sortElements);
		
		for(auto &e : elements) {
			if(e->isRendering()) {
				e->render();
			}
		}
		Brain.Screen.render();
		
		this_thread::sleep_for(10);
	}
	
	return 1;
}

/*-----------------------
        Element
-----------------------*/

Element::Element(int x, int y) {
	this->x = x;
	this->y = y;
	Display::addElement(this);
}
Element::~Element() {
  Display::removeElement(this);
}
			
bool Element::isRendering() {
	return rendering;
}
int Element::getX() {
	return x; 
}
int Element::getY() {
	return y;
}
int Element::getLayer() {
  return layer;
}

void Element::setPosition(int x, int y) {
	this->x = x;
	this->y = y;
}
void Element::setRendering(bool rendering) {
	this->rendering = rendering;
}
void Element::setLayer(int layer) {
  this->layer = layer;
}

void Element::pressed(function<void(Element&)> whenPressed) {
	this->whenPressed = whenPressed;
}

/*-----------------------
          Text
-----------------------*/

template <typename... Ts>
Text::Text(int x, int y, string text, Ts... values) : Element(x, y) {
  this->setText(text, values...);
}

string Text::getText() {
	return text;
}
color Text::getTextColor() {
	return textColor;
}
color Text::getBackgroundColor() {
	return backgroundColor;
}
fontType Text::getFont() {
	return font;
}

int Text::getWidth() {
  istringstream tokenStream(text);
  string token;
  int maxWidth = 0;
  while(getline(tokenStream, token, '\n')) {
    maxWidth = max(maxWidth, static_cast<int>(Display::getFontCellWidth(font) * token.length()));
  }
	return maxWidth;
}
int Text::getHeight() {
	return Display::getFontCellHeight(font) * (count(text.begin(), text.end(), '\n') + 1);
}

template <typename... Ts>
void Text::setText(string text, Ts... values) {
  size_t size = snprintf(nullptr, 0, text.c_str(), values...) + 1;
  unique_ptr<char[]> buffer(new char[size]);
  snprintf(buffer.get(), size, text.c_str(), values...);
  this->text = string(buffer.get(), buffer.get() + size - 1);
}
void Text::setTextColor(color textColor) {
	this->textColor = textColor;
}
void Text::setBackgroundColor(color backgroundColor) {
	this->backgroundColor = backgroundColor;
}
void Text::setFont(fontType font) {
	this->font = font;
}

void Text::handlePress() {
	if(whenPressed) {
		if(Brain.Screen.xPosition() >= x && Brain.Screen.xPosition() <= x + getWidth()) {
			if(Brain.Screen.yPosition() >= y && Brain.Screen.yPosition() <= y + getHeight()) {
				whenPressed(*this);
			}
		}
	}
}

void Text::render() {
  // If an element is declared outside of a function,
  // color defaults do not initialize properly. 
  // This is a band-aid.
  if(!textColor) {
    textColor = color::white;
  }
  if(!backgroundColor) {
    backgroundColor = color::transparent;
  }

	Brain.Screen.setFont(font);
	Brain.Screen.setPenColor(textColor);
	Brain.Screen.setFillColor(backgroundColor);

  istringstream tokenStream(text);
  string token;
  int lineNumber = 0;
  while(getline(tokenStream, token, '\n')) {
	  Brain.Screen.printAt(x, y + Display::getFontCellHeight(font) * lineNumber, backgroundColor != vex::transparent, token.c_str());
    lineNumber++;
  }
}

/*-----------------------
        Rectangle
-----------------------*/

Rectangle::Rectangle(int x, int y, int width, int height) : Element(x, y) {
	this->width = width;
	this->height = height;
}

int Rectangle::getWidth() {
	return width;
}
int Rectangle::getHeight() {
	return height;
}
color Rectangle::getFillColor() {
	return fillColor;
}
color Rectangle::getBorderColor() {
	return borderColor;
}
int Rectangle::getBorderWidth() {
	return borderWidth;
}

void Rectangle::setSize(int width, int height) {
	this->width = width;
	this->height = height;
}
void Rectangle::setFillColor(color fillColor) {
	this->fillColor = fillColor;
}
void Rectangle::setBorderColor(color borderColor) {
	this->borderColor = borderColor;
}
void Rectangle::setBorderWidth(int borderWidth) {
	this->borderWidth = max(0, min(35, borderWidth));
}

void Rectangle::handlePress() {
	if(whenPressed) {
		if(Brain.Screen.xPosition() >= x && Brain.Screen.xPosition() <= x + width) {
			if(Brain.Screen.yPosition() >= y && Brain.Screen.yPosition() <= y + height) {
				whenPressed(*this);
			}
		}
	}
}

void Rectangle::render() {
  // If an element is declared outside of a function,
  // color defaults do not initialize properly. 
  // This is a band-aid.
  if(!fillColor) {
    fillColor = color::white;
  }
  if(!borderColor) {
    borderColor = color::white;
  }

	Brain.Screen.setPenColor(borderColor);
	Brain.Screen.setPenWidth(borderWidth);
	Brain.Screen.setFillColor(fillColor);
  Brain.Screen.drawRectangle(x, y, width, height);
}

/*-----------------------
          Circle
-----------------------*/

Circle::Circle(int x, int y, int radius) : Element(x, y) {
	this->radius = radius;
}

int Circle::getRadius() {
	return radius;
}
color Circle::getFillColor() {
	return fillColor;
}
color Circle::getBorderColor() {
	return borderColor;
}
int Circle::getBorderWidth() {
	return borderWidth;
}

void Circle::setRadius(int radius) {
	this->radius = radius;
}
void Circle::setFillColor(color fillColor) {
	this->fillColor = fillColor;
}
void Circle::setBorderColor(color borderColor) {
	this->borderColor = borderColor;
}
void Circle::setBorderWidth(int borderWidth) {
	this->borderWidth = max(0, min(35, borderWidth));
}

void Circle::handlePress() {
	if(whenPressed) {
		if(sqrt(pow(Brain.Screen.xPosition() - (x + radius), 2) + pow(Brain.Screen.yPosition() - (y + radius), 2)) <= radius) {
      whenPressed(*this);
    }
	}
}

void Circle::render() {
  // If an element is declared outside of a function,
  // color defaults do not initialize properly. 
  // This is a band-aid.
  if(!fillColor) {
    fillColor = color::white;
  }
  if(!borderColor) {
    borderColor = color::white;
  }

	Brain.Screen.setPenColor(borderColor);
	Brain.Screen.setPenWidth(borderWidth);
	Brain.Screen.setFillColor(fillColor);
  Brain.Screen.drawCircle(x, y, radius);
}

/*-----------------------
          Line
-----------------------*/

Line::Line(int x, int y, int x2, int y2) : Element(x, y) {
  this->x2 = x2;
  this->y2 = y2;
}

int Line::getX2() {
  return x2;
}
int Line::getY2() {
  return y2;
}
int Line::getThickness() {
  return thickness;
}
color Line::getLineColor() {
  return lineColor;
}

void Line::setPosition(int x, int y, int x2, int y2) {
  this->x = x;
  this->y = y;
  this->x2 = x2;
  this->y2 = y2;
}
void Line::setThickness(int thickness) {
  this->thickness = max(0, min(35, thickness));
}
void Line::setLineColor(color lineColor) {
  this->lineColor = lineColor;
}

void Line::handlePress() {
  if(whenPressed) {
    bool inside = false;
    double xDiff = getX2() - getX();
    if(xDiff == 0) {
      double xMin = getX() - getThickness() / 2.0;
      double yMin = getY() < getY2() ? getY() : getY2();
      double xMax = getX() + getThickness() / 2.0;
      double yMax = getY() > getY2() ? getY() : getY2();
      
      inside = x >= xMin && x <= xMax && y >= yMin && y <= yMax;
    } else {
      double slope = (getY2() - getY()) / xDiff;
      if(slope == 0) {
        double xMin = getX() < getX2() ? getX() : getX2();
        double yMin = getY() - getThickness() / 2.0;
        double xMax = getX() > getX2() ? getX() : getX2();
        double yMax = getY() + getThickness() / 2.0;

        inside = x >= xMin && x <= xMax && y >= yMin && y <= yMax;
      } else {
        double orthoSlope = -1 / slope;

        double y11 = (orthoSlope * (x - getX())) + getY();
        double y12 = (orthoSlope * (x - getX2())) + getY2();

        double l1X = getX() - (getThickness() / 2.0 / sqrt(1 + pow(orthoSlope, 2)));
        double l1Y = (orthoSlope * (l1X - getX())) + getY();
        double y21 = (slope * (x - l1X)) + l1Y;

        double l2X = getX2() + (getThickness() / 2.0 / sqrt(1 + pow(orthoSlope, 2)));
        double l2Y = (orthoSlope * (l2X - getX2())) + getY2();
        double y22 = (slope * (x - l2X)) + l2Y;

        double y1Min = y11 < y12 ? y11 : y12;
        double y1Max = y11 > y12 ? y11 : y12;
        double y2Min = y21 < y22 ? y21 : y22;
        double y2Max = y21 > y22 ? y21 : y22;

        inside = y >= y1Min && y <= y1Max && y >= y2Min && y <= y2Max;
      }
    }

    if(inside) {
      whenPressed(*this);
    }
  }
}

void Line::render() {
  // If an element is declared outside of a function,
  // color defaults do not initialize properly. 
  // This is a band-aid.
  if(!lineColor) {
    lineColor = color::white;
  }

  Brain.Screen.setPenColor(lineColor);
	Brain.Screen.setPenWidth(thickness);
  Brain.Screen.drawLine(x, y, x2, y2);
}

/*-----------------------
          Pixel
-----------------------*/

Pixel::Pixel(int x, int y) : Element(x, y) {}

color Pixel::getPixelColor() {
  return pixelColor;
}

void Pixel::setPixelColor(color pixelColor) {
  this->pixelColor = pixelColor;
}

void Pixel::handlePress() {
  if(whenPressed) {
    if(Brain.Screen.xPosition() == x && Brain.Screen.yPosition() == y) {
      whenPressed(*this);
    }
  }
}

void Pixel::render() {
  // If an element is declared outside of a function,
  // color defaults do not initialize properly. 
  // This is a band-aid.
  if(!pixelColor) {
    pixelColor = color::white;
  }

  Brain.Screen.setPenColor(pixelColor);
  Brain.Screen.drawPixel(x, y);
}

/*-----------------------
          Image
-----------------------*/

Image::Image(int x, int y, uint8_t* buffer, int bufferLen, int width, int height) : Element(x, y) {
  this->buffer8 = buffer;
  this->bufferLen = bufferLen;
  this->width = width;
  this->height = height;
}
Image::Image(int x, int y, uint32_t* buffer, int width, int height) : Element(x, y) {
  this->buffer32 = buffer;
  this->width = width;
  this->height = height;
}
Image::Image(int x, int y, const char* fileName, int width, int height) : Element(x, y) {
  this->fileName = fileName;
  this->width = width;
  this->height = height;
}

int Image::getWidth() {
  return width;
}
int Image::getHeight() {
  return height;
}

template <typename ImageType>
ImageType Image::getImage() {
  if(buffer8) {
    return buffer8;
  } else if(buffer32) {
    return buffer32;
  } else {
    return fileName;
  }
}

void Image::setImage(uint8_t* buffer, int bufferLen, int width, int height) {
  buffer8 = buffer;
  buffer32 = 0;
  fileName = nullptr;
  this->bufferLen = bufferLen;
  this->width = width;
  this->height = height;
}
void Image::setImage(uint32_t* buffer, int width, int height) {
  buffer32 = buffer;
  fileName = nullptr;
  this->width = width;
  this->height = height;
  buffer8 = 0;
}
void Image::setImage(const char* fileName, int width, int height) {
  this->fileName = fileName;
  buffer8 = 0;
  buffer32 = 0;
  this->width = width;
  this->height = height;
}

void Image::handlePress() {
  if(whenPressed) {
		if(Brain.Screen.xPosition() >= x && Brain.Screen.xPosition() <= x + width) {
			if(Brain.Screen.yPosition() >= y && Brain.Screen.yPosition() <= y + height) {
				whenPressed(*this);
			}
		}
	}
}

void Image::render() {
  if(buffer8) {
    Brain.Screen.drawImageFromBuffer(buffer8, x, y, bufferLen);
  } else if(buffer32) {
    Brain.Screen.drawImageFromBuffer(buffer32, x, y, width, height);
  } else {
    Brain.Screen.drawImageFromFile(fileName, x, y);
  }
}