// Plotting Framework
//
// Copyright (C) 2019-2020  Mario Krüger
// Contact: mario.kruger@cern.ch
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef Plot_h
#define Plot_h

// make root style property enums available to user
#include "TAttMarker.h"
#include "TAttLine.h"
#include "TAttFill.h"
#include "Rtypes.h"

#include "PlottingFramework.h"

namespace PlottingFramework
{

enum drawing_options_t : uint8_t {
  // 1d options
  points = 0,
  points_xerr,
  points_endcaps,
  points_line,
  line,
  curve,
  band,
  band_line,
  hist,
  hist_no_borders,
  fit,
  bar,
  area,
  area_curve,
  area_line,
  boxes,
  boxes_only,
  stars,
  text,
  // 2d options
  colz,
  surf,
};

struct layout_t {
  optional<int16_t> color; // marker_color , line_color, fill_color, text_color
  optional<int16_t> style; // marker_style , line_style, fill_style, text_font
  optional<float_t> scale; // marker_size , line_width, fill_opacity, text_size
};

//**************************************************************************************************
/**
 * Class for internal representation of a plot.
 */
//**************************************************************************************************
class Plot
{
public:
  class Pad;

  Plot() = default;
  Plot(const ptree& plotTree);
  Plot(const string& name, const string& figureGroup, const string& plotTemplateName = "");
  Pad& operator[](uint8_t padID) { return mPads[padID]; }
  Pad& GetPad(uint8_t padID) { return mPads[padID]; }
  Pad& GetPadDefaults() { return mPads[0]; }
  void operator+=(const Plot& plot);
  friend Plot operator+(const Plot& templatePlot, const Plot& plot);
  Plot(const Plot& otherPlot, const string& name, const string& plotGroup);
  Plot Clone() const;

  // accessors for user
  void SetFigureCategory(const string& figureCategory) { mFigureCategory = figureCategory; }
  void SetPlotTemplateName(const string& plotTemplateName) { mPlotTemplateName = plotTemplateName; }

  void SetDimensions(int32_t width, int32_t height, bool fixAspectRatio = false)
  {
    mPlotDimensions = {width, height, fixAspectRatio};
  }
  void SetWidth(int32_t width) { mPlotDimensions.width = width; }
  void SetHeight(int32_t height) { mPlotDimensions.height = height; }
  void SetFixAspectRatio(bool fixAspectRatio = true)
  {
    mPlotDimensions.fixAspectRatio = fixAspectRatio;
  }

  Plot& SetFill(int16_t color, int16_t style = 1001);
  Plot& SetTransparent();

protected:
  friend class PlotManager;
  friend class PlotPainter;

  inline void SetFigureGroup(const string& figureGroup) { mFigureGroup = figureGroup; }

  // accessors for internal use by manager and painter
  const string& GetName() { return mName; }
  const string& GetFigureGroup() { return mFigureGroup; }
  const string& GetFigureCategory() { return mFigureCategory; }
  const optional<string>& GetPlotTemplateName() { return mPlotTemplateName; }
  string GetUniqueName()
  {
    return mName + gNameGroupSeparator + mFigureGroup + ((mFigureCategory != "") ? ":" + mFigureCategory : "");
  }
  ptree GetPropertyTree();

  map<uint8_t, Pad>& GetPads() { return mPads; }

  const optional<int32_t>& GetHeight() { return mPlotDimensions.height; }
  const optional<int32_t>& GetWidth() { return mPlotDimensions.width; }
  const optional<bool>& IsFixAspectRatio() { return mPlotDimensions.fixAspectRatio; }
  const optional<int16_t>& GetFillColor() { return mFill.color; }
  const optional<int16_t>& GetFillStyle() { return mFill.style; }

private:
  struct dimension_t {
    optional<int32_t> width;
    optional<int32_t> height;
    optional<bool> fixAspectRatio;
  };
  struct plot_fill_t {
    optional<int16_t> color;
    optional<int16_t> style;
  };

  string mName;
  string mFigureGroup;
  string mFigureCategory;
  optional<string> mPlotTemplateName;
  dimension_t mPlotDimensions;

  plot_fill_t mFill;

  map<uint8_t, Pad> mPads;
};

//**************************************************************************************************
/**
 * Representation of a pad.
 */
//**************************************************************************************************
class Plot::Pad
{
public:
  class Data;
  class Ratio;
  class Axis;
  template <class BoxType>
  class Box;
  class TextBox;
  class LegendBox;

