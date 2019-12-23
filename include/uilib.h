#include "vex.h"

#include <vector>

const int BRAIN_SCREEN_WIDTH = 480;
const int BRAIN_SCREEN_HEIGHT = 240;

using namespace std;
using namespace vex;

namespace uilib {
  class Element {
    protected:
      int x;
      int y;
      bool rendering = false;
      int layer = 0;

      function<void(Element&)> whenPressed;
    
    public:
      Element(int x, int y);
      ~Element();
    
      bool isRendering();
      int getX();
      int getY();
      int getLayer();
    
      virtual void setPosition(int x, int y);
      virtual void setRendering(bool rendering);
      virtual void setLayer(int layer);

      virtual void pressed(function<void(Element&)> whenPressed);

      virtual void handlePress() = 0;
      virtual void render() = 0;
  };

  class Text : public Element {
    private:
      string text;
      color textColor = color::white;
      color backgroundColor = color::transparent;
      fontType font = fontType::mono20;
    
    public:
      template<typename... Ts>
      Text(int x, int y, string text, Ts... values);
    
      string getText();
      color getTextColor();
      color getBackgroundColor();
      fontType getFont();
    
      int getWidth();
      int getHeight();

      template<typename... Ts>
      void setText(string text, Ts... values);
      void setTextColor(color textColor);
      void setBackgroundColor(color backgroundColor);
      void setFont(fontType font);

      void handlePress() override;
      void render() override;
  };

  class Rectangle : public Element {
    private:
      int width;
      int height;
      color borderColor = color::white;
      int borderWidth = 0;
      color fillColor = color::white;

    public:
      Rectangle(int x, int y, int width, int height);
    
      int getWidth();
      int getHeight();
      color getFillColor();
      color getBorderColor();
      int getBorderWidth();
    
      void setSize(int width, int height);
      void setFillColor(color fillColor);
      void setBorderColor(color borderColor);
      void setBorderWidth(int borderWidth);

      void handlePress() override;
      void render() override;
  };

  class Circle : public Element {
    private:
      int radius;
      color borderColor = color::white;
      int borderWidth = 0;
      color fillColor = color::white;

    public:
      Circle(int x, int y, int radius);
    
      int getRadius();
      color getFillColor();
      color getBorderColor();
      int getBorderWidth();
    
      void setRadius(int radius);
      void setFillColor(color fillColor);
      void setBorderColor(color borderColor);
      void setBorderWidth(int borderWidth);

      void handlePress() override;
      void render() override;
  };

  class Line : public Element {
    private:
      int x2;
      int y2;
      int thickness = 1;
      color lineColor = color::white;

    public:
      Line(int x, int y, int x2, int y2);

      int getX2();
      int getY2();
      int getThickness();
      color getLineColor();

      void setPosition(int x, int y, int x2, int y2);
      void setThickness(int thickness);
      void setLineColor(color lineColor);

      void handlePress() override;
      void render() override;
  };

  class Pixel : public Element {
    private:
      color pixelColor = color::white;

    public:
      Pixel(int x, int y);

      color getPixelColor();

      void setPixelColor(color pixelColor);

      void handlePress() override;
      void render() override;
  };

  class Image : public Element {
    private:
      const char* fileName;
      uint32_t* buffer32;
      uint8_t* buffer8;
      int width;
      int height;
      int bufferLen;

    public:
      Image(int x, int y, uint8_t* buffer, int bufferLen, int width, int height);
      Image(int x, int y, uint32_t* buffer, int width, int height);
      Image(int x, int y, const char* fileName, int width, int height);

      int getWidth();
      int getHeight();
      template <typename ImageType>
      ImageType getImage();

      void setImage(uint8_t* buffer, int bufferLen, int width, int height);
      void setImage(uint32_t* buffer, int width, int height);
      void setImage(const char* fileName, int width, int height);

      void handlePress() override;
      void render() override;
  };

  class Display {
    public:
      static color getBackgroundColor(); 

      static void clearScreen();
      static void setBackgroundColor(color backgroundColor);

      static int getFontCellHeight(fontType font);
      static int getFontCellWidth(fontType font);

      static function<void()> whenPressed;
      static void pressed(function<void()> whenPressed);
      static void handlePress();

      static void addElement(Element* e);
      static void removeElement(Element* e);

      static int render();

    private:
      static color backgroundColor;
      static vector<Element*> elements;
  };
};