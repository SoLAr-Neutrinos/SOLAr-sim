/**
 * @author      Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        compare_shielding
 * @created     martedì giu 13, 2023 11:09:28 CEST
 */

#include <iostream>
#include <vector>
#include <algorithm>
#include "TFile.h"
#include "TTree.h"
#include "TSystem.h"
#include "TStyle.h"
#include "TCanvas.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TGraphErrors.h"
#include "TF1.h"
#include "TRandom3.h"
#include "TPRegexp.h"
#include "TCutG.h"
#include "TCut.h"
#include "TLegend.h"
#include "THStack.h"

#define NSET 3

TFile* file_output[NSET]; 
TTree* tree[NSET]; 
TString  file_tag[NSET] = {"bare", "bpoly", "bbricks"}; 

double n_events[NSET] = {291000, 650000, 850000}; 

double count_events(TTree* t) {
  int iev =   0; 
  int iev_tmp  = -10; 
  double n = 0; 

  t->SetBranchAddress("iEv", &iev); 

  for (int i=0; i<t->GetEntries(); i++) {
    t->GetEntry(i); 
    if (iev != iev_tmp) {
      n++; 
      iev_tmp = iev; 
    }
  }

  return n;
}

void compare_shielding(bool do_count_events = false) {
  file_output[0] = new TFile("./gpvm_20230616/ext_n_bare_output.root"); 
  file_output[1] = new TFile("./gpvm_20230616/ext_n_bpol_output.root"); 
  file_output[2] = new TFile("./gpvm_20230616/ext_n_bbricks40_output.root");  

  tree[0] = (TTree*)file_output[0]->Get("externals"); 
  tree[1] = (TTree*)file_output[1]->Get("externals"); 
  tree[2] = (TTree*)file_output[2]->Get("externals"); 

  if (do_count_events) {
    for (int i=0; i<3; i++) {
      n_events[i] = count_events(tree[i]); 

      printf("%s: %g events\n", file_output[i]->GetName(), n_events[i]);
    }
  }
}


void compare_shielding_1d(TString var, TString cut, TString opt, TString name) 
{
  TH1F* h[3] = {0}; 
  for (int i=0; i<3; i++) {
    tree[i]->Draw(var, cut, opt+"goff"); 
    h[i] = (TH1F*)gDirectory->Get("h1")->Clone(file_tag[i]+"_"+name); 

    h[i]->Scale( 1.0 / n_events[i] ); 
  }
  return;
}

struct ProcessHist_t {
  TH1D* fHist; 
  TString fProcess; 
  Double_t fIntegral; 

  ProcessHist_t() : fHist(nullptr), fProcess(""), fIntegral(0.) {}
  ProcessHist_t(TString proc, TH1D* h, double intgrl) :
    fProcess(proc), fHist(h), fIntegral(intgrl) {}; 
  ~ProcessHist_t() {if (fHist) delete fHist;}

  bool operator < (const ProcessHist_t& hother) const {
    printf("calling that\n");
    return (fIntegral < hother.fIntegral); 
  }
  bool operator < (const ProcessHist_t* hother) const {
    printf("calling this\n");
    return (fIntegral < hother->fIntegral); 
  }
};

typedef std::vector<ProcessHist_t*> ProcessHistVec_t; 