  Pad() = default;
  Pad(const ptree& padTree);
  Axis& operator[](const string& axis);
  void operator+=(const Pad& pad);

  // User accessors:
  Data& AddData(const string& name, const string& inputIdentifier, const string& lable = "");
  Data& AddData(const string& name, const Data& dataTemplate, const string& lable = "");
  // Data& AddData(const Data& data, const string& lable);
  // Data& AddData(const Data& data);

  Ratio& AddRatio(const string& numeratorName, const string& numeratorInputIdentifier,
                  const string& denominatorName, const string& denominatorInputIdentifier,
                  const string& lable = "");
  Ratio& AddRatio(const string& numeratorName, const Data& data, const string& denominatorName,
                  const string& denominatorInputIdentifier, const string& lable = "");

  TextBox& AddText(double_t xPos, double_t yPos, const string& text);
  TextBox& AddText(const string& text);
  LegendBox& AddLegend(double_t xPos, double_t yPos);
  LegendBox& AddLegend();

  Axis& GetAxis(const string& axis);
  Data& GetData(uint8_t dataID);
  LegendBox& GetLegend(uint8_t legendID);
  TextBox& GetText(uint8_t textID);

  Pad& SetTitle(const string& title);
  Pad& SetPosition(double_t xlow, double_t ylow, double_t xup, double_t yup);
  Pad& SetMargins(float_t top, float_t bottom, float_t left, float_t right);
  Pad& SetPalette(int32_t palette);
  Pad& SetDefaultTextSize(float_t size);
  Pad& SetDefaultTextColor(int16_t color);
  Pad& SetDefaultTextFont(int16_t font);
  Pad& SetDefaultMarkerSize(float_t size);
  Pad& SetDefaultMarkerColors(const vector<int16_t>& colors);
  Pad& SetDefaultMarkerStyles(const vector<int16_t>& styles);
  Pad& SetDefaultLineWidth(float_t width);
  Pad& SetDefaultLineColors(const vector<int16_t>& colors);
  Pad& SetDefaultLineStyles(const vector<int16_t>& styles);
  Pad& SetDefaultFillOpacity(float_t opacity);
  Pad& SetDefaultFillColors(const vector<int16_t>& colors);
  Pad& SetDefaultFillStyles(const vector<int16_t>& styles);
  Pad& SetDefaultDrawingOptionGraph(drawing_options_t drawingOption);
  Pad& SetDefaultDrawingOptionHist(drawing_options_t drawingOption);
  Pad& SetDefaultDrawingOptionHist2d(drawing_options_t drawingOption);
  Pad& SetFill(int16_t color, int16_t style = 1001);
  Pad& SetTransparent();
  Pad& SetFillFrame(int16_t color, int16_t style = 1001);
  Pad& SetLineFrame(int16_t color, int16_t style = kSolid, float_t width = 1.f);
  Pad& SetTransparentFrame();
  Pad& SetRedrawAxes(bool redraw = true);
  Pad& SetRefFunc(const string& refFunc);

protected:
  friend class PlotManager;
  friend class PlotPainter;
  friend class Plot;

  ptree GetPropertyTree();

  vector<shared_ptr<Data>>& GetData() { return mData; }
  vector<shared_ptr<LegendBox>>& GetLegendBoxes() { return mLegendBoxes; }
  vector<shared_ptr<TextBox>>& GetTextBoxes() { return mTextBoxes; }

