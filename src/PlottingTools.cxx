// Plotting Framework
//
// Copyright (C) 2019-2020  Mario Krüger
// Contact: mario.kruger@cern.ch
// For a full list of contributors please see docs/Credits
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

#include "PlottingTools.h"
#include "PlotManager.h"

using namespace PlottingFramework;
namespace PlottingFramework {
namespace PlottingTools{

shared_ptr<TCanvas> GeneratePlot(Plot& plot, PlotStyle& plotStyle, TObjArray* availableData){
  
  // this should be in crator
  if(plotStyle.GetNPads() < plot.GetNumRequiredPads())
  {
    cout << "ERROR: Number of pads in style '"  << plotStyle.GetName() << "' ("
    << plotStyle.GetNPads() << ") does not match the number of pads needed for plotting '"
    << plot.GetUniqueName() << "' (" << plot.GetNumRequiredPads() << ")." << endl;
    // TODO: also catch if pad indices are available!
    return nullptr;
  }
  
  // Create empty plot
  gStyle->SetOptStat(0); // this needs to be done before creating the canvas! at later stage it would add to list of primitives in pad...
  string canvasName = plot.GetUniqueName();
  TCanvas* canvas = new TCanvas(canvasName.c_str(), canvasName.c_str(), plotStyle.GetWidth()+4, plotStyle.GetHeight()+28); // undo hard-coded offsets in TCanvas.cxx line 580
  canvas->SetMargin(0., 0., 0., 0.); // todo make this editable?
  canvas->SetFillStyle(plotStyle.GetFillStyle());
  //canvas->SetFixedAspectRatio();
  int padID = 1;
  for(auto& padStyle : plotStyle.GetPadStyles())
  {
    string controlString = plot.GetControlString(padID);
    
    vector<string> errorStyles;
    vector<string> lables;
    TObjArray legendEntries(1);
    
    
    canvas->cd();
    string padName = "Pad_" + std::to_string(padID);
    TPad* pad = new TPad(padName.c_str(),"", padStyle.GetXLow(), padStyle.GetYLow(), padStyle.GetXUp(), padStyle.GetYUp());
    //cout << "PAD dimensons: " <<  padStyle.GetXLow() << "," << padStyle.GetYLow() << "," << padStyle.GetXUp() << "," << padStyle.GetYUp() << endl;
    pad->SetFillStyle(plotStyle.GetFillStyle()); // todo make this pad dependent
    //pad->SetFillColor(plotStyle.GetFillColor());
    pad->SetTopMargin(padStyle.GetTopMargin());
    pad->SetBottomMargin(padStyle.GetBottomMargin());
    pad->SetLeftMargin(padStyle.GetLeftMargin());
    pad->SetRightMargin(padStyle.GetRightMargin());
    pad->SetNumber(padID);
    pad->Draw();
    pad->cd();
    
    // pad style settings (currently global, should be per pad...) TODO: avoid useless function calls
    TGaxis::SetMaxDigits(3);
    gStyle->SetPalette(plotStyle.GetPallette());
    gStyle->SetTextFont(plotStyle.GetTextFont());
    gStyle->SetLabelFont(plotStyle.GetLableFont(), "XY"); //XYZ
    gStyle->SetLabelFont(plotStyle.GetLableFont(), "Z");
    gStyle->SetTitleFont(plotStyle.GetTitleFont(), "XY");
    gStyle->SetTitleFont(plotStyle.GetTitleFont(), "Z");
    gStyle->SetTextSize(plotStyle.GetTextSize());
    gStyle->SetLabelSize(plotStyle.GetLableSize(), "XY");
    gStyle->SetLabelSize(plotStyle.GetLableSize(), "Z");
    gStyle->SetTitleSize(plotStyle.GetTitleSize(), "XY");
    gStyle->SetTitleSize(plotStyle.GetTitleSize(), "Z");
    
    //gStyle->SetPadColor(kGray); // todo this should be steerable from outside
    gStyle->SetTitleFont(plotStyle.GetTextFont());
    gStyle->SetTitleW((pad->GetX2() - pad->GetRightMargin()) - (pad->GetX1() + pad->GetLeftMargin()));
    gStyle->SetTitleH(pad->GetTopMargin()*0.8);
    gStyle->SetTitleAlign(kHAlignCenter + kVAlignTop);
    //gStyle->SetTitleBorderSize(1);
    gStyle->SetMarkerSize(plotStyle.GetMarkerSize());
    
    string drawingOptions = "";
    int dataIndex = 0;
    for(auto data : plot.GetData(padID)){
      int color = (data->GetColor()) ? data->GetColor() : plotStyle.GetDefaultColor(dataIndex); // TODO 0 is white!!
      int style = (data->GetStyle()) ? data->GetStyle() : plotStyle.GetDefaultMarker(dataIndex); // only gets marker not line style
      
      if(color < 0) {dataIndex += color; color = plotStyle.GetDefaultColor(dataIndex);} // todo how to implement this feature better?
      if(style < 0) {style = plotStyle.GetDefaultMarker(dataIndex);} // todo how to implement this feature better?
      
      drawingOptions += data->GetDrawingOptions(); // errorStyle etc
      // setdefaultstyles only once per pad (textsize, etc)!!
      if(data->GetType() == "hist")
      {
        TH1* histo = GetDataClone<TH1>(data->GetUniqueName(), availableData);
        if(!histo) continue; // avoid crashes if something goes wrong
        
        // do modifications to histogram
        CutHistogram(histo, std::dynamic_pointer_cast<Plot::Histogram>(data)->GetHistCutHigh(), std::dynamic_pointer_cast<Plot::Histogram>(data)->GetHistCutLow());
        if(controlString.find("normalize") != string::npos) histo->Scale(1/histo->Integral(), "width");
        
        if(controlString.find("thick") != string::npos){
          histo->SetLineWidth(plotStyle.GetLineWidthThick());
          histo->SetMarkerSize(plotStyle.GetMarkerSizeThick());
        }
        
        histo->UseCurrentStyle();
        histo->SetMarkerStyle(style);
        histo->SetMarkerColor(color);
        histo->SetLineColor(color);
        
        if(histo->InheritsFrom("TH2"))
        {
          drawingOptions += string(" ") + plotStyle.GetDefault2DStyle();
          if(plotStyle.GetDefault2DStyle() == "COLZ") gStyle->SetNumberContours(256); // TODO make this flexible
        }
        
        if(drawingOptions.find("none") != string::npos)
        {
          drawingOptions.erase(drawingOptions.find("none"), string("none").length());
          histo->SetLineWidth(0);
        }
        if(drawingOptions.find("hist") != string::npos)
        {
          drawingOptions += " HIST";
        }
        else if(drawingOptions.find("band") != string::npos)
        {
          drawingOptions.erase(drawingOptions.find("band"), string("band").length());
          drawingOptions += " E5";
          
          histo->SetMarkerSize(0.);
          histo->SetFillColor(color);
          histo->SetFillStyle(1);
        }
        else if(drawingOptions.find("boxes") != string::npos)
        {
          drawingOptions.erase(drawingOptions.find("boxes"), string("boxes").length());
          
          TExec errorBoxesOn("errorBoxesOn","gStyle->SetErrorX(0.48)");
          errorBoxesOn.Draw();
          histo->SetFillStyle(0);
          drawingOptions += " E2";
          TExec errorBoxesOff("errorBoxesOff","gStyle->SetErrorX(0)");
          errorBoxesOff.Draw("");
        }
        histo->Draw(drawingOptions.c_str());
        
      }
      else if(data->GetType() == "ratio")
      {
        TH1* ratio = GetDataClone<TH1>(data->GetUniqueName(), availableData);
        TH1* temp = GetDataClone<TH1>(std::dynamic_pointer_cast<Plot::Ratio>(data)->GetUniqueNameDenom(), availableData);
        if(!ratio || !temp) continue; // avoid crashes if something goes wrong
        ratio->Divide(temp); // todo add here alternative divide functions and options
        delete temp;
        
        // do modifications to histogram
        CutHistogram(ratio, std::dynamic_pointer_cast<Plot::Histogram>(data)->GetHistCutHigh(), std::dynamic_pointer_cast<Plot::Histogram>(data)->GetHistCutLow());
        
        ratio->SetTitle("");
        ratio->UseCurrentStyle();
        
        if(ratio->InheritsFrom("TH2"))
        {
          drawingOptions += string(" ") + plotStyle.GetDefault2DStyle();
          
          TView* view = TView::CreateView(1);
          view->SetRange(-0.5,-1,-3.610432,100.5,1.778151,0.1470754);
          pad->SetTheta(49.5);
          pad->SetPhi(230);
        }
        else{
          ratio->GetYaxis()->CenterTitle(1);
          ratio->GetXaxis()->SetTickLength(0.06);
          ratio->GetYaxis()->SetNdivisions(305); //506
          if(dataIndex == 0)
          {
            // todo:: this should be way more flexible!!
            TH1* dummyHist = (TH1*)ratio->Clone("dummy");
            dummyHist->GetXaxis()->SetTickLength(0.06); // todo:: this should automatically be the same as for the main plot!!
            dummyHist->GetYaxis()->SetNdivisions(305); //506
            dummyHist->SetLineColor(0); // make plot invisible
            TF1* line = new TF1("line", "1", dummyHist->GetXaxis()->GetXmin(), dummyHist->GetXaxis()->GetXmax());
            dummyHist->Draw("AXIS");
            //line->SetLineColor(color);
            line->SetLineColor(kBlack);
            line->SetLineWidth(2);
            // line->SetLineStyle(9);
            line->Draw("SAME");
            drawingOptions += " SAME";
            dataIndex++;
            color = (data->GetColor()) ? data->GetColor() : plotStyle.GetDefaultColor(dataIndex);
            style = (data->GetStyle()) ? data->GetStyle() : plotStyle.GetDefaultMarker(dataIndex);
          }
        }
        if(drawingOptions.find("boxes") != string::npos)
        {
          drawingOptions.erase(drawingOptions.find("boxes"), string("boxes").length());
          TExec errorBoxesOn("errorBoxesOn","gStyle->SetErrorX(0.48)");
          errorBoxesOn.Draw();
          ratio->SetFillStyle(0);
          drawingOptions += " E2";
          TExec errorBoxesOff("errorBoxesOff","gStyle->SetErrorX(0)");
          errorBoxesOff.Draw("");
        }
        ratio->SetMarkerStyle(style);
        ratio->SetMarkerColor(color);
        ratio->SetLineColor(color);
        ratio->Draw(drawingOptions.c_str());
      }
      else if(data->GetType() == "graph")
      {
        TGraph* graph = GetDataClone<TGraph>(data->GetUniqueName(), availableData);
        if(!graph) continue; // avoid crashes if something goes wrong
        //graph->SetTitle("");
        
        // do modifications to graph
        CutGraph(graph, std::dynamic_pointer_cast<Plot::Graph>(data)->GetGraphCutHigh(), std::dynamic_pointer_cast<Plot::Graph>(data)->GetGraphCutLow());
        
        
        graph->UseCurrentStyle();
        graph->SetMarkerStyle(style);
        graph->SetMarkerColor(color);
        graph->SetLineColor(color);
        
        if(padID ==2){
          graph->GetYaxis()->CenterTitle(1);
          graph->GetXaxis()->SetTickLength(0.06);
          graph->GetYaxis()->SetNdivisions(305); //506
        }
        if(drawingOptions.find("boxes") != string::npos)
        {
          drawingOptions.erase(drawingOptions.find("boxes"), string("boxes").length());
          TExec errorBoxesOn("errorBoxesOn","gStyle->SetErrorX(0.48)");
          errorBoxesOn.Draw();
          graph->SetFillStyle(0);
          drawingOptions = "E2 SAME"; // bug
          TExec errorBoxesOff("errorBoxesOff","gStyle->SetErrorX(0)");
          errorBoxesOff.Draw("");
        }
        
        
        if(dataIndex == 0) drawingOptions += " AP";
        graph->Draw(drawingOptions.c_str());
      }
      else{
        cout << "ERROR: no matching representation found for " << data->GetName() << endl;
        continue;
      }
      
      dataIndex++;
      drawingOptions = "EP SAME"; // next data should be drawn to same pad
      
      if (!data->GetLable().empty()) {
        lables.push_back(data->GetLable());
        legendEntries.Add(pad->GetListOfPrimitives()->Last());
        errorStyles.push_back(data->GetDrawingOptions());
      }
    }
    
    // TODO: set range and log scale properties must affect all linked pad-axes
    // also add safety in case log and range are not compatible (zero in range)
    if(controlString.find("logX") != string::npos)
    {
      pad->SetLogx();
    }
    if(controlString.find("logY") != string::npos)
    {
      pad->SetLogy();
    }
    if(controlString.find("logZ") != string::npos)
    {
      pad->SetLogz();
    }
    if(controlString.find("gridX") != string::npos)
    {
      pad->SetGridx();
    }
    if(controlString.find("gridY") != string::npos)
    {
      pad->SetGridy();
    }
    // todo this is to figure out bug in log scale tpave moving
    //pad->Modified();
    //pad->Update();
    //pad->ResizePad();
    
    int legendIndex = 1;
    int textIndex = 1;
    // now place legends, textboxes and shapes
    for(auto box : plot.GetBoxes(padID))
    {
      if(box->GetType() == "legend")
      {
        string legendName = "LegendBox_" + std::to_string(legendIndex);
        if(lables.empty()) break;
        TLegend* legend = MakeLegend(std::static_pointer_cast<Plot::LegendBox>(box), pad, legendEntries, lables, errorStyles);
        legend->SetName(legendName.c_str());
        legend->Draw("SAME");
        // todo this is to figure out bug in log scale tpave moving
        //legend->ConvertNDCtoPad();
        //legend->SetBorderSize(2);
        //legend->SetLineWidth(2);
        //legend->SetLineColor(kRed);
        legendIndex++;
      }
      else if(box->GetType() == "text")
      {
        string textName = "TextBox_" + std::to_string(textIndex);
        TPaveText* text = MakeText(std::static_pointer_cast<Plot::TextBox>(box));
        text->SetName(textName.c_str());
        text->Draw("SAME");
        
        // todo this is to figure out bug in log scale tpave moving
        //text->ConvertNDCtoPad();
        //text->SetBorderSize(2);
        //text->SetLineWidth(2);
        //text->SetLineColor(kRed);
        
        textIndex++;
        
      }
    }
    
    // after data is drawn to pad axis porperties can be set
    if(!pad->GetListOfPrimitives() || !pad->GetListOfPrimitives()->At(0)) continue; // if pad is empty
    
    TObject* axisObject = nullptr;
    // skip all non drawable objects stored in list of primitives
    // todo merge this with the code below...
    for(auto pointer : *pad->GetListOfPrimitives())
    {
      if(pointer->InheritsFrom(TH1::Class()) || pointer->InheritsFrom(TGraph::Class()) || pointer->InheritsFrom(TF1::Class())) {axisObject = pointer; break;}
    }
    TH1* axisHist = nullptr;
    if(axisObject->InheritsFrom(TH1::Class())) axisHist = (TH1*)axisObject;
    else if(axisObject->InheritsFrom(TGraph::Class())) axisHist = (TH1*)((TGraph*)axisObject)->GetHistogram();
    else if(axisObject->InheritsFrom(TF1::Class())) axisHist = (TH1*)((TF1*)axisObject)->GetHistogram();
    else cout << "ERROR: Cannot handle type " << typeid(axisObject).name() << endl;
    if(!axisHist){cout << "ERROR: unable to access axes!!" << endl; continue;}
    axisHist->GetXaxis()->SetTitleOffset(padStyle.GetTitleOffsetX());
    axisHist->GetYaxis()->SetTitleOffset(padStyle.GetTitleOffsetY());
    axisHist->GetZaxis()->SetTitleOffset(padStyle.GetTitleOffsetZ());
    axisHist->SetTitle(padStyle.GetTitle().c_str());
    
    
    // todo write axis wrapper, avoid multiple calls of setting axis range, how to handle more than one axis change?
    // change axis range
    for(auto& padID : plotStyle.GetLinkedPads("X", padID)){
      if(plot.IsAxisDefined(padID, "X") && plot.GetAxis(padID, "X")->IsRangeSet())
      {
        axisHist->GetXaxis()->SetRangeUser(plot.GetAxis(padID, "X")->GetAxisRange().first, plot.GetAxis(padID, "X")->GetAxisRange().second);
      }
    }
    for(auto& padID : plotStyle.GetLinkedPads("Y", padID)){
      if(plot.IsAxisDefined(padID, "Y") && plot.GetAxis(padID, "Y")->IsRangeSet())
      {
        axisHist->GetYaxis()->SetRangeUser(plot.GetAxis(padID, "Y")->GetAxisRange().first, plot.GetAxis(padID, "Y")->GetAxisRange().second);
      }
    }
    for(auto& padID : plotStyle.GetLinkedPads("Z", padID)){
      if(plot.IsAxisDefined(padID, "Z") && plot.GetAxis(padID, "Z")->IsRangeSet())
      {
        axisHist->GetZaxis()->SetRangeUser(plot.GetAxis(padID, "Z")->GetAxisRange().first, plot.GetAxis(padID, "Z")->GetAxisRange().second);
      }
    }
    // change axis title
    if(plot.IsAxisDefined(padID, "X") && plot.GetAxis(padID, "X")->IsTitleSet())
    {
      axisHist->GetXaxis()->SetTitle(plot.GetAxis(padID, "X")->GetTitle().c_str());
    }
    if(plot.IsAxisDefined(padID, "Y") && plot.GetAxis(padID, "Y")->IsTitleSet())
    {
      axisHist->GetYaxis()->SetTitle(plot.GetAxis(padID, "Y")->GetTitle().c_str());
    }
    if(plot.IsAxisDefined(padID, "Z") && plot.GetAxis(padID, "Z")->IsTitleSet())
    {
      axisHist->GetZaxis()->SetTitle(plot.GetAxis(padID, "Z")->GetTitle().c_str());
    }
    
    if(axisHist->InheritsFrom("TH2"))
    {
      pad->SetRightMargin(0.12+0.06);
      pad->SetTopMargin(0.12-0.05);
      pad->SetBottomMargin(0.12+0.02);
      
      axisHist->GetXaxis()->SetTitleOffset(1.1); //1.1
      axisHist->GetYaxis()->SetTitleOffset(1.1); //1.3
      axisHist->GetZaxis()->SetTitleOffset(1.6); //1.6
      // 2d hacks, re-adjust palette
      pad->Update(); // this adds something to list of primitives!! do not call here
      TPaletteAxis* palette = (TPaletteAxis*)axisHist->GetListOfFunctions()->FindObject("palette");
      if(palette)
      {
        palette->SetX2NDC(0.865); //0.88
        palette->SetTitleOffset();
      }
      pad->Update(); // this adds something to list of primitives!! do not call here
    }
    
    
    pad->Modified();
    pad->Update();
    padID++;
  }
  
  // finally apply canvas properties
  canvas->cd();
  if(plotStyle.IsDrawTimestamps()){
    TTimeStamp time;
    TString stTime;
    stTime = Form("#color[16]{%i}", time.GetDate()); // TODO: make format user definable
    TLatex timeStamp;
    timeStamp.SetNDC();
    timeStamp.SetTextAlign(22);
    timeStamp.SetTextFont(63);
    timeStamp.SetTextSizePixels(10);
    timeStamp.DrawLatex(plotStyle.GetTimestampPositon().first, plotStyle.GetTimestampPositon().second, stTime);
  }
  
  
  // patch truncated lowest lable
  //TPad* patch = new TPad("patch", "patch", mainPad->GetLeftMargin()/2 ,0, mainPad->GetLeftMargin()-0.004, mStyle.textSize / mainPad->YtoPixel(mainPad->GetY1()));
  //patch->Draw("SAME");
  //patch->SetFillColor(mainPad->GetFillColor());
  //patch->SetBorderMode(0);
  
  //    canvas->SetTheta(20);
  //    canvas->SetPhi(45);
  return shared_ptr<TCanvas>(canvas);
}


template<typename T>
T* GetDataClone(string dataName, TObjArray* availableData)
{
  T* data = nullptr;
  TObject* obj = availableData->FindObject(dataName.c_str());
  if(obj)
  {
    if(!obj->InheritsFrom(T::Class()))
    {
      cout << "ERROR: " << dataName << " is of type " << obj->ClassName() << " instead of " << string(typeid(T).name()).substr(1) << "." << endl;
    }
    else
    {
      data = (T*)obj->Clone();
      //data->SetDirectory(0); // does not work for tgraphs because they are not attached to current file
    }
  }else{
    cout << "ERROR: " << dataName << " was not loaded." << endl;
  }
  return data;
}


TLegend* MakeLegend(shared_ptr<Plot::LegendBox> legendBox, TPad* pad, TObjArray& legendEntries, vector<string> legendTitles, vector<string>& errorStyles){
  
  // todo this has to be included in legend box...
  double textSizePixel = 24;  //legendBox->GetTextSize;
  int textFont = 43; //legendBox->GetTextFont;
  
  int nColumns = legendBox->GetNumColumns();
  
  string dummyString = "AAA";
  int nEntries = legendEntries.GetEntries();
  if(!legendBox->GetTitle().empty()) nEntries++;
  
  int padWidthPixel = pad->XtoPixel(pad->GetX2()); // todo this is probably incorrect
  int padHeightPixel = pad->YtoPixel(pad->GetY1());
  
  
  // determine max width and height of legend entries
  int iColumn = 0;
  double legendWidthPixel = 0;
  double titleWidthPixel = 0;
  vector<unsigned int> legendWidthPixelPerColumn(nColumns, 0);
  double legendHeightPixel = 0;
  if(!legendBox->GetTitle().empty()) legendTitles.push_back(legendBox->GetTitle());
  int iLegend = 1;
  for(auto& legendTitle : legendTitles)
  {
    if(legendTitle.find("<name>") != string::npos)
    {
      string name = ((TNamed*)legendEntries[iLegend-1])->GetName();
      name = name.substr(0, name.find(gNameGroupSeparator));
      
      legendTitle.replace(legendTitle.find("<name>"), string("<name>").size(), name);
    }
    if(legendTitle.find("<title>") != string::npos)
    {
      string title = ((TNamed*)legendEntries[iLegend-1])->GetTitle();
      legendTitle.replace(legendTitle.find("<title>"), string("<title>").size(), title);
    }
    if(legendTitle.find("<entries>") != string::npos && legendEntries[iLegend-1]->InheritsFrom(TH1::Class()))
    {
      // todo add some formating options for this
      string entries = std::to_string((long int)((TH1*)legendEntries[iLegend-1])->GetEntries());
      legendTitle.replace(legendTitle.find("<entries>"), string("<entries>").size(), entries);
    }
    if(legendTitle.find("<integral>") != string::npos && legendEntries[iLegend-1]->InheritsFrom(TH1::Class()))
    {
      // todo add some formating options for this
      string integral = std::to_string(((TH1*)legendEntries[iLegend-1])->Integral());
      legendTitle.replace(legendTitle.find("<integral>"), string("<integral>").size(), integral);
    }
    if(legendTitle.find("<mean>") != string::npos && legendEntries[iLegend-1]->InheritsFrom(TH1::Class()))
    {
      // todo add some formating options for this
      string mean = std::to_string(((TH1*)legendEntries[iLegend-1])->GetMean());
      legendTitle.replace(legendTitle.find("<mean>"), string("<mean>").size(), mean);
    }
    if(legendTitle.find("<maximum>") != string::npos && legendEntries[iLegend-1]->InheritsFrom(TH1::Class()))
    {
      // todo add some formating options for this
      string maximum = std::to_string(((TH1*)legendEntries[iLegend-1])->GetMaximum());
      legendTitle.replace(legendTitle.find("<maximum>"), string("<maximum>").size(), maximum);
    }
    if(legendTitle.find("<minimum>") != string::npos && legendEntries[iLegend-1]->InheritsFrom(TH1::Class()))
    {
      // todo add some formating options for this
      string minimum = std::to_string(((TH1*)legendEntries[iLegend-1])->GetMinimum());
      legendTitle.replace(legendTitle.find("<minimum>"), string("<minimum>").size(), minimum);
    }
    
    unsigned int height = 0;
    unsigned int width = 0;
    TLatex textLine(0,0, legendTitle.c_str());
    textLine.SetTextFont(textFont);
    textLine.SetTextSize(textSizePixel);
    textLine.GetBoundingBox(width, height);
    if(height > legendHeightPixel) legendHeightPixel = height;
    if(!legendBox->GetTitle().empty() && iLegend == legendTitles.size())
    {
      titleWidthPixel = width;
      continue;
    }
    if(width > legendWidthPixelPerColumn[iColumn]) legendWidthPixelPerColumn[iColumn] = width;
    iColumn++;
    iColumn %= nColumns;
    iLegend++;
  }
  for(auto& length : legendWidthPixelPerColumn) legendWidthPixel += length;
  
  unsigned int markerWidthPixel = 0;
  {
    TLatex textLine(0,0, dummyString.c_str());
    textLine.SetTextFont(textFont);
    textLine.SetTextSize(textSizePixel);
    unsigned int height = 0;
    textLine.GetBoundingBox(markerWidthPixel, height);
  }
  
  double legendWidthNDC =  (double)legendWidthPixel/padWidthPixel;
  double legendHeightNDC =  (double)legendHeightPixel/padHeightPixel;
  double markerWidthNDC =  (double)markerWidthPixel/padWidthPixel;
  double titleWidthNDC =  (double)titleWidthPixel/padWidthPixel;
  
  double totalHeightNDC = (nEntries + 0.5*(nEntries+1))* legendHeightNDC / nColumns;
  double totalWidthNDC = (nColumns + 0.3333) * markerWidthNDC + legendWidthNDC;
  if(titleWidthPixel > legendWidthPixel) totalWidthNDC = (0.3333) * markerWidthNDC + titleWidthNDC;
  
  double upperLeftX = legendBox->GetXPosition();
  double upperLeftY = legendBox->GetYPosition();
  
  if(legendBox->IsAutoPlacement())
  {
    pad->cd();
    pad->Update();
    double lowerLeftX = 0;
    double lowerLeftY = 0;
    double fractionOfTickLenght = 0.9;
    // required distance in pad coordinates of box to objects and tics (set to be 90& of the tick length)
    double marginX = fractionOfTickLenght * gStyle->GetTickLength("Y") * (pad->GetUxmax() - pad->GetUxmin()) / (pad->GetX2()-pad->GetX1());
    double marginY = fractionOfTickLenght * gStyle->GetTickLength("X") * (pad->GetUymax() - pad->GetUymin()) / (pad->GetY2()-pad->GetY1());
    bool foundPosition = false;
    
    // draw temporary boxes to exclude areas outside of the coordinate system
    TBox marginsBottom (pad->GetX1(), pad->GetY1(), pad->GetX2(),pad->GetUymin() + gStyle->GetTickLength("X") * (pad->GetUymax() - pad->GetUymin()));
    TBox marginsTop (pad->GetX1(), pad->GetUymax() - gStyle->GetTickLength("X") * (pad->GetUymax() - pad->GetUymin()), pad->GetX2(),pad->GetY2());
    TBox marginsLeft (pad->GetX1(), pad->GetY1(), pad->GetUxmin() + gStyle->GetTickLength("Y") * (pad->GetUxmax() - pad->GetUxmin()), pad->GetY2());
    TBox marginsRight (pad->GetUxmax() - gStyle->GetTickLength("Y") * (pad->GetUxmax() - pad->GetUxmin()), pad->GetY1(), pad->GetX2(),pad->GetY2());
    marginsBottom.Draw("SAME");
    marginsTop.Draw("SAME");
    marginsLeft.Draw("SAME");
    marginsRight.Draw("SAME");
    
    // find box position that does not collide with any of the drawn objects
    for(TObject* o : *pad->GetListOfPrimitives())
    {
      foundPosition = pad->PlaceBox(o, totalWidthNDC + 2*marginX, totalHeightNDC + 2*marginY, lowerLeftX, lowerLeftY);
    }
    if(foundPosition)
    {
      upperLeftX = lowerLeftX + 2*marginX;
      upperLeftY = lowerLeftY + totalHeightNDC + 2*marginY;
    }
    else{
      cout << "Warning: Could not find enough space to place legend properly." << endl;
      // just place legend within axis ranges of pad
      upperLeftX = (pad->GetUxmin() - pad->GetX1())/(pad->GetX2()-pad->GetX1()) + (1+1/fractionOfTickLenght)*marginX;
      upperLeftY = (pad->GetUymax() - pad->GetY1())/(pad->GetY2()-pad->GetY1()) - (1+1/fractionOfTickLenght)*marginY;
      
      // && ! legendBox.
      //if(legendBox.IsResizeToFitLgend()){}
      /*
       TODO extent y range if auto placing does not find enough space
       TObject* dummyObject = pad->GetListOfPrimitives()->At(1);
       if(dummyObject && dummyObject->InheritsFrom(TH1::Class())){
       TH1* hist = (TH1*)dummyObject;
       cout << "hello " << hist->GetYaxis()->GetXmin() << "  " << totalHeightNDC / pad->GetWh()  << endl;
       hist->GetYaxis()->SetRangeUser(pad->GetUymin(), pad->GetUymax());
       pad->Update();
       }
       */
      
    }
    // now remove temporary boxes again
    pad->GetListOfPrimitives()->Remove(&marginsBottom);
    pad->GetListOfPrimitives()->Remove(&marginsTop);
    pad->GetListOfPrimitives()->Remove(&marginsLeft);
    pad->GetListOfPrimitives()->Remove(&marginsRight);
    
    //pad->DrawCollideGrid();
  }
  else if(legendBox->IsUserCoordinates())
  {
    // convert user coordinates to NDC
    pad->Update();
    upperLeftX = (upperLeftX - pad->GetX1())/(pad->GetX2()-pad->GetX1());
    upperLeftY = (upperLeftY - pad->GetY1())/(pad->GetY2()-pad->GetY1());
  }
  
  TLegend* legend = new TLegend(upperLeftX, upperLeftY - totalHeightNDC, upperLeftX + totalWidthNDC, upperLeftY, "", "NDC");
  if(!legendBox->GetTitle().empty()) legend->SetHeader((legendBox->GetTitle()).c_str(), ""); // option "C" allows to center the header
  
  int i = 0;
  for(auto entry : legendEntries)
  {
    string drawStyle = "ep";
    
    if((entry->InheritsFrom("TF1")) || errorStyles[i] == "hist")// || noErrBars)
    {
      drawStyle = "l";
    }
    
    legend->AddEntry((TH1*)entry, legendTitles[i].c_str(), drawStyle.c_str());
    i++;
  }
  legend->SetNColumns(nColumns);
  legend->SetMargin((markerWidthNDC*nColumns)/totalWidthNDC);
  legend->SetTextFont(textFont);
  legend->SetTextSize(textSizePixel);
  legend->SetTextAlign(kHAlignLeft+kVAlignBottom);
  legend->SetEntrySeparation(1.0 - 0.5* legendHeightNDC/( 0.5*(totalHeightNDC/(double)legend->GetNRows())));
  
  if(legend->GetHeader())
  {
    ((TLegendEntry*)(legend->GetListOfPrimitives()->At(0)))->SetTextFont(textFont);
    ((TLegendEntry*)(legend->GetListOfPrimitives()->At(0)))->SetTextSize(textSizePixel);
  }
  
  legend->SetLineStyle(legendBox->GetBorderStyle());
  legend->SetLineColor(legendBox->GetBorderColor());
  legend->SetLineWidth(legendBox->GetBorderSize());
  legend->SetFillStyle(0);
  //legend->SetFillStyle(legendBox->GetFillStyle());
  //legend->SetFillColor(legendBox->GetFillColor());
  return legend;
}


TPaveText* MakeText(shared_ptr<Plot::TextBox> textBox){
  // todo this has to be included in text box...
  double textSizePixel = 24;  //textBox->GetTextSize;
  int textFont = 43; //textBox->GetTextFont;
  
  string delimiter = " // ";
  string text = textBox->GetText();
  
  int nLetters = 0;
  vector<string> lines;
  
  size_t pos = string::npos;
  do{
    pos = text.find(delimiter);
    string token = text.substr(0, pos);
    if(token.length() > nLetters) nLetters = token.length();
    lines.push_back(token);
    size_t newStart = pos;
    if (pos != string::npos) newStart += delimiter.length();
    text.erase(0, newStart);
  } while (pos != string::npos);
  
  
  int nLines = lines.size();
  
  double textSizeNDC = textSizePixel / gPad->YtoPixel(gPad->GetY1());
  double textSizeNDCx = 0.6*textSizePixel / gPad->XtoPixel(gPad->GetX2());
  
  double margin = 0.5*textSizePixel;
  double yWidth = (1.0*nLines + 0.5*(nLines-1))* textSizeNDC;
  double xWidth = nLetters * textSizeNDCx;
  
  double upperLeftX = textBox->GetXPosition();
  double upperLeftY = textBox->GetYPosition();
  
  if(textBox->IsUserCoordinates())
  {
    gPad->Update();
    //      upperLeftX = (upperLeftX - gPad->GetX1())/(gPad->GetX2()-gPad->GetX1());
    //      upperLeftY = (upperLeftY - gPad->GetY1())/(gPad->GetY2()-gPad->GetY1());
    upperLeftX = (upperLeftX - gPad->GetX1())/(gPad->GetX2()-gPad->GetX1());
    upperLeftY = (upperLeftY - gPad->GetY1())/(gPad->GetY2()-gPad->GetY1());
  }
  
  TPaveText* tPaveText = new TPaveText(upperLeftX, upperLeftY - yWidth, upperLeftX + xWidth, upperLeftY, "NDC");
  
  double boxExtent = 0;
  for(auto &line : lines)
  {
    TText* text = tPaveText->AddText(line.c_str());
    text->SetTextFont(textFont);
    text->SetTextSize(textSizePixel);
    double width = text->GetBBox().fWidth;
    if(width > boxExtent) boxExtent = width;
  }
  tPaveText->SetBBoxX2(tPaveText->GetBBox().fX + boxExtent +2*margin);
  tPaveText->SetBorderSize(1);
  tPaveText->SetLineStyle(textBox->GetBorderStyle());
  tPaveText->SetLineColor(textBox->GetBorderColor());
  tPaveText->SetLineWidth(textBox->GetBorderSize());
  tPaveText->SetMargin(margin/(tPaveText->GetBBox().fX + boxExtent +2*margin));
  tPaveText->SetTextAlign(12);
  tPaveText->SetTextFont(textFont);
  tPaveText->SetTextSize(textSizePixel);
  tPaveText->SetFillStyle(0); //todo fix this hard coded value, 4000 only works for pads!
  //    tPaveText->SetTextColor(kRed);
  return tPaveText;
}


//****************************************************************************************
/**
 * Deletes data points of histogram beyond cutoff value (and below lower cutoff value).
 * @param hist: histogram to cut
 * @param cutoff: user axis value after which data is set to zero
 * @param cutoffLow: user axis value before which data is set to zero
 */
//****************************************************************************************
void CutHistogram(TH1* hist, double cutoff, double cutoffLow)
{
  if(cutoff < -997) return;
  int cutoffBin = hist->GetXaxis()->FindBin(cutoff);
  for(int i = cutoffBin; i <= hist->GetNbinsX(); i++){
    hist->SetBinContent(i, 0);
    hist->SetBinError(i, 0);
  }
  if(cutoffLow < -997) return;
  int cutoffBinLow = hist->GetXaxis()->FindBin(cutoffLow);
  for(int i = 1; i <= cutoffBinLow; i++){
    hist->SetBinContent(i, 0);
    hist->SetBinError(i, 0);
  }
  
}

//****************************************************************************************
/**
 * Deletes data points of graph beyond cutoff value (and below lower cutoff value).
 * @param graph: graph to cut
 * @param cutoff: x value after which data is set to zero
 * @param cutoffLow: x axis value before which data is set to zero
 */
//****************************************************************************************
void CutGraph(TGraph* graph, double cutoff, double cutoffLow)
{
  bool cutLow = true;
  bool cutHigh = true;
  
  if(cutoff < -997) cutHigh = false;
  if(cutoffLow < -997) cutLow = false; // TODO this is obviously a dangerous way to implement this.... Will lead to wrong results at some point
  
  int pointsToRemoveHigh = 0;
  int pointsToRemoveLow = 0;
  
  for(int i = 0; i < graph->GetN(); i++){
    if(cutLow && graph->GetX()[i] < cutoffLow)
    {
      pointsToRemoveLow++;
    }
    if(cutHigh && graph->GetX()[i] > cutoff)
    {
      pointsToRemoveHigh++;
    }
  }
  
  for(auto i = 0; i < pointsToRemoveHigh; i++)
  {
    graph->RemovePoint(graph->GetN()-1);
  }
  for(auto i = 0; i < pointsToRemoveLow; i++)
  {
    graph->RemovePoint(0);
  }
  
}

//****************************************************************************************
/**
 * Helper-function dividing two TGraphs
 */
//****************************************************************************************

TGraph* DivideTSpline(TGraph* numerator, TGraph* denominator){
  
  TGraph* result = (TGraph*)numerator->Clone("ratio");
  TSpline3* denSpline = new TSpline3("denSpline", denominator);
  
  int nPoints = result->GetN();
  
  double *x = result->GetX();
  double *y = result->GetY();
  double *ey = result->GetEY();
  
  for(int i = 0; i < nPoints; i++) {
    double deonomValiue = denominator->Eval(x[i], denSpline);
    y[i] = y[i] / deonomValiue;
    ey[i] = ey[i] * deonomValiue;
  }
  delete denSpline;
  return result;
}

//****************************************************************************************
/**
 * Helper-function dividing two histograms with different binning.
 */
//****************************************************************************************

TH1* DivideTSpline(TH1* numerator, TH1* denominator)
{
  TGraph denominatorGraph(denominator);
  TSpline3 denominatorSpline(denominator);
  
  TH1* ratio = (TH1*)numerator->Clone("dummyRatio");
  ratio->Reset();
  
  for(int i = 1; i <= numerator->GetNbinsX(); i++)
  {
    double numeratorValue = numerator->GetBinContent(i);
    double x = numerator->GetBinCenter(i);
    double denomValue = denominatorGraph.Eval(x, &denominatorSpline);
    if(denomValue) ratio->SetBinContent(i, numeratorValue/denomValue);
  }
  return ratio;
}

} // end namespace PlottingTools
} // end namespace PlottingFramework