void extract_process_counts(int iserie) {
  gStyle->SetPalette(kRainBow); 
  TColor::InvertPalette(); 
  // extract contribution from 
  // - support structure (steel),
  // - external shielding (water)
  // - foam + plywood (foam)
  // - borated polyethilene (bpe)
  // - LAr (lar)

  TCut weight("weight"); 
  TCut copyPU("origin_vol == 22 || origin_vol == 29"); 
  TCut copyBP("origin_vol == 23");
  TCut copyLAr("(origin_vol == 10 || origin_vol == 11 || origin_vol == 9) && lar_origin_energy == origin_energy"); 
  TCut copyWater("origin_vol == 2 && (creator.Contains(\"H1\") || creator.Contains(\"O16\"))"); 
  TCut copySteel("(creator.Contains(\"Fe\") || creator.Contains(\"Ni\") || creator.Contains(\"Cr\") )"); 
  TCut copyWood("origin_vol == 22 || origin_vol == 26 || origin_vol == 28 || origin_vol == 30"); 
  TCut gamma("pdgID == 22"); 
  TCut inLAr("lar_origin_energy > 0"); 
  TCut noTransport("creator != \"Transportation\""); 

  enum EOrigin {kWater, kSteel, kBPE, kFoam, kLAr}; 
  Color_t lineColl[5] = {kMagenta+1, kGray+2, kGreen+3, kCyan+2 , kBlue+1}; 
  Color_t fillColl[5] = {kMagenta-9, kGray  , kGreen-3, kCyan-10, kAzure-4};

  TCut originCut[5] = {
    copyWater && !copySteel, 
    copySteel, 
    copyBP && !copySteel, 
    (copyPU || copyWood) && !copySteel, 
    copyLAr && !copySteel}; 
  TString originTag[5] = {"Water", "Steel", "B-PE", "Foam", "LAr"}; 
  ProcessHistVec_t procVec[5]; 

  std::vector<TH1D*> hGlob; 

  for (int j=0; j<5; j++) {
    if (iserie == 0 && (j == 0 || j == 2)) continue;
    else if (iserie == 1 && j==0) continue;
    tree[iserie]->Draw(
        "lar_origin_energy:creator>>h2(100, 0, 100, 120, 0, 12)", 
        weight*(gamma && inLAr && originCut[j]), "goff"); 

    TH2F* h2 = (TH2F*)gDirectory->Get("h2")->Clone("h2_larEne_creator_"+file_tag[iserie]); 
    h2->Scale( 1.0 / n_events[iserie]);

    std::cout << "done" << std::endl;

    for (int i=1; i<= h2->GetNbinsX(); i++) {

      TString process = h2->GetXaxis()->GetBinLabel(i); 
      TH1D* h = h2->ProjectionY(file_tag[iserie]+"_"+originTag[j]+"_"+process, 
          i, i, "eo");

      ProcessHist_t* hist = new ProcessHist_t(process, h, h->Integral()); 
      if (hist->fIntegral > 0) {
        printf("Adding %s\n", process.Data());
        procVec[j].push_back( hist ); 
      }
    }

    delete h2; 
  
    //printf("Before sorting: \n");
    //for (const auto &proc : procVec[j]) printf("%s - %g\n", 
        //proc->fProcess.Data(), proc->fIntegral);
    std::sort( procVec[j].begin(), procVec[j].end(), 
        [](const ProcessHist_t* h0, const ProcessHist_t* h1) {
        return (h0->fIntegral > h1->fIntegral);}); 
    //printf("After sorting: \n");
    //for (const auto &proc : procVec[j]) printf("%s - %g\n", 
        //proc->fProcess.Data(), proc->fIntegral);

    TLegend* leg = new TLegend(0.5, 0.89, 0.89, 0.5); 
    TCanvas* cOrigin = new TCanvas("c"+originTag[j], "c"+originTag[j], 0, 0, 500, 400); 
    const size_t imax = TMath::Min((size_t)10, procVec[j].size()); 
    printf("imax is %lu\n", imax);

    TH1D* h_all = (TH1D*)procVec[j].front()->fHist->Clone(
        originTag[j]+"_"+file_tag[iserie]); 
    h_all->Reset(); 
    h_all->SetLineWidth(2); h_all->SetLineColor(kGray+2); 
    h_all->SetMarkerStyle(20); 
    h_all->SetMarkerSize(0.8); 
    leg->AddEntry(h_all, "#it{#gamma} produced in "+originTag[j], "pl"); 

    int iproc = 0; 
    THStack* hstack = new THStack(originTag[j]+"_"+file_tag[iserie]+"_stack", "#it{#gamma} creator process - "+originTag[j]); 
    for (const auto &proc : procVec[j]) {
      proc->fHist->SetLineWidth(2); 
      proc->fHist->SetLineColor(kBlack); 
      proc->fHist->SetFillColor( gStyle->GetColorPalette( (iproc+1) * 255 / (imax+1) ) ); 
      h_all->Add( proc->fHist ); 

      if (iproc < imax ) {
        printf("[%i]: %s -> integral %g\n", 
            iproc, proc->fProcess.Data(), proc->fIntegral);
        hstack->Add( proc->fHist, "hist"); 
        leg->AddEntry( proc->fHist, proc->fProcess, "f"); 
      }

      iproc++; 
    }

    gStyle->SetOptTitle(1); 
    gStyle->SetOptStat(0); 
    cOrigin->cd(); 
    cOrigin->SetTicks(1, 1); 
    cOrigin->SetGrid(1, 1); 
    hstack->Draw("noclear");
    hstack->GetXaxis()->SetTitle("Energy at LAr interface [MeV]"); 
    hstack->GetYaxis()->SetTitle("Counts (scaled)"); 
    h_all->Draw("pe same"); 
    leg->Draw(); 

    hGlob.push_back( (TH1D*)h_all->Clone() ); 
  }

  TCanvas* cGlob = new TCanvas("cGlob"+file_tag[iserie], "cGlob"+file_tag[iserie],
      0, 0, 700, 900); 
  cGlob->SetTicks(1, 1); 
  cGlob->SetGrid(1, 1); 

  TH1D* h_all = (TH1D*)hGlob.front()->Clone("h_all_"+file_tag[iserie]); 
  h_all->Reset(); 
  
  tree[iserie]->Draw(Form("lar_origin_energy>>%s", h_all->GetName()), 
      weight*(gamma && inLAr ), "goff"); 

  h_all->Scale( 1.0 / n_events[iserie] ); 

  TLegend* legGlob = new TLegend(0.5, 0.89, 0.89, 0.5); 

  h_all->Draw("axis"); 



  for (const auto& h : hGlob) {
    EOrigin kOrigin; 
    TString hname = h->GetName(); 
    if (hname.Contains("Steel")) kOrigin = kSteel;
    else if (hname.Contains("Water")) kOrigin = kWater; 
    else if (hname.Contains("Foam")) kOrigin = kFoam;
    else if (hname.Contains("B-PE")) kOrigin = kBPE;
    else if (hname.Contains("LAr")) kOrigin = kLAr;

    h->SetLineWidth(2); 
    h->SetFillColorAlpha( fillColl[kOrigin] , 0.4); 
    h->SetLineColor( lineColl[kOrigin] ); 
    
    legGlob->AddEntry(h, 
        Form("%s: %g #gamma/n", 
          originTag[kOrigin].Data(), h->Integral()), "f"); 

    h->Draw("hist same"); 
  }

  h_all->Draw("pe same"); 
  legGlob->Draw(); 

}