  const auto& GetAxes() { return mAxes; }
  const auto& GetTitle() { return mTitle; }
  const auto& GetXLow() { return mPosition.xlow; }
  const auto& GetYLow() { return mPosition.ylow; }
  const auto& GetXUp() { return mPosition.xup; }
  const auto& GetYUp() { return mPosition.yup; }
  const auto& GetMarginTop() { return mMargins.top; }
  const auto& GetMarginBottom() { return mMargins.bottom; }
  const auto& GetMarginLeft() { return mMargins.left; }
  const auto& GetMarginRight() { return mMargins.right; }
  const auto& GetPalette() { return mPalette; }
  const auto& GetFillColor() { return mFill.color; }
  const auto& GetFillStyle() { return mFill.style; }
  const auto& GetFillColorFrame() { return mFrame.fillColor; }
  const auto& GetFillStyleFrame() { return mFrame.fillStyle; }
  const auto& GetLineColorFrame() { return mFrame.lineColor; }
  const auto& GetLineStyleFrame() { return mFrame.lineStyle; }
  const auto& GetLineWidthFrame() { return mFrame.lineWidth; }
  const auto& GetDefaultTextColor() { return mText.color; }
  const auto& GetDefaultTextFont() { return mText.font; }
  const auto& GetDefaultTextSize() { return mText.size; }
  const auto& GetDefaultMarkerSize() { return mMarkerDefaults.scale; }
  const auto& GetDefaultLineWidth() { return mLineDefaults.scale; }
  const auto& GetDefaultFillOpacity() { return mFillDefaults.scale; }
  const auto& GetDefaultMarkerColors() { return mMarkerDefaults.colors; }
  const auto& GetDefaultLineColors() { return mLineDefaults.colors; }
  const auto& GetDefaultFillColors() { return mFillDefaults.colors; }
  const auto& GetDefaultMarkerStyles() { return mMarkerDefaults.styles; }
  const auto& GetDefaultLineStyles() { return mLineDefaults.styles; }
  const auto& GetDefaultFillStyles() { return mFillDefaults.styles; }
  const auto& GetDefaultDrawingOptionGraph() { return mDrawingOptionDefaults.graph; }
  const auto& GetDefaultDrawingOptionHist() { return mDrawingOptionDefaults.hist; }
  const auto& GetDefaultDrawingOptionHist2d() { return mDrawingOptionDefaults.hist2d; }
  const auto& GetRedrawAxes() { return mRedrawAxes; }
  const auto& GetRefFunc() { return mRefFunc; }

private:
  struct pad_position_t {
    optional<double_t> xlow;
    optional<double_t> ylow;
    optional<double_t> xup;
    optional<double_t> yup;
  };
  struct pad_margin_t {
    optional<float_t> top;
    optional<float_t> bottom;
    optional<float_t> left;
    optional<float_t> right;
  };
  struct pad_fill_t {
    optional<int16_t> color;
    optional<int16_t> style;
  };
  struct frame_t {
    optional<int16_t> fillColor;
    optional<int16_t> fillStyle;
    optional<int16_t> lineColor;
    optional<int16_t> lineStyle;
    optional<float_t> lineWidth;
  };
  struct text_t {
    optional<int16_t> color;
    optional<int16_t> font;
    optional<float_t> size;
  };
  struct view_defaults_t {
    optional<float_t> scale;
    optional<vector<int16_t>> styles;
    optional<vector<int16_t>> colors;
  };
  struct data_defaults_t {
    optional<drawing_options_t> graph;
    optional<drawing_options_t> hist;
    optional<drawing_options_t> hist2d;
  };

  // properties
  optional<string> mTitle;
  optional<string> mOptions;
  pad_position_t mPosition;
  pad_margin_t mMargins;
  pad_fill_t mFill;
  frame_t mFrame;
  text_t mText;

  // user defined default data representation
  view_defaults_t mMarkerDefaults;
  view_defaults_t mLineDefaults;
  view_defaults_t mFillDefaults;
  data_defaults_t mDrawingOptionDefaults;

  optional<int32_t> mPalette;

  optional<bool> mRedrawAxes;
  optional<string> mRefFunc;

  map<string, Axis> mAxes;
  vector<shared_ptr<Data>> mData;

  vector<shared_ptr<LegendBox>> mLegendBoxes;
  vector<shared_ptr<TextBox>> mTextBoxes;
};

//**************************************************************************************************
/**
 * Representation of drawable data.
 */
//**************************************************************************************************
class Plot::Pad::Data
{
public:
  Data() = default;
  Data(const string& name, const string& inputIdentifier, const string& lable = "");
  Data(const ptree& dataTree);

  virtual ~Data() = default;
  Data(const Data& other) = default;
  Data(Data&&) = default;
  Data& operator=(const Data& other) = default;
  Data& operator=(Data&& other) = default;

