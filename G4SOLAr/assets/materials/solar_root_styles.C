/**
 * @author      : Daniele Guffanti (University and INFN Milano-Bicocca)
 * @file        : wfms_root_styles
 * @created     : lunedì gen 24, 2022 11:43:18 CET
 */

#include <iostream>
#include "TStyle.h"
#include "TInterpreter.h"


void slide_default() {
  printf("ROOT: sourcing slide_default style...\n");

  TStyle* slide_default = new TStyle("slide_default", "Default slide plot style");

  //------------------------------ Global settings
  slide_default->SetTextFont(43);
  slide_default->SetPadTickX(1);
  slide_default->SetPadTickY(1);
  slide_default->SetPadLeftMargin  (0.12);
  slide_default->SetPadRightMargin (0.12);
  slide_default->SetPadBottomMargin(0.12);
  slide_default->SetPalette(kBlackBody);
  slide_default->SetCanvasBorderMode(0);
  slide_default->SetFrameBorderMode(0);
  slide_default->SetPadBorderMode(0);
  slide_default->SetCanvasColor(kWhite);
  slide_default->SetPadColor(kWhite);
  //------------------------------ Title settings
  slide_default->SetTitleX(0.5);
  slide_default->SetTitleAlign(23);
  slide_default->SetTitleFont(43);
  slide_default->SetTitleBorderSize(0);
  slide_default->SetTitleFillColor(0);
  //------------------------------ Stat settings
  slide_default->SetStatBorderSize(1);
  slide_default->SetStatFont(43);
  slide_default->SetStatColor(0);
  //------------------------------ Axes settings
  slide_default->SetLabelFont(43, "xy");
  slide_default->SetLabelSize(23, "xy");
  slide_default->SetTitleFont(43, "xy");
  slide_default->SetTitleSize(23, "xy");
  slide_default->SetTitleOffset(2.5, "xy");
  //------------------------------ Hist settings
  slide_default->SetHistLineWidth(2);
  slide_default->SetMarkerStyle(20);
  //------------------------------ Legend settings
  slide_default->SetLegendFont(43);
  slide_default->SetLegendTextSize(23);
  slide_default->SetLegendBorderSize(1);
  
  return;
}

void setdate() {
  if (gStyle->GetOptDate() > 0) {
    gStyle->SetOptDate(0);
  } else {
    gStyle->SetOptDate(223);
    auto date_ = gStyle->GetAttDate();
    date_->SetTextFont(42);
    date_->SetTextSize(0.04);
    date_->SetTextAngle(90);
    gStyle->SetDateX( 1.0 - gStyle->GetPadRightMargin() +0.02);
    gStyle->SetDateY( 1.0 - gStyle->GetPadTopMargin  () +0.00);
  }

  return;
}