//std::vector<ProcessHistVec_t> run_process_counts() {
  //std::vector<ProcessHistVec_t> process_data; 

  //TH2F* h2[NSET] = {nullptr}; 

  //TPRegexp rgxSteel("Fe\\d+|Ni\\d+|Cr\\d+"); 
  //TPRegexp rgxFoam("C\\d+|H1"); 


  //for (int n=0; n<NSET; n++) {
    //auto procVec = extract_process_counts(n); 
    //process_data.push_back(procVec); 
  //}

  //return process_data; 
//}

void neutron_stoppers(const int iserie) {
  TCut weight("weight"); 
  TCut copyPU("final_vol == 22 || final_vol == 29"); 
  TCut copyBP("final_vol == 23");
  TCut copyLAr("(final_vol == 10 || final_vol == 11 || final_vol == 9) && lar_origin_energy > 0"); 
  TCut copyWater("final_vol == 2 && (terminator.Contains(\"H1\") || terminator.Contains(\"O16\"))"); 
  TCut copySteel("(terminator.Contains(\"Fe\") || terminator.Contains(\"Ni\") || terminator.Contains(\"Cr\") )"); 
  TCut copyWood("final_vol == 22 || final_vol == 26 || final_vol == 28 || final_vol == 30"); 
  TCut neutron("pdgID == 2112"); 
  TCut noTransport("terminator != \"Transportation\""); 

  TString hist_name = file_tag[iserie]+"_n_destroyed";
  TH1D* hist_n_destroyed = new TH1D(hist_name, 
      "Destroyed neutron spectrum;Energy [MeV];Counts (scaled)]", 150, 0, 15); 
  hist_n_destroyed->SetXTitle("Energy [MeV]"); 
  hist_n_destroyed->SetYTitle("Counts (scaled)"); 
  hist_n_destroyed->SetTitleFont(43, "xy"); 
  hist_n_destroyed->SetLabelFont(43, "xy"); 
  hist_n_destroyed->SetTitleSize(20, "xy"); 
  hist_n_destroyed->SetLabelSize(20, "xy"); 


  tree[iserie]->Draw("origin_energy>>"+hist_name, 
      weight*(neutron && noTransport), 
      "goff"); 
  hist_n_destroyed->Scale( 1.0 / n_events[iserie] ); 

  double f_destroyed = hist_n_destroyed->Integral(); 

  // Get neutrons killed in the polyurethane + plywood 
  hist_name = file_tag[iserie]+"_n_destroyed_foam"; 
  TH1D* hist_n_destroyed_foam = (TH1D*)hist_n_destroyed->Clone(hist_name); 
  tree[iserie]->Draw("origin_energy>>"+hist_name, 
      weight*(neutron && noTransport && (copyWood || copyPU)), 
      "goff"); 
  hist_n_destroyed_foam->Scale( 1.0 / n_events[iserie] ); 
  double f_destroyed_foam = hist_n_destroyed_foam->Integral() / f_destroyed;

  hist_name = file_tag[iserie]+"_n_destroyed_lar"; 
  TH1D* hist_n_destroyed_lar = (TH1D*)hist_n_destroyed->Clone(hist_name); 
  hist_n_destroyed_lar->Reset(); 
  tree[iserie]->Draw("origin_energy>>"+hist_name, 
      weight*(neutron && noTransport && (copyLAr)), 
      "goff"); 
  hist_n_destroyed_lar->Scale( 1.0 / n_events[iserie] ); 
  double f_destroyed_lar = hist_n_destroyed_lar->Integral() / f_destroyed;

  hist_name = file_tag[iserie]+"_n_destroyed_bpol"; 
  TH1D* hist_n_destroyed_bpol = (TH1D*)hist_n_destroyed->Clone(hist_name); 
  hist_n_destroyed_bpol->Reset(); 
  tree[iserie]->Draw("origin_energy>>"+hist_name, 
      weight*(neutron && noTransport && (copyBP)), 
      "goff"); 
  hist_n_destroyed_bpol->Scale( 1.0 / n_events[iserie] ); 
  double f_destroyed_bpol = hist_n_destroyed_bpol->Integral() / f_destroyed;

  hist_name = file_tag[iserie]+"_n_destroyed_water"; 
  TH1D* hist_n_destroyed_water = (TH1D*)hist_n_destroyed->Clone(hist_name); 
  hist_n_destroyed_water->Reset(); 
  tree[iserie]->Draw("origin_energy>>"+hist_name, 
      weight*(noTransport && (copyWater)), 
      "goff"); 
  hist_n_destroyed_water->Scale( 1.0 / n_events[iserie] ); 
  double f_destroyed_water = hist_n_destroyed_water->Integral() / f_destroyed;

  hist_name = file_tag[iserie]+"_n_destroyed_steel"; 
  TH1D* hist_n_destroyed_steel = (TH1D*)hist_n_destroyed->Clone(hist_name); 
  hist_n_destroyed_steel->Reset(); 
  tree[iserie]->Draw("origin_energy>>"+hist_name, 
      weight*(neutron && noTransport && (copySteel)), 
      "goff"); 
  hist_n_destroyed_steel->Scale( 1.0 / n_events[iserie] ); 
  double f_destroyed_steel = hist_n_destroyed_steel->Integral() / f_destroyed;

  hist_n_destroyed_lar->SetLineWidth(2); 
  hist_n_destroyed_lar->SetLineColor(kBlue+1); 
  hist_n_destroyed_lar->SetFillColor(kAzure-4); 
  
  hist_n_destroyed_water->SetLineWidth(2); 
  hist_n_destroyed_water->SetLineColor(kMagenta+1); 
  hist_n_destroyed_water->SetFillColor(kMagenta-9); 

  hist_n_destroyed_bpol->SetLineWidth(2); 
  hist_n_destroyed_bpol->SetLineColor(kGreen+3); 
  hist_n_destroyed_bpol->SetFillColor(kGreen-3);   

  hist_n_destroyed_foam->SetLineWidth(2); 
  hist_n_destroyed_foam->SetLineColor(kCyan+2); 
  hist_n_destroyed_foam->SetFillColor(kCyan-10);   

  hist_n_destroyed_steel->SetLineWidth(2); 
  hist_n_destroyed_steel->SetLineColor(kGray+2); 
  hist_n_destroyed_steel->SetFillColor(kGray);   
  
  hist_n_destroyed->SetMarkerStyle(20); 
  hist_n_destroyed->SetMarkerSize(0.7); 
  hist_n_destroyed->SetLineWidth(2); 

  TCanvas* cStop = new TCanvas("cStop", "cStop", 0, 0, 700, 800); 
  cStop->SetTopMargin(0.05); cStop->SetRightMargin(0.05); 
  cStop->SetLeftMargin(0.12); cStop->SetBottomMargin(0.11); 
  cStop->SetLogy(1); 
  TLegend* leg = new TLegend(0.5, 0.94, 0.94, 0.5); 
  leg->AddEntry(hist_n_destroyed, "Destroyed neutrons", "lp"); 

  THStack* hstack = new THStack(); 
  if (iserie == 2) {
    hstack->Add(hist_n_destroyed_water, "hist"); 
    leg->AddEntry(hist_n_destroyed_water, 
        Form("Water [%.2f %%]", f_destroyed_water*100), "f"); 
  }
  hstack->Add(hist_n_destroyed_steel, "hist"); 
  leg->AddEntry(hist_n_destroyed_steel, 
      Form("Steel [%.2f %%]", f_destroyed_steel*100), "f"); 
  if (iserie >  0) {
    hstack->Add(hist_n_destroyed_bpol, "hist");
    leg->AddEntry(hist_n_destroyed_bpol, 
        Form("B-PE [%.2f %%]", f_destroyed_bpol*100), "f"); 
  }
  hstack->Add(hist_n_destroyed_foam, "hist"); 
  leg->AddEntry(hist_n_destroyed_foam, 
      Form("Foam + Wood [%.2f %%]", f_destroyed_foam*100), "f"); 
  hstack->Add(hist_n_destroyed_lar, "hist"); 
  leg->AddEntry(hist_n_destroyed_lar, 
      Form("LAr [%.2f %%]", f_destroyed_lar*100), "f"); 

  hstack->Draw();
  hist_n_destroyed->Draw("pe same"); 
  leg->SetTextFont(43); leg->SetTextSize(20); 
  leg->Draw(); 
}