  // user accessors
  virtual Data& SetLayout(const Data& dataLayout);
  virtual Data& SetRangeX(double_t min, double_t max);
  virtual Data& SetMaxRangeX(double_t max);
  virtual Data& SetMinRangeX(double_t min);
  virtual Data& UnsetRangeX();
  virtual Data& SetRangeY(double_t min, double_t max);
  virtual Data& SetMaxRangeY(double_t max);
  virtual Data& SetMinRangeY(double_t min);
  virtual Data& UnsetRangeY();
  virtual Data& SetLegendLable(const string& legendLable);
  virtual Data& SetLegendID(uint8_t legendID);
  virtual Data& SetOptions(const string& opions);
  virtual Data& SetOptions(drawing_options_t optionAlias);
  virtual Data& UnsetOptions();
  virtual Data& SetTextFormat(const string& textFormat);
  virtual Data& SetNormalize(bool useWidth = false);
  virtual Data& SetScaleFactor(double_t scale);
  virtual Data& SetColor(int16_t color);
  virtual Data& SetMarker(int16_t color, int16_t style, float_t size);
  virtual Data& SetMarkerColor(int16_t color);
  virtual Data& SetMarkerStyle(int16_t style);
  virtual Data& SetMarkerSize(float_t size);
  virtual Data& SetLine(int16_t color, int16_t style, float_t width);
  virtual Data& SetLineColor(int16_t color);
  virtual Data& SetLineStyle(int16_t style);
  virtual Data& SetLineWidth(float_t width);
  virtual Data& SetFill(int16_t color, int16_t style, float_t opacity = 1.);
  virtual Data& SetFillColor(int16_t color);
  virtual Data& SetFillStyle(int16_t style);
  virtual Data& SetFillOpacity(float_t opacity);
  virtual Data& SetDefinesFrame();

  Data& SetInputID(const string& inputIdentifier);
  const string& GetInputID() const { return mInputIdentifier; }

protected:
  friend class PlotManager;
  friend class PlotPainter;
  friend class Plot;

  virtual std::shared_ptr<Data> Clone() const { return std::make_shared<Data>(*this); }

  virtual ptree GetPropertyTree();

  void SetType(const string& type) { mType = type; }

  string GetUniqueName() { return mName + gNameGroupSeparator + mInputIdentifier; }

  const auto& GetType() const { return mType; }
  const auto& GetName() const { return mName; }
  const auto& GetLegendLable() { return mLegend.lable; }
  const auto& GetLegendID() { return mLegend.identifier; }

  const auto& GetMarkerColor() { return mMarker.color; }
  const auto& GetMarkerStyle() { return mMarker.style; }
  const auto& GetMarkerSize() { return mMarker.scale; }

  const auto& GetLineColor() { return mLine.color; }
  const auto& GetLineStyle() { return mLine.style; }
  const auto& GetLineWidth() { return mLine.scale; }

  const auto& GetFillColor() { return mFill.color; }
  const auto& GetFillStyle() { return mFill.style; }
  const auto& GetFillOpacity() { return mFill.scale; }

  const auto& GetDrawingOptions() { return mDrawingOptions; }
  const auto& GetDrawingOptionAlias() { return mDrawingOptionAlias; }
  const auto& GetTextFormat() { return mTextFormat; }
  const auto& GetScaleFactor() { return mModify.scale_factor; }
  const auto& GetNormMode() { return mModify.norm_mode; }

  const auto& GetMinRangeX() { return mRangeX.min; }
  const auto& GetMaxRangeX() { return mRangeX.max; }
  const auto& GetMinRangeY() { return mRangeY.min; }
  const auto& GetMaxRangeY() { return mRangeY.max; }

  const bool& GetDefinesFrame() { return mDefinesFrame; }

private:
  bool mDefinesFrame;

  string mType; // for introspection: "data" or "ratio"
  string mName;
  string mInputIdentifier;

  optional<string> mDrawingOptions;
  optional<drawing_options_t> mDrawingOptionAlias;
  optional<string> mTextFormat;

  struct modify_t {
    optional<uint8_t> norm_mode; // 0: sum over bin contents, 1: with bin width
    optional<double_t> scale_factor;
  };
  struct legend_t {
    optional<string> lable;
    optional<uint8_t> identifier;
  };

  struct dataRange_t {
    optional<double_t> min;
    optional<double_t> max;
  };

  legend_t mLegend;
  layout_t mMarker;
  layout_t mLine;
  layout_t mFill;
  dataRange_t mRangeX;
  dataRange_t mRangeY;
  modify_t mModify;
};

//**************************************************************************************************
/**
 * Representation of a Ratio.
 */
//**************************************************************************************************
class Plot::Pad::Ratio : public Plot::Pad::Data
{
public:
  Ratio(const string& name, const string& inputIdentifier, const string& denomName,
        const string& denomInputIdentifier, const string& lable);
  Ratio(const ptree& dataTree);

  virtual ~Ratio() = default;
  Ratio(const Ratio& other) = default;
  Ratio(Ratio&&) = default;
  Ratio& operator=(const Ratio& other) = default;
  Ratio& operator=(Ratio&& other) = default;

  Ratio& SetIsCorrelated(bool isCorrelated = true);

  // return correct type for the data accessors
  Ratio& SetLayout(const Data& dataLayout) { return static_cast<decltype(*this)&>(Data::SetLayout(dataLayout)); }
  Ratio& SetRangeX(double_t min, double_t max) { return static_cast<decltype(*this)&>(Data::SetRangeX(min, max)); }
  Ratio& SetMaxRangeX(double_t max) { return static_cast<decltype(*this)&>(Data::SetMaxRangeX(max)); }
  Ratio& SetMinRangeX(double_t min) { return static_cast<decltype(*this)&>(Data::SetMinRangeX(min)); }
  Ratio& UnsetRangeX() { return static_cast<decltype(*this)&>(Data::UnsetRangeY()); }
  Ratio& SetRangeY(double_t min, double_t max) { return static_cast<decltype(*this)&>(Data::SetRangeY(min, max)); }
  Ratio& SetMaxRangeY(double_t max) { return static_cast<decltype(*this)&>(Data::SetMaxRangeY(max)); }
  Ratio& SetMinRangeY(double_t min) { return static_cast<decltype(*this)&>(Data::SetMinRangeY(min)); }
  Ratio& UnsetRangeY() { return static_cast<decltype(*this)&>(Data::UnsetRangeY()); }
  Ratio& SetLegendLable(const string& legendLable) { return static_cast<decltype(*this)&>(Data::SetLegendLable(legendLable)); }
  Ratio& SetLegendID(uint8_t legendID) { return static_cast<decltype(*this)&>(Data::SetLegendID(legendID)); }
  Ratio& SetOptions(const string& opions) { return static_cast<decltype(*this)&>(Data::SetOptions(opions)); }
  Ratio& SetOptions(drawing_options_t optionAlias) { return static_cast<decltype(*this)&>(Data::SetOptions(optionAlias)); }
  Ratio& UnsetOptions() { return static_cast<decltype(*this)&>(Data::UnsetOptions()); }
  Ratio& SetTextFormat(const string& textFormat) { return static_cast<decltype(*this)&>(Data::SetTextFormat(textFormat)); }
  Ratio& SetNormalize(bool useWidth = false) { return static_cast<decltype(*this)&>(Data::SetNormalize(useWidth)); }
  Ratio& SetScale(double_t scale) { return static_cast<decltype(*this)&>(Data::SetScaleFactor(scale)); }
  Ratio& SetColor(int16_t color) { return static_cast<decltype(*this)&>(Data::SetColor(color)); }
  Ratio& SetMarker(int16_t color, int16_t style, float_t size) { return static_cast<decltype(*this)&>(Data::SetMarker(color, style, size)); }
  Ratio& SetMarkerColor(int16_t color) { return static_cast<decltype(*this)&>(Data::SetMarkerColor(color)); }
  Ratio& SetMarkerStyle(int16_t style) { return static_cast<decltype(*this)&>(Data::SetMarkerStyle(style)); }
  Ratio& SetMarkerSize(float_t size) { return static_cast<decltype(*this)&>(Data::SetMarkerSize(size)); }
  Ratio& SetLine(int16_t color, int16_t style, float_t width) { return static_cast<decltype(*this)&>(Data::SetLine(color, style, width)); }
  Ratio& SetLineColor(int16_t color) { return static_cast<decltype(*this)&>(Data::SetLineColor(color)); }
  Ratio& SetLineStyle(int16_t style) { return static_cast<decltype(*this)&>(Data::SetLineStyle(style)); }
  Ratio& SetLineWidth(float_t width) { return static_cast<decltype(*this)&>(Data::SetLineWidth(width)); }
  Ratio& SetFill(int16_t color, int16_t style, float_t opacity = 1.) { return static_cast<decltype(*this)&>(Data::SetFill(color, style, opacity)); }
  Ratio& SetFillColor(int16_t color) { return static_cast<decltype(*this)&>(Data::SetFillColor(color)); }
  Ratio& SetFillStyle(int16_t style) { return static_cast<decltype(*this)&>(Data::SetFillStyle(style)); }
  Ratio& SetFillOpacity(float_t opacity) { return static_cast<decltype(*this)&>(Data::SetFillOpacity(opacity)); }
  Ratio& SetDefinesFrame() { return static_cast<decltype(*this)&>(Data::SetDefinesFrame()); }

protected:
  friend class PlotManager;
  friend class PlotPainter;
  friend class Plot;

  virtual std::shared_ptr<Data> Clone() const { return std::make_shared<Ratio>(*this); }

  ptree GetPropertyTree();
  string GetDenomIdentifier() { return mDenomInputIdentifier; }
  string GetDenomName() { return mDenomName; }
  string GetUniqueNameDenom() { return mDenomName + gNameGroupSeparator + mDenomInputIdentifier; }

  const bool& GetIsCorrelated() { return mIsCorrelated; }

private:
  string mDenomName;
  string mDenomInputIdentifier;
  bool mIsCorrelated;
};

//**************************************************************************************************
/**
 * Representation of an axis.
 */
//**************************************************************************************************
class Plot::Pad::Axis
{
public:
  Axis() = default;

  Axis& SetTitle(const string& title);
  Axis& SetRange(double_t min, double_t max);
  Axis& SetMaxRange(double_t max);
  Axis& SetMinRange(double_t min);
  Axis& SetColor(int16_t color);
  Axis& SetAxisColor(int16_t color);
  Axis& SetNumDivisions(int32_t numDivisions);
  Axis& SetMaxDigits(int32_t maxDigtis);
  Axis& SetTickLength(float_t tickLength);
  Axis& SetTitleFont(int16_t font);
  Axis& SetLableFont(int16_t font);
  Axis& SetTitleSize(float_t size);
  Axis& SetLableSize(float_t size);
  Axis& SetTitleColor(int16_t color);
  Axis& SetLableColor(int16_t color);
  Axis& SetTitleOffset(float_t offset);
  Axis& SetLableOffset(float_t offset);
  Axis& SetTitleCenter(bool center = true);
  Axis& SetLableCenter(bool center = true);
  Axis& SetLog(bool isLog = true);
  Axis& SetGrid(bool isGrid = true);
  Axis& SetOppositeTicks(bool isOppositeTicks = true);
  Axis& SetTimeFormat(const string& timeFormat);
  Axis& SetTickOrientation(const string& tickOrientation);

protected:
  friend class PlotManager;
  friend class PlotPainter;
  friend class Plot;

  Axis(const string& axisName);
  Axis(const ptree& axisTree);

  ptree GetPropertyTree();
  void operator+=(const Axis& axis);

  const auto& GetMinRange() { return mRange.min; }
  const auto& GetMaxRange() { return mRange.max; }
  const auto& GetTickLength() { return mTickLength; }
  const auto& GetNumDivisions() { return mNumDivisions; }
  const auto& GetMaxDigits() { return mMaxDigits; }
  const auto& GetAxisColor() { return mAxisColor; }
  const auto& GetTitle() { return mTitle; }
  const auto& GetTitleFont() { return mTitleProperties.font; }
  const auto& GetLableFont() { return mLableProperties.font; }
  const auto& GetTitleSize() { return mTitleProperties.size; }
  const auto& GetLableSize() { return mLableProperties.size; }
  const auto& GetTitleColor() { return mTitleProperties.color; }
  const auto& GetLableColor() { return mLableProperties.color; }
  const auto& GetTitleOffset() { return mTitleProperties.offset; }
  const auto& GetLableOffset() { return mLableProperties.offset; }
  const auto& GetTitleCenter() { return mTitleProperties.center; }
  const auto& GetLableCenter() { return mLableProperties.center; }
  const auto& GetLog() { return mIsLog; }
  const auto& GetGrid() { return mIsGrid; }
  const auto& GetOppositeTicks() { return mIsOppositeTicks; }
  const auto& GetTimeFormat() { return mTimeFormat; }
  const auto& GetTickOrientation() { return mTickOrientation; }

private:
  struct axisTextProperties_t {
    optional<int16_t> font;
    optional<float_t> size;
    optional<int16_t> color;
    optional<float_t> offset;
    optional<bool> center;
  };
  struct axisRange_t {
    optional<double_t> min;
    optional<double_t> max;
  };

  string mName;
  axisRange_t mRange;
  optional<string> mTitle;
  optional<int32_t> mNumDivisions;
  optional<int32_t> mMaxDigits;
  optional<float_t> mTickLength;
  optional<int16_t> mAxisColor;

  optional<bool> mIsLog;
  optional<bool> mIsGrid;
  optional<bool> mIsOppositeTicks;
  optional<string> mTimeFormat;
  optional<string> mTickOrientation;

  axisTextProperties_t mTitleProperties;
  axisTextProperties_t mLableProperties;
};

//**************************************************************************************************
/**
 * Representation of a box.
 */
//**************************************************************************************************
template <class BoxType>
class Plot::Pad::Box
{
public:
  virtual ~Box() = default;
  Box(const Box& other) = default;
  Box(Box&&) = default;
  Box& operator=(const Box& other) = default;
  Box& operator=(Box&& other) = default;

  BoxType& SetPosition(double_t x, double_t y);
  BoxType& SetUserCoordinates(bool userCoordinates = true);
  BoxType& SetAutoPlacement();
  BoxType& SetBorder(int16_t color, int16_t style, float_t width);
  BoxType& SetBorderColor(int16_t color);
  BoxType& SetBorderStyle(int16_t style);
  BoxType& SetBorderWidth(float_t width);
  BoxType& SetText(int16_t color, int16_t font, float_t size);
  BoxType& SetTextColor(int16_t color);
  BoxType& SetTextFont(int16_t font);
  BoxType& SetTextSize(float_t size);
  BoxType& SetFill(int16_t color, int16_t style, float_t opacity);
  BoxType& SetFillColor(int16_t color);
  BoxType& SetFillStyle(int16_t style);
  BoxType& SetFillOpacity(float_t opacity);
  BoxType& SetTransparent();
  BoxType& SetNoBox();

protected:
  ptree GetPropertyTree();

  double_t GetXPosition() { return (mPos.x) ? *mPos.x : 0.; }
  double_t GetYPosition() { return (mPos.y) ? *mPos.y : 0.; }
  auto& GetBorderStyle() { return mBorder.style; }
  auto& GetBorderWidth() { return mBorder.scale; }
  auto& GetBorderColor() { return mBorder.color; }
  auto& GetFillStyle() { return mFill.style; }
  auto& GetFillOpacity() { return mFill.scale; }
  auto& GetFillColor() { return mFill.color; }
  auto& GetTextFont() { return mText.style; }
  auto& GetTextSize() { return mText.scale; }
  auto& GetTextColor() { return mText.color; }

  bool IsUserCoordinates() { return (mPos.isUserCoord) ? *mPos.isUserCoord : false; }
  bool IsAutoPlacement() { return (!mPos.x || !mPos.y); }

private:
  // allow construction of Box base class only in context actually useful boxes
  friend BoxType;
  Box() = default;
  Box(double_t xPos, double_t yPos);
  Box(const ptree& boxTree);

  auto GetThis() { return static_cast<BoxType*>(this); }

  struct position_t {
    optional<double_t> x;
    optional<double_t> y;
    optional<bool> isUserCoord;
  };

  position_t mPos;
  layout_t mText;
  layout_t mBorder;
  layout_t mFill;
};

//**************************************************************************************************
/**
 * Representation of a text box.
 */
//**************************************************************************************************
class Plot::Pad::TextBox : public Plot::Pad::Box<TextBox>
{
public:
  TextBox(const string& text);
  TextBox(double_t xPos, double_t yPos, const string& text);
  TextBox(const ptree& textBoxTree);

  virtual ~TextBox() = default;
  TextBox(const TextBox& other) = default;
  TextBox(TextBox&&) = default;
  TextBox& operator=(const TextBox& other) = default;
  TextBox& operator=(TextBox&& other) = default;

  TextBox& SetText(const string& text);

protected:
  friend class PlotManager;
  friend class PlotPainter;
  friend class Plot;

  ptree GetPropertyTree();
  const string& GetText() { return mText; }

private:
  string mText;
};

//**************************************************************************************************
/**
 * Representation of a legend box.
 */
//**************************************************************************************************
class Plot::Pad::LegendBox : public Plot::Pad::Box<LegendBox>
{
public:
  class LegendEntry;
  LegendBox();
  LegendBox(double_t xPos, double_t yPos);
  LegendBox(const ptree& legendBoxTree);

  virtual ~LegendBox() = default;
  LegendBox(const LegendBox& other) = default;
  LegendBox(LegendBox&&) = default;
  LegendBox& operator=(const LegendBox& other) = default;
  LegendBox& operator=(LegendBox&& other) = default;

  LegendBox& SetTitle(const string& title);
  LegendBox& SetNumColumns(uint8_t numColumns);
  LegendEntry& GetEntry(uint8_t entryID) { return mLegendEntriesUser[entryID]; }

  LegendBox& SetDefaultDrawStyle(string drawStyle);

  LegendBox& SetDefaultLineColor(int16_t color);
  LegendBox& SetDefaultLineStyle(int16_t style);
  LegendBox& SetDefaultLineWidth(float_t width);

  LegendBox& SetDefaultMarkerColor(int16_t color);
  LegendBox& SetDefaultMarkerStyle(int16_t style);
  LegendBox& SetDefaultMarkerSize(float_t size);

  LegendBox& SetDefaultFillColor(int16_t color);
  LegendBox& SetDefaultFillStyle(int16_t style);
  LegendBox& SetDefaultFillOpacity(float_t opacity);

protected:
  friend class PlotManager;
  friend class PlotPainter;
  friend class Plot;

  ptree GetPropertyTree();
  const optional<uint8_t>& GetNumColumns() { return mNumColumns; }
  const optional<string>& GetTitle() { return mTitle; }

  const vector<LegendEntry>& GetEntries() { return mLegendEntries; }

  LegendEntry& AddEntry(const string& name, const string& lable);

  const auto& GetDefaultDrawStyle() { return mDrawStyleDefault; }

  const auto& GetDefaultMarkerColor() { return mMarkerDefault.color; }
  const auto& GetDefaultMarkerStyle() { return mMarkerDefault.style; }
  const auto& GetDefaultMarkerSize() { return mMarkerDefault.scale; }

  const auto& GetDefaultLineColor() { return mLineDefault.color; }
  const auto& GetDefaultLineStyle() { return mLineDefault.style; }
  const auto& GetDefaultLineWidth() { return mLineDefault.scale; }

  const auto& GetDefaultFillColor() { return mFillDefault.color; }
  const auto& GetDefaultFillStyle() { return mFillDefault.style; }
  const auto& GetDefaultFillOpacity() { return mFillDefault.scale; }

private:
  optional<string> mTitle;
  optional<uint8_t> mNumColumns;
  vector<LegendEntry> mLegendEntries;           // this is transient and will be generated automatically
  map<uint8_t, LegendEntry> mLegendEntriesUser; // this is persistent and must be saved

  layout_t mLineDefault;
  layout_t mMarkerDefault;
  layout_t mFillDefault;
  optional<string> mDrawStyleDefault;

  void MergeLegendEntries();
};

//**************************************************************************************************
/**
 * Representation of a legend box entry.
 */
//**************************************************************************************************
class Plot::Pad::LegendBox::LegendEntry
{
public:
  LegendEntry(const optional<string>& lable = {}, const optional<string>& refDataName = {},
              const optional<string>& drawStyle = {})
    : mFill{}, mMarker{}, mLine{}
  {
    mLable = lable;
    mRefDataName = refDataName;
    mDrawStyle = drawStyle;
  }

  LegendEntry(const ptree& legendEntryTree);

  LegendEntry& SetLable(const string& lable);
  LegendEntry& SetRefData(const string& name, const string& inputIdentifier);
  LegendEntry& SetDrawStyle(const string& drawStyle);

  LegendEntry& SetMarkerColor(int16_t color);
  LegendEntry& SetMarkerStyle(int16_t style);
  LegendEntry& SetMarkerSize(float_t size);

  LegendEntry& SetLineColor(int16_t color);
  LegendEntry& SetLineStyle(int16_t style);
  LegendEntry& SetLineWidth(float_t width);

  LegendEntry& SetFillColor(int16_t color);
  LegendEntry& SetFillStyle(int16_t style);
  LegendEntry& SetFillOpacity(float_t opacity);

  LegendEntry& SetTextColor(int16_t color);
  LegendEntry& SetTextFont(int16_t font);
  LegendEntry& SetTextSize(float_t size);

protected:
  friend class PlotPainter;
  friend class LegendBox;
  void operator+=(const LegendEntry& legendEntry);

  ptree GetPropertyTree();
  const auto& GetRefDataName() const { return mRefDataName; }
  const auto& GetLable() const { return mLable; }
  const auto& GetDrawStyle() const { return mDrawStyle; }

  const auto& GetMarkerColor() const { return mMarker.color; }
  const auto& GetMarkerStyle() const { return mMarker.style; }
  const auto& GetMarkerSize() const { return mMarker.scale; }

  const auto& GetLineColor() const { return mLine.color; }
  const auto& GetLineStyle() const { return mLine.style; }
  const auto& GetLineWidth() const { return mLine.scale; }

  const auto& GetFillColor() const { return mFill.color; }
  const auto& GetFillStyle() const { return mFill.style; }
  const auto& GetFillOpacity() const { return mFill.scale; }

  const auto& GetTextColor() const { return mText.color; }
  const auto& GetTextFont() const { return mText.style; }
  const auto& GetTextSize() const { return mText.scale; }

private:
  optional<string> mLable;
  optional<string> mRefDataName;
  optional<string> mDrawStyle;
  layout_t mFill;
  layout_t mMarker;
  layout_t mLine;
  layout_t mText;
};

} // end namespace PlottingFramework
#endif /* Plot_h */
